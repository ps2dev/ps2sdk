/*
# _____     ___ ____     ___ ____
#  ____|   |    ____|   |        | |____|
# |     ___|   |____ ___|    ____| |    \    PS2DEV Open Source Project.
#-----------------------------------------------------------------------
# Copyright (c) 2006-2007 Eugene Plotnikov <e-plotnikov@operamail.com>
# Licenced under Academic Free License version 2.0
# Review ps2sdk README & LICENSE files for further details.
# Based on refernce software of MSSG
*/

#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <kernel.h>
#include <ee_regs.h>
#include <dma_tags.h>

#include "libmpeg.h"
#include "libmpeg_internal.h"

#define READ_ONCE(x)       (*(const volatile typeof(x) *)(&x))
#define WRITE_ONCE(x, val) (*(volatile typeof(x) *)(&x) = (val))

struct CSCParam
{
    u32 source;
    u32 dest;
    u32 blocks;
};

struct IPUState
{
    u32 d4_chcr;
    u32 d4_madr;
    u32 d4_qwc;

    u32 d3_chcr;
    u32 d3_madr;
    u32 d3_qwc;

    u32 ipu_ctrl;
    u32 ipu_bp;
};

static qword_t s_DMAPack[17] __attribute__((aligned(64)));
static int (*s_SetDMA_func)(void *);
static void *s_SetDMA_arg;
static struct IPUState s_IPUState;
static int *s_pEOF;
static int s_Sema;
static struct CSCParam s_CSCParam;
static int s_CSCID;
static u8 s_CSCFlag;
static u32 s_BitsBuffered;
static u32 s_LocalBits;

static u32 s_QmIntra[16] __attribute__((aligned(16))) = {
    0x13101008,
    0x16161310,
    0x16161616,
    0x1B1A181A,
    0x1A1A1B1B,
    0x1B1B1A1A,
    0x1D1D1D1B,
    0x1D222222,
    0x1B1B1D1D,
    0x20201D1D,
    0x26252222,
    0x22232325,
    0x28262623,
    0x30302828,
    0x38382E2E,
    0x5345453A,
};

static u32 s_QmNonIntra[16] __attribute__((aligned(16))) = {
    0x10101010,
    0x10101010,
    0x10101010,
    0x10101010,
    0x10101010,
    0x10101010,
    0x10101010,
    0x10101010,
    0x10101010,
    0x10101010,
    0x10101010,
    0x10101010,
    0x10101010,
    0x10101010,
    0x10101010,
    0x10101010,
};


extern s32 _mpeg_dmac_handler(s32 channel, void *arg, void *addr);

#define IPU_CMD_BUSY (1ull << 63)

#define IPU_CTRL_BUSY (1 << 31)
#define IPU_CTRL_RST  (1 << 30)
#define IPU_CTRL_MP1  (1 << 23)
#define IPU_CTRL_ECD  (1 << 14)


// clang-format off
#define IPU_COMMAND_BCLR         0x00000000
#define IPU_COMMAND_IDEC         0x10000000
#define IPU_COMMAND_BDEC         0x20000000
#define IPU_COMMAND_VDEC         0x30000000
#define  IPU_COMMAND_VDEC_MBAI   0x00000000
#define  IPU_COMMAND_VDEC_MBTYPE 0x04000000
#define  IPU_COMMAND_VDEC_MCODE  0x08000000
#define  IPU_COMMAND_VDEC_DMV    0x0C000000
#define IPU_COMMAND_FDEC         0x40000000
#define IPU_COMMAND_SETIQ        0x50000000
#define IPU_COMMAND_SETVQ        0x60000000
#define IPU_COMMAND_CSC          0x70000000
#define IPU_COMMAND_PACK         0x80000000
#define IPU_COMMAND_SETTH        0x90000000
// clang-format on

void _MPEG_Initialize(_MPEGContext *mc, int (*data_cb)(void *), void *cb_user, int *eof_flag)
{
    (void)mc;
    ee_sema_t sema;

    *R_EE_IPU_CTRL = IPU_CTRL_RST;
    while (*R_EE_IPU_CTRL & IPU_CTRL_BUSY)
        ;
    *R_EE_IPU_CMD = IPU_COMMAND_BCLR;
    while (*R_EE_IPU_CTRL & IPU_CTRL_BUSY)
        ;
    *R_EE_IPU_CTRL |= IPU_CTRL_MP1;
    *R_EE_D3_QWC  = 0;
    *R_EE_D4_QWC  = 0;
    s_SetDMA_func = data_cb;
    s_SetDMA_arg  = cb_user;
    s_pEOF        = eof_flag;
    *s_pEOF       = 0;
    // TODO: check if this is the correct options for the semaphore
    // Everything except init_count is uninitialized stack garbage in the ASM version
    memset(&sema, 0, sizeof(sema));
    sema.init_count = 0;
    sema.max_count  = 1;
    sema.option     = 0;
    s_Sema          = CreateSema(&sema);
    s_CSCID         = AddDmacHandler2(3, _mpeg_dmac_handler, 0, &s_CSCParam);
    s_BitsBuffered  = 0;
    s_LocalBits     = 0;
}

void _MPEG_Destroy(void)
{
    while (READ_ONCE(s_CSCFlag) != 0)
        ;
    RemoveDmacHandler(3, s_CSCID);
    DeleteSema(s_Sema);
}

void _ipu_suspend(void)
{
    int oldintr;
    u32 enabler;

    /* Stop ch 4 */
    oldintr = DIntr();
    enabler = *R_EE_D_ENABLER;

    *R_EE_D_ENABLEW = enabler | 0x10000;
    EE_SYNCL();

    *R_EE_D4_CHCR &= ~0x100;
    s_IPUState.d4_chcr = *R_EE_D4_CHCR;
    s_IPUState.d4_madr = *R_EE_D4_MADR;
    s_IPUState.d4_qwc  = *R_EE_D4_QWC;

    *R_EE_D_ENABLEW = enabler;
    if (oldintr) {
        EIntr();
    }

    /* Wait for IPU output fifo to drain */
    while (*R_EE_IPU_CTRL & 0xf0)
        ;

    /* Stop ch 3 */
    oldintr = DIntr();
    enabler = *R_EE_D_ENABLER;

    *R_EE_D_ENABLEW = enabler | 0x10000;
    EE_SYNCL();

    *R_EE_D3_CHCR &= ~0x100;
    s_IPUState.d3_chcr  = *R_EE_D3_CHCR;
    s_IPUState.d3_madr  = *R_EE_D3_MADR;
    s_IPUState.d3_qwc   = *R_EE_D3_QWC;
    s_IPUState.ipu_ctrl = *R_EE_IPU_CTRL;
    s_IPUState.ipu_bp   = *R_EE_IPU_BP;

    *R_EE_D_ENABLEW = enabler;
    if (oldintr) {
        EIntr();
    }
}

void _MPEG_Suspend(void)
{
    while (READ_ONCE(s_CSCFlag) != 0)
        ;

    _ipu_suspend();
}

void _ipu_resume(void)
{
    if (s_IPUState.d3_qwc != 0) {
        *R_EE_D3_MADR = s_IPUState.d3_madr;
        *R_EE_D3_QWC  = s_IPUState.d3_qwc;
        *R_EE_D3_CHCR = s_IPUState.d3_chcr | 0x100;
    }
    u32 qw_to_reinsert = (s_IPUState.ipu_bp >> 16 & 3) + (s_IPUState.ipu_bp >> 8 & 0xf);
    u32 actual_qwc     = s_IPUState.d4_qwc + qw_to_reinsert;
    if (actual_qwc != 0) {
        *R_EE_IPU_CMD = IPU_COMMAND_BCLR | (s_IPUState.ipu_bp & 0x7f);
        while (*R_EE_IPU_CMD & IPU_CMD_BUSY)
            ;
        *R_EE_IPU_CTRL = s_IPUState.ipu_ctrl;
        *R_EE_D4_MADR  = s_IPUState.d4_madr - (qw_to_reinsert * 16);
        *R_EE_D4_QWC   = actual_qwc;
        *R_EE_D4_CHCR  = s_IPUState.d4_chcr | 0x100;
    }
}

void _MPEG_Resume(void)
{
    _ipu_resume();
}

s32 _mpeg_dmac_handler(s32 channel, void *arg, void *addr)
{
    (void)channel;
    (void)addr;

    struct CSCParam *cp = arg;

    if (cp->blocks == 0) {
        iDisableDmac(3);
        iSignalSema(s_Sema);
        WRITE_ONCE(s_CSCFlag, 0);
        return -1;
    }

    u32 mbc = cp->blocks;
    if (mbc > 0x3ff) {
        mbc = 0x3ff;
    }
    *R_EE_D3_MADR = cp->dest;
    *R_EE_D4_MADR = cp->source;
    cp->source += mbc * 0x180;
    cp->dest += mbc * 0x400;
    cp->blocks    = cp->blocks - mbc;
    *R_EE_D3_QWC  = (mbc * 0x400) >> 4;
    *R_EE_D4_QWC  = (mbc * 0x180) >> 4;
    *R_EE_D4_CHCR = 0x101;
    *R_EE_IPU_CMD = IPU_COMMAND_CSC | mbc;
    *R_EE_D3_CHCR = 0x100;

    ExitHandler();
    return -1;
}

int _MPEG_CSCImage(void *source, void *dest, int mbcount)
{
    _ipu_suspend();
    *R_EE_IPU_CMD = IPU_COMMAND_BCLR;
    *R_EE_D_STAT  = 8; // ack ch3
    int mbc       = mbcount;
    if (mbc > 0x3ff) {
        mbc = 0x3ff;
    }

    *R_EE_D3_MADR = (u32)dest;
    *R_EE_D4_MADR = (u32)source;

    s_CSCParam.source = (u32)source + mbc * sizeof(_MPEGMacroBlock8); // 0x180
    s_CSCParam.dest   = (u32)dest + mbc * 0x400;                      // 1024
    s_CSCParam.blocks = mbcount - mbc;

    *R_EE_D4_QWC = (mbc * 0x180) >> 4;
    *R_EE_D3_QWC = (mbc * 0x400) >> 4;
    EnableDmac(3);
    *R_EE_D4_CHCR = 0x101;
    *R_EE_IPU_CMD = IPU_COMMAND_CSC | mbc;
    *R_EE_D3_CHCR = 0x100;

    WRITE_ONCE(s_CSCFlag, 1);
    WaitSema(s_Sema);
    _ipu_resume();
    return mbc;
}
static int _ipu_bits_in_fifo()
{
    u32 bp_reg;
    u32 bp, ifc, fp;

    bp_reg = *R_EE_IPU_BP;
    bp     = bp_reg & 0x7f;
    ifc    = (bp_reg >> 8) & 0xf;
    fp     = (bp_reg >> 16) & 0x3;

    return (ifc << 7) + (fp << 7) - bp;
}

static int _ipu_needs_bits()
{
    return _ipu_bits_in_fifo() < 32;
}

static int _req_data()
{
    if (*R_EE_D4_QWC == 0) {
        if (s_SetDMA_func(s_SetDMA_arg) == 0) {
            s_BitsBuffered = 32;
            s_LocalBits    = _MPEG_CODE_SEQ_END;
            *s_pEOF        = 1;
            return 1;
        }
    }

    return 0;
}

void _ipu_sync(void)
{
    u32 ctrl = *R_EE_IPU_CTRL;

    while ((ctrl & IPU_CTRL_ECD) == 0) {
        if (_ipu_needs_bits()) {
            if (_req_data()) {
                return;
            }
        }

        if ((ctrl & IPU_CTRL_BUSY) == 0) {
            return;
        }

        ctrl = *R_EE_IPU_CTRL;
    }
}

u32 _ipu_sync_data(void)
{
    u32 ctrl = *R_EE_IPU_CTRL;
    u64 cmd  = *R_EE_IPU_CMD;

    while ((ctrl & IPU_CTRL_ECD) == 0) {
        if (_ipu_needs_bits()) {
            if (_req_data()) {
                return _MPEG_CODE_SEQ_END;
            }
        }

        if ((cmd & IPU_CMD_BUSY) == 0) {
            return cmd;
        }

        cmd  = *R_EE_IPU_CMD;
        ctrl = *R_EE_IPU_CTRL;
    }

    return cmd;
}

unsigned int _ipu_get_bits(unsigned int bits)
{
    u32 ret;

    _ipu_sync();
    if (s_BitsBuffered < bits) {
        *R_EE_IPU_CMD  = IPU_COMMAND_FDEC;
        s_LocalBits    = _ipu_sync_data();
        s_BitsBuffered = 32;
    }
    *R_EE_IPU_CMD  = IPU_COMMAND_FDEC | bits;
    s_BitsBuffered = s_BitsBuffered - bits;
    ret            = s_LocalBits >> ((32 - bits) & 0x1f);
    s_LocalBits    = s_LocalBits << (bits & 0x1f);
    // printf("_ipu_get_bits %d bits ==  %x (%08x)\n", bits, ret, s_LocalBits);

    return ret;
}


unsigned int _MPEG_GetBits(unsigned int bits)
{
    return _ipu_get_bits(bits);
}

unsigned int _ipu_show_bits(unsigned int bits)
{
    if (s_BitsBuffered < bits) {
        _ipu_sync();
        *R_EE_IPU_CMD  = IPU_COMMAND_FDEC;
        s_LocalBits    = _ipu_sync_data();
        s_BitsBuffered = 32;
    }

    return s_LocalBits >> ((32 - bits) & 0x1f);
}

unsigned int _MPEG_ShowBits(unsigned int bits)
{
    return _ipu_show_bits(bits);
}

void _ipu_align_bits(void)
{
    _ipu_sync();
    u32 var3 = -(*R_EE_IPU_BP & 7) & 7;
    if (var3 != 0) {
        _MPEG_GetBits(var3);
    }
}

void _MPEG_AlignBits(void)
{
    _ipu_align_bits();
}

unsigned int _MPEG_NextStartCode(void)
{
    _MPEG_AlignBits();
    while (_MPEG_ShowBits(24) != 1) {
        _MPEG_GetBits(8);
    }

    return _MPEG_ShowBits(32);
}

void _MPEG_SetDefQM(int arg0)
{
    (void)arg0;
    qword_t *q;
    int i;

    _ipu_suspend();
    *R_EE_IPU_CMD = IPU_COMMAND_BCLR;
    while (*R_EE_IPU_CTRL & IPU_CTRL_BUSY)
        ;

    q = (qword_t *)s_QmIntra;
    for (i = 0; i < 4; i++) {
        __asm__ volatile(
            "lq $2, 0(%0)    \n"
            "sq $2, 0(%1)    \n"
            :
            : "d"(&q[i]), "d"(A_EE_IPU_in_FIFO)
            : "2");
    }

    *R_EE_IPU_CMD = IPU_COMMAND_SETIQ;
    while (*R_EE_IPU_CTRL & IPU_CTRL_BUSY)
        ;

    q = (qword_t *)s_QmNonIntra;
    for (i = 0; i < 4; i++) {
        __asm__ volatile(
            "lq $2, 0(%0)    \n"
            "sq $2, 0(%1)    \n"
            :
            : "d"(&q[i]), "d"(A_EE_IPU_in_FIFO)
            : "2");
    }

    *R_EE_IPU_CMD = IPU_COMMAND_SETIQ | 0x08000000;
    while (*R_EE_IPU_CTRL & IPU_CTRL_BUSY)
        ;

    _ipu_resume();
}

void _MPEG_SetQM(int iqm)
{
    _ipu_sync();
    *R_EE_IPU_CMD  = IPU_COMMAND_SETIQ | iqm << 27;
    s_BitsBuffered = 0;
}

int _MPEG_GetMBAI(void)
{
    u32 mbai = 0, ret;
    _ipu_sync();
    while (1) {
        *R_EE_IPU_CMD = IPU_COMMAND_VDEC | IPU_COMMAND_VDEC_MBAI;
        ret           = _ipu_sync_data() & 0xffff;

        if (ret == 0) {
            return 0;
        }

        // Stuffing
        if (ret == 0x22) {
        }

        // MB escape
        if (ret == 0x23) {
            mbai += 0x21;
        }

        if (ret < 0x22) {
            mbai += ret;
            break;
        }
    }

    s_BitsBuffered = 32;
    s_LocalBits    = *R_EE_IPU_TOP;
    return mbai;
}

int _MPEG_GetMBType(void)
{
    _ipu_sync();
    *R_EE_IPU_CMD = IPU_COMMAND_VDEC | IPU_COMMAND_VDEC_MBTYPE;
    u32 ret       = _ipu_sync_data();
    if (ret != 0) {
        ret &= 0xffff;
        s_BitsBuffered = 32;
        s_LocalBits    = *R_EE_IPU_TOP;
    }
    return ret;
}

int _MPEG_GetMotionCode(void)
{
    short mcode;
    u32 ret;

    _ipu_sync();
    *R_EE_IPU_CMD = IPU_COMMAND_VDEC | IPU_COMMAND_VDEC_MCODE;
    ret           = _ipu_sync_data();

    if (ret == 0) {
        return -32768;
    }

    s_BitsBuffered = 32;
    s_LocalBits    = *R_EE_IPU_TOP;

    mcode = (short)ret;

    return mcode;
}
int _MPEG_GetDMVector(void)
{
    u32 ret;

    _ipu_sync();
    *R_EE_IPU_CMD  = IPU_COMMAND_VDEC | IPU_COMMAND_VDEC_DMV;
    ret            = _ipu_sync_data() & 0xffff;
    s_BitsBuffered = 32;
    s_LocalBits    = *R_EE_IPU_TOP;
    return ret;
}

void _MPEG_SetIDCP(void)
{
    unsigned int var1 = _MPEG_GetBits(2);
    *R_EE_IPU_CTRL    = (*R_EE_IPU_CTRL & ~0x30000) | var1 << 0x10;
}

void _MPEG_SetQSTIVFAS(void)
{
    unsigned int qst = _MPEG_GetBits(1);
    unsigned int ivf = _MPEG_GetBits(1);
    unsigned int as  = _MPEG_GetBits(1);
    *R_EE_IPU_CTRL   = (*R_EE_IPU_CTRL & ~0x700000) | qst << 22 | ivf << 21 | as << 20;
}

void _MPEG_SetPCT(unsigned int arg0)
{
    _ipu_sync();
    *R_EE_IPU_CTRL = (*R_EE_IPU_CTRL & ~0x07000000) | arg0 << 0x18;
}

void _MPEG_BDEC(int mbi, int dcr, int dt, int qsc, void *spaddr)
{
    *R_EE_D3_MADR = ((u32)spaddr & ~0xf0000000) | 0x80000000;
    *R_EE_D3_QWC  = 0x30;
    *R_EE_D3_CHCR = 0x100;
    _ipu_sync();
    *R_EE_IPU_CMD = IPU_COMMAND_BDEC | mbi << 27 | dcr << 26 | dt << 25 | qsc << 16;
}

int _MPEG_WaitBDEC(void)
{
    int oldintr;

    while (1) {
        _ipu_sync();
        if ((*s_pEOF != 0)) {
            break;
        }

        if (*R_EE_IPU_CTRL & IPU_CTRL_ECD) {
            break;
        }

        if (*R_EE_D3_QWC == 0) {
            s_BitsBuffered = 32;
            s_LocalBits    = *R_EE_IPU_TOP;
            return 1;
        }
    }

    _ipu_suspend();
    *R_EE_IPU_CTRL = IPU_CTRL_RST;
    _ipu_resume();

    oldintr         = DIntr();
    *R_EE_D_ENABLEW = *R_EE_D_ENABLER | 0x10000;
    *R_EE_D3_CHCR   = 0;
    *R_EE_D_ENABLEW = *R_EE_D_ENABLER & ~0x10000;
    if (oldintr) {
        EIntr();
    }
    *R_EE_D3_QWC   = 0;
    s_BitsBuffered = 0;
    s_LocalBits    = 0;
    return 0;
}

void _MPEG_dma_ref_image(_MPEGMacroBlock8 *mb, _MPEGMotion *motions, s64 n_motions, int width)
{
    const u32 mbwidth = width * sizeof(*mb);
    qword_t *q;
    int i;

    if (n_motions > 0) {
        while (*R_EE_D9_CHCR & 0x100)
            ;

        *R_EE_D9_QWC  = 0;
        *R_EE_D9_SADR = (u32)mb & 0xFFFFFFF;
        *R_EE_D9_TADR = (u32)s_DMAPack;

        q = UNCACHED_SEG(s_DMAPack);
        for (i = 0; i < n_motions; i++) {
            DMATAG_REF(q, 0x30, (u32)motions[i].m_pSrc, 0, 0, 0);
            q++;
            DMATAG_REF(q, 0x30, (u32)motions[i].m_pSrc + mbwidth, 0, 0, 0);
            q++;

            motions[i].m_pSrc = (unsigned char *)mb;
            mb += 4;
        }

        DMATAG_REFE(q, 0, 0, 0, 0, 0);
        motions[i].MC_Luma = NULL;

        EE_SYNCL();
        *R_EE_D9_CHCR = 0x105;
    }
}

/*
// TODO: verify delay slots
void _MPEG_do_mc(_MPEGMotion *arg0)
{
    int var0 = 16;           // addiu   $v0, $zero, 16
    u8 *arg1 = arg0->m_pSrc; // lw      $a1,  0($a0)
    // addiu   $sp, $sp, -16
    u16 *arg2 = (u16 *)arg0->m_pDstY; // lw      $a2,  4($a0)
    int arg3  = arg0->m_X;            // lw      $a3, 12($a0)
    int tmp0  = arg0->m_Y;            // lw      $t0, 16($a0)
    int tmp1  = arg0->m_H;            // lw      $t1, 20($a0)
    int tmp2  = arg0->m_fInt;         // lw      $t2, 24($a0)
    int tmp4  = arg0->m_Field;        // lw      $t4, 28($a0)
    // lw      $t5, 32($a0) <-- MC_Luma
    tmp0 -= tmp4;            // subu    $t0, $t0, $t4
    tmp4 <<= 4;              // sll     $t4, $t4, 4
    arg1 += tmp4;            // addu    $a1, $a1, $t4
    int var1 = var0 - tmp0;  // subu    $v1, $v0, $t0
    int tmp3 = var0 << tmp2; // sllv    $t3, $v0, $t2
    var1 >>= tmp2;           // srlv    $v1, $v1, $t2
    int ta = tmp0 << 4;      // sll     $at, $t0, 4
    // sw      $ra, 0($sp)
    arg1 += ta;                                      // addu    $a1, $a1, $at
    ta = tmp1 - var1;                                // subu    $at, $t1, $v1
    arg0->MC_Luma(arg1, arg2, arg3, tmp3, var1, ta); // jalr    $t5
    arg1 = arg0->m_pSrc;                             // lw      $a1,  0($a0)
    arg2 = (u16 *)arg0->m_pDstCbCr;                  // lw      $a2,  8($a0)
    // lw      $t5, 36($a0) <-- MC_Chroma
    arg1 += 256; // addiu   $a1, $a1, 256
    tmp4 >>= 1;  // srl     $t4, $t4, 1
    arg3 >>= 1;  // srl     $a3, $a3, 1
    tmp0 >>= 1;  // srl     $t0, $t0, 1
    tmp1 >>= 1;  // srl     $t1, $t1, 1
    // lw      $ra, 0($sp)
    tmp0 >>= tmp2;                                     // srlv    $t0, $t0, $t2
    arg1 += tmp4;                                      // addu    $a1, $a1, $t4
    var0 = 8;                                          // addiu   $v0, $zero, 8
    tmp0 <<= tmp2;                                     // sllv    $t0, $t0, $t2
    var1 = var0 - tmp0;                                // subu    $v1, $v0, $t0
    tmp3 = var0 << tmp2;                               // sllv    $t3, $v0, $t2
    var1 >>= tmp2;                                     // srlv    $v1, $v1, $t2
    ta = tmp0 << 3;                                    // sll     $at, $t0, 3
    arg1 += ta;                                        // addu    $a1, $a1, $at
    ta = tmp1 - var1;                                  // subu    $at, $t1, $v1
    arg0->MC_Chroma(arg1, arg2, arg3, tmp3, var1, ta); // jr      $t5
                                                       // addiu   $sp, $sp, 16
}
*/

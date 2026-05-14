/*
# _____     ___ ____     ___ ____
#  ____|   |    ____|   |        | |____|
# |     ___|   |____ ___|    ____| |    \    PS2DEV Open Source Project.
#-----------------------------------------------------------------------
# Copyright 2001-2004, ps2dev - http://www.ps2dev.org
# Licenced under Academic Free License version 2.0
# Review ps2sdk README & LICENSE files for further details.
*/

/**
 * @file
 * EE UGLY DEBUG ON SCREEN
 */

#include <stdio.h>
#include <tamtypes.h>
#include <sifcmd.h>
#include <kernel.h>
#include <rom0_info.h>
#include <stdarg.h>
#include <debug.h>
#include <ee_regs.h>

static short int X = 0, Y = 0;
static short int MX = 80, MY = 40;
static u32 bgcolor = 0, fontcolor = 0xffffff, cursorcolor = 0xffffff;
static short int cursor = 1;

struct t_setupscr
{
    u64 dd0[6];
    u32 dw0[2];
    u64 dd1[1];
    u16 dh[4];
    u64 dd2[21];
};

struct t_setupchar
{
    u64 dd0[4];
    u32 dw0[1];
    u16 x, y;
    u64 dd1[1];
    u32 dw1[2];
    u64 dd2[5];
};

// from gsKit
static int debug_detect_signal()
{
    char romname[14];
    GetRomName(romname);
    return ((romname[4] == 'E') ? 1 : 0);
}

static void Init_GS(int interlace, int omode, int ffmd)
{
    // Reset GS
    *R_EE_GS_CSR = 0x200;
    // Mask interrupts
    GsPutIMR(0xff00);
    // Configure GS CRT
    SetGsCrt(interlace, omode, ffmd);
}

static void SetVideoMode(void)
{
    /*	DISPLAY1	0x0983227c001bf9ff
            DX = 0x27C (636)
            DY = 0x032 (50)

            MAGH = 0x03 (4x)
            MAGV = 0x01 (2x)

            DW = 0x9FF (2560)
            DH = 0x1BF (447)	*/

    *R_EE_GS_PMODE = 0xff62;
    *R_EE_GS_DISPFB2 = 0x1400;
    *R_EE_GS_DISPLAY2 = 0x001bf9ff0983227c;
}

static inline void Dma02Wait(void)
{
    while ((*R_EE_D2_CHCR & 0x100) != 0); // Wait until STR bit of D2_CHCR = 0
}

static void DmaReset(void)
{
    // This appears to have been based on code from Sony that initializes DMA channels 0-9, in bulk.
    *((vu32 *)0x1000a080) = 0; // D2_SADR = 0. Documented to not exist, but is done.
    *R_EE_D2_CHCR = 0;
    *R_EE_D2_TADR = 0;
    *R_EE_D2_MADR = 0;
    *R_EE_D2_ASR1 = 0;
    *R_EE_D2_ASR0 = 0;
    *R_EE_D_STAT = 0xff1f; // Clear all interrupt status under D_STAT, other than SIF0, SIF1 & SIF2.
    *R_EE_D_STAT &= 0xff1f << 16; // Clear all interrupt masks under D_STAT, other SIF0, SIF1 & SIF2. Writing a 1 reverses the bit.
    *R_EE_D_CTRL = 0;
    *R_EE_D_PCR = 0;
    *R_EE_D_SQWC = 0;
    *R_EE_D_RBOR = 0;
    *R_EE_D_RBSR = 0;
    *R_EE_D_CTRL |= 1; // (DMAE 1)
}

/**
 * Initiates a normal-mode DMA transfer over the GIF.
 * @param addr The address of the data to be transfered, which must be 16 byte aligned.
 * @param size The size (in 16 byte quads) of the data to be transfered.
 */

static inline void progdma(void *addr, int size)
{
    *R_EE_D2_QWC = (u32)size; // D2_QWC
    *R_EE_D2_MADR = (u32)addr; // D2_MADR
    *R_EE_D2_CHCR = 0x101; // D2_CHCR = STR 1, DIR 1
}

void scr_setbgcolor(u32 color)
{
    bgcolor = color;
}

void scr_setfontcolor(u32 color)
{
	fontcolor = color;
}

void scr_setcursorcolor(u32 color)
{
	cursorcolor = color;
}

void init_scr(void)
{
    static struct t_setupscr setupscr __attribute__((aligned(16))) = {
        {0x100000000000800E, 0xE, 0xA0000, 0x4C, 0x8C, 0x4E}, // GIFtag (REGS: A+D, NREG 1, FLG PACKED, EOP, NLOOP 14), FRAME_1 (PSM PSMCT32, FBW 10, FBP 0), ZBUF_1 (PSM PSMZ32, ZBP 140)
        {27648, 30976},
        {0x18},                                                           // XYOFFSET_1 (OFX 1728.0, OFY 1936.0)
        {0, 639, 0, 223},                                                 // SCISSOR_1 (SCAX0 0, SCAX1 639, SCAY0 0, SCAY1 223)
        {0x40, 1, 0x1a, 1, 0x46, 0, 0x45, 0x70000,                        // PRMODECONT (AC PRIM), COLCLAMP (CLAMP 1), DTHE (DTHE 0)
         0x47, 0x30000, 0x47, 6, 0, 0x3F80000000000000, 1, 0x79006C00, 5, // TEST_1 (ZTST GREATER, ZTE 1), TEST_1 (ZTST ALWAYS, ZTE 1), PMODE (CRTMD 1, EN2 1), RGBAQ (Q 1.0, A 0, B 0, G 0, R 0), XYZ2 (Z 0.0, Y 1728.0, X 1936.0)
         0x87009400, 5, 0x70000, 0x47}                                    // XYZ2 (Z 0, Y 2160.0, X 2368.0), TEST_1 (ZTST GREATER, ZTE 1)
    };

    X = Y = 0;
    DmaReset();

    Init_GS(1, debug_detect_signal() == 1 ? 3 : 2, 0); // Interlaced, NTSC/PAL and FIELD mode

    SetVideoMode();
    Dma02Wait();
    progdma(&setupscr, 15);
    Dma02Wait();
}

extern u8 msx[];

void scr_putchar(int x, int y, u32 color, int ch)
{
    static struct t_setupchar setupchar __attribute__((aligned(16))) = {
        {0x1000000000000004, 0xE, 0xA000000000000, 0x50}, // GIFtag (REGS: A+D, NREG 1, FLG PACKED, NLOOP 4), BITBLTBUF (DPSM PSMCT32, DBW 10, DBP 0)
        {0},
        100,
        100,
        {0x51},                               // TRXPOS (DSAX 100, DSAY 100)
        {8, 8},                               // TRXREG (RRW 8, RRH 8)
        {0x52, 0, 0x53, 0x800000000008010, 0} // TRXDIR (XDIR Host -> Local), GIFtag (FLG IMAGE, EOP, NLOOP 16)
    };
    /* charmap must be aligned to a 16-bye boundary.  */
    static u32 charmap[64] __attribute__((aligned(16)));
    int i, j, l;
    u8 *font;
    u32 pixel;

    ((struct t_setupchar *)UNCACHED_SEG(&setupchar))->x = x;
    ((struct t_setupchar *)UNCACHED_SEG(&setupchar))->y = y;

    progdma(&setupchar, 6);

    font = &msx[ch * 8];
    for (i = l = 0; i < 8; i++, l += 8, font++) {
        for (j = 0; j < 8; j++) {
            pixel = ((*font & (128 >> j))) ? color : bgcolor;
            *(u32 *)UNCACHED_SEG(&charmap[l + j]) = pixel;
        }
    }

    Dma02Wait();

    progdma(charmap, (8 * 8 * 4) / 16);
    Dma02Wait();
}

void scr_clearchar(int X, int Y)
{
    scr_putchar(X * 8, Y * 8, bgcolor, ' ');
}

void scr_clearline(int Y)
{
    int i;
    for (i = 0; i < MX; i++)
        scr_putchar(i * 8, Y * 8, bgcolor, ' ');
}

void scr_printf(const char *format, ...)
{
    va_list opt;
    va_start(opt, format);
    scr_vprintf(format, opt);
    va_end(opt);
}

void scr_vprintf(const char *format, va_list opt)
{
    char buff[2048];
    int i, bufsz, j;


    bufsz = vsnprintf(buff, sizeof(buff), format, opt);

    for (i = 0; i < bufsz; i++) {
        char c;
        c = buff[i];
        switch (c) {
            case '\n':
                X = 0;
                Y++;
                if (Y == MY)
                    Y = 0;
                scr_clearline(Y);
                break;
            case '\t':
                for (j = 0; j < 5; j++) {
                    scr_putchar(X * 7, Y * 8, fontcolor, ' ');
                    X++;
                }
                break;
            case '\r':
                X = 0;
                // scr_clearline(Y); //Should we clear the line?
                break;
            default:
                scr_putchar(X * 7, Y * 8, fontcolor, c);
                X++;
                if (X == MX) {
                    X = 0;
                    Y++;
                    if (Y == MY)
                        Y = 0;
                    scr_clearline(Y);
                }
        }
    }
    if (cursor)
        scr_putchar(X * 7, Y * 8, cursorcolor, 219);
}

void scr_setXY(int x, int y)
{
    if (x < MX && x >= 0)
        X = x;
    if (y < MY && y >= 0)
        Y = y;
}

int scr_getX()
{
    return X;
}

int scr_getY()
{
    return Y;
}

void scr_clear()
{
    int y;
    for (y = 0; y < MY; y++)
        scr_clearline(y);
    scr_setXY(0, 0);
}

void scr_setCursor(int enable)
{
    cursor = enable;
}

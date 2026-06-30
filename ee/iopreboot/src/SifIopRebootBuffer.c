/*
# _____     ___ ____     ___ ____
#  ____|   |    ____|   |        | |____|
# |     ___|   |____ ___|    ____| |    \    PS2DEV Open Source Project.
#-----------------------------------------------------------------------
# Licenced under Academic Free License version 2.0
# Review ps2sdk README & LICENSE files for further details.
*/

/**
 * @file
 * Special IOP Reboot routines for rebooting with an IOPRP image/encrypted UDNL module in a buffer.
 */

#include "kernel.h"
#include "iopcontrol.h"
#include "iopcontrol_special.h"
#include "loadfile.h"
#include "iopheap.h"
#include "sifdma.h"
#include "sifrpc.h"
#include "string.h"
#include "sbv_patches.h"

#define IOPBTCONF_IOP_MAX_SIZE 0x400

extern int _iop_reboot_count;

extern u8 iopbtconf_img[IOPBTCONF_IOP_MAX_SIZE];

extern int send_romdrv_rom1_payloads(void *payload1, int payload1_size, void *payload2, int payload2_size);

typedef struct romdir
{
    char name[10];
    u16 ExtInfoEntrySize;
    u32 size;
} romdir_t;

typedef struct extinfo
{
    u16 value;    /* Only applicable for the version field type. */
    u8 ExtLength; /* The length of data appended to the end of this entry. */
    u8 type;
} extinfo_t;

enum EXTINFO_TYPE {
    EXTINFO_TYPE_DATE = 1,
    EXTINFO_TYPE_VERSION,
    EXTINFO_TYPE_COMMENT,
    EXTINFO_TYPE_FIXED = 0x7F // Must exist at a fixed location.
};

#ifdef F__iopcontrol_special_internals
u8 iopbtconf_img[IOPBTCONF_IOP_MAX_SIZE] __attribute__((aligned(64)));

struct payload_img
{
    romdir_t romdir[5];
};

static const struct payload_img g_payload_img_base = {
    {{"RESET", 0, 0},
     {"ROMDIR", 0, 0x50},
     {"0", 0, 0},
     {"1", 0, 0},
     {"", 0, 0}},
    };

int send_romdrv_rom1_payloads(void *payload1, int payload1_size, void *payload2, int payload2_size)
{
    unsigned int i;
    unsigned int size_total;
    struct payload_img cur_payload_img __attribute__((__aligned__(64)));
    SifDmaTransfer_t dmat[3];
    int trid;

    cur_payload_img = g_payload_img_base;    
    dmat[0].src        = &cur_payload_img;
    dmat[0].size       = sizeof(cur_payload_img);
    dmat[1].src        = payload1;
    dmat[1].size       = (payload1_size + 15) & -16;
    dmat[2].src        = payload2;
    dmat[2].size       = (payload2_size + 15) & -16;
    for (i = 0; i < (sizeof(dmat)/sizeof(dmat[0])); i += 1)
        dmat[i].attr = 0;
    cur_payload_img.romdir[1].size = dmat[0].size;
    cur_payload_img.romdir[2].size = dmat[1].size;
    cur_payload_img.romdir[3].size = dmat[2].size;
    size_total = 0;
    for (i = 0; i < (sizeof(dmat)/sizeof(dmat[0])); i += 1)
        size_total += dmat[i].size;
    dmat[0].dest = SifAllocIopHeap(size_total);
    if (!dmat[0].dest)
        return -1;
    for (i = 1; i < (sizeof(dmat)/sizeof(dmat[0])); i += 1)
        dmat[i].dest = ((u8 *)dmat[i - 1].dest) + dmat[i - 1].size;
    for (i = 0; i < (sizeof(dmat)/sizeof(dmat[0])); i += 1)
        sceSifWriteBackDCache(dmat[i].src, dmat[i].size);
    while (!(trid = sceSifSetDma(dmat, sizeof(dmat) / sizeof(dmat[0]))));
    while (sceSifDmaStat(trid) >= 0);
    return sbv_patch_romdrv_set_rom1_info(dmat[0].dest, dmat[0].size) ? 1 : 0;
}
#endif

// Our LOADFILE functions are slightly different.
#define SifLoadModuleSpecial(path, arg_len, args, dontwait) \
    _SifLoadModule(path, arg_len, args, NULL, LF_F_MOD_LOAD, dontwait)

#define SifLoadModuleEncryptedSpecial(path, arg_len, args, dontwait) \
    _SifLoadModule(path, arg_len, args, NULL, LF_F_MG_MOD_LOAD, dontwait)

#ifdef F_SifIopRebootBufferEncrypted
int SifIopRebootBufferEncrypted(const void *udnl, int size)
{
    int iopbtconf_img_size;

    sceSifInitRpc(0);
    sceSifExitRpc();

    SifIopReset("", 0);
    while (!SifIopSync());

    iopbtconf_img_size = 0; // No support for IOPBTCONF manipulation

    /* Unofficial: Use following instead of patching LoadModuleBuffer and using embedded imgdrv */
    if (send_romdrv_rom1_payloads((void *)udnl, size, iopbtconf_img, iopbtconf_img_size))
        return -1;

    /* Unofficial: do not load rom:SYSCLIB (DMA end checking is done above) */
    /* If ELF header detected, load UDNL as unencrypted module */
    (void)((((u32 *)udnl)[0] == 0x464c457f) ? SifLoadModuleSpecial("rom1:0", 0, NULL, 1) : SifLoadModuleEncryptedSpecial("rom1:0", 0, NULL, 1));

    sceSifExitRpc();
    SifLoadFileExit();

    sceSifSetReg(SIF_REG_SMFLAG, SIF_STAT_SIFINIT);
    sceSifSetReg(SIF_REG_SMFLAG, SIF_STAT_CMDINIT);
    sceSifSetReg(SIF_REG_SMFLAG, SIF_STAT_BOOTEND);
    sceSifSetReg(SIF_SYSREG_RPCINIT, 0);
    sceSifSetReg(SIF_SYSREG_SUBADDR, 0);

    _iop_reboot_count++; // Not originally here: increment to allow RPC clients to detect unbinding.

    return 1;
}
#endif

#ifdef F_SifIopRebootBuffer
static int generateIOPBTCONF_img(void *output, void *ioprp);

int SifIopRebootBuffer(void *ioprp, int size)
{
    int iopbtconf_img_size;

    sceSifInitRpc(0);
    sceSifExitRpc();

    SifIopReset("", 0);
    while (!SifIopSync());

    iopbtconf_img_size = generateIOPBTCONF_img(iopbtconf_img, ioprp);

    /* Unofficial: Use following instead of patching LoadModuleBuffer and using embedded imgdrv */
    if (send_romdrv_rom1_payloads((void *)ioprp, size, iopbtconf_img, iopbtconf_img_size))
        return -1;

    /* Unofficial: do not load rom:SYSCLIB (DMA end checking is done above) */
    SifLoadModuleSpecial("rom:UDNL", 13, "rom1:0\x00rom1:1", 1);

    sceSifExitRpc();
    SifLoadFileExit();

    sceSifSetReg(SIF_REG_SMFLAG, SIF_STAT_SIFINIT);
    sceSifSetReg(SIF_REG_SMFLAG, SIF_STAT_CMDINIT);
    sceSifSetReg(SIF_REG_SMFLAG, SIF_STAT_BOOTEND);
    sceSifSetReg(SIF_SYSREG_RPCINIT, 0);
    sceSifSetReg(SIF_SYSREG_SUBADDR, 0);

    _iop_reboot_count++; // Not originally here: increment to allow RPC clients to detect unbinding.

    return 1;
}

struct iopbtconf_img
{
    romdir_t romdir[5];
    u32 extinfo[4];
};

static const struct iopbtconf_img iopbtconf_img_base = {
    {{"RESET", 8, 0},
     {"ROMDIR", 0, 0x50},
     {"EXTINFO", 0, 0x10},
     {"IOPBTCONF", 8, -1},
     {"", 0, 0}},
    {
        0x01040000, // EXTINFO {0x0000, 4, EXTINFO_TYPE_DATE},
        0x20010406, // Date: 2001/04/06
        0x01040000, // EXTINFO {0x0000, 4, EXTINFO_TYPE_DATE},
        0x20010406, // Date: 2001/04/06
    }};

/*  Generate an IOPRP image that contains IOPBTCONF, if the original contains IOPBTCONF.
    This is required because UDNL will only seach succeeding IOPRP images for modules specified within IOPBTCONF.    */
static int generateIOPBTCONF_img(void *output, void *ioprp)
{
    int offset, fsize_rounded;
    romdir_t *romdir;
    u8 *ptr_in;
    u8 *ptr_out;

    romdir = (romdir_t *)ioprp;
    if (strcmp(romdir[0].name, "RESET") || strcmp(romdir[1].name, "ROMDIR"))
        return -1;

    // Now locate IOPBTCONF
    for (offset = 0; romdir->name[0] != '\0'; romdir++, offset += fsize_rounded) {
        fsize_rounded = (romdir->size + 15) & ~15;
        /* Unofficial: Check for already-patched image */
        if (!strcmp(romdir->name, "IOPBTCONF") || !strcmp(romdir->name, "XOPBTCONF")) {
            romdir->name[0] = 'X'; // Change 'IOPBTCONF' to 'XOPBTCONF', so that UDNL will not find the original.
            // Copy IOPBTCONF into the new image.
            ptr_out = (u8 *)output;
            ptr_in  = (u8 *)ioprp;
            memcpy(ptr_out, &iopbtconf_img_base, sizeof(iopbtconf_img_base));
            memcpy(ptr_out + sizeof(iopbtconf_img_base), &ptr_in[offset], romdir->size);
            ((romdir_t *)ptr_out)[3].size = romdir->size; // Update the size of IOPBTCONF within the generated IOPRP image.
            return romdir->size + sizeof(iopbtconf_img_base);
        }
    }

    return 0;
}
#endif

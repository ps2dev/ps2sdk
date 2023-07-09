/*
# _____     ___ ____     ___ ____
#  ____|   |    ____|   |        | |____|
# |     ___|   |____ ___|    ____| |    \    PS2DEV Open Source Project.
#-----------------------------------------------------------------------
# Copyright 2001-2009, ps2dev - http://www.ps2dev.org
# Licenced under Academic Free License version 2.0
# Review ps2sdk README & LICENSE files for further details.
*/

#include "irx_imports.h"
#include "dmacman.h"

extern struct irx_export_table _exp_dmacman;

#ifdef _IOP
IRX_ID("dmacman", 1, 1);
#endif
// Based on the module from SCE SDK 1.3.4.

static vu32 *dmac_channel_0_madr  = (vu32 *)0xBF801080;
static vu32 *dmac_channel_0_bcr   = (vu32 *)0xBF801084;
static vu32 *dmac_channel_0_chcr  = (vu32 *)0xBF801088;
static vu32 *dmac_channel_1_madr  = (vu32 *)0xBF801090;
static vu32 *dmac_channel_1_bcr   = (vu32 *)0xBF801094;
static vu32 *dmac_channel_1_chcr  = (vu32 *)0xBF801098;
static vu32 *dmac_channel_2_madr  = (vu32 *)0xBF8010A0;
static vu32 *dmac_channel_2_bcr   = (vu32 *)0xBF8010A4;
static vu32 *dmac_channel_2_chcr  = (vu32 *)0xBF8010A8;
static vu32 *dmac_channel_3_madr  = (vu32 *)0xBF8010B0;
static vu32 *dmac_channel_3_bcr   = (vu32 *)0xBF8010B4;
static vu32 *dmac_channel_3_chcr  = (vu32 *)0xBF8010B8;
static vu32 *dmac_channel_4_madr  = (vu32 *)0xBF8010C0;
static vu32 *dmac_channel_4_bcr   = (vu32 *)0xBF8010C4;
static vu32 *dmac_channel_4_chcr  = (vu32 *)0xBF8010C8;
static vu32 *dmac_channel_4_tadr  = (vu32 *)0xBF8010CC;
static vu32 *dmac_channel_5_madr  = (vu32 *)0xBF8010D0;
static vu32 *dmac_channel_5_bcr   = (vu32 *)0xBF8010D4;
static vu32 *dmac_channel_5_chcr  = (vu32 *)0xBF8010D8;
static vu32 *dmac_channel_6_madr  = (vu32 *)0xBF8010E0;
static vu32 *dmac_channel_6_bcr   = (vu32 *)0xBF8010E4;
static vu32 *dmac_channel_6_chcr  = (vu32 *)0xBF8010E8;
static vu32 *dmac_channel_7_madr  = (vu32 *)0xBF801500;
static vu32 *dmac_channel_7_bcr   = (vu32 *)0xBF801504;
static vu32 *dmac_channel_7_chcr  = (vu32 *)0xBF801508;
static vu32 *dmac_channel_8_madr  = (vu32 *)0xBF801510;
static vu32 *dmac_channel_8_bcr   = (vu32 *)0xBF801514;
static vu32 *dmac_channel_8_chcr  = (vu32 *)0xBF801518;
static vu32 *dmac_channel_9_madr  = (vu32 *)0xBF801520;
static vu32 *dmac_channel_9_bcr   = (vu32 *)0xBF801524;
static vu32 *dmac_channel_9_chcr  = (vu32 *)0xBF801528;
static vu32 *dmac_channel_9_tadr  = (vu32 *)0xBF80152C;
static vu32 *dmac_channel_A_madr  = (vu32 *)0xBF801530;
static vu32 *dmac_channel_A_bcr   = (vu32 *)0xBF801534;
static vu32 *dmac_channel_A_chcr  = (vu32 *)0xBF801538;
static vu32 *dmac_channel_B_madr  = (vu32 *)0xBF801540;
static vu32 *dmac_channel_B_bcr   = (vu32 *)0xBF801544;
static vu32 *dmac_channel_B_chcr  = (vu32 *)0xBF801548;
static vu32 *dmac_channel_C_madr  = (vu32 *)0xBF801550;
static vu32 *dmac_channel_C_bcr   = (vu32 *)0xBF801554;
static vu32 *dmac_channel_C_chcr  = (vu32 *)0xBF801558;
static vu32 *dmac_channel_9_4_9_a = (vu32 *)0xBF801560;
static vu32 *dmac_channel_A_4_9_a = (vu32 *)0xBF801564;
static vu32 *dmac_channel_4_4_9_a = (vu32 *)0xBF801568;
static vu32 *dmac_dpcr            = (vu32 *)0xBF8010F0;
static vu32 *dmac_dpcr2           = (vu32 *)0xBF801570;
static vu32 *dmac_dpcr3           = (vu32 *)0xBF8015F0;
static vu32 *dmac_dicr            = (vu32 *)0xBF8010F4;
static vu32 *dmac_dicr2           = (vu32 *)0xBF801574;
static vu32 *dmac_BF80157C        = (vu32 *)0xBF80157C;
static vu32 *dmac_BF801578        = (vu32 *)0xBF801578;

void dmac_ch_set_madr(u32 channel, u32 val)
{
    switch (channel) {
        case IOP_DMAC_MDECin:
            *dmac_channel_0_madr = val;
            break;
        case IOP_DMAC_MDECout:
            *dmac_channel_1_madr = val;
            break;
        case IOP_DMAC_SIF2:
            *dmac_channel_2_madr = val;
            break;
        case IOP_DMAC_CDVD:
            *dmac_channel_3_madr = val;
            break;
        case IOP_DMAC_SPU:
            *dmac_channel_4_madr = val;
            break;
        case IOP_DMAC_PIO:
            *dmac_channel_5_madr = val;
            break;
        case IOP_DMAC_OTC:
            *dmac_channel_6_madr = val;
            break;
        case IOP_DMAC_SPU2:
            *dmac_channel_7_madr = val;
            break;
        case IOP_DMAC_DEV9:
            *dmac_channel_8_madr = val;
            break;
        case IOP_DMAC_SIF0:
            *dmac_channel_9_madr = val;
            break;
        case IOP_DMAC_SIF1:
            *dmac_channel_A_madr = val;
            break;
        case IOP_DMAC_SIO2in:
            *dmac_channel_B_madr = val;
            break;
        case IOP_DMAC_SIO2out:
            *dmac_channel_C_madr = val;
            break;
        default:
            break;
    }
}

u32 dmac_ch_get_madr(u32 channel)
{
    switch (channel) {
        case IOP_DMAC_MDECin:
            return *dmac_channel_0_madr;
        case IOP_DMAC_MDECout:
            return *dmac_channel_1_madr;
        case IOP_DMAC_SIF2:
            return *dmac_channel_2_madr;
        case IOP_DMAC_CDVD:
            return *dmac_channel_3_madr;
        case IOP_DMAC_SPU:
            return *dmac_channel_4_madr;
        case IOP_DMAC_PIO:
            return *dmac_channel_5_madr;
        case IOP_DMAC_OTC:
            return *dmac_channel_6_madr;
        case IOP_DMAC_SPU2:
            return *dmac_channel_7_madr;
        case IOP_DMAC_DEV9:
            return *dmac_channel_8_madr;
        case IOP_DMAC_SIF0:
            return *dmac_channel_9_madr;
        case IOP_DMAC_SIF1:
            return *dmac_channel_A_madr;
        case IOP_DMAC_SIO2in:
            return *dmac_channel_B_madr;
        case IOP_DMAC_SIO2out:
            return *dmac_channel_C_madr;
        default:
            return 0;
    }
}

void dmac_ch_set_bcr(u32 channel, u32 val)
{
    switch (channel) {
        case IOP_DMAC_MDECin:
            *dmac_channel_0_bcr = val;
            break;
        case IOP_DMAC_MDECout:
            *dmac_channel_1_bcr = val;
            break;
        case IOP_DMAC_SIF2:
            *dmac_channel_2_bcr = val;
            break;
        case IOP_DMAC_CDVD:
            *dmac_channel_3_bcr = val;
            break;
        case IOP_DMAC_SPU:
            *dmac_channel_4_bcr = val;
            break;
        case IOP_DMAC_PIO:
            *dmac_channel_5_bcr = val;
            break;
        case IOP_DMAC_OTC:
            *dmac_channel_6_bcr = val;
            break;
        case IOP_DMAC_SPU2:
            *dmac_channel_7_bcr = val;
            break;
        case IOP_DMAC_DEV9:
            *dmac_channel_8_bcr = val;
            break;
        case IOP_DMAC_SIF0:
            *dmac_channel_9_bcr = val;
            break;
        case IOP_DMAC_SIF1:
            *dmac_channel_A_bcr = val;
            break;
        case IOP_DMAC_SIO2in:
            *dmac_channel_B_bcr = val;
            break;
        case IOP_DMAC_SIO2out:
            *dmac_channel_C_bcr = val;
            break;
        default:
            break;
    }
}

u32 dmac_ch_get_bcr(u32 channel)
{
    switch (channel) {
        case IOP_DMAC_MDECin:
            return *dmac_channel_0_bcr;
        case IOP_DMAC_MDECout:
            return *dmac_channel_1_bcr;
        case IOP_DMAC_SIF2:
            return *dmac_channel_2_bcr;
        case IOP_DMAC_CDVD:
            return *dmac_channel_3_bcr;
        case IOP_DMAC_SPU:
            return *dmac_channel_4_bcr;
        case IOP_DMAC_PIO:
            return *dmac_channel_5_bcr;
        case IOP_DMAC_OTC:
            return *dmac_channel_6_bcr;
        case IOP_DMAC_SPU2:
            return *dmac_channel_7_bcr;
        case IOP_DMAC_DEV9:
            return *dmac_channel_8_bcr;
        case IOP_DMAC_SIF0:
            return *dmac_channel_9_bcr;
        case IOP_DMAC_SIF1:
            return *dmac_channel_A_bcr;
        case IOP_DMAC_SIO2in:
            return *dmac_channel_B_bcr;
        case IOP_DMAC_SIO2out:
            return *dmac_channel_C_bcr;
        default:
            return 0;
    }
}

void dmac_ch_set_chcr(u32 channel, u32 val)
{
    switch (channel) {
        case IOP_DMAC_MDECin:
            *dmac_channel_0_chcr = val;
            break;
        case IOP_DMAC_MDECout:
            *dmac_channel_1_chcr = val;
            break;
        case IOP_DMAC_SIF2:
            *dmac_channel_2_chcr = val;
            break;
        case IOP_DMAC_CDVD:
            *dmac_channel_3_chcr = val;
            break;
        case IOP_DMAC_SPU:
            *dmac_channel_4_chcr = val;
            break;
        case IOP_DMAC_PIO:
            *dmac_channel_5_chcr = val;
            break;
        case IOP_DMAC_OTC:
            *dmac_channel_6_chcr = val;
            break;
        case IOP_DMAC_SPU2:
            *dmac_channel_7_chcr = val;
            break;
        case IOP_DMAC_DEV9:
            *dmac_channel_8_chcr = val;
            break;
        case IOP_DMAC_SIF0:
            *dmac_channel_9_chcr = val;
            break;
        case IOP_DMAC_SIF1:
            *dmac_channel_A_chcr = val;
            break;
        case IOP_DMAC_SIO2in:
            *dmac_channel_B_chcr = val;
            break;
        case IOP_DMAC_SIO2out:
            *dmac_channel_C_chcr = val;
            break;
        default:
            break;
    }
}

u32 dmac_ch_get_chcr(u32 channel)
{
    switch (channel) {
        case IOP_DMAC_MDECin:
            return *dmac_channel_0_chcr;
        case IOP_DMAC_MDECout:
            return *dmac_channel_1_chcr;
        case IOP_DMAC_SIF2:
            return *dmac_channel_2_chcr;
        case IOP_DMAC_CDVD:
            return *dmac_channel_3_chcr;
        case IOP_DMAC_SPU:
            return *dmac_channel_4_chcr;
        case IOP_DMAC_PIO:
            return *dmac_channel_5_chcr;
        case IOP_DMAC_OTC:
            return *dmac_channel_6_chcr;
        case IOP_DMAC_SPU2:
            return *dmac_channel_7_chcr;
        case IOP_DMAC_DEV9:
            return *dmac_channel_8_chcr;
        case IOP_DMAC_SIF0:
            return *dmac_channel_9_chcr;
        case IOP_DMAC_SIF1:
            return *dmac_channel_A_chcr;
        case IOP_DMAC_SIO2in:
            return *dmac_channel_B_chcr;
        case IOP_DMAC_SIO2out:
            return *dmac_channel_C_chcr;
        default:
            return 0;
    }
}

void dmac_ch_set_tadr(u32 channel, u32 val)
{
    switch (channel) {
        case IOP_DMAC_SPU:
            *dmac_channel_4_tadr = val;
            break;
        case IOP_DMAC_SIF0:
            *dmac_channel_9_tadr = val;
            break;
        default:
            break;
    }
}

u32 dmac_ch_get_tadr(u32 channel)
{
    switch (channel) {
        case IOP_DMAC_SPU:
            return *dmac_channel_4_tadr;
        case IOP_DMAC_SIF0:
            return *dmac_channel_9_tadr;
        default:
            return 0;
    }
}

void dmac_set_4_9_a(u32 channel, u32 val)
{
    switch (channel) {
        case IOP_DMAC_SPU:
            *dmac_channel_4_4_9_a = val;
            break;
        case IOP_DMAC_SIF0:
            *dmac_channel_9_4_9_a = val;
            break;
        case IOP_DMAC_SIF1:
            *dmac_channel_A_4_9_a = val;
            break;
        default:
            break;
    }
}

u32 dmac_get_4_9_a(u32 channel)
{
    switch (channel) {
        case IOP_DMAC_SPU:
            return *dmac_channel_4_4_9_a;
        case IOP_DMAC_SIF0:
            return *dmac_channel_9_4_9_a;
        case IOP_DMAC_SIF1:
            return *dmac_channel_A_4_9_a;
        default:
            return 0;
    }
}

void dmac_set_dpcr(u32 val)
{
    *dmac_dpcr = val;
}

u32 dmac_get_dpcr(void)
{
    return *dmac_dpcr;
}

void dmac_set_dpcr2(u32 val)
{
    *dmac_dpcr2 = val;
}

u32 dmac_get_dpcr2(void)
{
    return *dmac_dpcr2;
}

void dmac_set_dpcr3(u32 val)
{
    *dmac_dpcr3 = val;
}

u32 dmac_get_dpcr3(void)
{
    return *dmac_dpcr3;
}

void dmac_set_dicr(u32 val)
{
    *dmac_dicr = val;
}

u32 dmac_get_dicr(void)
{
    return *dmac_dicr;
}

void dmac_set_dicr2(u32 val)
{
    *dmac_dicr2 = val;
}

u32 dmac_get_dicr2(void)
{
    return *dmac_dicr2;
}

void dmac_set_BF80157C(u32 val)
{
    *dmac_BF80157C = val;
}

u32 dmac_get_BF80157C(void)
{
    return *dmac_BF80157C;
}

void dmac_set_BF801578(u32 val)
{
    *dmac_BF801578 = val;
}

u32 dmac_get_BF801578(void)
{
    return *dmac_BF801578;
}

int _start(int argc, char *argv[])
{
    int state;

    (void)argc;
    (void)argv;

    if (RegisterLibraryEntries(&_exp_dmacman) != 0) {
        return 1;
    }
    CpuSuspendIntr(&state);
    dmac_set_dpcr(0x7777777);
    dmac_set_dpcr2(0x7777777);
    dmac_set_dpcr3(0x777);
    {
        int i;
        for (i = 0; i < 0xD; i += 1) {
            dmac_ch_set_madr(i, 0);
            dmac_ch_set_bcr(i, 0);
            dmac_ch_set_chcr(i, 0);
        }
    }

    dmac_ch_set_tadr(IOP_DMAC_SPU, 0);
    dmac_ch_set_tadr(IOP_DMAC_SIF0, 0);
    dmac_set_4_9_a(IOP_DMAC_SPU, 0);
    dmac_set_4_9_a(IOP_DMAC_SIF0, 0);
    dmac_set_4_9_a(IOP_DMAC_SIF1, 0);
    dmac_set_BF801578(1);
    CpuResumeIntr(state);
    return 0;
}

int dmacman_deinit()
{
    int state;

    CpuSuspendIntr(&state);
    dmac_set_BF801578(0);
    {
        int i;
        for (i = 0; i < 0xD; i += 1) {
            u32 value;

            value = dmac_ch_get_chcr(i);
            if ((value & 0x1000000) != 0) {
                Kprintf("WARNING:DMA %dch has been continued until shutdown\n", i);
            }
            dmac_ch_set_chcr(i, value & (~0x1000000));
        }
    }

    CpuResumeIntr(state);
    return 1;
}

int dmac_request(u32 channel, void *addr, u32 size, u32 count, int dir)
{
    if (channel >= 0xD || channel == IOP_DMAC_OTC) {
        return 0;
    }
    dmac_ch_set_madr(channel, (unsigned int)addr & 0xFFFFFF);
    dmac_ch_set_bcr(channel, (size & 0xFFFF) | (count << 16));
    dmac_ch_set_chcr(channel, (dir & 1) | 0x200 | ((dir == 0) ? 0x40000000 : 0));
    return 1;
}

int dmac_set_dma_chained_spu_sif0(u32 channel, u32 size, u32 tadr)
{
    if (channel == IOP_DMAC_SPU || channel == IOP_DMAC_SIF0) {
        dmac_ch_set_bcr(channel, size & 0xFFFF);
        dmac_ch_set_chcr(channel, 0x601);
        dmac_ch_set_tadr(channel, tadr & 0xFFFFFF);
        return 1;
    }
    return 0;
}

int dmac_set_dma_sif0(u32 channel, u32 size, u32 tadr)
{
    if (channel != IOP_DMAC_SIF0) {
        return 0;
    }
    dmac_ch_set_bcr(IOP_DMAC_SIF0, size & 0xFFFF);
    dmac_ch_set_chcr(IOP_DMAC_SIF0, 0x701);
    dmac_ch_set_tadr(IOP_DMAC_SIF0, tadr & 0xFFFFFF);
    return 1;
}

int dmac_set_dma_sif1(u32 ch, u32 size)
{
    if (ch != IOP_DMAC_SIF1) {
        return 0;
    }
    dmac_ch_set_bcr(IOP_DMAC_SIF1, size & 0xFFFF);
    dmac_ch_set_chcr(IOP_DMAC_SIF1, 0x40000300);
    return 1;
}

void dmac_transfer(u32 channel)
{
    if (channel < 0xF) {
        dmac_ch_set_chcr(channel, dmac_ch_get_chcr(channel) | 0x1000000);
    }
}

void dmac_ch_set_dpcr(u32 channel, u32 val)
{
    int state;

    CpuSuspendIntr(&state);
    switch (channel) {
        case IOP_DMAC_MDECin:
            dmac_set_dpcr((dmac_get_dpcr() & (~0x7)) | (val & 7));
            break;
        case IOP_DMAC_MDECout:
            dmac_set_dpcr((dmac_get_dpcr() & (~0x70)) | ((val & 7) << 4));
            break;
        case IOP_DMAC_SIF2:
            dmac_set_dpcr((dmac_get_dpcr() & (~0x700)) | ((val & 7) << 8));
            break;
        case IOP_DMAC_CDVD:
            dmac_set_dpcr((dmac_get_dpcr() & (~0x7000)) | ((val & 7) << 12));
            break;
        case IOP_DMAC_SPU:
            dmac_set_dpcr((dmac_get_dpcr() & (~0x70000)) | ((val & 7) << 16));
            break;
        case IOP_DMAC_PIO:
            dmac_set_dpcr((dmac_get_dpcr() & (~0x700000)) | ((val & 7) << 20));
            break;
        case IOP_DMAC_OTC:
            dmac_set_dpcr((dmac_get_dpcr() & (~0x7000000)) | ((val & 7) << 24));
            break;
        case IOP_DMAC_SPU2:
            dmac_set_dpcr2((dmac_get_dpcr2() & (~0x7)) | (val & 7));
            break;
        case IOP_DMAC_DEV9:
            dmac_set_dpcr2((dmac_get_dpcr2() & (~0x70)) | ((val & 7) << 4));
            break;
        case IOP_DMAC_SIF0:
            dmac_set_dpcr2((dmac_get_dpcr2() & (~0x700)) | ((val & 7) << 8));
            break;
        case IOP_DMAC_SIF1:
            dmac_set_dpcr2((dmac_get_dpcr2() & (~0x7000)) | ((val & 7) << 12));
            break;
        case IOP_DMAC_SIO2in:
            dmac_set_dpcr2((dmac_get_dpcr2() & (~0x70000)) | ((val & 7) << 16));
            break;
        case IOP_DMAC_SIO2out:
            dmac_set_dpcr2((dmac_get_dpcr2() & (~0x700000)) | ((val & 7) << 20));
            break;
        case IOP_DMAC_FDMA0:
            dmac_set_dpcr3((dmac_get_dpcr3() & (~0x7)) | (val & 7));
            break;
        case IOP_DMAC_FDMA1:
            dmac_set_dpcr3((dmac_get_dpcr3() & (~0x70)) | ((val & 7) << 4));
            break;
        case IOP_DMAC_FDMA2:
            dmac_set_dpcr3((dmac_get_dpcr3() & (~0x700)) | ((val & 7) << 8));
            break;
        case IOP_DMAC_CPU:
            dmac_set_dpcr((dmac_get_dpcr() & (~0x70000000)) | ((val & 7) << 28));
            break;
        case IOP_DMAC_USB:
            dmac_set_dpcr2((dmac_get_dpcr2() & (~0x7000000)) | ((val & 7) << 24));
            break;
        default:
            break;
    }
    CpuResumeIntr(state);
}

void dmac_enable(u32 channel)
{
    int state;

    CpuSuspendIntr(&state);
    switch (channel) {
        case IOP_DMAC_MDECin:
            dmac_set_dpcr(dmac_get_dpcr() | 0x8);
            break;
        case IOP_DMAC_MDECout:
            dmac_set_dpcr(dmac_get_dpcr() | 0x80);
            break;
        case IOP_DMAC_SIF2:
            dmac_set_dpcr(dmac_get_dpcr() | 0x800);
            break;
        case IOP_DMAC_CDVD:
            dmac_set_dpcr(dmac_get_dpcr() | 0x8000);
            break;
        case IOP_DMAC_SPU:
            dmac_set_dpcr(dmac_get_dpcr() | 0x80000);
            break;
        case IOP_DMAC_PIO:
            dmac_set_dpcr(dmac_get_dpcr() | 0x800000);
            break;
        case IOP_DMAC_OTC:
            dmac_set_dpcr(dmac_get_dpcr() | 0x8000000);
            break;
        case IOP_DMAC_SPU2:
            dmac_set_dpcr2(dmac_get_dpcr2() | 0x8);
            break;
        case IOP_DMAC_DEV9:
            dmac_set_dpcr2(dmac_get_dpcr2() | 0x80);
            break;
        case IOP_DMAC_SIF0:
            dmac_set_dpcr2(dmac_get_dpcr2() | 0x800);
            break;
        case IOP_DMAC_SIF1:
            dmac_set_dpcr2(dmac_get_dpcr2() | 0x8000);
            break;
        case IOP_DMAC_SIO2in:
            dmac_set_dpcr2(dmac_get_dpcr2() | 0x80000);
            break;
        case IOP_DMAC_SIO2out:
            dmac_set_dpcr2(dmac_get_dpcr2() | 0x800000);
            break;
        case IOP_DMAC_FDMA0:
            dmac_set_dpcr3(dmac_get_dpcr3() | 0x8);
            break;
        case IOP_DMAC_FDMA1:
            dmac_set_dpcr3(dmac_get_dpcr3() | 0x80);
            break;
        case IOP_DMAC_FDMA2:
            dmac_set_dpcr3(dmac_get_dpcr3() | 0x800);
            break;
        case IOP_DMAC_USB:
            dmac_set_dpcr2(dmac_get_dpcr2() | 0x8000000);
            break;
        default:
            break;
    }
    CpuResumeIntr(state);
}

void dmac_disable(u32 channel)
{
    int state;

    CpuSuspendIntr(&state);
    switch (channel) {
        case IOP_DMAC_MDECin:
            dmac_set_dpcr(dmac_get_dpcr() & (~0x8));
            break;
        case IOP_DMAC_MDECout:
            dmac_set_dpcr(dmac_get_dpcr() & (~0x80));
            break;
        case IOP_DMAC_SIF2:
            dmac_set_dpcr(dmac_get_dpcr() & (~0x800));
            break;
        case IOP_DMAC_CDVD:
            dmac_set_dpcr(dmac_get_dpcr() & (~0x8000));
            break;
        case IOP_DMAC_SPU:
            dmac_set_dpcr(dmac_get_dpcr() & (~0x80000));
            break;
        case IOP_DMAC_PIO:
            dmac_set_dpcr(dmac_get_dpcr() & (~0x800000));
            break;
        case IOP_DMAC_OTC:
            dmac_set_dpcr(dmac_get_dpcr() & (~0x8000000));
            break;
        case IOP_DMAC_SPU2:
            dmac_set_dpcr2(dmac_get_dpcr2() & (~0x8));
            break;
        case IOP_DMAC_DEV9:
            dmac_set_dpcr2(dmac_get_dpcr2() & (~0x80));
            break;
        case IOP_DMAC_SIF0:
            dmac_set_dpcr2(dmac_get_dpcr2() & (~0x800));
            break;
        case IOP_DMAC_SIF1:
            dmac_set_dpcr2(dmac_get_dpcr2() & (~0x8000));
            break;
        case IOP_DMAC_SIO2in:
            dmac_set_dpcr2(dmac_get_dpcr2() & (~0x80000));
            break;
        case IOP_DMAC_SIO2out:
            dmac_set_dpcr2(dmac_get_dpcr2() & (~0x800000));
            break;
        case IOP_DMAC_FDMA0:
            dmac_set_dpcr3(dmac_get_dpcr3() & (~0x8));
            break;
        case IOP_DMAC_FDMA1:
            dmac_set_dpcr3(dmac_get_dpcr3() & (~0x80));
            break;
        case IOP_DMAC_FDMA2:
            dmac_set_dpcr3(dmac_get_dpcr3() & (~0x800));
            break;
        case IOP_DMAC_USB:
            dmac_set_dpcr2(dmac_get_dpcr2() & (~0x8000000));
            break;
        default:
            break;
    }
    CpuResumeIntr(state);
}

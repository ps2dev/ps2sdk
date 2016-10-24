/*
# _____     ___ ____     ___ ____
#  ____|   |    ____|   |        | |____|
# |     ___|   |____ ___|    ____| |    \    PS2DEV Open Source Project.
#-----------------------------------------------------------------------
# Copyright 2001-2004, ps2dev - http://www.ps2dev.org
# Licenced under Academic Free License version 2.0
# Review ps2sdk README & LICENSE files for further details.
#
# $Id$
# EE SIF control function prototypes and structures
*/

#ifndef _SIFDMA_H
#define _SIFDMA_H

#include <tamtypes.h>

#ifdef __cplusplus
extern "C" {
#endif

#define SIF_DMA_INT_I 0x2
#define SIF_DMA_INT_O 0x4

#define SIF_DMA_ERT 0x40

#define SIF_REG_ID_SYSTEM 0x80000000

enum _sif_regs {
    SIF_REG_MAINADDR = 1,  //Main -> sub-CPU command buffer (MSCOM)
    SIF_REG_SUBADDR,       //Sub -> main-CPU command buffer (SMCOM)
    SIF_REG_MSFLAG,        //Main -> sub-CPU flag (MSFLAG)
    SIF_REG_SMFLAG,        //Sub -> main-CPU flag (SMFLAG)

    //Used with the EE kernel. Not actually physical registers like the above, but are implemented in software.
    SIF_SYSREG_SUBADDR = SIF_REG_ID_SYSTEM | 0,
    SIF_SYSREG_MAINADDR,
    SIF_SYSREG_RPCINIT,
};

//Status bits for the SM and MS SIF registers
#define SIF_STAT_SIFINIT 0x10000  //SIF initialized
#define SIF_STAT_CMDINIT 0x20000  //SIFCMD initialized
#define SIF_STAT_BOOTEND 0x40000  //Bootup completed

typedef struct t_SifDmaTransfer
{
    void *src,
        *dest;
    int size;
    int attr;
} SifDmaTransfer_t;

u32 SifSetDma(SifDmaTransfer_t *sdd, s32 len);
s32 SifDmaStat(u32 id);

#ifdef __cplusplus
}
#endif

#endif

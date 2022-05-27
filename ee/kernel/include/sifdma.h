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
 * EE SIF control function prototypes and structures
 */

#ifndef __SIFDMA_H__
#define __SIFDMA_H__

#include <tamtypes.h>

#define SIF_DMA_INT_I 0x2
#define SIF_DMA_INT_O 0x4

#define SIF_DMA_ERT 0x40

#define SIF_REG_ID_SYSTEM 0x80000000

enum _sif_regs {
    /** Main -> sub-CPU command buffer (MSCOM) */
    SIF_REG_MAINADDR = 1,
    /** Sub -> main-CPU command buffer (SMCOM) */
    SIF_REG_SUBADDR,
    /** Main -> sub-CPU flag (MSFLAG) */
    SIF_REG_MSFLAG,
    /** Sub -> main-CPU flag (SMFLAG) */
    SIF_REG_SMFLAG,

    // Used with the EE kernel. Not actually physical registers like the above, but are implemented in software.
    SIF_SYSREG_SUBADDR = SIF_REG_ID_SYSTEM | 0,
    SIF_SYSREG_MAINADDR,
    SIF_SYSREG_RPCINIT,
};

// Status bits for the SM and MS SIF registers
/** SIF initialized */
#define SIF_STAT_SIFINIT 0x10000
/** SIFCMD initialized */
#define SIF_STAT_CMDINIT 0x20000
/** Bootup completed */
#define SIF_STAT_BOOTEND 0x40000

typedef struct t_SifDmaTransfer
{
    void *src,
        *dest;
    int size;
    int attr;
} SifDmaTransfer_t;

#ifdef __cplusplus
extern "C" {
#endif

u32 SifSetDma(SifDmaTransfer_t *sdd, s32 len);
s32 SifDmaStat(u32 id);

#ifdef __cplusplus
}
#endif

#endif /* __SIFDMA_H__ */

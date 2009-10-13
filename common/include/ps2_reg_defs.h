#ifndef _PS2_REG_DEFS_H
#define _PS2_REG_DEFS_H

#include "tamtypes.h"
#include "ee_regs.h"
#include "iop_regs.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef volatile struct st_PS2_SBUS_Registers
{
    u32 main_addr;
    u32 pad0[3];
    u32 sub_addr;
    u32 pad1[3];
    u32 ms_flag;
    u32 pad2[3];
    u32 sm_flag;
    u32 pad3[3];

    u32 reg_40;
    u32 pad4[3];
    u32 reg_50;
    u32 pad5[3];
    u32 reg_60;
    u32 pad6[3];
    u32 reg_70;
    u32 pad7[3];
} PS2_SBUS_Registers;

// "modes" for SIF transfers.
#define SIF_XFER_MODE_IN (0 << 0)
#define SIF_XFER_MODE_OUT (1 << 0)

// IRQ bits for "PS2_IRQ" register.
#define PS2_IRQ_UNK0 (0)
#define PS2_IRQ_SBUS (1)
#define PS2_IRQ_UNK2 (2)
#define PS2_IRQ_UNK3 (3)
#define PS2_IRQ_UNK4 (4)
#define PS2_IRQ_SIF0 (5)
#define PS2_IRQ_SIF1 (6)
#define PS2_IRQ_SIF2 (7)
#define PS2_IRQ_UNK8 (8)
#define PS2_IRQ_UNK9 (9)
#define PS2_IRQ_UNK10 (10)

// PS1 GPU interrupt, IOP->EE
#define SBUS_CTRL_PGPU_INT (1 << 0)

// I suspect this is bit 8
#define SBUS_CTRL_MSCLK (1 << 8)

// right? bit 18 is set to interrupt IOP from EE.  there should be a corresponding "SMINT" for IOP...
#define SBUS_CTRL_MSINT  (1 << 18)

// PS1 Mode Reset, EE->IOP.  Resets IOP into "PS1 Mode" though EE needs to do some initializing first.
#define SBUS_CTRL_PS1_RESET (1 << 19)

#define R_PS2_SBUS(__base_addr, __reg_no) ((vu32 *) ((u32) (__base_addr) + (__reg_no * 0x10)))

#define R_EE_SBUS(__reg_no) R_PS2_SBUS(A_EE_SBUS_REG_BASE, (__reg_no))
#define R_IOP_SBUS(__reg_no) R_PS2_SBUS(A_IOP_SBUS_REG_BASE, (__reg_no))

#define PS2_SBUS_MS_ADDR    (0)
#define PS2_SBUS_SM_ADDR    (1)
#define PS2_SBUS_MS_FLAG    (2)
#define PS2_SBUS_SM_FLAG    (3)
#define PS2_SBUS_REG4       (4)
#define PS2_SBUS_REG5       (5)
#define PS2_SBUS_REG6       (6)
#define PS2_SBUS_REG7       (7)

// DMA related
#define PS2_DMA_TO_MEM (0)
#define PS2_DMA_FROM_MEM (1)

// SIF..
#define SIF_FLAG_INIT (1 << 16)

#ifdef __cplusplus
}
#endif

#endif // #ifndef _PS2_REG_DEFS_H



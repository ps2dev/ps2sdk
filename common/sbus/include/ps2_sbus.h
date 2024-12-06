/**
 * @file
 * SBUS definitions.
 */

#ifndef __PS2_SBUS_H__
#define __PS2_SBUS_H__

#include <tamtypes.h>
#include <ps2_reg_defs.h>

#ifdef _IOP
#include <irx.h>

#define sbus_IMPORTS_start DECLARE_IMPORT_TABLE(sbus, 1, 1)
#define sbus_IMPORTS_end   END_IMPORT_TABLE
#endif

typedef void (*SBUS_IrqHandlerFunc)(int irq, void *param);

typedef struct st_SBUS_IrqHandler
{
    SBUS_IrqHandlerFunc func;
    void *param;
} SBUS_IrqHandler;

typedef struct st_SIF2_CmdPkt
{
    /** 32-bit command id */
    u32 cid;
    /** 32-bit size of command packet. */
    u32 size;
    /** 32-bit remote address of parameters */
    u32 extra;
    /** 32-bit size of remote parameters */
    u32 extra_size;
} SIF2_CmdPkt;

typedef void (*SIF2_CmdHandlerFunc)(SIF2_CmdPkt *pkt, void *param);

typedef struct st_SIF2_CmdHandler
{
    SIF2_CmdHandlerFunc func;
    void *param;
} SIF2_CmdHandler;

// SBUS
extern int SBUS_init(void);
extern int SBUS_deinit(void);
extern void *SBUS_set_irq_handler(int irq, SBUS_IrqHandlerFunc func, void *param);
extern int SBUS_rem_irq_handler(int irq);
extern u32 SBUS_get_reg(int reg_no);
extern void SBUS_set_reg(int reg_no, u32 val);
extern int SBUS_interrupt_remote(int irq);

// SIF2
extern int SIF2_init(void);
extern int SIF2_deinit(void);
extern int SIF2_set_dma(u32 addr, u32 size, u32 attr);
extern void SIF2_sync_dma(void);

// SIF2 Command
extern int SIF2_init_cmd(void);
extern int SIF2_set_cmd_handler(int cid, SIF2_CmdHandlerFunc func, void *param);
extern int SIF2_rem_cmd_handler(int cid);
extern void SIF2_send_cmd(u32 cid, void *extra, int extra_size);
extern void SBUS_check_intr(void);

// SBUS
#define I_SBUS_init             DECLARE_IMPORT(4, SBUS_init)
#define I_SBUS_deinit           DECLARE_IMPORT(5, SBUS_deinit)
#define I_SBUS_set_irq_handler  DECLARE_IMPORT(6, SBUS_set_irq_handler)
#define I_SBUS_rem_irq_handler  DECLARE_IMPORT(7, SBUS_rem_irq_handler)
#define I_SBUS_get_reg          DECLARE_IMPORT(8, SBUS_get_reg)
#define I_SBUS_set_reg          DECLARE_IMPORT(9, SBUS_set_reg)
#define I_SBUS_interrupt_remote DECLARE_IMPORT(10, SBUS_interrupt_remote)
#define I_SBUS_check_intr       DECLARE_IMPORT(11, SBUS_check_intr)

// SIF2 DMA
#define I_SIF2_init     DECLARE_IMPORT(12, SIF2_init)
#define I_SIF2_deinit   DECLARE_IMPORT(13, SIF2_deinit)
#define I_SIF2_set_dma  DECLARE_IMPORT(14, SIF2_set_dma)
#define I_SIF2_sync_dma DECLARE_IMPORT(15, SIF2_sync_dma)

// SIF2 CMD
#define I_SIF2_init_cmd        DECLARE_IMPORT(16, SIF2_init_cmd)
#define I_SIF2_set_cmd_handler DECLARE_IMPORT(17, SIF2_set_cmd_handler)
#define I_SIF2_rem_cmd_handler DECLARE_IMPORT(18, SIF2_rem_cmd_handler)
#define I_SIF2_send_cmd        DECLARE_IMPORT(19, SIF2_send_cmd)

#endif /* __PS2_SBUS_H__ */

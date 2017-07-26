/**
 * @file
 * IOP SBUS debug library.
 */

#ifndef __IOP_SBUSDBG_H__
#define __IOP_SBUSDBG_H__

#include <types.h>
#include <ps2_debug.h>

int iop_dbg_install(void);
int iop_dbg_remove(void);

u32 iop_dbg_get_dcic(void);
u32 iop_dbg_get_bpc(void);
u32 iop_dbg_get_bpcm(void);
u32 iop_dbg_get_bda(void);
u32 iop_dbg_get_bdam(void);

void iop_dbg_set_dcic(u32 v);
void iop_dbg_set_bpc(u32 v);
void iop_dbg_set_bpcm(u32 v);
void iop_dbg_set_bda(u32 v);
void iop_dbg_set_bdam(u32 v);

void iop_dbg_set_bpr(u32 addr, u32 mask, u32 user_mask);
void iop_dbg_set_bpw(u32 addr, u32 mask, u32 user_mask);
void iop_dbg_set_bpx(u32 addr, u32 mask, u32 user_mask);

void iop_dbg_clear(void);
void iop_dbg_clr_bpda(void);
void iop_dbg_clr_bpx(void);

void tty_puts(const char *str);
void signal_iop_exception(IOP_RegFrame *frame);

#endif /* __IOP_SBUSDBG_H__ */

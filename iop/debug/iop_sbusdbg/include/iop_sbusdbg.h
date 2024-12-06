/**
 * @file
 * IOP SBUS debug library.
 */

#ifndef __IOP_SBUSDBG_H__
#define __IOP_SBUSDBG_H__

#include <types.h>
#include <ps2_debug.h>

extern int iop_dbg_install(void);
extern int iop_dbg_remove(void);

extern u32 iop_dbg_get_dcic(void);
extern u32 iop_dbg_get_bpc(void);
extern u32 iop_dbg_get_bpcm(void);
extern u32 iop_dbg_get_bda(void);
extern u32 iop_dbg_get_bdam(void);

extern void iop_dbg_set_dcic(u32 v);
extern void iop_dbg_set_bpc(u32 v);
extern void iop_dbg_set_bpcm(u32 v);
extern void iop_dbg_set_bda(u32 v);
extern void iop_dbg_set_bdam(u32 v);

extern void iop_dbg_set_bpr(u32 addr, u32 mask, u32 user_mask);
extern void iop_dbg_set_bpw(u32 addr, u32 mask, u32 user_mask);
extern void iop_dbg_set_bpx(u32 addr, u32 mask, u32 user_mask);

extern void iop_dbg_clear(void);
extern void iop_dbg_clr_bpda(void);
extern void iop_dbg_clr_bpx(void);

extern void tty_puts(const char *str);
extern void signal_iop_exception(IOP_RegFrame *frame);

#endif /* __IOP_SBUSDBG_H__ */

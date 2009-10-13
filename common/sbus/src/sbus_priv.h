/*

definitions for use inside the PS2 sbus project.

*/

#ifndef __SBUS_PRIV
#define __SBUS_PRIV

#include <tamtypes.h>
#include <ps2_reg_defs.h>

#ifdef _EE
#define R_LOCAL_SBUS(__reg_no) R_EE_SBUS(__reg_no)
#define PS2_SBUS_LR_FLAG PS2_SBUS_MS_FLAG
#define PS2_SBUS_RL_FLAG PS2_SBUS_SM_FLAG
#else
#define R_LOCAL_SBUS(__reg_no) R_IOP_SBUS(__reg_no)
#define PS2_SBUS_LR_FLAG PS2_SBUS_SM_FLAG
#define PS2_SBUS_RL_FLAG PS2_SBUS_MS_FLAG
#endif

#ifdef _EE

#include <kernel.h>

#ifdef _KMODE
#define M_SuspendIntr(__stat_ptr) { }
#define M_ResumeIntr(__stat) { }
#else
#define M_SuspendIntr(__stat_ptr) *(int *) (__stat_ptr) = DIntr(); *(int *) (__stat_ptr) |= ((((ee_kmode_enter() >> 3) & 3) != 0) << 1)
#define M_ResumeIntr(__stat) { if(__stat & 2) { ee_kmode_exit(); } if(__stat & 1) { EIntr(); } }
#endif

#define M_DisableIrq(__irq, __stat_ptr) *(int *) (__stat_ptr) = DisableIntc(__irq)
#define M_EnableIrq(__irq) EnableIntc(__irq)

// TODO: FIX ME!!!
#define M_ReleaseIrqHandler(__irq) if(0) { }

#define M_RegisterIrqHandler(__irq, __handler, __param) _SetIntcHandler((__irq), (__handler))

#else
// #_IOP

#include <intrman.h>
#include <sysclib.h>

#define M_SuspendIntr(__old_state_ptr) CpuSuspendIntr(__old_state_ptr)
#define M_ResumeIntr(__old_state) CpuResumeIntr(__old_state)

#define M_DisableIrq(__irq, __old_state_ptr) DisableIntr((__irq), (__old_state_ptr))
#define M_EnableIrq(__irq) EnableIntr(__irq)

#define M_ReleaseIrqHandler(__irq) ReleaseIntrHandler(__irq)
#define M_RegisterIrqHandler(__irq, __handler, __param) RegisterIntrHandler((__irq), 1, (__handler), __param)

#endif

#define SIF2_MAX_CMD_HANDLERS (32)

#define SIF2_XFER_CHUNK_SIZE (128)

typedef void (*SIF2_TransferCbFunc) (void);

#endif // #ifndef __SBUS_PRIV

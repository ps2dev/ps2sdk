/*
# _____     ___ ____     ___ ____
#  ____|   |    ____|   |        | |____|
# |     ___|   |____ ___|    ____| |    \    PS2DEV Open Source Project.
#-----------------------------------------------------------------------
# Copyright ps2dev - http://www.ps2dev.org
# Licenced under Academic Free License version 2.0
# Review ps2sdk README & LICENSE files for further details.
*/

/**
 * @file
 * Definitions for low-memory globals for IOP.
 */

#ifndef __IOP_LOW_MEMORY_GLOBALS__
#define __IOP_LOW_MEMORY_GLOBALS__

typedef struct iop_low_memory_globals_
{
  u32 eh_at_save;
  u32 eh_epc_save;
  u32 eh_status_save;
  u32 eh_cause_save;
  u32 eh_k0_save;
  u32 unk14;
  u32 unk18;
  u32 dispatch_interrupt_state;
  u32 bh_k0_save;
  u32 bh_epc_save;
  u32 bh_cause_save;
  u32 bh_status_save;
  u32 bh_breakpoint_control_save;
  u32 unk34;
  u32 unk38;
  u32 unk3c;
  void *exc_table[0x10];
  intrman_intr_handler_data_t intr_handlers[0x40];
} iop_low_memory_globals_t;

#if !defined(USE_IOP_LOW_MEMORY_GLOBALS) && defined(_IOP)
// cppcheck-suppress-macro constVariablePointer
#define USE_IOP_LOW_MEMORY_GLOBALS() iop_low_memory_globals_t *const iop_low_memory_globals = (iop_low_memory_globals_t *)0x400
#endif
#if !defined(USE_IOP_LOW_MEMORY_GLOBALS)
#define USE_IOP_LOW_MEMORY_GLOBALS()
#endif

#endif /* __IOP_LOW_MEMORY_GLOBALS__ */

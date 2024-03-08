/*
# _____     ___ ____     ___ ____
#  ____|   |    ____|   |        | |____|
# |     ___|   |____ ___|    ____| |    \    PS2DEV Open Source Project.
#-----------------------------------------------------------------------
# Copyright ps2dev - http://www.ps2dev.org
# Licenced under Academic Free License version 2.0
# Review ps2sdk README & LICENSE files for further details.
*/

#include "intrman.h"
#include "irx_imports.h"
#include "kerr.h"

#include "iop_low_memory_globals.h"
#include "iop_mmio_hwport.h"

extern struct irx_export_table _exp_intrman;

#ifdef _IOP
IRX_ID("Interrupt_Manager", 1, 1);
#endif
// Based on the module from SCE SDK 1.3.4.

#define PRID $15

#define _mfc0(reg)                                                                                                     \
	({                                                                                                                   \
		u32 val;                                                                                                           \
		__asm__ volatile("mfc0 %0, " #reg : "=r"(val));                                                                    \
		val;                                                                                                               \
	})

#define mfc0(reg) _mfc0(reg)

extern int CpuGetICTRL();
extern void CpuEnableICTRL();
extern int dma_interrupt_handler(void *userdata);

extern exception_handler_struct_t exception_interrupt_handler;
extern exception_handler_struct_t exception_priority_interrupt_handler;
extern exception_handler_struct_t exception_system_handler;

static intrman_internals_t intrman_internals;

int _start(int ac, char **av)
{
	s32 prid;
	int i;
	USE_IOP_MMIO_HWPORT();
	USE_IOP_LOW_MEMORY_GLOBALS();

	(void)ac;
	(void)av;

	prid = mfc0(PRID);

#ifdef BUILDING_INTRMANP
	if ( prid >= 16 )
	{
		if ( (iop_mmio_hwport->iop_sbus_ctrl[0] & 8) == 0 )
		{
			return MODULE_NO_RESIDENT_END;
		}
	}
#else
	if ( prid < 16 )
	{
		return MODULE_NO_RESIDENT_END;
	}

	if ( (iop_mmio_hwport->iop_sbus_ctrl[0] & 8) != 0 )
	{
		return MODULE_NO_RESIDENT_END;
	}
#endif

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Warray-bounds"
	iop_low_memory_globals->dispatch_interrupt_state = -1;
#pragma GCC diagnostic pop
	iop_mmio_hwport->imask = 0;
	iop_mmio_hwport->dmac1.dicr1 = 0;
#ifndef BUILDING_INTRMANP
	iop_mmio_hwport->dmac2.dicr2 = 0;
#endif
	intrman_internals.masked_icr_1 = -1;
	intrman_internals.masked_icr_2 = -1;
	intrman_internals.interrupt_handler_table = &(iop_low_memory_globals->intr_handlers[0]);
	// Unofficial: The upper bound was changed from 0x40 because it is not actually used
	for ( i = 0; i < 0x30; i += 1 )
	{
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Warray-bounds"
		intrman_internals.interrupt_handler_table[i].handler = NULL;
		intrman_internals.interrupt_handler_table[i].userdata = NULL;
#pragma GCC diagnostic pop
	}
	RegisterExceptionHandler(0, &exception_interrupt_handler);
	RegisterPriorityExceptionHandler(0, 3, &exception_priority_interrupt_handler);
	RegisterExceptionHandler(8, &exception_system_handler);
	// Unofficial: pass internal structure
	RegisterIntrHandler(IOP_IRQ_DMA, 1, dma_interrupt_handler, (void *)&intrman_internals);
	RegisterLibraryEntries(&_exp_intrman);
	return 0;
}

int intrman_deinit(void)
{
	USE_IOP_MMIO_HWPORT();

	iop_mmio_hwport->imask = 0;
	iop_mmio_hwport->dmac1.dicr1 = 0;
#ifndef BUILDING_INTRMANP
	iop_mmio_hwport->dmac2.dicr2 = 0;
#endif
	return 0;
}

intrman_internals_t *GetIntrmanInternalData(void)
{
	return &intrman_internals;
}

int RegisterIntrHandler(int irq, int mode, int (*handler)(void *), void *arg)
{
	int intr_handler_offset;
	int state;

	if ( QueryIntrContext() != 0 )
	{
		return KE_ILLEGAL_CONTEXT;
	}
	CpuSuspendIntr(&state);
	if ( irq >= IOP_IRQ_VBLANK && irq <= IOP_IRQ_DMA_SIO2_OUT )
	{
		intr_handler_offset = irq;
	}
	else if ( irq >= IOP_IRQ_SW1 && IOP_IRQ_SW1 <= IOP_IRQ_SW2 )
	{
		intr_handler_offset = irq - 0x10;
	}
	else
	{
		CpuResumeIntr(state);
		return KE_ILLEGAL_INTRCODE;
	}
	if ( intrman_internals.interrupt_handler_table[intr_handler_offset].handler )
	{
		CpuResumeIntr(state);
		return KE_FOUND_HANDLER;
	}
	if ( irq < IOP_IRQ_DMA_MDEC_IN || irq > IOP_IRQ_DMA_SIO2_OUT )
	{
		intrman_internals.interrupt_handler_table[intr_handler_offset].handler =
			(int (*)(void *))((uiptr)handler | (mode & 3));
	}
	else
	{
		intrman_internals.interrupt_handler_table[intr_handler_offset].handler = handler;
	}
	intrman_internals.interrupt_handler_table[intr_handler_offset].userdata = arg;
	CpuResumeIntr(state);
	return 0;
}

int ReleaseIntrHandler(int irq)
{
	int intr_handler_offset;
	int state;

	if ( QueryIntrContext() != 0 )
	{
		return KE_ILLEGAL_CONTEXT;
	}
	CpuSuspendIntr(&state);
	if ( irq >= IOP_IRQ_VBLANK && irq <= IOP_IRQ_DMA_SIO2_OUT )
	{
		intr_handler_offset = irq;
	}
	else if ( irq >= IOP_IRQ_SW1 && IOP_IRQ_SW1 <= IOP_IRQ_SW2 )
	{
		intr_handler_offset = irq - 0x10;
	}
	else
	{
		CpuResumeIntr(state);
		return KE_ILLEGAL_INTRCODE;
	}
	if ( !intrman_internals.interrupt_handler_table[intr_handler_offset].handler )
	{
		CpuResumeIntr(state);
		return KE_NOTFOUND_HANDLER;
	}
	intrman_internals.interrupt_handler_table[intr_handler_offset].handler = NULL;
	CpuResumeIntr(state);
	return 0;
}

extern int intrman_syscall_04_CpuDisableIntr(void);
extern int intrman_syscall_08_CpuEnableIntr(void);
extern int intrman_syscall_10(void);
extern int intrman_syscall_14(int state);

int CpuSuspendIntr(int *state)
{
	int intrstate;

#ifdef BUILDING_INTRMANP
	intrstate = intrman_syscall_10();
#else
	USE_IOP_MMIO_HWPORT();
	intrstate = iop_mmio_hwport->iop_sbus_info;
#endif
	if ( state )
		*state = intrstate;
#ifdef BUILDING_INTRMANP
	if ( (intrstate & 0x404) != 0x404 )
		return KE_CPUDI;
#else
	if ( !intrstate )
		return KE_CPUDI;
#endif
	return 0;
}

int CpuResumeIntr(int state)
{
#ifdef BUILDING_INTRMANP
	intrman_syscall_14(state);
#else
	USE_IOP_MMIO_HWPORT();
	iop_mmio_hwport->iop_sbus_info = state;
#endif
	return 0;
}

int CpuDisableIntr()
{
#ifdef BUILDING_INTRMANP
	if ( intrman_syscall_04_CpuDisableIntr() == 0 )
		return KE_CPUDI;
#else
	if ( CpuGetICTRL() == 0 )
		return KE_CPUDI;
#endif
	return 0;
}

int CpuEnableIntr()
{
	intrman_syscall_08_CpuEnableIntr();
#ifndef BUILDING_INTRMANP
	CpuEnableICTRL();
#endif
	return 0;
}

int CpuGetICTRL()
{
#ifdef BUILDING_INTRMANP
	return intrman_syscall_04_CpuDisableIntr();
#else
	USE_IOP_MMIO_HWPORT();
	return iop_mmio_hwport->iop_sbus_info;
#endif
}

void CpuEnableICTRL()
{
#ifdef BUILDING_INTRMANP
	intrman_syscall_08_CpuEnableIntr();
#else
	USE_IOP_MMIO_HWPORT();
	iop_mmio_hwport->iop_sbus_info = 1;
#endif
}

// clang-format off
__asm__ (
	"\t" ".set push" "\n"
	"\t" ".set noat" "\n"
	"\t" ".set noreorder" "\n"
	"\t" ".global intrman_syscall_04_CpuDisableIntr" "\n"
	"\t" "intrman_syscall_04_CpuDisableIntr:" "\n"
	"\t" "  addiu       $v0, $zero, 0x04" "\n"
	"\t" "  syscall     0" "\n"
	"\t" "  jr          $ra" "\n"
	"\t" "   nop" "\n"
	"\t" ".set pop" "\n"
);

__asm__ (
	"\t" ".set push" "\n"
	"\t" ".set noat" "\n"
	"\t" ".set noreorder" "\n"
	"\t" ".global intrman_syscall_08_CpuEnableIntr" "\n"
	"\t" "intrman_syscall_08_CpuEnableIntr:" "\n"
	"\t" "  addiu       $v0, $zero, 0x08" "\n"
	"\t" "  syscall     0" "\n"
	"\t" "  jr          $ra" "\n"
	"\t" "   nop" "\n"
	"\t" ".set pop" "\n"
);

__asm__ (
	"\t" ".set push" "\n"
	"\t" ".set noat" "\n"
	"\t" ".set noreorder" "\n"
	"\t" ".global intrman_syscall_10" "\n"
	"\t" "intrman_syscall_10:" "\n"
	"\t" "  addiu       $v0, $zero, 0x10" "\n"
	"\t" "  syscall     0" "\n"
	"\t" "  jr          $ra" "\n"
	"\t" "   nop" "\n"
	"\t" ".set pop" "\n"
);

__asm__ (
	"\t" ".set push" "\n"
	"\t" ".set noat" "\n"
	"\t" ".set noreorder" "\n"
	"\t" ".global intrman_syscall_14" "\n"
	"\t" "intrman_syscall_14:" "\n"
	"\t" "  addiu       $v0, $zero, 0x14" "\n"
	"\t" "  syscall     0" "\n"
	"\t" "  jr          $ra" "\n"
	"\t" "   nop" "\n"
	"\t" ".set pop" "\n"
);

__asm__ (
	"\t" ".set push" "\n"
	"\t" ".set noat" "\n"
	"\t" ".set noreorder" "\n"
	"\t" ".global CpuInvokeInKmode" "\n"
	"\t" "CpuInvokeInKmode:" "\n"
	"\t" "  addiu       $v0, $zero, 0x0C" "\n"
	"\t" "  syscall     0" "\n"
	"\t" "  jr          $ra" "\n"
	"\t" "   nop" "\n"
	"\t" ".set pop" "\n"
);
// clang-format on

int EnableIntr(int irq)
{
	int ret;
	int irq_index;
	int state;
	USE_IOP_MMIO_HWPORT();

	ret = 0;
	irq_index = irq & 0xFF;
	CpuSuspendIntr(&state);
	if ( irq_index < IOP_IRQ_DMA_MDEC_IN )
	{
		iop_mmio_hwport->imask |= (1 << irq_index);
	}
	else if ( (irq_index >= IOP_IRQ_DMA_MDEC_IN) && (irq_index <= IOP_IRQ_DMA_GPU_OTC) )
	{
		iop_mmio_hwport->dmac1.dicr1 = (iop_mmio_hwport->dmac1.dicr1 & (~(1 << (irq_index - 32)) & 0xFFFFFF))
																 | (((((irq & 0xFF00) >> 8) & 0x1) != 0) ? (1 << (irq_index - 32)) : 0)
																 | (1 << (irq_index - 32 + 16)) | 0x800000;
#ifndef BUILDING_INTRMANP
		iop_mmio_hwport->dmac2.dicr2 = (iop_mmio_hwport->dmac2.dicr2 & (~(1 << (irq_index - 32)) & 0xFFFFFF))
																 | (((((irq & 0xFF00) >> 8) & 0x2) != 0) ? (1 << (irq_index - 32)) : 0);
#endif
		iop_mmio_hwport->imask |= 8;
#if 0
		/* The following was in the original. */
#ifdef BUILDING_INTRMANP
		ret = KE_ILLEGAL_INTRCODE;
#endif
#endif
	}
#ifndef BUILDING_INTRMANP
	else if ( (irq_index >= IOP_IRQ_DMA_SPU2) && (irq_index <= IOP_IRQ_DMA_SIO2_OUT) )
	{
		iop_mmio_hwport->dmac2.dicr2 = (iop_mmio_hwport->dmac2.dicr2 & (~(1 << (irq_index - 40 + 7)) & 0xFFFFFF))
																 | (((((irq & 0xFF00) >> 8) & 0x2) != 0) ? (1 << (irq_index - 33)) : 0)
																 | (1 << (irq_index - 40 + 16));
		iop_mmio_hwport->dmac1.dicr1 = (iop_mmio_hwport->dmac1.dicr1 & 0x7FFFFF) | 0x800000;
		iop_mmio_hwport->imask |= 8;
	}
#endif
	else
	{
		ret = KE_ILLEGAL_INTRCODE;
	}
	CpuResumeIntr(state);
	return ret;
}

int DisableIntr(int irq, int *res)
{
	int ret;
	int res_temp;
	int irq_index;
	u32 imask;
	int dicr_tmp;
	int state;
	USE_IOP_MMIO_HWPORT();

	ret = 0;
	res_temp = KE_INTRDISABLE;
	irq_index = irq & 0xFF;
	CpuSuspendIntr(&state);
	if ( irq_index < IOP_IRQ_DMA_MDEC_IN )
	{
		imask = iop_mmio_hwport->imask;
		iop_mmio_hwport->imask = imask & ~(1 << irq_index);
		if ( (imask & (1 << irq_index)) != 0 )
		{
			res_temp = irq_index;
		}
		else
		{
			ret = KE_INTRDISABLE;
		}
	}
	else if ( (irq_index >= IOP_IRQ_DMA_MDEC_IN) && (irq_index <= IOP_IRQ_DMA_GPU_OTC) )
	{
		dicr_tmp = iop_mmio_hwport->dmac1.dicr1 & 0xFFFFFF;
		if ( (dicr_tmp & (1 << (irq_index - 16))) != 0 )
		{
			res_temp = irq_index;
			if ( ((dicr_tmp >> (irq_index - 32)) & 1) != 0 )
				res_temp |= 0x100;
#ifndef BUILDING_INTRMANP
			if ( iop_mmio_hwport->dmac2.dicr2 & (1 << (irq_index - 32)) )
				res_temp |= 0x200;
#endif
			iop_mmio_hwport->dmac1.dicr1 = dicr_tmp & ~(1 << (irq_index - 16));
		}
		else
		{
			ret = KE_INTRDISABLE;
		}
	}
#ifndef BUILDING_INTRMANP
	else if ( (irq_index >= IOP_IRQ_DMA_SPU2) && (irq_index <= IOP_IRQ_DMA_SIO2_OUT) )
	{
		dicr_tmp = iop_mmio_hwport->dmac2.dicr2 & 0xFFFFFF;
		if ( (dicr_tmp & (1 << (irq_index - 24))) != 0 )
		{
			res_temp = irq_index;
			if ( (dicr_tmp >> (irq_index - 33)) & 1 )
				res_temp |= 0x200;
			iop_mmio_hwport->dmac2.dicr2 = dicr_tmp & ~(1 << (irq_index - 24));
		}
		else
		{
			ret = KE_INTRDISABLE;
		}
	}
#endif
	else
	{
		ret = KE_ILLEGAL_INTRCODE;
	}
	if ( res )
		*res = res_temp;
	CpuResumeIntr(state);
	return ret;
}

void EnableDispatchIntr(int irq)
{
	int irq_index;
	int state;
	USE_IOP_LOW_MEMORY_GLOBALS();

	irq_index = irq & 0xFF;
	CpuSuspendIntr(&state);
	if ( irq_index < IOP_IRQ_DMA_MDEC_IN )
	{
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Warray-bounds"
		iop_low_memory_globals->dispatch_interrupt_state |= 1 << irq_index;
#pragma GCC diagnostic pop
	}
	else if ( (irq_index >= IOP_IRQ_DMA_MDEC_IN) && (irq_index <= IOP_IRQ_DMA_BERR) )
	{
		intrman_internals.masked_icr_1 |= 1 << (irq_index - 8);
	}
	else if ( (irq_index >= IOP_IRQ_DMA_SPU2) && (irq_index <= IOP_IRQ_DMA_SIO2_OUT) )
	{
		intrman_internals.masked_icr_2 |= 1 << (irq_index - 16);
	}
	CpuResumeIntr(state);
}

void DisableDispatchIntr(int irq)
{
	int irq_index;
	int state;
	USE_IOP_LOW_MEMORY_GLOBALS();

	irq_index = irq & 0xFF;
	CpuSuspendIntr(&state);
	if ( irq_index < IOP_IRQ_DMA_MDEC_IN )
	{
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Warray-bounds"
		iop_low_memory_globals->dispatch_interrupt_state &= ~(1 << irq_index);
#pragma GCC diagnostic pop
	}
	else if ( (irq_index >= IOP_IRQ_DMA_MDEC_IN) && (irq_index <= IOP_IRQ_DMA_BERR) )
	{
		intrman_internals.masked_icr_1 &= ~(1 << (irq_index - 8));
	}
	else if ( (irq_index >= IOP_IRQ_DMA_SPU2) && (irq_index <= IOP_IRQ_DMA_SIO2_OUT) )
	{
		intrman_internals.masked_icr_2 &= ~(1 << (irq_index - 16));
	}
	CpuResumeIntr(state);
}

void intrman_set_dmac2_interrupt_handler_mask(int mask)
{
	intrman_internals.dmac2_interrupt_handler_mask = mask;
}

#ifndef BUILDING_INTRMANP
// Unofficial: reference relative to internal structure
static void dmac2_enable_set(const intrman_internals_t *p_intrman_internals, int mask)
{
	USE_IOP_MMIO_HWPORT();

	if ( (p_intrman_internals->dmac2_interrupt_handler_mask & mask) != 0 )
	{
		while ( iop_mmio_hwport->dmac2.dmacen != 1 )
		{
			iop_mmio_hwport->dmac2.dmacen = 1;
		}
	}
}

// Unofficial: reference relative to internal structure
static void dmac2_enable_unset(const intrman_internals_t *p_intrman_internals, int mask)
{
	USE_IOP_MMIO_HWPORT();

	if ( (p_intrman_internals->dmac2_interrupt_handler_mask & mask) != 0 )
	{
		while ( iop_mmio_hwport->dmac2.dmacen != 0 )
		{
			iop_mmio_hwport->dmac2.dmacen = 0;
		}
	}
}
#endif

int dma_interrupt_handler(void *userdata)
{
#ifndef BUILDING_INTRMANP
	u32 dma2_intr_flags;
#endif
	u32 dma1_intr_flags_tmp;
	u32 dma1_intr_flags;
	int bus_error_intr_flag;
	int i;
	int masked_icr_1_tmp;
#ifndef BUILDING_INTRMANP
	int masked_icr_2_tmp;
#endif
	intrman_internals_t *p_intrman_internals;
	intrman_intr_handler_data_t *interrupt_handler_table;
	USE_IOP_MMIO_HWPORT();

	// Unofficial: reference relative to internal structure
	p_intrman_internals = (intrman_internals_t *)userdata;
	interrupt_handler_table = p_intrman_internals->interrupt_handler_table;
	masked_icr_1_tmp = p_intrman_internals->masked_icr_1;
#ifndef BUILDING_INTRMANP
	masked_icr_2_tmp = p_intrman_internals->masked_icr_2;
#endif
#ifndef BUILDING_INTRMANP
	dmac2_enable_unset(p_intrman_internals, 1);
#endif
	while ( 1 )
	{
#ifndef BUILDING_INTRMANP
		dmac2_enable_unset(p_intrman_internals, 2);
#endif
#ifndef BUILDING_INTRMANP
		dma2_intr_flags = ((iop_mmio_hwport->dmac2.dicr2 & masked_icr_2_tmp) >> 24) & 0x3F;
#endif
		dma1_intr_flags_tmp = iop_mmio_hwport->dmac1.dicr1 & masked_icr_1_tmp;
		dma1_intr_flags = ((dma1_intr_flags_tmp & 0xFF00) >> 8) & 0x7F;
		bus_error_intr_flag = (dma1_intr_flags_tmp >> 15) & 1;
#ifndef BUILDING_INTRMANP
		dmac2_enable_set(p_intrman_internals, 2);
#endif
#ifdef BUILDING_INTRMANP
		if ( !(dma1_intr_flags | bus_error_intr_flag) )
			break;
#else
		if ( !(dma1_intr_flags | dma2_intr_flags | bus_error_intr_flag) )
			break;
#endif
		if ( bus_error_intr_flag )
		{
#ifndef BUILDING_INTRMANP
			dmac2_enable_unset(p_intrman_internals, 2);
#endif
			iop_mmio_hwport->dmac1.dicr1 &= 0xFF7FFF;
#ifndef BUILDING_INTRMANP
			dmac2_enable_set(p_intrman_internals, 2);
#endif
			if ( interrupt_handler_table[IOP_IRQ_DMA_BERR].handler )
			{
				interrupt_handler_table[IOP_IRQ_DMA_BERR].handler(interrupt_handler_table[IOP_IRQ_DMA_BERR].userdata);
			}
		}
		if ( dma1_intr_flags )
		{
			for ( i = 0; i < 7; i += 1 )
			{
				if ( (dma1_intr_flags & 1) != 0 )
				{
#ifndef BUILDING_INTRMANP
					dmac2_enable_unset(p_intrman_internals, 2);
#endif
					iop_mmio_hwport->dmac1.dicr1 &= ((1 << (i + 24)) | 0xFFFFFF);
					if ( interrupt_handler_table[i + IOP_IRQ_DMA_MDEC_IN].handler )
					{
#ifndef BUILDING_INTRMANP
						dmac2_enable_set(p_intrman_internals, 4);
#endif
						if ( !interrupt_handler_table[i + IOP_IRQ_DMA_MDEC_IN].handler(
									 interrupt_handler_table[i + IOP_IRQ_DMA_MDEC_IN].userdata) )
						{
#ifndef BUILDING_INTRMANP
							dmac2_enable_unset(p_intrman_internals, 4);
#endif
							iop_mmio_hwport->dmac1.dicr1 &= 0xFFFFFF & ~(1 << (i + 16));
						}
#ifndef BUILDING_INTRMANP
						dmac2_enable_unset(p_intrman_internals, 4);
#endif
					}
					else
					{
						iop_mmio_hwport->dmac1.dicr1 &= 0xFFFFFF & ~(1 << (i + 16));
					}
#ifndef BUILDING_INTRMANP
					dmac2_enable_set(p_intrman_internals, 2);
#endif
				}
				dma1_intr_flags >>= 1;
				if ( !dma1_intr_flags )
					break;
			}
		}
#ifndef BUILDING_INTRMANP
		if ( dma2_intr_flags )
		{
			for ( i = 0; i < 6; i += 1 )
			{
				if ( (dma2_intr_flags & 1) != 0 )
				{
					dmac2_enable_unset(p_intrman_internals, 2);
					iop_mmio_hwport->dmac2.dicr2 &= (1 << (i + 24)) | 0xFFFFFF;
					if ( interrupt_handler_table[i + IOP_IRQ_DMA_SPU2].handler )
					{
						dmac2_enable_set(p_intrman_internals, 4);
						if ( !(interrupt_handler_table[i + IOP_IRQ_DMA_SPU2].handler(
									 interrupt_handler_table[i + IOP_IRQ_DMA_SPU2].userdata)) )
						{
							dmac2_enable_unset(p_intrman_internals, 4);
							iop_mmio_hwport->dmac2.dicr2 &= 0xFFFFFF & ~(1 << (i + 16));
						}
						dmac2_enable_unset(p_intrman_internals, 4);
					}
					else
					{
						iop_mmio_hwport->dmac2.dicr2 &= 0xFFFFFF & ~(1 << (i + 16));
					}
					dmac2_enable_set(p_intrman_internals, 2);
				}
				dma2_intr_flags >>= 1;
				if ( !dma2_intr_flags )
					break;
			}
		}
#endif
	}
#ifndef BUILDING_INTRMANP
	dmac2_enable_unset(p_intrman_internals, 2);
#endif
	{
		u32 dicr1x;

		dicr1x = iop_mmio_hwport->dmac1.dicr1;
		iop_mmio_hwport->dmac1.dicr1 = (dicr1x & 0x7FFFFF);
		iop_mmio_hwport->dmac1.dicr1 = (dicr1x & 0x7FFFFF) | 0x800000;
	}
#ifndef BUILDING_INTRMANP
	dmac2_enable_set(p_intrman_internals, 2);
	dmac2_enable_set(p_intrman_internals, 1);
#endif
	return 1;
}

static void *new_context_stub_cb(void *ctx);
static int preempt_stub_cb(int unk);

void *ctx_switch_cb = new_context_stub_cb;
void *ctx_switch_required_cb = preempt_stub_cb;

void SetNewCtxCb(void *cb)
{
	ctx_switch_cb = cb;
}

void ResetNewCtxCb(void)
{
	ctx_switch_cb = new_context_stub_cb;
}

void SetShouldPreemptCb(void *cb)
{
	ctx_switch_required_cb = cb;
}

void ResetShouldPreemptCb(void)
{
	ctx_switch_required_cb = preempt_stub_cb;
}

u32 tempstack[0x200];

// clang-format off
__asm__ (
	"\t" ".set push" "\n"
	"\t" ".set noat" "\n"
	"\t" ".set noreorder" "\n"
	"\t" ".global QueryIntrContext" "\n"
	"\t" "QueryIntrContext:" "\n"
	"\t" "  addu        $a0, $sp, $zero" "\n"
	"\t" ".global QueryIntrStack" "\n"
	"\t" "QueryIntrStack:" "\n"
	"\t" "  lui         $v1, %hi(tempstack + 0x800)" "\n"
	"\t" "  addiu       $v1, $v1, %lo(tempstack + 0x800)" "\n" // sizeof(tempstack)
	"\t" "  sltu        $v0, $a0, $v1" "\n"
	"\t" "  beqz        $v0, 9f" "\n"
	"\t" "   nop" "\n"
	"\t" "  addiu       $v1, $v1, -0x800" "\n"
	"\t" "  sltu        $v0, $v1, $a0" "\n"
	"\t" "9:" "\n"
	"\t" "  jr          $ra" "\n"
	"\t" "   nop" "\n"
	"\t" ".set pop" "\n"
);

__asm__ (
	"\t" ".set push" "\n"
	"\t" ".set noat" "\n"
	"\t" ".set noreorder" "\n"
	"\t" ".global iCatchMultiIntr" "\n"
	"\t" "iCatchMultiIntr:" "\n"
	"\t" "  lui         $v1, %hi(tempstack + 0x800)" "\n"
	"\t" "  addiu       $v1, $v1, %lo(tempstack + 0x800)" "\n" // sizeof(tempstack)
	"\t" "  sltu        $v0, $sp, $v1" "\n"
	"\t" "  beqz        $v0, 1f" "\n"
	"\t" "   nop" "\n"
	"\t" "  addiu       $v1, $v1, -0x800" "\n"
	"\t" "  sltu        $v0, $v1, $sp" "\n"
	"\t" "  beqz        $v0, 2f" "\n"
	"\t" "   nop" "\n"
	"\t" "  addiu       $v1, $v1, 0x160" "\n"
	"\t" "  sltu        $v0, $v1, $sp" "\n"
	"\t" "  beqz        $v0, 1f" "\n"
	"\t" "   nop" "\n"
	"\t" "  mfc0        $v0, $12" "\n"
	"\t" "  nop" "\n"
	"\t" "  andi        $v1, $v0, 0x1" "\n"
	"\t" "  bnez        $v1, 1f" "\n"
	"\t" "   ori        $v1, $v0, 0x1" "\n"
	"\t" "  mtc0        $v1, $12" "\n"
	"\t" "  nop" "\n"
	"\t" "  nop" "\n"
	"\t" "  mtc0        $v0, $12" "\n"
	"\t" "1:" "\n"
	"\t" "  jr          $ra" "\n"
	"\t" "   nop" "\n"
	"\t" "2:" "\n"
	"\t" "  break       2" "\n"
	"\t" "  jr          $ra" "\n"
	"\t" "   nop" "\n"
	"\t" ".set pop" "\n"
);

__asm__ (
	"\t" ".set push" "\n"
	"\t" ".set noat" "\n"
	"\t" ".set noreorder" "\n"
	"\t" "exception_interrupt_handler:" "\n"
	"\t" "  .word 0" "\n"
	"\t" "  .word 0" "\n"

	"\t" "exception_interrupt_handler_code:" "\n"
	"\t" "  addiu       $sp, $sp, -0x98" "\n"
	"\t" "  lw          $at, 0x400($zero)" "\n"
	"\t" "  sw          $ra, 0x7C($sp)" "\n"
	"\t" "  sw          $at, 0x4($sp)" "\n"
	"\t" "  sw          $v0, 0x8($sp)" "\n"
	"\t" "  sw          $v1, 0xC($sp)" "\n"
	"\t" "  sw          $a0, 0x10($sp)" "\n"
	"\t" "  sw          $a1, 0x14($sp)" "\n"
	"\t" "  sw          $a2, 0x18($sp)" "\n"
	"\t" "  sw          $a3, 0x1C($sp)" "\n"
	"\t" "  addiu       $v0, $sp, 0x98" "\n"
	"\t" "  sw          $v0, 0x74($sp)" "\n"
	"\t" "  mfhi        $v0" "\n"
	"\t" "  mflo        $v1" "\n"
	"\t" "  sw          $v0, 0x80($sp)" "\n"
	"\t" "  sw          $v1, 0x84($sp)" "\n"
	"\t" "  lw          $v0, 0x408($zero)" "\n"
	"\t" "  lw          $v1, 0x404($zero)" "\n"
	"\t" "  sw          $v0, 0x88($sp)" "\n"
	"\t" "  sw          $v1, 0x8C($sp)" "\n"
#ifndef BUILDING_INTRMANP
	"\t" "  lui         $v0, (0xBF801078 >> 16)" "\n"
	"\t" "  ori         $v0, $v0, (0xBF801078 & 0xFFFF)" "\n"
	"\t" "  lw          $v1, 0x0($v0)" "\n"
	"\t" "  addiu       $a0, $zero, 0x1" "\n"
	"\t" "  sw          $v1, 0x90($sp)" "\n"
	"\t" "  sw          $a0, 0x0($v0)" "\n"
#endif
	"\t" "  lui         $v0, (0xAC0000FE >> 16)" "\n"
	"\t" "  ori         $v0, $v0, (0xAC0000FE & 0xFFFF)" "\n"
	"\t" "  sw          $v0, 0x0($sp)" "\n"
	"\t" "  jal         QueryIntrContext" "\n"
	"\t" "   nop" "\n"
	"\t" "  beqz        $v0, .Lexception_interrupt_handler_code_1" "\n"
	"\t" "   nop" "\n"
	"\t" "  addu        $v0, $sp, $zero" "\n"
	"\t" "  addiu       $sp, $sp, -0x18" "\n"
	"\t" "  b           .Lexception_interrupt_handler_code_2" "\n"
	"\t" "   sw         $v0, 0x14($sp)" "\n"
	"\t" ".Lexception_interrupt_handler_code_1:" "\n"
	"\t" "  addu        $v0, $sp, $zero" "\n"
	"\t" "  lui         $sp, %hi(tempstack + 0x7E0)" "\n"
	"\t" "  addiu       $sp, $sp, %lo(tempstack + 0x7E0)" "\n"
	"\t" "  sw          $v0, 0x14($sp)" "\n"
	"\t" ".Lexception_interrupt_handler_code_2:" "\n"
	"\t" "  lw          $a0, 0x40C($zero)" "\n"
	"\t" "  nop" "\n"
	"\t" "  sll         $a0, $a0, 22" "\n"
	"\t" "  srl         $a0, $a0, 30" "\n"
	"\t" "  bne         $zero, $a0, .Lexception_interrupt_handler_code_5" "\n"
	"\t" "   nop" "\n"
	"\t" "  lui         $k0, (0xBF801070 >> 16)" "\n"
	"\t" "  ori         $k0, $k0, (0xBF801070 & 0xFFFF)" "\n"
	"\t" "  lw          $k1, 0x4($k0)" "\n"
	"\t" "  lw          $a0, 0x0($k0)" "\n"
	"\t" "  lw          $a1, 0x41C($zero)" "\n"
	"\t" "  and         $a0, $a0, $k1" "\n"
	"\t" "  and         $a0, $a0, $a1" "\n"
	"\t" "  beq         $zero, $a0, .Lexception_interrupt_handler_code_4" "\n"
	"\t" "   nop" "\n"
	"\t" "  addiu       $v0, $zero, -0x4" "\n"
	"\t" ".Lexception_interrupt_handler_code_3:" "\n"
	"\t" "  sll         $a2, $a0, 28" "\n"
	"\t" "  srl         $a0, $a0, 4" "\n"
	"\t" "  beqz        $a2, .Lexception_interrupt_handler_code_3" "\n"
	"\t" "   addi       $v0, $v0, 0x4" "\n"
	"\t" "  srl         $a0, $a2, 26" "\n"
	"\t" "  andi        $a0, $a0, 0x1C" "\n"
	"\t" "  lui         $a2, (0x1020103 >> 16)" "\n"
	"\t" "  ori         $a2, $a2, (0x1020103 & 0xFFFF)" "\n"
	"\t" "  srlv        $a2, $a2, $a0" "\n"
	"\t" "  andi        $a2, $a2, 0xF" "\n"
	"\t" "  add         $v0, $v0, $a2" "\n"
	"\t" "  addiu       $a2, $zero, 0x1" "\n"
	"\t" "  sllv        $a2, $a2, $v0" "\n"
	"\t" "  sw          $a2, 0x10($sp)" "\n"
	"\t" "  not         $a2, $a2" "\n"
	"\t" "  and         $a3, $a2, $k1" "\n"
	"\t" "  sw          $a3, 0x4($k0)" "\n"
	"\t" "  sw          $a2, 0x0($k0)" "\n"
	"\t" "  sll         $a2, $v0, 3" "\n"
	"\t" "  lw          $a3, 0x480($a2)" "\n"
	"\t" "  lw          $a0, 0x484($a2)" "\n"
	"\t" "  bne         $zero, $a3, .Lexception_interrupt_handler_code_7" "\n"
	"\t" "   nop" "\n"
	"\t" ".Lexception_interrupt_handler_code_4:" "\n"
	"\t" "  lw          $sp, 0x14($sp)" "\n"
	"\t" "  nop" "\n"
	"\t" "  lw          $ra, 0x7C($sp)" "\n"
	"\t" "  lw          $at, 0x4($sp)" "\n"
	"\t" "  lw          $v0, 0x8($sp)" "\n"
	"\t" "  lw          $v1, 0xC($sp)" "\n"
	"\t" "  lw          $a0, 0x10($sp)" "\n"
	"\t" "  lw          $a1, 0x14($sp)" "\n"
	"\t" "  lw          $a2, 0x18($sp)" "\n"
	"\t" "  lw          $a3, 0x1C($sp)" "\n"
	"\t" "  addiu       $sp, $sp, 0x98" "\n"
	"\t" "  lui         $k0, %hi(exception_interrupt_handler)" "\n"
	"\t" "  lw          $k0, %lo(exception_interrupt_handler)($k0)" "\n"
	"\t" "  nop" "\n"
	"\t" "  jr          $k0" "\n"
	"\t" "   nop" "\n"
	"\t" ".Lexception_interrupt_handler_code_5:" "\n"
	"\t" "  sw          $zero, 0x10($sp)" "\n"
	"\t" "  andi        $a0, $a0, 0x1" "\n"
	"\t" "  bnez        $a0, .Lexception_interrupt_handler_code_6" "\n"
	"\t" "   addiu      $v0, $zero, 0x0" "\n"
	"\t" "  addiu       $v0, $zero, 0x1" "\n"
	"\t" ".Lexception_interrupt_handler_code_6:" "\n"
	"\t" "  addiu       $a2, $zero, 0x100" "\n"
	"\t" "  sllv        $a2, $a2, $v0" "\n"
	"\t" "  mfc0        $a0, $13" "\n"
	"\t" "  not         $a2, $a2" "\n"
	"\t" "  and         $a0, $a0, $a2" "\n"
	"\t" "  mtc0        $a0, $13" "\n"
	"\t" "  sll         $a2, $v0, 3" "\n"
	"\t" "  lw          $a3, 0x5F0($a2)" "\n"
	"\t" "  lw          $a0, 0x5F4($a2)" "\n"
	"\t" "  beq         $zero, $a3, .Lexception_interrupt_handler_code_4" "\n"
	"\t" "   nop" "\n"
	"\t" ".Lexception_interrupt_handler_code_7:" "\n"
	"\t" "  sll         $a2, $a3, 30" "\n"
	"\t" "  bnez        $a2, .Lexception_interrupt_handler_code_8" "\n"
	"\t" "   nop" "\n"
	"\t" "  srl         $a3, $a3, 2" "\n"
	"\t" "  sll         $a3, $a3, 2" "\n"
	"\t" "  jalr        $a3" "\n"
	"\t" "   nop" "\n"
	"\t" "  b           .Lexception_interrupt_handler_code_10" "\n"
	"\t" "   nop" "\n"
	"\t" ".Lexception_interrupt_handler_code_8:" "\n"
	"\t" "  lw          $v0, 0x14($sp)" "\n"
	"\t" "  nop" "\n"
	"\t" "  sw          $t0, 0x20($v0)" "\n"
	"\t" "  sw          $t1, 0x24($v0)" "\n"
	"\t" "  sw          $t2, 0x28($v0)" "\n"
	"\t" "  sw          $t3, 0x2C($v0)" "\n"
	"\t" "  sw          $t4, 0x30($v0)" "\n"
	"\t" "  sw          $t5, 0x34($v0)" "\n"
	"\t" "  sw          $t6, 0x38($v0)" "\n"
	"\t" "  sw          $t7, 0x3C($v0)" "\n"
	"\t" "  sw          $t8, 0x60($v0)" "\n"
	"\t" "  sw          $t9, 0x64($v0)" "\n"
	"\t" "  sw          $gp, 0x70($v0)" "\n"
	"\t" "  sw          $fp, 0x78($v0)" "\n"
	"\t" "  lui         $t0, (0xFF00FFFE >> 16)" "\n"
	"\t" "  ori         $t0, $t0, (0xFF00FFFE & 0xFFFF)" "\n"
	"\t" "  sw          $t0, 0x0($v0)" "\n"
	"\t" "  lui         $gp, (0xFFFF0000 >> 16)" "\n"
	"\t" "  sll         $a2, $a2, 1" "\n"
	"\t" "  beqz        $a2, .Lexception_interrupt_handler_code_9" "\n"
	"\t" "   nop" "\n"
	"\t" "  srl         $a3, $a3, 2" "\n"
	"\t" "  sll         $a3, $a3, 2" "\n"
	"\t" "  jalr        $a3" "\n"
	"\t" "   nop" "\n"
	"\t" "  b           .Lexception_interrupt_handler_code_10" "\n"
	"\t" "   nop" "\n"
	"\t" ".Lexception_interrupt_handler_code_9:" "\n"
	"\t" "  lw          $v0, 0x14($sp)" "\n"
	"\t" "  nop" "\n"
	"\t" "  sw          $s0, 0x40($v0)" "\n"
	"\t" "  sw          $s1, 0x44($v0)" "\n"
	"\t" "  sw          $s2, 0x48($v0)" "\n"
	"\t" "  sw          $s3, 0x4C($v0)" "\n"
	"\t" "  sw          $s4, 0x50($v0)" "\n"
	"\t" "  sw          $s5, 0x54($v0)" "\n"
	"\t" "  sw          $s6, 0x58($v0)" "\n"
	"\t" "  sw          $s7, 0x5C($v0)" "\n"
	"\t" "  addiu       $t0, $zero, -0x2" "\n"
	"\t" "  sw          $t0, 0x0($v0)" "\n"
	"\t" "  srl         $a3, $a3, 2" "\n"
	"\t" "  sll         $a3, $a3, 2" "\n"
	"\t" "  jalr        $a3" "\n"
	"\t" "   nop" "\n"
	"\t" ".Lexception_interrupt_handler_code_10:" "\n"
	"\t" "  lw          $a3, 0x10($sp)" "\n"
	"\t" "  mtc0        $zero, $12" "\n"
	"\t" "  lui         $a0, (0xBF801074 >> 16)" "\n"
	"\t" "  beq         $zero, $v0, .Lexception_interrupt_handler_code_11" "\n"
	"\t" "   ori        $a0, $a0, (0xBF801074 & 0xFFFF)" "\n"
	"\t" "  lw          $a2, 0x0($a0)" "\n"
	"\t" "  nop" "\n"
	"\t" "  or          $a3, $a3, $a2" "\n"
	"\t" "  sw          $a3, 0x0($a0)" "\n"
	"\t" ".Lexception_interrupt_handler_code_11:" "\n"
	"\t" "  lw          $a0, 0x14($sp)" "\n"
	"\t" "  jal         QueryIntrStack" "\n"
	"\t" "   nop" "\n"
	"\t" "  bnez        $v0, .Lexception_interrupt_handler_code_14" "\n"
	"\t" "   nop" "\n"
	"\t" "  lui         $v1, %hi(ctx_switch_required_cb)" "\n"
	"\t" "  lw          $v1, %lo(ctx_switch_required_cb)($v1)" "\n"
	"\t" "  nop" "\n"
	"\t" "  jalr        $v1" "\n"
	"\t" "   nop" "\n"
	"\t" "  lw          $a0, 0x14($sp)" "\n"
	"\t" "  beqz        $v0, .Lexception_interrupt_handler_code_14" "\n"
	"\t" "   nop" "\n"
	"\t" "  lw          $v0, 0x0($a0)" "\n"
	"\t" "  addiu       $a1, $zero, -0x2" "\n"
	"\t" "  beq         $v0, $a1, intrman_perform_context_switch" "\n"
	"\t" "   nop" "\n"
	"\t" "  lui         $a1, (0xFF00FFFE >> 16)" "\n"
	"\t" "  ori         $a1, $a1, (0xFF00FFFE & 0xFFFF)" "\n"
	"\t" "  beq         $v0, $a1, .Lexception_interrupt_handler_code_12" "\n"
	"\t" "   nop" "\n"
	"\t" "  sw          $t0, 0x20($a0)" "\n"
	"\t" "  sw          $t1, 0x24($a0)" "\n"
	"\t" "  sw          $t2, 0x28($a0)" "\n"
	"\t" "  sw          $t3, 0x2C($a0)" "\n"
	"\t" "  sw          $t4, 0x30($a0)" "\n"
	"\t" "  sw          $t5, 0x34($a0)" "\n"
	"\t" "  sw          $t6, 0x38($a0)" "\n"
	"\t" "  sw          $t7, 0x3C($a0)" "\n"
	"\t" "  sw          $t8, 0x60($a0)" "\n"
	"\t" "  sw          $t9, 0x64($a0)" "\n"
	"\t" "  sw          $gp, 0x70($a0)" "\n"
	"\t" "  sw          $fp, 0x78($a0)" "\n"
	"\t" ".Lexception_interrupt_handler_code_12:" "\n"
	"\t" "  sw          $s0, 0x40($a0)" "\n"
	"\t" "  sw          $s1, 0x44($a0)" "\n"
	"\t" "  sw          $s2, 0x48($a0)" "\n"
	"\t" "  sw          $s3, 0x4C($a0)" "\n"
	"\t" "  sw          $s4, 0x50($a0)" "\n"
	"\t" "  sw          $s5, 0x54($a0)" "\n"
	"\t" "  sw          $s6, 0x58($a0)" "\n"
	"\t" "  sw          $s7, 0x5C($a0)" "\n"
	"\t" "  addiu       $v0, $zero, -0x2" "\n"
	"\t" "  sw          $v0, 0x0($a0)" "\n"
	"\t" "intrman_perform_context_switch:" "\n"
	"\t" "  lui         $v1, %hi(ctx_switch_cb)" "\n"
	"\t" "  lw          $v1, %lo(ctx_switch_cb)($v1)" "\n"
	"\t" "  nop" "\n"
	"\t" "  jalr        $v1" "\n"
	"\t" "   nop" "\n"
	"\t" "  addu        $a0, $v0, $zero" "\n"
	"\t" ".Lexception_interrupt_handler_code_14:" "\n"
	"\t" "  addu        $sp, $a0, $zero" "\n"
	"\t" "  lw          $a0, 0x0($sp)" "\n"
	"\t" "  lui         $a1, (0xF0FF000C >> 16)" "\n"
	"\t" "  ori         $a1, $a1, (0xF0FF000C & 0xFFFF)" "\n"
	"\t" "  beq         $a0, $a1, .Lintrman_perform_context_switch_4" "\n"
	"\t" "   lui        $a1, (0xAC0000FE >> 16)" "\n"
	"\t" "  ori         $a1, $a1, (0xAC0000FE & 0xFFFF)" "\n"
	"\t" "  beq         $a0, $a1, .Lintrman_perform_context_switch_2" "\n"
	"\t" "   lui        $a1, (0xFF00FFFE >> 16)" "\n"
	"\t" "  ori         $a1, $a1, (0xFF00FFFE & 0xFFFF)" "\n"
	"\t" "  beq         $a0, $a1, .Lintrman_perform_context_switch_1" "\n"
	"\t" "   nop" "\n"
	"\t" "  lw          $s0, 0x40($sp)" "\n"
	"\t" "  lw          $s1, 0x44($sp)" "\n"
	"\t" "  lw          $s2, 0x48($sp)" "\n"
	"\t" "  lw          $s3, 0x4C($sp)" "\n"
	"\t" "  lw          $s4, 0x50($sp)" "\n"
	"\t" "  lw          $s5, 0x54($sp)" "\n"
	"\t" "  lw          $s6, 0x58($sp)" "\n"
	"\t" "  lw          $s7, 0x5C($sp)" "\n"
	"\t" ".Lintrman_perform_context_switch_1:" "\n"
	"\t" "  lw          $t0, 0x20($sp)" "\n"
	"\t" "  lw          $t1, 0x24($sp)" "\n"
	"\t" "  lw          $t2, 0x28($sp)" "\n"
	"\t" "  lw          $t3, 0x2C($sp)" "\n"
	"\t" "  lw          $t4, 0x30($sp)" "\n"
	"\t" "  lw          $t5, 0x34($sp)" "\n"
	"\t" "  lw          $t6, 0x38($sp)" "\n"
	"\t" "  lw          $t7, 0x3C($sp)" "\n"
	"\t" "  lw          $t8, 0x60($sp)" "\n"
	"\t" "  lw          $t9, 0x64($sp)" "\n"
	"\t" "  lw          $gp, 0x70($sp)" "\n"
	"\t" "  lw          $fp, 0x78($sp)" "\n"
	"\t" ".Lintrman_perform_context_switch_2:" "\n"
#ifndef BUILDING_INTRMANP
	"\t" "  lw          $v1, 0x90($sp)" "\n"
	"\t" "  lui         $v0, (0xBF801078 >> 16)" "\n"
	"\t" "  ori         $v0, $v0, (0xBF801078 & 0xFFFF)" "\n"
	"\t" "  sw          $v1, 0x0($v0)" "\n"
#endif
	"\t" "  lw          $v0, 0x80($sp)" "\n"
	"\t" "  lw          $v1, 0x84($sp)" "\n"
	"\t" "  mthi        $v0" "\n"
	"\t" "  mtlo        $v1" "\n"
	"\t" "  lw          $a0, 0x88($sp)" "\n"
	"\t" "  lw          $ra, 0x7C($sp)" "\n"
	"\t" "  srl         $a0, $a0, 1" "\n"
	"\t" "  sll         $a0, $a0, 1" "\n"
	"\t" "  mtc0        $a0, $12" "\n"
	"\t" "  lw          $at, 0x4($sp)" "\n"
	"\t" "  lw          $v0, 0x8($sp)" "\n"
	"\t" "  lw          $v1, 0xC($sp)" "\n"
	"\t" "  lw          $a0, 0x10($sp)" "\n"
	"\t" "  lw          $a1, 0x14($sp)" "\n"
	"\t" "  lw          $a2, 0x18($sp)" "\n"
	"\t" "  lw          $a3, 0x1C($sp)" "\n"
	"\t" ".Lintrman_perform_context_switch_3:" "\n"
	"\t" "  lw          $k0, 0x8C($sp)" "\n"
	"\t" "  lw          $sp, 0x74($sp)" "\n"
	"\t" "  jr          $k0" "\n"
	"\t" "   .word      0x42000010" "\n" // cop0 0x10 # return from exception
	"\t" "  nop" "\n"
	"\t" ".Lintrman_perform_context_switch_4:" "\n"
#ifndef BUILDING_INTRMANP
	"\t" "  lw          $v1, 0x90($sp)" "\n"
	"\t" "  lui         $v0, (0xBF801078 >> 16)" "\n"
	"\t" "  ori         $v0, $v0, (0xBF801078 & 0xFFFF)" "\n"
	"\t" "  sw          $v1, 0x0($v0)" "\n"
#endif
	"\t" "  lw          $s0, 0x40($sp)" "\n"
	"\t" "  lw          $s1, 0x44($sp)" "\n"
	"\t" "  lw          $s2, 0x48($sp)" "\n"
	"\t" "  lw          $s3, 0x4C($sp)" "\n"
	"\t" "  lw          $s4, 0x50($sp)" "\n"
	"\t" "  lw          $s5, 0x54($sp)" "\n"
	"\t" "  lw          $s6, 0x58($sp)" "\n"
	"\t" "  lw          $s7, 0x5C($sp)" "\n"
	"\t" "  lw          $v0, 0x8($sp)" "\n"
	"\t" "  lw          $v1, 0xC($sp)" "\n"
	"\t" "  lw          $gp, 0x70($sp)" "\n"
	"\t" "  lw          $fp, 0x78($sp)" "\n"
	"\t" "  lw          $a0, 0x88($sp)" "\n"
	"\t" "  lw          $ra, 0x7C($sp)" "\n"
	"\t" "  srl         $a0, $a0, 1" "\n"
	"\t" "  sll         $a0, $a0, 1" "\n"
	"\t" "  mtc0        $a0, $12" "\n"
	"\t" "  b           .Lintrman_perform_context_switch_3" "\n"
	"\t" "   nop" "\n"
	"\t" ".set pop" "\n"
);
// clang-format on

static void *new_context_stub_cb(void *ctx)
{
	return ctx;
}

static int preempt_stub_cb(int unk)
{
	(void)unk;

	return 0;
}

// clang-format off
__asm__ (
	"\t" ".set push" "\n"
	"\t" ".set noat" "\n"
	"\t" ".set noreorder" "\n"
	"\t" "exception_priority_interrupt_handler:" "\n"
	"\t" "  .word 0" "\n"
	"\t" "  .word 0" "\n"

	"\t" "exception_priority_interrupt_handler_code:" "\n"
	"\t" "  lw          $k0, 0x408($zero)" "\n"
	"\t" "  lw          $at, 0x400($zero)" "\n"
	"\t" "  mtc0        $k0, $12" "\n"
	"\t" "  lw          $k0, 0x404($zero)" "\n"
	"\t" "  nop" "\n"
	"\t" "  jr          $k0" "\n"
	"\t" "   .word      0x42000010" "\n" // cop0 0x10 # return from exception
	"\t" ".set pop" "\n"
);

__asm__ (
	"\t" ".set push" "\n"
	"\t" ".set noat" "\n"
	"\t" ".set noreorder" "\n"
	"\t" "exception_system_handler:" "\n"
	"\t" "  .word 0" "\n"
	"\t" "  .word 0" "\n"

	"\t" "exception_system_handler_code:" "\n"
	"\t" "  andi        $k0, $v0, 0x3" "\n"
	"\t" "  beqz        $k0, syscall_handler_dispatch" "\n"

	"\t" "syscall_handler_callnext:" "\n"
	"\t" "  lui         $k0, %hi(exception_system_handler)" "\n"
	"\t" "  lw          $k0, %lo(exception_system_handler)($k0)" "\n"
	"\t" "  nop" "\n"
	"\t" "  jr          $k0" "\n"
	"\t" "   nop" "\n"

	"\t" "syscall_handler_dispatch:" "\n"
	"\t" "  lui         $k0, %hi(syscall_dsp)" "\n"
	"\t" "  addiu       $k0, $k0, %lo(syscall_dsp)" "\n"
	"\t" "  addu        $k0, $k0, $v0" "\n"
	"\t" "  lw          $k0, 0x0($k0)" "\n"
	"\t" "  nop" "\n"
	"\t" "  jr          $k0" "\n"
	"\t" "   nop" "\n"

	"\t" "syscall_handler_0C_CpuInvokeInKmode:" "\n"
	"\t" "  lw          $t0, 0x404($zero)" "\n"
	"\t" "  lw          $t1, 0x408($zero)" "\n"
	"\t" "  addiu       $sp, $sp, -0x1C" "\n"
	"\t" "  sw          $ra, 0x10($sp)" "\n"
	"\t" "  sw          $t0, 0x14($sp)" "\n"
	"\t" "  sw          $t1, 0x18($sp)" "\n"
	"\t" "  addu        $t0, $a0, $zero" "\n"
	"\t" "  addu        $a0, $a1, $zero" "\n"
	"\t" "  addu        $a1, $a2, $zero" "\n"
	"\t" "  addu        $a2, $a3, $zero" "\n"
	"\t" "  jalr        $t0" "\n"
	"\t" "   nop" "\n"
	"\t" "  lw          $ra, 0x10($sp)" "\n"
	"\t" "  lw          $t0, 0x14($sp)" "\n"
	"\t" "  lw          $t1, 0x18($sp)" "\n"
	"\t" "  addiu       $sp, $sp, 0x1C" "\n"
	"\t" "  mtc0        $t1, $12" "\n"
	"\t" "  addiu       $t0, $t0, 0x4" "\n"
	"\t" "  jr          $t0" "\n"
	"\t" "   .word      0x42000010" "\n" // cop0 0x10 # return from exception

	"\t" "syscall_handler_10:" "\n"
	"\t" "  lw          $t0, 0x408($zero)" "\n"
	"\t" "  addiu       $t1, $zero, 0x414" "\n"
	"\t" "  and         $v0, $t0, $t1" "\n"
	"\t" "  addiu       $at, $zero, -0x405" "\n"
	"\t" "  b           do_set_status_register" "\n"
	"\t" "   and        $t0, $t0, $at" "\n"

	"\t" "syscall_handler_14:" "\n"
	"\t" "  lw          $t0, 0x408($zero)" "\n"
	"\t" "  addiu       $t1, $zero, -0x415" "\n"
	"\t" "  and         $t0, $t0, $t1" "\n"
	"\t" "  b           do_set_status_register" "\n"
	"\t" "   or         $t0, $t0, $a0" "\n"

	"\t" "syscall_handler_08_CpuEnableIntr:" "\n"
	"\t" "  lw          $t0, 0x408($zero)" "\n"
	"\t" "  b           do_set_status_register" "\n"
	"\t" "   ori        $t0, $t0, 0x404" "\n"

	"\t" "syscall_handler_04_CpuDisableIntr:" "\n"
	"\t" "  lw          $t0, 0x408($zero)" "\n"
	"\t" "  addiu       $t1, $zero, 0x404" "\n"
	"\t" "  and         $v0, $t0, $t1" "\n"
	"\t" "  beq         $v0, $t1, 9f" "\n"
	"\t" "   nop" "\n"
	"\t" "  addiu       $v0, $zero, 0x0" "\n"
	"\t" "9:" "\n"
	"\t" "  addiu       $at, $zero, -0x405" "\n"
	"\t" "  and         $t0, $t0, $at" "\n"
	"\t" "do_set_status_register:" "\n"
	"\t" "  mtc0        $t0, $12" "\n" // $12

	"\t" "syscall_handler_00_return_from_exception:" "\n"
	"\t" "  lw          $a0, 0x404($zero)" "\n"
	"\t" "  nop" "\n"
	"\t" "  addiu       $a0, $a0, 0x4" "\n"
	"\t" "  jr          $a0" "\n"
	"\t" "   .word      0x42000010" "\n" // cop0 0x10 # return from exception

	"\t" "syscall_handler_20_threadman:" "\n"
	"\t" "  addiu       $sp, $sp, -0x98" "\n"
#ifndef BUILDING_INTRMANP
	"\t" "  sw          $a2, 0x90($sp)" "\n"
#endif
	"\t" "  lw          $t0, 0x408($zero)" "\n"
#ifdef BUILDING_INTRMANP
	"\t" "  addiu       $t1, $zero, -0x415" "\n"
	"\t" "  and         $t0, $t0, $t1" "\n"
	"\t" "  or          $t0, $t0, $a2" "\n"
#endif
	"\t" "  lw          $t1, 0x404($zero)" "\n"
	"\t" "  sw          $t0, 0x88($sp)" "\n"
	"\t" "  addiu       $t1, $t1, 0x4" "\n"
	"\t" "  sw          $t1, 0x8C($sp)" "\n"
	"\t" "  sw          $s0, 0x40($sp)" "\n"
	"\t" "  sw          $s1, 0x44($sp)" "\n"
	"\t" "  sw          $s2, 0x48($sp)" "\n"
	"\t" "  sw          $s3, 0x4C($sp)" "\n"
	"\t" "  sw          $s4, 0x50($sp)" "\n"
	"\t" "  sw          $s5, 0x54($sp)" "\n"
	"\t" "  sw          $s6, 0x58($sp)" "\n"
	"\t" "  sw          $s7, 0x5C($sp)" "\n"
	"\t" "  sw          $a0, 0x8($sp)" "\n"
	"\t" "  sw          $a1, 0xC($sp)" "\n"
	"\t" "  sw          $gp, 0x70($sp)" "\n"
	"\t" "  sw          $fp, 0x78($sp)" "\n"
	"\t" "  sw          $ra, 0x7C($sp)" "\n"
	"\t" "  addiu       $t0, $sp, 0x98" "\n"
	"\t" "  sw          $t0, 0x74($sp)" "\n"
	"\t" "  lui         $t0, (0xF0FF000C >> 16)" "\n"
	"\t" "  ori         $t0, $t0, (0xF0FF000C & 0xFFFF)" "\n"
	"\t" "  sw          $t0, 0x0($sp)" "\n"
	"\t" "  addu        $a0, $sp, $zero" "\n"
	"\t" "  addiu       $sp, $sp, -0x10" "\n"
	"\t" "  j           intrman_perform_context_switch" "\n"
	"\t" "   nop" "\n"
	"\t" ".set pop" "\n"
);
// clang-format on

extern u32 syscall_handler_00_return_from_exception[];
extern u32 syscall_handler_04_CpuDisableIntr[];
extern u32 syscall_handler_08_CpuEnableIntr[];
extern u32 syscall_handler_0C_CpuInvokeInKmode[];
extern u32 syscall_handler_10[];
extern u32 syscall_handler_14[];
extern u32 syscall_handler_callnext[];
extern u32 syscall_handler_20_threadman[];

void *syscall_dsp[] = {
	&syscall_handler_00_return_from_exception,
	&syscall_handler_04_CpuDisableIntr,
	&syscall_handler_08_CpuEnableIntr,
	&syscall_handler_0C_CpuInvokeInKmode,
	&syscall_handler_10,
	&syscall_handler_14,
	&syscall_handler_callnext,
	&syscall_handler_callnext,
	&syscall_handler_20_threadman,
	&syscall_handler_callnext,
	&syscall_handler_callnext,
	&syscall_handler_callnext,
	&syscall_handler_callnext,
	&syscall_handler_callnext,
	&syscall_handler_callnext,
	&syscall_handler_callnext,
};

/*
# _____     ___ ____     ___ ____
#  ____|   |    ____|   |        | |____|
# |     ___|   |____ ___|    ____| |    \    PS2DEV Open Source Project.
#-----------------------------------------------------------------------
# Copyright (c) 2003 Marcus R. Brown <mrbrown@0xd6.org>
# Licenced under Academic Free License version 2.0
# Review ps2sdk README & LICENSE files for further details.
#
# $Id$
# SBUS interrupt handlers for IOP.
*/

#include "types.h"
#include "defs.h"
#include "irx.h"

#include "loadcore.h"
#include "intrman.h"
#include "sifman.h"
#include "sbusintr.h"

IRX_ID("sbusintr", 1, 1);

static struct {
	sbus_intr_handler_t handler;
	void	*arg;
} sbus_handler_table[32];

struct irx_export_table _exp_sbusintr;

int _start(int argc, const char **argv)
{
	if (RegisterLibraryEntries(&_exp_sbusintr) != 0)
		return 1;

	return 0;
}

static int sbus_dispatch(void *arg)
{
	u32 msflag, irq;

	if (!(msflag = sceSifGetMSFlag()))
		return 1;

	for (irq = 0; msflag != 0 && irq < 32; irq++, msflag >>= 1) {
		if (!(msflag & 1))
			continue;

		/* "Acknowledge" the interrupt */
		sceSifSetMSFlag(1 << irq);

		if (sbus_handler_table[irq].handler)
			sbus_handler_table[irq].handler(irq,
					sbus_handler_table[irq].arg);
	}

	return 1;
}

int sbus_intr_handler_add(u32 irq, sbus_intr_handler_t handler, void *arg)
{
	int res = irq, state = 0;

	/* I hope this correct... */
	CpuSuspendIntr(&state);

	if (irq > 31) {
		res = -SBUS_E_ARG;
		goto out;
	}

	if (sbus_handler_table[irq].handler) {
		res = -SBUS_E_IRQ;
		goto out;
	}

	sbus_handler_table[irq].handler = handler;
	sbus_handler_table[irq].arg     = arg;
out:
	CpuResumeIntr(state);
	return res;
}

int sbus_intr_handler_del(u32 irq)
{
	if (irq > 31)
		return -SBUS_E_ARG;

	sbus_handler_table[irq].handler = NULL;
	sbus_handler_table[irq].arg     = NULL;

	return irq;
}

void sbus_intr_main_interrupt(u32 irq)
{
	u32 flag = 1 << irq;

	if (irq > 31 || (sceSifGetSMFlag() & flag))
		return;

	sceSifSetSMFlag(flag);
	sceSifIntrMain();
}

static int initialized = 0;

/*
 * - Register SBUS handler
 * - Clear handler table
 */
int sbus_intr_init()
{
	int i, state, res = 0;

	if (initialized)
		return res;

	CpuSuspendIntr(&state);

	for (i = 0; i < 32; ++i) {
		sbus_handler_table[i].handler = NULL;
		sbus_handler_table[i].arg     = NULL;
	}

	if (RegisterIntrHandler(IOP_IRQ_SBUS, 1, sbus_dispatch, NULL) < 0) {
		res = -SBUS_E_INIT;
		goto out;
	}
	EnableIntr(IOP_IRQ_SBUS);
	initialized = 1;
out:
	CpuResumeIntr(state);
	return res;
}

void sbus_intr_exit()
{
	DisableIntr(IOP_IRQ_SBUS, NULL);
	ReleaseIntrHandler(IOP_IRQ_SBUS);
	initialized = 0;
}

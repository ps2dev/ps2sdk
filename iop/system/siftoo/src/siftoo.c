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
# SIFToo low-level driver.
*/

/*
 * The SBUS Interface (SIF) has 3 DMA channels, with the first 2 reserved
 * for SCEI's SIF RPC interface.  As far as I know, the last channel is
 * only used for debugging within the EE's kernel (I haven't seen it in use
 * on the IOP yet, I guess they decided to leave those modules out of the
 * BIOS).  So on consumer units SIF DMA channel 2 (SIF2) is unused.
 *
 * SIF2 is a bidirectional channel, which means data can be transferred in
 * either direction on the SBUS.  The SIFToo driver provides an interface
 * for IOP drivers to communicate via the EE using this channel.
 */

#include "types.h"
#include "defs.h"
#include "irx.h"

#include "loadcore.h"
#include "intrman.h"
#include "sifman.h"
#include "sbusintr.h"
#include "siftoo.h"
#include "stdio.h"

IRX_ID("SIFToo_driver", 1, 1);

#define SIF2_SBUS_IRQ	0

int sif2_control(u32 intr, void *unused);

struct irx_export_table _exp_siftoo;

int _start(int argc, char **argv)
{
	u32 state;

	if (RegisterLibraryEntries(&_exp_siftoo) != 0)
		return 1;

	/* Add our SBUS interrupt handler.  */
	if (sbus_intr_init() < 0) {
		printf("Unable to initialize SBUS interrupt driver, exiting.\n");
		return 1;
	}

	if (sbus_intr_handler_add(SIF2_SBUS_IRQ, sif2_control, NULL) < 0) {
		printf("Unable to register SIFToo Control handler, exiting.\n");
		return 1;
	}

	CpuSuspendIntr((int *)&state);
	sceSifDma2Init();
	CpuResumeIntr(state);

	return 0;
}

int shutdown()
{
	return 0;
}

int sif2_init()
{
	/* Perform the EE handshake:
	 * - Check if the EE was already initialized, and if so send a control
	 *   message with our recieve buffer address.
	 * - If it hasn't been initialized yet, set SIF SM flag 0x80000 and
	 *   wait for an event signaling that the EE is ready.
	 */
	
	return 0;
}

int sif2_exit()
{
	return 0;
}

int sif2_control(u32 intr, void *unused)
{
	return 0;
}

int sif2_mem_read(u32 addr, void *buf, u32 size)
{
	return 0;
}

int sif2_mem_write(u32 addr, void *buf, u32 size)
{
	return 0;
}

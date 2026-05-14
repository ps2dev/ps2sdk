/*
# _____     ___ ____     ___ ____
#  ____|   |    ____|   |        | |____|
# |     ___|   |____ ___|    ____| |    \    PS2DEV Open Source Project.
#-----------------------------------------------------------------------
# Copyright ps2dev - http://www.ps2dev.org
# Licenced under Academic Free License version 2.0
# Review ps2sdk README & LICENSE files for further details.
*/

#include <debug.h>
#include <iopcontrol.h>
#include <iopcontrol_special.h>
#include <ioprpgen.h>
#include <kernel.h>
#include <malloc.h>
#include <sifrpc.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

extern unsigned char igreeting_irx[];
extern unsigned int size_igreeting_irx;

extern unsigned char threadman_irx[];
extern unsigned int size_threadman_irx;

extern unsigned char sifinit_irx[];
extern unsigned int size_sifinit_irx;

int main(int ac, char **av)
{
	struct ioprpgen_ctx ctx;
	struct ioprpgen_memwrite_ctx memwrite_ctx;
	struct ioprpgen_entry entries[3];
	int sz;
	int sz2;
	void *buf;

	(void)ac;
	(void)av;

	init_scr();
	scr_printf("Preparing IOPRP image\n");
	memset(&entries, 0, sizeof(entries));
	entries[0].m_name = "IGREETING";
	entries[0].m_data = igreeting_irx;
	entries[0].m_data_size = size_igreeting_irx;
	entries[1].m_name = "SIFINIT";
	entries[1].m_data = sifinit_irx;
	entries[1].m_data_size = size_sifinit_irx;
	// Ensure the end is NULL terminated
	entries[2].m_name = NULL;
	entries[2].m_data = NULL;
	entries[2].m_data_size = 0;
	ioprpgen_setup_membuf(&ctx, &memwrite_ctx, NULL, 0);
	sz = ioprpgen_write_ioprp(&ctx, entries);
	if ( !sz )
	{
		scr_printf("Error getting size of IOPRP image\n");
		SleepThread();
		return 0;
	}
	buf = memalign(64, sz);
	if ( !buf )
	{
		scr_printf("Error allocating memory for IOPRP image\n");
		SleepThread();
		return 0;
	}
	ioprpgen_setup_membuf(&ctx, &memwrite_ctx, buf, sz);
	sz2 = ioprpgen_write_ioprp(&ctx, entries);
	if ( sz != sz2 )
	{
		scr_printf("Error generating IOPRP image\n");
		SleepThread();
		return 0;
	}
	sceSifInitRpc(0);
	SifIopRebootBuffer(buf, sz);
	while ( !SifIopSync() )
		;
	free(buf);
	sceSifInitRpc(0);
	scr_printf("Successfully rebooted IOP with IOPRP image!\n");
	SleepThread();
	return 0;
}

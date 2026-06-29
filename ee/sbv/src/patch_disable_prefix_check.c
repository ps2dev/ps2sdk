/*
# _____     ___ ____     ___ ____
#  ____|   |    ____|   |        | |____|
# |     ___|   |____ ___|    ____| |    \    PS2DEV Open Source Project.
#-----------------------------------------------------------------------
# Copyright (c) 2003  Marcus R. Brown <mrbrown@0xd6.org>
# Licenced under Academic Free License version 2.0
# Review ps2sdk README & LICENSE files for further details.
*/

/**
 * @file
 * Disable the MODLOAD check for .elf's or .irx's on certain devices.
 */

#include <tamtypes.h>
#include <kernel.h>
#include <string.h>

#include "slib.h"

extern slib_exp_lib_list_t _slib_cur_exp_lib_list;

static const u32 g_patch[2] = {
	0x03e00008,	/* jr $ra */
	0x00001021	/* addiu $v0, $0, 0 */
};

int sbv_patch_disable_prefix_check(void)
{
	union {
		u8 buf[256];
		slib_exp_lib_t exp_lib;
	} buf;
	static u32 patch[sizeof(g_patch)/sizeof(g_patch[0])] ALIGNED(16);
	SifDmaTransfer_t dmat;
	slib_exp_lib_t *modload_lib = &buf.exp_lib;

	memcpy(UNCACHED_SEG(patch), g_patch, sizeof(g_patch));
	memset(&_slib_cur_exp_lib_list, 0, sizeof(slib_exp_lib_list_t));

	if (!slib_get_exp_lib("modload", modload_lib))
		return -1;

	dmat.src=patch;
	dmat.size=sizeof(patch);	//16-bytes will be written to IOP RAM, but the function on the IOP side is longer than 16 bytes in length.
	dmat.dest=modload_lib->exports[15];
	dmat.attr=0;

	sceSifSetDma(&dmat, 1);

	return 0;
}

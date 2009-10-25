/*
# _____     ___ ____     ___ ____
#  ____|   |    ____|   |        | |____|
# |     ___|   |____ ___|    ____| |    \    PS2DEV Open Source Project.
#-----------------------------------------------------------------------
# Copyright (c) 2003  Marcus R. Brown <mrbrown@0xd6.org>
# Licenced under Academic Free License version 2.0
# Review ps2sdk README & LICENSE files for further details.
#
# $Id$
# Disable the MODLOAD check for .elf's or .irx's on certain devices.
*/

#include "tamtypes.h"
#include "string.h"

#include "smem.h"
#include "slib.h"

extern slib_exp_lib_list_t _slib_cur_exp_lib_list;

int sbv_patch_disable_prefix_check()
{
	u8 buf[512];
	u32 patch[2];
	slib_exp_lib_t *modload_lib = (slib_exp_lib_t *)buf;

	patch[0] = 0x03e00008;	/* jr $ra */
	patch[1] = 0x00001021;	/* addiu $v0, $0, 0 */

	memset(&_slib_cur_exp_lib_list, 0, sizeof(slib_exp_lib_list_t));

	if (!slib_get_exp_lib("modload", modload_lib))
		return -1;

	smem_write(modload_lib->exports[15], patch, sizeof patch);
	return 0;
}

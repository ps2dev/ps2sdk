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
# Sub CPU library interface.
*/

#include "tamtypes.h"
#include "string.h"

#include "smem.h"
#include "slib.h"

slib_exp_lib_list_t _slib_cur_exp_lib_list = {0, 0};

#define SEARCH_SIZE	(16 * 1024)
static u8 smem_buf[SEARCH_SIZE];

/**
 * slib_exp_lib_list - Find the head and tail of the export library list.
 *
 * Returns NULL if the list couldn't be found, and the address of the head and
 * tail pointers if it was located.
 *
 * This routine will need to be called everytime the IOP is reset.
 */
slib_exp_lib_list_t *slib_exp_lib_list()
{
	slib_exp_lib_t *core_exps;
	u32 *exp_func, *core_info;
	u8 *smem_loc;	/* Current location into our IOP mem buffer.  */
	slib_exp_lib_list_t *exp_lib_list = 0;
	u32 i, addr, core_end, gmt_ofs = 0x800;

	/* Read 16k of IOP RAM from the start of the global module table -
	   this is where we will search.  */
	smem_read((void *)gmt_ofs, smem_buf, SEARCH_SIZE);

	/* The first entry points to LOADCORE's module info.  We then use the
	   module info to determine the end of LOADCORE's .text segment (just
	   past the export library we're trying to find.  */
	core_info = (u32 *)(smem_buf + (*(u32 *)smem_buf - gmt_ofs));
	core_end = core_info[6] + core_info[7];  /* 6 and 7 are .text start and end.  */

	/* Back up so we position ourselves infront of where the export
	   library will be.  */
	smem_loc = smem_buf + core_end - gmt_ofs - 512;
	
	/* Search for LOADCORE's export library.  */
	for (i = 0; i < 512; i += 4) {
		/* SYSMEM's export library sits at 0x830, so it should appear in
		   LOADCORE's prev pointer.  */
		if (*(u32 *)(smem_loc + i) == 0x830) {
			if (!memcmp(smem_loc + i + 12, "loadcore", 8))
				break;
		}
	}
	if (i >= 512)
		return 0;

	/* Get to the start of the export table, and find the address of the
	   routine that will get us the export library list info.  */
	core_exps = (slib_exp_lib_t *)(smem_loc + i);
	exp_func = (u32 *)(smem_buf + (u32)core_exps->exports[3] - gmt_ofs);

	/* Parse the two instructions that hold the address of the table.  */
	if ((exp_func[0] & 0xffff0000) != 0x3c020000)	/* lui v0, XXXX */
		return 0;
	if ((exp_func[1] & 0xffff0000) != 0x24420000)	/* addiu v0, v0, XXXX */
		return 0;

	addr = (exp_func[0] & 0xffff) << 16;
	addr |= exp_func[1] & 0xffff;
	addr -= gmt_ofs;
	_slib_cur_exp_lib_list.tail = (slib_exp_lib_t *)(*(u32 *)(smem_buf + addr));
	_slib_cur_exp_lib_list.head = (slib_exp_lib_t *)(*(u32 *)(smem_buf + addr + 4));
	exp_lib_list = &_slib_cur_exp_lib_list;

	return exp_lib_list;
}

/**
 * slib_get_exp_lib - Retrieve an export library by name.
 */
int slib_get_exp_lib(const char *name, slib_exp_lib_t *library)
{
	u8 buf[0x300];	/* We can even handle CDVDMAN's bloat!  */
	slib_exp_lib_list_t *exp_lib_list = &_slib_cur_exp_lib_list;
	slib_exp_lib_t *exp_lib = (slib_exp_lib_t *)buf;
	void *cur_lib;
	int len = strlen(name), count = 0;

	if (!exp_lib_list->head && !(exp_lib_list = slib_exp_lib_list()))
		return 0;

	/* Read the tail export library to initiate the search.  */
	cur_lib = exp_lib_list->tail;

	while (cur_lib) {
		smem_read(cur_lib, exp_lib, sizeof buf);

		if (!memcmp(exp_lib->name, name, len)) {
			while (exp_lib->exports[count] != 0)
				count++;

			if (library)
				memcpy(library, exp_lib, sizeof(slib_exp_lib_t) + count * 4);

			return count;
		}

		cur_lib = exp_lib->prev;
	}

	return 0;
}

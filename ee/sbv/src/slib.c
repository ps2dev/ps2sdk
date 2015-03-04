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

#include <tamtypes.h>
#include <kernel.h>
#include <string.h>
#include <sifrpc.h>

#include "slib.h"
#include "smod.h"

slib_exp_lib_list_t _slib_cur_exp_lib_list = {NULL, NULL};

/* from common.c */
extern u8 smem_buf[];
extern int __memcmp(const void *s1, const void *s2, unsigned int length);

/**
 * slib_exp_lib_list - Find the head and tail of the export library list.
 *
 * Returns NULL if the list couldn't be found, and the address of the head and
 * tail pointers if it was located.
 *
 * This routine will need to be called everytime the IOP is reset.
 */
slib_exp_lib_list_t *slib_exp_lib_list(void)
{
	SifRpcReceiveData_t RData;
	slib_exp_lib_t *core_exps;
	slib_exp_lib_list_t *exp_lib_list = NULL;
	u32 i, addr, core_end, NextMod, *exp_func;
	void *pGetLoadcoreInternalData;
	smod_mod_info_t *ModInfo;

	/* Read the start of the global module table - this is where we will search.  */
	SyncDCache(smem_buf, smem_buf+sizeof(smod_mod_info_t));
	if(sceSifRpcGetOtherData(&RData, (void*)0x800, smem_buf, sizeof(smod_mod_info_t), 0)>=0){
		/* The first entry points to LOADCORE's module info.  We then use the
		   module info to determine the end of LOADCORE's .text segment (just
		   past the export library we're trying to find.  */
		NextMod = *(u32 *)smem_buf;
		SyncDCache(smem_buf, smem_buf+sizeof(smod_mod_info_t));
		if(sceSifRpcGetOtherData(&RData, (void*)NextMod, smem_buf, sizeof(smod_mod_info_t), 0)>=0){
			ModInfo = (smod_mod_info_t *)smem_buf;
			core_end = ModInfo->text_start+ModInfo->text_size;

			/* Back up so we position ourselves infront of where the export
			   library will be.  */
			SyncDCache(smem_buf, smem_buf+512);
			if(sceSifRpcGetOtherData(&RData, (void*)(core_end - 512), smem_buf, 512, 0)>=0){
				/* Search for LOADCORE's export library.  */
				for (i = 0; i < 512; i += 4) {
					/* SYSMEM's export library sits at 0x830, so it should appear in
					   LOADCORE's prev pointer.  */
					if (*(u32 *)(smem_buf + i) == 0x830) {
						if (!__memcmp(smem_buf + i + 12, "loadcore", 8))
							break;
					}
				}
				if (i >= 512)
					return NULL;

				/* Get to the start of the export table, and find the address of the
				   routine that will get us the export library list info.  */
				core_exps = (slib_exp_lib_t *)(smem_buf + i);
				pGetLoadcoreInternalData = core_exps->exports[3];

				SyncDCache(smem_buf, smem_buf+8);
				if(sceSifRpcGetOtherData(&RData, pGetLoadcoreInternalData, smem_buf, 8, 0)>=0){
					exp_func = (u32*)smem_buf;

					/* Parse the two instructions that hold the address of the table.  */
					if ((exp_func[0] & 0xffff0000) != 0x3c020000)	/* lui v0, XXXX */
						return NULL;
					if ((exp_func[1] & 0xffff0000) != 0x24420000)	/* addiu v0, v0, XXXX */
						return NULL;

					addr = ((exp_func[0] & 0xffff) << 16) | (exp_func[1] & 0xffff);

					SyncDCache(smem_buf, smem_buf+8);
					if(sceSifRpcGetOtherData(&RData, (void*)addr, smem_buf, 8, 0)>=0){
						_slib_cur_exp_lib_list.tail = (slib_exp_lib_t *)(*(u32 *)(smem_buf));
						_slib_cur_exp_lib_list.head = (slib_exp_lib_t *)(*(u32 *)(smem_buf + 4));
						exp_lib_list = &_slib_cur_exp_lib_list;
					}
				}
			}
		}
	}

	return exp_lib_list;
}

#define EXP_LIB_MAX	0x300	/* We can even handle CDVDMAN's bloat!  */

/**
 * slib_get_exp_lib - Retrieve an export library by name.
 */
int slib_get_exp_lib(const char *name, slib_exp_lib_t *library)
{
	SifRpcReceiveData_t RData;
	slib_exp_lib_list_t *exp_lib_list = &_slib_cur_exp_lib_list;
	slib_exp_lib_t *exp_lib = (slib_exp_lib_t *)smem_buf;
	void *cur_lib;
	int len = strlen(name), count = 0;

	if (!exp_lib_list->head && !(exp_lib_list = slib_exp_lib_list()))
		return 0;

	/* Read the tail export library to initiate the search.  */
	cur_lib = exp_lib_list->tail;

	while (cur_lib) {
		SyncDCache(smem_buf, smem_buf+EXP_LIB_MAX);
		if(sceSifRpcGetOtherData(&RData, cur_lib, exp_lib, EXP_LIB_MAX, 0)>=0){
			if (!__memcmp(exp_lib->name, name, len)) {
				while (exp_lib->exports[count] != 0)
					count++;

				if (library)
					memcpy(library, exp_lib, sizeof(slib_exp_lib_t) + count * 4);

				return count;
			}

			cur_lib = exp_lib->prev;
		}
	}

	return 0;
}

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
 * Sub CPU library interface.
 */

#include <tamtypes.h>
#include <kernel.h>
#include <string.h>
#include <sifrpc.h>

#include "slib.h"
#include "smod.h"

#include "common.h"

slib_exp_lib_list_t _slib_cur_exp_lib_list = {NULL, NULL};

/* from common.c */
extern struct smem_buf smem_buf;

slib_exp_lib_list_t *slib_exp_lib_list(void)
{
	SifRpcReceiveData_t RData;
	slib_exp_lib_t *core_exps;
	slib_exp_lib_list_t *exp_lib_list = NULL;
	u32 i, addr, core_end, NextMod;
	smod_mod_info_t *ModInfo;

	/* Read the start of the global module table - this is where we will search.  */
	SyncDCache(&smem_buf, smem_buf.bytes+sizeof(smod_mod_info_t));
	if(SifRpcGetOtherData(&RData, (void*)0x800, &smem_buf, sizeof(smod_mod_info_t), 0)>=0){
		/* The first entry points to LOADCORE's module info.  We then use the
		   module info to determine the end of LOADCORE's .text segment (just
		   past the export library we're trying to find.  */
		NextMod = *smem_buf.words;
		SyncDCache(&smem_buf, smem_buf.bytes+sizeof(smod_mod_info_t));
		if(SifRpcGetOtherData(&RData, (void*)NextMod, &smem_buf, sizeof(smod_mod_info_t), 0)>=0){
			ModInfo = &smem_buf.mod_info;
			core_end = ModInfo->text_start+ModInfo->text_size;

			/* Back up so we position ourselves infront of where the export
			   library will be.  */
			SyncDCache(&smem_buf, smem_buf.bytes+512);
			if(SifRpcGetOtherData(&RData, (void*)(core_end - 512), &smem_buf, 512, 0)>=0){
				void *pGetLoadcoreInternalData;

				/* Search for LOADCORE's export library.  */
				for (i = 0; i < 512; i += 4) {
					/* SYSMEM's export library sits at 0x830, so it should appear in
					   LOADCORE's prev pointer.  */
					if (smem_buf.words[i / sizeof(u32)] == 0x830) {
						if (!__memcmp(smem_buf.bytes + i + 12, "loadcore", 8))
							break;
					}
				}
				if (i >= 512)
					return NULL;

				/* Get to the start of the export table, and find the address of the
				   routine that will get us the export library list info.  */
				core_exps = (slib_exp_lib_t *)(smem_buf.bytes + i);
				pGetLoadcoreInternalData = core_exps->exports[3];

				SyncDCache(&smem_buf, smem_buf.bytes+8);
				if(SifRpcGetOtherData(&RData, pGetLoadcoreInternalData, &smem_buf, 8, 0)>=0){
					u32 *exp_func;
					exp_func = smem_buf.words;

					/* Parse the two instructions that hold the address of the table.  */
					if ((exp_func[0] & 0xffff0000) != 0x3c020000)	/* lui v0, XXXX */
						return NULL;
					if ((exp_func[1] & 0xffff0000) != 0x24420000)	/* addiu v0, v0, XXXX */
						return NULL;

					addr = (u32) ((exp_func[0] & 0xFFFF) << 16) + (s16) (exp_func[1] & 0xFFFF);

					SyncDCache(&smem_buf, smem_buf.bytes+8);
					if(SifRpcGetOtherData(&RData, (void*)addr, &smem_buf, 8, 0)>=0){
						_slib_cur_exp_lib_list.tail = (slib_exp_lib_t *)(smem_buf.words[0]);
						_slib_cur_exp_lib_list.head = (slib_exp_lib_t *)(smem_buf.words[1]);
						exp_lib_list = &_slib_cur_exp_lib_list;
					}
				}
			}
		}
	}

	return exp_lib_list;
}

#define EXP_LIB_MAX	SMEM_BUF_SIZE	/* We can even handle CDVDMAN's bloat!  */

int slib_get_exp_lib(const char *name, slib_exp_lib_t *library)
{
	SifRpcReceiveData_t RData;
	slib_exp_lib_list_t *exp_lib_list = &_slib_cur_exp_lib_list;
	slib_exp_lib_t *exp_lib = &smem_buf.exp_lib;
	void *cur_lib;
	int len = strlen(name), count = 0;

	if (!exp_lib_list->head && !(exp_lib_list = slib_exp_lib_list()))
		return 0;

	/* Read the tail export library to initiate the search.  */
	cur_lib = exp_lib_list->tail;

	while (cur_lib) {
		SyncDCache(&smem_buf, smem_buf.bytes+EXP_LIB_MAX);
		if(SifRpcGetOtherData(&RData, cur_lib, exp_lib, EXP_LIB_MAX, 0)>=0){
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

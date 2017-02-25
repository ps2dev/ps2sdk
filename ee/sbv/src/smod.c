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
 * Sub-CPU module interface.
 */

#include <tamtypes.h>
#include <kernel.h>
#include <string.h>
#include <sifrpc.h>

#include "smem.h"
#include "slib.h"
#include "smod.h"

#include "common.h"

/* from common.c */
extern struct smem_buf smem_buf;

int smod_get_next_mod(smod_mod_info_t *cur_mod, smod_mod_info_t *next_mod)
{
	SifRpcReceiveData_t RData;
	void *addr;

	/* If cur_mod is 0, return the head of the list (typically IOP address 0x800).  */
	if (!cur_mod) {
		addr = (void *)0x800;
	} else {
		if (!cur_mod->next)
			return 0;
		else
			addr = cur_mod->next;
	}

	SyncDCache(&smem_buf, smem_buf.bytes+sizeof(smod_mod_info_t));
	if(SifRpcGetOtherData(&RData, addr, &smem_buf, sizeof(smod_mod_info_t), 0)>=0){
		memcpy(next_mod, &smem_buf.mod_info, sizeof(smod_mod_info_t));
		return next_mod->id;
	}

	return 0;
}

int smod_get_mod_by_name(const char *name, smod_mod_info_t *info)
{
	SifRpcReceiveData_t RData;
	int len = strlen(name);

	if (!smod_get_next_mod(NULL, info))
		return 0;

	smem_buf.bytes[64]='\0';
	do {
		SyncDCache(&smem_buf, smem_buf.bytes+64);
		if(SifRpcGetOtherData(&RData, info->name, &smem_buf, 64, 0)>=0){
			if (!__memcmp(smem_buf.bytes, name, len))
				return info->id;
		}
	} while (smod_get_next_mod(info, info) != 0);

	return 0;
}

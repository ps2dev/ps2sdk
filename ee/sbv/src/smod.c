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
# Sub-CPU module interface.
*/

#include "tamtypes.h"
#include "string.h"

#include "smem.h"
#include "smod.h"

/**
 * smod_get_next_mod - Return the next module referenced in the global module list.
 */
int smod_get_next_mod(smod_mod_info_t *cur_mod, smod_mod_info_t *next_mod)
{
	void *addr;

	/* If cur_mod is 0, return the head of the list (IOP address 0x800).  */
	if (!cur_mod) {
		addr = (void *)0x800;
	} else {
		if (!cur_mod->next)
			return 0;
		else
			addr = cur_mod->next;
	}

	smem_read(addr, next_mod, sizeof(smod_mod_info_t));
	return next_mod->id;
}

/**
 * smod_get_mod_by_name - Find and retreive a module by it's module name.
 */
int smod_get_mod_by_name(const char *name, smod_mod_info_t *info)
{
	u8 search_name[60];
	int len = strlen(name);

	if (!smod_get_next_mod(0, info))
		return 0;

	do {
		smem_read(info->name, search_name, sizeof search_name);

		if (!memcmp(search_name, name, len))
			return info->id;
	} while (smod_get_next_mod(info, info) != 0);

	return 0;
}

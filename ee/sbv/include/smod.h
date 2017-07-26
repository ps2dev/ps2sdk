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

#ifndef __SMOD_H__
#define __SMOD_H__

#include <tamtypes.h>

/** Module info entry.  
 * Most of the fields are self-explanatory.  I don't know what the *flags fields do, and they don't seem to be important.
 */
typedef struct _smod_mod_info {
	struct _smod_mod_info *next;
	/** A pointer to the name in IOP RAM, this must be smem_read().  */
	char	*name;
	u16	version;
	/** For MODLOAD shipped with games. The old MODLOAD module from boot ROMs do not use a flags field.  */
	u16	newflags;
	u16	id;
	u16	unused;
	/** _start */
	u32	entry;
	u32	gp;
	u32	text_start;
	u32	text_size;
	u32	data_size;
	u32	bss_size;
	u32	unused1;
	u32	unused2;
} smod_mod_info_t;

#ifdef __cplusplus
extern "C" {
#endif

/** Return the next module referenced in the global module list.  */
int smod_get_next_mod(smod_mod_info_t *cur_mod, smod_mod_info_t *next_mod);

/** Find and retreive a module by it's module name.  */
int smod_get_mod_by_name(const char *name, smod_mod_info_t *info);

#ifdef __cplusplus
}
#endif

#endif /* __SMOD_H__ */

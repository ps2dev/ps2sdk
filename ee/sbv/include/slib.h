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
 * Sub-CPU library interface.
 */

#ifndef __SLIB_H__
#define __SLIB_H__

#include <tamtypes.h>

/** Describes an IRX import list.  */
typedef struct _slib_imp_list {
	u8	magic;
	struct _slib_imp_list *next;
	u16	version;
	u16	flags;
	u8	name[8];
	void	*imports[];
} slib_imp_list_t;

/** Describes an IRX export library.  */
typedef struct _slib_exp_lib {
	struct _slib_exp_lib *prev;
	struct _slib_imp_list *caller;
	u16	version;
	u16	flags;
	u8	name[8];
	void	*exports[];
} slib_exp_lib_t;

/**
 * The LOADCORE library has a routine that returns a pointer to the following
 * structure.  It keeps track of all libraries added to the system with RegisterLibraryEntries().
 */
typedef struct _slib_exp_lib_list {
	struct _slib_exp_lib *tail;
	struct _slib_exp_lib *head;
} slib_exp_lib_list_t;

#ifdef __cplusplus
extern "C" {
#endif

/** Find the head and tail of the export library list.
 *
 * @return NULL if the list couldn't be found, and the address of the head and tail pointers if it was located.
 *
 * This routine will need to be called everytime the IOP is reset.
 */
slib_exp_lib_list_t *slib_exp_lib_list();

/** Retrieve an export library by name.  */
int slib_get_exp_lib(const char *name, slib_exp_lib_t *library);

#ifdef __cplusplus
}
#endif

#endif /* __SLIB_H__ */

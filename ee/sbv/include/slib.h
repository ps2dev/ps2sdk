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
# Sub-CPU library interface.
*/

#ifndef SBV_SLIB_H
#define SBV_SLIB_H

#include "tamtypes.h"

#ifdef __cplusplus
extern "C" {
#endif

/* Describes an IRX import list.  */
typedef struct _slib_imp_list {
	u8	magic;
	struct _slib_imp_list *next;
	u16	version;
	u16	flags;
	u8	name[8];
	void	*imports[0];
} slib_imp_list_t;

/* Describes an IRX export library.  */
typedef struct _slib_exp_lib {
	struct _slib_exp_lib *prev;
	struct _slib_imp_list *caller;
	u16	version;
	u16	flags;
	u8	name[8];
	void	*exports[0];
} slib_exp_lib_t;

/* The LOADCORE library has a routine that returns a pointer to the following
   structure.  It keeps track of all libraries added to the system with
   RegisterLibraryEntries().  */
typedef struct _slib_exp_lib_list {
	struct _slib_exp_lib *tail;
	struct _slib_exp_lib *head;
} slib_exp_lib_list_t;

/* Find the head and tail of the export library list.  */
slib_exp_lib_list_t *slib_exp_lib_list();

/* Retrieve an export library by name.  */
int slib_get_exp_lib(const char *name, slib_exp_lib_t *library);

#ifdef __cplusplus
}
#endif

#endif /* SBV_SLIB_H */

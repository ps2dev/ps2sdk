/*
# _____     ___ ____     ___ ____
#  ____|   |    ____|   |        | |____|
# |     ___|   |____ ___|    ____| |    \    PS2DEV Open Source Project.
#-----------------------------------------------------------------------
# Copyright 2001-2004, ps2dev - http://www.ps2dev.org
# Licenced under Academic Free License version 2.0
# Review ps2sdk README & LICENSE files for further details.
#
# $Id$
# Kernel module loader.
*/

#ifndef IOP_LOADCORE_H
#define IOP_LOADCORE_H

#include "types.h"
#include "irx.h"

#define MODULE_RESIDENT_END		0
#define MODULE_NO_RESIDENT_END	1

typedef struct _iop_library {
	struct	_iop_library *prev;
	struct	irx_import_table *caller;
	u16	version;
	u16	flags;
	u8	name[8];
	void	*exports[0];
} iop_library_t;

typedef struct {
	struct	_iop_library *tail;
	struct	_iop_library *head;
} iop_library_table_t;

#define loadcore_IMPORTS_start DECLARE_IMPORT_TABLE(loadcore, 1, 1)
#define loadcore_IMPORTS_end END_IMPORT_TABLE

iop_library_table_t *GetLibraryEntryTable();
#define I_GetLibraryEntryTable DECLARE_IMPORT(3, GetLibraryEntryTable)

void FlushIcache();
#define I_FlushIcache DECLARE_IMPORT(4, FlushIcache)
void FlushDcache();
#define I_FlushDcache DECLARE_IMPORT(5, FlushDcache)

int RegisterLibraryEntries(struct irx_export_table *exports);
#define I_RegisterLibraryEntries DECLARE_IMPORT(6, RegisterLibraryEntries)
int ReleaseLibraryEntries(struct irx_export_table *exports);
#define I_ReleaseLibraryEntries DECLARE_IMPORT(7, ReleaseLibraryEntries);

void *QueryLibraryEntryTable(iop_library_t *library);
#define I_QueryLibraryEntryTable DECLARE_IMPORT(11, QueryLibraryEntryTable);
int * QueryBootMode(int mode);
#define I_QueryBootMode DECLARE_IMPORT(12, QueryBootMode)

#endif /* IOP_LOADCORE_H */

/*
# _____     ___ ____     ___ ____
#  ____|   |    ____|   |        | |____|
# |     ___|   |____ ___|    ____| |    \    PS2DEV Open Source Project.
#-----------------------------------------------------------------------
# Copyright (c) 2003 Marcus R. Brown <mrbrown@0xd6.org>
# Licenced under Academic Free License version 2.0
# Review ps2sdk README & LICENSE files for further details.
*/

/**
 * @file
 * Kernel module loader.
 */

#ifndef __LOADCORE_H__
#define __LOADCORE_H__

#include <types.h>
#include <irx.h>

#define MODULE_RESIDENT_END		0
#define MODULE_NO_RESIDENT_END	1

/** Module info entry. Taken from iopmgr. */
typedef struct _ModuleInfo {
	struct _ModuleInfo *next;
	char	*name;
	u16	version;
	/** For modload shipped with games. */
	u16	newflags;
	u16	id;
	/** I believe this is where flags are kept for BIOS versions. */
	u16	flags;
	/** _start */
	u32	entry;
	u32	gp;
	u32	text_start;
	u32	text_size;
	u32	data_size;
	u32	bss_size;
	u32	unused1;
	u32	unused2;
} ModuleInfo_t;

typedef struct _iop_library {
	struct	_iop_library *prev;
	struct	irx_import_table *caller;
	u16	version;
	u16	flags;
	char	name[8];
	void	*exports[0];
} iop_library_t;

/** Loadcore internal data structure. Based on the one from loadcore.c of the FPS2BIOS project from PCSX2. */
typedef struct tag_LC_internals {
	iop_library_t* let_next, *let_prev;	//let_next = tail, let_prev = head. I don't know what "let" and "mda" stand for.
	iop_library_t* mda_next, *mda_prev;
	/** Points to the module image information structure chain that usually starts at 0x800. */
	ModuleInfo_t *image_info;
	int	module_count;
	int	module_index;
} lc_internals_t;

typedef struct {
	u16	value;
	u8	id;
	u8	len;
	u32	data[0];
} iop_bootmode_t;

/** 
 * For backward-compatibility with older projects. Read the comment on GetLibraryEntryTable(). This structure is incomplete. 
 */
typedef struct {
	struct	_iop_library *tail;
	struct	_iop_library *head;
} iop_library_table_t __attribute__ ((deprecated));

/**
 * Retained for backward-compatibility with older projects. This function actually returns a pointer to LOADCORE's internal data structure.
 */
iop_library_table_t *GetLibraryEntryTable(void) __attribute__ ((deprecated));
lc_internals_t *GetLoadcoreInternalData(void);

void FlushIcache(void);
void FlushDcache(void);

int RegisterLibraryEntries(struct irx_export_table *exports);
int ReleaseLibraryEntries(struct irx_export_table *exports);

int RegisterNonAutoLinkEntries(struct irx_export_table *exports);

void *QueryLibraryEntryTable(iop_library_t *library);
int * QueryBootMode(int mode);
void RegisterBootMode(iop_bootmode_t *b);

#define loadcore_IMPORTS \
	loadcore_IMPORTS_start \
 \
 	I_GetLibraryEntryTable \
 \
 	I_FlushIcache \
	I_FlushDcache \
 \
 	I_RegisterLibraryEntries \
	I_ReleaseLibraryEntries \
 \
 	I_QueryLibraryEntryTable \
	I_QueryBootMode \
 \
	loadcore_IMPORTS_end

#define loadcore_IMPORTS_start DECLARE_IMPORT_TABLE(loadcore, 1, 1)
#define loadcore_IMPORTS_end END_IMPORT_TABLE

#define I_GetLibraryEntryTable DECLARE_IMPORT(3, GetLibraryEntryTable)
#define I_GetLoadcoreInternalData DECLARE_IMPORT(3, GetLoadcoreInternalData)
#define I_FlushIcache DECLARE_IMPORT(4, FlushIcache)
#define I_FlushDcache DECLARE_IMPORT(5, FlushDcache)
#define I_RegisterLibraryEntries DECLARE_IMPORT(6, RegisterLibraryEntries)
#define I_ReleaseLibraryEntries DECLARE_IMPORT(7, ReleaseLibraryEntries)
#define I_RegisterNonAutoLinkEntries DECLARE_IMPORT(10, RegisterNonAutoLinkEntries)
#define I_QueryLibraryEntryTable DECLARE_IMPORT(11, QueryLibraryEntryTable)
#define I_QueryBootMode DECLARE_IMPORT(12, QueryBootMode)
#define I_RegisterBootMode DECLARE_IMPORT(13, RegisterBootMode)

#endif /* __LOADCORE_H__ */

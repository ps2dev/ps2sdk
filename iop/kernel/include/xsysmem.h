/*
# _____     ___ ____     ___ ____
#  ____|   |    ____|   |        | |____|
# |     ___|   |____ ___|    ____| |    \    PS2DEV Open Source Project.
#-----------------------------------------------------------------------
# Copyright 2001-2004, ps2dev - http://www.ps2dev.org
# Licenced under Academic Free License version 2.0
# Review ps2sdk README & LICENSE files for further details.
*/

/**
 * @file
 * Additional sysmem functions only found in newer IOPRP images
 */

#ifndef __XSYSMEM_H__
#define __XSYSMEM_H__

#include <sysmem.h>

#ifdef __cplusplus
extern "C" {
#endif

#define xsysmem_IMPORTS_start DECLARE_IMPORT_TABLE(sysmem, 1, 2)
#define xsysmem_IMPORTS_end END_IMPORT_TABLE

typedef struct sysmem_meminfo_
{
	int allocation_count;
	int memsize;
	sysmem_alloc_table_t *memlist_last;
	sysmem_alloc_table_t *memlist_first;
} sysmem_meminfo_t;

typedef struct sysmem_blockinfo_
{
	void *block_address;
	/* Low 8 bits are flags. The rest is the size of this block */
	/* flags_memsize == 0: block not found in passed in list */
	/* flags_memsize & 1: passed in list is empty */
	/* flags_memsize & 2: block is allocated */
	u32 flags_memsize;
	void *unused08;
	sysmem_alloc_table_t *table_info;
} sysmem_blockinfo_t;

typedef union sysmem_info_
{
	sysmem_blockinfo_t blockinfo; /* when flag of GetSysMemoryInfo is zero */
	sysmem_meminfo_t meminfo; /* when flag of GetSysMemoryInfo is nonzero */
} sysmem_info_t;

void GetSysMemoryInfo(int flag, sysmem_info_t *info);

#define I_GetSysMemoryInfo DECLARE_IMPORT(11, GetSysMemoryInfo)

#ifdef __cplusplus
}
#endif

#endif /* __XSYSMEM_H__ */

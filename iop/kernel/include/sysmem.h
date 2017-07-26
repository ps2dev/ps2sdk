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
 * System memory manager.
 */

#ifndef __SYSMEM_H__
#define __SYSMEM_H__

#include <types.h>
#include <irx.h>
#include <stdarg.h>

// Allocation strategies
#define ALLOC_FIRST	0
#define ALLOC_LAST	1
#define ALLOC_ADDRESS	2

// see QueryBlockTopAddress, QueryBlockSize
#define USED	0x00000000
#define FREE	0x80000000

void * AllocSysMemory(int mode, int size, void *ptr);

int FreeSysMemory(void *ptr);

u32 QueryMemSize();
u32 QueryMaxFreeMemSize();
u32 QueryTotalFreeMemSize();
void * QueryBlockTopAddress(void *address);
int QueryBlockSize(void *address);

typedef int (KprintfHandler_t)(void *context, const char *format, va_list ap);

int Kprintf(const char *format,...);
void KprintfSet(KprintfHandler_t *, void *context);

#define sysmem_IMPORTS_start DECLARE_IMPORT_TABLE(sysmem, 1, 1)
#define sysmem_IMPORTS_end END_IMPORT_TABLE

#define I_AllocSysMemory DECLARE_IMPORT(4, AllocSysMemory)
#define I_FreeSysMemory DECLARE_IMPORT(5, FreeSysMemory)
#define I_QueryMemSize DECLARE_IMPORT(6, QueryMemSize)
#define I_QueryMaxFreeMemSize DECLARE_IMPORT(7, QueryMaxFreeMemSize)
#define I_QueryTotalFreeMemSize DECLARE_IMPORT(8, QueryTotalFreeMemSize)
#define I_QueryBlockTopAddress DECLARE_IMPORT(9, QueryBlockTopAddress)
#define I_QueryBlockSize DECLARE_IMPORT(10, QueryBlockSize)
#define I_Kprintf DECLARE_IMPORT(14, Kprintf)
#define I_KprintfSet DECLARE_IMPORT(15, KprintfSet)

#endif /* IOP_SYSMEM_H */

/*
 * sysmem.h - System memory manager.
 *
 * Copyright (c) 2003 Marcus R. Brown <mrbrown@0xd6.org>
 *
 * This file is based off of the work [RO]man, Herben, and any others involved
 * in the "modules" project at http://ps2dev.pgamers.com/.
 *
 * See the file LICENSE included with this distribution for licensing terms.
 */

#ifndef IOP_SYSMEM_H
#define IOP_SYSMEM_H

#include "types.h"
#include "irx.h"

// Allocation strategies
#define ALLOC_FIRST	0
#define ALLOC_LAST	1
#define ALLOC_LATER	2

// see QueryBlockTopAddress, QueryBlockSize
#define USED	0x00000000
#define FREE	0x80000000

#define sysmem_IMPORTS_start DECLARE_IMPORT_TABLE(sysmem, 1, 1)
#define sysmem_IMPORTS_end END_IMPORT_TABLE

void * AllocSysMemory(int mode, int size, void *ptr);
#define I_AllocSysMemory DECLARE_IMPORT(4, AllocSysMemory)

int FreeSysMemory(void *ptr);
#define I_FreeSysMemory DECLARE_IMPORT(5, FreeSysMemory)

u32 QueryMemSize();
#define I_QueryMemSize DECLARE_IMPORT(6, QueryMemSize)
u32 QueryMaxFreeMemSize();
#define I_QueryMaxFreeMemSize DECLARE_IMPORT(7, QueryMaxFreeMemSize)
u32 QueryTotalFreeMemSize();
#define I_QueryTotalFreeMemSize DECLARE_IMPORT(8, QueryTotalFreeMemSize)
void * QueryBlockTopAddress(void *address);
#define I_QueryBlockTopAddress DECLARE_IMPORT(9, QueryBlockTopAddress)
int QueryBlockSize(void *address);
#define I_QueryBlockSize DECLARE_IMPORT(10, QueryBlockSize)

char * Kprintf(const char *format,...);
#define I_Kprintf DECLARE_IMPORT(14, Kprintf)
void Kprintf_set(char* (*newKprintf)(unsigned int unk, const char *, ...),
		unsigned int newunk);
#define I_Kprintf_set DECLARE_IMPORT(15, Kprintf_set)

#endif /* IOP_SYSMEM_H */

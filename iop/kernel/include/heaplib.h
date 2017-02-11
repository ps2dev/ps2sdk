/*
# _____     ___ ____     ___ ____
#  ____|   |    ____|   |        | |____|
# |     ___|   |____ ___|    ____| |    \    PS2DEV Open Source Project.
#-----------------------------------------------------------------------
# Copyright 2001-2004, ps2dev - http://www.ps2dev.org
# Licenced under Academic Free License version 2.0
# Review ps2sdk README & LICENSE files for further details.
#
# Heap library.
*/

#ifndef IOP_HEAPLIB_H
#define IOP_HEAPLIB_H

#include "types.h"
#include "irx.h"

#define heaplib_IMPORTS_start DECLARE_IMPORT_TABLE(heaplib, 1, 1)
#define heaplib_IMPORTS_end END_IMPORT_TABLE

void *CreateHeap(int heapblocksize, int flag);
#define I_CreateHeap DECLARE_IMPORT(4, CreateHeap)
void DeleteHeap(void *heap);
#define I_DeleteHeap DECLARE_IMPORT(5, DeleteHeap)
void *AllocHeapMemory(void *heap, size_t nbytes);
#define I_AllocHeapMemory DECLARE_IMPORT(6, AllocHeapMemory)
int FreeHeapMemory(void *heap, void *ptr);
#define I_FreeHeapMemory DECLARE_IMPORT(7, FreeHeapMemory)
int HeapTotalFreeSize(void *heap);
#define I_HeapTotalFreeSize DECLARE_IMPORT(8, HeapTotalFreeSize)

#endif /* IOP_HEAPLIB_H */

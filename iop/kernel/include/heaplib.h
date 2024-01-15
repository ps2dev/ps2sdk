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
 * Heap library.
 */

#ifndef __HEAPLIB_H__
#define __HEAPLIB_H__

#include <types.h>
#include <irx.h>

#ifdef __cplusplus
extern "C" {
#endif

void *CreateHeap(int heapblocksize, int flag);
void DeleteHeap(void *heap);
void *AllocHeapMemory(void *heap, size_t nbytes);
int FreeHeapMemory(void *heap, void *ptr);
int HeapTotalFreeSize(void *heap);

void HeapPrepare(void* mem, int size);
int HeapChunkSize(void* chunk);

#define heaplib_IMPORTS_start DECLARE_IMPORT_TABLE(heaplib, 1, 1)
#define heaplib_IMPORTS_end END_IMPORT_TABLE

#define I_CreateHeap DECLARE_IMPORT(4, CreateHeap)
#define I_DeleteHeap DECLARE_IMPORT(5, DeleteHeap)
#define I_AllocHeapMemory DECLARE_IMPORT(6, AllocHeapMemory)
#define I_FreeHeapMemory DECLARE_IMPORT(7, FreeHeapMemory)
#define I_HeapTotalFreeSize DECLARE_IMPORT(8, HeapTotalFreeSize)
#define I_HeapPrepare DECLARE_IMPORT(11, HeapPrepare)
#define I_HeapChunkSize DECLARE_IMPORT(15, HeapChunkSize)

#ifdef __cplusplus
}
#endif

#endif /* __HEAPLIB_H__ */

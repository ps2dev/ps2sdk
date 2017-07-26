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
 * Basic alloc library
 */

#ifndef __ALLOC_H__
#define __ALLOC_H__

#include <types.h>
#include <irx.h>

void * malloc(size_t size);
void * realloc(void * ptr, size_t size);
void free(void * ptr);
void * calloc(size_t n, size_t size);
void * memalign(size_t align, size_t size);
void * __mem_walk_begin();
void __mem_walk_read(void * token, u32 * size, void ** ptr, int * valid);
void * __mem_walk_inc(void * token);
int __mem_walk_end(void * token);

#define alloc_IMPORTS_start DECLARE_IMPORT_TABLE(alloc, 1, 1)
#define alloc_IMPORTS_end END_IMPORT_TABLE

#define I_malloc DECLARE_IMPORT(4, malloc)
#define I_realloc DECLARE_IMPORT(5, realloc)
#define I_free DECLARE_IMPORT(6, free)
#define I_calloc DECLARE_IMPORT(7, calloc)
#define I_memalign DECLARE_IMPORT(8, memalign)
#define I___mem_walk_begin DECLARE_IMPORT(9, __mem_walk_begin)
#define I___mem_walk_read DECLARE_IMPORT(10, __mem_walk_read)
#define I___mem_walk_inc DECLARE_IMPORT(11, __mem_walk_inc)
#define I___mem_walk_end DECLARE_IMPORT(12, __mem_walk_end)

#endif /* __ALLOC_H__ */

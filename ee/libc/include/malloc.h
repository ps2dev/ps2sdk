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
# EE memory allocation prototypes
*/

#ifndef _MALLOC_H
#define _MALLOC_H

#include <stddef.h>
#include <stdarg.h>

#ifdef __cplusplus
extern "C" {
#endif

/* stdlib/malloc */
void *	malloc(size_t size);
void *	realloc(void *ptr, size_t size);
void *	calloc(size_t n, size_t size);
void *	memalign(size_t align, size_t size);
void	free(void * ptr);

/* Memory walkers. Used for debugging/profiling purposes. */
void * __mem_walk_begin();
void __mem_walk_read(void * token, u32 * size, void ** ptr, int * valid);
void * __mem_walk_inc(void * token);
int __mem_walk_end(void * token);

/* Example of use:

  void * i;
  
  for (i = __mem_walk_begin(); !__mem_walk_end(i); i = __mem_walk_inc(i)) {
      u32 block_size;
      void * block_ptr;
      int valid;

      __mem_walk_read(i, &block_size, &block_ptr, &valid);
      if (!valid) {
          fprintf(stderr, "Block at token %p is invalid.\n", i);
	  break;
      }
      printf("Block at token %p points at a memory block of %i bytes at %p.\n", i, block_size, block_ptr);
  }
  
  note that 'valid' will be always true if DEBUG_ALLOC was not defined when alloc.c got compiled.

*/


/* You should never need to use this normally.  */
void *	ps2_sbrk(size_t incr);

#ifdef __cplusplus
}
#endif

#endif	// _MALLOC_H


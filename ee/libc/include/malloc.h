/*      
  _____     ___ ____ 
   ____|   |    ____|      PSX2 OpenSource Project
  |     ___|   |____       
  ------------------------------------------------------------------------
  malloc.h
			EE memory allocation prototypes
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

/* You should never need to use this normally.  */
void *	ps2_sbrk(size_t incr);

#ifdef __cplusplus
}
#endif

#endif	// _MALLOC_H


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

/* You should never need to use this normally.  */
void *	ps2_sbrk(size_t incr);

#ifdef __cplusplus
}
#endif

#endif	// _MALLOC_H


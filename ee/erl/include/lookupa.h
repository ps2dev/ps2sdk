/*
------------------------------------------------------------------------------
By Bob Jenkins, September 1996.
lookupa.h, a hash function for table lookup, same function as lookup.c.
Use this code in any way you wish.  Public Domain.  It has no warranty.
Source is http://burtleburtle.net/bob/c/lookupa.h
------------------------------------------------------------------------------
*/

#ifndef LOOKUPA_H
#define LOOKUPA_H

#include "standard.h"

#ifdef __cplusplus
extern "C" {
#endif

#define CHECKSTATE 8
#define hashsize(n) ((size_t)1<<(n))
#define hashmask(n) (hashsize(n)-1)

extern uint32_t lookup(const char *k, size_t length, uint32_t level);
extern void checksum(const char *k, size_t length, uint32_t *state);

#ifdef __cplusplus
}
#endif

#endif /* LOOKUPA */

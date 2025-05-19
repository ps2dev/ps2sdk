/*
------------------------------------------------------------------------------
Standard definitions and types, Bob Jenkins
------------------------------------------------------------------------------
*/
#ifndef STANDARD_H
#define STANDARD_H

#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>

#define UB8MAXVAL 0xffffffffffffffffLL
#define UB8BITS 64
#define SB8MAXVAL 0x7fffffffffffffffLL
#define UB4MAXVAL 0xffffffff
#define UB4BITS 32
#define SB4MAXVAL 0x7fffffff
#define UB2MAXVAL 0xffff
#define UB2BITS 16
#define SB2MAXVAL 0x7fff
#define UB1MAXVAL 0xff
#define UB1BITS 8
#define SB1MAXVAL 0x7f

#define align(a) (((uint32_t)a+(sizeof(void *)-1))&(~(sizeof(void *)-1)))

#endif /* STANDARD_H */

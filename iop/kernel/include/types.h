/*
 * types.h - Standard type definitions.
 *
 * Copyright (c) 2003 Marcus R. Brown <mrbrown@0xd6.org>
 *
 * See the file LICENSE included with this distribution for licensing terms.
 */

#ifndef IOP_TYPES_H
#define IOP_TYPES_H

/* Pull in some standard types and definitions.  */
#include <stddef.h>

typedef __signed__ char __s8;
typedef unsigned char __u8;
#if !defined(s8) && !defined(u8)
#define s8 __s8
#define u8 __u8
#endif

typedef __signed__ short __s16;
typedef unsigned short __u16;
#if !defined(s16) && !defined(u16)
#define s16 __s16
#define u16 __u16
#endif

typedef __signed__ int __s32;
typedef unsigned int __u32;
#if !defined(s32) && !defined(u32)
#define s32 __s32
#define u32 __u32
#endif

typedef __signed__ long long __s64;
typedef unsigned long long __u64;
#if !defined(s64) && !defined(u64)
#define s64 __s64
#define u64 __u64
#endif

#endif /* IOP_TYPES_H */

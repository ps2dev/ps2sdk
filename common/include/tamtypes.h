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
 * Commonly used typedefs
 */

#ifndef __TAMTYPES_H__
#define __TAMTYPES_H__

#if !defined(_EE) && !defined(_IOP)
#error Either _EE or _IOP must be defined!
#endif

typedef unsigned char u8;
typedef unsigned short u16;

typedef volatile u8 vu8;
typedef volatile u16 vu16;

#ifdef _EE
typedef unsigned int u32;
#if __GNUC__ > 3
typedef unsigned long long u64;
#else
typedef unsigned long u64;
#endif
typedef unsigned int u128 __attribute__((mode(TI)));

typedef volatile u32 vu32;
typedef volatile u64 vu64;
typedef volatile u128 vu128 __attribute__((mode(TI)));
#endif

#ifdef _IOP
typedef unsigned long u32;
typedef unsigned long long u64;

typedef volatile u32 vu32;
typedef volatile u64 vu64;
#endif

typedef signed char s8;
typedef signed short s16;

typedef volatile s8 vs8;
typedef volatile s16 vs16;

#ifdef _EE
typedef signed int s32;
#if __GNUC__ > 3
typedef signed long long s64;
#else
typedef signed long s64;
#endif
typedef signed int s128 __attribute__((mode(TI)));

typedef volatile s32 vs32;
typedef volatile s64 vs64;
typedef volatile s128 vs128 __attribute__((mode(TI)));
#endif

#ifdef _IOP
typedef signed long s32;
typedef signed long long s64;

typedef volatile s32 vs32;
typedef volatile s64 vs64;
#endif

/* Pointers are 32-bit on both EE and IOP. */
typedef u32 uiptr;
typedef s32 siptr;

typedef volatile u32 vuiptr;
typedef volatile s32 vsiptr;

#ifdef _EE
typedef union
{
    u128 qw;
    u8 b[16];
    u16 hw[8];
    u32 sw[4];
    u64 dw[2];
} qword_t;

#endif

#ifndef NULL
#define NULL (void *)0
#endif

static inline u8 _lb(u32 addr)
{
    return *(vu8 *)addr;
}
static inline u16 _lh(u32 addr) { return *(vu16 *)addr; }
static inline u32 _lw(u32 addr) { return *(vu32 *)addr; }

static inline void _sb(u8 val, u32 addr) { *(vu8 *)addr = val; }
static inline void _sh(u16 val, u32 addr) { *(vu16 *)addr = val; }
static inline void _sw(u32 val, u32 addr) { *(vu32 *)addr = val; }

#ifdef _EE
static inline u64 _ld(u32 addr)
{
    return *(vu64 *)addr;
}
static inline u128 _lq(u32 addr) { return *(vu128 *)addr; }
static inline void _sd(u64 val, u32 addr) { *(vu64 *)addr = val; }
static inline void _sq(u128 val, u32 addr) { *(vu128 *)addr = val; }
#endif

#endif /* __TAMTYPES_H__ */

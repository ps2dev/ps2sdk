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
# Common used typedef
*/

#ifndef _TAMTYPES_H_
#define _TAMTYPES_H_ 1

typedef	unsigned char 		u8;
typedef unsigned short 		u16;
typedef unsigned int 		u32;
typedef unsigned long int	u64;

#ifdef _EE
typedef unsigned int		u128 __attribute__(( mode(TI) ));
#endif

typedef signed char 		s8;
typedef signed short 		s16;
typedef	signed int 			s32;
typedef signed long int		s64;

#ifdef _EE
typedef int			s128 __attribute__(( mode(TI) ));
#endif

#ifndef NULL
#define NULL	(void *)0
#endif

static inline u8  _lb(u32 addr) { return *(volatile u8 *)addr; }
static inline u16 _lh(u32 addr) { return *(volatile u16 *)addr; }
static inline u32 _lw(u32 addr) { return *(volatile u32 *)addr; }
static inline u64 _ld(u32 addr) { return *(volatile u64 *)addr; }

static inline void _sb(u8 val, u32 addr) { *(volatile u8 *)addr = val; }
static inline void _sh(u16 val, u32 addr) { *(volatile u16 *)addr = val; }
static inline void _sw(u32 val, u32 addr) { *(volatile u32 *)addr = val; }
static inline void _sd(u64 val, u32 addr) { *(volatile u64 *)addr = val; }

#ifdef _EE
static inline u128 _lq(u32 addr) { return *(volatile u128 *)addr; }
static inline void _sq(u128 val, u32 addr) { *(volatile u128 *)addr = val; }
#endif

#endif

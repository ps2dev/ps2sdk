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

typedef	volatile unsigned char 		vu8;
typedef volatile unsigned short 	vu16;

#ifdef _EE
typedef unsigned int		u32;
typedef unsigned long int	u64;
typedef unsigned int		u128 __attribute__(( mode(TI) ));

typedef volatile unsigned int		vu32;
typedef volatile unsigned long int	vu64;
typedef volatile unsigned int		vu128 __attribute__(( mode(TI) ));
#else
typedef unsigned long int	u32;
typedef unsigned long long	u64;

typedef volatile unsigned long int	vu32;
typedef volatile unsigned long long	vu64;
#endif

typedef signed char 		s8;
typedef signed short 		s16;

typedef volatile signed char	vs8;
typedef volatile signed short	vs16;

#ifdef _EE
typedef signed int		s32;
typedef signed long int		s64;
typedef signed int		s128 __attribute__(( mode(TI) ));

typedef volatile signed int		vs32;
typedef volatile signed long int	vs64;
typedef volatile signed int		vs128 __attribute__(( mode(TI) ));
#else
typedef signed long int		s32;
typedef signed long long	s64;

typedef volatile signed long int	vs32;
typedef volatile signed long long	vs64;
#endif

#ifdef _EE
typedef union {
	u128 qw;
	u8   b[16];
	u16  hw[8];
	u32  sw[4];
	u64  dw[2];
} QWORD;

#endif

#ifndef NULL
#define NULL	(void *)0
#endif

static inline u8  _lb(u32 addr) { return *(vu8 *)addr; }
static inline u16 _lh(u32 addr) { return *(vu16 *)addr; }
static inline u32 _lw(u32 addr) { return *(vu32 *)addr; }
static inline u64 _ld(u32 addr) { return *(vu64 *)addr; }

static inline void _sb(u8 val, u32 addr) { *(vu8 *)addr = val; }
static inline void _sh(u16 val, u32 addr) { *(vu16 *)addr = val; }
static inline void _sw(u32 val, u32 addr) { *(vu32 *)addr = val; }
static inline void _sd(u64 val, u32 addr) { *(vu64 *)addr = val; }

#ifdef _EE
static inline u128 _lq(u32 addr) { return *(vu128 *)addr; }
static inline void _sq(u128 val, u32 addr) { *(vu128 *)addr = val; }
#endif

#endif

/*
 * iop/defs.h - IOPkit standard definitions.
 *
 * Copyright (c) 2003 Marcus R. Brown <mrbrown@0xd6.org>
 *
 * See the file LICENSE included with this distribution for licensing terms.
 */
#ifndef IOP_DEFS_H
#define IOP_DEFS_H

#include "types.h"

#ifndef NULL
#define NULL	((void *)0)
#endif

#define ALIGN(x, align)	(((x)+((align)-1))&~((align)-1))

#define PHYSADDR(a)	(((u32)(a)) & 0x1fffffff)

#define KSEG1		0xa0000000
#define KSEG1ADDR(a)	((__typeof__(a))(((u32)(a) & 0x1fffffff) | KSEG1))

extern inline u8  _lb(u32 addr) { return *(volatile u8 *)addr; }
extern inline u16 _lh(u32 addr) { return *(volatile u16 *)addr; }
extern inline u32 _lw(u32 addr) { return *(volatile u32 *)addr; }
extern inline u64 _ld(u32 addr) { return *(volatile u64 *)addr; }

extern inline void _sb(u8 val, u32 addr) { *(volatile u8 *)addr = val; }
extern inline void _sh(u16 val, u32 addr) { *(volatile u16 *)addr = val; }
extern inline void _sw(u32 val, u32 addr) { *(volatile u32 *)addr = val; }
extern inline void _sd(u64 val, u32 addr) { *(volatile u64 *)addr = val; }

static inline void *iop_memcpy(void *dest, const void *src, int size)
{
	u8 *d = (u8 *)dest, *s = (u8 *)src;

	while (size--)
		*d++ = *s++;
	return dest;
}

#endif /* IOP_DEFS_H */

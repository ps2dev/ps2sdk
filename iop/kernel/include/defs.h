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

static inline void *iop_memcpy(void *dest, const void *src, int size)
{
	u8 *d = (u8 *)dest, *s = (u8 *)src;

	while (size--)
		*d++ = *s++;
	return dest;
}

#endif /* IOP_DEFS_H */

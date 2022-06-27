/*
# _____     ___ ____     ___ ____
#  ____|   |    ____|   |        | |____|
# |     ___|   |____ ___|    ____| |    \    PS2DEV Open Source Project.
#-----------------------------------------------------------------------
# Copyright 2001-2004, ps2dev - http://www.ps2dev.org
# Licenced under Academic Free License version 2.0
# Review ps2sdk README & LICENSE files for further details.
*/

#define SYSCLIB_DISABLE_BUILTINS
#include <sysclib.h>

void *memset(void *ptr, int c, size_t size)
{
	if (ptr)
	{
		u8 *i;

		if (!c && (((((u32)ptr) & 0xff) | (u8)size) & 3) == 0)
		{
			return _wmemset((u32 *)ptr, 0, size);
		}
		for (i = ptr; size > 0; i += 1)
		{
			*i = c;
			size -= 1;
		}
	}
	return ptr;
}

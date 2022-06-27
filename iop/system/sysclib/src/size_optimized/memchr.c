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

void *memchr(const void *s, int c, size_t n)
{
	if (!s || (s32)n <= 0)
	{
		return 0;
	}
	while (1)
	{
		n -= 1;
		if (*(u8 *)s == (u8)c)
		{
			break;
		}
		s = (char *)s + 1;
		if ((s32)n <= 0)
		{
			return 0;
		}
	}
	return (void *)s;
}

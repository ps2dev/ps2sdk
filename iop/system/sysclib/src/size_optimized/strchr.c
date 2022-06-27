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

char *strchr(const char *s, int c)
{

	if (!s)
	{
		return 0;
	}
	while (1)
	{
		int v2;

		v2 = *s;
		if (v2 == (char)c)
		{
			break;
		}
		s += 1;
		if (!v2)
		{
			return 0;
		}
	}
	return (char *)s;
}

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

int strcmp(const char *p, const char *q)
{
	int result;

	if (p && q)
	{
		while (1)
		{
			int v3;
			int v4;

			v3 = *q;
			v4 = *(u8 *)p;
			q += 1;
			if (*p != v3)
			{
				break;
			}
			p += 1;
			if (!v4)
			{
				return 0;
			}
		}
		return *p - *(q - 1);
	}
	else
	{
		result = 0;
		if (p != q)
		{
			result = -1;
			if (p)
			{
				return 1;
			}
		}
	}
	return result;
}

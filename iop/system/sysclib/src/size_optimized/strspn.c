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

size_t strspn(const char *s, const char *accept)
{
	const char *v2;

	v2 = s;
	if (*s)
	{
		const char *v3;

		v3 = accept;
		do
		{
			int v4;

			v4 = *(u8 *)v3 << 24;
			if (!*v3)
			{
				break;
			}
			do
			{
				if (v4 >> 24 == *s)
				{
					break;
				}
				v4 = *(u8 *)++v3 << 24;
			}
			while (*v3);
			if (!*v3)
			{
				break;
			}
			s += 1;
			v3 = accept;
		}
		while (*s);
	}
	return s - v2;
}

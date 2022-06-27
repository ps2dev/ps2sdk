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

// non-standard function (not to be confused with the wchar_t related function)
void *_wmemset(u32 *dest, u32 c, size_t size)
{
	void *result;
	s32 v4;
	s32 v5;

	result = dest;
	v4 = (u32)size >> 2;
	if (v4 > 0)
	{
		v5 = 0;
		if ((v4 & 3) == 0)
		{
			goto LABEL_8;
		}
		do
		{
			*dest++ = c;
			v5 += 1;
		}
		while ((v4 & 3) != v5);
		if (v5 != v4)
		{
LABEL_8:
			do
			{
				*dest = c;
				dest[1] = c;
				dest[2] = c;
				dest[3] = c;
				v5 += 4;
				dest += 4;
			}
			while (v5 != v4);
		}
	}
	return result;
}

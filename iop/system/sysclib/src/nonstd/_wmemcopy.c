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
void *_wmemcopy(u32 *dest, const u32 *src, size_t size)
{
	void *result;
	s32 v4;
	s32 v5;
	u32 v6;
	u32 v7;
	u32 v8;
	u32 v9;

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
			v6 = *src++;
			*dest++ = v6;
			v5 += 1;
		}
		while ((v4 & 3) != v5);
		if (v5 != v4)
		{
LABEL_8:
			do
			{
				v7 = src[1];
				v8 = src[2];
				v9 = src[3];
				*dest = *src;
				dest[1] = v7;
				dest[2] = v8;
				dest[3] = v9;
				v5 += 4;
				src += 4;
				dest += 4;
			}
			while (v5 != v4);
		}
	}
	return result;
}

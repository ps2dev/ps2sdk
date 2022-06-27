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

char *strtok_r(char *s, const char *delim, char **lasts)
{
	char *result;
	const char *v4;
	int v5;
	int v6;
	int v7;
	char *v9;
	const char *v10;
	int v11;
	char v12;

	if (s || (s = *lasts, result = 0, *lasts))
	{
		do
		{
			v7 = (u8)*s;
			if (!*s++)
			{
				break;
			}
			v4 = delim;
			v5 = 0;
			if (*delim)
			{
				v6 = *(u8 *)delim << 24;
				while (v6 >> 24 != (char)v7)
				{
					v6 = *(u8 *)++v4 << 24;
					if (!*v4)
					{
						goto LABEL_9;
					}
				}
				v5 = 1;
			}
LABEL_9:
			;
		}
		while (v5);
		v9 = s - 1;
		if (v7)
		{
			do
			{
LABEL_18:
				v12 = *s;
				if (!*s++)
				{
					*lasts = 0;
					return v9;
				}
				v10 = delim;
			}
			while (!*delim);
			v11 = *(u8 *)delim << 24;
			while (1)
			{
				v10 += 1;
				if (v11 >> 24 == v12)
				{
					break;
				}
				v11 = *(u8 *)v10 << 24;
				if (!*v10)
				{
					goto LABEL_18;
				}
			}
			result = v9;
			*(s - 1) = 0;
			*lasts = s;
		}
		else
		{
			*lasts = 0;
			return 0;
		}
	}
	return result;
}

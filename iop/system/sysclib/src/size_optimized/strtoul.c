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
#include "sysclib_determine_base.h"

unsigned long strtoul(const char *s, char **endptr, int base)
{
	char *v3;
	unsigned int v6;
	int v7;
	int v8;
	char v9;
	int v10;
	char *v11;

	v3 = (char *)s;
	v6 = 0;
	if (!s)
	{
		return v6;
	}
	while (isspace(*v3) != 0)
	{
		v3 += 1;
	}
	if ((unsigned int)(base - 2) >= 0x23)
	{
		base = 0;
	}
	if (!base)
	{
		base = 10;
	}
	v7 = *v3;
	if (v7 != 48)
	{
		if (toupper(v7) == 79)
		{
			v3 += 1;
			base = 8;
		}
		goto LABEL_19;
	}
	v8 = *++v3;
	if (v8 == 88)
	{
		goto LABEL_15;
	}
	if (v8 < 89)
	{
		if (v8 != 66)
		{
			goto LABEL_19;
		}
		goto LABEL_16;
	}
	if (v8 == 98)
	{
LABEL_16:
		v3 += 1;
		base = 2;
		goto LABEL_19;
	}
	if (v8 == 120)
	{
LABEL_15:
		v3 += 1;
		base = 16;
	}
LABEL_19:
	while (1)
	{
		v9 = *v3++;
		v10 = sysclib_determine_base(v9);
		if (v10 >= base)
		{
			break;
		}
		v6 = v6 * base + v10;
	}
	v11 = v3 - 1;
	if (endptr)
	{
		*endptr = v11;
	}
	return v6;
}

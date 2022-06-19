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

long strtol(const char *s, char **endptr, int base)
{
	char *v3;
	int v6;
	int v7;
	int v9;
	int v10;
	char v11;
	int v12;
	char *v13;

	v3 = (char *)s;
	v6 = 1;
	v7 = 0;
	if (!s)
	{
		return 0;
	}
	while (isspace(*v3) != 0)
	{
		v3 += 1;
	}
	for (; *v3 == 45; v6 = -v6)
	{
		v3 += 1;
	}
	if ((unsigned int)(base - 2) >= 0x23)
		base = 0;
	if (!base)
		base = 10;
	v9 = *v3;
	if (v9 != 48)
	{
		if (toupper(v9) == 79)
		{
			v3 += 1;
			base = 8;
		}
		goto LABEL_22;
	}
	v10 = *++v3;
	if (v10 == 88)
	{
		goto LABEL_18;
	}
	if (v10 < 89)
	{
		if (v10 != 66)
		{
			goto LABEL_22;
		}
		goto LABEL_19;
	}
	if (v10 == 98)
	{
LABEL_19:
		v3 += 1;
		base = 2;
		goto LABEL_22;
	}
	if (v10 == 120)
	{
LABEL_18:
		v3 += 1;
		base = 16;
	}
LABEL_22:
	while (1)
	{
		v11 = *v3++;
		v12 = sysclib_determine_base(v11);
		if (v12 >= base)
		{
			break;
		}
		v7 = v7 * base + v12;
	}
	v13 = v3 - 1;
	if (endptr)
	{
		*endptr = v13;
	}
	return v7 * v6;
}

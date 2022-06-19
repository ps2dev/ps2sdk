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

int sysclib_determine_base(char a1)
{
	int v1;
	int result;

	v1 = a1;
	result = v1 - 48;
	if ((isdigit(a1)) == 0)
	{
		if ((isalpha(v1)) != 0)
		{
			return tolower(v1) - 87;
		}
		else
		{
			return 9999999;
		}
	}
	return result;
}

/*
# _____     ___ ____     ___ ____
#  ____|   |    ____|   |        | |____|
# |     ___|   |____ ___|    ____| |    \    PS2DEV Open Source Project.
#-----------------------------------------------------------------------
# Copyright 2001-2005, ps2dev - http://www.ps2dev.org
# Licenced under Academic Free License version 2.0
# Review ps2sdk README & LICENSE files for further details.
#
# $Id$
# Standard libc time functions
*/

#include <time.h>

/* According to the POSIX standard, if the process time can't be determined
 * clock should return (clock_t)-1
 */
clock_t clock(void) 
{
	/* POSIX standard require clock() to return -1 if process time
	 * cannot be determined.
	 */
	return (clock_t)-1;
}

time_t time(time_t *t)
{
	if (t != 0) {
		*t = (time_t)-1;
	}

	return (time_t)-1;
}

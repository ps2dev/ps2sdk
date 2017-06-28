/*
# _____     ___ ____     ___ ____
#  ____|   |    ____|   |        | |____|
# |     ___|   |____ ___|    ____| |    \    PS2DEV Open Source Project.
#-----------------------------------------------------------------------
# Copyright 2001-2005, ps2dev - http://www.ps2dev.org
# Licenced under Academic Free License version 2.0
# Review ps2sdk README & LICENSE files for further details.
#
# -pg example
*/

#include <stdio.h>
#include <kernel.h>
#include <sifrpc.h>

static int dummy = 0;
static int dummy2 = 0;

void nested()
{
	dummy2++;
}

void loops_10_times()
{
	if (dummy & 1)
	{
		nested();
	}

	dummy++;
}

volatile int locked = 1;

void wakeup(s32 id, u16 time, void *arg)
{
	locked = 0;
	ExitHandler();
}

void sleeping_beauty()
{
	/* sleep around 3 seconds on NTSC */
	SetAlarm(15734*3, wakeup, 0);
	while (locked);
}

int main()
{
	int i;

	SifInitRpc(0);

	for (i=0; i<10; i++)
	{
		loops_10_times();
	}

	sleeping_beauty();

	return 0;
}

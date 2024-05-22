/*
# _____     ___ ____     ___ ____
#  ____|   |    ____|   |        | |____|
# |     ___|   |____ ___|    ____| |    \    PS2DEV Open Source Project.
#-----------------------------------------------------------------------
# Copyright ps2dev - http://www.ps2dev.org
# Licenced under Academic Free License version 2.0
# Review ps2sdk README & LICENSE files for further details.
*/

#include "accore_internal.h"

static acUint8 masks_8[3] = {128u, 2u, 64u};
static struct intr_softc Intrc;

static int intr_intr(void *arg)
{
	struct intr_softc *argt;

	argt = (struct intr_softc *)arg;
	if ( argt )
	{
		int index;
		struct intr_handler *v2;
		int sr;

		index = 2;
		v2 = &argt->handlers[1];
		sr = (*((volatile acUint16 *)0xB241C000) & 0xFF00) >> 8;
		while ( index >= 0 )
		{
			if ( masks_8[index] && masks_8[index] == ((sr & masks_8[index]) & 0xFF) && v2->func )
				v2->func(v2->arg);
			--index;
			--v2;
		}
	}
	return 1;
}

int acIntrClear(acIntrNum inum)
{
	acUint16 *v2;

	if ( inum == AC_INTR_NUM_ATA )
	{
		v2 = (acUint16 *)0xB3000000;
		*v2 = 0;
	}
	if ( inum == AC_INTR_NUM_UART )
	{
		v2 = (acUint16 *)0xB3100000;
		*v2 = 0;
	}
	return 0;
}

int acIntrEnable(acIntrNum inum)
{
	acUint32 enable;

	if ( (unsigned int)inum >= (AC_INTR_NUM_LAST | AC_INTR_NUM_JV) )
		return -22;
	enable = Intrc.enable;
	Intrc.enable |= 1 << inum;
	if ( inum )
	{
		if ( inum == AC_INTR_NUM_UART )
			*((volatile acUint16 *)0xB241511E) = 0;
	}
	else
	{
		*((volatile acUint16 *)0xB241510C) = 0;
		*((volatile acUint16 *)0xB3000000) = 0;
	}
	if ( !enable )
	{
		switch ( EnableIntr(13) )
		{
			case 0:
				return 0;
			default:
				return -22;
		}
	}
	return 0;
}

int acIntrDisable(acIntrNum inum)
{
	int v1;
	acUint32 enable;
	int old_status;

	if ( (unsigned int)inum >= (AC_INTR_NUM_LAST | AC_INTR_NUM_JV) )
		return -22;
	if ( inum == AC_INTR_NUM_ATA )
	{
		*((volatile acUint16 *)0xB241510C) = 0;
	}
	if ( inum == AC_INTR_NUM_UART )
	{
		*((volatile acUint16 *)0xB241511C) = 0;
	}
	v1 = 1 << inum;
	Intrc.enable &= ~v1;
	if ( Intrc.enable )
		return 0;
	enable = DisableIntr(13, &old_status);
	if ( enable != (acUint32)-103 )
	{
		if ( !enable )
			return 0;
		return -22;
	}
	return 1;
}

int acIntrRegister(acIntrNum inum, acIntrHandler func, void *arg)
{
	char v3;
	struct intr_handler *handler;
	acUint32 active;
	acSpl state;

	v3 = inum;
	handler = &Intrc.handlers[inum];
	if ( (unsigned int)inum >= (AC_INTR_NUM_LAST | AC_INTR_NUM_JV) )
		return -22;
	if ( handler->func )
		return -11;
	CpuSuspendIntr(&state);
	active = Intrc.active;
	handler->func = func;
	handler->arg = arg;
	Intrc.active = active | (1 << v3);
	CpuResumeIntr(state);
	if ( !active )
	{
		switch ( RegisterIntrHandler(13, 1, intr_intr, Intrc.handlers) )
		{
			case -104:
				return -11;
			case -100:
				return -4;
			case 0:
				return 0;
			default:
				return -22;
		}
	}
	return 0;
}

int acIntrRelease(acIntrNum inum)
{
	char v1;
	struct intr_handler *handler;
	acUint32 active;
	acUint32 active_v5;
	acSpl state;

	v1 = inum;
	handler = &Intrc.handlers[inum];
	if ( (unsigned int)inum >= (AC_INTR_NUM_LAST | AC_INTR_NUM_JV) )
		return -22;
	if ( !handler->func )
	{
		return -2;
	}
	CpuSuspendIntr(&state);
	active = Intrc.active;
	handler->func = 0;
	handler->arg = 0;
	active_v5 = active & ~(1 << v1);
	Intrc.active = active_v5;
	CpuResumeIntr(state);
	if ( !active_v5 )
	{
		switch ( ReleaseIntrHandler(13) )
		{
			case -100:
				return -4;
			case -101:
				return -22;
			case 0:
				return 0;
			default:
				return -2;
		}
	}
	return 0;
}

int acIntrModuleStart(int argc, char **argv)
{
	int index;
	int v4;
	int state;

	(void)argc;
	(void)argv;
	if ( Intrc.active != 0 || !Intrc.enable )
	{
		return -16;
	}
	CpuSuspendIntr(&state);
	index = 2;
	v4 = 24;
	while ( index >= 0 )
	{
		acUint32 *v5;

		v5 = (acUint32 *)((char *)&Intrc.active + v4);
		v4 -= 8;
		--index;
		*v5 = 0;
		v5[1] = 0;
	}
	CpuResumeIntr(state);
	Intrc.enable = 0;
	Intrc.active = 0;
	return 0;
}

int acIntrModuleStop()
{
	int v1;

	DisableIntr(13, &v1);
	ReleaseIntrHandler(13);
	Intrc.enable = 0;
	Intrc.active = 0;
	return 0;
}

int acIntrModuleRestart(int argc, char **argv)
{
	(void)argc;
	(void)argv;

	return -88;
}

int acIntrModuleStatus()
{
	int ret;
	int state;

	CpuSuspendIntr(&state);
	if ( Intrc.enable )
		ret = 2;
	else
		ret = Intrc.active != 0;
	CpuResumeIntr(state);
	return ret;
}

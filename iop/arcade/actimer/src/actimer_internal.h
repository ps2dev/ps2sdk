/*
# _____     ___ ____     ___ ____
#  ____|   |    ____|   |        | |____|
# |     ___|   |____ ___|    ____| |    \    PS2DEV Open Source Project.
#-----------------------------------------------------------------------
# Copyright ps2dev - http://www.ps2dev.org
# Licenced under Academic Free License version 2.0
# Review ps2sdk README & LICENSE files for further details.
*/

#ifndef _ACTIMER_INTERNAL_H
#define _ACTIMER_INTERNAL_H

#include <actimer.h>
#include <irx_imports.h>

struct timer_softc
{
	acQueueHeadData waitq;
	acTime tick;
};

#endif

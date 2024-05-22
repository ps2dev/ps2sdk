/*
# _____     ___ ____     ___ ____
#  ____|   |    ____|   |        | |____|
# |     ___|   |____ ___|    ____| |    \    PS2DEV Open Source Project.
#-----------------------------------------------------------------------
# Copyright ps2dev - http://www.ps2dev.org
# Licenced under Academic Free License version 2.0
# Review ps2sdk README & LICENSE files for further details.
*/

#ifndef _ACCORE_INTERNAL_H
#define _ACCORE_INTERNAL_H

#include <accore.h>
#include <irx_imports.h>

typedef int (*acCoreInit)(int argc, char **argv);

struct dma_softc
{
	acQueueHeadData requestq;
	acInt32 status;
	acUint32 padding;
};

struct intr_handler
{
	acIntrHandler func;
	void *arg;
};

struct intr_softc
{
	acUint32 active;
	acUint32 enable;
	struct intr_handler handlers[3];
};

extern int acDev9ModuleStart(int argc, char **argv);

#endif

/*
# _____     ___ ____     ___ ____
#  ____|   |    ____|   |        | |____|
# |     ___|   |____ ___|    ____| |    \    PS2DEV Open Source Project.
#-----------------------------------------------------------------------
# Copyright ps2dev - http://www.ps2dev.org
# Licenced under Academic Free License version 2.0
# Review ps2sdk README & LICENSE files for further details.
*/

#ifndef _ACRAM_INTERNAL_H
#define _ACRAM_INTERNAL_H

#include <acram.h>
#include <irx_imports.h>

typedef volatile acUint16 *acRamReg;

struct ram_softc
{
	acDmaData dma;
	acRamAddr addr;
	acUint8 *buf;
	acInt32 size;
	acInt32 result;
	acQueueHeadData requestq;
	acInt32 thid;
	acUint32 refresh;
};

#endif

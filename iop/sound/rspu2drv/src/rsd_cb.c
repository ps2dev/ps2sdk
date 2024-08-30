/*
# _____     ___ ____     ___ ____
#  ____|   |    ____|   |        | |____|
# |     ___|   |____ ___|    ____| |    \    PS2DEV Open Source Project.
#-----------------------------------------------------------------------
# Copyright ps2dev - http://www.ps2dev.org
# Licenced under Academic Free License version 2.0
# Review ps2sdk README & LICENSE files for further details.
*/

#include "rs_i.h"

static SpuEECBData eeCBData;

static SifRpcClientData_t cd;

void sceSifCmdLoop2(void)
{
	while ( 1 )
	{
		if ( eeCBData.mode )
		{
			sceSifCallRpc(&cd, 0, 0, &eeCBData, 16, 0, 0, 0, 0);
			if ( eeCBData.mode < 4 )
				memcpy(gStPtr, &gStBuff, sizeof(SpuStEnv));
			eeCBData.mode = 0;
		}
		SleepThread();
	}
}

void DMA0CallBackProc(void)
{
	eeCBData.mode = 4;
	iWakeupThread(gStThid);
}

void DMA1CallBackProc(void)
{
	eeCBData.mode = 5;
	iWakeupThread(gStThid);
}

void IRQCallBackProc(void)
{
	eeCBData.mode = 6;
	iWakeupThread(gStThid);
}

void spustCB_preparation_finished(unsigned int voice_bit, int p_status)
{
	eeCBData.mode = 1;
	eeCBData.voice_bit = voice_bit;
	eeCBData.status = p_status;
	iWakeupThread(gStThid);
}

void spustCB_transfer_finished(unsigned int voice_bit, int t_status)
{
	eeCBData.mode = 2;
	eeCBData.voice_bit = voice_bit;
	eeCBData.status = t_status;
	iWakeupThread(gStThid);
}

void spustCB_stream_finished(unsigned int voice_bit, int s_status)
{
	eeCBData.mode = 3;
	eeCBData.voice_bit = voice_bit;
	eeCBData.status = s_status;
	iWakeupThread(gStThid);
}

void sce_spust_loop(void *userdata)
{
	int i;

	(void)userdata;

	eeCBData.mode = 0;
	while ( sceSifBindRpc(&cd, sce_SPUST_CB, 0) >= 0 )
	{
		for ( i = 0; i < 10000; i += 1 )
		{
			__asm__ __volatile__("" : "+g"(i) : :);
		}
		if ( cd.server )
			sceSifCmdLoop2();
	}
	while ( 1 )
		;
}

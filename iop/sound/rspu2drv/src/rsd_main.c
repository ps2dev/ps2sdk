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

IRX_ID("rspu2_driver", 1, 3);

int _start(int ac, char **av)
{
	const int *BootMode;
	iop_thread_t param;
	int th;

	(void)ac;
	(void)av;
	FlushDcache();
	BootMode = QueryBootMode(3);
	if ( BootMode && (BootMode[1] & 2) != 0 )
		return MODULE_NO_RESIDENT_END;
	CpuEnableIntr();
	param.attr = TH_C;
	param.thread = create_th;
	param.priority = 32;
#ifdef LIB_OSD_100
	param.stacksize = 512;
#else
	param.stacksize = 1024;
#endif
	param.option = 0;
	th = CreateThread(&param);
	if ( th <= 0 )
		return MODULE_NO_RESIDENT_END;
	StartThread(th, 0);
	return MODULE_RESIDENT_END;
}

void create_th(void *userdata)
{
	iop_thread_t param;
	int th;

	(void)userdata;

	if ( !sceSifCheckInit() )
		sceSifInit();
	sceSifInitRpc(0);
	param.attr = TH_C;
	param.thread = sce_spu2_loop;
	param.priority = 34;
#ifdef LIB_OSD_100
	param.stacksize = 2048;
#else
	param.stacksize = 4096;
#endif
	param.option = 0;
	th = CreateThread(&param);
	StartThread(th, 0);
#if 0
	param.attr = TH_C;
	param.thread = sce_osd_spu2_loop;
	param.priority = 35;
#ifdef LIB_OSD_100
	param.stacksize = 4096;
#else
	param.stacksize = 0x2000;
#endif
	param.option = 0;
	gSth = CreateThread(&param);
	StartThread(gSth, 0);
#endif
	SleepThread();
}

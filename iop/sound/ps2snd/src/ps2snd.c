/*
# _____     ___ ____     ___ ____
#  ____|   |    ____|   |        | |____|
# |     ___|   |____ ___|    ____| |    \    PS2DEV Open Source Project.
#-----------------------------------------------------------------------
# Copyright 2005, James Lee (jbit<at>jbit<dot>net)
# Licenced under Academic Free License version 2.0
# Review ps2sdk README & LICENSE files for further details.
#
# $Id: $
*/


#include <types.h>
#include <irx.h>
#include <sifrpc.h>
#include <loadcore.h>
#include <stdio.h>
#include <libsd.h>
#include <thbase.h>
#include <intrman.h>
#include <sysmem.h>
#include <ioman.h>
#include <thsemap.h>
#include <ps2snd.h>
#include "mod.h"
IRX_ID(MODNAME,VER_MAJOR,VER_MINOR)
int debug_level = OUT_WARNING;

static SifRpcDataQueue_t  queue;
static SifRpcServerData_t server;

static u32 rpc_buffer[2][32] ALIGNED(16); /* XXX: how big should this be? */


#define DS ((u32*)data)
#define DU ((u32*)data)


void *rpc_server(u32 func, void *data, u32 size)
{
	u32 *ru = rpc_buffer[1];
	s32  *rs = rpc_buffer[1];
	
	dprintf(OUT_DEBUG, "func:0x%x, data:%p, size:%u\n", (unsigned int)func, data, (unsigned int)size);

	switch(func)
	{
	case PS2SND_Init:             *rs = sceSdInit       (DS[0]); break;
	case PS2SND_SetParam:               sceSdSetParam   (DU[0], DU[1]); break;
	case PS2SND_GetParam:         *ru = sceSdGetParam   (DU[0]); break;
	case PS2SND_SetSwitch:              sceSdSetSwitch  (DU[0], DU[1]); break;
	case PS2SND_GetSwitch:        *ru = sceSdGetSwitch  (DU[0]); break;
	case PS2SND_SetAddr:                sceSdSetAddr    (DU[0], DU[1]); break;
	case PS2SND_GetAddr:          *ru = sceSdGetAddr    (DU[0]); break;
	case PS2SND_SetCoreAttr:            sceSdSetCoreAttr(DU[0], DU[1]); break;
	case PS2SND_GetCoreAttr:      *ru = sceSdGetCoreAttr(DU[0]); break;
	case PS2SND_Note2Pitch:       *ru = sceSdNote2Pitch (DU[0], DU[1], DU[2], DS[3]); break;
	case PS2SND_Pitch2Note:       *ru = sceSdPitch2Note (DU[0], DU[1], DU[2]); break;
	case PS2SND_ProcBatch:        *rs = sceSdProcBatch  ((SdBatch *)&DU[1], &ru[1], DU[0]); break;
	case PS2SND_ProcBatchEx:      *rs = sceSdProcBatchEx((SdBatch *)&DU[2], &ru[1], DU[0], DU[1]); break;
	case PS2SND_VoiceTrans:       *rs = sceSdVoiceTrans(DS[0], DU[1], (u8 *)DU[2], (u8 *)DU[3], DU[4]); break;
	case PS2SND_BlockTrans:       *rs = sceSdBlockTrans(DS[0], DU[1], (u8 *)DU[2], DU[3], DU[4]); break;
	case PS2SND_VoiceTransStatus: *ru = sceSdVoiceTransStatus (DS[0], DS[1]); break;
	case PS2SND_BlockTransStatus: *ru = sceSdBlockTransStatus (DS[0], DS[1]); break;
//	case PS2SND_SetTransCallback: void* sceSdSetTransCallback (u16 channel, void SD_TRANS_CBProc(void *) );
//	case PS2SND_SetIRQCallback:   void* sceSdSetIRQCallback ( void SD_IRQ_CBProc(void *) );
	case PS2SND_SetEffectAttr:    *rs = sceSdSetEffectAttr (DU[0], (SdEffectAttr *)&DU[1]); break;
	case PS2SND_GetEffectAttr:          sceSdGetEffectAttr (DU[0], (SdEffectAttr *)rpc_buffer[1]); break;
	case PS2SND_ClearEffectWorkArea: *rs = sceSdClearEffectWorkArea (DS[0], DS[1], DS[2]); break;
//	case PS2SND_SetTransIntrHandler: SdIntrHandler sceSdSetTransIntrHandler(int channel, SdIntrHandler func, void *arg);
//	case PS2SND_SetSpu2IntrHandler:  SdIntrHandler sceSdSetSpu2IntrHandler(SdIntrHandler func, void *arg);

	case PS2SND_StreamOpen:  *rs = sndStreamOpen((char*)&DS[4], DU[0], DU[1], DU[2], DU[3]); break;
	case PS2SND_StreamClose: *rs = sndStreamClose(); break;
	case PS2SND_StreamPlay:  *rs = sndStreamPlay(); break;
	case PS2SND_StreamPause: *rs = sndStreamPause(); break;
	case PS2SND_StreamSetPosition: *rs = sndStreamSetPosition(DS[0]); break;
	case PS2SND_StreamGetPosition: *rs = sndStreamGetPosition(); break;
	case PS2SND_StreamSetVolume:   *rs = sndStreamSetVolume(DS[0], DS[1]); break;

	case PS2SND_QueryMaxFreeMemSize: *ru = QueryMaxFreeMemSize(); break;
	default:
		dprintf(OUT_WARNING, "Unknown function id '%u'\n", (unsigned int)func);
		return(NULL);
	}

	return(rpc_buffer[1]);
}


void rpc_thread(void *d)
{
	printf(BANNER, VER_MAJOR, VER_MINOR);
	sceSifInitRpc(0);
	sceSifSetRpcQueue(&queue, GetThreadId());
	sceSifRegisterRpc(&server, BINDID_PS2SND, (void*)rpc_server, rpc_buffer[0], 0, 0, &queue);
	sceSifRpcLoop(&queue);
}

int _start(int argc, char *argv[])
{
	iop_thread_t thread;
	int threadid;

	FlushDcache();
	CpuEnableIntr(0);

	if (argc>=1)
	{
		switch(argv[1][0])
		{
		case 'e': debug_level = OUT_ERROR;   break; /* only errors */
		case 'w': debug_level = OUT_WARNING; break; /* warnings too */
		case 'i': debug_level = OUT_INFO;    break; /* and info */
		case 'd': debug_level = OUT_DEBUG;   break; /* and debuging */
		}
		printf("devug_level = %d (%c)\n", debug_level, argv[1][0]);
	}
	
	thread.attr      = TH_C;
	thread.thread    = rpc_thread;
	thread.priority  = 40;
	thread.stacksize = 0x800;
	thread.option    = 0;
	threadid = CreateThread(&thread);
	if (threadid < 0)
	{
		dprintf(OUT_ERROR, "Failed to make thread\n");
		return(1);
	}

  	StartThread(threadid, 0);

	return(0);
}


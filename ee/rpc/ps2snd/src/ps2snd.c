/*
# _____     ___ ____     ___ ____
#  ____|   |    ____|   |        | |____|
# |     ___|   |____ ___|    ____| |    \    PS2DEV Open Source Project.
#-----------------------------------------------------------------------
# Copyright 2005, James Lee (jbit<at>jbit<dot>net)
# Licenced under Academic Free License version 2.0
# Review ps2sdk README & LICENSE files for further details.
*/

#include <string.h>
#include <kernel.h>
#include <sifrpc.h>
#include <iopheap.h>
#include <stdio.h>
#include <ps2snd.h>

static SifRpcClientData_t sd_client ALIGNED(64);
static int sd_started = 0;

int sceSdInit(int flag)
{
	s32 buf[1] ALIGNED(64);

	if (!sd_started)
	{

		while(1)
		{
			if (SifBindRpc(&sd_client, BINDID_PS2SND, 0) < 0)
			{
				printf("%s: Failed to bind to rpc '0x%08x'\n", __FUNCTION__, BINDID_PS2SND);
				return(-1);
			}

 			if (sd_client.server != NULL)
				break;

			for(int i=0;i<0x10000;i++);
		}

	}

	buf[0] = flag;
	SifCallRpc(&sd_client, PS2SND_Init, 0, buf, 4, buf, 4, NULL, NULL);

	return(buf[0]);
}

void sceSdSetParam(u16 entry, u16 value)
{
	u32 buf[2] ALIGNED(64);
	buf[0] = entry;
	buf[1] = value;
	SifCallRpc(&sd_client, PS2SND_SetParam, 0, buf, 8, NULL, 0, NULL, NULL);
}

u16 sceSdGetParam(u16 entry)
{
	u32 buf[1] ALIGNED(64);
	buf[0] = entry;
	SifCallRpc(&sd_client, PS2SND_GetParam, 0, buf, 4, buf, 4, NULL, NULL);
	return(buf[0]);
}

void sceSdSetSwitch(u16 entry, u32 value)
{
	u32 buf[2] ALIGNED(64);
	buf[0] = entry;
	buf[1] = value;
	SifCallRpc(&sd_client, PS2SND_SetSwitch, 0, buf, 8, NULL, 0, NULL, NULL);
}

u32 sceSdGetSwitch(u16 entry)
{
	u32 buf[1] ALIGNED(64);
	buf[0] = entry;
	SifCallRpc(&sd_client, PS2SND_GetSwitch, 0, buf, 4, buf, 4, NULL, NULL);
	return(buf[0]);
}

void sceSdSetAddr(u16 entry, u32 value)
{
	u32 buf[2] ALIGNED(64);
	buf[0] = entry;
	buf[1] = value;
	SifCallRpc(&sd_client, PS2SND_SetAddr, 0, buf, 8, NULL, 0, NULL, NULL);
}

u32 sceSdGetAddr(u16 entry)
{
	u32 buf[1] ALIGNED(64);
	buf[0] = entry;
	SifCallRpc(&sd_client, PS2SND_GetAddr, 0, buf, 4, buf, 4, NULL, NULL);
	return(buf[0]);
}

void sceSdSetCoreAttr(u16 entry, u16 value)
{
	u32 buf[2] ALIGNED(64);
	buf[0] = entry;
	buf[1] = value;
	SifCallRpc(&sd_client, PS2SND_SetCoreAttr, 0, buf, 8, NULL, 0, NULL, NULL);
}

u16 sceSdGetCoreAttr(u16 entry)
{
	u32 buf[1] ALIGNED(64);
	buf[0] = entry;
	SifCallRpc(&sd_client, PS2SND_GetCoreAttr, 0, buf, 4, buf, 4, NULL, NULL);
	return(buf[0]);
}

u16 sceSdNote2Pitch(u16 center_note, u16 center_fine, u16 note, s16 fine)
{
	/* TODO: These functions were documented for the ps1 once... */
	return(0);
}

u16 sceSdPitch2Note(u16 center_note, u16 center_fine, u16 pitch)
{
	/* TODO: These functions were documented for the ps1 once... */
	return(0);
}

int sceSdProcBatch(sceSdBatch* batch, u32 returns[], u32 num)
{
	/* TODO */
	return(-1);
}

int sceSdProcBatchEx(sceSdBatch* batch, u32 returns[], u32 num, u32 voice)
{
	/* TODO */
	return(-1);
}


int sceSdVoiceTrans(s16 channel, u16 mode, u8 *m_addr, u32 *s_addr, u32 size)
{
	u32 buf[5] ALIGNED(64);
	((s32 *)buf)[0] = channel;
	buf[1] = mode;
	buf[2] = (u32)m_addr;
	buf[3] = (u32)s_addr;
	buf[4] = size;

	SifCallRpc(&sd_client, PS2SND_VoiceTrans, 0, buf, 20, buf, 4, NULL, NULL);
	return(((s32 *)buf)[0]);
}

int sceSdBlockTrans(s16 channel, u16 mode, u8 *m_addr, u32 size, ...)
{
	return(-1);
}

u32 sceSdVoiceTransStatus (s16 channel, s16 flag)
{
	s32 buf[2] ALIGNED(64);
	buf[0] = channel;
	buf[1] = flag;

	SifCallRpc(&sd_client, PS2SND_VoiceTransStatus, 0, buf, 8, buf, 4, NULL, NULL);
	return(((u32 *)buf)[0]);
}

u32 sceSdBlockTransStatus (s16 channel, s16 flag)
{
	s32 buf[2] ALIGNED(64);
	buf[0] = channel;
	buf[1] = flag;

	SifCallRpc(&sd_client, PS2SND_BlockTransStatus, 0, buf, 8, buf, 4, NULL, NULL);
	return(((u32 *)buf)[0]);
}

//void* sceSdSetTransCallback (u16 channel, void SD_TRANS_CBProc(void *) );
//void *sceSdSetIRQCallback( void SD_IRQ_CBProc(void *) );

int sceSdSetEffectAttr (int core, sceSdEffectAttr *attr)
{
	s32 buf[1+((sizeof(sceSdEffectAttr)+3)/4)] ALIGNED(64);
	buf[0] = core;
	memcpy(&buf[1], attr, sizeof(sceSdEffectAttr));
	SifCallRpc(&sd_client, PS2SND_SetEffectAttr, 0, buf, 4+sizeof(sceSdEffectAttr), buf, 4, NULL, NULL);
	return(buf[0]);
}

void sceSdGetEffectAttr (int core, sceSdEffectAttr *attr)
{
	s32 buf[((sizeof(sceSdEffectAttr)+3)/4)] ALIGNED(64);
	buf[0] = core;
	SifCallRpc(&sd_client, PS2SND_GetEffectAttr, 0, buf, 4, buf, sizeof(sceSdEffectAttr), NULL, NULL);
	memcpy(attr, buf, sizeof(sceSdEffectAttr));
}

int sceSdClearEffectWorkArea (int core, int channel, int effect_mode)
{
	s32 buf[3] ALIGNED(64);
	buf[0] = core;
	buf[1] = channel;
	buf[2] = effect_mode;
	SifCallRpc(&sd_client, PS2SND_ClearEffectWorkArea, 0, buf, 12, buf, 4, NULL, NULL);
	return(buf[0]);
}


u32 sndQueryMaxFreeMemSize(void)
{
	u32 buf[1] ALIGNED(64);
	SifCallRpc(&sd_client, PS2SND_QueryMaxFreeMemSize, 0, NULL, 0, buf, 4, NULL, NULL);
	return(buf[0]);
}

int sndStreamOpen(char *file, u32 voices, u32 flags, u32 bufaddr, u32 bufsize)
{
	u32 buf[32] ALIGNED(64);
	buf[0] = voices;
	buf[1] = flags;
	buf[2] = bufaddr;
	buf[3] = bufsize;
	strncpy((char*)&buf[4], file, 27*4);
	buf[31] = 0;

	SifCallRpc(&sd_client, PS2SND_StreamOpen, 0, buf, 128, buf, 4, NULL, NULL);
	return(((s32 *)buf)[0]);
}


int sndStreamClose(void)
{
	s32 buf[1] ALIGNED(64);
	SifCallRpc(&sd_client, PS2SND_StreamClose, 0, NULL, 0, buf, 4, NULL, NULL);
	return(buf[0]);
}

int sndStreamPlay(void)
{
	s32 buf[1] ALIGNED(64);
	SifCallRpc(&sd_client, PS2SND_StreamPlay, 0, NULL, 0, buf, 4, NULL, NULL);
	return(buf[0]);
}


int sndStreamPause(void)
{
	s32 buf[1] ALIGNED(64);
	SifCallRpc(&sd_client, PS2SND_StreamPause, 0, NULL, 0, buf, 4, NULL, NULL);
	return(buf[0]);
}

int sndStreamSetPosition(int block)
{
	s32 buf[1] ALIGNED(64);
	buf[0] = block;
	SifCallRpc(&sd_client, PS2SND_StreamSetPosition, 0, buf, 4, buf, 4, NULL, NULL);
	return(buf[0]);
}

int sndStreamSetVolume(int left, int right)
{
	s32 buf[2] ALIGNED(64);
	buf[0] = left;
	buf[1] = right;
	SifCallRpc(&sd_client, PS2SND_StreamSetVolume, 0, buf, 8, buf, 4, NULL, NULL);
	return(buf[0]);
}

int sndStreamGetPosition(void)
{
	s32 buf[1] ALIGNED(64);
	SifCallRpc(&sd_client, PS2SND_StreamGetPosition, 0, NULL, 0, buf, 4, NULL, NULL);
	return(buf[0]);
}

int sndLoadSample(void *buf, u32 spuaddr, int size)
{
	void *iopbuf;
	int id, iopfree;
	SifDmaTransfer_t sifdma;

	iopfree = sndQueryMaxFreeMemSize()/2;
	if (size>iopfree)
	{
		return(-1);
	}

	iopbuf = SifAllocIopHeap(size);
	if (iopbuf==0)
		return(-1);

	FlushCache(0);

	sifdma.src  = buf;
	sifdma.dest = iopbuf;
	sifdma.size = size;
	sifdma.attr = 0;

	id = SifSetDma(&sifdma, 1);
	while(SifDmaStat(id) >= 0);;
	FlushCache(0);

	sceSdVoiceTrans(0, SD_TRANS_WRITE, iopbuf, (void*)spuaddr, size);
	sceSdVoiceTransStatus(0, 1);

	SifFreeIopHeap(iopbuf);

	return(size);
}

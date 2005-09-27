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

#define FLAG_STEREO 0x1
#define FLAG_BUFID  0x10000
#define STATUS_CLOSED   0
#define STATUS_OPEN     1
#define STATUS_PLAYING  2
#define STATUS_ERROR    3

#define stream_endflag (stream_flags&0xF000)

/*
Flag layout:
	bit0     = 1=stereo 0=mono
	bit12-15 = end flag
*/

static u32 stream_flags=0; /* bit0=stereo/mono, bit12->bit15 bit16=current buffer, bit24->bit31=status */
static int stream_sema=-1, stream_thid=-1; /* Semaphore ID, Thread ID */
static int stream_fd; /* Current FD */
static u32 stream_bufid; /* The current buffer the SPU2 is playing */
#define stream_bufsafe (1-stream_bufid)
static int stream_cur; /* The 16-byte block ID of the begining of bufid */
static int stream_status;
static u8 *stream_buf; /* buffer */
static u32 stream_buflen; /* length of buffers (clipped to 16bytes) */
static u32 stream_bufspu[2]; /* position of buffer in spu ram 0=left, 1=right */
static u32 stream_voice[2]; /* voice ID */

static inline void setloopflags(int id, u8 *buf, int len)
{
	/* XXX: double check these numbers */
	/* Set loop flags */
	for (int i=0;i<len;i+=16)
	{
//		if (buf[i+1] == 0)
//		{
			if (id == 0 && i==0)
				buf[i+1] = 6; /* Set 'repeat start' when at the begining of buf0 */
			else if (id == 1 && ((i+16)>=len))
				buf[i+1] = 3; /* Set 'repeat from begining' when at the end of buf1 */
			else
				buf[i+1] = 2; /* Set 'in repeat section' when elsewhere */
//		}
	}
}

/* 
	fillbuf - Fill a specific buffer

	This kinda sucks at the moment, should make the IOP blocks smaller than the SPU2 blocks...

	Load data from file and send it to the SPU2.
	If we're stereo and near the end of the file, we send half the data to left and half to right

*/
int fillbuf(int id, int chan)
{
	u8 *buf = stream_buf;
	int size;

	id &= 1;

	size = read(stream_fd, buf, stream_buflen);
	if (size<0)  /* Error */
	{
		dprintf(OUT_ERROR, "Channel%d: error: %d\n", chan,  size);
		return(-1);
	}
	if (size==0)
		return(0);   /* EOF */

	/* If we're stereo and we've read less than a chunk, we're screwed  */
	if ((stream_flags&FLAG_STEREO) && (size<stream_buflen)) 
	{
		dprintf(OUT_ERROR, "Channel%d: failed to read entire chunk (read %d bytes)\n", chan, size);
		return(-1);
	}

	setloopflags(id, buf, size);

	sceSdVoiceTrans(0, SD_TRANS_WRITE | SD_TRANS_MODE_DMA, buf, (u8*)(stream_bufspu[chan]+(id*stream_buflen)), size);
	sceSdVoiceTransStatus(0, 1);

	return(size);
}


void stream_thread(void *a)
{
	while(1)
	{
		/* Wait for an interrupt */
		WaitSema(stream_sema);

		/* Update stuff */
		stream_bufid = stream_bufsafe;
		stream_cur+=(stream_buflen/16);

		dprintf(OUT_DEBUG, "SPU2 now playing buffer %d (block %d)\n", (int)stream_bufid, stream_cur);

		/* Fill the buffer the SPU2 isn't playing */
		for (int i=0;i<1+(stream_flags&FLAG_STEREO);i++)
		{
			int r;
			r = fillbuf(stream_bufsafe, i);
			if (r<stream_buflen) /* treat EOF and errors as the same thing atm */
			{
				sndStreamClose();
				return;
			}
		}

		sceSdSetAddr(SD_ADDR_IRQA, stream_bufspu[0]+(stream_bufsafe*stream_buflen));
		sceSdSetCoreAttr((2<<1), 1);
	}

}

int stream_handler(int core, void *data)
{
	iSignalSema(stream_sema);
	return(0); /* What should i return here? */
}

int sndStreamOpen(char *file, u32 voices, u32 flags, u32 bufaddr, u32 bufsize)
{
	iop_thread_t thread;

	dprintf(OUT_DEBUG, "%s\n", file);
	if (stream_status)
	{
		dprintf(OUT_WARNING, "There's a stream already open\n");
		return(-1);
	}

	stream_voice[0] = voices&0xffff;
	stream_voice[1] = (voices>>16)&0xffff;

	dprintf(OUT_INFO, "%s  Voices %d:%d and %d:%d\n", file, (int)stream_voice[0]&1, (int)stream_voice[0]>>1, (int)stream_voice[1]&1, (int)stream_voice[1]>>1);

	if ((stream_voice[0]&1) != (stream_voice[1]&1))
	{
		dprintf(OUT_ERROR, "Stream voices arn't on the same core!!!!\n");
		return(-1);
	}

	/* Setup Semaphore */
	if (stream_sema<0)
	{
		iop_sema_t sema;

		sema.attr = 0;
		sema.option = 0;
		sema.initial = 0;
		sema.max = 1;

		stream_sema = CreateSema(&sema);
		if (stream_sema<0)
			dprintf(OUT_ERROR, "Failed to get a semaphore\n");
	}

	stream_buflen = bufsize*16;
	stream_flags = flags&0xffff; /* only the bottom 16bits are for user use! */

	stream_buf = AllocSysMemory(ALLOC_FIRST, stream_buflen, NULL);
	if (stream_buf==NULL)
	{
		dprintf(OUT_ERROR, "malloc failed (%u bytes)\n", (unsigned int)stream_buflen);
		return(-2);
	}

	/* Try to open file... */
	stream_fd = open(file, O_RDONLY);
	if (stream_fd<0)
	{
		dprintf(OUT_ERROR, "open failed (%d)\n", stream_fd);
		FreeSysMemory(stream_buf);
		return(-3);
	}

	stream_bufspu[0] = bufaddr;
	stream_bufspu[1] = bufaddr + (stream_buflen*2);

	/* Setup SPU2 voice volumes.... */
	if (stream_flags&FLAG_STEREO)
	{
		dprintf(OUT_INFO, "stereo...\n");
		sceSdSetParam(stream_voice[0] | SD_VPARAM_VOLL,  0x1fff);
		sceSdSetParam(stream_voice[0] | SD_VPARAM_VOLR,  0);
		sceSdSetParam(stream_voice[1] | SD_VPARAM_VOLL,  0);
		sceSdSetParam(stream_voice[1] | SD_VPARAM_VOLR,  0x1fff);
	}
	else
	{
		dprintf(OUT_INFO, "mono...\n");
		sceSdSetParam(stream_voice[0] | SD_VPARAM_VOLL,  0x1fff);
		sceSdSetParam(stream_voice[0] | SD_VPARAM_VOLR,  0x1fff);
	}

	/* Setup other SPU2 voice stuff... */
	for (int i=0;i<1+(stream_flags&FLAG_STEREO);i++) /* XXX */ 
	{
		sceSdSetParam(stream_voice[i] | SD_VPARAM_PITCH, 0x1000); /* 0x1000 = normal pitch */
		sceSdSetParam(stream_voice[i] | SD_VPARAM_ADSR1, SD_SET_ADSR1(SD_ADSR_AR_EXPi, 0, 0xf, 0xf));
		sceSdSetParam(stream_voice[i] | SD_VPARAM_ADSR2, SD_SET_ADSR2(SD_ADSR_SR_EXPd, 127, SD_ADSR_RR_EXPd, 0));
	}

	/* Get a thread */
	thread.attr      = TH_C;
	thread.thread    = stream_thread;
	thread.priority  = 40;
	thread.stacksize = 0x800;
	thread.option    = 0;
	stream_thid = CreateThread(&thread);
	if (stream_thid<0)
	{
		dprintf(OUT_ERROR, "Failed to make play thread\n");
		sndStreamClose();
		return(-1);
	}

	/* Start the thread (it'll wait patiently until it gets an interrupt) */
	StartThread(stream_thid, NULL);

	/* Get some interrupts going! */
	sceSdSetCoreAttr((2<<1), 0); /* disable pesky interrupts for a minute */
	sceSdSetSpu2IntrHandler(stream_handler, (void*)1);

	stream_status = STATUS_OPEN;

	stream_cur = 0;
	if (sndStreamSetPosition(0)<0)
	{
		sndStreamClose();
		return(-1);
	}

	dprintf(OUT_INFO, "Opened %s\n", file);

	return(0);
}

int sndStreamClose(void)
{
	if (stream_status==STATUS_PLAYING)
		sndStreamPause();

	if (stream_status==STATUS_CLOSED)
		return(-1);

	if (stream_thid>=0)
		DeleteThread(stream_thid);


	close(stream_fd);

	FreeSysMemory(stream_buf);
	sceSdSetSpu2IntrHandler(NULL, NULL);

	stream_status = STATUS_CLOSED;
	dprintf(OUT_INFO, "Closed!\n");
	return(0);
}

int sndStreamPlay(void)
{
	if (stream_status==STATUS_CLOSED)
		return(-1);
	if (stream_status==STATUS_PLAYING)
		return(0);

	/* Press down the keys :) */
	if (stream_flags&FLAG_STEREO)
		sceSdSetSwitch((stream_voice[0]&1) | SD_SWITCH_KEYDOWN, 1<<(stream_voice[0]>>1) | 1<<(stream_voice[1]>>1));
	else
		sceSdSetSwitch((stream_voice[0]&1) | SD_SWITCH_KEYDOWN, 1<<(stream_voice[0]>>1));

	sceSdSetCoreAttr((2<<1), 1); 

	stream_status=STATUS_PLAYING;

	dprintf(OUT_INFO, "Playing!\n");
	return(1);
}


int sndStreamPause(void)
{
	if (stream_status==STATUS_CLOSED)
		return(-1);
	if (stream_status==STATUS_OPEN)
		return(0);

	/* Release keys */
	if (stream_flags&FLAG_STEREO)
		sceSdSetSwitch((stream_voice[0]&1) | SD_SWITCH_KEYUP, 1<<(stream_voice[0]>>1) | 1<<(stream_voice[1]>>1));
	else
		sceSdSetSwitch((stream_voice[0]&1) | SD_SWITCH_KEYUP, 1<<(stream_voice[0]>>1));

	sceSdSetCoreAttr((2<<1), 0); /* disable pesky interrupts for a minute */

	stream_status=STATUS_OPEN;

	dprintf(OUT_INFO, "Paused!\n");
	return(1);
}

int sndStreamSetPosition(int block)
{
	int chunk;
	int r=0;
	if (stream_status==STATUS_CLOSED)
		return(-1);
	if (stream_status==STATUS_PLAYING)
		r = sndStreamPause();
	if (block<0)
		return(-2);

	/* lock block number to a chunk */
	chunk = (stream_buflen/16)*(1+(stream_flags&FLAG_STEREO)); /* XXX */
	dprintf(OUT_DEBUG, "chunk = %d\n", chunk);
	block = (block/chunk)*chunk;

	stream_cur = block;
	lseek(stream_fd, block*16, SEEK_SET);

	stream_bufid = 0;

	sceSdSetAddr(SD_ADDR_IRQA, stream_bufspu[0]+stream_buflen);

	for (int i=0;i<2;i++)
	for (int c=0;c<1+(stream_flags&FLAG_STEREO);c++) /* XXX */ 
	if (fillbuf(i, c)<=0)
		{
			dprintf(OUT_ERROR, "Hit EOF or error on buffer fill %d\n", i);
			sndStreamClose();
			return(-1);
		}

	for (int c=0;c<1+(stream_flags&FLAG_STEREO);c++) /* XXX */ 
	{
		sceSdSetAddr(stream_voice[c] | SD_VADDR_SSA, stream_bufspu[c]);
	}

	/* Restart playing if we were playing before */
	if (r)
		sndStreamPlay();

	dprintf(OUT_INFO, "Position %d!\n", stream_cur);

	return(block);
}

int sndStreamGetPosition(void)
{
	int i;
	if (stream_status==STATUS_CLOSED)
		return(-1);

	if (stream_status==STATUS_OPEN)
		return(stream_cur);

	/* During playback we can get the current position in the buffer from the SPU2! */
	i = sceSdGetAddr(stream_voice[0] | SD_VADDR_NAX);
	i -= stream_bufspu[0]+(stream_buflen*stream_bufid);
	i /= 16;

	return(stream_cur+i);
}

int sndStreamSetVolume(int left, int right)
{
	if (stream_status==STATUS_CLOSED)
		return(-1);

	if (stream_flags&FLAG_STEREO)
	{
		sceSdSetParam(stream_voice[0] | SD_VPARAM_VOLL,  left);
		sceSdSetParam(stream_voice[0] | SD_VPARAM_VOLR,  0);
		sceSdSetParam(stream_voice[1] | SD_VPARAM_VOLL,  0);
		sceSdSetParam(stream_voice[1] | SD_VPARAM_VOLR,  right);
	}
	else
	{
		sceSdSetParam(stream_voice[0] | SD_VPARAM_VOLL,  left);
		sceSdSetParam(stream_voice[0] | SD_VPARAM_VOLR,  right);
	}
	return(0);
}

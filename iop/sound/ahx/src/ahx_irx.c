/*
# _____     ___ ____     ___ ____
#  ____|   |    ____|   |        | |____|
# |     ___|   |____ ___|    ____| |    \    PS2DEV Open Source Project.
#-----------------------------------------------------------------------
# Copyright 2001-2004, ps2dev - http://www.ps2dev.org
# Licenced under Academic Free License version 2.0
# Review ps2sdk README & LICENSE files for further details.
#
# $Id$
*/

#include "irx_imports.h"
#include "freesd.h"
#include "ahx.h"

// if this is defined the irx will be compiled without PRINTF output
#define COMPACT_CODE

#define MODNAME "AHXplayer" // module name
#define MODVERSION "1.0b"   // module version
#define MODAUTHOR  "Raizor"   // module author
#define M_PRINTF(format, args...)	printf(MODNAME ": " format, ## args) // module printf

// LIBSD defines
#define SD_CORE_1			1
#define SD_INIT_COLD		0

char *spubuf = NULL; // buffer for storing data, SPU2 grabs it from here
char *pcmbuf = NULL; // ahx data is mixed to PCM straight into this buffer
int transfer_sema = 0; // semaphore to indicate when a transfer has completed
int play_tid = 0; // play_thread ID
int intr_state; // interrupt state

// IRX Stuff for Export and ID
#define	AHX_IRX		         0xC001D0E  // unique ID of our IRX
#define AHX_INIT             0x01
#define AHX_PLAY             0x02
#define AHX_PAUSE            0x03
#define AHX_QUIT             0x04
#define AHX_LOADSONG         0x05
#define AHX_SETVOLUME        0x06
#define AHX_SETBOOST         0x07
#define AHX_OVERSAMPLE       0x08
#define AHX_SUBSONG          0x09
#define TH_C		   0x02000000 // I have no idea what this is o_O

int readpos = 0; // offset that spu is reading audio data from
int seg_done[8]; // we have 8 segments each containing a full AHX->PCM buffer
int playing = 0; // are we playing a tune at the moment?
int songloaded = 0; // is a song loaded and ready?
int boost_val = 0; // sound output multiply value
int oversample_enabled = 0; // are we oversampling? (nicer sound)
u16 SPU2_Volume = 0x3fff; // current output volume of SPU2
char mod_buffer[50*1024]; // 50kb buffer for loading songs

SifRpcDataQueue_t qd;
SifRpcServerData_t Sd0;

extern void wmemcpy(void *dest, void *src, int numwords);
static unsigned int buffer[0x80];

// function prototypes
void       AHX_Thread(void* param);
void       AHX_PlayThread(void* param);
static int AHX_TransCallback(void* param);
void       AHX_SetVol(u16 vol);
void       AHX_ResetPlayThread();
void       AHX_ClearSoundBuffers();
void*      AHX_rpc_server(unsigned int funcno, void *data, int size);
void*      AHX_Init(unsigned int* sbuff);
void*      AHX_LoadSong(unsigned int* sbuff);
void*      AHX_SubSong(unsigned int* sbuff);
void*      AHX_Play(unsigned int* sbuff);
void*      AHX_Pause(unsigned int* sbuff);
void*      AHX_SetVolume(unsigned int* sbuff);
void*      AHX_SetBoost(unsigned int* sbuff);
void*      AHX_ToggleOversampling(unsigned int* sbuff);
void*      AHX_Quit(unsigned int* sbuff);

//***************************************************************
//	AHX Entry Point
//	-------------
//		This function is called automatically when the IRX module
//      is loaded. We simply set up the AHX Setup thread and 
//		exit...
//***************************************************************
int _start ()
{
  iop_thread_t param;
  int th;
  FlushDcache();
  CpuEnableIntr(0);
  EnableIntr(40);	// Enables SPU DMA (channel 1) interrupt.
  param.attr         = TH_C;
  param.thread       = AHX_Thread;
  param.priority 	 = 40;
  param.stacksize    = 0x800;
  param.option       = 0;
  th = CreateThread(&param);
  if (th > 0) {
  	StartThread(th,0);
	return 0;
  }else{
    return 1;
  }
}

//***************************************************************
//	AHX Setup Thread
//	-------------
//		This inits the AHX output system (AHX->PCM decoder), sets
//		up our semaphore so the SPU can flag when it wants more
//		data. We also init the RPC server in here, as well as 
//		creating buffers for the SPU to transfer PCM data from 
//		and also a buffer to decode AHX to PCM.
//***************************************************************
void AHX_Thread(void* param)
{
	// int i;
	iop_sema_t sema; // semaphore

	// set sema properties and create...
	sema.attr = 0; // SA_THFIFO;
	sema.initial = 0;
	sema.max = 1;
	transfer_sema= CreateSema(&sema);

	// check sema created successfully, or exit
	if(transfer_sema <= 0) {
		#ifndef COMPACT_CODE
			M_PRINTF("FATAL - Failed to create semaphore!\n");
		#endif
		ExitDeleteThread();
	}

	// allocate memory for SPU2 xfer buffer
	spubuf = AllocSysMemory(0, 0x800, NULL); // 2048 bytes (enough for 2 SPU cycles at 512 bytes per channel) 
	pcmbuf = AllocSysMemory(0, 0xF00*8, NULL); // enough to hold 8 full AHX-PCM decoded chunks (0xF00 per chunk)
	if(spubuf == NULL) {
		#ifndef COMPACT_CODE
		M_PRINTF("FATAL - Failed to allocate memory for sound buffer!\n");
		#endif
		ExitDeleteThread();
	}

	// print version #
	#ifndef COMPACT_CODE
		M_PRINTF("AHXplayer %s Started\n", MODVERSION);
	#endif	
	
	AHX_ClearSoundBuffers();

	#ifndef COMPACT_CODE
		M_PRINTF("Memory Allocated. %d bytes left.\n",QueryTotalFreeMemSize());
	#endif

	// Init the RPC server
	SifInitRpc(0);
	SifSetRpcQueue(&qd, GetThreadId());
	SifRegisterRpc(&Sd0, AHX_IRX, (void*)AHX_rpc_server, (void*)&buffer[0], 0, 0, &qd);
	SifRpcLoop(&qd);
}

//***************************************************************
//	Playing Thread
//	--------------
//		This code is called repeatedly, in here we mix our AHX
//		data into PCM format and store it in a buffer. We then
//		move it to another buffer so the SPU can grab it and 
//		play it.
//***************************************************************
void AHX_PlayThread(void* param)
{
	int i;
	int chunk; // we transfer to 2 blocks of SPU mem, are we working chunk 1 or 2? 	

	// reset position and flags etc
	AHX_ResetPlayThread();

	// main thread loop
	while(1)
	{
		// backfill chunks that have already been passed to SPU. We do this before the WaitSema
		// because we could be stuck there waiting for a while. If any spare time exists, we use 
		// it to mix the buffers...
		if (playing)
		{		
			for (i=0; i<7; i++)
			{
				// check if we've just passed a section of data and refill it 
				if (readpos>=0xF00*(i+1) && readpos<=0xF00*(i+2) && !seg_done[i])
				{				
					if (playing)
					{
						AHXOutput_MixBuffer( (short*)(pcmbuf+(0xF00*i)) ); // remix
					}
					seg_done[i] = 1; // mark segment as mixed and ready
					if (i>0) seg_done[i-1] = 0; else seg_done[7] = 0; // catch end segment
				}
			}
		}

		// wait for SPU2 to set sema to indicate it's ready for more data
		WaitSema(transfer_sema);
		
		// suspend interrupts so our sema trigger doesn't get fired more than once
		// while we work in here...
		CpuSuspendIntr(&intr_state);

		if (playing)
		{
			// get SPU transfer status to determine which area of SPU buffer to write PCM data to
			chunk = 1 - (SdBlockTransStatus(1, 0 )>>24);

			wmemcpy(spubuf+(1024*chunk),pcmbuf+readpos,512);		// left channel  
			wmemcpy(spubuf+(1024*chunk)+512,pcmbuf+readpos,512);	// right channel (same cos we're mono)

			// SPU2 reads 512 bytes for each channel per frame, we're using the same PCM buffer for each
			// channel (left/right), so update read position by 512 bytes. SPU reads data like so:
			// 512 bytes for left channel | 512 bytes for right channel | 512 for left... etc etc
			readpos += 512;

			// check to see whether we've reached the end of our PCM buffer, if so, reset read position
			// remix end chunk of AHX data in PCM buffer and mark segment as complete
			if(readpos >= 0xF00 * 8) 
			{
				readpos = 0;			
				AHXOutput_MixBuffer( (short*)(pcmbuf+(0xF00*7)) );
				seg_done[7] = 1;
				seg_done[6] = 0;
			}
		}

		// re-enable interrupts
		CpuResumeIntr(intr_state);
	}
}

//***************************************************************
//	AHX Callback
//	--------
//		Called by SPU2 when it wants more sound data
//***************************************************************
static int AHX_TransCallback(void* param)
{
	// signal our semaphore to indicate that SPU2 has finished 
	// processing current sound data and that it's time to 
	// xfer some more data across...
	iSignalSema(transfer_sema);
	return 1;
}

//***************************************************************
//	AHX Set Volume
//	--------
//		Sets the output SPU volume for both channels (left/right)
//***************************************************************

void AHX_SetVol(u16 vol)
{
	SdSetParam(SD_CORE_1|SD_PARAM_BVOLL,vol);
	SdSetParam(SD_CORE_1|SD_PARAM_BVOLR,vol);
}

//***************************************************************
//	AHX Reset Play Thread
//	---------------------
//		Resets the play thread properties
//***************************************************************
void AHX_ResetPlayThread()
{
	int i;

	// loop through and mark all segments as not done
	for (i=0; i<8; i++)
	{
		seg_done[i] = 0;
	}

	// set play marker back to beginning
	readpos = 0;
}

//***************************************************************
//	AHX Clear Sound Buffers
//	---------------------
//		Clears the pcm and spu buffers
//***************************************************************
void AHX_ClearSoundBuffers()
{
	// clear sound buffer
	memset(spubuf, 0, 0x800);
	memset(pcmbuf, 0, 0xF00*8);
}

//***************************************************************
//	AHX RPC Server
//	-------------
//		This catches messages sent across from the EE interface
//		and calls the appropriate function (according to the 
//      function code 'funcno'.
//***************************************************************
void* AHX_rpc_server(unsigned int funcno, void *data, int size)
{
	switch(funcno) {
		case AHX_INIT:
			return AHX_Init((unsigned*)data);
		case AHX_PLAY:
			return AHX_Play((unsigned*)data);
		case AHX_PAUSE:
			return AHX_Pause((unsigned*)data);
		case AHX_QUIT:
			return AHX_Quit((unsigned*)data);
		case AHX_LOADSONG:
			return AHX_LoadSong((unsigned*)data);
		case AHX_SETVOLUME:
			return AHX_SetVolume((unsigned*)data);
		case AHX_SETBOOST:
			return AHX_SetBoost((unsigned*)data);
		case AHX_OVERSAMPLE:
			return AHX_ToggleOversampling((unsigned*)data);
		case AHX_SUBSONG:
			return AHX_SubSong((unsigned*)data);
	}
	return NULL;
}

//***************************************************************
//	AHX Main Init 
//	-------------
//		Called via RPC interface. This inits the player, SPU and
//		sets up the playing thread.
//***************************************************************
void* AHX_Init(unsigned int* sbuff)
{
	iop_thread_t play_thread; // play thread

	// init AXH Player
	AHXPlayer_Init();

	#ifndef COMPACT_CODE
		printf("AHX INIT DONE!\n");
	#endif	

	// Initialise SPU
	SdInit(SD_INIT_COLD);

	// set left and right channel volumes
	SdSetParam(SD_CORE_1|SD_PARAM_MVOLL,0x3fff);
	SdSetParam(SD_CORE_1|SD_PARAM_MVOLR,0x3fff);

	// Start audio streaming
	SdBlockTrans(1,SD_BLOCK_TRANS_LOOP,spubuf, 0x800, 0);

	// set SPU2 callback function pointer
	SdSetTransCallback(1, (void *)AHX_TransCallback);

	// Start playing thread
	play_thread.attr         = TH_C;
  	play_thread.thread       = AHX_PlayThread;
  	play_thread.priority 	 = 39;
  	play_thread.stacksize    = 0x800;
  	play_thread.option       = 0;
  	play_tid = CreateThread(&play_thread);
	if (play_tid > 0) StartThread(play_tid,0);
	else {
		#ifndef COMPACT_CODE
		M_PRINTF("FATAL - Failed to start playing thread!\n");
		#endif
		ExitDeleteThread();
	}

	// volume up
	AHX_SetVol(0x3fff);

	// send out addresses for EE RPC to write data to
	sbuff[1] = (unsigned)mod_buffer;

	return sbuff;
}

//***************************************************************
//	AHX Load Song 
//	-------------
//		Loads a song via RPC
//***************************************************************
void* AHX_LoadSong(unsigned int* sbuff)
{
	// if we're playing, stop
	if (playing)
	{
		AHX_Pause(sbuff);
	}

	// reset playing params
	AHX_ResetPlayThread();

	#ifndef COMPACT_CODE
		printf("Loading song - oversampling = %d, boost = %d\n", oversample_enabled, boost_val);
	#endif	

	// load song
	sbuff[1] = (unsigned)AHXPlayer_LoadSongBuffer(mod_buffer, (int)sbuff[0]);
	
	// wait for SPU
	WaitSema(transfer_sema);

	AHX_ClearSoundBuffers();

	// init first sub-song
	AHXPlayer_InitSubsong(0);

	AHX_Play(sbuff);

	#ifndef COMPACT_CODE
		M_PRINTF("Memory Allocated. %d bytes left.\n",QueryTotalFreeMemSize());
	#endif

	return sbuff; // return number of subsongs
}

//***************************************************************
//	AHX Play
//	-------------
//		Plays the currently loaded/paused song
//***************************************************************
void* AHX_Play(unsigned int* sbuff)
{
	AHX_SetVol(SPU2_Volume);
	playing = 1;
	return sbuff;
}

//***************************************************************
//	AHX Pause
//	-------------
//		Pauses the currently playing song
//***************************************************************
void* AHX_Pause(unsigned int* sbuff)
{
	AHX_SetVol(0);
	playing = 0;
	return sbuff;
}

//***************************************************************
//	AHX Set Volume
//	-------------
//		Sets SPU2 Volume to %
//***************************************************************
void* AHX_SetVolume(unsigned int* sbuff)
{
	SPU2_Volume = (u16)(0x3fff*(u16)sbuff[1])/100;
	AHX_SetVol(SPU2_Volume);
	return sbuff;
}

//***************************************************************
//	AHX Set Boost
//	-------------
//		Sets the AHX Output Boost value. Each increment increases
//		output volume by 100%. Large numbers can cause 
//		distortion. It's advisable not to boost above 1 or 2.
//***************************************************************
void* AHX_SetBoost(unsigned int* sbuff)
{
	boost_val = (int)sbuff[0];
	AHXPlayer_SetBoost(boost_val);
	return sbuff;
}

//***************************************************************
//	AHX Set Oversample
//	-------------
//		Enables oversampling. This gives a smoother sound but
//		requires more CPU power. It can result in slowdown of
//		playback with certain songs or produce an awful din...
//***************************************************************
void* AHX_ToggleOversampling(unsigned int* sbuff)
{
	oversample_enabled = oversample_enabled ? 0 : 1;
	AHXPlayer_SetOversampling(oversample_enabled);
	return sbuff;
}

//***************************************************************
//	AHX Play Subsong
//	-------------
//		Plays the requested subsong. Number of subsongs is
//		returned by LoadSong(). A lot of AHX files report the
//		wrong number of subsongs - please check your AHX files...
//***************************************************************
void* AHX_SubSong(unsigned int* sbuff)
{
	AHXPlayer_InitSubsong((int)sbuff[0]);
	return sbuff;
}

//***************************************************************
//	AHX Quit
//	-------------
//		 Cleans up and quits
//***************************************************************
void* AHX_Quit(unsigned int* sbuff)
{
	SdSetTransCallback(1,NULL);
	SdBlockTrans(1,SD_BLOCK_TRANS_STOP,0,0,0);

	TerminateThread(play_tid);
	DeleteThread(play_tid);

	DeleteSema(transfer_sema);
	return sbuff;
}

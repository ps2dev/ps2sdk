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
# AHX EE-side RPC code.
*/

#include <tamtypes.h>
#include <kernel.h>
#include <sifrpc.h>
#include <stdarg.h>
#include <string.h>
#include <malloc.h>
#include <fileio.h>
#include <stdio.h>
#include "ahx_rpc.h"

static unsigned sbuff[64] __attribute__((aligned (64)));
static struct t_SifRpcClientData cd0;
#define IOP_MEM	0xbc000000 // EE mapped IOP mem 
char* songbuffer_addr;
int ahx_init_done = 0;

//***************************************************************
//	Read/Write IOP Mem
//	-------------
//		This is an internal function used to read and write
//      directly to IOP memory (write songdata to iop etc)
//***************************************************************
void iop_readwrite(void *addr, void *buf, u32 size, u32 read)
{
	DI();
	ee_kmode_enter();
	if (read) memcpy(buf, addr + IOP_MEM, size); else memcpy(addr + IOP_MEM, buf, size);
	ee_kmode_exit();
	EI();
}

//***************************************************************
//	AHX Init
//	-------------
//		Sends a call the the loaded AHX IRX, telling it to set
//      things up ready for loading a song.
//***************************************************************
int AHX_Init()
{
	int i;	
	// struct t_SifDmaTransfer sdt;

	// if already init'd, exit
	if (ahx_init_done) return 0;

	// bind rpc
	while(1){
		if (SifBindRpc( &cd0, AHX_IRX, 0) < 0) return -1; // bind error
 		if (cd0.server != 0) break;
    	i = 0x10000;
    	while(i--);
	}

	SifCallRpc(&cd0,AHX_INIT,0,(void*)(&sbuff[0]),64,(void*)(&sbuff[0]),64,0,0);
	FlushCache(0);

	songbuffer_addr = (char*)sbuff[1];

	// set flag, init done
	ahx_init_done = 1;
	return 0;
}

//***************************************************************
//	AHX Play
//	-------------
//		Sends a call the the loaded AHX IRX, telling it to play
//      the currently loaded song.
//***************************************************************
int AHX_Play()
{
	int i;
	while(1){
		if (SifBindRpc( &cd0, AHX_IRX, 0) < 0) return -1;
 		if (cd0.server != 0) break;
    	i = 0x10000;
    	while(i--);
	}
	SifCallRpc(&cd0,AHX_PLAY,0,(void*)(&sbuff[0]),64,(void*)(&sbuff[0]),64,0,0);
	FlushCache(0);
	return 0;
}

//***************************************************************
//	AHX Pause
//	-------------
//		Sends a call the the loaded AHX IRX, telling it to pause
//      the currently loaded song.
//***************************************************************
int AHX_Pause()
{
	int i;
	while(1){
		if (SifBindRpc( &cd0, AHX_IRX, 0) < 0) return -1;
 		if (cd0.server != 0) break;
    	i = 0x10000;
    	while(i--);
	}
	SifCallRpc(&cd0,AHX_PAUSE,0,(void*)(&sbuff[0]),64,(void*)(&sbuff[0]),64,0,0);
	FlushCache(0);
	return 0;
}

//***************************************************************
//	AHX Play Subsong
//	-------------
//		Sends a call the the loaded AHX IRX, telling it to load
//      subsong (songNo). You can determine the number of subsongs
//      by checking the values returned by AHX_LoadSong() and
//      AHX_LoadSongBuffer();
//***************************************************************
int AHX_SubSong(int songNo)
{
	int i;
	while(1){
		if (SifBindRpc( &cd0, AHX_IRX, 0) < 0) return -1;
 		if (cd0.server != 0) break;
    	i = 0x10000;
    	while(i--);
	}
	sbuff[0] = (unsigned)songNo;
	SifCallRpc(&cd0,AHX_PAUSE,0,(void*)(&sbuff[0]),64,(void*)(&sbuff[0]),64,0,0);
	FlushCache(0);
	return 0;
}

//***************************************************************
//	AHX Set Volume
//	-------------
//		Sends a call the the loaded AHX IRX, telling it to change
//      the output volume of the SPU2.  volumePercentage argument
//      can range from 0 (0% silence) to 100 (100% full volume)
//***************************************************************
int AHX_SetVolume(int volumePercentage)
{
	int i;
	while(1){
		if (SifBindRpc( &cd0, AHX_IRX, 0) < 0) return -1;
 		if (cd0.server != 0) break;
    	i = 0x10000;
    	while(i--);
	}
	sbuff[0] = (unsigned)volumePercentage;
	SifCallRpc(&cd0,AHX_QUIT,0,(void*)(&sbuff[0]),64,(void*)(&sbuff[0]),64,0,0);
	FlushCache(0);
	return 0;
}

//***************************************************************
//	AHX Set Boost
//	-------------
//		Sends a call the the loaded AHX IRX, telling it to change
//      the output boost value.  Boost value multiplies the level
//      of the output for the AHX Mixer. A boost value of 1 is
//      twice as load as a boost value of 0. A boost value of 3
//      is twice as load as 2 etc etc (ala DB)
//***************************************************************
int  AHX_SetBoost(int boostValue)
{
	int i;
	while(1){
		if (SifBindRpc( &cd0, AHX_IRX, 0) < 0) return -1; 
 		if (cd0.server != 0) break;
    	i = 0x10000;
    	while(i--);
	}
	sbuff[0] = (unsigned)boostValue;
	SifCallRpc(&cd0,AHX_SETBOOST,0,(void*)(&sbuff[0]),64,(void*)(&sbuff[0]),64,0,0);
	FlushCache(0);
	return 0;
}

//***************************************************************
//	AHX Toggle Oversampling
//	-------------
//		Switches oversampling on/off.  Oversampling produces a
//      smoother output sound but uses a lot more CPU power. It
//      sounds nasty/slows down for a lot of songs - use with 
//      caution (or not at all)
//***************************************************************
int  AHX_ToggleOversampling()
{
	int i;
	while(1){
		if (SifBindRpc( &cd0, AHX_IRX, 0) < 0) return -1; 
 		if (cd0.server != 0) break;
    	i = 0x10000;
    	while(i--);
	}
	SifCallRpc(&cd0,AHX_OVERSAMPLING,0,(void*)(&sbuff[0]),64,(void*)(&sbuff[0]),64,0,0);
	FlushCache(0);
	return 0;
}

//***************************************************************
//	AHX Quit
//	-------------
//		Sends a call the the loaded AHX IRX, telling it to quit.
//      This frees up IOP mem, quites threads, deletes semaphores
//      and all that jazz....
//***************************************************************
int AHX_Quit()
{
	int i;
	while(1){
		if (SifBindRpc( &cd0, AHX_IRX, 0) < 0) return -1; 
 		if (cd0.server != 0) break;
    	i = 0x10000;
    	while(i--);
	}
	SifCallRpc(&cd0,AHX_QUIT,0,(void*)(&sbuff[0]),64,(void*)(&sbuff[0]),64,0,0);
	FlushCache(0);
	return 0;
}

//***************************************************************
//	AHX LongSongBuffer
//	-------------
//		This loads a song from a buffer in memory. It copies
//      [songsize] bytes from [songdata] to the IOP memory
//      song buffer. 
//
//      returns number of subsongs.
//***************************************************************
int AHX_LoadSongBuffer(char* songdata, int songsize)
{
	int i;
	
	// write song data to IOP song buffer
	iop_readwrite(songbuffer_addr, songdata, songsize, 0);

	while(1){
		if (SifBindRpc( &cd0, AHX_IRX, 0) < 0) return -1;
 		if (cd0.server != 0) break;
    	i = 0x10000;
    	while(i--);
	}
	
	// set oversample and boost
	sbuff[0] = (unsigned)songsize;

	// call rpc
	SifCallRpc(&cd0,AHX_LOADSONG,0,(void*)(&sbuff[0]),64,(void*)(&sbuff[0]),64,0,0);
	FlushCache(0);

	// return number of sub-songs
	return (int)sbuff[1];
}

//***************************************************************
//	AHX LongSongBuffer
//	-------------
//		This loads a song from disk etc. It loads the songdata 
//      into memory and passes it to LongSongBuffer.
//
//      returns number of subsongs.
//***************************************************************
int AHX_LoadSong(char* filename)
{
	int fd, fdSize;
	char* buffer;

	fd = fioOpen(filename, O_RDONLY);  
	if(fd < 0) 
	{ 
		 printf("ERROR LOADING SONG\n");
		 return -1;
	}
	fdSize = fioLseek(fd, 0, SEEK_END);  
	fioLseek(fd, 0, SEEK_SET);  	  
	buffer = malloc(fdSize);  
	if(!buffer)  
	{ 
		 printf("ERROR ALLOCATING SONG MEMORY SONG\n"); 
		 return -1;
	} 	 
	fioRead(fd, buffer, fdSize); 
	fioClose(fd);
	return AHX_LoadSongBuffer(buffer, fdSize);
}

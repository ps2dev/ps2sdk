/*
    ---------------------------------------------------------------------
    ahx_rpc.c - AHX EE-side RPC code. (c) Raizor (raizor@c0der.net), 2004
	---------------------------------------------------------------------

    This code is licensed under the Academic Free License v2.0.
    See the file LICENSE included with this distribution for licensing terms.

	Designed for usage with PS2SDK...

*/

#ifndef _AHX_H
#define _AHX_H

#ifdef __cplusplus
extern "C" {
#endif

#define	AHX_IRX		         0xC001D0E
#define AHX_INIT             0x01
#define AHX_PLAY             0x02
#define AHX_PAUSE            0x03
#define AHX_QUIT             0x04
#define AHX_LOADSONG         0x05
#define AHX_SETVOLUME        0x06
#define AHX_SETBOOST         0x07
#define AHX_OVERSAMPLING     0x08
#define AHX_SUBSONG          0x09

// See ahx_rpc.c for info on functions...
int  AHX_Init();
int  AHX_LoadSongBuffer(char* songdata, int songsize);
int  AHX_LoadSong(char* filename);
int  AHX_SubSong(int songNo);
int  AHX_SetVolume(int volumePercentage);
int  AHX_SetBoost(int boostValue);
int  AHX_ToggleOversampling();
int  AHX_Quit();
int  AHX_Play();
int  AHX_Pause();

#ifdef __cplusplus
}
#endif

#endif // _AHX_H

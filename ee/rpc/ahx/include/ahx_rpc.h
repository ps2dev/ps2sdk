/*
# _____     ___ ____     ___ ____
#  ____|   |    ____|   |        | |____|
# |     ___|   |____ ___|    ____| |    \    PS2DEV Open Source Project.
#-----------------------------------------------------------------------
# Copyright 2001-2004, ps2dev - http://www.ps2dev.org
# Licenced under Academic Free License version 2.0
# Review ps2sdk README & LICENSE files for further details.
*/

/**
 * @file
 * AHX EE-side RPC code.
 */

#ifndef __AHX_H__
#define __AHX_H__

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

#ifdef __cplusplus
extern "C" {
#endif

/** AHX Init
 *
 * Sends a call the the loaded AHX IRX, telling it to set
 * things up ready for loading a song.
 */
int  AHX_Init();

/** AHX LongSongBuffer
 *
 * This loads a song from a buffer in memory. It copies
 * [songsize] bytes from [songdata] to the IOP memory
 * song buffer.
 *
 * @return number of subsongs.
 */
int  AHX_LoadSongBuffer(char* songdata, int songsize);

/** AHX LongSongBuffer
 *
 * This loads a song from disk etc. It loads the songdata
 * into memory and passes it to LongSongBuffer.
 *
 * @return number of subsongs.
 */
int  AHX_LoadSong(char* filename);

/** AHX Play Subsong
 *
 * Sends a call the the loaded AHX IRX, telling it to load
 * subsong (songNo). You can determine the number of subsongs
 * by checking the values returned by AHX_LoadSong() and
 * AHX_LoadSongBuffer();
 */
int  AHX_SubSong(int songNo);

/** AHX Set Volume
 *
 * Sends a call the the loaded AHX IRX, telling it to change
 * the output volume of the SPU2.  volumePercentage argument
 * can range from 0 (0% silence) to 100 (100% full volume)
 */
int  AHX_SetVolume(int volumePercentage);

/** AHX Set Boost
 *
 * Sends a call the the loaded AHX IRX, telling it to change
 * the output boost value.  Boost value multiplies the level
 * of the output for the AHX Mixer. A boost value of 1 is
 * twice as load as a boost value of 0. A boost value of 3
 * is twice as load as 2 etc etc (ala DB)
 */
int  AHX_SetBoost(int boostValue);

/** AHX Toggle Oversampling
 *
 * Switches oversampling on/off.  Oversampling produces a
 * smoother output sound but uses a lot more CPU power. It
 * sounds nasty/slows down for a lot of songs - use with
 * caution (or not at all)
 */
int  AHX_ToggleOversampling();

/** AHX Quit
 *
 * Sends a call the the loaded AHX IRX, telling it to quit.
 * This frees up IOP mem, quites threads, deletes semaphores
 * and all that jazz....
 */
int  AHX_Quit();

/** AHX Play
 *
 * Sends a call the the loaded AHX IRX, telling it to play
 * the currently loaded song.
 */
int  AHX_Play();

/** AHX Pause
 *
 * Sends a call the the loaded AHX IRX, telling it to pause
 * the currently loaded song.
 */
int  AHX_Pause();

#ifdef __cplusplus
}
#endif

#endif /* __AHX_H__ */

/*
# _____     ___ ____     ___ ____
#  ____|   |    ____|   |        | |____|
# |     ___|   |____ ___|    ____| |    \    PS2DEV Open Source Project.
#-----------------------------------------------------------------------
# Copyright 2005, ps2dev - http://www.ps2dev.org
# Licenced under GNU Library General Public License version 2
# Review ps2sdk README & LICENSE files for further details.
#
# $Id$
# audsrv EE-side RPC code.
*/

#ifndef _AUDSRV_H
#define _AUDSRV_H

/**
 * \file audsrv.h
 * \author gawd (Gil Megidish)
 * \date 04-24-05
 */

#ifdef __cplusplus
extern "C" {
#endif

#define	AUDSRV_IRX              0x870884d

/** minmum volume */
#define MIN_VOLUME                 0x0000

/** maximum volume */
#define MAX_VOLUME                 0x3fff

/** error codes */
#define AUDSRV_ERR_NOERROR                 0x0000
#define AUDSRV_ERR_NOT_INITIALIZED         0x0001
#define AUDSRV_ERR_RPC_FAILED              0x0002
#define AUDSRV_ERR_FORMAT_NOT_SUPPORTED    0x0003
#define AUDSRV_ERR_OUT_OF_MEMORY           0x0004
#define AUDSRV_ERR_ARGS                    0x0005
#define AUDSRV_ERR_NO_DISC                 0x0006
#define AUDSRV_ERR_NO_MORE_CHANNELS        0x0007

#define AUDSRV_ERR_FAILED_TO_LOAD_ADPCM    0x0010
#define AUDSRV_ERR_FAILED_TO_CREATE_SEMA    0x0011

/** structure used to set new format */
typedef struct audsrv_fmt_t
{
	int freq;	                ///< output frequency in hz
	int bits;                       ///< bits per sample (8, 16)
	int channels;                   ///< output channels (1, 2)
} audsrv_fmt_t;

/** adpcm sample definition */
typedef struct audsrv_adpcm_t
{
	int pitch;
	int loop;
	int channels;
	void *buffer;
	int size;
} audsrv_adpcm_t;

typedef int (*audsrv_callback_t)(void *arg);

/** Initializes audsrv library
    @returns error code
*/
int audsrv_init();

/** Shutdowns audsrv
    @returns AUDSRV_ERR_NOERROR
*/
int audsrv_quit();

/** Configures audio stream
    @param fmt output specification structure
    @returns 0 on success, or one of the error codes otherwise

    This sets up audsrv to accept stream in this format and convert
    it to SPU2's native format if required. Note: it is possible to
    change the format at any point. You might want to stop audio prior
    to that, to prevent mismatched audio output.
*/
int audsrv_set_format(struct audsrv_fmt_t *fmt);

/** Blocks until there is enough space to enqueue chunk
    @param bytes size of chunk requested to be enqueued (in bytes)
    @returns error code
    
    Blocks until there are enough space to store the upcoming chunk
    in audsrv's internal ring buffer.
*/
int audsrv_wait_audio(int bytes);

/** Sets output volume
    @param vol volume in SPU2 units [MIN_VOLUME .. MAX_VOLUME]
    @returns error code
*/
int audsrv_set_volume(int volume);

/** Uploads audio buffer to SPU
    @param chunk   audio buffer
    @param bytes   size of chunk in bytes
    @returns positive number of bytes sent to processor or negative error status

    Plays an audio buffer; It will not interrupt a playing
    buffer, rather queue it up and play it as soon as possible without
    interfering with fluent streaming. The buffer and buflen are given
    in host format (i.e, 11025hz 8bit stereo.)
*/
int audsrv_play_audio(const char *chunk, int bytes);

/** Stops audio from playing.
    @returns status code
*/
int audsrv_stop_audio();

/** Returns the last error audsrv raised
    @returns error code
*/
int audsrv_get_error();

/** Translates audsrv_get_error() response to readable string
    @returns string representation of error code
*/
const char *audsrv_get_error_string();

/** Starts playing the request track
    @param track segment to play
    @returns status code
*/
int audsrv_play_cd(int track);

/** Stops CD from playing.
    @returns status code
*/
int audsrv_stop_cd();

/** Returns the current playing sector
    @returns sector number

    CDDA type discs have sector size of 2352 bytes. There are 75
    such sectors per second.
*/
int audsrv_get_cdpos();

/** Returns the current playing sector, relative to track
    @returns sector number

    There are 75 sectors a second. To translate this position to mm:ss:ff 
    use the following:
    mm = sector / (75*60)
    ss = (sector / 75) % 60
    ff = sector % 75 

    where ff is the frame number, 1/75th of a second.
*/
int audsrv_get_trackpos();

/** Returns the number of tracks available on the CD in tray
    @returns positive track count, or negative error status code
*/
int audsrv_get_numtracks();

/** Returns the first sector for the given track
    @param track   track index, must be between 1 and the trackcount
    @returns sector number, or negative status code
*/
int audsrv_get_track_offset(int track);

/** Pauses CDDA playing
    @returns error status code

    If CDDA is paused, no operation is taken
*/
int audsrv_pause_cd();

/** Resumes CDDA playing
    @returns error status code

    If CDDA was not paused, no operation is taken
*/
int audsrv_resume_cd();

/** Starts playing at a specific sector
    @param start first sector to play
    @param end   last sector to play
    @returns status code
*/
int audsrv_play_sectors(int start, int end);

/** Returns the status of the CD tray (open, closed, seeking etc.)
    @returns value as defined in libcdvd, negative on error
*/
int audsrv_get_cd_status();

/** Returns the type of disc currently in tray
    @returns value as defined in libcdvd, negative on error
*/
int audsrv_get_cd_type();

/** Installs a callback function to be called when ringbuffer has enough
    space to transmit the request number of bytes.
    @param bytes request a callback when this amount of bytes is available
    @param cb your callback
    @param arg extra parameter to pass to callback function later
    @returns AUDSRV_ERR_NOERROR, AUDSRV_ERR_ARGS if amount is greater than sizeof(ringbuf)
*/
int audsrv_on_fillbuf(int amount, audsrv_callback_t cb, void *arg);

/** Initializes adpcm unit of audsrv
    @returns zero on success, negative value on error

    Frees up all memory taken by samples, and stops all voices from
    being played. This can be called multiple times
*/
int audsrv_adpcm_init();

/** Uploads a sample to SPU2 memory
    @param adpcm    adpcm descriptor structure
    @param buffer   pointer to adpcm sample
    @param size     size of sample (including the header)
    @returns zero on success, negative error code otherwise
*/
int audsrv_load_adpcm(audsrv_adpcm_t *adpcm, void *buffer, int size);

/** Plays an adpcm sample already uploaded with audsrv_load_adpcm()
    @param id    sample identifier, as specified in load()
    @returns zero on success, negative value on error

    The sample will be played in an unoccupied channel. If all 24 channels
    are used, then -AUDSRV_ERR_NO_MORE_CHANNELS is returned. Trying to play
    a sample which is unavailable will result in -AUDSRV_ERR_ARGS
*/
int audsrv_play_adpcm(audsrv_adpcm_t *adpcm);

#ifdef __cplusplus
}
#endif

#endif // _AUDSRV_H

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
# Function definitions for libcdvd (EE side calls to the iop module cdvdfsv).
#
# NOTE: These functions will work with the CDVDMAN/CDVDFSV or XCDVDMAN/XCDVDFSV
# modules stored in rom0.
#
# NOTE: not all functions work with each set of modules!
*/

#ifndef _LIBCDVD_H_
#define _LIBCDVD_H_

#include <tamtypes.h>


// struct for chain reading
typedef struct {
	u32 sectorLoc;		// sector location to start reading from
	u32 numSectors;		// number of sectors to read
	u32 buffer;			// buffer address to read to ( bit0: 0=EE, 1=IOP )
						// (EE addresses must be on 64byte alignment)
} CdvdChain_t;


// fioOpen() flag to open file for streaming
#define CDVD_STREAM			0x40000000

// Low Level File System for CdSearchFile() 
#define CDVD_FILE_MAXFILES	64			// max number of files in a directory
#define CDVD_FILE_MAXDIRS	128			// max number of directories
#define CDVD_FILE_MAXLEVELS	8			// max levels of directories

typedef struct {
	u32	 lsn;							// file location
	u32	 size;							// file size
	char name[16];						// file name (body)
	u8	 date[8];						// date (1=secs, 2=mins, 3=hours, 4=day, 5=mon, 6,7=year,   0=iso file flags)
} CdvdFileSpec_t;


// Cdvd Read Mode struct
typedef struct {
	u8 retries;							// number of times to retry reads when an error occurs
	u8 readSpeed;						// speed to read at CDVD_SPIN_??? (also speed to spin at)
	u8 sectorType;						// sector size value CDVD_SECTOR_???
	u8 pad;								// padding
} CdvdReadMode_t;

// sector type settings for CdReadMode structs
typedef enum {
	/* Game CD and DVD sector types.  */
	CDVD_SECTOR_2048 = 0,
	CDVD_SECTOR_2328,
	CDVD_SECTOR_2340,

	/* CDDA sector types.  */
	CDVD_SECTOR_2352 = 0,
	CDVD_SECTOR_2368,
	CDVD_SECTOR_2448
} CdvdSectorType_t;

// cd rotation speeds for CdReadMode structs (affects read speed)
#define CDVD_SPIN_MAX		0			// spin at max speed
#define CDVD_SPIN_NORMAL	1			// starts reading at max speed, slows down as read errors occur
#define CDVD_SPIN_STREAM	0			// spins at the recommended speed for streaming data
#define CDVD_SPIN_DVD0		0			// spins at the DVD layer 0 speed


// cdGetError() return values
#define CDVD_ERR_FAIL		-1			// error in cdGetError()
#define CDVD_ERR_NO			0x00		// no error occurred
#define CDVD_ERR_ABRT		0x01		// command was aborted due to cdBreak() call
#define CDVD_ERR_CMD		0x10		// unsupported command
#define CDVD_ERR_OPENS		0x11		// tray is open
#define CDVD_ERR_NODISC		0x12		// no disk inserted
#define CDVD_ERR_NORDY		0x13		// drive is busy processing another command
#define CDVD_ERR_CUD		0x14		// command unsupported for disc currently in drive
#define CDVD_ERR_IPI		0x20		// sector address error
#define CDVD_ERR_ILI		0x21		// num sectors error
#define CDVD_ERR_PRM		0x22		// command parameter error
#define CDVD_ERR_READ		0x30		// error while reading
#define CDVD_ERR_TRMOPN		0x31		// tray was opened
#define CDVD_ERR_EOM		0x32		// outermost error
#define CDVD_ERR_READCF		0xFD		// error setting command
#define CDVD_ERR_READCFR  	0xFE		// error setting command


// cdGetDiskType() return values
typedef enum {
	CDVD_TYPE_NODISK =	0x00,		// No Disc inserted
	CDVD_TYPE_DETECT,			// Detecting disc type
	CDVD_TYPE_DETECT_CD,
	CDVD_TYPE_DETECT_DVDSINGLE,
	CDVD_TYPE_DETECT_DVDDUAL,
	CDVD_TYPE_UNKNOWN,			// Unknown disc type

	CDVD_TYPE_PS1CD	=	0x10,		// PS1 CD with no CDDA tracks
	CDVD_TYPE_PS1CDDA,			// PS1 CD with CDDA tracks
	CDVD_TYPE_PS2CD,			// PS2 CD with no CDDA tracks
	CDVD_TYPE_PS2CDDA,			// PS2 CD with CDDA tracks
	CDVD_TYPE_PS2DVD,			// PS2 DVD

	CDVD_TYPE_CDDA =	0xFD,		// CDDA
	CDVD_TYPE_DVDVIDEO,			// DVD Video
	CDVD_TYPE_ILLEGAL,			// Illegal disk type
} CdvdDiscType_t;


// cdStatus() return values
#define CDVD_STAT_STOP		0x00		// disc has stopped spinning
#define CDVD_STAT_OPEN		0x01		// tray is open
#define CDVD_STAT_SPIN		0x02		// disc is spinning
#define CDVD_STAT_READ		0x06		// reading from disc
#define CDVD_STAT_PAUSE		0x0A		// disc is paused
#define CDVD_STAT_SEEK		0x12		// disc is seeking
#define CDVD_STAT_ERROR		0x20		// error occurred

typedef enum {
	CDVD_MEDIA_MODE_CD = 1,
	CDVD_MEDIA_MODE_DVD
} CdvdMediaMode_t;

// max number of toc entries for cdGetToc()
#define CDVD_MAXTOC			100

// cd callback func typedef for cdSetCallback()
typedef void (*CdCBFunc)(s32);


// cdvd location struct, used with cdIntToPos() and cdPosToInt()
typedef struct {
	u8 minute;							// minute (BCD)
	u8 second;							// second (BCD)
	u8 sector;							// sector (BCD)
	u8 track;							// track (void)
} CdvdLocation_t;

// macros for converting between an integer and a BCD number
#ifndef btoi
#define btoi(b)		((b)/16*10 + (b)%16)	// BCD to int
#endif
#ifndef itob
#define itob(i)		((i)/10*16 + (i)%10)	// int to BCD
#endif


// Modes for cdInit()
#define CDVD_INIT_INIT		0x00		// init cd system and wait till commands can be issused
#define CDVD_INIT_NOCHECK	0x01		// init cd system
#define CDVD_INIT_EXIT		0x05		// de-init system

// cdDiskReady() return values
#define CDVD_READY_READY	0x02
#define CDVD_READY_NOTREADY	0x06


// struct for ps2 clock values cdReadClock()/cdWriteClock()
typedef struct {
	u8 status;							// status
	u8 second;							// second
	u8 minute;							// minute
	u8 hour;							// hour
	u8 pad;								// padding
	u8 day;								// day
	u8 month;							// month
	u8 year;							// year
} CdvdClock_t;

/* Stream commands. */
typedef enum {
	CDVD_ST_CMD_START = 1,
	CDVD_ST_CMD_READ,
	CDVD_ST_CMD_STOP,
	CDVD_ST_CMD_SEEK,
	CDVD_ST_CMD_INIT,
	CDVD_ST_CMD_STAT,
	CDVD_ST_CMD_PAUSE,
	CDVD_ST_CMD_RESUME,
	CDVD_ST_CMD_SEEKF
} CdvdStCmd_t;

// streaming modes for cdStRead()
#define CDVD_STREAM_NONBLOCK 0
#define CDVD_STREAM_BLOCK	1

// cdTrayReq() values
#define CDVD_TRAY_OPEN		0			// Tray Open
#define CDVD_TRAY_CLOSE		1			// Tray Close
#define CDVD_TRAY_CHECK		2			// Tray Check



#ifdef __cplusplus
extern "C" {
#endif


// **** N-Command Functions ****


// read data from disc
// non-blocking, requires cdSync() call
// 
// args:	sector location to start reading from
//			number of sectors to read
//			buffer to read to
//			mode to read as
// returns: 1 if successful
//			0 if error
s32  cdRead(	u32 sectorLoc, u32 numSectors, void *buffer, CdvdReadMode_t *mode);
s32  cdDvdRead(	u32 sectorLoc, u32 numSectors, void *buffer, CdvdReadMode_t *mode);
s32  cdCddaRead(u32 sectorLoc, u32 numSectors, void *buffer, CdvdReadMode_t *mode);

// get toc from inserted disc
// 
// args:	buffer to hold toc (1024 or 2064 bytes?)
// returns:	1 if successful
//			0 otherwise
s32  cdGetToc(u8 *toc);

// seek to given sector on disc
// non-blocking, requires cdSync() call
// 
// args:	sector to seek to on disc
// returns:	1 if successful
//			0 if error
s32  cdSeek(u32 sectorLoc);

// puts ps2 cd drive into standby mode
// non-blocking, requires cdSync() call
// 
// returns:	1 if successful
//			0 if error
s32  cdStandby(void);

// stops ps2 cd drive from spinning
// non-blocking, requires cdSync() call
// 
// returns:	1 if successful
//			0 if error
s32  cdStop(void);

// pauses ps2 cd drive
// non-blocking, requires cdSync() call
// 
// returns:	1 if successful
//			0 if error
s32  cdPause(void);

// send an n-command by function number
// 
// args:	command number
//			input buffer  (can be null)
//			size of input buffer  (0 - 16 byte)
//			output buffer (can be null)
//			size of output buffer (0 - 16 bytes)
// returns:	1 if successful
//			0 if error
s32  cdApplyNCmd(u8 cmdNum, const void* inBuff, u16 inBuffSize, void* outBuff, u16 outBuffSize);

// read data to iop memory
// 
// args:	sector location to read from
//			number of sectors to read
//			buffer to read to (in iop memory)
//			read mode
// returns:	1 if successful
//			0 if error
s32  cdReadIOPMem(u32 sectorLoc, u32 numSectors, void *buf, CdvdReadMode_t *mode);

// wait for disc to finish all n-commands
// (shouldnt really need to call this yourself)
// 
// returns:	6 if busy
//			2 if ready
//			0 if error
s32  cdNCmdDiskReady(void);

// do a 'chain' of reads with one command
// last chain value must be all 0xFFFFFFFF
// (max of 64 reads can be set at once)
// non-blocking, requires cdSync() call
// 
// SUPPORTED IN XCDVDMAN/XCDVDFSV ONLY
// 
// args:	pointer to an array of read chain data
//			read mode
// returns:	1 if successful
//			0 if error
s32  cdReadChain(CdvdChain_t *readChain, CdvdReadMode_t *mode);


// **** S-Command Functions ****


// read clock value from ps2s clock
// (time values seem to be in hex or BCD)
// 
// args:	time/date struct
// returns:	1 if successful
//			0 if error
s32  cdReadClock(CdvdClock_t *clock);

// write clock value to ps2s clock
// (time values seem to be in hex or BCD)
// 
// args:	time/date struct to set clocks time with
// returns:	1 if successful
//			0 if error
s32  cdWriteClock(const CdvdClock_t *clock);

// gets the type of the currently inserted disc
// 
// returns:	disk type (CDVD_TYPE_???)
CdvdDiscType_t cdGetDiscType(void);

// gets the last error that occurred
// 
// returns:	error type (CDVD_ERR_???)
s32  cdGetError(void);

// open/close/check disk tray
// 
// args:	param (CDVD_TRAY_???)
//			address for returning tray state change
// returns:	1 if successful
//			0 if error
s32  cdTrayReq(s32 param, u32 *traychk);

// send an s-command by function number
// 
// args:	command number
//			input buffer  (can be null)
//			size of input buffer  (0 - 16 byte)
//			output buffer (can be null)
//			size of output buffer (0 - 16 bytes)
// returns:	1 if successful
//			0 if error
s32  cdApplySCmd(u8 cmdNum, const void* inBuff, u16 inBuffSize, void *outBuff, u16 outBuffSize);

// gets the status of the cd system
// 
// returns:	status (CDVD_STAT_???)
s32  cdStatus(void);

// 'breaks' the currently executing command
// 
// returns:	1 if successful
//			0 if error
s32  cdBreak(void);

// cancel power off
// 
// SUPPORTED IN XCDVDMAN/XCDVDFSV ONLY
// 
// args:	result
// returns:	1 if successful
//			0 if error
s32  cdCancelPowerOff(u32* result);

// blue led control
// 
// SUPPORTED IN XCDVDMAN/XCDVDFSV ONLY
// 
// args:	control value
//			result
// returns:	1 if successful
//			0 if error
s32  cdBlueLedCtrl(u8 control, u32 *result);

// power off
// 
// SUPPORTED IN XCDVDMAN/XCDVDFSV ONLY
// 
// args:	result
// returns:	1 if successful
//			0 if error
s32  cdPowerOff(u32 *result);

// set media mode
// 
// SUPPORTED IN XCDVDMAN/XCDVDFSV ONLY
// 
// args:	media mode
// returns:	1 if successful
//			0 if error
s32  cdSetMediaMode(u32 mode);

// change cd thread priority
// 
// SUPPORTED IN XCDVDMAN/XCDVDFSV ONLY
// 
// args:	media mode
// returns:	1 if successful
//			0 if error
s32  cdChangeThreadPriority(u32 priority);


// **** Stream Functions ****


// start streaming data
// 
// args:	sector location to start streaming from
//			mode to read in
// returns:	1 if successful
//			0 otherwise
s32  cdStStart(u32 sectorLoc, CdvdReadMode_t *mode);

// read stream data
// 
// args:	number of sectors to read
//			read buffer address
//			data stream mode (CDVD_STREAM_BLOCK oo CDVD_STREAM_NONBLOCK)
//			error value holder
// returns:	number of sectors read if successful
//			0 otherwise
s32  cdStRead(u32 sectorSize, u32 *buffer, u32 mode, u32 *error);

// stop streaming
// 
// returns:	1 if successful
//			0 otherwise
s32  cdStStop(void);

// seek to a new stream position
// 
// args:	sector location to start streaming from
// returns:	1 if successful
//			0 otherwise
s32  cdStSeek(u32 sectorLoc);

// init streaming
// 
// args:	stream buffer size
//			number of ring buffers
//			buffer address on iop
// returns:	1 if successful
//			0 otherwise
s32  cdStInit(u32 buffSize, u32 numBuffers, void *buf);

// get stream read status
// 
// returns:	number of sectors read if successful
//			0 otherwise
s32  cdStStat(void);

// pause streaming
// 
// returns:	1 if successful
//			0 otherwise
s32  cdStPause(void);

// continue streaming
// 
// returns:	1 if successful
//			0 otherwise
s32  cdStResume(void);

/* Low-level stream dispatch.  */
int cdStream(u32 lbn, u32 nsectors, void *buf, CdvdStCmd_t cmd, CdvdReadMode_t *rm);
int cdCddaStream(u32 lbn, u32 nsectors, void *buf, CdvdStCmd_t cmd, CdvdReadMode_t *rm);


// **** Other Functions ****


// init cdvd system
// 
// args:	init mode (CDVD_INIT_???)
// returns:	1 if successful
//			0 if error
s32  cdInit(s32 mode);

// waits/checks for completion of n-commands
// 
// args:	0 = wait for completion of command (blocking)
//			1 = check current status and return immediately
// returns:	0 = completed
//			1 = not completed
s32  cdSync(s32 mode);

// search for a file on disc
// 
// args:	file structure to get file info in
//			name of file to search for (no wildcard characters)
//				(should be in the form '\\SYSTEM.CNF;1')
// returns:	1 if successful
//			0 if error (or no file found)
s32  cdSearchFile(CdvdFileSpec_t *file, const char *name);

// checks if drive is ready
// 
// args:	 mode
// returns:	2 if ready
//			6 if busy
s32  cdDiskReady(s32 mode);

// convert from sector number to minute:second:frame
CdvdLocation_t *cdIntToPos(s32 i, CdvdLocation_t *p);

// convert from minute:second:frame to sector number
s32  cdPosToInt(CdvdLocation_t *p);

// get the current read position (when reading using cdRead)
u32  cdGetReadPos(void);

// initialise callback thread
// 
// args:	callback thread priority
//			callback thread stack address
//			callback thread stack size
// returns:	1 if initialised callback
//			0 if only priority was changed
s32 cdInitCallbackThread(s32 priority, void *stackAddr, s32 stackSize);

// set cd callback function
// 
// args:	pointer to new callback function
// returns:	pointer to old function
CdCBFunc cdSetCallback(CdCBFunc func);


#ifdef __cplusplus
}
#endif


#endif // _LIBCDVD_H_


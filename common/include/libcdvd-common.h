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
 * Common definitions for libcdvd on the EE and IOP
 */

#ifndef __LIBCDVD_COMMON_H__
#define __LIBCDVD_COMMON_H__

#include <tamtypes.h>

/* File open modes */
/** Open file for streaming */
#define SCE_CdSTREAM	0x40000000 

enum SCECdvdSectorType{
	/* Game CD and DVD sector types.  */
	SCECdSecS2048=0,
	SCECdSecS2328,
	SCECdSecS2340,

	/* CDDA sector types.  */
	SCECdSecS2352=0,
	SCECdSecS2368,
	SCECdSecS2448,
};

enum SCECdvdSpinValue{
	/** Spin at maximum speed */
	SCECdSpinMax=0,	
	/** Spins at the recommended speed for streaming data */
	SCECdSpinStm=0,	
	/** Spins at the DVD layer 0 speed */
	SCECdSpinDvdDL0=0,

	/** Optimized speed. Starts reading at max speed, slows down as read errors occur. */
	SCECdSpinNom=1,
	/** 1x. For CDDA reading. */
	SCECdSpinX1,
	//Note: These modes were taken from the PS2Linux libcdvd.h file, and they don't appear in any other Sony libcdvd.h file.
	/** 2x */
	SCECdSpinX2,
	/** 4x */
	SCECdSpinX4,
	/** 12x */
	SCECdSpinX12,
	/** Optimized speed, based on current speed. */
	SCECdSpinNm2=10,
	/** DVD x1.6 CLV */
	SCECdSpin1p6,
	/** Spin at maximum speed (Not sure what's the difference between this and SCECdSpinMax) */
	SCECdSpinMx=20,
};

enum SCECdvdMModeMediaType{
	SCECdMmodeCd=1,
	SCECdMmodeDvd
};

enum SCECdvdErrorCode{
	/** Can't get error code */
	SCECdErFAIL	= -1,
	/** No Error */
	SCECdErNO	= 0x00,
	/** Aborted */
	SCECdErABRT,

	/** Unsupported command */
	SCECdErCMD	= 0x10,
	/** Tray is open */
	SCECdErOPENS,
	/** No disc inserted */
	SCECdErNODISC,
	/** Device not ready */
	SCECdErNORDY,
	/** Unsupported command for current disc */
	SCECdErCUD,

	/** Illegal position/LSN */
	SCECdErIPI	= 0x20,
	/** Illegal length */
	SCECdErILI,
	/** Invalid parameter */
	SCECdErPRM,

	/** Read error */
	SCECdErREAD	= 0x30,
	/** Tray was opened while reading */
	SCECdErTRMOPN,
	/** End Of Media */
	SCECdErEOM,
	SCECdErSFRMTNG	= 0x38,

	/** Error setting command */
	SCECdErREADCF	= 0xFD,
	/** Error setting command */
	SCECdErREADCFR
};

enum SCECdvdMediaType{
	SCECdGDTFUNCFAIL	= -1,
	SCECdNODISC		= 0x00,
	SCECdDETCT,
	SCECdDETCTCD,
	SCECdDETCTDVDS,
	SCECdDETCTDVDD,
	SCECdUNKNOWN,

	SCECdPSCD		= 0x10,
	SCECdPSCDDA,
	SCECdPS2CD,
	SCECdPS2CDDA,
	SCECdPS2DVD,

	SCECdCDDA		= 0xFD,
	SCECdDVDV,
	SCECdIllegalMedia
};

enum SCECdvdInterruptCode{
	/** No interrupt */
	CdlNoIntr	= 0x00,
	/** Data Ready */
	CdlDataReady,
	/** Command Complete */
	SCECdComplete,
	/** Acknowledge (reserved) */
	CdlAcknowledge,
	/** End of Data Detected */
	CdlDataEnd,
	/** Error Detected */
	CdlDiskError,
	/** Drive Not Ready */
	SCECdNotReady

};

//Tray request modes
enum SCECdvdTrayReqMode{
	SCECdTrayOpen=0,
	SCECdTrayClose,
	SCECdTrayCheck
};

//Drive states
enum SCECdvdDriveState{
	SCECdStatStop	= 0x00,
	SCECdStatShellOpen,
	SCECdStatSpin,
	SCECdStatRead	= 0x06,
	SCECdStatPause	= 0x0A,
	SCECdStatSeek	= 0x12,
	SCECdStatEmg	= 0x20,
};

typedef struct {
	u8 stat;
	u8 second;
	u8 minute;
	u8 hour;
	u8 pad;
	u8 day;
	u8 month;
	u8 year;
} sceCdCLOCK;

typedef struct {
	u32 lsn;
	u32 size;
	char name[16];
	u8 date[8];
} sceCdlFILE;

// location structure, used with sceCdIntToPos() and sceCdPosToInt()
typedef struct {
	/** minute (BCD) */
	u8 minute;
	/** second (BCD) */
	u8 second;
	/** sector (BCD) */
	u8 sector;
	/** track (void), aka "frame" */
	u8 track;
} sceCdlLOCCD;

typedef struct {
	u8 trycount;
	u8 spindlctrl;
	u8 datapattern;
	u8 pad;
} sceCdRMode;

typedef struct {
	/** sector location to start reading from */
	u32 lbn;
	/** number of sectors to read */
	u32 sectors;
	/** buffer address to read to ( bit0: 0=EE, 1=IOP ) (EE addresses must be on 64-byte alignment) */
	u32 buffer;
} sceCdRChain;

// macros for converting between an integer and a BCD number
#ifndef btoi
#define btoi(b)		((b)/16*10 + (b)%16)	// BCD to int
#endif
#ifndef itob
#define itob(i)		((i)/10*16 + (i)%10)	// int to BCD
#endif

/** max number of toc entries for sceCdGetToc() */
#define CdlMAXTOC	100

enum SCECdvdCallbackReason{
	SCECdFuncRead	= 1,
	SCECdFuncReadCDDA,
	SCECdFuncGetToc,
	SCECdFuncSeek,
	SCECdFuncStandby,
	SCECdFuncStop,
	SCECdFuncPause,
	SCECdFuncBreak
};

/** sceCd callback function typedef for sceCdCallback() */
typedef void (*sceCdCBFunc)(int reason);

enum SCECdvdInitMode{
	/** Initializes library and waits until commands can be sent. */
	SCECdINIT	= 0x00,
	/** Initialize only the library. */
	SCECdINoD,
	/** Deinitialize library. */
	SCECdEXIT	= 0x05
};

// Low-level filesystem properties for sceCdSearchFile() 
/** Maximum number of files in a directory. */
#define CdlMAXFILE	64
/** Maximum number of total directories. */
#define CdlMAXDIR	128
/** Maximum levels of directories. */
#define CdlMAXLEVEL	8


// For streaming operations (Use with sceCdStRead())
enum SCECdvdStreamMode{
	/** Stream without blocking. */
	STMNBLK	= 0,
	/** Stream, but block. */
	STMBLK 
};

// EE read modes (Used with sceCdSetEEReadMode()).
#define SCECdNoCheckReady	0x00000001
#define SCECdNoWriteBackDCache	0x00000002

#ifdef __cplusplus
extern "C" {
#endif

// **** N-Command Functions ****

/** read data from disc
 * non-blocking, requires sceCdSync() call
 * 
 * @param lbn sector location to start reading from
 * @param sectors number of sectors to read
 * @param buffer buffer to read to
 * @param mode mode to read as
 * @return 1 on success, 0 on failure.
 */
int sceCdRead(u32 lbn, u32 sectors, void *buffer, sceCdRMode *mode);
int sceCdReadDVDV(u32 lbn, u32 sectors, void *buffer, sceCdRMode *mode);
int sceCdReadCDDA(u32 lbn, u32 sectors, void *buffer, sceCdRMode *mode);

/** get toc from inserted disc
 * 
 * @param toc buffer to hold toc (1024 bytes)
 * @return 1 on success, 0 on failure.
 */
int sceCdGetToc(u8 *toc);

/** Alternate TOC retrieving function with parameter
 *
 * @param toc buffer to hold toc (1024 bytes)
 * @param param Parameter
 * @return 1 on success, 0 on failure.
 */
int sceCdGetToc2(u8 *toc, int param);

/** seek to given sector on disc
 * non-blocking, requires sceCdSync() call
 * 
 * @param lbn sector to seek to on disc
 * @return 1 on success, 0 on failure.
 */
int sceCdSeek(u32 lbn);

/** puts ps2 sceCd drive into standby mode
 * non-blocking, requires sceCdSync() call
 * 
 * @return 1 on success, 0 on failure.
 */
int sceCdStandby(void);

/** stops ps2 sceCd drive from spinning
 * non-blocking, requires sceCdSync() call
 * 
 * @return 1 on success, 0 on failure.
 */
int sceCdStop(void);

/** pauses ps2 sceCd drive
 * non-blocking, requires sceCdSync() call
 * 
 * @return 1 on success, 0 on failure.
 */
int sceCdPause(void);

/** do a 'chain' of reads with one command
 * last chain value must be all 0xFFFFFFFF
 * (max of 64 reads can be set at once)
 * non-blocking, requires sceCdSync() call
 * 
 * SUPPORTED IN XCDVDMAN/XCDVDFSV ONLY
 * 
 * @param tag pointer to an array of read chain data
 * @param mode read mode
 * @return 1 on success, 0 on failure.
 */
int sceCdReadChain(sceCdRChain *tag, sceCdRMode *mode);

// **** S-Command functions ****

/** Reads the PlayStation clock.
 * (time value is in BCD)
 *
 * NOTE: For the old CDVDMAN module, the month field will not have its most significant bit automatically filed off.
 *
 * @param clock time/date struct
 * @return 1 on success, 0 on failure.
 */
int sceCdReadClock(sceCdCLOCK *clock);

/** Sets the PlayStation 2 clock.
 * (time value is in BCD)
 * 
 * @param clock time/date struct to set clocks time with
 * @return 1 on success, 0 on failure.
 */
int sceCdWriteClock(const sceCdCLOCK *clock);

/** gets the type of the currently inserted disc
 * 
 * @return disk type (SCECdvdMediaTypes)
 */
int sceCdGetDiskType(void);

/** gets the last error that occurred
 * 
 * @return error type (SCECdvdErrorCode)
 */
int sceCdGetError(void);

/** open/close/check disk tray
 * 
 * @param param param (SCECdvdTrayReqMode)
 * @param traychk address for returning tray state change
 * @return 1 on success, 0 on failure.
 */
int sceCdTrayReq(int param, u32 *traychk);

/** gets the state of the drive
 * 
 * @return status (SCECdvdDriveStates)
 */
int sceCdStatus(void);

/** 'breaks' the currently executing command
 * 
 * @return 1 on success, 0 on failure.
 */
int sceCdBreak(void);

/** cancel power off
 * 
 * SUPPORTED IN XCDVDMAN/XCDVDFSV ONLY
 * 
 * @param result result
 * @return 1 on success, 0 on failure.
 */
int sceCdCancelPOffRdy(u32* result);

/** blue led control
 * 
 * SUPPORTED IN XCDVDMAN/XCDVDFSV ONLY
 * 
 * @param control control value
 * @param result result
 * @return 1 on success, 0 on failure.
 */
int sceCdBlueLEDCtl(u8 control, u32 *result);

/** power off
 * 
 * SUPPORTED IN XCDVDMAN/XCDVDFSV ONLY
 * 
 * @param result result
 * @return 1 on success, 0 on failure.
 */
int sceCdPowerOff(u32 *result);

/** set media mode
 * 
 * SUPPORTED IN XCDVDMAN/XCDVDFSV ONLY
 * 
 * @param media media mode (SCECdvdMModeMediaTypes)
 * @return 1 on success, 0 on failure.
 */
int sceCdMmode(int media);

/** change libcdvd thread priority
 * 
 * SUPPORTED IN XCDVDMAN/XCDVDFSV ONLY
 * 
 * @param priority priority
 * @return 1 on success, 0 on failure.
 */
int sceCdChangeThreadPriority(int priority);


// **** Streaming Functions ****

/** start streaming data
 * 
 * @param lbn sector location to start streaming from
 * @param mode mode to read in
 * @return 1 on success, 0 on failure.
 */
int sceCdStStart(u32 lbn, sceCdRMode *mode);

/** read stream data
 * 
 * @param sectors number of sectors to read
 * @param buffer read buffer address
 * @param mode data stream mode (STMNBLK or STMBLK)
 * @param error error value holder
 * @return number of sectors read if successful, 0 otherwise
 */
int sceCdStRead(u32 sectors, u32 *buffer, u32 mode, u32 *error);

/** stop streaming
 * 
 * @return 1 on success, 0 on failure.
 */
int sceCdStStop(void);

/** seek to a new stream position
 * 
 * @param lbn sector location to start streaming from
 * @return 1 on success, 0 on failure.
 */
int sceCdStSeek(u32 lbn);

/** init streaming
 * 
 * @param bufmax stream buffer size
 * @param bankmax number of ring buffers
 * @param buffer buffer address on iop
 * @return 1 on success, 0 on failure.
 */
int sceCdStInit(u32 bufmax, u32 bankmax, void *buffer);

/** get stream read status
 * 
 * @return number of sectors read if successful, 0 otherwise
 */
int sceCdStStat(void);

/** pause streaming
 * 
 * @return 1 on success, 0 on failure.
 */
int sceCdStPause(void);

/** continue streaming
 * 
 * @return 1 on success, 0 on failure.
 */
int sceCdStResume(void);

// **** Other Functions ****

/** initializes libcdvd
 * 
 * @param mode mode (SCECdvdInitModes)
 * @return 1 on success, 0 on failure.
 */
int sceCdInit(int mode);

/** waits/checks for completion of n-commands
 * 
 * @param mode 0 = wait for completion of command (blocking), 1 = check current status and return immediately
 * @return 0 = completed, 1 = not completed
 */
int sceCdSync(int mode);

/** search for a file on disc
 * 
 * @param file file structure to get file info in
 * @param name name of file to search for (no wildcard characters) (should be in the form '\\SYSTEM.CNF;1')
 * @return 1 on success, 0 on failure. (or no file found)
 */
int sceCdSearchFile(sceCdlFILE *file, const char *name);

/** checks if drive is ready
 * 
 * @param mode mode
 * @return SCECdComplete if ready, SCECdNotReady if busy
 */
int sceCdDiskReady(int mode);

/** convert from sector number to minute:second:frame  */
sceCdlLOCCD *sceCdIntToPos(u32 i, sceCdlLOCCD *p);

/** convert from minute:second:frame to sector number  */
u32 sceCdPosToInt(sceCdlLOCCD *p);

/** get the current read position (when reading using sceCdRead)  */
u32 sceCdGetReadPos(void);

/** initialise EE callback thread
 * 
 * @param priority callback thread priority
 * @param stackAddr callback thread stack address
 * @param stackSize callback thread stack size
 * @return 1 if initialised callback, 0 if only priority was changed
 */
int sceCdInitEeCB(int priority, void *stackAddr, int stackSize);

/** set sceCd callback function
 * 
 * @param function pointer to new callback function
 * @return pointer to old function
 */
sceCdCBFunc sceCdCallback(sceCdCBFunc function);

/** Reads SUBQ data from the disc.
//
// @param buffer Pointer to the buffer for storing the SUBQ data in.
// @param status Result code.
// @return 1 on success, 0 on failure.
 */
int sceCdReadSUBQ(void *buffer, u32 *status);

// **** System functions ****

/** Disables (Forbids) the DVD READ N-command, so that DVD video disc sectors cannot be read.
 * Support for the DVD READ N-command is re-enabled when a DVD player is loaded.
 *
 * @param result Result code.
 * @return 1 on success, 0 on failure.
 */
int sceCdForbidDVDP(u32 *result);

/** Controls automatic adjustment of the CD/DVD drive.
 * This also causes the auto-tilt motor (on units that have one) to be activated.
 *
 * @param mode Mode
 * @param result Result code.
 * @return 1 on success, 0 on failure.
 */
int sceCdAutoAdjustCtrl(int mode, u32 *result);

/** Controls on-the-fly (hardware) data decryption. Setting all options to 0 will disable decryption.
 * This is used for decrypting encrypted sectors like the PlayStation 2 logo.
 *
 * @param arg1 Unknown
 * @param arg2 Unknown
 * @param shift Shift amount
 * @return 1 on success, 0 on failure.
 */
int sceCdDecSet(unsigned char arg1, unsigned char arg2, unsigned char shift);

/** Reads the requested key from the CD/DVD.
 *
 * @param arg1 Unknown
 * @param arg2 Unknown
 * @param command Command
 * @param key Buffer to store the key in.
 * @return 1 on success, 0 on failure.
 */
int sceCdReadKey(unsigned char arg1, unsigned char arg2, unsigned int command, unsigned char *key);

/** Sets the "HD mode", whatever that means.
 *
 * @param mode mode
 * @return 1 on success, 0 on failure.
 */
int sceCdSetHDMode(u32 mode);

/** Opens a specified configuration block, within NVRAM. Each block is 15 bytes long.
 *
 * @param block Block number.
 * @param mode Mode (0 = read, 1 = write).
 * @param NumBlocks Number of blocks.
 * @param status Result code.
 * @return 1 on success, 0 on failure.
 */
int sceCdOpenConfig(int block, int mode, int NumBlocks, u32 *status);

/** Closes the configuration block.
 *
 * @param result Result code.
 * @return 1 on success, 0 on failure.
 */
int sceCdCloseConfig(u32 *result);

/** Reads the configuration block(s) that was/were previously opened. Each block is 15 bytes long.
 *
 * @param buffer Pointer to the buffer to store the configuration data in.
 * @param result Result code.
 * @return 1 on success, 0 on failure.
 */
int sceCdReadConfig(void *buffer, u32 *result);

/** Writes to the configuration block(s) that was/were previously opened. Each block is 15 bytes long.
 *
 * @param buffer Pointer to the buffer that contains the new configuration data.
 * @param result Result code.
 * @return 1 on success, 0 on failure.
 */
int sceCdWriteConfig(const void *buffer, u32 *result);

/** Reads a single word from the NVRAM storage.
 *
 * @param address Address in 2-byte words, of the word that will be read.
 * @param data Pointer to the buffer that will contain the data read.
 * @param result Result code.
 * @return 1 on success, 0 on failure.
 */
int sceCdReadNVM(u32 address, u16 *data, u8 *result);

/** Writes a single word to the NVRAM storage.
 * Does not fully work on consoles starting from the SCPH-50000 ("Dragon" MECHACON):
 * Some parts, like those containing the console IDs, cannot be overwritten.
 *
 * @param address Address in 2-byte words, of the word that will be written to.
 * @param data Pointer to the buffer that contains the new data.
 * @param result Result code.
 * @return 1 on success, 0 on failure.
 */
int sceCdWriteNVM(u32 address, u16 data, u8 *result);

/** Reads the i.Link ID of the console.
 * All consoles have an i.Link ID, including those that do not have a physical i.Link port.
 *
 * @param buffer Pointer to the buffer that will contain the data read.
 * @param result Result code.
 * @return 1 on success, 0 on failure.
 */
int sceCdRI(u8 *buffer, u32 *result);

/** Writes a new i.Link ID for the console.
 * Does not work on consoles starting from the SCPH-50000 ("Dragon" MECHACON):
 *
 * @param buffer Pointer to the buffer that contains the new i.Link ID.
 * @param result Result code.
 * @return 1 on success, 0 on failure.
 */
int sceCdWI(const u8 *buffer, u32 *result);

/** Reads the ID of the console. This is not the same as the i.Link ID.
 *
 * @param buffer Pointer to the buffer that will contain the data read.
 * @param result Result code.
 * @return 1 on success, 0 on failure.
 */
int sceCdReadConsoleID(u8 *buffer, u32 *result);

/** Writes a new ID for the console. This is not the same as the i.Link ID.
 * Does not work on consoles starting from the SCPH-50000 ("Dragon" MECHACON):
 *
 * @param buffer Pointer to the buffer that contains the new i.Link ID.
 * @param status Result code.
 * @return 1 on success, 0 on failure.
 */
int sceCdWriteConsoleID(const u8 *buffer, u32 *status);

/** Controls Audio Digital output.
 *
 * @param arg1 Unknown
 * @param status Result code.
 * @return 1 on success, 0 on failure.
 */
int sceCdCtrlADout(int arg1, u32 *status);

/** Reads MECHACON version data (RR MM mm TT): RR = Magicgate region, MM = major, mm = minor, TT = system type (00 = PS2, 01 = PSX)
 * Magicgate region codes are: 00 = Japan, 01 = USA, 02 = Europe, 03 = Oceania, 04 = Asia, 05 = Russia, 06 = China, and 07 = Mexico.
 *
 * NOTE: The old CDVDMAN module only returns MM mm TT instead. The region byte is made up of the lower 7 bits of the status/result code.
 *
 * @param buffer Pointer to the buffer to store the version data in.
 * @param status Result code.
 * @return 1 on success, 0 on failure.
 */
int sceCdMV(u8 *buffer, u32 *status);

/** Performs bootup certification.
 * SUPPORTED IN XCDVDMAN/XCDVDFSV ONLY
 *
 * @param romname ROM version - The ROM "name" of the console in this format:
 *				romname[0]	- ROM major version number (In decimal).
 *				romname[1]	- ROM minor version number (In decimal).
 *				romname[2]	- ROM region.
 *				romname[3]	- Console type (E.g. C, D or T).
 * @return 1 on success, 0 on failure.
 */
int sceCdBootCertify(const u8 *romname);

/** Reads the console's model "name": e.g. "SCPH-39000".
 * SUPPORTED IN XCDVDMAN/XCDVDFSV ONLY
 *
 * @param buffer Pointer to the buffer for storing the model name in.
 * @param status Result code.
 * @return 1 on success, 0 on failure.
 */
int sceCdRM(char *buffer, u32 *status);

/** Sets the console's model name.
 * Does not work on consoles starting from the SCPH-50000 ("Dragon" MECHACON).
 * SUPPORTED IN XCDVDMAN/XCDVDFSV ONLY
 *
 * @param buffer Pointer to a buffer containing the new model name.
 * @param status Result code.
 * @return 1 on success, 0 on failure.
 */
int sceCdWM(const char *buffer, u32 *status);

/** Disables (Forbids) the READ N-command, so that disc sectors cannot be read.
 * Support for the READ N-command is re-enabled when the disc is replaced.
 * SUPPORTED IN XCDVDMAN/XCDVDFSV ONLY
 *
 * @param result Result code.
 * @return 1 on success, 0 on failure.
 */
int sceCdForbidRead(u32 *result);

// **** Extra Functions ****

/** Reads sectors to a buffer
 * SUPPORTED IN NEWER CDVDMAN MODULES INCLUDED WITHIN NEWER IOPRP ONLY
 *
 * @param lsn sector location to start from
 * @param sectors amount to sectors to read
 * @param buf pointer to a buffer
 * @param mode mode to read in
 * @return 1 on success, 0 on failure.
 */
int sceCdReadFull(unsigned long int lsn, unsigned long int sectors, void *buf, sceCdRMode *mode);

/** Seeks to a given position
 * SUPPORTED IN NEWER CDVDMAN MODULES INCLUDED WITHIN NEWER IOPRP ONLY
 *
 * @param lsn sector location to seek to
 * @return 1 on success, 0 on failure.
 */
int sceCdStSeekF(unsigned long int lsn);

/** Sets a handler for the power-off event which will be called before the console switches off.
 * SUPPORTED IN NEWER CDVDMAN MODULES INCLUDED WITHIN NEWER IOPRP ONLY
 *
 * @param func pointer of callback function to be called when power-off processing is activated
 * @param addr an argument which will be passed to the callback function
 * returns:	a pointer to the previous handler function, or a null pointer if nothing has been set eariler
 */
void *sceCdPOffCallback(void(*func)(void *),void *addr);

/** Sets the timeout lengths for the certain CDVDMAN's operations.
 * SUPPORTED IN NEWER CDVDMAN MODULES INCLUDED WITHIN NEWER IOPRP ONLY
 *
 * @param param the timeout type to set
 * @param timeout a timeout value
 * @return 1 on success, 0 on failure.
 */
int sceCdSetTimeout(int param, int timeout);

/** Reads the Model ID.
 * SUPPORTED IN NEWER CDVDMAN MODULES INCLUDED WITHIN DNAS IOPRP ONLY
 *
 * @param id integer where the Model ID is stored.
 * @return 1 on success, 0 on failure.
 */
int sceCdReadModelID(unsigned long int *id);

/** Reads the information about DVD disk.
 * SUPPORTED IN NEWER CDVDMAN MODULES INCLUDED WITHIN NEWER IOPRP ONLY
 *
 * @param on_dual pointer to variable where type of DVD disc is returned
 * @param layer1_start pointer to variable where the address of the second layer of a dual-layer DVD disc is returned.
 * @return 1 on success, 0 on failure.
 */
int sceCdReadDvdDualInfo(int *on_dual, unsigned long int *layer1_start);

/** Retrieves basic information about a file on CD or the specified layer of DVD media.
 * SUPPORTED IN NEWER CDVDMAN MODULES INCLUDED WITHIN NEWER IOPRP ONLY
 *
 * @param fp file structure to get file info in
 * @param path name of file to search for (no wildcard characters) (should be in the form '\\SYSTEM.CNF;1')
 * @param layer layer to search (0 for the first layer, 1 for the second layer)
 * @return 1 on success, 0 on failure.
 */
int sceCdLayerSearchFile(sceCdlFILE *fp, const char *path, int layer);

/** Returns the CD/DVD drive status.
 * SUPPORTED IN NEWER CDVDMAN MODULES INCLUDED WITHIN NEWER IOPRP ONLY
 *
 * @return status
 */
int sceCdStatus2(void);

/** Reads sectors from CD or DVD disc.
 * SUPPORTED IN NEWER CDVDMAN MODULES INCLUDED WITHIN NEWER IOPRP ONLY
 *
 * @param lsn sector location to start from
 * @param sectors amount to sectors to read
 * @param buf pointer to a buffer
 * @param mode mode to read in
 * @return 1 on success, 0 on failure.
 */
int sceCdRE(unsigned long int lsn,unsigned long int sectors,void *buf,sceCdRMode *mode);

/** Reads a GUID.
 * SUPPORTED IN NEWER CDVDMAN MODULES INCLUDED WITHIN DNAS IOPRP ONLY
 *
 * @param guid u64 integer where the GUID is stored.
 * @return 1 on success, 0 on failure.
 */
int sceCdReadGUID(u64 *guid);

/** Controls remote-control bypass
 * SUPPORTED BY ONLY DESR/PSX DVR CDVDMAN MODULES
 *
 * @param mode 0 = Bypass, 1 = Normal
 * @param status Result code.
 * @return 1 on success, 0 on failure
 */
int sceCdRcBypassCtl(int mode, u32 *status);

/** Reads wake up time.
 * SUPPORTED IN XCDVDMAN INCLUDED WITHIN NEWER BOOT ROMS ONLY
 *
 * @return 1 on success, 0 on failure
 */
int sceCdReadWakeUpTime(sceCdCLOCK *clock, u16 *arg2, u32 *arg3, int *arg4);

/** Writes wake up time.
 * SUPPORTED IN XCDVDMAN INCLUDED WITHIN NEWER BOOT ROMS ONLY
 *
 * @return 1 on success, 0 on failure
 */
int sceCdWriteWakeUpTime(const sceCdCLOCK *clock, u16 arg2, int arg3);

/** Remote control 2_7.
 * SUPPORTED IN XCDVDMAN INCLUDED WITHIN NEWER BOOT ROMS ONLY
 *
 * @return 1 on success, 0 on failure
 */
int sceRemote2_7(u16 a1, u32 *a2);

/** Set LEDs mode.
 * SUPPORTED IN XCDVDMAN INCLUDED WITHIN NEWER BOOT ROMS ONLY
 *
 * @return 1 on success, 0 on failure
 */
int sceCdSetLEDsMode(u32 arg1, u32 *result);

/** Reads PS1 boot parameter.
 * SUPPORTED IN XCDVDMAN INCLUDED WITHIN NEWER BOOT ROMS ONLY
 *
 * @return 1 on success, 0 on failure
 */
int sceCdReadPS1BootParam(u8 *out, u32 *result);

/** Sets fan profile.
 * SUPPORTED IN XCDVDMAN INCLUDED WITHIN NEWER BOOT ROMS ONLY
 *
 * @return 1 on success, 0 on failure
 */
int sceCdSetFanProfile(u8 arg1, u32 *result);

/** Change sys.
 * SUPPORTED BY ONLY DESR/PSX DVR CDVDMAN MODULES
 *
 * @return 1 on success, 0 on failure
 */
int sceCdChgSys(u32 arg1);

/** Notice game start.
 * SUPPORTED BY ONLY DESR/PSX DVR CDVDMAN MODULES
 *
 * @return 1 on success, 0 on failure
 */
int sceCdNoticeGameStart(u8 arg1, u32 *result);

/** Extended LED control.
 * SUPPORTED BY ONLY DESR/PSX DVR CDVDMAN MODULES
 *
 * @return 1 on success, 0 on failure
 */
int sceCdXLEDCtl(u8 arg1, u8 arg2, u32 *result1, u32 *result2);

/** Buzzer control.
 * SUPPORTED BY ONLY DESR/PSX DVR CDVDMAN MODULES
 *
 * @return 1 on success, 0 on failure
 */
int sceCdBuzzerCtl(u32 *result);

/** XBS power control.
 * SUPPORTED BY ONLY DESR/PSX DVR CDVDMAN MODULES
 *
 * @return 1 on success, 0 on failure
 */
int sceCdXBSPowerCtl(u8 arg1, u8 arg2, u32 *result1, u32 *result2);

/** Set medium removal.
 * SUPPORTED BY ONLY DESR/PSX DVR CDVDMAN MODULES
 *
 * @return 1 on success, 0 on failure
 */
int sceCdSetMediumRemoval(u8 arg1, u32 *result);

/** Get medium removal.
 * SUPPORTED BY ONLY DESR/PSX DVR CDVDMAN MODULES
 *
 * @return 1 on success, 0 on failure
 */
int sceCdGetMediumRemoval(u32 *result1, u32 *result2);

/** XDVRP reset.
 * SUPPORTED BY ONLY DESR/PSX DVR CDVDMAN MODULES
 *
 * @return 1 on success, 0 on failure
 */
int sceCdXDVRPReset(u8 arg1, u32 *result);

/** Get wake up reason.
 * SUPPORTED BY ONLY DESR/PSX DVR CDVDMAN MODULES
 *
 * @return 1 on success, 0 on failure
 */
int sceCdGetWakeUpReason(void);

/** Reads region parameters.
 * SUPPORTED IN XCDVDMAN INCLUDED WITHIN NEWER BOOT ROMS ONLY
 *
 * @return 1 on success, 0 on failure
 */
int sceCdReadRegionParams(u32 *arg1, u32 *result);

/** Writes region parameters.
 * SUPPORTED IN XCDVDMAN INCLUDED WITHIN NEWER BOOT ROMS ONLY
 *
 * @return 1 on success, 0 on failure
 */
int sceCdWriteRegionParams(u8 arg1, u32 *arg2, u8 *arg3, u32 *result);

#ifdef __cplusplus
}
#endif

#endif /* __LIBCDVD_COMMON_H__ */

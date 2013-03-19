/*
# _____     ___ ____     ___ ____
#  ____|   |    ____|   |        | |____|
# |     ___|   |____ ___|    ____| |    \    PS2DEV Open Source Project.
#-----------------------------------------------------------------------
# Copyright 2001-2004, ps2dev - http://www.ps2dev.org
# Licenced under Academic Free License version 2.0
# Review ps2sdk README & LICENSE files for further details.
#
# $Id: libcdvd.h  $
# Common definitions for libcdvd on the EE and IOP
*/

#ifndef _LIBCDVD_H_
#define _LIBCDVD_H_

/* File open modes */
#define SCE_CdSTREAM	0x40000000  /* Open file for streaming */

enum SCECdvdSectorTypes{
	/* Game CD and DVD sector types.  */
	SCECdSecS2048=0,
	SCECdSecS2328,
	SCECdSecS2340,

	/* CDDA sector types.  */
	SCECdSecS2352=0,
	SCECdSecS2368,
	SCECdSecS2448,
};

enum SCECdvdSpinValues{
	SCECdSpinMax=0,		//Spin at maximum speed
	SCECdSpinStm=0,		//Spins at the recommended speed for streaming data
	SCECdSpinDvdDL0=0,	//Spins at the DVD layer 0 speed

	SCECdSpinNom=1,		//Optimized speed. Starts reading at max speed, slows down as read errors occur.
	SCECdSpinX1,		//1x. For CDDA reading.
	//Note: These modes were taken from the PS2Linux libcdvd.h file, and they don't appear in any other Sony libcdvd.h file.
	SCECdSpinX2,		//2x
	SCECdSpinX4,		//4x
	SCECdSpinX12,		//12x
	SCECdSpinNm2=10,	//Optimized speed, based on current speed.
	SCECdSpin1p6,		//DVD x1.6 CLV
	SCECdSpinMx=20,		//Spin at maximum speed (Not sure what's the difference between this and SCECdSpinMax)
};

enum SCECdvdMModeMediaTypes{
	SCECdMmodeCd=1,
	SCECdMmodeDvd
};

enum SCECdvdErrorCodes{
	SCECdErFAIL	= -1,	/* Can't get error code		*/
	SCECdErNO	= 0x00,	/* No Error			*/
	SCECdErABRT,		/* Aborted			*/

	SCECdErCMD	= 0x10,	/* Unsupported command		*/
	SCECdErOPENS,		/* Tray is open			*/
	SCECdErNODISC,		/* No disc inserted		*/
	SCECdErNORDY,		/* Device not ready		*/
	SCECdErCUD,		/* Unsupported command for current disc */

	SCECdErIPI	= 0x20,	/* Illegal position/LSN		*/
	SCECdErILI,		/* Illegal length		*/
	SCECdErPRM,		/* Invalid parameter		*/

	SCECdErREAD	= 0x30,	/* Read error			*/
	SCECdErTRMOPN,		/* Tray was opened while reading */
	SCECdErEOM,		/* End of Media			*/
	SCECdErSFRMTNG	= 0x38,

	SCECdErREADCF	= 0xFD,	/* Error setting command */
	SCECdErREADCFR		/* Error setting command */
};

enum SCECdvdMediaTypes{
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

enum SCECdvdInterruptCodes{
	CdlNoIntr	= 0x00,	/* No interrupt		  */
	CdlDataReady,		/* Data Ready		  */
	SCECdComplete,		/* Command Complete 	  */
	CdlAcknowledge,		/* Acknowledge (reserved) */
	CdlDataEnd,		/* End of Data Detected   */
	CdlDiskError,		/* Error Detected 	  */
	SCECdNotReady		/* Drive Not Ready	  */
};

//Tray request modes
enum SCECdvdTrayReqModes{
	SCECdTrayOpen=0,
	SCECdTrayClose,
	SCECdTrayCheck
};

//Drive states
enum SCECdvdDriveStates{
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

// sceCdvd location struct, used with sceCdIntToPos() and sceCdPosToInt()
typedef struct {
	u8 minute;	// minute (BCD)
	u8 second;	// second (BCD)
	u8 sector;	// sector (BCD)
	u8 track;	// track (void), aka "frame"
} sceCdlLOCCD;

typedef struct {
	u8 trycount;
	u8 spindlctrl;
	u8 datapattern;
	u8 pad;
} sceCdRMode;

typedef struct {
	u32 lbn;	// sector location to start reading from
	u32 sectors;	// number of sectors to read
	u32 buffer;	// buffer address to read to ( bit0: 0=EE, 1=IOP )
			// (EE addresses must be on 64-byte alignment)
} sceCdRChain;

// macros for converting between an integer and a BCD number
#ifndef btoi
#define btoi(b)		((b)/16*10 + (b)%16)	// BCD to int
#endif
#ifndef itob
#define itob(i)		((i)/10*16 + (i)%10)	// int to BCD
#endif

// max number of toc entries for sceCdGetToc()
#define CdlMAXTOC	100

enum SCECdvdCallbackReasons{
	SCECdFuncRead	= 1,
	SCECdFuncReadCDDA,
	SCECdFuncGetToc,
	SCECdFuncSeek,
	SCECdFuncStandby,
	SCECdFuncStop,
	SCECdFuncPause,
	SCECdFuncBreak
};

// sceCd callback function typedef for sceCdCallback()
typedef void (*CdCBFunc)(int reason);

enum SCECdvdInitModes{
	SCECdINIT	= 0x00,	/* Initializes library and waits until commands can be sent. */
	SCECdINoD,		/* Initialize only the library. */
	SCECdEXIT	= 0x05	/* Deinitialize library. */
};

// Low-level filesystem properties for sceCdSearchFile() 
#define CdlMAXFILE	64	/* Maximum number of files in a directory. */
#define CdlMAXDIR	128	/* Maximum number of total directories. */
#define CdlMAXLEVEL	8	/* Maximum levels of directories. */

// For streaming operations (Use with sceCdStRead())
enum SCECdvdStreamModes{
	STMNBLK	= 0,	// Stream without blocking.
	STMBLK		// Stream, but block.
};

// EE read modes (Used with sceCdSetEEReadMode()).
#define SCECdNoCheckReady       0x00000001
#define SCECdNoWriteBackDCache  0x00000002

#ifdef __cplusplus
extern "C" {
#endif

// **** N-Command Functions ****

// read data from disc
// non-blocking, requires sceCdSync() call
// 
// args:	sector location to start reading from
//			number of sectors to read
//			buffer to read to
//			mode to read as
// returns: 1 if successful
//			0 if error
int sceCdRead(u32 lbn, u32 sectors, void *buffer, sceCdRMode *mode);
int sceCdReadDVDV(u32 lbn, u32 sectors, void *buffer, sceCdRMode *mode);
int sceCdReadCDDA(u32 lbn, u32 sectors, void *buffer, sceCdRMode *mode);

// get toc from inserted disc
// 
// args:	buffer to hold toc (1024 bytes)
// returns:	1 if successful
//			0 otherwise
int sceCdGetToc(u8 *toc);

// seek to given sector on disc
// non-blocking, requires sceCdSync() call
// 
// args:	sector to seek to on disc
// returns:	1 if successful
//			0 if error
int sceCdSeek(u32 lbn);

// puts ps2 sceCd drive into standby mode
// non-blocking, requires sceCdSync() call
// 
// returns:	1 if successful
//			0 if error
int sceCdStandby(void);

// stops ps2 sceCd drive from spinning
// non-blocking, requires sceCdSync() call
// 
// returns:	1 if successful
//			0 if error
int sceCdStop(void);

// pauses ps2 sceCd drive
// non-blocking, requires sceCdSync() call
// 
// returns:	1 if successful
//			0 if error
int sceCdPause(void);

// send an n-command by function number
// 
// args:	command number
//			input buffer  (can be null)
//			size of input buffer  (0 - 16 byte)
//			output buffer (can be null)
//			size of output buffer (0 - 16 bytes)
// returns:	1 if successful
//			0 if error
int sceCdApplyNCmd(u8 cmdNum, const void* inBuff, u16 inBuffSize, void* outBuff, u16 outBuffSize);

// read data to iop memory
// 
// args:	sector location to read from
//			number of sectors to read
//			buffer to read to (in iop memory)
//			read mode
// returns:	1 if successful
//			0 if error
int sceCdReadIOPMem(u32 lbn, u32 sectors, void *buf, sceCdRMode *mode);

// wait for disc to finish all n-commands
// (shouldnt really need to call this yourself)
// 
// returns:	6 if busy
//			2 if ready
//			0 if error
int sceCdNCmdDiskReady(void);

// do a 'chain' of reads with one command
// last chain value must be all 0xFFFFFFFF
// (max of 64 reads can be set at once)
// non-blocking, requires sceCdSync() call
// 
// SUPPORTED IN XCDVDMAN/XCDVDFSV ONLY
// 
// args:	pointer to an array of read chain data
//			read mode
// returns:	1 if successful
//			0 if error
int sceCdReadChain(sceCdRChain *tag, sceCdRMode *mode);

// **** S-Command Functions ****

// read clock value from ps2s clock
// (time values seem to be in hex or BCD)
// 
// args:	time/date struct
// returns:	1 if successful
//			0 if error
int sceCdReadClock(sceCdCLOCK *clock);

// write clock value to ps2s clock
// (time values seem to be in hex or BCD)
// 
// args:	time/date struct to set clocks time with
// returns:	1 if successful
//			0 if error
int sceCdWriteClock(const sceCdCLOCK *clock);

// gets the type of the currently inserted disc
// 
// returns:	disk type (CDVD_TYPE_???)
int sceCdGetDiskType(void);

// gets the last error that occurred
// 
// returns:	error type (CDVD_ERR_???)
int sceCdGetError(void);

// open/close/check disk tray
// 
// args:	param (CDVD_TRAY_???)
//			address for returning tray state change
// returns:	1 if successful
//			0 if error
int sceCdTrayReq(int param, u32 *traychk);

// send an s-command by function number
// 
// args:	command number
//			input buffer  (can be null)
//			size of input buffer  (0 - 16 byte)
//			output buffer (can be null)
//			size of output buffer (0 - 16 bytes)
// returns:	1 if successful
//			0 if error
int sceCdApplySCmd(u8 cmdNum, const void* inBuff, u16 inBuffSize, void *outBuff, u16 outBuffSize);

// gets the status of the sceCd system
// 
// returns:	status (CDVD_STAT_???)
int sceCdStatus(void);

// 'breaks' the currently executing command
// 
// returns:	1 if successful
//			0 if error
int sceCdBreak(void);

// cancel power off
// 
// SUPPORTED IN XCDVDMAN/XCDVDFSV ONLY
// 
// args:	result
// returns:	1 if successful
//			0 if error
int sceCdCancelPowerOff(u32* result);

// blue led control
// 
// SUPPORTED IN XCDVDMAN/XCDVDFSV ONLY
// 
// args:	control value
//			result
// returns:	1 if successful
//			0 if error
int sceCdBlueLedCtrl(u8 control, u32 *result);

// power off
// 
// SUPPORTED IN XCDVDMAN/XCDVDFSV ONLY
// 
// args:	result
// returns:	1 if successful
//			0 if error
int sceCdPowerOff(u32 *result);

// set media mode
// 
// SUPPORTED IN XCDVDMAN/XCDVDFSV ONLY
// 
// args:	media mode
// returns:	1 if successful
//			0 if error
int sceCdMmode(int media);

// change sceCd thread priority
// 
// SUPPORTED IN XCDVDMAN/XCDVDFSV ONLY
// 
// args:	media mode
// returns:	1 if successful
//			0 if error
int sceCdChangeThreadPriority(int priority);


// **** Stream Functions ****


// start streaming data
// 
// args:	sector location to start streaming from
//			mode to read in
// returns:	1 if successful
//			0 otherwise
int sceCdStStart(u32 lbn, sceCdRMode *mode);

// read stream data
// 
// args:	number of sectors to read
//			read buffer address
//			data stream mode (STMNBLK or STMBLK)
//			error value holder
// returns:	number of sectors read if successful
//			0 otherwise
int sceCdStRead(u32 sectors, u32 *buffer, u32 mode, u32 *error);

// stop streaming
// 
// returns:	1 if successful
//			0 otherwise
int sceCdStStop(void);

// seek to a new stream position
// 
// args:	sector location to start streaming from
// returns:	1 if successful
//			0 otherwise
int sceCdStSeek(u32 lbn);

// init streaming
// 
// args:	stream buffer size
//			number of ring buffers
//			buffer address on iop
// returns:	1 if successful
//			0 otherwise
int sceCdStInit(u32 bufmax, u32 bankmax, void *buffer);

// get stream read status
// 
// returns:	number of sectors read if successful
//			0 otherwise
int sceCdStStat(void);

// pause streaming
// 
// returns:	1 if successful
//			0 otherwise
int sceCdStPause(void);

// continue streaming
// 
// returns:	1 if successful
//			0 otherwise
int sceCdStResume(void);

// **** Other Functions ****

// init sceCdvd system
// 
// args:	init mode (CDVD_INIT_???)
// returns:	1 if successful
//			0 if error
int sceCdInit(int mode);

// waits/checks for completion of n-commands
// 
// args:	0 = wait for completion of command (blocking)
//			1 = check current status and return immediately
// returns:	0 = completed
//			1 = not completed
int sceCdSync(int mode);

// search for a file on disc
// 
// args:	file structure to get file info in
//			name of file to search for (no wildcard characters)
//				(should be in the form '\\SYSTEM.CNF;1')
// returns:	1 if successful
//			0 if error (or no file found)
int sceCdSearchFile(sceCdlFILE *file, const char *name);

// checks if drive is ready
// 
// args:	 mode
// returns:	SCECdComplete if ready
//		SCECdNotReady if busy
int sceCdDiskReady(int mode);

// convert from sector number to minute:second:frame
sceCdlLOCCD *sceCdIntToPos(u32 i, sceCdlLOCCD *p);

// convert from minute:second:frame to sector number
u32 sceCdPosToInt(sceCdlLOCCD *p);

// get the current read position (when reading using sceCdRead)
u32 sceCdGetReadPos(void);

// initialise EE callback thread
// 
// args:	callback thread priority
//			callback thread stack address
//			callback thread stack size
// returns:	1 if initialised callback
//			0 if only priority was changed
int sceCdInitEeCB(int priority, void *stackAddr, int stackSize);

// set sceCd callback function
// 
// args:	pointer to new callback function
// returns:	pointer to old function
CdCBFunc sceCdCallback(CdCBFunc function);

//TODO: comment on all these functions and add missing functions.
int sceCdForbidDVDP(u32 *result);
int sceCdDecSet(unsigned char arg1, unsigned char arg2, unsigned char shift);
int sceCdReadKey(unsigned char arg1, unsigned char arg2, unsigned int command, unsigned char *key);
int sceCdOpenConfig(int block, int mode, int NumBlocks, u32 *result);	/* The IOP-side sceCdOpenConfig() function does not have a fourth argument. */
int sceCdCloseConfig(u32 *result);
int sceCdReadConfig(void *buffer, u32 *result);
int sceCdWriteConfig(const void *buffer, u32 *result);
int sceCdReadNVM(unsigned int address, unsigned short int *data, u8 *result);
int sceCdWriteNVM(unsigned int address, unsigned short int data, u8 *result);	//Does not fully work on consoles starting from the SCPH-50000 ("Dragon" MECHACON).
int sceCdRI(u8 *buffer, u32 *result);	// Read i.Link ID.
int sceCdWI(const u8 *buffer, u32 *result);	// Write i.Link ID. Does not work on consoles starting from the SCPH-50000 ("Dragon" MECHACON).
int sceCdReadConsoleID(u8 *buffer, u32 *result);
int sceCdWriteConsoleID(const u8 *buffer, u32 *result);	//Does not work on consoles starting from the SCPH-50000 ("Dragon" MECHACON).
int sceCdMV(u8 *buffer, u32 *result);	// Read Mecha version.

// set sceCd callback function
// 
// args:	the ROM 'name' (e.g. 0160HC -> 0x01,0x3C,'H','C', 0220JC -> 0x02,0x14,'J','C').
// returns:	1 on success, 0 on failure.
int sceCdBootCertify(const u8 *romname);	// Only available in newer CDVDMAN versions.
int sceCdRM(char *buffer, u32 *result);		// Read Model name.
int sceCdWM(const char *buffer, u32 *result);	// Write Model name. Does not work on consoles starting from the SCPH-50000 ("Dragon" MECHACON).

#ifdef __cplusplus
}
#endif

#endif // _LIBCDVD_H_



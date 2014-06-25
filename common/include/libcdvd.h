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
	SCECdSpinMax=0,		// Spin at maximum speed
	SCECdSpinStm=0,		// Spins at the recommended speed for streaming data
	SCECdSpinDvdDL0=0,	// Spins at the DVD layer 0 speed

	SCECdSpinNom=1,		// Optimized speed. Starts reading at max speed, slows down as read errors occur.
	SCECdSpinX1,		// 1x. For CDDA reading.
	//Note: These modes were taken from the PS2Linux libcdvd.h file, and they don't appear in any other Sony libcdvd.h file.
	SCECdSpinX2,		// 2x
	SCECdSpinX4,		// 4x
	SCECdSpinX12,		// 12x
	SCECdSpinNm2=10,	// Optimized speed, based on current speed.
	SCECdSpin1p6,		// DVD x1.6 CLV
	SCECdSpinMx=20,		// Spin at maximum speed (Not sure what's the difference between this and SCECdSpinMax)
};

enum SCECdvdMModeMediaType{
	SCECdMmodeCd=1,
	SCECdMmodeDvd
};

enum SCECdvdErrorCode{
	SCECdErFAIL	= -1,	// Can't get error code
	SCECdErNO	= 0x00,	// No Error
	SCECdErABRT,		// Aborted

	SCECdErCMD	= 0x10,	// Unsupported command
	SCECdErOPENS,		// Tray is open
	SCECdErNODISC,		// No disc inserted
	SCECdErNORDY,		// Device not ready
	SCECdErCUD,		// Unsupported command for current disc

	SCECdErIPI	= 0x20,	// Illegal position/LSN
	SCECdErILI,		// Illegal length
	SCECdErPRM,		// Invalid parameter

	SCECdErREAD	= 0x30,	// Read error
	SCECdErTRMOPN,		// Tray was opened while reading
	SCECdErEOM,		// End Of Media
	SCECdErSFRMTNG	= 0x38,

	SCECdErREADCF	= 0xFD,	// Error setting command
	SCECdErREADCFR		// Error setting command
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
	CdlNoIntr	= 0x00,	// No interrupt
	CdlDataReady,		// Data Ready
	SCECdComplete,		// Command Complete
	CdlAcknowledge,		// Acknowledge (reserved)
	CdlDataEnd,		// End of Data Detected
	CdlDiskError,		// Error Detected
	SCECdNotReady		// Drive Not Ready
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
typedef void (*sceCdCBFunc)(int reason);

enum SCECdvdInitModes{
	SCECdINIT	= 0x00,	// Initializes library and waits until commands can be sent.
	SCECdINoD,		// Initialize only the library.
	SCECdEXIT	= 0x05	// Deinitialize library.
};

// Low-level filesystem properties for sceCdSearchFile() 
#define CdlMAXFILE	64	// Maximum number of files in a directory.
#define CdlMAXDIR	128	// Maximum number of total directories.
#define CdlMAXLEVEL	8	// Maximum levels of directories.

// For streaming operations (Use with sceCdStRead())
enum SCECdvdStreamModes{
	STMNBLK	= 0,	// Stream without blocking.
	STMBLK		// Stream, but block.
};

// EE read modes (Used with sceCdSetEEReadMode()).
#define SCECdNoCheckReady	0x00000001
#define SCECdNoWriteBackDCache	0x00000002

#ifdef __cplusplus
extern "C" {
#endif

// **** N-Command Functions ****

// read data from disc
// non-blocking, requires sceCdSync() call
// 
// arguments:	sector location to start reading from
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
// arguments:	buffer to hold toc (1024 bytes)
// returns:	1 if successful
//			0 otherwise
int sceCdGetToc(u8 *toc);

// seek to given sector on disc
// non-blocking, requires sceCdSync() call
// 
// arguments:	sector to seek to on disc
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
// arguments:	command number
//			input buffer  (can be null)
//			size of input buffer  (0 - 16 byte)
//			output buffer (can be null)
//			size of output buffer (0 - 16 bytes)
// returns:	1 if successful
//			0 if error
int sceCdApplyNCmd(u8 cmdNum, const void* inBuff, u16 inBuffSize, void* outBuff, u16 outBuffSize);

// read data to iop memory
// 
// arguments:	sector location to read from
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
// arguments:	pointer to an array of read chain data
//			read mode
// returns:	1 if successful
//			0 if error
int sceCdReadChain(sceCdRChain *tag, sceCdRMode *mode);

// **** S-Command functions ****

// Reads the PlayStation clock.
// (time value is in BCD)
//
// NOTE: For the old CDVDMAN module, the month field will not have its most significant bit automatically filed off.
//
// arguments:	time/date struct
// returns:	1 if successful
//			0 if error
int sceCdReadClock(sceCdCLOCK *clock);

// Sets the PlayStation 2 clock.
// (time value is in BCD)
// 
// arguments:	time/date struct to set clocks time with
// returns:	1 if successful
//			0 if error
int sceCdWriteClock(const sceCdCLOCK *clock);

// gets the type of the currently inserted disc
// 
// returns:	disk type (SCECdvdMediaTypes)
int sceCdGetDiskType(void);

// gets the last error that occurred
// 
// returns:	error type (SCECdvdErrorCode)
int sceCdGetError(void);

// open/close/check disk tray
// 
// arguments:	param (SCECdvdTrayReqMode)
//			address for returning tray state change
// returns:	1 if successful
//			0 if error
int sceCdTrayReq(int param, u32 *traychk);

// send an s-command by function number
// 
// arguments:	command number
//			input buffer  (can be null)
//			size of input buffer  (0 - 16 byte)
//			output buffer (can be null)
//			size of output buffer (0 - 16 bytes)
// returns:	1 if successful
//			0 if error
int sceCdApplySCmd(u8 cmdNum, const void* inBuff, u16 inBuffSize, void *outBuff, u16 outBuffSize);

// gets the state of the drive
// 
// returns:	status (SCECdvdDriveStates)
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
// arguments:	result
// returns:	1 if successful
//			0 if error
int sceCdCancelPOffRdy(u32* result);

// blue led control
// 
// SUPPORTED IN XCDVDMAN/XCDVDFSV ONLY
// 
// arguments:	control value
//			result
// returns:	1 if successful
//			0 if error
int sceCdBlueLedCtrl(u8 control, u32 *result);

// power off
// 
// SUPPORTED IN XCDVDMAN/XCDVDFSV ONLY
// 
// arguments:	result
// returns:	1 if successful
//			0 if error
int sceCdPowerOff(u32 *result);

// set media mode
// 
// SUPPORTED IN XCDVDMAN/XCDVDFSV ONLY
// 
// arguments:	media mode (SCECdvdMModeMediaTypes)
// returns:	1 if successful
//			0 if error
int sceCdMmode(int media);

// change libcdvd thread priority
// 
// SUPPORTED IN XCDVDMAN/XCDVDFSV ONLY
// 
// arguments:	media mode
// returns:	1 if successful
//			0 if error
int sceCdChangeThreadPriority(int priority);


// **** Streaming Functions ****

// start streaming data
// 
// arguments:	sector location to start streaming from
//			mode to read in
// returns:	1 if successful
//			0 otherwise
int sceCdStStart(u32 lbn, sceCdRMode *mode);

// read stream data
// 
// arguments:	number of sectors to read
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
// arguments:	sector location to start streaming from
// returns:	1 if successful
//			0 otherwise
int sceCdStSeek(u32 lbn);

// init streaming
// 
// arguments:	stream buffer size
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

// initializes libcdvd
// 
// arguments:	mode (SCECdvdInitModes)
// returns:	1 if successful
//			0 if error
int sceCdInit(int mode);

// waits/checks for completion of n-commands
// 
// arguments:	0 = wait for completion of command (blocking)
//			1 = check current status and return immediately
// returns:	0 = completed
//			1 = not completed
int sceCdSync(int mode);

// search for a file on disc
// 
// arguments:	file structure to get file info in
//			name of file to search for (no wildcard characters)
//				(should be in the form '\\SYSTEM.CNF;1')
// returns:	1 if successful
//			0 if error (or no file found)
int sceCdSearchFile(sceCdlFILE *file, const char *name);

// checks if drive is ready
// 
// arguments:	 mode
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
// arguments:	callback thread priority
//			callback thread stack address
//			callback thread stack size
// returns:	1 if initialised callback
//			0 if only priority was changed
int sceCdInitEeCB(int priority, void *stackAddr, int stackSize);

// set sceCd callback function
// 
// arguments:	pointer to new callback function
// returns:	pointer to old function
sceCdCBFunc sceCdCallback(sceCdCBFunc function);

// Reads SUBQ data from the disc.
//
// arguments:	Pointer to the buffer for storing the SUBQ data in.
//		Result code.
// returns:	1 on success, 0 on failure.
int sceCdReadSUBQ(void *buffer, u32 *status);

// **** System functions ****

// Disables (Forbids) the DVD READ N-command, so that DVD video disc sectors cannot be read.
// Support for the DVD READ N-command is re-enabled when a DVD player is loaded.
//
// arguments:	Result code.
// returns:	1 on success, 0 on failure.
int sceCdForbidDVDP(u32 *result);

// Controls automatic adjustment of the CD/DVD drive.
// This also causes the auto-tilt motor (on units that have one) to be activated.
//
// arguments:	Mode
//		Result code.
// returns:	1 on success, 0 on failure.
int sceCdAutoAdjustCtrl(int mode, u32 *result);

// Controls on-the-fly (hardware) data decryption. Setting all options to 0 will disable decryption.
// This is used for decrypting encrypted sectors like the PlayStation 2 logo.
//
// arguments:	Unknown
//		Unknown
//		Shift amount
// returns:	1 on success, 0 on failure.
int sceCdDecSet(unsigned char arg1, unsigned char arg2, unsigned char shift);

// Reads the requested key from the CD/DVD.
//
// arguments:	Unknown
//		Unknown
//		Command
//		Buffer to store the key in.
// returns:	1 on success, 0 on failure.
int sceCdReadKey(unsigned char arg1, unsigned char arg2, unsigned int command, unsigned char *key);

// Opens a specified configuration block, within NVRAM. Each block is 15 bytes long.
//
// arguments:	Block number.
//		Mode (0 = read, 1 = write).
//		Number of blocks.
//		Result code.
// returns:	1 on success, 0 on failure.
int sceCdOpenConfig(int block, int mode, int NumBlocks, u32 *status);

// Closes the configuration block.
//
// arguments:	Result code.
// returns:	1 on success, 0 on failure.
int sceCdCloseConfig(u32 *result);

// Reads the configuration block(s) that was/were previously opened. Each block is 15 bytes long.
//
// arguments:	Pointer to the buffer to store the configuration data in.
//		Result code.
// returns:	1 on success, 0 on failure.
int sceCdReadConfig(void *buffer, u32 *result);

// Writes to the configuration block(s) that was/were previously opened. Each block is 15 bytes long.
//
// arguments:	Pointer to the buffer that contains the new configuration data.
//		Result code.
// returns:	1 on success, 0 on failure.
int sceCdWriteConfig(const void *buffer, u32 *result);

// Reads a single word from the NVRAM storage.
//
// arguments:	Addess in 2-byte words, of the word that will be read.
//		Pointer to the buffer that will contain the data read.
//		Result code.
// returns:	1 on success, 0 on failure.
int sceCdReadNVM(u32 address, u16 *data, u8 *result);

// Writes a single word to the NVRAM storage.
// Does not fully work on consoles starting from the SCPH-50000 ("Dragon" MECHACON):
// Some parts, like those containing the console IDs, cannot be overwritten.
//
// arguments:	Addess in 2-byte words, of the word that will be written to.
//		Pointer to the buffer that contains the new data.
//		Result code.
// returns:	1 on success, 0 on failure.
int sceCdWriteNVM(u32 address, u16 data, u8 *result);

// Reads the i.Link ID of the console.
// All consoles have an i.Link ID, including those that do not have a physical i.Link port.
//
// arguments:	Pointer to the buffer that will contain the data read.
//		Result code.
// returns:	1 on success, 0 on failure.
int sceCdRI(u8 *buffer, u32 *result);

// Writes a new i.Link ID for the console.
// Does not work on consoles starting from the SCPH-50000 ("Dragon" MECHACON):
//
// arguments:	Pointer to the buffer that contains the new i.Link ID.
//		Result code.
// returns:	1 on success, 0 on failure.
int sceCdWI(const u8 *buffer, u32 *result);

// Reads the ID of the console. This is not the same as the i.Link ID.
//
// arguments:	Pointer to the buffer that will contain the data read.
//		Result code.
// returns:	1 on success, 0 on failure.
int sceCdReadConsoleID(u8 *buffer, u32 *result);

// Writes a new ID for the console. This is not the same as the i.Link ID.
// Does not work on consoles starting from the SCPH-50000 ("Dragon" MECHACON):
//
// arguments:	Pointer to the buffer that contains the new i.Link ID.
//		Result code.
// returns:	1 on success, 0 on failure.
int sceCdWriteConsoleID(const u8 *buffer, u32 *status);

// Controls Audio Digital output.
//
// arguments:	Unknown
//		Result code.
// returns:	1 on success, 0 on failure.
int sceCdCtrlADout(int arg1, u32 *status);

// Reads MECHACON version data (RR MM mm TT): RR = Magicgate region, MM = major, mm = minor, TT = system type (00 = PS2, 01 = PSX)
// Magicgate region codes are: 00 = Japan, 01 = USA, 02 = Europe, 03 = Oceania, 04 = Asia, 05 = Russia, 06 = China, and 07 = Mexico.
//
// NOTE: The old CDVDMAN module only returns MM mm TT instead. The region byte is made up of the lower 7 bits of the status/result code.
//
// arguments:	Pointer to the buffer to store the version data in.
//		Result code.
// returns:	1 on success, 0 on failure.
int sceCdMV(u8 *buffer, u32 *status);

// Performs bootup certification.
// SUPPORTED IN XCDVDMAN/XCDVDFSV ONLY
//
// arguments:	ROM version - The ROM "name" of the console in this format:
//				romname[0]	- ROM major version number (In decimal).
//				romname[1]	- ROM minor version number (In decimal).
//				romname[2]	- ROM region.
//				romname[3]	- Console type (E.g. C, X or T).
// returns:	1 on success, 0 on failure.
int sceCdBootCertify(const u8 *romname);

// Reads the console's model "name": e.g. "SCPH-39000".
// SUPPORTED IN XCDVDMAN/XCDVDFSV ONLY
//
// arguments:	Pointer to the buffer for storing the model name in.
//		Result code.
// returns:	1 on success, 0 on failure.
int sceCdRM(char *buffer, u32 *status);

// Sets the console's model name.
// Does not work on consoles starting from the SCPH-50000 ("Dragon" MECHACON).
// SUPPORTED IN XCDVDMAN/XCDVDFSV ONLY
//
// arguments:	Pointer to a buffer containing the new model name.
//		Result code.
// returns:	1 on success, 0 on failure.
int sceCdWM(const char *buffer, u32 *status);

#ifdef __cplusplus
}
#endif

#endif // _LIBCDVD_H_

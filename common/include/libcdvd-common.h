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
#define SCE_CdSTREAM 0x40000000

enum SCECdvdSectorType {
    /* Game CD and DVD sector types.  */
    SCECdSecS2048 = 0,
    SCECdSecS2328,
    SCECdSecS2340,

    /* CDDA sector types.  */
    SCECdSecS2352 = 0,
    SCECdSecS2368,
    SCECdSecS2448,
};

enum SCECdvdSpinValue {
    /** Spin at maximum speed */
    SCECdSpinMax = 0,
    /** Spins at the recommended speed for streaming data */
    SCECdSpinStm = 0,
    /** Spins at the DVD layer 0 speed */
    SCECdSpinDvdDL0 = 0,

    /** Optimized speed. Starts reading at max speed, slows down as read errors occur. */
    SCECdSpinNom = 1,
    /** 1x. For CDDA reading. */
    SCECdSpinX1,
    // Note: These modes were taken from the PS2Linux libcdvd.h file, and they don't appear in any other Sony libcdvd.h file.
    /** 2x */
    SCECdSpinX2,
    /** 4x */
    SCECdSpinX4,
    /** 12x */
    SCECdSpinX12,
    /** Optimized speed, based on current speed. */
    SCECdSpinNm2 = 10,
    /** DVD x1.6 CLV */
    SCECdSpin1p6,
    /** Spin at maximum speed (Not sure what's the difference between this and SCECdSpinMax) */
    SCECdSpinMx = 20,
};

enum SCECdvdMModeMediaType {
    SCECdMmodeCd = 1,
    SCECdMmodeDvd
};

enum SCECdvdErrorCode {
    /** Can't get error code */
    SCECdErFAIL = -1,
    /** No Error */
    SCECdErNO = 0x00,
    /** Aborted */
    SCECdErABRT,

    /** Unsupported command */
    SCECdErCMD = 0x10,
    /** Tray is open */
    SCECdErOPENS,
    /** No disc inserted */
    SCECdErNODISC,
    /** Device not ready */
    SCECdErNORDY,
    /** Unsupported command for current disc */
    SCECdErCUD,

    /** Illegal position/LSN */
    SCECdErIPI = 0x20,
    /** Illegal length */
    SCECdErILI,
    /** Invalid parameter */
    SCECdErPRM,

    /** Read error */
    SCECdErREAD = 0x30,
    /** Tray was opened while reading */
    SCECdErTRMOPN,
    /** End Of Media */
    SCECdErEOM,
    SCECdErSFRMTNG = 0x38,

    /** Error setting command */
    SCECdErREADCF = 0xFD,
    /** Error setting command */
    SCECdErREADCFR
};

enum SCECdvdMediaType {
    SCECdGDTFUNCFAIL = -1,
    /** No Disc inserted */
    SCECdNODISC      = 0x00,
    /** Detecting disc type */
    SCECdDETCT,
    SCECdDETCTCD,
    SCECdDETCTDVDS,
    SCECdDETCTDVDD,
    /** Unknown disc type */
    SCECdUNKNOWN,

    /** PS1 CD with no CDDA tracks */
    SCECdPSCD = 0x10,
    /** PS1 CD with CDDA tracks */
    SCECdPSCDDA,
    /** PS2 CD with no CDDA tracks */
    SCECdPS2CD,
    /** PS2 CD with CDDA tracks */
    SCECdPS2CDDA,
    /** PS2 DVD */
    SCECdPS2DVD,

    /** DVD-VR (Minimum mechacon firmware version: 50000) */
    SCECdDVDVR = 0xFC,
    /** CDDA */
    SCECdCDDA = 0xFD,
    /** DVD Video */
    SCECdDVDV,
    /** Illegal disk type */
    SCECdIllegalMedia
};

enum SCECdvdInterruptCode {
    /** No interrupt */
    CdlNoIntr = 0x00,
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

// Tray request modes
enum SCECdvdTrayReqMode {
    /** Tray Open */
    SCECdTrayOpen = 0,
    /** Tray Close */
    SCECdTrayClose,
    /** Tray Check */
    SCECdTrayCheck
};

// Drive states
enum SCECdvdDriveState {
    /** disc has stopped spinning */
    SCECdStatStop = 0x00,
    /** tray is open */
    SCECdStatShellOpen,
    /** disc is spinning */
    SCECdStatSpin,
    /** reading from disc */
    SCECdStatRead  = 0x06,
    /** disc is paused */
    SCECdStatPause = 0x0A,
    /** disc is seeking */
    SCECdStatSeek  = 0x12,
    /** error occurred */
    SCECdStatEmg   = 0x20,
};

typedef struct
{
    /** status */
    u8 stat;
    /** second */
    u8 second;
    /** minute */
    u8 minute;
    /** hour */
    u8 hour;
    /** padding */
    u8 pad;
    /** day */
    u8 day;
    /** month */
    u8 month;
    /** year */
    u8 year;
} sceCdCLOCK;

typedef struct
{
    /** file location */
    u32 lsn;
    /** file size */
    u32 size;
    /** file name (body) */
    char name[16];
    /** date (1=secs, 2=mins, 3=hours, 4=day, 5=mon, 6,7=year,   0=iso file flags) */
    u8 date[8];
} sceCdlFILE;

// location structure, used with sceCdIntToPos() and sceCdPosToInt()
typedef struct
{
    /** minute (BCD) */
    u8 minute;
    /** second (BCD) */
    u8 second;
    /** sector (BCD) */
    u8 sector;
    /** track (void), aka "frame" */
    u8 track;
} sceCdlLOCCD;

typedef struct
{
    /** number of times to retry reads when an error occurs */
    u8 trycount;
    /** speed to read at SCECdvdSpinValue enumeration values (also speed to spin at) */
    u8 spindlctrl;
    /** sector size value SCECdvdSectorType enumeration values */
    u8 datapattern;
    /** padding */
    u8 pad;
} sceCdRMode;

typedef struct
{
    /** sector location to start reading from */
    u32 lbn;
    /** number of sectors to read */
    u32 sectors;
    /** buffer address to read to ( bit0: 0=EE, 1=IOP ) (EE addresses must be on 64-byte alignment) */
    u32 buffer;
} sceCdRChain;

// macros for converting between an integer and a BCD number
#ifndef btoi
#define btoi(b) ((b) / 16 * 10 + (b) % 16) // BCD to int
#endif
#ifndef itob
#define itob(i) ((i) / 10 * 16 + (i) % 10) // int to BCD
#endif

/** max number of toc entries for sceCdGetToc() */
#define CdlMAXTOC 100

enum SCECdvdCallbackReason {
    SCECdFuncRead = 1,
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

enum SCECdvdInitMode {
    /** Initializes library and waits until commands can be sent. */
    SCECdINIT = 0x00,
    /** Initialize only the library. */
    SCECdINoD,
    /** Deinitialize library. */
    SCECdEXIT = 0x05
};

// Low-level filesystem properties for sceCdSearchFile()
/** Maximum number of files in a directory. */
#define CdlMAXFILE  64
/** Maximum number of total directories. */
#define CdlMAXDIR   128
/** Maximum levels of directories. */
#define CdlMAXLEVEL 8

enum SCECdvdFanSpeed {
    /** The fan will spin at the minimum speed. */
    SCECdvdFanSpeedMinimum = 0x00,
    /** The fan will spin at a medium speed. */
    SCECdvdFanSpeedMedium,
    /** The fan will spin at the maximum speed. */
    SCECdvdFanSpeedMaximum,
};

// LED definitions for sceCdSetLEDsMode()
/** Light up the red LED on the power button. */
#define CdlLEDPowerRed 1
/** Light up the green LED on the power button. */
#define CdlLEDPowerGreen 2
/** Light up the yellow LED on the power button. */
#define CdlLEDPowerYellow 4
/** Light up the blue LED on the eject button. */
#define CdlLEDEjectBlue 8

// Remote control disable definitions for sceRemote2_7()
/** Disable the power/reset functionalty. (RM_PS2_POWER) */
#define CdlRCDisablePowerReset 1
/** Disable the power off functionality. (RM_PS2_POWEROFF) */
#define CdlRCDisablePowerOff 2
/** Disable the reset functionalty. (RM_PS2_RESET) */
#define CdlRCDisableReset 4
/** Disable the eject functionalty. (RM_PS2_EJECT) */
#define CdlRCDisableEject 8
/** Disable the power on functionality. (RM_PS2_POWERON) */
#define CdlRCDisablePowerOn 0x10

// Flag definitions for sceCdCLOCK.stat
/** The RTC clock is stopped */
#define CdlRTCStatClockStopDetected 1
/** There was an issue checking the RTC battery voltage */
#define CdlRTCStatClockBatteryMonitoringVoltageProblem 2
/** The Rohm RTC hardware returned an error */
#define CdlRTCStatCTLRegProblem 4
/** There was an error while preparing to send the command from Mechacon to RTC */
#define CdlRTCStatCommandError 128

// Value definitions for the wakeupreason argument of sceCdReadWakeUpTime
/** The system was powered on using the front panel buttons or the remote */
#define CdlWakeUpReasonMainPowerOn 0
/** The system was reset using the front panel button, the remote, or the SCMD */
#define CdlWakeUpReasonMainReset 1
/** The system was powered on because the timer set by sceCdWriteWakeUpTime expired */
#define CdlWakeUpReasonMainTimer 2
/** The system was powered on using the PON_REQ signal connected to the expansion bay port */
#define CdlWakeUpReasonMainDevice 3

// Value definitions for the return value of sceCdGetWakeUpReason
#define CdlWakeUpReasonExtraSupportHard 0
/** The system was reset using the front panel button, the remote, or the SCMD */
#define CdlWakeUpReasonExtraReset 1
/** The system was reset using the "Quit Game" button */
#define CdlWakeUpReasonExtraGameReset 2
/** The system was powered on because the timer set by sceCdWriteWakeUpTime expired */
#define CdlWakeUpReasonExtraTimer 3
/** The system was powered on using the front panel buttons or the remote */
#define CdlWakeUpReasonExtraPowerOn 4
/** The system was powered on when an object was inserted into the disc slot */
#define CdlWakeUpReasonExtraSlotIn 7
#define CdlWakeUpReasonExtraBackGround 8

// For streaming operations (Use with sceCdStRead())
enum SCECdvdStreamMode {
    /** Stream without blocking. */
    STMNBLK = 0,
    /** Stream, but block. */
    STMBLK
};

// EE read modes (Used with sceCdSetEEReadMode()).
#define SCECdNoCheckReady      0x00000001
#define SCECdNoWriteBackDCache 0x00000002

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

/** send an n-command by function number
 *
 * @param cmdNum command number
 * @param inBuff input buffer  (can be null)
 * @param inBuffSize size of input buffer  (0 - 16 bytes)
 * @return 1 on success, 0 on failure.
 */
int sceCdApplyNCmd(u8 cmdNum, const void* inBuff, u16 inBuffSize);

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
 * NOTE: The normalized value will be written back to the buffer specified.
 *
 * @param clock time/date struct to set clocks time with
 * @return 1 on success, 0 on failure.
 */
int sceCdWriteClock(sceCdCLOCK *clock);

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
 * Minimum Mechacon firmware version: 20400
 * SUPPORTED IN XCDVDMAN/XCDVDFSV ONLY
 *
 * @param result result
 * @return 1 on success, 0 on failure.
 */
int sceCdCancelPOffRdy(u32 *result);

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

/** send an s-command by function number
 *
 * @param command number
 * @param input buffer  (can be null)
 * @param size of input buffer  (0 - 16 bytes)
 * @param output buffer (16 bytes, can be null)
 * @return 1 if successful, 0 if error
 */
int sceCdApplySCmd(u8 cmdNum, const void *inBuff, u16 inBuffSize, void *outBuff);

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
 * @param enable_xor Set to a non-zero value to enable xor by index 4 of the key argument in sceCdReadKey
 * @param enable_shift Set to a non-zero value to enable rotate shift right by the amount specified by the "shiftval" argument
 * @param shiftval Shift amount enable_shift is enabled
 * @return 1 on success, 0 on failure.
 */
int sceCdDecSet(unsigned char enable_xor, unsigned char enable_shift, unsigned char shiftval);

/** Reads the requested key from the CD/DVD.
 *
 * @param arg1 Unknown
 * @param arg2 Unknown
 * @param command Command
 * @param key Buffer to store the key in.
 * @return 1 on success, 0 on failure.
 */
int sceCdReadKey(unsigned char arg1, unsigned char arg2, unsigned int command, unsigned char *key);

/** Determines if unique disc key exists
 * Unofficial name.
 * SUPPORTED IN NEWER CDVDMAN MODULES INCLUDED WITHIN DNAS IOPRP ONLY
 *
 * @param status Command status
 * @return 1 on success, 0 on failure.
 */
int sceCdDoesUniqueKeyExist(u32 *status);

/** Blocks disc tray eject functionality and turns off the blue eject LED when enabled.
 *
 * @param mode Set to a non-zero value to enable the tray eject functionality
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

/** Closes the configuration block that is currently opened.
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
 * Starting from Mechacon firmware version 50000, attempting to write to address values over 150 will error.
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
 * Starting from Mechacon firmware version 50000, a unlock combination (0x03 0x46 and 0x03 0x47) needs to be executed first.
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
 * Starting from Mechacon firmware version 50000, a unlock combination (0x03 0x46 and 0x03 0x47) needs to be executed first.
 *
 * @param buffer Pointer to the buffer that contains the new i.Link ID.
 * @param status Result code.
 * @return 1 on success, 0 on failure.
 */
int sceCdWriteConsoleID(const u8 *buffer, u32 *status);

/** Controls Audio Digital output.
 *
 * @param mode Set to a non-zero value to enable digital output.
 * @param status Result code.
 * @return 1 on success, 0 on failure.
 */
int sceCdCtrlADout(int mode, u32 *status);

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
 *                romname[0]    - ROM major version number (In decimal).
 *                romname[1]    - ROM minor version number (In decimal).
 *                romname[2]    - ROM region.
 *                romname[3]    - Console type (E.g. C, D or T).
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
 * Starting from Mechacon firmware version 50000, a unlock combination (0x03 0x46 and 0x03 0x47) needs to be executed first.
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
int sceCdReadFull(unsigned int lsn, unsigned int sectors, void *buf, sceCdRMode *mode);

/** Seeks to a given position
 * SUPPORTED IN NEWER CDVDMAN MODULES INCLUDED WITHIN NEWER IOPRP ONLY
 *
 * @param lsn sector location to seek to
 * @return 1 on success, 0 on failure.
 */
int sceCdStSeekF(unsigned int lsn);

/** Sets a handler for the power-off event which will be called before the console switches off.
 * SUPPORTED IN NEWER CDVDMAN MODULES INCLUDED WITHIN NEWER IOPRP ONLY
 *
 * @param func pointer of callback function to be called when power-off processing is activated
 * @param addr an argument which will be passed to the callback function
 * returns: a pointer to the previous handler function, or a null pointer if nothing has been set eariler
 */
void *sceCdPOffCallback(void (*func)(void *), void *addr);

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
int sceCdReadModelID(unsigned int *id);

/** Reads the information about DVD disk.
 * SUPPORTED IN NEWER CDVDMAN MODULES INCLUDED WITHIN NEWER IOPRP ONLY
 *
 * @param on_dual pointer to variable where type of DVD disc is returned
 * @param layer1_start pointer to variable where the address of the second layer of a dual-layer DVD disc is returned.
 * @return 1 on success, 0 on failure.
 */
int sceCdReadDvdDualInfo(int *on_dual, unsigned int *layer1_start);

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
int sceCdRE(unsigned int lsn, unsigned int sectors, void *buf, sceCdRMode *mode);

/** Reads a GUID.
 * SUPPORTED IN NEWER CDVDMAN MODULES INCLUDED WITHIN DNAS IOPRP ONLY
 *
 * @param guid u64 integer where the GUID is stored.
 * @return 1 on success, 0 on failure.
 */
int sceCdReadGUID(u64 *guid);

/** Controls whether SIRCS remote control input should be translated to SIO2 pad input.
 * This corresponds to the "Remote Control" "Gameplay Function On/Off" option in OSDSYS.
 * Minimum Mechacon firmware version: 50000
 * SUPPORTED BY ONLY NEWER CDVDMAN MODULES INCLUDED WITHIN OSD ONLY
 *
 * @param mode 0 = SIRCS->SIO2 conversion enabled, 1 = SIRCS->SIO2 conversion disabled
 * @param status Result code.
 * @return 1 on success, 0 on failure
 */
int sceCdRcBypassCtl(int mode, u32 *status);

/** Reads wake up time.
 * Minimum Mechacon firmware version: 50000
 * SUPPORTED IN XCDVDMAN INCLUDED WITHIN NEWER BOOT ROMS ONLY
 *
 * @param clock The time to wake up the system.
 * @param userdata Any arbitrary value (not used in timer processing)
 * @param wakeupreason The reason why the system woke up.
 * @param flags bit 0 -> disable timer after expiration, bit 1 -> disable timer
 * @return 1 on success, 0 on failure
 */
int sceCdReadWakeUpTime(sceCdCLOCK *clock, u16 *userdata, u32 *wakeupreason, int *flags);

/** Writes wake up time.
 * Minimum Mechacon firmware version: 50000
 * Note: in newer Mechacon firmware versions (TODO: determine the range), the wake up timer function is removed. However, the storage for the parameters remain.
 * Note: if there was a non-zero value in sceCdCLOCK.stat the last time sceCdReadClock was called, no data will be written/enabled.
 * SUPPORTED IN XCDVDMAN INCLUDED WITHIN NEWER BOOT ROMS ONLY
 *
 * @param clock The time to wake up the system.
 * @param userdata Any arbitrary value (not used in timer processing)
 * @param flags When 1, disables the timer after it has expired. When 255, disables the timer and sets the seconds value to 0xFF.
 * @return 1 on success, 0 on failure
 */
int sceCdWriteWakeUpTime(const sceCdCLOCK *clock, u16 userdata, int flags);

/** Disables Mechacon actions performed using the remote control.
 * The actions that can be specified are poweron, poweroff, reset and eject.
 * The action performed by RM_PS2_NOLIGHT cannot be disabled.
 * Minimum Mechacon firmware version: 50000
 * SUPPORTED IN XCDVDMAN INCLUDED WITHIN NEWER BOOT ROMS ONLY
 *
 * @return 1 on success, 0 on failure
 */
int sceRemote2_7(u16 param, u32 *status);

/** Retrieves the value set by sceRemote2_7.
 * Minimum Mechacon firmware version: 50000
 * Unofficial name.
 * SUPPORTED IN XCDVDMAN INCLUDED WITHIN NEWER BOOT ROMS ONLY
 *
 * @param param The value set by sceRemote2_7
 * @param status Command status
 * @return 1 on success, 0 on failure.
 */
int sceRemote2_7Get(u32 *param, u32 *status);

/** Set the LED state of the face buttons of the console.
 * The state of the buttons will be reset when the power or eject button is pressed.
 * Minimum Mechacon firmware version: 50000
 * SUPPORTED IN XCDVDMAN INCLUDED WITHIN NEWER BOOT ROMS ONLY
 *
 * @param param The LED state to set.
 * @return 1 on success, 0 on failure
 */
int sceCdSetLEDsMode(u32 param, u32 *result);

/** Reads PS1 boot parameter.
 * Minimum Mechacon firmware version: 50200
 * SUPPORTED IN XCDVDMAN INCLUDED WITHIN NEWER BOOT ROMS ONLY
 *
 * @return 1 on success, 0 on failure
 */
int sceCdReadPS1BootParam(u8 *out, u32 *result);

/** Sets fan profile.
 * Minimum Mechacon firmware version: 50400
 * SUPPORTED IN XCDVDMAN INCLUDED WITHIN NEWER BOOT ROMS ONLY
 *
 * @param param param (SCECdvdFanSpeed)
 * @return 1 on success, 0 on failure
 */
int sceCdSetFanProfile(u8 param, u32 *result);

/** Sends SCMD 0x1D. Appears to be stubbed in Mechacon firmware 50000.
 * Minimum Mechacon firmware version: 50000
 * Unofficial name.
 * SUPPORTED IN XCDVDMAN INCLUDED WITHIN NEWER BOOT ROMS ONLY
 *
 * @param arg1 Unknown
 * @param arg2 Unknown
 * @param arg3 Unknown
 * @param status Command status
 * @return 1 on success, 0 on failure.
 */
int sceCdSendSCmd1D(int *arg1, unsigned int *arg2, unsigned int *arg3, u32 *status);

/** Change sys.
 * SUPPORTED BY ONLY DESR/PSX DVR CDVDMAN MODULES
 *
 * @return 1 on success, 0 on failure
 */
int sceCdChgSys(u32 arg1);

/** Notice game start.
 * Minimum Mechacon firmware version: 50400
 * SUPPORTED BY ONLY DESR/PSX DVR CDVDMAN MODULES
 *
 * @return 1 on success, 0 on failure
 */
int sceCdNoticeGameStart(u8 arg1, u32 *result);

/** Extended LED control.
 * Minimum Mechacon firmware version: 50600
 * SUPPORTED BY ONLY DESR/PSX DVR CDVDMAN MODULES
 *
 * @return 1 on success, 0 on failure
 */
int sceCdXLEDCtl(u8 arg1, u8 arg2, u32 *result1, u32 *result2);

/** Buzzer control.
 * Minimum Mechacon firmware version: 50600
 * SUPPORTED BY ONLY DESR/PSX DVR CDVDMAN MODULES
 *
 * @return 1 on success, 0 on failure
 */
int sceCdBuzzerCtl(u32 *result);

/** XBS power control.
 * Minimum Mechacon firmware version: 50600
 * SUPPORTED BY ONLY DESR/PSX DVR CDVDMAN MODULES
 *
 * @return 1 on success, 0 on failure
 */
int sceCdXBSPowerCtl(u8 arg1, u8 arg2, u32 *result1, u32 *result2);

/** Set medium removal.
 * Minimum Mechacon firmware version: 50600
 * SUPPORTED BY ONLY DESR/PSX DVR CDVDMAN MODULES
 *
 * @return 1 on success, 0 on failure
 */
int sceCdSetMediumRemoval(u8 arg1, u32 *result);

/** Get medium removal.
 * Minimum Mechacon firmware version: 50600
 * SUPPORTED BY ONLY DESR/PSX DVR CDVDMAN MODULES
 *
 * @return 1 on success, 0 on failure
 */
int sceCdGetMediumRemoval(u32 *result1, u32 *result2);

/** XDVRP reset.
 * Minimum Mechacon firmware version: 50600
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
 * Minimum Mechacon firmware version: 60000
 * SUPPORTED IN XCDVDMAN INCLUDED WITHIN NEWER BOOT ROMS ONLY
 *
 * @return 1 on success, 0 on failure
 */
int sceCdReadRegionParams(u32 *arg1, u32 *result);

/** Writes region parameters.
 * Minimum Mechacon firmware version: 60600
 * SUPPORTED IN XCDVDMAN INCLUDED WITHIN NEWER BOOT ROMS ONLY
 *
 * @return 1 on success, 0 on failure
 */
int sceCdWriteRegionParams(u8 arg1, u32 *arg2, u8 *arg3, u32 *result);

// Compatibility names for older ps2sdk versions.

// sceCdlFILE renames
#define CdvdFileSpec_t sceCdlFILE
#define cd_file_t      sceCdlFILE
// Structure member names are the same

// sceCdRMode renames
#define CdvdReadMode_t sceCdRMode
#define cd_read_mode_t sceCdRMode
// Structure member names differ:
// retries -> trycount
// readSpeed -> spindlctrl
// sectorType -> datapattern

// SCECdvdSectorType renames
#define SCECdvdSectorTypes SCECdvdSectorType
#define CdvdSectorType_t   SCECdvdSectorType
#define CDVD_SECTOR_2048   SCECdSecS2048
#define CDVD_SECTOR_2328   SCECdSecS2328
#define CDVD_SECTOR_2340   SCECdSecS2340
#define CDVD_SECTOR_2352   SCECdSecS2352
#define CDVD_SECTOR_2368   SCECdSecS2368
#define CDVD_SECTOR_2448   SCECdSecS2448
#define CdSecS2048         SCECdSecS2048
#define CdSecS2328         SCECdSecS2328
#define CdSecS2340         SCECdSecS2340

// SCECdvdSpinValue renames
#define SCECdvdSpinValues SCECdvdSpinValue
#define CDVD_SPIN_MAX     SCECdSpinMax
#define CDVD_SPIN_NORMAL  SCECdSpinNom
#define CDVD_SPIN_STREAM  SCECdSpinStm
#define CDVD_SPIN_DVD0    SCECdSpinDvdDL0
#define CdSpinMax         SCECdSpinMax
#define CdSpinNom         SCECdSpinNom
#define CdSpinStm         SCECdSpinStm

// SCECdvdErrorCode renames
#define SCECdvdErrorCodes SCECdvdErrorCode
#define CDVD_ERR_FAIL     SCECdErFAIL
#define CDVD_ERR_NO       SCECdErNO
#define CDVD_ERR_ABRT     SCECdErABRT
#define CDVD_ERR_CMD      SCECdErCMD
#define CDVD_ERR_OPENS    SCECdErOPENS
#define CDVD_ERR_NODISC   SCECdErNODISC
#define CDVD_ERR_NORDY    SCECdErNORDY
#define CDVD_ERR_CUD      SCECdErCUD
#define CDVD_ERR_IPI      SCECdErIPI
#define CDVD_ERR_ILI      SCECdErILI
#define CDVD_ERR_PRM      SCECdErPRM
#define CDVD_ERR_READ     SCECdErREAD
#define CDVD_ERR_TRMOPN   SCECdErTRMOPN
#define CDVD_ERR_EOM      SCECdErEOM
#define CDVD_ERR_READCF   SCECdErREADCF
#define CDVD_ERR_READCFR  SCECdErREADCFR

// SCECdvdMediaType renames
#define SCECdvdMediaTypes          SCECdvdMediaType
#define CdvdDiscType_t             SCECdvdMediaType
#define CDVD_TYPE_NODISK           SCECdNODISC
#define CDVD_TYPE_DETECT           SCECdDETCT
#define CDVD_TYPE_DETECT_CD        SCECdDETCTCD
#define CDVD_TYPE_DETECT_DVDSINGLE SCECdDETCTDVDS
#define CDVD_TYPE_DETECT_DVDDUAL   SCECdDETCTDVDD
#define CDVD_TYPE_UNKNOWN          SCECdUNKNOWN
#define CDVD_TYPE_PS1CD            SCECdPSCD
#define CDVD_TYPE_PS1CDDA          SCECdPSCDDA
#define CDVD_TYPE_PS2CD            SCECdPS2CD
#define CDVD_TYPE_PS2CDDA          SCECdPS2CDDA
#define CDVD_TYPE_PS2DVD           SCECdPS2DVD
#define CDVD_TYPE_CDDA             SCECdCDDA
#define CDVD_TYPE_DVDVIDEO         SCECdDVDV
#define CDVD_TYPE_ILLEGAL          SCECdIllegalMedia

// SCECdvdDriveState renames
#define SCECdvdDriveStates SCECdvdDriveState
#define CDVD_STAT_STOP     SCECdStatStop
#define CDVD_STAT_OPEN     SCECdStatShellOpen
#define CDVD_STAT_SPIN     SCECdStatSpin
#define CDVD_STAT_READ     SCECdStatRead
#define CDVD_STAT_PAUSE    SCECdStatPause
#define CDVD_STAT_SEEK     SCECdStatSeek
#define CDVD_STAT_ERROR    SCECdStatEmg

// SCECdvdMModeMediaType renames
#define SCECdvdMModeMediaTypes SCECdvdMModeMediaType
#define CdvdMediaMode_t        SCECdvdMModeMediaType
#define CDVD_MEDIA_MODE_CD     SCECdMmodeCd
#define CDVD_MEDIA_MODE_DVD    SCECdMmodeDvd
#define CdMmodeCd              SCECdMmodeCd
#define CdMmodeDvd             SCECdMmodeDvd

// sceCdlLOCCD renames
#define CdvdLocation_t sceCdlLOCCD
#define cd_location_t  sceCdlLOCCD
// Structure member names are the same

// SCECdvdInitMode renames
#define SCECdvdInitModes  SCECdvdInitMode
#define CDVD_INIT_INIT    SCECdINIT
#define CDVD_INIT_NOCHECK SCECdINoD
#define CDVD_INIT_EXIT    SCECdEXIT

// SCECdvdInterruptCode renames
#define SCECdvdInterruptCodes SCECdvdInterruptCode
#define CDVD_READY_READY      SCECdComplete
#define CDVD_READY_NOTREADY   SCECdNotReady

// sceCdCLOCK renames
#define CdvdClock_t sceCdCLOCK
#define cd_clock_t  sceCdCLOCK
// Structure member names differ:
// status -> stat
// week -> pad

// SCECdvdStreamMode renames
#define SCECdvdStreamModes   SCECdvdStreamMode
#define CDVD_STREAM_NONBLOCK STMNBLK
#define CDVD_STREAM_BLOCK    STMBLK

// SCECdvdTrayReqMode renames
#define SCECdvdTrayReqModes SCECdvdTrayReqMode
#define CDVD_TRAY_OPEN      SCECdTrayOpen
#define CDVD_TRAY_CLOSE     SCECdTrayClose
#define CDVD_TRAY_CHECK     SCECdTrayCheck

// sceCdCBFunc rename
#define CdCBFunc sceCdCBFunc

// SCECdvdCallbackReason rename
#define SCECdvdCallbackReasons SCECdvdCallbackReason


// function renames
#define CdInit         sceCdInit
#define cdInit         sceCdInit
#define CdStandby      sceCdStandby
#define cdStandby      sceCdStandby
#define CdRead         sceCdRead
#define cdRead         sceCdRead
#define CdSeek         sceCdSeek
#define cdSeek         sceCdSeek
#define CdGetError     sceCdGetError
#define cdGetError     sceCdGetError
#define CdGetToc       sceCdGetToc
#define cdGetToc       sceCdGetToc
#define CdSearchFile   sceCdSearchFile
#define cdSearchFile   sceCdSearchFile
#define CdSync         sceCdSync
#define cdSync         sceCdSync
#define CdGetDiskType  sceCdGetDiskType
#define cdGetDiscType  sceCdGetDiskType
#define CdDiskReady    sceCdDiskReady
#define cdDiskReady    sceCdDiskReady
#define CdTrayReq      sceCdTrayReq
#define cdTrayReq      sceCdTrayReq
#define CdStop         sceCdStop
#define cdStop         sceCdStop
#define CdPosToInt     sceCdPosToInt
#define cdPosToInt     sceCdPosToInt
#define CdIntToPos     sceCdIntToPos
#define cdIntToPos     sceCdIntToPos
#define CdReadClock    sceCdReadClock
#define cdReadClock    sceCdReadClock
#define CdStatus       sceCdStatus
#define cdStatus       sceCdStatus
#define CdCallback     sceCdCallback
#define cdSetCallback  sceCdCallback
#define CdPause        sceCdPause
#define cdPause        sceCdPause
#define CdBreak        sceCdBreak
#define cdBreak        sceCdBreak
#define CdReadCdda     sceCdReadCdda
#define cdCddaRead     sceCdReadCdda
#define CdGetReadPos   sceCdGetReadPos
#define cdGetReadPos   sceCdGetReadPos
#define CdMmode        sceCdMmode
#define cdSetMediaMode sceCdMmode

#define cdDvdRead              sceCdReadDVDV
#define cdApplyNCmd            sceCdApplyNCmd
#define cdReadIOPMem           sceCdReadIOPMem
#define cdNCmdDiskReady        sceCdNCmdDiskReady
#define cdReadChain            sceCdReadChain
#define cdWriteClock           sceCdWriteClock
#define cdApplySCmd            sceCdApplySCmd
#define cdCancelPowerOff       sceCdCancelPOffRdy
#define cdBlueLedCtrl          sceCdBlueLEDCtl
#define sceCdBlueLedCtrl       sceCdBlueLEDCtl
#define cdPowerOff             sceCdPowerOff
#define cdChangeThreadPriority sceCdChangeThreadPriority
#define cdStStart              sceCdStStart
#define cdStRead               sceCdStRead
#define cdStStop               sceCdStStop
#define cdStSeek               sceCdStSeek
#define cdStInit               sceCdStInit
#define cdStStat               sceCdStStat
#define cdStPause              sceCdStPause
#define cdStResume             sceCdStResume
#define CdRC                   sceCdRC

// Internal definitions no longer exposed:
// cdStream
// cdCddaStream
// cdInitCallbackThread

#ifdef __cplusplus
}
#endif

#endif /* __LIBCDVD_COMMON_H__ */

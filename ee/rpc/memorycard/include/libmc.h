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
 * Macros, structures & function prototypes for mclib.
 */

/*
	NOTE: These functions will work with the MCMAN/MCSERV or XMCMAN/XMCSERV
	modules stored in rom0. To determine which one you are using, send the
	appropriate arg to the mcInit() function (MC_TYPE_MC or MC_TYPE_XMC)

        NOTE: These functions seem to work for both psx and ps2 memcards

        to use memcards:
        1) first load modules (sio2man then mcman/mcserv)
        2) call mcInit(MC_TYPE)
        3) use mcGetInfo() to see if memcards are connected
        4) use mcSync to check that the function has finished

        all mc* functions except mcInit() are asynchronous and require mcSync()
        usage to test when they are done
*/

#ifndef __LIBMC_H__
#define __LIBMC_H__

#include <libmc-common.h>

#define MC_WAIT					0
#define MC_NOWAIT				1

#define MC_TYPE_PSX				sceMcTypePS1
#define MC_TYPE_PS2				sceMcTypePS2
#define MC_TYPE_POCKET			sceMcTypePDA
#define MC_TYPE_NONE		sceMcTypeNoCard

#define MC_FORMATTED		1
#define MC_UNFORMATTED		0

// Valid bits in memcard file attributes (mctable.AttrFile)
#define MC_ATTR_READABLE        sceMcFileAttrReadable
#define MC_ATTR_WRITEABLE       sceMcFileAttrWriteable
#define MC_ATTR_EXECUTABLE      sceMcFileAttrExecutable
#define MC_ATTR_PROTECTED       sceMcFileAttrDupProhibit
#define MC_ATTR_FILE            sceMcFileAttrFile
#define MC_ATTR_SUBDIR          sceMcFileAttrSubdir
/** File or directory */
#define MC_ATTR_OBJECT          (sceMcFileAttrFile|sceMcFileAttrSubdir)
#define MC_ATTR_CLOSED          sceMcFileAttrClosed
#define MC_ATTR_PDAEXEC         sceMcFileAttrPDAExec
#define MC_ATTR_PSX             sceMcFileAttrPS1
/** not hidden in osdsys, but it is to games */
#define MC_ATTR_HIDDEN          sceMcFileAttrHidden

/** function numbers returned by mcSync in the 'cmd' pointer */
enum MC_FUNC_NUMBERS{
	MC_FUNC_NONE		= 0x00,
	MC_FUNC_GET_INFO,
	MC_FUNC_OPEN,
	MC_FUNC_CLOSE,
	MC_FUNC_SEEK,
	MC_FUNC_READ,
	MC_FUNC_WRITE,
	MC_FUNC_FLUSH		= 0x0A,
	MC_FUNC_MK_DIR,
	MC_FUNC_CH_DIR,
	MC_FUNC_GET_DIR,
	MC_FUNC_SET_INFO,
	MC_FUNC_DELETE,
	MC_FUNC_FORMAT,
	MC_FUNC_UNFORMAT,
	MC_FUNC_GET_ENT,
	MC_FUNC_RENAME,
	MC_FUNC_CHG_PRITY,
	MC_FUNC_ERASE_BLOCK	= 0x5A,
	MC_FUNC_READ_PAGE,
	MC_FUNC_WRITE_PAGE,
};

/**
 * These types show up in the OSD browser when set.
 * If the OSD doesn't know the number it'll display "Unrecognizable Data" or so.
 * AFAIK these have no other effects.
 * Known type IDs for icon.sys file:
 */
enum MCICON_TYPES{
	MCICON_TYPE_SAVED_DATA		= 0,	// "Saved Data (PlayStation(r)2)"
	MCICON_TYPE_SOFTWARE_PS2,		// "Software (PlayStation(r)2)"
	MCICON_TYPE_SOFTWARE_PKT,		// "Software (PocketStation(r))"
	MCICON_TYPE_SETTINGS_DATA,		// "Settings File (PlayStation(r)2)"
    MCICON_TYPE_SYSTEM_DRIVER       // "System driver"; Implemented on SCPH-5XXXX, previous models can't recognize it unless HDD-OSD is active
};

typedef int iconIVECTOR[4];
typedef float iconFVECTOR[4];

typedef struct
{
    /** header = "PS2D" */
    unsigned char  head[4];
    /** filetype, used to be "unknown1" (see MCICON_TYPE_* above) */
    unsigned short type;
    /** new line pos within title name */
    unsigned short nlOffset;
    /** unknown */
    unsigned unknown2;
    /** transparency */
    unsigned trans;
    /** background color for each of the four points */
    iconIVECTOR bgCol[4];
    /** directions of three light sources */
    iconFVECTOR lightDir[3];
    /** colors of each of these sources */
    iconFVECTOR lightCol[3];
    /** ambient light */
    iconFVECTOR lightAmbient;
    /** application title - NOTE: stored in sjis, NOT normal ascii */
    unsigned short title[34];
    /** list icon filename */
    unsigned char view[64];
    /** copy icon filename */
    unsigned char copy[64];
    /** delete icon filename */
    unsigned char del[64];
    /** unknown */
    unsigned char unknown3[512];
} mcIcon;

typedef struct _sceMcTblGetDir {	// size = 64
	sceMcStDateTime _Create;	// 0
	sceMcStDateTime _Modify;	// 8
	u32 FileSizeByte;		// 16
	u16 AttrFile;			// 20
	u16 Reserve1;			// 22
	u32 Reserve2;			// 24
	u32 PdaAplNo;			// 28
	unsigned char EntryName[32];	// 32
} sceMcTblGetDir __attribute__((aligned(64)));

typedef struct
{
    struct
    {
        unsigned char unknown1;
        /** Entry creation date/time (second) */        
        unsigned char sec;
        /** Entry creation date/time (minute) */
        unsigned char min;
        /** Entry creation date/time (hour) */
        unsigned char hour;
        /** Entry creation date/time (day) */
        unsigned char day;
        /** Entry creation date/time (month) */
        unsigned char month;
        /** Entry creation date/time (year) */
        unsigned short year;
    } _create;

    struct
    {
        unsigned char unknown2;
        /** Entry modification date/time (second) */        
        unsigned char sec;
        /** Entry modification date/time (minute) */
        unsigned char min;
        /** Entry modification date/time (hour) */
        unsigned char hour;
        /** Entry modification date/time (day) */
        unsigned char day;
        /** Entry modification date/time (month) */
        unsigned char month;
        /** Entry modification date/time (year) */
        unsigned short year;
    } _modify;

    /** File size (bytes). For a directory entry: 0 */
    unsigned fileSizeByte;
    /** File attribute */
    unsigned short attrFile;
    unsigned short unknown3;
    unsigned unknown4[2];
    /** Entry name */
    unsigned char name[32];
} mcTable __attribute__((deprecated, aligned (64)));

// values to send to mcInit() to use either mcserv or xmcserv
#define MC_TYPE_MC	0
#define MC_TYPE_XMC	1

#ifdef __cplusplus
extern "C" {
#endif

/** init memcard lib
 *
 * @param type MC_TYPE_MC = use MCSERV/MCMAN; MC_TYPE_XMC = use XMCSERV/XMCMAN
 * @return 0 = successful; < 0 = error
 */
int mcInit(int type);

/** get memcard state
 * mcSync result:	 0 = same card as last getInfo call
 *					-1 = formatted card inserted since last getInfo call
 *					-2 = unformatted card inserted since last getInfo call
 *					< -2 = memcard access error (could be due to accessing psx memcard)
 *
 * @param port port number
 * @param slot slot number
 * @param type pointer to get memcard type
 * @param free pointer to get number of free clusters
 * @param format pointer to get whether or not the card is formatted (Note: Originally, sceMcGetInfo didn't have a 5th argument for returning the format status. As this is emulated based on the return value of sceMcSync() when rom0:MCSERV is used, please keep track of the return value from sceMcSync instead!)
 * @return 0 = successful; < 0 = error
 */
int mcGetInfo(int port, int slot, int* type, int* free, int* format);

/** open a file on memcard
 * mcSync returns:	0 or more = file descriptor (success)
 *					< 0 = error
 *
 * @param port port number
 * @param slot slot number
 * @param name filename to open
 * @param mode open file mode (O_RDWR, O_CREAT, etc)
 * @return 0 = successful; < 0 = error
 */
int mcOpen(int port, int slot, const char *name, int mode);

/** close an open file on memcard
 * mcSync returns:	0 if closed successfully
 *					< 0 = error
 *
 * @param fd file descriptor of open file
 * @return 0 successful; < 0 = error
 */
int mcClose(int fd);

/** move memcard file pointer
 * mcSync returns:	0 or more = offset of file pointer from start of file
 *					< 0 = error
 *
 * @param fd file descriptor
 * @param offset number of bytes from origin
 * @param origin initial position for offset
 * @return 0 = successful; < 0 = error
 */
int mcSeek(int fd, int offset, int origin);


/** read from file on memcard
 * mcSync returns:	0 or more = number of bytes read from memcard
 *					< 0 = error
 *
 * @param fd file descriptor
 * @param buffer buffer to read to
 * @param size number of bytes to read
 * @return 0 = successful; < 0 = error
 */
int mcRead(int fd, void *buffer, int size);

/** write to file on memcard
 * mcSync returns:	0 or more = number of bytes written to memcard
 *					< 0 = error
 *
 * @param fd file descriptor
 * @param buffer to write from write
 * @param size number of bytes to read
 * @return 0 = successful; < 0 = error
 */
int mcWrite(int fd, const void *buffer, int size);

/** flush file cache to memcard
 * mcSync returns:	0 if ok
 *					< 0 if error
 *
 * @param fd file descriptor
 * @return 0 = successful; < 0 = error
 */
int mcFlush(int fd);

/** create a dir
 * mcSync returns:	0 if ok
 *					< 0 if error
 *
 * @param port port number
 * @param slot slot number
 * @param name directory name
 * @return 0 = successful; < 0 = error
 */
int mcMkDir(int port, int slot, const char* name);

/** change current dir
 * (can also get current dir)
 * mcSync returns:	0 if ok
 *					< 0 if error
 *
 * @param port port number
 * @param slot slot number
 * @param newDir new dir to change to
 * @param currentDir buffer to get current dir (use 0 if not needed)
 * @return 0 = successful; < 0 = error
 */
int mcChdir(int port, int slot, const char* newDir, char* currentDir);

/** get memcard filelist
 * mcSync result:	 0 or more = number of file entries obtained (success)
 *					-2 = unformatted card
 *					-4 = dirname error
 *
 * @param port port number of memcard
 * @param slot slot number of memcard
 * @param name filename to search for (can use wildcard and relative dirs)
 * @param mode mode: 0 = first call, otherwise = followup call
 * @param maxext maximum number of entries to be written to filetable in 1 call
 * @param table mc table array
 * @return 0 = successful; < 0 = error
 */
int mcGetDir(int port, int slot, const char *name, unsigned mode, int maxent, sceMcTblGetDir* table);

/** change file information
 * mcSync returns:	0 if ok
 *					< 0 if error
 *
 * @param port port number
 * @param slot slot number
 * @param name filename to access
 * @param info data to be changed
 * @param flags flags to show which data is valid
 * @return 0 = successful; < 0 = error
 */
int mcSetFileInfo(int port, int slot, const char* name, const sceMcTblGetDir* info, unsigned flags);

/** delete file
 * mcSync returns:	0 if deleted successfully
 *					< 0 if error
 *
 * @param port port number to delete from
 * @param slot slot number to delete from
 * @param name filename to delete
 * @return 0 = successful; < 0 = error
 */
int mcDelete(int port, int slot, const char *name);

/** format memory card
 * mcSync returns:	0 if ok
 *					< 0 if error
 *
 * @param port port number
 * @param slot slot number
 * @return 0 = success; -1 = error
 */
int mcFormat(int port, int slot);

/** unformat memory card
 * mcSync returns:	0 if ok
 *					< 0 if error
 *
 * @param port port number
 * @param slot slot number
 * @return 0 = success; -1 = error
 */
int mcUnformat(int port, int slot);

/** get free space info
 * mcSync returns:	0 or more = number of free entries (success)
 *					< 0 if error
 *
 * @param port port number
 * @param slot slot number
 * @param path path to be checked
 * @return 0 or more = number of empty entries; -1 = error
 */
int mcGetEntSpace(int port, int slot, const char* path);

/** rename file or dir on memcard
 * Note: rom0:MCSERV does not support this.
 * mcSync returns:	0 if ok
 *					< 0 if error
 *
 * @param port port number
 * @param slot slot number
 * @param oldName name of file/dir to rename
 * @param newName new name to give to file/dir
 * @return 1 = success; < 0 = error
 */
int mcRename(int port, int slot, const char* oldName, const char* newName);

/** Erases a block on the memory card.
 * Note: rom0:XMCSERV does not support this.
 * mcSync returns:	0 if ok
 *					< 0 if error
 *
 * @param port port number
 * @param slot slot number
 * @param block Block number of the block to be erased.
 * @param mode Mode: -1 to inhibit ECC recalculation of the erased block's pages (useful if sceMcWritePage is used to fill in its contents later on), 0 for normal operation.
 * @return 0 = success; -1 = error
 */
int mcEraseBlock(int port, int slot, int block, int mode);

/** Reads a page from the memory card.
 * Note: rom0:XMCSERV does not support this.
 * mcSync returns:	0 if ok
 *					< 0 if error
 *
 * @param port port number
 * @param slot slot number
 * @param page Page number of the page to be read.
 * @param buffer Pointer to buffer that will contain the read data.
 * @return 0 = success; -1 = error
 */
int mcReadPage(int port, int slot, unsigned int page, void *buffer);

/** Writes a page to the memory card. (The block which the page resides on must be erased first!)
 * Note: rom0:XMCSERV does not support this.
 * mcSync returns:	0 if ok
 *					< 0 if error
 *
 * @param port port number
 * @param slot slot number
 * @param page Page number of the page to be written.
 * @param buffer Pointer to buffer containing data to be written.
 * @return 0 = success; -1 = error
 */
int mcWritePage(int port, int slot, int page, const void *buffer);

/** change mcserv thread priority
 * (I don't think this is implemented properly)
 * Note: rom0:MCSERV does not support this.
 * mcSync returns:	0 if ok
 *					< 0 if error
 *
 * @param level thread priority
 * @return 0 = success; -1 = error
 */
int mcChangeThreadPriority(int level);

/** wait for mc functions to finish or check if they have finished yet
 *
 * @param mode mode 0=wait till function finishes, 1=check function status
 * @param cmd pointer for storing the number of the currenlty processing function
 * @param result pointer for storing result of function if it finishes
 * @return 0 = function is still executing (mode=1); 1 = function has finished executing; -1 = no function registered
 */
int mcSync(int mode, int *cmd, int *result);

/** Reset (force deinit) of library
 *
 * @return 0 = success
 */
int mcReset(void);

#ifdef __cplusplus
}
#endif

#endif /* __LIBMC_H__ */

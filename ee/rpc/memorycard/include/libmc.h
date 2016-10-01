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
# Macro's, structures & function prototypes for mclib.
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

#ifndef _MCLIB_H_
#define _MCLIB_H_

#ifdef __cplusplus
extern "C" {
#endif

#define MC_WAIT 0
#define MC_NOWAIT 1

#define MC_TYPE_PSX 1
#define MC_TYPE_PS2 2
#define MC_TYPE_POCKET 3
#define MC_TYPE_NONE 0

#define MC_FORMATTED 1
#define MC_UNFORMATTED 0

// Valid bits in memcard file attributes (mctable.AttrFile)
#define MC_ATTR_READABLE 0x0001
#define MC_ATTR_WRITEABLE 0x0002
#define MC_ATTR_EXECUTABLE 0x0004
#define MC_ATTR_PROTECTED 0x0008
#define MC_ATTR_FILE 0x0010
#define MC_ATTR_SUBDIR 0x0020
#define MC_ATTR_OBJECT 0x0030  // File or directory
#define MC_ATTR_CLOSED 0x0080
#define MC_ATTR_PDAEXEC 0x0800
#define MC_ATTR_PSX 0x1000
#define MC_ATTR_HIDDEN 0x2000  // not hidden in osdsys, but it is to games

// function numbers returned by mcSync in the 'cmd' pointer
enum MC_FUNC_NUMBERS {
    MC_FUNC_NONE = 0x00,
    MC_FUNC_GET_INFO,
    MC_FUNC_OPEN,
    MC_FUNC_CLOSE,
    MC_FUNC_SEEK,
    MC_FUNC_READ,
    MC_FUNC_WRITE,
    MC_FUNC_FLUSH = 0x0A,
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
    MC_FUNC_ERASE_BLOCK = 0x5A,
    MC_FUNC_READ_PAGE,
    MC_FUNC_WRITE_PAGE,
};

// These types show up in the OSD browser when set.
// If the OSD doesn't know the number it'll display "Unrecognizable Data" or so.
// AFAIK these have no other effects.
// Known type IDs for icon.sys file:
enum MCICON_TYPES {
    MCICON_TYPE_SAVED_DATA = 0,  // "Saved Data (PlayStation(r)2)"
    MCICON_TYPE_SOFTWARE_PS2,    // "Software (PlayStation(r)2)"
    MCICON_TYPE_SOFTWARE_PKT,    // "Software (PocketStation(r))"
    MCICON_TYPE_SETTINGS_DATA    // "Settings File (PlayStation(r)2)"
};

typedef int iconIVECTOR[4];
typedef float iconFVECTOR[4];

typedef struct
{
    unsigned char head[4];        // header = "PS2D"
    unsigned short type;          // filetype, used to be "unknown1" (see MCICON_TYPE_* above)
    unsigned short nlOffset;      // new line pos within title name
    unsigned unknown2;            // unknown
    unsigned trans;               // transparency
    iconIVECTOR bgCol[4];         // background color for each of the four points
    iconFVECTOR lightDir[3];      // directions of three light sources
    iconFVECTOR lightCol[3];      // colors of each of these sources
    iconFVECTOR lightAmbient;     // ambient light
    unsigned short title[34];     // application title - NOTE: stored in sjis, NOT normal ascii
    unsigned char view[64];       // list icon filename
    unsigned char copy[64];       // copy icon filename
    unsigned char del[64];        // delete icon filename
    unsigned char unknown3[512];  // unknown
} mcIcon;


typedef struct
{
    struct
    {
        unsigned char unknown1;
        unsigned char sec;    // Entry creation date/time (second)
        unsigned char min;    // Entry creation date/time (minute)
        unsigned char hour;   // Entry creation date/time (hour)
        unsigned char day;    // Entry creation date/time (day)
        unsigned char month;  // Entry creation date/time (month)
        unsigned short year;  // Entry creation date/time (year)
    } _create;

    struct
    {
        unsigned char unknown2;
        unsigned char sec;    // Entry modification date/time (second)
        unsigned char min;    // Entry modification date/time (minute)
        unsigned char hour;   // Entry modification date/time (hour)
        unsigned char day;    // Entry modification date/time (day)
        unsigned char month;  // Entry modification date/time (month)
        unsigned short year;  // Entry modification date/time (year)
    } _modify;

    unsigned fileSizeByte;    // File size (bytes). For a directory entry: 0
    unsigned short attrFile;  // File attribute
    unsigned short unknown3;
    unsigned unknown4[2];
    unsigned char name[32];  //Entry name
} mcTable __attribute__((aligned(64)));


// values to send to mcInit() to use either mcserv or xmcserv
#define MC_TYPE_MC 0
#define MC_TYPE_XMC 1


// init memcard lib
//
// args:	MC_TYPE_MC  = use MCSERV/MCMAN
//			MC_TYPE_XMC = use XMCSERV/XMCMAN
// returns:	0   = successful
//			< 0 = error
int mcInit(int type);

// get memcard state
// mcSync result:	 0 = same card as last getInfo call
//					-1 = formatted card inserted since last getInfo call
//					-2 = unformatted card inserted since last getInfo call
//					< -2 = memcard access error (could be due to accessing psx memcard)
//
// args:    port number
//          slot number
//          pointer to get memcard type
//          pointer to get number of free clusters
//          pointer to get whether or not the card is formatted	(Note: Originally, sceMcGetInfo didn't have a 5th argument for returning the format status. As this is emulated based on the return value of sceMcSync() when rom0:MCSERV is used, please keep track of the return value from sceMcSync instead!)
// returns:	0   = successful
//			< 0 = error
int mcGetInfo(int port, int slot, int *type, int *free, int *format);

// open a file on memcard
// mcSync returns:	0 or more = file descriptor (success)
//					< 0 = error
//
// args:	port number
//			slot number
//			filename to open
//			open file mode (O_RDWR, O_CREAT, etc)
// returns:	0   = successful
//			< 0 = error
int mcOpen(int port, int slot, const char *name, int mode);

// close an open file on memcard
// mcSync returns:	0 if closed successfully
//					< 0 = error
//
// args:	file descriptor of open file
// returns:	0   = successful
//			< 0 = error
int mcClose(int fd);

// move memcard file pointer
// mcSync returns:	0 or more = offset of file pointer from start of file
//					< 0 = error
//
// args:	file descriptor
//			number of bytes from origin
//			initial position for offset
// returns:	0   = successful
//			< 0 = error
int mcSeek(int fd, int offset, int origin);

// read from file on memcard
// mcSync returns:	0 or more = number of bytes read from memcard
//					< 0 = error
//
// args:	file descriptor
//			buffer to read to
//			number of bytes to read
// returns:	0   = successful
//			< 0 = error
int mcRead(int fd, void *buffer, int size);

// write to file on memcard
// mcSync returns:	0 or more = number of bytes written to memcard
//					< 0 = error
//
// args:	file descriptor
//			buffer to write from write
// returns:	0   = successful
//			< 0 = error
int mcWrite(int fd, const void *buffer, int size);

// flush file cache to memcard
// mcSync returns:	0 if ok
//					< 0 if error
//
// args:	file descriptor
// returns:	0   = successful
//			< 0 = error
int mcFlush(int fd);

// create a dir
// mcSync returns:	0 if ok
//					< 0 if error
//
// args:	port number
//			slot number
//			directory name
// returns:	0   = successful
//			< 0 = error
int mcMkDir(int port, int slot, const char *name);

// change current dir
// (can also get current dir)
// mcSync returns:	0 if ok
//					< 0 if error
//
// args:	port number
//			slot number
//			new dir to change to
//			buffer to get current dir (use 0 if not needed)
// returns:	0   = successful
//			< 0 = error
int mcChdir(int port, int slot, const char *newDir, char *currentDir);

// get memcard filelist
// mcSync result:	 0 or more = number of file entries obtained (success)
//					-2 = unformatted card
//					-4 = dirname error
//
// args:    port number of memcard
//          slot number of memcard
//          filename to search for (can use wildcard and relative dirs)
//          mode: 0 = first call, otherwise = followup call
//          maximum number of entries to be written to filetable in 1 call
//          mc table array
// returns:	0   = successful
//			< 0 = error
int mcGetDir(int port, int slot, const char *name, unsigned mode, int maxent, mcTable *table);

// change file information
// mcSync returns:	0 if ok
//					< 0 if error
//
// args:	port number
//			slot number
//			filename to access
//			data to be changed
//			flags to show which data is valid
// returns:	0   = successful
//			< 0 = error
int mcSetFileInfo(int port, int slot, const char *name, const mcTable *info, unsigned flags);

// delete file
// mcSync returns:	0 if deleted successfully
//					< 0 if error
//
// args:	port number to delete from
//			slot number to delete from
//			filename to delete
// returns:	0   = successful
//			< 0 = error
int mcDelete(int port, int slot, const char *name);

// format memory card
// mcSync returns:	0 if ok
//					< 0 if error
//
// args:    port number
//          slot number
// returns: 0  = success
//          -1 = error
int mcFormat(int port, int slot);

// unformat memory card
// mcSync returns:	0 if ok
//					< 0 if error
//
// args:    port number
//          slot number
// returns: 0  = success
//          -1 = error
int mcUnformat(int port, int slot);

// get free space info
// mcSync returns:	0 or more = number of free entries (success)
//					< 0 if error
//
// args:	port number
//			slot number
//			path to be checked
// returns:	0 or more = number of empty entries
//			-1 = error
int mcGetEntSpace(int port, int slot, const char *path);

// Note: rom0:MCSERV does not support this.
// rename file or dir on memcard
// mcSync returns:	0 if ok
//					< 0 if error
//
// args:	port number
//			slot number
//			name of file/dir to rename
//			new name to give to file/dir
// returns:	1  = success
//			<0 = error
int mcRename(int port, int slot, const char *oldName, const char *newName);

// Note: rom0:XMCSERV does not support this.
// Erases a block on the memory card.
// mcSync returns:	0 if ok
//					< 0 if error
//
// args:	port number
//			slot number
//			Block number of the block to be erased.
//			Mode: -1 to inhibit ECC recalculation of the erased block's pages (useful if sceMcWritePage is used to fill in its contents later on), 0 for normal operation.
// returns:	0  = success
//			-1 = error
int mcEraseBlock(int port, int slot, int block, int mode);

// Note: rom0:XMCSERV does not support this.
// Reads a page from the memory card.
// mcSync returns:	0 if ok
//					< 0 if error
//
// args:	port number
//			slot number
//			Page number of the page to be read.
//			Pointer to buffer that will contain the read data.
// returns:	0  = success
//			-1 = error
int mcReadPage(int port, int slot, unsigned int page, void *buffer);

// Note: rom0:XMCSERV does not support this.
// Writes a page to the memory card. (The block which the page resides on must be erased first!)
// mcSync returns:	0 if ok
//					< 0 if error
//
// args:	port number
//			slot number
//			Page number of the page to be written.
//			Pointer to buffer containing data to be written.
// returns:	0  = success
//			-1 = error
int mcWritePage(int port, int slot, int page, const void *buffer);

// Note: rom0:MCSERV does not support this.
// change mcserv thread priority
// (i dont think this is implemented properly)
// mcSync returns:	0 if ok
//					< 0 if error
//
// args:	thread priority
// returns:	0  = success
//			-1 = error
int mcChangeThreadPriority(int level);

// wait for mc functions to finish
// or check if they have finished yet
//
// args:	mode 0=wait till function finishes, 1=check function status
//			pointer for storing the number of the currenlty processing function
//			pointer for storing result of function if it finishes
// returns:	0  = function is still executing (mode=1)
//			1  = function has finished executing
//			-1 = no function registered
int mcSync(int mode, int *cmd, int *result);

// Reset (force deinit) of library
//
// returns:	0  = success
int mcReset(void);

#ifdef __cplusplus
}
#endif

#endif  // _MCLIB_H_

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

#define MC_WAIT			0
#define MC_NOWAIT		1

#define MC_TYPE_PSX     1
#define MC_TYPE_PS2     2
#define MC_TYPE_POCKET  3
#define MC_TYPE_NONE    0

// Problem: Format is always 0 with ROM irx's
#define MC_FORMATTED    -1
#define MC_UNFORMATTED  0

// Valid bits in memcard file attributes (mctable.AttrFile)
#define MC_ATTR_READABLE        0x0001
#define MC_ATTR_WRITEABLE       0x0002
#define MC_ATTR_EXECUTABLE      0x0004
#define MC_ATTR_PROTECTED       0x0008
#define MC_ATTR_SUBDIR          0x0020
#define MC_ATTR_CLOSED          0x0080
#define MC_ATTR_PDAEXEC         0x0800
#define MC_ATTR_PSX             0x1000

// function numbers returned by mcSync in the 'cmd' pointer
#define MC_FUNC_GET_INFO	0x01
#define MC_FUNC_OPEN		0x02
#define MC_FUNC_CLOSE		0x03
#define MC_FUNC_SEEK		0x04
#define MC_FUNC_READ		0x05
#define MC_FUNC_WRITE		0x06
#define MC_FUNC_FLUSH		0x0A
#define MC_FUNC_MK_DIR		0x0B
#define MC_FUNC_CH_DIR		0x0C
#define MC_FUNC_GET_DIR		0x0D
#define MC_FUNC_SET_INFO	0x0E
#define MC_FUNC_DELETE		0x0F
#define MC_FUNC_FORMAT		0x10
#define MC_FUNC_UNFORMAT	0x11
#define MC_FUNC_GET_ENT		0x12
#define MC_FUNC_RENAME		0x13
#define MC_FUNC_CHG_PRITY	0x14
//#define MC_FUNC_UNKNOWN_1	0x5A	// mcserv version
//#define MC_FUNC_UNKNOWN_2	0x5C	// mcserv version

typedef int iconIVECTOR[4];
typedef float iconFVECTOR[4];

typedef struct
{
    unsigned char  head[4];     // header = "PS2D"
    unsigned short unknown1;    // unknown
    unsigned short nlOffset;    // new line pos within title name
    unsigned unknown2;          // unknown
    unsigned trans;             // transparency
    iconIVECTOR bgCol[4];       // background color for each of the four points
    iconFVECTOR lightDir[3];    // directions of three light sources
    iconFVECTOR lightCol[3];    // colors of each of these sources
    iconFVECTOR lightAmbient;   // ambient light
    unsigned short title[34];   // application title - NOTE: stored in sjis, NOT normal ascii
    unsigned char view[64];     // list icon filename
    unsigned char copy[64];     // copy icon filename
    unsigned char del[64];      // delete icon filename
    unsigned char unknown3[512];// unknown
} mcIcon;


typedef struct
{
    struct
    {
        unsigned char unknown1;
        unsigned char sec;      // Entry creation date/time (second)
        unsigned char min;      // Entry creation date/time (minute)
        unsigned char hour;     // Entry creation date/time (hour)
        unsigned char day;      // Entry creation date/time (day)
        unsigned char month;    // Entry creation date/time (month)
        unsigned short year;    // Entry creation date/time (year)
    } _create;

    struct
    {
        unsigned char unknown2;
        unsigned char sec;      // Entry modification date/time (second)
        unsigned char min;      // Entry modification date/time (minute)
        unsigned char hour;     // Entry modification date/time (hour)
        unsigned char day;      // Entry modification date/time (day)
        unsigned char month;    // Entry modification date/time (month)
        unsigned short year;    // Entry modification date/time (year)
    } _modify;

    unsigned fileSizeByte;      // File size (bytes). For a directory entry: 0
    unsigned short attrFile;    // File attribute
    unsigned short unknown3;
    unsigned unknown4[2];
    unsigned char name[32];         //Entry name
} mcTable __attribute__((aligned (64)));


// values to send to mcInit() to use either mcserv or xmcserv
#define MC_TYPE_MC	0
#define MC_TYPE_XMC	1


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
//          pointer to get whether or not the card is formatted
// returns:	0   = successful
//			< 0 = error
int mcGetInfo(int port, int slot, int* type, int* free, int* format);

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
int mcMkDir(int port, int slot, const char* name);

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
int mcChdir(int port, int slot, const char* newDir, char* currentDir);

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
int mcGetDir(int port, int slot, const char *name, unsigned mode, int maxent, mcTable* table);

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
int mcSetFileInfo(int port, int slot, const char* name, const char* info, unsigned flags);

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
int mcGetEntSpace(int port, int slot, const char* path);

// rename file or dir on memcard
// mcSync returns:	0 if ok
//					< 0 if error
// 
// args:	port number
//			slot number
//			name of file/dir to rename
//			new name to give to file/dir
// returns:	0  = success
//			-1 = error
int mcRename(int port, int slot, const char* oldName, const char* newName);

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

#ifdef __cplusplus
}
#endif

#endif // _MCLIB_H_

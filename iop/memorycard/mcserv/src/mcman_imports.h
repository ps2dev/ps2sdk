#ifndef MCMAN_IMPORTS_H
#define MCMAN_IMPORTS_H

#include "types.h"
#include "irx.h"

#define MCMAN 	0
#define XMCMAN 	1

char mcman_modname[8] = "mcman\0\0\0";
int mcman_type = MCMAN;

typedef struct t_mcTable {
    struct {
        u8 unknown1;
        u8 sec;      // Entry creation date/time (second)
        u8 min;      // Entry creation date/time (minute)
        u8 hour;     // Entry creation date/time (hour)
        u8 day;      // Entry creation date/time (day)
        u8 month;    // Entry creation date/time (month)
        u16 year;    // Entry creation date/time (year)
    } _create;
    struct {
        u8 unknown2;
        u8 sec;      // Entry modification date/time (second)
        u8 min;      // Entry modification date/time (minute)
        u8 hour;     // Entry modification date/time (hour)
        u8 day;      // Entry modification date/time (day)
        u8 month;    // Entry modification date/time (month)
        u16 year;    // Entry modification date/time (year)
    } _modify;
    u32 fileSizeByte;// File size (bytes). For a directory entry: 0
    u16 attrFile;    // File attribute
    u16 unknown3;
    u32 unknown4[2];
    u8  name[32];    //Entry name
} mcTable_t __attribute__((aligned (64)));


// modInfo struct returned by xmcman exports 42
struct modInfo_t { 
	const char *name;
	u16 version;
};

/* 05 */ int  (*McDetectCard)(int port, int slot);
/* 06 */ int  (*McOpen)(int port, int slot, char *filename, int flags);
/* 07 */ int  (*McClose)(int fd);
/* 08 */ int  (*McRead)(int fd, void *buf, int length);
/* 09 */ int  (*McWrite)(int fd, void *buf, int length);
/* 10 */ int  (*McSeek)(int fd, int offset, int origin);
/* 11 */ int  (*McFormat)(int port, int slot);
/* 12 */ int  (*McGetDir)(int port, int slot, char *dirname, int flags, int nument, mcTable_t *info);
/* 13 */ int  (*McDelete)(int port, int slot, char *filename, int flags);
/* 14 */ int  (*McFlush)(int fd);
/* 15 */ int  (*McChDir)(int port, int slot, char *newdir, char *currentdir);
/* 16 */ int  (*McSetFileInfo)(int port, int slot, char *filename, mcTable_t *info, int flags);
/* 17 */ int  (*McEraseBlock)(int port, int block, void **pagebuf, void *eccbuf);
/* 18 */ int  (*McReadPage)(int port, int slot, int page, void *buf);
/* 19 */ int  (*McWritePage)(int port, int slot, int page, void *pagebuf, void *eccbuf);
/* 20 */ void (*McDataChecksum)(void *buf, void *ecc);
/* 29 */ int  (*McReadPS1PDACard)(int port, int slot, int page, void *buf);
/* 30 */ int  (*McWritePS1PDACard)(int port, int slot, int page, void *buf);
/* 36 */ int  (*McUnformat)(int port, int slot);
/* 37 */ int  (*McRetOnly)(int fd);
/* 38 */ int  (*McGetFreeClusters)(int port, int slot);
/* 39 */ int  (*McGetMcType)(int port, int slot);
/* 40 */ void (*McSetPS1CardFlag)(int flag);

/* Available in XMCMAN only */
/* 21 */ int  (*McDetectCard2)(int port, int slot);
/* 22 */ int  (*McGetFormat)(int port, int slot);
/* 23 */ int  (*McGetEntSpace)(int port, int slot, char *dirname);
/* 24 */ int  (*McReplaceBadBlock)(void);
/* 42 */ struct modInfo_t *(*McGetModuleInfo)(void);
/* 45 */ int  (*McCheckBlock)(int port, int slot, int block);

#endif

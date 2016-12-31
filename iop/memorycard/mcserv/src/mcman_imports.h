#ifndef MCMAN_IMPORTS_H
#define MCMAN_IMPORTS_H

#include "types.h"
#include "irx.h"

#define MCMAN 	0
#define XMCMAN 	1

char mcman_modname[8] = "mcman\0\0\0";
int mcman_type = MCMAN;

typedef struct _sceMcTblGetDir {	// size = 64
	sceMcStDateTime _Create;	// 0
	sceMcStDateTime _Modify;	// 8
	u32 FileSizeByte;		// 16
	u16 AttrFile;			// 20
	u16 Reserve1;			// 22
	u32 Reserve2;			// 24
	u32 PdaAplNo;			// 28
	unsigned char EntryName[32];	// 32
} sceMcTblGetDir;

// filename related mc command
// used by: mcOpen, mcGetDir, mcChdir, mcDelete, mcSetFileInfo, mcRename, mcGetEntSpace
typedef struct {			// size = 1044
	int port;			// 0
	int slot;			// 4
	int flags;			// 8
	int maxent;			// 12
	union {
		sceMcTblGetDir *mcT;	// 16
		char *curdir;
	};
	char name[1024];		// 20
} mcNameParam_t;

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
/* 12 */ int  (*McGetDir)(int port, int slot, char *dirname, int flags, int nument, sceMcTblGetDir *info);
/* 13 */ int  (*McDelete)(int port, int slot, char *filename, int flags);
/* 14 */ int  (*McFlush)(int fd);
/* 15 */ int  (*McChDir)(int port, int slot, char *newdir, char *currentdir);
/* 16 */ int  (*McSetFileInfo)(int port, int slot, char *filename, sceMcTblGetDir *info, int flags);
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

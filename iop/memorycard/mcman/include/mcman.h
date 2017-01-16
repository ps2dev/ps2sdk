/*
# _____     ___ ____     ___ ____
#  ____|   |    ____|   |        | |____|
# |     ___|   |____ ___|    ____| |    \    PS2DEV Open Source Project.
#-----------------------------------------------------------------------
# Copyright (c) 2009 jimmikaelkael
# Licenced under Academic Free License version 2.0
# Review ps2sdk README & LICENSE files for further details.
*/

#ifndef __MCMAN_H__
#define __MCMAN_H__

#include <libmc-common.h>

/* MCMAN public structure */
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

/* MCMAN EXPORTS */
/* 05 */ int  McDetectCard(int port, int slot);
/* 06 */ int  McOpen(int port, int slot, char *filename, int flags);
/* 07 */ int  McClose(int fd);
/* 08 */ int  McRead(int fd, void *buf, int length);
/* 09 */ int  McWrite(int fd, void *buf, int length);
/* 10 */ int  McSeek(int fd, int offset, int origin);
/* 11 */ int  McFormat(int port, int slot);
/* 12 */ int  McGetDir(int port, int slot, char *dirname, int flags, int maxent, sceMcTblGetDir *info);
/* 13 */ int  McDelete(int port, int slot, char *filename, int flags);
/* 14 */ int  McFlush(int fd);
/* 15 */ int  McChDir(int port, int slot, char *newdir, char *currentdir);
/* 16 */ int  McSetFileInfo(int port, int slot, char *filename, sceMcTblGetDir *info, int flags);
/* 17 */ int  McEraseBlock(int port, int block, void **pagebuf, void *eccbuf);
/* 18 */ int  McReadPage(int port, int slot, int page, void *buf);
/* 19 */ int  McWritePage(int port, int slot, int page, void *pagebuf, void *eccbuf);
/* 20 */ void McDataChecksum(void *buf, void *ecc);
/* 29 */ int  McReadPS1PDACard(int port, int slot, int page, void *buf);
/* 30 */ int  McWritePS1PDACard(int port, int slot, int page, void *buf);
/* 36 */ int  McUnformat(int port, int slot);
/* 37 */ int  McRetOnly(int fd);
/* 38 */ int  McGetFreeClusters(int port, int slot);
/* 39 */ int  McGetMcType(int port, int slot);
/* 40 */ void McSetPS1CardFlag(int flag);

/* Available in XMCMAN only */
/* 17 */ int  McEraseBlock2(int port, int slot, int block, void **pagebuf, void *eccbuf);
/* 21 */ int  McDetectCard2(int port, int slot);
/* 22 */ int  McGetFormat(int port, int slot);
/* 23 */ int  McGetEntSpace(int port, int slot, char *dirname);
/* 24 */ int  mcman_replacebadblock(void);
/* 25 */ int  McCloseAll(void);
/* 42 */ struct irx_id *McGetModuleInfo(void);
/* 43 */ int  McGetCardSpec(int port, int slot, s16 *pagesize, u16 *blocksize, int *cardsize, u8 *flags);
/* 44 */ int  mcman_getFATentry(int port, int slot, int fat_index, int *fat_entry);
/* 45 */ int  McCheckBlock(int port, int slot, int block);
/* 46 */ int  mcman_setFATentry(int port, int slot, int fat_index, int fat_entry);
/* 47 */ int  mcman_readdirentry(int port, int slot, int cluster, int fsindex, McFsEntry **pfse);
/* 48 */ void mcman_1stcacheEntsetwrflagoff(void);

#endif

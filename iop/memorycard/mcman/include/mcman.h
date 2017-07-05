/*
# _____     ___ ____     ___ ____
#  ____|   |    ____|   |        | |____|
# |     ___|   |____ ___|    ____| |    \    PS2DEV Open Source Project.
#-----------------------------------------------------------------------
# Copyright (c) 2009 jimmikaelkael
# Licenced under Academic Free License version 2.0
# Review ps2sdk README & LICENSE files for further details.
*/

/**
 * @file
 * Memory card manager definitions
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
#define mcman_IMPORTS_start DECLARE_IMPORT_TABLE(mcman, 1, 1)
#define mcman_IMPORTS_end END_IMPORT_TABLE

#define xmcman_IMPORTS_start DECLARE_IMPORT_TABLE(mcman, 2, 3)
#define xmcman_IMPORTS_end END_IMPORT_TABLE

/* 05 */ int  McDetectCard(int port, int slot);
#define	I_McDetectCard DECLARE_IMPORT(5, McDetectCard)
/* 06 */ int  McOpen(int port, int slot, char *filename, int flags);
#define	I_McOpen DECLARE_IMPORT(6, McOpen)
/* 07 */ int  McClose(int fd);
#define	I_McClose DECLARE_IMPORT(7, McClose)
/* 08 */ int  McRead(int fd, void *buf, int length);
#define	I_McRead DECLARE_IMPORT(8, McRead)
/* 09 */ int  McWrite(int fd, void *buf, int length);
#define	I_McRead DECLARE_IMPORT(8, McRead)
/* 10 */ int  McSeek(int fd, int offset, int origin);
#define	I_McSeek DECLARE_IMPORT(10, McSeek)
/* 11 */ int  McFormat(int port, int slot);
#define	I_McFormat DECLARE_IMPORT(11, McFormat)
/* 12 */ int  McGetDir(int port, int slot, char *dirname, int flags, int maxent, sceMcTblGetDir *info);
#define	I_McGetDir DECLARE_IMPORT(12, McGetDir)
/* 13 */ int  McDelete(int port, int slot, char *filename, int flags);
#define	I_McDelete DECLARE_IMPORT(13, McDelete)
/* 14 */ int  McFlush(int fd);
#define	I_McFlush DECLARE_IMPORT(14, McFlush)
/* 15 */ int  McChDir(int port, int slot, char *newdir, char *currentdir);
#define	I_McFlush DECLARE_IMPORT(14, McFlush)
/* 16 */ int  McSetFileInfo(int port, int slot, char *filename, sceMcTblGetDir *info, int flags);
#define	I_McSetFileInfo DECLARE_IMPORT(16, McSetFileInfo)
/* 17 */ int  McEraseBlock(int port, int block, void **pagebuf, void *eccbuf);	//MCMAN v1.1 does not have a slot argument
#define	I_McEraseBlock DECLARE_IMPORT(17, McEraseBlock)
/* 18 */ int  McReadPage(int port, int slot, int page, void *buf);
#define	I_McReadPage DECLARE_IMPORT(18, McReadPage)
/* 19 */ int  McWritePage(int port, int slot, int page, void *pagebuf, void *eccbuf);
#define	I_McWritePage DECLARE_IMPORT(19, McWritePage)
/* 20 */ void McDataChecksum(void *buf, void *ecc);
#define	I_McDataChecksum DECLARE_IMPORT(20, McDataChecksum);
/* 29 */ int  McReadPS1PDACard(int port, int slot, int page, void *buf);
#define	I_McReadPS1PDACard DECLARE_IMPORT(29,  McReadPS1PDACard)
/* 30 */ int  McWritePS1PDACard(int port, int slot, int page, void *buf);
#define	I_McWritePS1PDACard DECLARE_IMPORT(30, McWritePS1PDACard)
/* 36 */ int  McUnformat(int port, int slot);
#define	I_McUnformat DECLARE_IMPORT(36, McUnformat)
/* 37 */ int  McRetOnly(int fd);
#define	I_McRetOnly DECLARE_IMPORT(37, McRetOnly)
/* 38 */ int  McGetFreeClusters(int port, int slot);
#define	I_McGetFreeClusters DECLARE_IMPORT(38, McGetFreeClusters)
/* 39 */ int  McGetMcType(int port, int slot);
#define	I_McGetMcType DECLARE_IMPORT(39, McGetMcType)
/* 40 */ void McSetPS1CardFlag(int flag);
#define	I_McSetPS1CardFlag DECLARE_IMPORT(40, McSetPS1CardFlag)

/* Available in XMCMAN only */
/* 17 */ int  McEraseBlock2(int port, int slot, int block, void **pagebuf, void *eccbuf);
#define	I_McEraseBlock2 DECLARE_IMPORT(17, McEraseBlock2)
/* 21 */ int  McDetectCard2(int port, int slot);
#define	I_McDetectCard2 DECLARE_IMPORT(21, McDetectCard2)
/* 22 */ int  McGetFormat(int port, int slot);
#define	I_McGetFormat DECLARE_IMPORT(22, McGetFormat)
/* 23 */ int  McGetEntSpace(int port, int slot, char *dirname);
#define	I_McGetEntSpace DECLARE_IMPORT(23, McGetEntSpace)
/* 24 */ int  McReplaceBadBlock(void);
#define	I_McReplaceBadBlock DECLARE_IMPORT(24, McReplaceBadBlock)
/* 25 */ int  McCloseAll(void);
#define	I_McCloseAll DECLARE_IMPORT(25, McCloseAll)
/* 42 */ struct irx_id *McGetModuleInfo(void);
#define	I_McGetModuleInfo DECLARE_IMPORT(42, McGetModuleInfo)
/* 43 */ int  McGetCardSpec(int port, int slot, s16 *pagesize, u16 *blocksize, int *cardsize, u8 *flags);
#define	I_McGetCardSpec DECLARE_IMPORT(43,  McGetCardSpec)
/* 44 */ int  McGetFATentry(int port, int slot, int fat_index, int *fat_entry);
#define	I_McGetFATentry DECLARE_IMPORT(44, McGetFATentry)
/* 45 */ int  McCheckBlock(int port, int slot, int block);
#define	I_McCheckBlock DECLARE_IMPORT(45,McCheckBlock)
/* 46 */ int  McSetFATentry(int port, int slot, int fat_index, int fat_entry);
#define	I_McSetFATentry DECLARE_IMPORT(46, McSetFATentry)
/* 47 */ int  McReadDirEntry(int port, int slot, int cluster, int fsindex, McFsEntry **pfse);
#define	I_McReadDirEntry DECLARE_IMPORT(47, McReadDirEntry)
/* 48 */ void Mc1stCacheEntSetWrFlagOff(void);
#define	I_Mc1stCacheEntSetWrFlagOff DECLARE_IMPORT(48, Mc1stCacheEntSetWrFlagOff)
/* 49 */ int McCreateDirentry(int port, int slot, int parent_cluster, int num_entries, int cluster, sceMcStDateTime *ctime);
#define	I_McCreateDirentry DECLARE_IMPORT(49, McCreateDirentry)
/* 50 */ int McReadCluster(int port, int slot, int cluster, McCacheEntry **pmce);
#define	I_McReadCluster DECLARE_IMPORT(50, McReadCluster)
/* 51 */ int McFlushCache(int port, int slot);
#define	I_McFlushCache DECLARE_IMPORT(51, McFlushCache)
/* 52 */ int McSetDirEntryState(int port, int slot, int cluster, int fsindex, int flags);
#define	I_McSetDirEntryState DECLARE_IMPORT(52, McSetDirEntryState)

#endif	//__MCMAN_H__

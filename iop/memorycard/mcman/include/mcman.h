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

#include <types.h>
#include <irx.h>
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
	char EntryName[32];	// 32
} sceMcTblGetDir;

int  McDetectCard(int port, int slot);
int  McOpen(int port, int slot, const char *filename, int flags);
int  McClose(int fd);
int  McRead(int fd, void *buf, int length);
int  McWrite(int fd, void *buf, int length);
int  McSeek(int fd, int offset, int origin);
int  McFormat(int port, int slot);
int  McGetDir(int port, int slot, const char *dirname, int flags, int maxent, sceMcTblGetDir *info);
int  McDelete(int port, int slot, const char *filename, int flags);
int  McFlush(int fd);
int  McChDir(int port, int slot, const char *newdir, char *currentdir);
int  McSetFileInfo(int port, int slot, const char *filename, sceMcTblGetDir *info, int flags);
int  McEraseBlock(int port, int block, void **pagebuf, void *eccbuf);	//MCMAN v1.1 does not have a slot argument
int  McReadPage(int port, int slot, int page, void *buf);
int  McWritePage(int port, int slot, int page, void *pagebuf, void *eccbuf);
void McDataChecksum(void *buf, void *ecc);
int  McReadPS1PDACard(int port, int slot, int page, void *buf);
int  McWritePS1PDACard(int port, int slot, int page, void *buf);
int  McUnformat(int port, int slot);
int  McRetOnly(int fd);
int  McGetFreeClusters(int port, int slot);
int  McGetMcType(int port, int slot);
void McSetPS1CardFlag(int flag);

/* Available in XMCMAN only */
int  McEraseBlock2(int port, int slot, int block, void **pagebuf, void *eccbuf);
int  McDetectCard2(int port, int slot);
int  McGetFormat(int port, int slot);
int  McGetEntSpace(int port, int slot, const char *dirname);
int  McReplaceBadBlock(void);
int  McCloseAll(void);
#ifdef _IOP
struct irx_id *McGetModuleInfo(void);
#endif
int  McGetCardSpec(int port, int slot, s16 *pagesize, u16 *blocksize, int *cardsize, u8 *flags);
int  McGetFATentry(int port, int slot, int fat_index, int *fat_entry);
int  McCheckBlock(int port, int slot, int block);
int  McSetFATentry(int port, int slot, int fat_index, int fat_entry);
int  McReadDirEntry(int port, int slot, int cluster, int fsindex, McFsEntry **pfse);
void Mc1stCacheEntSetWrFlagOff(void);
int McCreateDirentry(int port, int slot, int parent_cluster, int num_entries, int cluster, const sceMcStDateTime *ctime);
int McReadCluster(int port, int slot, int cluster, McCacheEntry **pmce);
int McFlushCache(int port, int slot);
int McSetDirEntryState(int port, int slot, int cluster, int fsindex, int flags);

#define xfromman_IMPORTS_start DECLARE_IMPORT_TABLE(xfromman, 2, 3)
#define xfromman_IMPORTS_end END_IMPORT_TABLE

#define mcman_IMPORTS_start DECLARE_IMPORT_TABLE(mcman, 1, 1)
#define mcman_IMPORTS_end END_IMPORT_TABLE

#define xmcman_IMPORTS_start DECLARE_IMPORT_TABLE(mcman, 2, 3)
#define xmcman_IMPORTS_end END_IMPORT_TABLE

#define I_McDetectCard DECLARE_IMPORT(5, McDetectCard)
#define I_McOpen DECLARE_IMPORT(6, McOpen)
#define I_McClose DECLARE_IMPORT(7, McClose)
#define I_McRead DECLARE_IMPORT(8, McRead)
#define I_McWrite DECLARE_IMPORT(9, McWrite)
#define I_McSeek DECLARE_IMPORT(10, McSeek)
#define I_McFormat DECLARE_IMPORT(11, McFormat)
#define I_McGetDir DECLARE_IMPORT(12, McGetDir)
#define I_McDelete DECLARE_IMPORT(13, McDelete)
#define I_McFlush DECLARE_IMPORT(14, McFlush)
#define I_McChDir DECLARE_IMPORT(15, McChDir)
#define I_McSetFileInfo DECLARE_IMPORT(16, McSetFileInfo)
#define I_McEraseBlock DECLARE_IMPORT(17, McEraseBlock)
#define I_McReadPage DECLARE_IMPORT(18, McReadPage)
#define I_McWritePage DECLARE_IMPORT(19, McWritePage)
#define I_McDataChecksum DECLARE_IMPORT(20, McDataChecksum);
#define I_McReadPS1PDACard DECLARE_IMPORT(29,  McReadPS1PDACard)
#define I_McWritePS1PDACard DECLARE_IMPORT(30, McWritePS1PDACard)
#define I_McUnformat DECLARE_IMPORT(36, McUnformat)
#define I_McRetOnly DECLARE_IMPORT(37, McRetOnly)
#define I_McGetFreeClusters DECLARE_IMPORT(38, McGetFreeClusters)
#define I_McGetMcType DECLARE_IMPORT(39, McGetMcType)
#define I_McSetPS1CardFlag DECLARE_IMPORT(40, McSetPS1CardFlag)
#define I_McEraseBlock2 DECLARE_IMPORT(17, McEraseBlock2)
#define I_McDetectCard2 DECLARE_IMPORT(21, McDetectCard2)
#define I_McGetFormat DECLARE_IMPORT(22, McGetFormat)
#define I_McGetEntSpace DECLARE_IMPORT(23, McGetEntSpace)
#define I_McReplaceBadBlock DECLARE_IMPORT(24, McReplaceBadBlock)
#define I_McCloseAll DECLARE_IMPORT(25, McCloseAll)
#define I_McGetModuleInfo DECLARE_IMPORT(42, McGetModuleInfo)
#define I_McGetCardSpec DECLARE_IMPORT(43,  McGetCardSpec)
#define I_McGetFATentry DECLARE_IMPORT(44, McGetFATentry)
#define I_McCheckBlock DECLARE_IMPORT(45, McCheckBlock)
#define I_McSetFATentry DECLARE_IMPORT(46, McSetFATentry)
#define I_McReadDirEntry DECLARE_IMPORT(47, McReadDirEntry)
#define I_Mc1stCacheEntSetWrFlagOff DECLARE_IMPORT(48, Mc1stCacheEntSetWrFlagOff)
#define I_McCreateDirentry DECLARE_IMPORT(49, McCreateDirentry)
#define I_McReadCluster DECLARE_IMPORT(50, McReadCluster)
#define I_McFlushCache DECLARE_IMPORT(51, McFlushCache)
#define I_McSetDirEntryState DECLARE_IMPORT(52, McSetDirEntryState)

#endif /* __MCMAN_H__ */

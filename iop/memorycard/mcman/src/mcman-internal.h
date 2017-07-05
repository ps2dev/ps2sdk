/*
# _____     ___ ____     ___ ____
#  ____|   |    ____|   |        | |____|
# |     ___|   |____ ___|    ____| |    \    PS2DEV Open Source Project.
#-----------------------------------------------------------------------
# Copyright (c) 2009 jimmikaelkael
# Licenced under Academic Free License version 2.0
# Review ps2sdk README & LICENSE files for further details.
*/

#ifndef __MCMAN_INTERNAL_H__
#define __MCMAN_INTERNAL_H__

#include <loadcore.h>
#include <intrman.h>
#include <sysclib.h>
#include <thbase.h>
#include <thsemap.h>
#include <timrman.h>
#include <modload.h>
#include <ioman.h>
#include <secrman.h>
#include <cdvdman.h>
#include <stdio.h>
#include <errno.h>
#include <io_common.h>
#include "sio2man_imports.h"

#ifdef SIO_DEBUG
	#include <sior.h>
	#define DEBUG
	#define DPRINTF(args...)	sio_printf(args)
#else
	#define DPRINTF(args...)	printf(args)
#endif

#define MODNAME "mcman_cex"
#define MODVER  0x20b

typedef struct _MCCacheDir {
	int  cluster;   // 0
	int  fsindex;   // 4
	int  maxent;    // 8
	u32  unused;
} McCacheDir;

// Card Flags
#define CF_USE_ECC   				0x01
#define CF_BAD_BLOCK 				0x08
#define CF_ERASE_ZEROES 			0x10

#define MCMAN_MAXSLOT				4
#define MCMAN_CLUSTERSIZE 			1024
#define MCMAN_CLUSTERFATENTRIES		256

typedef struct _McFatCluster {
	int entry[MCMAN_CLUSTERFATENTRIES];
} McFatCluster;

#define MAX_CACHEENTRY 			0x24

typedef struct {
	int entry[1 + (MCMAN_CLUSTERFATENTRIES * 2)];
} McFatCache;

#define MAX_CACHEDIRENTRY 		0x3

typedef struct {  // size = 48
	u8  status;   // 0
	u8  wrflag;   // 1
	u8  rdflag;   // 2
	u8  unknown1; // 3
	u8  drdflag;  // 4
	u8  unknown2; // 5
	u16 port;     // 6
	u16 slot;     // 8
	u16 unknown3; // 10
	u32 position; // 12
	u32 filesize; // 16
	u32 freeclink; // 20 link to next free cluster
	u32 clink;	  // 24  link to next cluster
	u32 clust_offset;// 28
	u32 field_20; // 32
	u32 field_24; // 36
	u32 field_28; // 40
	u32 field_2C; // 44
} MC_FHANDLE;

#define MAX_FDHANDLES 		3

// internal functions prototypes
int  mcsio2_transfer(int port, int slot, sio2_transfer_data_t *sio2data);
int  mcsio2_transfer2(int port, int slot, sio2_transfer_data_t *sio2data);
void long_multiply(u32 v1, u32 v2, u32 *HI, u32 *LO);
int  mcman_chrpos(char *str, int chr);
void mcman_wmemset(void *buf, int size, int value);
int  mcman_calcEDC(void *buf, int size);
int  mcman_checkpath(char *str);
int  mcman_checkdirpath(char *str1, char *str2);
void mcman_invhandles(int port, int slot);
int  McCloseAll(void);
int  mcman_detectcard(int port, int slot);
int  mcman_dread(int fd, fio_dirent_t *dirent);
int  mcman_getstat(int port, int slot, char *filename, fio_stat_t *stat);
int  mcman_getmcrtime(sceMcStDateTime *time);
void mcman_initPS2com(void);
void sio2packet_add(int port, int slot, int cmd, u8 *buf);
int  mcman_eraseblock(int port, int slot, int block, void **pagebuf, void *eccbuf);
int  mcman_readpage(int port, int slot, int page, void *buf, void *eccbuf);
int  mcman_cardchanged(int port, int slot);
int  mcman_resetauth(int port, int slot);
int  mcman_probePS2Card2(int port, int slot);
int  mcman_probePS2Card(int port, int slot);
int  secrman_mc_command(int port, int slot, sio2_transfer_data_t *sio2data);
int  mcman_getcnum (int port, int slot);
int  mcman_correctdata(void *buf, void *ecc);
int  mcman_sparesize(int port, int slot);
int  mcman_setdevspec(int port, int slot);
int  mcman_reportBadBlocks(int port, int slot);
int  mcman_setdevinfos(int port, int slot);
int  mcman_format2(int port, int slot);
int  mcman_fatRseek(int fd);
int  mcman_fatWseek(int fd);
int  mcman_findfree2(int port, int slot, int reserve);
int  mcman_dread2(int fd, fio_dirent_t *dirent);
int  mcman_getstat2(int port, int slot, char *filename, fio_stat_t *stat);
int  mcman_setinfo2(int port, int slot, char *filename, sceMcTblGetDir *info, int flags);
int  mcman_read2(int fd, void *buffer, int nbyte);
int  mcman_write2(int fd, void *buffer, int nbyte);
int  mcman_close2(int fd);
int  mcman_getentspace(int port, int slot, char *dirname);
int  mcman_cachedirentry(int port, int slot, char *filename, McCacheDir *pcacheDir, McFsEntry **pfse, int unknown_flag);
int  mcman_getdirinfo(int port, int slot, McFsEntry *pfse, char *filename, McCacheDir *pcd, int unknown_flag);
int  mcman_open2(int port, int slot, char *filename, int flags);
int  mcman_chdir(int port, int slot, char *newdir, char *currentdir);
int  mcman_writecluster(int port, int slot, int cluster, int flag);
int  mcman_getdir2(int port, int slot, char *dirname, int flags, int maxent, sceMcTblGetDir *info);
int  mcman_delete2(int port, int slot, char *filename, int flags);
int  mcman_checkBackupBlocks(int port, int slot);
int  mcman_unformat2(int port, int slot);
void mcman_initPS1PDAcom(void);
int  mcman_probePS1Card2(int port, int slot);
int  mcman_probePS1Card(int port, int slot);
int  mcman_probePDACard(int port, int slot);
int  mcman_setPS1devinfos(int port, int slot);
int  mcman_format1(int port, int slot);
int  mcman_open1(int port, int slot, char *filename, int flags);
int  mcman_read1(int fd, void *buffer, int nbyte);
int  mcman_write1(int fd, void *buffer, int nbyte);
int  mcman_getPS1direntry(int port, int slot, char *filename, McFsEntryPS1 **pfse, int flag);
int  mcman_dread1(int fd, fio_dirent_t *dirent);
int  mcman_getstat1(int port, int slot, char *filename, fio_stat_t *stat);
int  mcman_setinfo1(int port, int slot, char *filename, sceMcTblGetDir *info, int flags);
int  mcman_getdir1(int port, int slot, char *dirname, int flags, int maxent, sceMcTblGetDir *info);
int  mcman_clearPS1direntry(int port, int slot, int cluster, int flags);
int  mcman_delete1(int port, int slot, char *filename, int flags);
int  mcman_close1(int fd);
int  mcman_findfree1(int port, int slot, int reserve);
int  mcman_fatRseekPS1(int fd);
int  mcman_fatWseekPS1(int fd);
int  mcman_FNC8ca4(int port, int slot, MC_FHANDLE *fh);
int  mcman_PS1pagetest(int port, int slot, int page);
int  mcman_unformat1(int port, int slot);
int  mcman_cachePS1dirs(int port, int slot);
int  mcman_fillPS1backuparea(int port, int slot, int block);
void mcman_initcache(void);
int  mcman_clearcache(int port, int slot);
McCacheEntry *mcman_getcacheentry(int port, int slot, int cluster);
void mcman_freecluster(int port, int slot, int cluster);
int  mcman_getFATindex(int port, int slot, int num);
McCacheEntry *mcman_get1stcacheEntp(void);
void mcman_addcacheentry(McCacheEntry *mce);
int  mcman_flushcacheentry(McCacheEntry *mce);
int  mcman_readdirentryPS1(int port, int slot, int cluster, McFsEntryPS1 **pfse);
int  mcman_readclusterPS1(int port, int slot, int cluster, McCacheEntry **pmce);
int  mcman_replaceBackupBlock(int port, int slot, int block);
int  mcman_fillbackupblock1(int port, int slot, int block, void **pagedata, void *eccdata);
int  mcman_clearsuperblock(int port, int slot);
int  mcman_ioerrcode(int errcode);
int  mcman_modloadcb(char *filename, int *port, int *slot); // used as callback by modload
void mcman_unit2card(u32 unit);
int  mcman_initdev(void);

typedef struct { 				// size = 384
    char  magic[28];				// Superblock magic, on PS2 MC : "Sony PS2 Memory Card Format "
    u8  version[12];  			// Version number of the format used, 1.2 indicates full support for bad_block_list
    s16 pagesize;				// size in bytes of a memory card page
    u16 pages_per_cluster;		// number of pages in a cluster
    u16 blocksize;				// number of pages in an erase block
    u16 unused;					// unused
    u32 clusters_per_card;		// total size in clusters of the memory card
    u32 alloc_offset;			// Cluster offset of the first allocatable cluster. Cluster values in the FAT and directory entries are relative to this. This is the cluster immediately after the FAT
    u32 alloc_end;				// The cluster after the highest allocatable cluster. Relative to alloc_offset. Not used
    u32 rootdir_cluster;		// First cluster of the root directory. Relative to alloc_offset. Must be zero
    u32 backup_block1;			// Erase block used as a backup area during programming. Normally the the last block on the card, it may have a different value if that block was found to be bad
    u32 backup_block2;			// This block should be erased to all ones. Normally the the second last block on the card
    u8  unused2[8];
    u32 ifc_list[32];			// List of indirect FAT clusters. On a standard 8M card there's only one indirect FAT cluster
    int bad_block_list[32];		// List of erase blocks that have errors and shouldn't be used
    u8  cardtype;				// Memory card type. Must be 2, indicating that this is a PS2 memory card
    u8  cardflags;				// Physical characteristics of the memory card
    u16 unused3;
    u32 cluster_size;
    u32 FATentries_per_cluster;
    u32 clusters_per_block;
    int cardform;
    u32 rootdir_cluster2;
    u32 unknown1;
    u32 unknown2;
    u32 max_allocatable_clusters;
    u32 unknown3;
    u32 unknown4;
    int unknown5;
} MCDevInfo;

union mcman_pagebuf {
	u32 word[1056/sizeof(u32)];
	u8 byte[1056/sizeof(u8)];
	char magic[1056/sizeof(char)];
};

union mcman_PS1PDApagebuf {
	u32 word[128/sizeof(u32)];
	u16 half[128/sizeof(u16)];
	u8 byte[128/sizeof(u8)];
};

#endif	// __MCMAN_INTERNAL_H__

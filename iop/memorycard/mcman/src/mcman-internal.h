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

#ifndef MCMAN_ENABLE_EXTENDED_DEV_OPS
#if defined(BUILDING_VMCMAN)
#define MCMAN_ENABLE_EXTENDED_DEV_OPS 1
#else
#define MCMAN_ENABLE_EXTENDED_DEV_OPS 0
#endif
#endif

#include <loadcore.h>
#include <intrman.h>
#include <sysclib.h>
#include <thbase.h>
#include <thsemap.h>
#if !defined(BUILDING_XFROMMAN) && !defined(BUILDING_VMCMAN)
#include <timrman.h>
#include <modload.h>
#include <secrman.h>
#endif
#if MCMAN_ENABLE_EXTENDED_DEV_OPS
#include <iomanX.h>
#else
#include <ioman.h>
#endif
#ifdef _IOP
#include <cdvdman.h>
#else
#include <time.h>
#endif
#include <stdio.h>
#include <errno.h>
#ifndef BUILDING_VMCMAN
#ifdef BUILDING_XFROMMAN
#include <fls.h>
#else
#ifdef BUILDING_XMCMAN
#ifndef SIO2MAN_V2
#include <xsio2man.h>
#else
#include <rsio2man.h>
#endif
#else
#include <sio2man.h>
#endif
#endif
#endif

#if !defined(BUILDING_XFROMMAN) && !defined(BUILDING_VMCMAN)
#define MODNAME "mcman_cex"
#elif defined(BUILDING_VMCMAN)
#define MODNAME "vmcman"
#elif defined(BUILDING_XFROMMAN)
#define MODNAME "xfromman"
#endif
#define MODVER  0x20b

#ifndef MCMAN_ENTRYPOINT
#ifdef _IOP
#define MCMAN_ENTRYPOINT _start
#else
#define MCMAN_ENTRYPOINT mcman_start
#endif
#endif

#ifdef SIO_DEBUG
	#include <sior.h>
	#define DEBUG
	#define DPRINTF(format, args...) \
		sio_printf(MODNAME ": " format, ##args)
#else
	#ifdef DEBUG
		#define DPRINTF(format, args...) \
			printf(MODNAME ": " format, ##args)
    #else
		#define DPRINTF(format, args...)
    #endif
#endif

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

#ifdef BUILDING_VMCMAN
#define MCMAN_MAXSLOT				10
#else
#define MCMAN_MAXSLOT				4
#endif
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

#if MCMAN_ENABLE_EXTENDED_DEV_OPS
#define MC_IO_DEV_T iomanX_iop_device_t
#define MC_IO_FIL_T iomanX_iop_file_t
#define MC_IO_DRE_T iox_dirent_t
#define MC_IO_STA_T iox_stat_t
#define MC_IO_OPS_T iomanX_iop_device_ops_t

// FIXME: The r/w/x flag order is opposite from iomanX
#define MC_IO_S_RD (FIO_S_IRUSR | FIO_S_IRGRP | FIO_S_IROTH)
#define MC_IO_S_WR (FIO_S_IWUSR | FIO_S_IWGRP | FIO_S_IWOTH)
#define MC_IO_S_EX (FIO_S_IXUSR | FIO_S_IXGRP | FIO_S_IXOTH)
#define MC_IO_S_FL FIO_S_IFREG
#define MC_IO_S_DR FIO_S_IFDIR

#define MC_IO_CST_ATTR FIO_CST_ATTR
#define MC_IO_CST_MODE FIO_CST_MODE
#define MC_IO_CST_CT FIO_CST_CT
#define MC_IO_CST_MT FIO_CST_MT
#else
#define MC_IO_DEV_T iop_device_t
#define MC_IO_FIL_T iop_file_t
#define MC_IO_DRE_T io_dirent_t
#define MC_IO_STA_T io_stat_t
#define MC_IO_OPS_T iop_device_ops_t

#define MC_IO_S_RD SCE_STM_R
#define MC_IO_S_WR SCE_STM_W
#define MC_IO_S_EX SCE_STM_X
#define MC_IO_S_FL SCE_STM_F
#define MC_IO_S_DR SCE_STM_D

#define MC_IO_CST_ATTR SCE_CST_ATTR
#define MC_IO_CST_MODE SCE_CST_MODE
#define MC_IO_CST_CT SCE_CST_CT
#define MC_IO_CST_MT SCE_CST_MT
#endif

// internal functions prototypes
#if !defined(BUILDING_XFROMMAN) && !defined(BUILDING_VMCMAN)
int  mcsio2_transfer(int port, int slot, sio2_transfer_data_t *sio2data);
#endif
void long_multiply(u32 v1, u32 v2, u32 *HI, u32 *LO);
int  mcman_chrpos(const char *str, int chr);
void mcman_wmemset(void *buf, int size, int value);
int  mcman_calcEDC(void *buf, int size);
int  mcman_checkpath(const char *str);
int  mcman_checkdirpath(const char *str1, const char *str2);
void mcman_invhandles(int port, int slot);
int  McCloseAll(void);
int  mcman_detectcard(int port, int slot);
int  mcman_dread(int fd, MC_IO_DRE_T *dirent);
int  mcman_getstat(int port, int slot, const char *filename, MC_IO_STA_T *stat);
int  mcman_getmcrtime(sceMcStDateTime *tm);
void mcman_initPS2com(void);
#if !defined(BUILDING_XFROMMAN) && !defined(BUILDING_VMCMAN)
void sio2packet_add(int port, int slot, int cmd, u8 *buf);
#endif
int  mcman_eraseblock(int port, int slot, int block, void **pagebuf, void *eccbuf);
int  mcman_readpage(int port, int slot, int page, void *buf, void *eccbuf);
int  mcman_cardchanged(int port, int slot);
int  mcman_resetauth(int port, int slot);
int  mcman_probePS2Card2(int port, int slot);
int  mcman_probePS2Card(int port, int slot);
#if !defined(BUILDING_XFROMMAN) && !defined(BUILDING_VMCMAN)
int  secrman_mc_command(int port, int slot, sio2_transfer_data_t *sio2data);
#endif
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
int  mcman_dread2(int fd, MC_IO_DRE_T *dirent);
int  mcman_getstat2(int port, int slot, const char *filename, MC_IO_STA_T *stat);
int  mcman_setinfo2(int port, int slot, const char *filename, sceMcTblGetDir *info, int flags);
int  mcman_read2(int fd, void *buffer, int nbyte);
int  mcman_write2(int fd, void *buffer, int nbyte);
int  mcman_close2(int fd);
int  mcman_getentspace(int port, int slot, const char *dirname);
int  mcman_cachedirentry(int port, int slot, const char *filename, McCacheDir *pcacheDir, McFsEntry **pfse, int unknown_flag);
int  mcman_getdirinfo(int port, int slot, McFsEntry *pfse, const char *filename, McCacheDir *pcd, int unknown_flag);
int  mcman_open2(int port, int slot, const char *filename, int flags);
int  mcman_chdir(int port, int slot, const char *newdir, char *currentdir);
int  mcman_writecluster(int port, int slot, int cluster, int flag);
int  mcman_getdir2(int port, int slot, const char *dirname, int flags, int maxent, sceMcTblGetDir *info);
int  mcman_delete2(int port, int slot, const char *filename, int flags);
int  mcman_checkBackupBlocks(int port, int slot);
int  mcman_unformat2(int port, int slot);
void mcman_initPS1PDAcom(void);
int  mcman_probePS1Card2(int port, int slot);
int  mcman_probePS1Card(int port, int slot);
int  mcman_probePDACard(int port, int slot);
int  mcman_setPS1devinfos(int port, int slot);
int  mcman_format1(int port, int slot);
int  mcman_open1(int port, int slot, const char *filename, int flags);
int  mcman_read1(int fd, void *buffer, int nbyte);
int  mcman_write1(int fd, void *buffer, int nbyte);
int  mcman_getPS1direntry(int port, int slot, const char *filename, McFsEntryPS1 **pfse, int flag);
int  mcman_dread1(int fd, MC_IO_DRE_T *dirent);
int  mcman_getstat1(int port, int slot, const char *filename, MC_IO_STA_T *stat);
int  mcman_setinfo1(int port, int slot, const char *filename, sceMcTblGetDir *info, int flags);
int  mcman_getdir1(int port, int slot, const char *dirname, int flags, int maxent, sceMcTblGetDir *info);
int  mcman_clearPS1direntry(int port, int slot, int cluster, int flags);
int  mcman_delete1(int port, int slot, const char *filename, int flags);
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
int  mcman_modloadcb(const char *filename, int *port, int *slot); // used as callback by modload
void mcman_unit2card(u32 unit);
int  mcman_initdev(void);

#if defined(BUILDING_VMCMAN)
int mcman_iomanx_backing_mount(int port, int slot, const char *filename);
int mcman_iomanx_backing_umount(int port, int slot);
int mcman_iomanx_backing_getcardspec(int port, int slot, s16 *pagesize, u16 *blocksize, int *cardsize, u8 *flags);
int mcman_iomanx_backing_erase(int port, int slot, int page);
int mcman_iomanx_backing_write(int port, int slot, int page, void *pagebuf, void *eccbuf);
int mcman_iomanx_backing_read(int port, int slot, int page, void *pagebuf, void *eccbuf);
#endif

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

// Defined in main.c

extern char SUPERBLOCK_MAGIC[];
extern char SUPERBLOCK_VERSION[];

extern int mcman_wr_port;
extern int mcman_wr_slot;
extern int mcman_wr_block;
extern int mcman_wr_flag3;
extern int mcman_curdircluster;

extern union mcman_pagebuf mcman_pagebuf;
extern union mcman_PS1PDApagebuf mcman_PS1PDApagebuf;

#ifndef BUILDING_XFROMMAN
extern int timer_ID;
#endif
extern int PS1CardFlag;

extern McFsEntry mcman_dircache[MAX_CACHEDIRENTRY];

extern MC_FHANDLE mcman_fdhandles[MAX_FDHANDLES];
extern MCDevInfo mcman_devinfos[4][MCMAN_MAXSLOT];

extern u8 mcman_eccdata[512]; // size for 32 ecc

// Defined in mcsio2.c
extern u8 mcman_sio2outbufs_PS1PDA[0x90];

#endif	// __MCMAN_INTERNAL_H__

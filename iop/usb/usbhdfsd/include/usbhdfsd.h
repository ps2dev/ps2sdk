/*
# _____     ___ ____     ___ ____
#  ____|   |    ____|   |        | |____|
# |     ___|   |____ ___|    ____| |    \    PS2DEV Open Source Project.
#-----------------------------------------------------------------------
# Licenced under Academic Free License version 2.0
# Review ps2sdk README & LICENSE files for further details.
*/

/**
 * @file
 * IOP USBHDFSD definitions.
 */

#ifndef __USBHDFSD_H__
#define __USBHDFSD_H__

#include <types.h>
#include <irx.h>
#include <usbhdfsd-common.h>

// Structure definitions
typedef struct UsbMassDeviceInfo
{
    /** If the CONNected bit is not set, the contents of the other fields of this structure are undefined. */
    unsigned short int status;
    unsigned short int SectorSize;
    unsigned int MaxLBA;
} UsbMassDeviceInfo_t;

typedef void (*usbmass_cb_t)(int cause);

struct _cache_set;
typedef struct _cache_set cache_set;
struct _mass_dev;
typedef struct _mass_dev mass_dev;

// number of cache slots (1 slot = block)
#define CACHE_SIZE 32

typedef struct _cache_record
{
    unsigned int sector;
    int tax;
    char writeDirty;
} cache_record;

struct _cache_set
{
    mass_dev *dev;
    unsigned int sectorSize;
    unsigned int indexLimit;
    unsigned char *sectorBuf;     // = NULL; //sector content - the cache buffer
    cache_record rec[CACHE_SIZE]; // cache info record

#ifdef SCACHE_RECORD_STATS
    // statistical information
    unsigned int cacheAccess;
    unsigned int cacheHits;
#endif
    unsigned int writeFlag;
};

struct _mass_dev
{
    int controlEp;          // config endpoint id
    int bulkEpI;            // in endpoint id
    int bulkEpO;            // out endpoint id
    int devId;              // device id
    unsigned char configId; // configuration id
    unsigned char status;
    unsigned char interfaceNumber; // interface number
    unsigned char interfaceAlt;    // interface alternate setting
    unsigned int sectorSize;       // = 512; // store size of sector from usb mass
    unsigned int maxLBA;
    int ioSema;
    cache_set *cache;
    usbmass_cb_t callback;
};

typedef struct _fat_bpb
{
    unsigned int sectorSize;     // bytes per sector - should be 512
    unsigned char clusterSize;   // sectors per cluster - power of two
    unsigned int resSectors;     // reserved sectors - typically 1 (boot sector)
    unsigned char fatCount;      // number of FATs - must be 2
    unsigned int rootSize;       // number of rootdirectory entries - typically 512
    unsigned int fatSize;        // sectors per FAT - varies
    unsigned int trackSize;      // sectors per track
    unsigned int headCount;      // number of heads
    unsigned int sectorCount;    // number of sectors
    unsigned int partStart;      // sector where partition starts (boot sector)
    unsigned int rootDirStart;   // sector where root directory starts
    unsigned int rootDirCluster; // fat32 - cluster of the root directory
    unsigned int activeFat;      // fat32 - current active fat number
    unsigned char fatType;       // 12-FAT16, 16-FAT16, 32-FAT32
    unsigned char fatId[9];      // File system ID. "FAT12", "FAT16" or "FAT  " - for debug only
    unsigned int dataStart;      // sector where data starts
} fat_bpb;

typedef struct _fat_driver
{
    mass_dev *dev;
    fat_bpb partBpb; // partition bios parameter block

    // modified by Hermes
#define MAX_DIR_CLUSTER 512
    unsigned int cbuf[MAX_DIR_CLUSTER]; // cluster index buffer // 2048 by Hermes

    unsigned int lastChainCluster;
    int lastChainResult;

/* enough for long filename of length 260 characters (20*13) and one short filename */
#define MAX_DE_STACK 21
    unsigned int deSec[MAX_DE_STACK]; // direntry sector
    int deOfs[MAX_DE_STACK];          // direntry offset
    int deIdx;                        // direntry index

#define SEQ_MASK_SIZE 2048               // Allow 2K files per directory
    u8 seq_mask[SEQ_MASK_SIZE / 8];      // bitmask for consumed seq numbers
#define DIR_MASK_SIZE 2048 * 11          // Allow 2K maxed fullnames per directory
    u8 dir_used_mask[DIR_MASK_SIZE / 8]; // bitmask for used directory entries

#define MAX_CLUSTER_STACK 128
    unsigned int clStack[MAX_CLUSTER_STACK]; // cluster allocation stack
    int clStackIndex;
    unsigned int clStackLast; // last free cluster of the fat table
} fat_driver;

// Exported functions
int UsbMassGetDeviceInfo(int device, UsbMassDeviceInfo_t *info);
int UsbMassRegisterCallback(int device, usbmass_cb_t callback);
fat_driver *UsbMassFatGetData(int device);
int UsbMassReadSector(fat_driver *fatd, void **buffer, u32 sector);
int UsbMassWriteSector(fat_driver *fatd, u32 sector);
void UsbMassFlushCache(fat_driver *fatd);
int UsbMassFatGetClusterChain(fat_driver *fatd, unsigned int cluster, unsigned int *buf, unsigned int bufSize, int startFlag);

#define usbmass_IMPORTS_start DECLARE_IMPORT_TABLE(usbmass, 1, 2)
#define usbmass_IMPORTS_end   END_IMPORT_TABLE

#define I_UsbMassGetDeviceInfo      DECLARE_IMPORT(4, UsbMassGetDeviceInfo)
#define I_UsbMassRegisterCallback   DECLARE_IMPORT(5, UsbMassRegisterCallback)
#define I_UsbMassFatGetData         DECLARE_IMPORT(6, UsbMassFatGetData)
#define I_UsbMassReadSector         DECLARE_IMPORT(7, UsbMassReadSector)
#define I_UsbMassWriteSector        DECLARE_IMPORT(8, UsbMassWriteSector)
#define I_UsbMassFlushCache         DECLARE_IMPORT(9, UsbMassFlushCache)
#define I_UsbMassFatGetClusterChain DECLARE_IMPORT(10, UsbMassFatGetClusterChain)

#endif /* __USBHDFSD_H__ */

//---------------------------------------------------------------------------
// File name:    fat_driver.c
//---------------------------------------------------------------------------
#include <stdio.h>
#include <errno.h>
#include <limits.h>

#ifdef WIN32
#include <stdlib.h>
#include <memory.h>
#include <string.h>
#else
#include <sysclib.h>
// #include <sys/stat.h>

#include <thbase.h>
#include <sysmem.h>
#endif

#ifdef BUILDING_USBHDFSD
#include <usbhdfsd.h>
#endif /* BUILDING_USBHDFSD */
#include "usbhd_common.h"
#include "scache.h"
#include "fat_driver.h"
#include "fat.h"
#ifdef BUILDING_USBHDFSD
#include "mass_stor.h"
#endif /* BUILDING_USBHDFSD */
#ifdef BUILDING_IEEE1394_DISK
#include "sbp2_disk.h"
#include "scsi.h"
#endif /* BUILDING_IEEE1394_DISK */

// #define DEBUG  //comment out this line when not debugging

#include "mass_debug.h"

#define READ_SECTOR(d, a, b)         scache_readSector((d)->cache, (a), (void **)&b)
#if !defined(BUILDING_IEEE1394_DISK) && !defined(BUILDING_USBHDFSD)
#define DEV_ACCESSOR(d) (d)
#else
#define DEV_ACCESSOR(d) ((d)->dev)
#endif
#ifdef BUILDING_USBHDFSD
#define READ_SECTORS_RAW(d, a, c, b) mass_stor_readSector((d), a, b, c);
#define INVALIDATE_SECTORS(d, s, c)  scache_invalidate((d)->cache, s, c)
#endif /* BUILDING_USBHDFSD */

#define NUM_DRIVES 10
static fat_driver *g_fatd[NUM_DRIVES];

//---------------------------------------------------------------------------
int InitFAT(void)
{
    int i;

    M_DEBUG("%s\n", __func__);

    for (i = 0; i < NUM_DRIVES; ++i)
        g_fatd[i] = NULL;

    return 0;
}

//---------------------------------------------------------------------------
int strEqual(const char *s1, const char *s2)
{
    M_DEBUG("%s\n", __func__);

    for (;;) {
        char u1, u2;

        u1 = *s1++;
        u2 = *s2++;
        if (u1 > 64 && u1 < 91)
            u1 += 32;
        if (u2 > 64 && u2 < 91)
            u2 += 32;

        if (u1 != u2) {
            return -1;
        }
        if (u1 == '\0') {
            return 0;
        }
    }
}

/*

   0x321, 0xABC

     byte| byte| byte|
   +--+--+--+--+--+--+
   |2 |1 |C |3 |A |B |
   +--+--+--+--+--+--+

*/

//---------------------------------------------------------------------------
unsigned int fat_getClusterRecord12(const unsigned char *buf, int type)
{
    M_DEBUG("%s\n", __func__);

    if (type) { // 1
        return ((buf[1] << 4) + (buf[0] >> 4));
    } else { // 0
        return (((buf[1] & 0x0F) << 8) + buf[0]);
    }
}

//---------------------------------------------------------------------------
// Get Cluster chain into <buf> buffer
// returns:
// 0    :if buf is full (bufSize entries) and more chain entries exist
// 1-n  :number of filled entries of the buf
// -1   :error
//---------------------------------------------------------------------------
// for fat12
/* fat12 cluster records can overlap the edge of the sector so we need to detect and maintain
   these cases
*/
static int fat_getClusterChain12(fat_driver *fatd, unsigned int cluster, unsigned int *buf, unsigned int bufSize, int startFlag)
{
    int ret;
    unsigned int i, lastFatSector;
    unsigned char xbuf[4], cont;
    unsigned char *sbuf = NULL; // sector buffer

    M_DEBUG("%s\n", __func__);

    cont          = 1;
    lastFatSector = -1;
    i             = 0;
    if (startFlag) {
        buf[i] = cluster; // store first cluster
        i++;
    }
    while (i < bufSize && cont) {
        unsigned int recordOffset, fatSector;
        unsigned char sectorSpan;

        recordOffset = (cluster * 3) / 2; // offset of the cluster record (in bytes) from the FAT start
        fatSector    = recordOffset / fatd->partBpb.sectorSize;
        sectorSpan   = 0;
        if ((recordOffset % fatd->partBpb.sectorSize) == (fatd->partBpb.sectorSize - 1)) {
            sectorSpan = 1;
        }
        if (lastFatSector != fatSector || sectorSpan) {
            ret = READ_SECTOR(DEV_ACCESSOR(fatd), fatd->partBpb.partStart + fatd->partBpb.resSectors + fatSector, sbuf);
            if (ret < 0) {
                XPRINTF("Read fat12 sector failed! sector=%u! \n", fatd->partBpb.partStart + fatd->partBpb.resSectors + fatSector);
                return -EIO;
            }
            lastFatSector = fatSector;

            if (sectorSpan) {
                xbuf[0] = sbuf[fatd->partBpb.sectorSize - 2];
                xbuf[1] = sbuf[fatd->partBpb.sectorSize - 1];
                ret     = READ_SECTOR(DEV_ACCESSOR(fatd), fatd->partBpb.partStart + fatd->partBpb.resSectors + fatSector + 1, sbuf);
                if (ret < 0) {
                    XPRINTF("Read fat12 sector failed sector=%u! \n", fatd->partBpb.partStart + fatd->partBpb.resSectors + fatSector + 1);
                    return -EIO;
                }
                xbuf[2] = sbuf[0];
                xbuf[3] = sbuf[1];
            }
        }
        if (sectorSpan) { // use xbuf as source buffer
            cluster = fat_getClusterRecord12(xbuf + (recordOffset % fatd->partBpb.sectorSize) - (fatd->partBpb.sectorSize - 2), cluster % 2);
        } else { // use sector buffer as source buffer
            cluster = fat_getClusterRecord12(sbuf + (recordOffset % fatd->partBpb.sectorSize), cluster % 2);
        }

        if ((cluster & 0xFFF) >= 0xFF8) {
            cont = 0; // continue = false
        } else {
            buf[i] = cluster & 0xFFF;
            i++;
        }
    }
    return i;
}


//---------------------------------------------------------------------------
// for fat16
static int fat_getClusterChain16(fat_driver *fatd, unsigned int cluster, unsigned int *buf, unsigned int bufSize, int startFlag)
{
    int ret;
    unsigned int i, indexCount, lastFatSector;
    unsigned char cont;
    unsigned char *sbuf = NULL; // sector buffer

    M_DEBUG("%s\n", __func__);

    cont          = 1;
    indexCount    = fatd->partBpb.sectorSize / 2; // FAT16->2, FAT32->4
    lastFatSector = -1;
    i             = 0;
    if (startFlag) {
        buf[i] = cluster; // store first cluster
        i++;
    }
    while (i < bufSize && cont) {
        unsigned int fatSector;

        fatSector = cluster / indexCount;
        if (lastFatSector != fatSector) {
            ret = READ_SECTOR(DEV_ACCESSOR(fatd), fatd->partBpb.partStart + fatd->partBpb.resSectors + fatSector, sbuf);
            if (ret < 0) {
                XPRINTF("Read fat16 sector failed! sector=%u! \n", fatd->partBpb.partStart + fatd->partBpb.resSectors + fatSector);
                return -EIO;
            }

            lastFatSector = fatSector;
        }
        cluster = getUI16(sbuf + ((cluster % indexCount) * 2));
        if ((cluster & 0xFFFF) >= 0xFFF8) {
            cont = 0; // continue = false
        } else {
            buf[i] = cluster & 0xFFFF;
            i++;
        }
    }
    return i;
}

//---------------------------------------------------------------------------
// for fat32
static int fat_getClusterChain32(fat_driver *fatd, unsigned int cluster, unsigned int *buf, unsigned int bufSize, int startFlag)
{
    int ret;
    unsigned int i, indexCount, lastFatSector;
    unsigned char cont;
    unsigned char *sbuf = NULL; // sector buffer

    M_DEBUG("%s\n", __func__);

    cont          = 1;
    indexCount    = fatd->partBpb.sectorSize / 4; // FAT16->2, FAT32->4
    lastFatSector = -1;
    i             = 0;
    if (startFlag) {
        buf[i] = cluster; // store first cluster
        i++;
    }
    while (i < bufSize && cont) {
        unsigned int fatSector;

        fatSector = cluster / indexCount;
        if (lastFatSector != fatSector) {
            ret = READ_SECTOR(DEV_ACCESSOR(fatd), fatd->partBpb.partStart + fatd->partBpb.resSectors + fatSector, sbuf);
            if (ret < 0) {
                XPRINTF("Read fat32 sector failed sector=%u! \n", fatd->partBpb.partStart + fatd->partBpb.resSectors + fatSector);
                return -EIO;
            }

            lastFatSector = fatSector;
        }
        cluster = getUI32(sbuf + ((cluster % indexCount) * 4));
        if ((cluster & 0xFFFFFFF) >= 0xFFFFFF8) {
            cont = 0; // continue = false
        } else {
            buf[i] = cluster & 0xFFFFFFF;
            i++;
        }
    }
    return i;
}

//---------------------------------------------------------------------------
int fat_getClusterChain(fat_driver *fatd, unsigned int cluster, unsigned int *buf, unsigned int bufSize, int startFlag)
{
    M_DEBUG("%s\n", __func__);

    if (cluster == fatd->lastChainCluster) {
        return fatd->lastChainResult;
    }

    switch (fatd->partBpb.fatType) {
        case FAT12:
            fatd->lastChainResult = fat_getClusterChain12(fatd, cluster, buf, bufSize, startFlag);
            break;
        case FAT16:
            fatd->lastChainResult = fat_getClusterChain16(fatd, cluster, buf, bufSize, startFlag);
            break;
        case FAT32:
            fatd->lastChainResult = fat_getClusterChain32(fatd, cluster, buf, bufSize, startFlag);
            break;
    }
    fatd->lastChainCluster = cluster;
    return fatd->lastChainResult;
}

//---------------------------------------------------------------------------
#ifndef BUILDING_IEEE1394_DISK
int fat_CheckChain(fat_driver *fatd, unsigned int cluster)
{
    int i, nextChain = 1;
    int clusterChainStart = 1;

    if (cluster < 2)
        return 0;

    while (nextChain) {
        int chainSize;

        chainSize         = fat_getClusterChain(fatd, cluster, fatd->cbuf, MAX_DIR_CLUSTER, clusterChainStart);
        clusterChainStart = 0;
        if (chainSize >= MAX_DIR_CLUSTER) { // the chain is full, but more chain parts exist
            cluster = fatd->cbuf[MAX_DIR_CLUSTER - 1];
        } else { // chain fits in the chain buffer completely - no next chain needed
            nextChain = 0;
        }

        // process the cluster chain (fatd->cbuf) and skip leading clusters if needed
        for (i = 0; i < (chainSize - 1); i++) {
            if ((fatd->cbuf[i] + 1) != fatd->cbuf[i + 1])
                return 0;
        }
    }

    return 1;
}
#endif /* BUILDING_IEEE1394_DISK */

//---------------------------------------------------------------------------
void fat_invalidateLastChainResult(fat_driver *fatd)
{
    fatd->lastChainCluster = 0;
}

//---------------------------------------------------------------------------
static void fat_determineFatType(fat_bpb *partBpb)
{
    unsigned int sector, clusterCount;

    M_DEBUG("%s\n", __func__);

    // get sector of cluster 0
    sector = fat_cluster2sector(partBpb, 0);
    // remove partition start sector to get BR+FAT+ROOT_DIR sector count
    sector -= partBpb->partStart;
    sector       = partBpb->sectorCount - sector;
    clusterCount = sector / partBpb->clusterSize;
    // XPRINTF("Data cluster count = %u \n", clusterCount);

    if (clusterCount < 4085) {
        partBpb->fatType = FAT12;
    } else if (clusterCount < 65525) {
        partBpb->fatType = FAT16;
    } else {
        partBpb->fatType = FAT32;
    }
}

//---------------------------------------------------------------------------
#if defined(BUILDING_USBHDFSD)
static int fat_getPartitionBootSector(mass_dev *dev, unsigned int sector, fat_bpb *partBpb)
#elif defined(BUILDING_IEEE1394_DISK)
static int fat_getPartitionBootSector(struct SBP2Device *dev, unsigned int sector, fat_bpb *partBpb)
#else
static int fat_getPartitionBootSector(struct block_device *bd, unsigned int sector, fat_bpb *partBpb)
#endif
{
    fat_raw_bpb *bpb_raw;     // fat16, fat12
    fat32_raw_bpb *bpb32_raw; // fat32
    int ret;
    unsigned char *sbuf = NULL; // sector buffer

    M_DEBUG("%s\n", __func__);

#if !defined(BUILDING_USBHDFSD) && !defined(BUILDING_IEEE1394_DISK)
    sbuf = malloc(bd->sectorSize);
    ret = bd->read(bd, sector, sbuf, 1); // read partition boot sector (first sector on partition)
#else
    ret = READ_SECTOR(dev, sector, sbuf); // read partition boot sector (first sector on partition)
#endif

    if (ret < 0) {
        XPRINTF("Read partition boot sector failed sector=%u! \n", sector);
#if !defined(BUILDING_USBHDFSD) && !defined(BUILDING_IEEE1394_DISK)
        free(sbuf);
#endif
        return -EIO;
    }

    bpb_raw   = (fat_raw_bpb *)sbuf;
    bpb32_raw = (fat32_raw_bpb *)sbuf;

#if !defined(BUILDING_USBHDFSD) && !defined(BUILDING_IEEE1394_DISK)
    if ((bpb32_raw->bootSignature[0] != 0x55) || (bpb32_raw->bootSignature[1] != 0xAA)) {
        M_DEBUG("Invalid bootSignature (0x%x - 0x%x)\n", bpb32_raw->bootSignature[0], bpb32_raw->bootSignature[1]);
        free(sbuf);
        return -EIO;
    }
#endif

    // set fat common properties
    partBpb->sectorSize  = getUI16(bpb_raw->sectorSize);
    partBpb->clusterSize = bpb_raw->clusterSize;
    partBpb->resSectors  = getUI16(bpb_raw->resSectors);
    partBpb->fatCount    = bpb_raw->fatCount;
    partBpb->rootSize    = getUI16(bpb_raw->rootSize);
    partBpb->fatSize     = getUI16(bpb_raw->fatSize);
    partBpb->trackSize   = getUI16(bpb_raw->trackSize);
    partBpb->headCount   = getUI16(bpb_raw->headCount);
    partBpb->sectorCount = getUI16(bpb_raw->sectorCountO);
    if (partBpb->sectorCount == 0) {
        partBpb->sectorCount = getUI32(bpb_raw->sectorCount); // large partition
    }
    partBpb->partStart    = sector;
    partBpb->rootDirStart = partBpb->partStart + (partBpb->fatCount * partBpb->fatSize) + partBpb->resSectors;
    for (ret = 0; ret < 8; ret++) {
        partBpb->fatId[ret] = bpb_raw->fatId[ret];
    }
    partBpb->fatId[ret]     = 0;
    partBpb->rootDirCluster = 0;
    partBpb->dataStart      = partBpb->rootDirStart + (partBpb->rootSize / (partBpb->sectorSize >> 5));

    fat_determineFatType(partBpb);

    // fat32 specific info
    if (partBpb->fatType == FAT32 && partBpb->fatSize == 0) {
        partBpb->fatSize   = getUI32(bpb32_raw->fatSize32);
        partBpb->activeFat = getUI16(bpb32_raw->fatStatus);
        if (partBpb->activeFat & 0x80) { // fat not synced
            partBpb->activeFat = (partBpb->activeFat & 0xF);
        } else {
            partBpb->activeFat = 0;
        }
        partBpb->rootDirStart   = partBpb->partStart + (partBpb->fatCount * partBpb->fatSize) + partBpb->resSectors;
        partBpb->rootDirCluster = getUI32(bpb32_raw->rootDirCluster);
        for (ret = 0; ret < 8; ret++) {
            partBpb->fatId[ret] = bpb32_raw->fatId[ret];
        }
        partBpb->fatId[ret] = 0;
        partBpb->dataStart  = partBpb->rootDirStart;
    }

    M_PRINTF("Fat type %u Id %s \n", partBpb->fatType, partBpb->fatId);
#if !defined(BUILDING_USBHDFSD) && !defined(BUILDING_IEEE1394_DISK)
    free(sbuf);
#endif
    return 1;
}

//---------------------------------------------------------------------------
/*
 returns:
 0 - no more dir entries
 1 - short name dir entry found
 2 - long name dir entry found
 3 - deleted dir entry found
*/
int fat_getDirentry(unsigned char fatType, fat_direntry *dir_entry, fat_direntry_summary *dir)
{
    int i;

    u16 character;

    M_DEBUG("%s\n", __func__);

    // detect last entry - all zeros (slight modification by radad)
    if (dir_entry->sfn.name[0] == 0) {
        return 0;
    }
    // detect deleted entry - it will be ignored
    if (dir_entry->sfn.name[0] == 0xE5) {
        return 3;
    }

    // detect long filename
    if (dir_entry->lfn.rshv == 0x0F && dir_entry->lfn.reserved1 == 0x00 && dir_entry->lfn.reserved2[0] == 0x00) {
        unsigned int offset;
        unsigned char cont;

        // long filename - almost whole direntry is unicode string - extract it
        offset = dir_entry->lfn.entrySeq & 0x3f;
        offset--;
        offset = offset * 13;
        // name - 1st part
        cont = 1;
        for (i = 0; i < 10 && cont; i += 2) {
            character = dir_entry->lfn.name1[i] | (dir_entry->lfn.name1[i + 1] << 8);

            if (character == 0 || offset >= (FAT_MAX_NAME - 1)) {
                dir->name[offset] = 0; // terminate
                cont              = 0; // stop
            } else {
                // Handle characters that we don't support.
                dir->name[offset] = character <= UCHAR_MAX ? dir_entry->lfn.name1[i] : '?';
                offset++;
            }
        }
        // name - 2nd part
        for (i = 0; i < 12 && cont; i += 2) {
            character = dir_entry->lfn.name2[i] | (dir_entry->lfn.name2[i + 1] << 8);

            if (character == 0 || offset >= (FAT_MAX_NAME - 1)) {
                dir->name[offset] = 0; // terminate
                cont              = 0; // stop
            } else {
                // Handle characters that we don't support.
                dir->name[offset] = character <= UCHAR_MAX ? dir_entry->lfn.name2[i] : '?';
                offset++;
            }
        }
        // name - 3rd part
        for (i = 0; i < 4 && cont; i += 2) {
            character = dir_entry->lfn.name3[i] | (dir_entry->lfn.name3[i + 1] << 8);

            if (character == 0 || offset >= (FAT_MAX_NAME - 1)) {
                dir->name[offset] = 0; // terminate
                cont              = 0; // stop
            } else {
                // Handle characters that we don't support.
                dir->name[offset] = character <= UCHAR_MAX ? dir_entry->lfn.name3[i] : '?';
                offset++;
            }
        }
        if ((dir_entry->lfn.entrySeq & 0x40)) { // terminate string flag
            dir->name[offset] = 0;
        }
        return 2;
    } else {
        int j;

        // short filename
        // copy name
        for (i = 0; i < 8 && dir_entry->sfn.name[i] != ' '; i++) {
            dir->sname[i] = dir_entry->sfn.name[i];
            // NT-adaption for LaunchELF
            if (dir_entry->sfn.reservedNT & 0x08 &&
                dir->sname[i] >= 'A' && dir->sname[i] <= 'Z') {
                dir->sname[i] += 0x20; // Force standard letters in name to lower case
            }
        }
        for (j = 0; j < 3 && dir_entry->sfn.ext[j] != ' '; j++) {
            if (j == 0) {
                dir->sname[i] = '.';
                i++;
            }
            dir->sname[i + j] = dir_entry->sfn.ext[j];
            // NT-adaption for LaunchELF
            if (dir_entry->sfn.reservedNT & 0x10 &&
                dir->sname[i + j] >= 'A' && dir->sname[i + j] <= 'Z') {
                dir->sname[i + j] += 0x20; // Force standard letters in ext to lower case
            }
        }
        dir->sname[i + j] = 0;   // terminate
        if (dir->name[0] == 0) { // long name desn't exit
            for (i = 0; dir->sname[i] != 0; i++)
                dir->name[i] = dir->sname[i];
            dir->name[i] = 0;
        }
        dir->attr    = dir_entry->sfn.attr;
        dir->size    = getUI32(dir_entry->sfn.size);
        dir->cluster = (fatType == FAT32) ? getUI32_2(dir_entry->sfn.clusterL, dir_entry->sfn.clusterH) : getUI16(dir_entry->sfn.clusterL);

        return 1;
    }
}

//---------------------------------------------------------------------------
// Set chain info (cluster/offset) cache
void fat_setFatDirChain(fat_driver *fatd, fat_dir *fatDir)
{
    int i, j;
    unsigned int index, clusterChainStart, fileCluster, fileSize, blockSize;
    unsigned char nextChain;
    int chainSize;

    M_DEBUG("%s\n", __func__);
    XPRINTF("reading cluster chain  \n");
    fileCluster = fatDir->chain[0].cluster;

    if (fileCluster < 2) {
        XPRINTF("   early exit... \n");
        return;
    }

    fileSize  = fatDir->size;
    blockSize = fileSize / DIR_CHAIN_SIZE;

    nextChain         = 1;
    clusterChainStart = 0;
    j                 = 1;
    fileSize          = 0;
    index             = 0;

    while (nextChain) {
        if ((chainSize = fat_getClusterChain(fatd, fileCluster, fatd->cbuf, MAX_DIR_CLUSTER, 1)) >= 0) {
            if (chainSize >= MAX_DIR_CLUSTER) { // the chain is full, but more chain parts exist
                fileCluster = fatd->cbuf[MAX_DIR_CLUSTER - 1];
            } else { // chain fits in the chain buffer completely - no next chain exist
                nextChain = 0;
            }
        } else {
            XPRINTF("fat_setFatDirChain(): fat_getClusterChain() failed: %d\n", chainSize);
            return;
        }

        // process the cluster chain (fatd->cbuf)
        for (i = clusterChainStart; i < chainSize; i++) {
            fileSize += (fatd->partBpb.clusterSize * fatd->partBpb.sectorSize);
            while (fileSize >= (j * blockSize) && j < DIR_CHAIN_SIZE) {
                fatDir->chain[j].cluster = fatd->cbuf[i];
                fatDir->chain[j].index   = index;
                j++;
            } // ends "while"
            index++;
        } // ends "for"
        clusterChainStart = 1;
    } // ends "while"
    fatDir->lastCluster = fatd->cbuf[i - 1];

#ifdef DEBUG_EXTREME // dlanor: I patched this because this bloat hid important stuff
    // debug
    XPRINTF("SEEK CLUSTER CHAIN CACHE fileSize=%u blockSize=%u \n", fatDir->size, blockSize);
    for (i = 0; i < DIR_CHAIN_SIZE; i++) {
        XPRINTF("index=%u cluster=%u offset= %u - %u start=%u \n",
                fatDir->chain[i].index, fatDir->chain[i].cluster,
                fatDir->chain[i].index * fatd->partBpb.clusterSize * fatd->partBpb.sectorSize,
                (fatDir->chain[i].index + 1) * fatd->partBpb.clusterSize * fatd->partBpb.sectorSize,
                i * blockSize);
    }
#endif /* debug */
    XPRINTF("read cluster chain  done!\n");
}

//---------------------------------------------------------------------------
/* Set base attributes of direntry */
static void fat_setFatDir(fat_driver *fatd, fat_dir *fatDir, unsigned int parentDirCluster, fat_direntry_sfn *dsfn, fat_direntry_summary *dir, int getClusterInfo)
{
    unsigned int i;
    char *srcName;

    M_DEBUG("%s\n", __func__);
    XPRINTF("setting fat dir...\n");
    srcName = dir->sname;
    if (dir->name[0] != 0) { // long filename not empty
        srcName = dir->name;
    }
    // copy name
    for (i = 0; srcName[i] != 0; i++)
        fatDir->name[i] = srcName[i];
    fatDir->name[i] = 0; // terminate

    fatDir->attr = dir->attr;
    fatDir->size = dir->size;

    // created Date: Day, Month, Year-low, Year-high
    fatDir->cdate[0] = (dsfn->dateCreate[0] & 0x1F);
    fatDir->cdate[1] = (dsfn->dateCreate[0] >> 5) + ((dsfn->dateCreate[1] & 0x01) << 3);
    i                = 1980 + (dsfn->dateCreate[1] >> 1);
    fatDir->cdate[2] = (i & 0xFF);
    fatDir->cdate[3] = ((i & 0xFF00) >> 8);

    // created Time: Hours, Minutes, Seconds
    fatDir->ctime[0] = ((dsfn->timeCreate[1] & 0xF8) >> 3);
    fatDir->ctime[1] = ((dsfn->timeCreate[1] & 0x07) << 3) + ((dsfn->timeCreate[0] & 0xE0) >> 5);
    fatDir->ctime[2] = ((dsfn->timeCreate[0] & 0x1F) << 1);

    // accessed Date: Day, Month, Year-low, Year-high
    fatDir->adate[0] = (dsfn->dateAccess[0] & 0x1F);
    fatDir->adate[1] = (dsfn->dateAccess[0] >> 5) + ((dsfn->dateAccess[1] & 0x01) << 3);
    i                = 1980 + (dsfn->dateAccess[1] >> 1);
    fatDir->adate[2] = (i & 0xFF);
    fatDir->adate[3] = ((i & 0xFF00) >> 8);

    // modified Date: Day, Month, Year-low, Year-high
    fatDir->mdate[0] = (dsfn->dateWrite[0] & 0x1F);
    fatDir->mdate[1] = (dsfn->dateWrite[0] >> 5) + ((dsfn->dateWrite[1] & 0x01) << 3);
    i                = 1980 + (dsfn->dateWrite[1] >> 1);
    fatDir->mdate[2] = (i & 0xFF);
    fatDir->mdate[3] = ((i & 0xFF00) >> 8);

    // modified Time: Hours, Minutes, Seconds
    fatDir->mtime[0] = ((dsfn->timeWrite[1] & 0xF8) >> 3);
    fatDir->mtime[1] = ((dsfn->timeWrite[1] & 0x07) << 3) + ((dsfn->timeWrite[0] & 0xE0) >> 5);
    fatDir->mtime[2] = ((dsfn->timeWrite[0] & 0x1F) << 1);

    fatDir->chain[0].cluster = dir->cluster;
    fatDir->chain[0].index   = 0;
    if (getClusterInfo) {
        fat_setFatDirChain(fatd, fatDir);
    }

    fatDir->parentDirCluster = parentDirCluster;
    fatDir->startCluster     = dir->cluster;
}

//---------------------------------------------------------------------------
int fat_getDirentrySectorData(fat_driver *fatd, unsigned int *startCluster, unsigned int *startSector, unsigned int *dirSector)
{
    unsigned int chainSize;

    M_DEBUG("%s\n", __func__);

    if (*startCluster == 0 && fatd->partBpb.fatType < FAT32) { // Root directory
        *startSector = fatd->partBpb.rootDirStart;
        *dirSector   = fatd->partBpb.rootSize / (fatd->partBpb.sectorSize / 32);
        return 0;
    }
    // other directory or fat 32
    if (*startCluster == 0 && fatd->partBpb.fatType == FAT32) {
        *startCluster = fatd->partBpb.rootDirCluster;
    }
    *startSector = fat_cluster2sector(&fatd->partBpb, *startCluster);
    chainSize    = fat_getClusterChain(fatd, *startCluster, fatd->cbuf, MAX_DIR_CLUSTER, 1);
    if (chainSize >= MAX_DIR_CLUSTER) {
        XPRINTF("Chain too large\n");
        return -EFAULT;
    } else if (chainSize > 0) {
        *dirSector = chainSize * fatd->partBpb.clusterSize;
    } else {
        XPRINTF("Error getting cluster chain! startCluster=%u \n", *startCluster);
        return -EFAULT;
    }

    return chainSize;
}

//---------------------------------------------------------------------------
static int fat_getDirentryStartCluster(fat_driver *fatd, char *dirName, unsigned int *startCluster, fat_dir *fatDir)
{
#if !defined(BUILDING_IEEE1394_DISK) && !defined(BUILDING_USBHDFSD)
    static fat_direntry_summary dir; // TOO BIG FOR STACK!
#else
    fat_direntry_summary dir;
#endif
    unsigned int i, dirSector, startSector;
    unsigned char cont;
    int ret;
#ifdef DEBUG
#ifdef BUILDING_USBHDFSD
    mass_dev *mass_device = fatd->dev;
#endif /* BUILDING_USBHDFSD */
#ifdef BUILDING_IEEE1394_DISK
    struct SBP2Device *mass_device = fatd->dev;
#endif /* BUILDING_IEEE1394_DISK */
#endif

    M_DEBUG("%s(%s)\n", __func__, dirName);

    cont = 1;
    XPRINTF("getting cluster for dir entry: %s \n", dirName);
    // clear name strings
    dir.sname[0] = 0;
    dir.name[0]  = 0;

    ret = fat_getDirentrySectorData(fatd, startCluster, &startSector, &dirSector);
    if (ret < 0)
        return ret;

    XPRINTF("dirCluster=%u startSector=%u (%u) dirSector=%u \n", *startCluster, startSector, startSector
#if !defined(BUILDING_IEEE1394_DISK) && !defined(BUILDING_USBHDFSD)
        * fatd->bd->sectorSize,
#else
        * mass_device->sectorSize,
#endif
        dirSector);

    // go through first directory sector till the max number of directory sectors
    // or stop when no more direntries detected
    for (i = 0; i < dirSector && cont; i++) {
        unsigned int dirPos;
        unsigned char *sbuf = NULL; // sector buffer

        // At cluster borders, get correct sector from cluster chain buffer
        if ((*startCluster != 0) && (i % fatd->partBpb.clusterSize == 0)) {
            startSector = fat_cluster2sector(&fatd->partBpb, fatd->cbuf[(i / fatd->partBpb.clusterSize)]) - i;
        }

        ret = READ_SECTOR(DEV_ACCESSOR(fatd), startSector + i, sbuf);
        if (ret < 0) {
            XPRINTF("read directory sector failed ! sector=%u\n", startSector + i);
            return -EIO;
        }
        XPRINTF("read sector ok, scanning sector for direntries...\n");
        dirPos = 0;

        // go through start of the sector till the end of sector
        while (cont && dirPos < fatd->partBpb.sectorSize) {
            fat_direntry *dir_entry = (fat_direntry *)(sbuf + dirPos);
            cont                    = fat_getDirentry(fatd->partBpb.fatType, dir_entry, &dir); // get single directory entry from sector buffer
            if (cont == 1) {                                                                   // when short file name entry detected
                if (!(dir.attr & FAT_ATTR_VOLUME_LABEL)) {                                     // not volume label
                    if ((strEqual(dir.sname, dirName) == 0) ||
                        (strEqual(dir.name, dirName) == 0)) {
                        XPRINTF("found! %s\n", dir.name);
                        if (fatDir != NULL) { // fill the directory properties
                            fat_setFatDir(fatd, fatDir, *startCluster, &dir_entry->sfn, &dir, 1);
                        }
                        *startCluster = dir.cluster;
                        XPRINTF("direntry %s found at cluster: %u \n", dirName, dir.cluster);
                        return dir.attr; // returns file or directory attr
                    }
                } // ends "if(!(dir.attr & FAT_ATTR_VOLUME_LABEL))"
                // clear name strings
                dir.sname[0] = 0;
                dir.name[0]  = 0;
            } // ends "if (cont == 1)"
            dirPos += sizeof(fat_direntry);
        } // ends "while"
    }     // ends "for"
    XPRINTF("direntry %s not found! \n", dirName);
    return -ENOENT;
}

//---------------------------------------------------------------------------
// start cluster should be 0 - if we want to search from root directory
// otherwise the start cluster should be correct cluster of directory
// to search directory - set fatDir as NULL
int fat_getFileStartCluster(fat_driver *fatd, const char *fname, unsigned int *startCluster, fat_dir *fatDir)
{
#if !defined(BUILDING_IEEE1394_DISK) && !defined(BUILDING_USBHDFSD)
    static char tmpName[FAT_MAX_NAME + 1]; // TOO BIG FOR STACK!
#else
    char tmpName[FAT_MAX_NAME + 1];
#endif
    unsigned int i, offset;
    int ret;

    M_DEBUG("%s\n", __func__);
    XPRINTF("Entering fat_getFileStartCluster\n");

    offset = 0;
    i      = 0;

    *startCluster = 0;
    if (fatDir != NULL) {
        memset(fatDir, 0, sizeof(fat_dir));
        fatDir->attr = FAT_ATTR_DIRECTORY;
    }
    if (fname[i] == '/') {
        i++;
    }

    for (; fname[i] != 0; i++) {
        if (fname[i] == '/') {   // directory separator
            tmpName[offset] = 0; // terminate string
            ret             = fat_getDirentryStartCluster(fatd, tmpName, startCluster, fatDir);
            if (ret < 0) {
                return -ENOENT;
            }
            offset = 0;
        } else {
            tmpName[offset] = fname[i];
            offset++;
        }
    } // ends "for"
    // and the final file
    tmpName[offset] = 0; // terminate string
    XPRINTF("Ready to get cluster for file \"%s\"\n", tmpName);
    if (fatDir != NULL) {
        // if the last char of the name was slash - the name was already found -exit
        if (offset == 0) {
            XPRINTF("Exiting from fat_getFileStartCluster with a folder\n");
            return 2;
        }
        ret = fat_getDirentryStartCluster(fatd, tmpName, startCluster, fatDir);
        if (ret < 0) {
            XPRINTF("Exiting from fat_getFileStartCluster with error %i\n", ret);
            return ret;
        }
        XPRINTF("file's startCluster found. Name=%s, cluster=%u \n", fname, *startCluster);
    }
    XPRINTF("Exiting from fat_getFileStartCluster with no error.\n");
    return 1;
}

//---------------------------------------------------------------------------
void fat_getClusterAtFilePos(fat_driver *fatd, fat_dir *fatDir, unsigned int filePos, unsigned int *cluster, unsigned int *clusterPos)
{
    unsigned int i, j, blockSize;

    M_DEBUG("%s\n", __func__);

    blockSize = fatd->partBpb.clusterSize * fatd->partBpb.sectorSize;

    for (i = 0, j = (DIR_CHAIN_SIZE - 1); i < (DIR_CHAIN_SIZE - 1); i++) {
        if (fatDir->chain[i].index * blockSize <= filePos &&
            fatDir->chain[i + 1].index * blockSize > filePos) {
            j = i;
            break;
        }
    }
    *cluster    = fatDir->chain[j].cluster;
    *clusterPos = (fatDir->chain[j].index * blockSize);
}

//---------------------------------------------------------------------------
#ifdef BUILDING_USBHDFSD
static int fat_readSingleSector(mass_dev *dev, unsigned int sector, void *buffer, int size, int dataSkip)
{
    unsigned char *sbuf = NULL; // sector buffer
    int ret;

    ret = READ_SECTOR(dev, sector, sbuf);
    if (ret < 0) {
        XPRINTF("Read sector failed ! sector=%u\n", sector);
        return 0;
    }

    memcpy(buffer, sbuf + dataSkip, size);

    return 1;
}
#endif /* BUILDING_USBHDFSD */

int fat_readFile(fat_driver *fatd, fat_dir *fatDir, unsigned int filePos, unsigned char *buffer, unsigned int size)
{
    int ret;
#ifdef BUILDING_USBHDFSD
    unsigned int toRead;
#endif
    unsigned int i, j, startSector, clusterChainStart, bufSize, sectorSkip, clusterSkip, dataSkip;
    unsigned char nextChain;
#ifdef BUILDING_USBHDFSD
    // cppcheck-suppress unreadVariable
    mass_dev *mass_device = fatd->dev;
#endif /* BUILDING_USBHDFSD */
#ifdef BUILDING_IEEE1394_DISK
    struct SBP2Device *mass_device = fatd->dev;
#endif /* BUILDING_IEEE1394_DISK */

    unsigned int bufferPos, fileCluster, clusterPos;

    M_DEBUG("%s\n", __func__);

    fat_getClusterAtFilePos(fatd, fatDir, filePos, &fileCluster, &clusterPos);
    sectorSkip  = (filePos - clusterPos) / fatd->partBpb.sectorSize;
    clusterSkip = sectorSkip / fatd->partBpb.clusterSize;
    sectorSkip %= fatd->partBpb.clusterSize;
    dataSkip  = filePos % fatd->partBpb.sectorSize;
    bufferPos = 0;

    XPRINTF("fileCluster = %u,  clusterPos= %u clusterSkip=%u, sectorSkip=%u dataSkip=%u \n",
            fileCluster, clusterPos, clusterSkip, sectorSkip, dataSkip);

    if (fileCluster < 2) {
        return 0;
    }

#ifdef BUILDING_IEEE1394_DISK
    // cppcheck-suppress unreadVariable
    bufSize           = mass_device->sectorSize;
#endif /* BUILDING_IEEE1394_DISK */
#if !defined(BUILDING_IEEE1394_DISK) && !defined(BUILDING_USBHDFSD)
    bufSize = fatd->bd->sectorSize;
#endif
    nextChain         = 1;
    clusterChainStart = 1;

    while (nextChain && size > 0) {
        int chainSize;

        if ((chainSize = fat_getClusterChain(fatd, fileCluster, fatd->cbuf, MAX_DIR_CLUSTER, clusterChainStart)) < 0) {
            return chainSize;
        }

        clusterChainStart = 0;
        if (chainSize >= MAX_DIR_CLUSTER) { // the chain is full, but more chain parts exist
            fileCluster = fatd->cbuf[MAX_DIR_CLUSTER - 1];
        } else { // chain fits in the chain buffer completely - no next chain needed
            nextChain = 0;
        }
        while (clusterSkip >= MAX_DIR_CLUSTER) {
            chainSize         = fat_getClusterChain(fatd, fileCluster, fatd->cbuf, MAX_DIR_CLUSTER, clusterChainStart);
            clusterChainStart = 0;
            if (chainSize >= MAX_DIR_CLUSTER) { // the chain is full, but more chain parts exist
                fileCluster = fatd->cbuf[MAX_DIR_CLUSTER - 1];
            } else { // chain fits in the chain buffer completely - no next chain needed
                nextChain = 0;
            }
            clusterSkip -= MAX_DIR_CLUSTER;
        }

        // process the cluster chain (fatd->cbuf) and skip leading clusters if needed
        for (i = 0 + clusterSkip; i < (unsigned int)chainSize && size > 0; i++) {
            // read cluster and save cluster content
            startSector = fat_cluster2sector(&fatd->partBpb, fatd->cbuf[i]);

#ifdef BUILDING_USBHDFSD
            // Calculate how long we can continuously read for.
            j      = (size + dataSkip) / fatd->partBpb.sectorSize + sectorSkip;
            toRead = 0;
            while (1) {
                if (j >= fatd->partBpb.clusterSize) {
                    toRead += fatd->partBpb.clusterSize;
                    j -= fatd->partBpb.clusterSize;
                } else {
                    toRead += j;
                    j = 0;
                }

                // Check that the next cluster is adjacent to this one, so we can read across.
                if ((i >= (unsigned int)(chainSize - 1)) || (fatd->cbuf[i] != (fatd->cbuf[i + 1] - 1)))
                    break;
                if (j == 0)
                    break;
                i++; // Advance to the next cluster.
            }

            // Consider the number of sectors within the cluster to skip.
            startSector += sectorSkip;
            toRead -= sectorSkip;
#endif /* BUILDING_USBHDFSD */

            // process all sectors of the cluster (and skip leading sectors if needed)
#ifndef BUILDING_USBHDFSD
            for (j = 0 + sectorSkip; j < fatd->partBpb.clusterSize && size > 0; j++) {
                unsigned char *sbuf = NULL; // sector buffer

                ret = READ_SECTOR(DEV_ACCESSOR(fatd), startSector + j, sbuf);
                if (ret < 0) {
                    XPRINTF("Read sector failed ! sector=%u\n", startSector + j);
                    return bufferPos;
                }

                // compute exact size of transfered bytes
                if (size < bufSize) {
                    bufSize = size + dataSkip;
                }
#ifdef BUILDING_IEEE1394_DISK
                if (bufSize > mass_device->sectorSize) {
                    bufSize = mass_device->sectorSize;
                }
#else
                if (bufSize > fatd->bd->sectorSize) {
                    bufSize = fatd->bd->sectorSize;
                }
#endif
                XPRINTF("memcopy dst=%u, src=%u, size=%u  bufSize=%u \n", bufferPos, dataSkip, bufSize - dataSkip, bufSize);
                memcpy(buffer + bufferPos, sbuf + dataSkip, bufSize - dataSkip);
                size -= (bufSize - dataSkip);
                bufferPos += (bufSize - dataSkip);
                dataSkip = 0;
#ifdef BUILDING_IEEE1394_DISK
                bufSize = mass_device->sectorSize;
#else
                bufSize = fatd->bd->sectorSize;
#endif
            }
#endif /* BUILDING_USBHDFSD */

#ifdef BUILDING_USBHDFSD
            if (dataSkip > 0) {
                bufSize = mass_device->sectorSize - dataSkip;
                if (size < bufSize)
                    bufSize = size;

                ret = fat_readSingleSector(fatd->dev, startSector, buffer + bufferPos, bufSize, dataSkip);
                if (ret != 1) {
                    return bufferPos;
                }

                if (size + dataSkip >= mass_device->sectorSize)
                    toRead--;

                size -= bufSize;
                bufferPos += bufSize;
                dataSkip = 0;
                startSector++;
            }

            if (toRead > 0) {
                INVALIDATE_SECTORS(DEV_ACCESSOR(fatd), startSector, toRead);
                ret = READ_SECTORS_RAW(DEV_ACCESSOR(fatd), startSector, toRead, buffer + bufferPos);
                if (ret != 0) {
                    XPRINTF("Read sectors failed ! sector=%u (%u)\n", startSector, toRead);
                    return bufferPos;
                }

                bufSize = toRead * mass_device->sectorSize;
                size -= bufSize;
                bufferPos += bufSize;
                startSector += toRead;
            }

            if (size > 0 && size <= mass_device->sectorSize) {
                bufSize = size;

                ret = fat_readSingleSector(fatd->dev, startSector, buffer + bufferPos, bufSize, 0);
                if (ret != 1) {
                    return bufferPos;
                }

                size -= bufSize;
                bufferPos += bufSize;
            }
#endif /* BUILDING_USBHDFSD */

            sectorSkip = 0;
        }
        clusterSkip = 0;
    }
    return bufferPos;
}

//---------------------------------------------------------------------------
int fat_getNextDirentry(fat_driver *fatd, fat_dir_list *fatdlist, fat_dir *fatDir)
{
#if !defined(BUILDING_IEEE1394_DISK) && !defined(BUILDING_USBHDFSD)
    static fat_direntry_summary dir; // TOO BIG FOR STACK!
#else
    fat_direntry_summary dir;
#endif
    int i, ret;
    unsigned int startSector, dirSector, dirPos, dirCluster;
    unsigned char cont, new_entry;
#ifdef DEBUG
#ifdef BUILDING_USBHDFSD
    mass_dev *mass_device = fatd->dev;
#endif /* BUILDING_USBHDFSD */
#ifdef BUILDING_IEEE1394_DISK
    struct SBP2Device *mass_device = fatd->dev;
#endif /* BUILDING_IEEE1394_DISK */
#endif

    M_DEBUG("%s\n", __func__);

    // the getFirst function was not called
    if (fatdlist->direntryCluster == 0xFFFFFFFF || fatDir == NULL) {
        return -EFAULT;
    }

    dirCluster = fatdlist->direntryCluster;

    // clear name strings
    dir.sname[0] = 0;
    dir.name[0]  = 0;

    ret = fat_getDirentrySectorData(fatd, &dirCluster, &startSector, &dirSector);
    if (ret < 0)
        return ret;

    XPRINTF("dirCluster=%u startSector=%u (%u) dirSector=%u \n", dirCluster, startSector, startSector
#if !defined(BUILDING_IEEE1394_DISK) && !defined(BUILDING_USBHDFSD)
        * fatd->bd->sectorSize,
#else
        * mass_device->sectorSize,
#endif
        dirSector);

    // go through first directory sector till the max number of directory sectors
    // or stop when no more direntries detected
    // dlanor: but avoid rescanning same areas redundantly (if possible)
    cont      = 1;
    new_entry = 1;
    dirPos    = (fatdlist->direntryIndex * 32) % fatd->partBpb.sectorSize;
    for (i = ((fatdlist->direntryIndex * 32) / fatd->partBpb.sectorSize); ((unsigned int)i < dirSector) && cont; i++) {
        unsigned char *sbuf = NULL; // sector buffer

        // At cluster borders, get correct sector from cluster chain buffer
        if ((dirCluster != 0) && (new_entry || (i % fatd->partBpb.clusterSize == 0))) {
            startSector = fat_cluster2sector(&fatd->partBpb, fatd->cbuf[(i / fatd->partBpb.clusterSize)]) - i + (i % fatd->partBpb.clusterSize);
            new_entry   = 0;
        }
        ret = READ_SECTOR(DEV_ACCESSOR(fatd), startSector + i, sbuf);
        if (ret < 0) {
            XPRINTF("Read directory  sector failed ! sector=%u\n", startSector + i);
            return -EIO;
        }

        // go through sector from current pos till its end
        while (cont && (dirPos < fatd->partBpb.sectorSize)) {
            fat_direntry *dir_entry = (fat_direntry *)(sbuf + dirPos);
            cont                    = fat_getDirentry(fatd->partBpb.fatType, dir_entry, &dir); // get a directory entry from sector
            fatdlist->direntryIndex++;                                                         // Note current entry processed
            if (cont == 1) {                                                                   // when short file name entry detected
                fat_setFatDir(fatd, fatDir, dirCluster, &dir_entry->sfn, &dir, 0);
#if 0
                XPRINTF("fat_getNextDirentry %c%c%c%c%c%c %x %s %s\n",
                        (dir.attr & FAT_ATTR_VOLUME_LABEL) ? 'v' : '-',
                        (dir.attr & FAT_ATTR_DIRECTORY) ? 'd' : '-',
                        (dir.attr & FAT_ATTR_READONLY) ? 'r' : '-',
                        (dir.attr & FAT_ATTR_ARCHIVE) ? 'a' : '-',
                        (dir.attr & FAT_ATTR_SYSTEM) ? 's' : '-',
                        (dir.attr & FAT_ATTR_HIDDEN) ? 'h' : '-',
                        dir.attr,
                        dir.sname,
                        dir.name);
#endif
                return 1;
            }
            dirPos += sizeof(fat_direntry);
        } // ends "while"
        dirPos = 0;
    } // ends "for"
    // when we get this far - reset the direntry cluster
    fatdlist->direntryCluster = 0xFFFFFFFF; // no more files
    return 0;                               // indicate that no direntry is avalable
}

//---------------------------------------------------------------------------
int fat_getFirstDirentry(fat_driver *fatd, const char *dirName, fat_dir_list *fatdlist, fat_dir *fatDir_host, fat_dir *fatDir)
{
    int ret;
    unsigned int startCluster = 0;

    M_DEBUG("%s\n", __func__);

    ret = fat_getFileStartCluster(fatd, dirName, &startCluster, fatDir_host);
    if (ret < 0) { // dir name not found
        return -ENOENT;
    }
    // check that direntry is directory
    if (!(fatDir_host->attr & FAT_ATTR_DIRECTORY)) {
        return -ENOTDIR; // it's a file - exit
    }
    fatdlist->direntryCluster = startCluster;
    fatdlist->direntryIndex   = 0;
    return fat_getNextDirentry(fatd, fatdlist, fatDir);
}

//---------------------------------------------------------------------------
#if defined(BUILDING_USBHDFSD)
int fat_mount(mass_dev *dev, unsigned int start, unsigned int count)
#elif defined(BUILDING_IEEE1394_DISK)
int fat_mount(struct SBP2Device *dev, unsigned int start, unsigned int count)
#else
int fat_mount(struct block_device *bd)
#endif
{
    fat_driver *fatd = NULL;
    unsigned int i;

    M_DEBUG("%s\n", __func__);

#if defined(BUILDING_USBHDFSD) || defined(BUILDING_IEEE1394_DISK)
    (void)count;
#endif

    // Filter for supported partition IDs:
    // - 0x0b = FAT32 with CHS addressing
    // - 0x0c = FAT32 with LBA addressing
    // AKuHAK: dont filter partition, fat_getPartitionBootSector() will care about
    // that, this is filtering too much if (bd->parId != 0x0b && bd->parId !=
    // 0x0c)
    //     return -1;

    for (i = 0; i < NUM_DRIVES && fatd == NULL; ++i) {
        if (g_fatd[i] == NULL) {
            XPRINTF("usb fat: allocate fat_driver %d!\n", sizeof(fat_driver));
            g_fatd[i] = malloc(sizeof(fat_driver));
            if (g_fatd[i] != NULL) {
#if !defined(BUILDING_IEEE1394_DISK) && !defined(BUILDING_USBHDFSD)
                g_fatd[i]->bd = NULL;
                g_fatd[i]->cache = NULL;
#else
                g_fatd[i]->dev = NULL;
#endif
            }
            fatd = g_fatd[i];
        } else if (
#if !defined(BUILDING_IEEE1394_DISK) && !defined(BUILDING_USBHDFSD)
            g_fatd[i]->bd
#else
            g_fatd[i]->dev
#endif
            == NULL) {
            fatd = g_fatd[i];
        }
    }

    if (fatd == NULL) {
        M_PRINTF("unable to allocate drive!\n");
        return -1;
    }

#if !defined(BUILDING_IEEE1394_DISK) && !defined(BUILDING_USBHDFSD)
    if (fatd->bd != NULL) {
        M_PRINTF("mount ERROR: alread mounted\n");
        fat_forceUnmount(fatd->bd);
    }
#else
    if (fatd->dev != NULL) {
        M_PRINTF("mount ERROR: alread mounted\n");
        fat_forceUnmount(fatd->dev);
    }
#endif

#if !defined(BUILDING_IEEE1394_DISK) && !defined(BUILDING_USBHDFSD)
    if (fatd->cache != NULL) {
        M_PRINTF("ERROR: cache already created\n");
        scache_kill(fatd->cache);
        fatd->cache = NULL;
    }
#endif

#if !defined(BUILDING_IEEE1394_DISK) && !defined(BUILDING_USBHDFSD)
    if (fat_getPartitionBootSector(bd, bd->sectorOffset, &fatd->partBpb) < 0)
        return -1;
#else
    if (fat_getPartitionBootSector(dev, start, &fatd->partBpb) < 0)
        return -1;
#endif

#if !defined(BUILDING_IEEE1394_DISK) && !defined(BUILDING_USBHDFSD)
    fatd->cache = scache_init(bd);
    if (fatd->cache == NULL) {
        M_PRINTF("Error - scache_init failed\n");
        return -1;
    }
#endif

#if !defined(BUILDING_IEEE1394_DISK) && !defined(BUILDING_USBHDFSD)
    fatd->bd               = bd;
#else
    fatd->dev              = dev;
#endif
    fatd->deIdx            = 0;
    fatd->clStackIndex     = 0;
    fatd->clStackLast      = 0;
    fatd->lastChainCluster = 0xFFFFFFFF;
    fatd->lastChainResult  = -1;
    return 0;
}

//---------------------------------------------------------------------------
#if defined(BUILDING_USBHDFSD)
void fat_forceUnmount(mass_dev *dev)
#elif defined(BUILDING_IEEE1394_DISK)
void fat_forceUnmount(struct SBP2Device *dev)
#else
void fat_forceUnmount(struct block_device *bd)
#endif
{
    unsigned int i;

    M_DEBUG("%s\n", __func__);

#ifdef BUILDING_USBHDFSD
    XPRINTF("usb fat: forceUnmount devId %i \n", dev->devId);
#endif /* BUILDING_USBHDFSD */
#ifdef BUILDING_IEEE1394_DISK
    XPRINTF("usb fat: forceUnmount devId %i \n", dev->nodeID);
#endif /* BUILDING_IEEE1394_DISK */

    for (i = 0; i < NUM_DRIVES; ++i) {
#if !defined(BUILDING_IEEE1394_DISK) && !defined(BUILDING_USBHDFSD)
        if (g_fatd[i] != NULL && g_fatd[i]->bd == bd)
#else
        if (g_fatd[i] != NULL && g_fatd[i]->dev == dev)
#endif
        {
#if !defined(BUILDING_IEEE1394_DISK) && !defined(BUILDING_USBHDFSD)
            scache_kill(g_fatd[i]->cache);
            free(g_fatd[i]);
#endif
            g_fatd[i] = NULL;
        }
    }
}

//---------------------------------------------------------------------------
fat_driver *fat_getData(int device)
{
    M_DEBUG("%s(%d)\n", __func__, device);

    if (device >= NUM_DRIVES)
        return NULL;

#ifdef BUILDING_USBHDFSD
    while (g_fatd[device] == NULL || g_fatd[device]->dev == NULL) {
        if (mass_stor_configureNextDevice() <= 0)
            break;
        // Do not block USBD by busy-looping here, if the device is not ready. This function call will carry the priority of the calling thread.
        DelayThread(5000);
    }
#endif /* BUILDING_USBHDFSD */

#if !defined(BUILDING_IEEE1394_DISK) && !defined(BUILDING_USBHDFSD)
    if (g_fatd[device] == NULL || g_fatd[device]->bd == NULL)
#else
    if (g_fatd[device] == NULL || g_fatd[device]->dev == NULL)
#endif
        return NULL;
    return g_fatd[device];
}

//---------------------------------------------------------------------------
#if !defined(BUILDING_IEEE1394_DISK) && !defined(BUILDING_USBHDFSD)
int fat_stopUnit(int device)
{
    fat_driver *fatd;

    fatd = fat_getData(device);
    return (fatd != NULL) ? fatd->bd->stop(fatd->bd) : -ENODEV;
}

void fat_stopAll(void)
{
    int i;

    for (i = 0; i < NUM_DRIVES; i++) {
        fat_driver *fatd;

        fatd = fat_getData(i);
        if (fatd != NULL)
            fatd->bd->stop(fatd->bd);
    }
}
#endif

//---------------------------------------------------------------------------
// End of file:  fat_driver.c
//---------------------------------------------------------------------------

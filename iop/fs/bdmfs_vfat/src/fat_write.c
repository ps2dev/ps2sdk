/*
 * fat_driver.c - USB Mass storage driver for PS2
 *
 * (C) 2005, Marek Olejnik (ole00@post.cz)
 *
 * FAT filesystem layer -  write functions
 *
 * See the file LICENSE included with this distribution for licensing terms.
 */
//---------------------------------------------------------------------------
#include "common.h"
#include "fat.h"
#include "fat_driver.h"
#include "scache.h"
#include <cdvdman.h>
#include <errno.h>
#include <stdio.h>
#include <sysclib.h>

//#define DEBUG  //comment out this line when not debugging
#include "module_debug.h"

#define DATE_CREATE 1
#define DATE_MODIFY 2

#define READ_SECTOR(d, a, b) scache_readSector((d)->cache, (a), (void**)&b)
#define ALLOC_SECTOR(d, a, b) scache_allocSector((d)->cache, (a), (void**)&b)
#define WRITE_SECTOR(d, a) scache_writeSector((d)->cache, (a))
#define FLUSH_SECTORS(d) scache_flushSectors((d)->cache)

//---------------------------------------------------------------------------
/*
 reorder (swap) the cluster stack records
*/
static void swapClStack(fat_driver* fatd, int startIndex, int endIndex)
{
    int i;
    int size;
    int offset1, offset2;
    unsigned int tmp;

    size = endIndex - startIndex;
    if (size < 2) {
        return;
    }

    size /= 2;
    for (i = 0; i < size; i++) {
        offset1                = startIndex + i;
        offset2                = endIndex - 1 - i;
        tmp                    = fatd->clStack[offset1];
        fatd->clStack[offset1] = fatd->clStack[offset2];
        fatd->clStack[offset2] = tmp;
    }
}

//---------------------------------------------------------------------------
/*
 scan FAT12 for free clusters and store them to the cluster stack
*/

static int fat_readEmptyClusters12(fat_driver* fatd)
{
    int ret;
    int recordOffset;
    int sectorSpan;
    int fatSector;
    int lastFatSector;
    unsigned int cluster;
    unsigned int clusterValue;
    unsigned char xbuf[4];
    int oldClStackIndex;
    unsigned char* sbuf = NULL; //sector buffer

    oldClStackIndex = fatd->clStackIndex;

    lastFatSector = -1;
    cluster       = fatd->clStackLast;

    while (fatd->clStackIndex < MAX_CLUSTER_STACK) {
        recordOffset = (cluster * 3) / 2; //offset of the cluster record (in bytes) from the FAT start
        fatSector    = recordOffset / fatd->partBpb.sectorSize;
        sectorSpan   = 0;
        if ((recordOffset % fatd->partBpb.sectorSize) == (fatd->partBpb.sectorSize - 1)) {
            sectorSpan = 1;
        }
        if (lastFatSector != fatSector || sectorSpan) {
            ret = READ_SECTOR(fatd, fatd->partBpb.partStart + fatd->partBpb.resSectors + fatSector, sbuf);
            if (ret < 0) {
                M_DEBUG("Read fat12 sector failed! sector=%u! \n", fatd->partBpb.partStart + fatd->partBpb.resSectors + fatSector);
                return -EIO;
            }
            lastFatSector = fatSector;

            if (sectorSpan) {
                xbuf[0] = sbuf[fatd->partBpb.sectorSize - 2];
                xbuf[1] = sbuf[fatd->partBpb.sectorSize - 1];
                ret     = READ_SECTOR(fatd, fatd->partBpb.partStart + fatd->partBpb.resSectors + fatSector + 1, sbuf);
                if (ret < 0) {
                    M_DEBUG("Read fat12 sector failed sector=%u! \n", fatd->partBpb.partStart + fatd->partBpb.resSectors + fatSector + 1);
                    return -EIO;
                }
                xbuf[2] = sbuf[0];
                xbuf[3] = sbuf[1];
            }
        }
        if (sectorSpan) { // use xbuf as source buffer
            clusterValue = fat_getClusterRecord12(xbuf + (recordOffset % fatd->partBpb.sectorSize) - (fatd->partBpb.sectorSize - 2), cluster % 2);
        } else { // use sector buffer as source buffer
            clusterValue = fat_getClusterRecord12(sbuf + (recordOffset % fatd->partBpb.sectorSize), cluster % 2);
        }
        if (clusterValue == 0) {
            fatd->clStackLast                 = cluster;
            fatd->clStack[fatd->clStackIndex] = cluster;
            fatd->clStackIndex++;
        }
        cluster++; //read next cluster record in the sequence
    }
    //the stack operates as LIFO but we put in the clusters as FIFO
    //we should reverse the cluster order - not necessary
    //but it will retain the natural (increasing) order of
    //the cluster chain
    swapClStack(fatd, oldClStackIndex, fatd->clStackIndex);
    return fatd->clStackIndex;
}

//---------------------------------------------------------------------------
/*
 scan FAT32 for free clusters and store them to the cluster stack
*/
static int fat_readEmptyClusters32(fat_driver* fatd)
{
    unsigned int i, j;
    int ret;
    unsigned int indexCount;
    unsigned int fatStartSector;
    unsigned int cluster;
    unsigned int clusterValue;
    int oldClStackIndex;
    int sectorSkip;
    int recordSkip;

    oldClStackIndex = fatd->clStackIndex;

    //indexCount = numer of cluster indices per sector
    indexCount = fatd->partBpb.sectorSize / 4; //FAT16->2, FAT32->4
    //skip areas we have already searched through
    sectorSkip = fatd->clStackLast / indexCount;
    recordSkip = fatd->clStackLast % indexCount;

    fatStartSector = fatd->partBpb.partStart + fatd->partBpb.resSectors;

    for (i = sectorSkip; i < fatd->partBpb.fatSize && fatd->clStackIndex < MAX_CLUSTER_STACK; i++) {
        unsigned char* sbuf = NULL; //sector buffer

        ret = READ_SECTOR(fatd, fatStartSector + i, sbuf);
        if (ret < 0) {
            M_DEBUG("Read fat32 sector failed! sector=%u! \n", fatStartSector + i);
            return -EIO;
        }
        for (j = recordSkip; j < indexCount && fatd->clStackIndex < MAX_CLUSTER_STACK; j++) {
            cluster = getI32(sbuf + (j * 4));
            if (cluster == 0) { //the cluster is free
                clusterValue = (i * indexCount) + j;
                if (clusterValue < 0xFFFFFF7) {
                    fatd->clStackLast                 = clusterValue;
                    fatd->clStack[fatd->clStackIndex] = clusterValue;
                    fatd->clStackIndex++;
                }
            }
        }
        recordSkip = 0;
    }
    //the stack operates as LIFO but we put in the clusters as FIFO
    //we should reverse the cluster order - not necessary
    //but it will retain the natural (increasing) order of
    //the cluster chain
    swapClStack(fatd, oldClStackIndex, fatd->clStackIndex);
    return fatd->clStackIndex;
}

//---------------------------------------------------------------------------
/*
 scan FAT16 for free clusters and store them to the cluster stack
*/
static int fat_readEmptyClusters16(fat_driver* fatd)
{
    unsigned int i, j;
    int ret;
    unsigned int indexCount;
    unsigned int fatStartSector;
    unsigned int cluster;
    int oldClStackIndex;
    int sectorSkip;
    int recordSkip;

    oldClStackIndex = fatd->clStackIndex;
    //M_DEBUG("#### Read empty clusters16: clStackIndex=%d MAX=%d\n",  clStackIndex, MAX_CLUSTER_STACK);

    //indexCount = numer of cluster indices per sector
    indexCount = fatd->partBpb.sectorSize / 2; //FAT16->2, FAT32->4

    //skip areas we have already searched through
    sectorSkip = fatd->clStackLast / indexCount;
    recordSkip = fatd->clStackLast % indexCount;

    fatStartSector = fatd->partBpb.partStart + fatd->partBpb.resSectors;

    for (i = sectorSkip; i < fatd->partBpb.fatSize && fatd->clStackIndex < MAX_CLUSTER_STACK; i++) {
        unsigned char* sbuf = NULL; //sector buffer

        ret = READ_SECTOR(fatd, fatStartSector + i, sbuf);
        if (ret < 0) {
            M_DEBUG("Read fat16 sector failed! sector=%u! \n", fatStartSector + i);
            return -EIO;
        }
        for (j = recordSkip; j < indexCount && fatd->clStackIndex < MAX_CLUSTER_STACK; j++) {
            cluster = getI16(sbuf + (j * 2));
            if (cluster == 0) { //the cluster is free
                fatd->clStackLast                 = (i * indexCount) + j;
                fatd->clStack[fatd->clStackIndex] = fatd->clStackLast;
                M_DEBUG("%u ", fatd->clStack[fatd->clStackIndex]);
                fatd->clStackIndex++;
            }
        }
        recordSkip = 0;
    }
    M_DEBUG("\n");
    //the stack operates as LIFO but we put in the clusters as FIFO
    //we should reverse the cluster order - not necessary
    //but it will retain the natural (increasing) order of
    //the cluster chain
    swapClStack(fatd, oldClStackIndex, fatd->clStackIndex);
    return fatd->clStackIndex;
}

//---------------------------------------------------------------------------
/*
 scan FAT for free clusters and store them to the cluster stack
*/
static int fat_readEmptyClusters(fat_driver* fatd)
{
    switch (fatd->partBpb.fatType) {
    case FAT12:
        return fat_readEmptyClusters12(fatd);
    case FAT16:
        return fat_readEmptyClusters16(fatd);
    case FAT32:
        return fat_readEmptyClusters32(fatd);
    }

    return (-1);
}

//---------------------------------------------------------------------------
/*
   set sinlge cluster record (FAT12)into buffer

   0x321, 0xABC

    byte0|byte1|byte2|
   +--+--+--+--+--+--+
   |2 |1 |C |3 |A |B |
   +--+--+--+--+--+--+

*/
static void fat_setClusterRecord12(unsigned char* buf, unsigned int cluster, int type)
{

    if (type) { //type 1
        buf[0] = (buf[0] & 0x0F) + ((cluster & 0x0F) << 4);
        buf[1] = (cluster & 0xFF0) >> 4;
    } else { // type 0
        buf[0] = (cluster & 0xFF);
        buf[1] = (buf[1] & 0xF0) + ((cluster & 0xF00) >> 8);
    }
}

//---------------------------------------------------------------------------
static void fat_setClusterRecord12part1(unsigned char* buf, unsigned int cluster, int type)
{
    if (type) { //type 1
        buf[0] = (buf[0] & 0x0F) + ((cluster & 0x0F) << 4);
    } else { // type 0
        buf[0] = (cluster & 0xFF);
    }
}

//---------------------------------------------------------------------------
static void fat_setClusterRecord12part2(unsigned char* buf, unsigned int cluster, int type)
{
    if (type) { //type 1
        buf[0] = (cluster & 0xFF0) >> 4;
    } else { // type 0
        buf[0] = (buf[0] & 0xF0) + ((cluster & 0xF00) >> 8);
    }
}

//---------------------------------------------------------------------------
/*
   save value at the cluster record in FAT 12
*/
static int fat_saveClusterRecord12(fat_driver* fatd, unsigned int currentCluster, unsigned int value)
{
    int ret;
    int sectorSpan;
    int recordOffset;
    int recordType;
    int fatNumber;
    unsigned int fatSector;

    ret = -1;
    //recordOffset is byte offset of the record from the start of the fat table
    recordOffset = (currentCluster * 3) / 2;

    //save both fat tables
    for (fatNumber = 0; fatNumber < fatd->partBpb.fatCount; fatNumber++) {
        unsigned char* sbuf = NULL; //sector buffer

        fatSector = fatd->partBpb.partStart + fatd->partBpb.resSectors + (fatNumber * fatd->partBpb.fatSize);
        fatSector += recordOffset / fatd->partBpb.sectorSize;
        sectorSpan = fatd->partBpb.sectorSize - (recordOffset % fatd->partBpb.sectorSize);
        if (sectorSpan > 1) {
            sectorSpan = 0;
        }
        recordType = currentCluster % 2;

        ret = READ_SECTOR(fatd, fatSector, sbuf);
        if (ret < 0) {
            M_DEBUG("Read fat16 sector failed! sector=%u! \n", fatSector);
            return -EIO;
        }
        if (!sectorSpan) { // not sector span - the record is copmact and fits in single sector
            fat_setClusterRecord12(sbuf + (recordOffset % fatd->partBpb.sectorSize), value, recordType);
            ret = WRITE_SECTOR(fatd, fatSector);
            if (ret < 0) {
                M_DEBUG("Write fat12 sector failed! sector=%u! \n", fatSector);
                return -EIO;
            }
        } else { // sector span - the record is broken in 2 pieces - each one on different sector
            //modify one last byte of the sector buffer
            fat_setClusterRecord12part1(sbuf + (recordOffset % fatd->partBpb.sectorSize), value, recordType);
            //save current sector
            ret = WRITE_SECTOR(fatd, fatSector);
            if (ret < 0) {
                M_DEBUG("Write fat12 sector failed! sector=%u! \n", fatSector);
                return -EIO;
            }
            //read next sector from the fat
            fatSector++;
            ret = READ_SECTOR(fatd, fatSector, sbuf);
            if (ret < 0) {
                M_DEBUG("Read fat16 sector failed! sector=%u! \n", fatSector);
                return -EIO;
            }
            //modify first byte of the sector buffer
            fat_setClusterRecord12part2(sbuf, value, recordType);
            //save current sector
            ret = WRITE_SECTOR(fatd, fatSector);
            if (ret < 0) {
                M_DEBUG("Write fat12 sector failed! sector=%u! \n", fatSector);
                return -EIO;
            }
        }
    } //end for
    return ret;
}

//---------------------------------------------------------------------------
/*
   save value at the cluster record in FAT 32
*/
static int fat_saveClusterRecord16(fat_driver* fatd, unsigned int currentCluster, unsigned int value)
{
    int i;
    int ret;
    int indexCount;
    int fatNumber;
    unsigned int fatSector;

    ret = -1;
    //indexCount is numer of cluster indices per sector
    indexCount = fatd->partBpb.sectorSize / 2; //FAT16->2, FAT32->4

    //save both fat tables
    for (fatNumber = 0; fatNumber < fatd->partBpb.fatCount; fatNumber++) {
        unsigned char* sbuf = NULL; //sector buffer

        fatSector = fatd->partBpb.partStart + fatd->partBpb.resSectors + (fatNumber * fatd->partBpb.fatSize);
        fatSector += currentCluster / indexCount;

        ret = READ_SECTOR(fatd, fatSector, sbuf);
        if (ret < 0) {
            M_DEBUG("Read fat16 sector failed! sector=%u! \n", fatSector);
            return -EIO;
        }
        i = currentCluster % indexCount;
        i *= 2; //fat16
        sbuf[i++] = value & 0xFF;
        sbuf[i]   = ((value & 0xFF00) >> 8);
        ret       = WRITE_SECTOR(fatd, fatSector);
        if (ret < 0) {
            M_DEBUG("Write fat16 sector failed! sector=%u! \n", fatSector);
            return -EIO;
        }
    }
    return ret;
}

//---------------------------------------------------------------------------
/*
   save value at the cluster record in FAT 16
*/
static int fat_saveClusterRecord32(fat_driver* fatd, unsigned int currentCluster, unsigned int value)
{
    int i;
    int ret;
    int indexCount;
    int fatNumber;
    unsigned int fatSector;

    ret = -1;
    //indexCount is numer of cluster indices per sector
    indexCount = fatd->partBpb.sectorSize / 4; //FAT16->2, FAT32->4

    //save both fat tables
    for (fatNumber = 0; fatNumber < fatd->partBpb.fatCount; fatNumber++) {
        unsigned char* sbuf = NULL; //sector buffer

        fatSector = fatd->partBpb.partStart + fatd->partBpb.resSectors + (fatNumber * fatd->partBpb.fatSize);
        fatSector += currentCluster / indexCount;

        ret = READ_SECTOR(fatd, fatSector, sbuf);
        if (ret < 0) {
            M_DEBUG("Read fat32 sector failed! sector=%u! \n", fatSector);
            return -EIO;
        }
        i = currentCluster % indexCount;
        i *= 4; //fat32
        sbuf[i++] = value & 0xFF;
        sbuf[i++] = ((value & 0xFF00) >> 8);
        sbuf[i++] = ((value & 0xFF0000) >> 16);
        sbuf[i]   = (sbuf[i] & 0xF0) + ((value >> 24) & 0x0F); //preserve the highest nibble intact

        ret = WRITE_SECTOR(fatd, fatSector);
        if (ret < 0) {
            M_DEBUG("Write fat32 sector failed! sector=%u! \n", fatSector);
            return -EIO;
        }
    }
    return ret;
}

//---------------------------------------------------------------------------
/*
  Append (and write) cluster chain to the FAT table.

  currentCluster - current end cluster record.
  endCluster     - new end cluster record.

  Note: there is no checking wether the currentCluster holds the EOF marker!

  Example
  current FAT:

  index   09    10    11    12
        +-----+-----+-----+-----+
  value + 10  | EOF | 0   | 0   |
        +-----+-----+-----+-----+


  currentcluster = 10, endcluster = 12
  updated FAT (after the function ends):

  index   09    10    11    12
        +-----+-----+-----+-----+
  value + 10  | 12  | 0   | EOF |
        +-----+-----+-----+-----+

*/
//---------------------------------------------------------------------------
static int fat_appendClusterChain(fat_driver* fatd, unsigned int currentCluster, unsigned int endCluster)
{
    int ret;
    ret = -1;
    switch (fatd->partBpb.fatType) {
    case FAT12:
        ret = fat_saveClusterRecord12(fatd, currentCluster, endCluster);
        if (ret < 0)
            return ret;
        ret = fat_saveClusterRecord12(fatd, endCluster, 0xFFF);
        break;

    case FAT16:
        ret = fat_saveClusterRecord16(fatd, currentCluster, endCluster);
        if (ret < 0)
            return ret;
        ret = fat_saveClusterRecord16(fatd, endCluster, 0xFFFF);
        break;

    case FAT32:
        ret = fat_saveClusterRecord32(fatd, currentCluster, endCluster);
        if (ret < 0)
            return ret;
        ret = fat_saveClusterRecord32(fatd, endCluster, 0xFFFFFFF);
        break;
    }
    return ret;
}

//---------------------------------------------------------------------------
/*
 create new cluster chain (of size 1 cluster) at the cluster index
*/
static int fat_createClusterChain(fat_driver* fatd, unsigned int cluster)
{
    switch (fatd->partBpb.fatType) {
    case FAT12:
        return fat_saveClusterRecord12(fatd, cluster, 0xFFF);
    case FAT16:
        return fat_saveClusterRecord16(fatd, cluster, 0xFFFF);
    case FAT32:
        return fat_saveClusterRecord32(fatd, cluster, 0xFFFFFFF);
    }
    return -EFAULT;
}

//---------------------------------------------------------------------------
/*
 modify the cluster (in FAT table) at the cluster index
*/
static int fat_modifyClusterChain(fat_driver* fatd, unsigned int cluster, unsigned int value)
{
    switch (fatd->partBpb.fatType) {
    case FAT12:
        return fat_saveClusterRecord12(fatd, cluster, value);
    case FAT16:
        return fat_saveClusterRecord16(fatd, cluster, value);
    case FAT32:
        return fat_saveClusterRecord32(fatd, cluster, value);
    }
    return -EFAULT;
}

//---------------------------------------------------------------------------
/*
  delete cluster chain starting at cluster
*/
static int fat_deleteClusterChain(fat_driver* fatd, unsigned int cluster)
{
    int ret;
    int size;
    int cont;
    int end;
    int i;

    if (cluster < 2) {
        return -EFAULT;
    }
    M_DEBUG("I: delete cluster chain starting at cluster=%u\n", cluster);

    cont = 1;

    while (cont) {
        size = fat_getClusterChain(fatd, cluster, fatd->cbuf, MAX_DIR_CLUSTER, 1);

        end = size - 1; //do not delete last cluster in the chain buffer

        for (i = 0; i < end; i++) {
            ret = fat_modifyClusterChain(fatd, fatd->cbuf[i], 0);
            if (ret < 0) {
                return ret;
            }
        }
        //the cluster chain continues
        if (size == MAX_DIR_CLUSTER) {
            cluster = fatd->cbuf[end];
        } else {
            //no more cluster entries - delete the last cluster entry
            ret = fat_modifyClusterChain(fatd, fatd->cbuf[end], 0);
            if (ret < 0) {
                return ret;
            }
            cont = 0;
        }
        fat_invalidateLastChainResult(fatd); //prevent to misuse current (now deleted) fatd->cbuf
    }
    return 1;
}

//---------------------------------------------------------------------------
/*
  Get single empty cluster from the clusterStack (cS is small cache of free clusters)
  Passed currentCluster is updated in the FAT and the new returned cluster index is
  appended at the end of fat chain!
*/
static unsigned int fat_getFreeCluster(fat_driver* fatd, unsigned int currentCluster)
{

    int ret;
    unsigned int result;

    //cluster stack is empty - find and fill the cS
    if (fatd->clStackIndex <= 0) {
        fatd->clStackIndex = 0;
        ret                = fat_readEmptyClusters(fatd);
        if (ret <= 0)
            return 0;
        fatd->clStackIndex = ret;
    }
    //pop from cluster stack
    fatd->clStackIndex--;
    result = fatd->clStack[fatd->clStackIndex];
    //append the cluster chain
    if (currentCluster) {
        ret = fat_appendClusterChain(fatd, currentCluster, result);
    } else { //create new cluster chain
        ret = fat_createClusterChain(fatd, result);
    }
    if (ret < 0)
        return 0;
    return result;
}

//---------------------------------------------------------------------------
/*
returns number of direntry positions that the name takes
//dlanor: Note that this only includes the long_name entries
*/
static int getDirentrySize(const char* lname)
{
    int len;
    int result;
    len    = strlen(lname);
    result = len / 13;
    if (len % 13 > 0)
        result++;
    return result;
}

//---------------------------------------------------------------------------
/*
compute checksum of the short filename
*/
static unsigned char computeNameChecksum(const char* sname)
{
    unsigned char result;
    int i;

    result = 0;
    for (i = 0; i < 11; i++) {
        result = (0x80 * (0x01 & result)) + (result >> 1); //ROR 1
        result += sname[i];
    }
    return result;
}

//---------------------------------------------------------------------------
/*
  fill the LFN (long filename) direntry
*/
static void setLfnEntry(const char* lname, int nameSize, unsigned char chsum, fat_direntry_lfn* dlfn, int part, int maxPart)
{
    int i, j;
    unsigned char name[26]; //unicode name buffer = 13 characters per 2 bytes
    int nameStart;

    nameStart = 13 * (part - 1);
    j         = nameSize - nameStart;
    if (j > 13) {
        j = 13;
    }

    //fake unicode conversion
    for (i = 0; i < j; i++) {
        name[i * 2]     = (unsigned char)lname[nameStart + i];
        name[i * 2 + 1] = 0;
    }

    //rest of the name is zero terminated and padded with 0xFF
    for (i = j; i < 13; i++) {
        if (i == j) {
            name[i * 2]     = 0;
            name[i * 2 + 1] = 0;
        } else {
            name[i * 2]     = 0xFF;
            name[i * 2 + 1] = 0xFF;
        }
    }

    dlfn->entrySeq = part;
    if (maxPart == part)
        dlfn->entrySeq |= 0x40;
    dlfn->checksum = chsum;
    //1st part of the name
    for (i = 0; i < 10; i++)
        dlfn->name1[i] = name[i];
    //2nd part of the name
    for (i = 0; i < 12; i++)
        dlfn->name2[i] = name[i + 10];
    //3rd part of the name
    for (i = 0; i < 4; i++)
        dlfn->name3[i] = name[i + 22];
    dlfn->rshv         = 0x0f;
    dlfn->reserved1    = 0;
    dlfn->reserved2[0] = 0;
    dlfn->reserved2[1] = 0;
}

//---------------------------------------------------------------------------
/*
  update the SFN (long filename) direntry - DATE and TIME
*/
static void setSfnDate(fat_direntry_sfn* dsfn, int mode)
{
    int year, month, day, hour, minute, sec;
    unsigned char tmpClk[4];

#ifdef WIN32
    year   = 0;
    month  = 0;
    day    = 0;
    hour   = 0;
    minute = 0;
    sec    = 0;
#else
    //ps2 specific routine to get time and date
    sceCdCLOCK cdtime;
    s32 tmp;

    if (sceCdReadClock(&cdtime) != 0 && cdtime.stat == 0) {

        tmp = cdtime.second >> 4;
        sec = (u32)(((tmp << 2) + tmp) << 1) + (cdtime.second & 0x0F);

        tmp    = cdtime.minute >> 4;
        minute = (((tmp << 2) + tmp) << 1) + (cdtime.minute & 0x0F);

        tmp  = cdtime.hour >> 4;
        hour = (((tmp << 2) + tmp) << 1) + (cdtime.hour & 0x0F);

        tmp = cdtime.day >> 4;
        day = (((tmp << 2) + tmp) << 1) + (cdtime.day & 0x0F);

        tmp   = (cdtime.month & 0x7F) >> 4;
        month = (((tmp << 2) + tmp) << 1) + (cdtime.month & 0x0F);

        tmp  = cdtime.year >> 4;
        year = (((tmp << 2) + tmp) << 1) + (cdtime.year & 0xF) + 2000;
    } else {
        year   = 2005;
        month  = 1;
        day    = 6;
        hour   = 14;
        minute = 12;
        sec    = 10;
    }
#endif

    if (dsfn == NULL || mode == 0) {
        return;
    }

    tmpClk[0] = (sec / 2) & 0x1F;      //seconds
    tmpClk[0] += (minute & 0x07) << 5; // minute
    tmpClk[1] = (minute & 0x38) >> 3;  // minute
    tmpClk[1] += (hour & 0x1F) << 3;   // hour

    tmpClk[2] = (day & 0x1F);                 //day
    tmpClk[2] += (month & 0x07) << 5;         // month
    tmpClk[3] = (month & 0x08) >> 3;          // month
    tmpClk[3] += ((year - 1980) & 0x7F) << 1; //year

    M_DEBUG("year=%d, month=%d, day=%d   h=%d m=%d s=%d \n", year, month, day, hour, minute, sec);
    //set date & time of creation
    if (mode & DATE_CREATE) {
        dsfn->timeCreate[0] = tmpClk[0];
        dsfn->timeCreate[1] = tmpClk[1];
        dsfn->dateCreate[0] = tmpClk[2];
        dsfn->dateCreate[1] = tmpClk[3];
        dsfn->dateAccess[0] = tmpClk[2];
        dsfn->dateAccess[1] = tmpClk[3];
    }
    //set  date & time of modification
    if (mode & DATE_MODIFY) {
        dsfn->timeWrite[0] = tmpClk[0];
        dsfn->timeWrite[1] = tmpClk[1];
        dsfn->dateWrite[0] = tmpClk[2];
        dsfn->dateWrite[1] = tmpClk[3];
    }
}

//---------------------------------------------------------------------------
/*
  fill the SFN (short filename) direntry
*/
static void setSfnEntry(const char* shortName, char directory, fat_direntry_sfn* dsfn, unsigned int cluster)
{
    int i;

    //name + ext
    for (i = 0; i < 8; i++)
        dsfn->name[i] = shortName[i];
    for (i = 0; i < 3; i++)
        dsfn->ext[i] = shortName[i + 8];

    if (directory > 0) {
        dsfn->attr = FAT_ATTR_DIRECTORY;
    } else {
        dsfn->attr = FAT_ATTR_ARCHIVE;
    }
    dsfn->reservedNT  = 0;
    dsfn->clusterH[0] = (cluster & 0xFF0000) >> 16;
    dsfn->clusterH[1] = (cluster & 0xFF000000) >> 24;
    dsfn->clusterL[0] = (cluster & 0x00FF);
    dsfn->clusterL[1] = (cluster & 0xFF00) >> 8;

    //size is zero - because we don't know the filesize yet
    for (i = 0; i < 4; i++)
        dsfn->size[i] = 0;

    setSfnDate(dsfn, DATE_CREATE | DATE_MODIFY);
}

static void setSfnEntryFromOld(const char* shortName, fat_direntry_sfn* dsfn, const fat_direntry_sfn* orig_dsfn)
{
    int i;

    memcpy(dsfn, orig_dsfn, sizeof(fat_direntry_sfn));

    //name + ext
    for (i = 0; i < 8; i++)
        dsfn->name[i] = shortName[i];
    for (i = 0; i < 3; i++)
        dsfn->ext[i] = shortName[i + 8];
}

//---------------------------------------------------------------------------
/*
 Create short name by squeezing long name into the 8.3 name boundaries
 lname - existing long name
 sname - buffer where to store short name

 returns: 0  if longname completely fits into the 8.3 boundaries
          1  if long name have to be truncated (ie. INFORM~1.TXT)
	 <0  if invalid long name detected
*/
static int createShortNameMask(char* lname, char* sname)
{
    int i;
    int size;
    int j;
    int fit;

    if ((lname[0] == '.')
        && ((lname[1] == 0)
               || ((lname[1] == '.')
                      && (lname[2] == 0)))) {
        return -EINVAL;
    }

    fit = 0;
    //clean short name by putting space
    for (i = 0; i < 11; i++)
        sname[i] = ' ';
    M_DEBUG("Clear short name ='%s'\n", sname);

    //detect number of dots and space characters in the long name
    j = 0;
    for (i = 0; lname[i] != 0; i++) {
        if (lname[i] == '.')
            j++;
        else if (lname[i] == ' ')
            j += 2;
    }
    //long name contains no dot or one dot and no space char
    if (j <= 1)
        fit++;
    //M_DEBUG("fit1=%d j=%d\n", fit, j);

    //store name
    for (i = 0; lname[i] != 0 && lname[i] != '.' && i < 8; i++) {
        sname[i] = toupper(lname[i]);
        //short name must not contain spaces - replace space by underscore
        if (sname[i] == ' ')
            sname[i] = '_';
    }
    //check wether last char is '.' and the name is shorter than 8
    if (lname[i] == '.' || lname[i] == 0) {
        fit++;
    }
    //M_DEBUG("fit2=%d\n", fit);

    //find the last dot "." - filename extension
    size = strlen(lname);
    size--;

    for (i = size; i > 0 && lname[i] != '.'; i--)
        ;
    if (lname[i] == '.') {
        i++;
        for (j = 0; lname[i] != 0 && j < 3; i++, j++) {
            sname[j + 8] = toupper(lname[i]);
        }
        //no more than 3 characters of the extension
        if (lname[i] == 0)
            fit++;
    } else {
        //no dot detected in the long filename
        fit++;
    }
    //	M_DEBUG("fit3=%d\n", fit);
    //	M_DEBUG("Long name=%s  Short name=%s \n", lname, sname);

    //all 3 checks passed  - the long name fits in the short name without restrictions
    if (fit == 3) {
        M_DEBUG("Short name is loseles!\n");
        return 0;
    }

    //one of the check failed - the short name have to be 'sequenced'
    //do not allow spaces in the short name
    for (i = 0; i < 8; i++) {
        if (sname[i] == ' ')
            sname[i] = '_';
    }
    return 1;
}

//---------------------------------------------------------------------------
/*
  separate path and filename
  fname - the source (merged) string (input)
  path  - separated path (output)
  name  - separated filename (output)
*/
static int separatePathAndName(const char* fname, char* path, char* name)
{
    int path_len;
    const char *sp, *np;

    if (!(sp = strrchr(fname, '/')))               //if last path separator missing ?
        np = fname;                                //  name starts at start of fname string
    else                                           //else last path separator found
        np = sp + 1;                               //  name starts after separator
    if (strlen(np) >= FAT_MAX_NAME)                //if name is too long
        return -ENAMETOOLONG;                      //  return error code
    strcpy(name, np);                              //copy name from correct part of fname string
    if ((path_len = (np - fname)) >= FAT_MAX_PATH) //if path is too long
        return -ENAMETOOLONG;                      //  return error code
    strncpy(path, fname, path_len);                //copy path from start of fname string
    path[path_len] = 0;                            //terminate path
    return 1;
}

//---------------------------------------------------------------------------
/*
 get the sequence number from existing direntry name
*/
static int getShortNameSequence(char* name, char* ext, const char* sname)
{
    int i, j;
    const char* tmp;
    char buf[8];

    //at first: compare extensions
    //if extensions differ then filenames are diffrerent and seq is 0
    tmp = sname + 8;
    for (i = 0; i < 3; i++) {
        if (ext[i] != tmp[i])
            return 0;
    }

    //at second: find tilde '~' character (search backward from the end)
    for (i = 7; i > 0 && name[i] != '~'; i--)
        ;

    if (i == 0)
        return 0; // tilde char was not found or is at first character

    //now compare the names - up to '~' position
    //if names differ then filenames are different;
    for (j = 0; j < i; j++) {
        if (name[j] != sname[j])
            return 0;
    }

    //if we get to this point we know that extension and name match
    //now get the sequence number behind the '~'
    for (j = i + 1; j < 8; j++)
        buf[j - i - 1] = name[j];
    buf[j - i - 1] = 0; //terminate

    M_DEBUG("found short name sequence number='%s' \n", buf);
    return strtol(buf, NULL, 10);
}

//---------------------------------------------------------------------------
/*
  set the short name sequence number
*/
static int setShortNameSequence(fat_driver* fatd, char* sname)
{
    char number[8];
    char* buf;
    int i, j;
    int seq;
    unsigned char mask;

    //dlanor: The code below was bugged in several ways, as it treated the high seq
    //values stored in long_name records as separate seqs, and also failed to accept
    //any valid seq value whose low part (stored in short_name record) was zero.
    //I'm now replacing this garbage with a bit-mask oriented scheme

    seq = SEQ_MASK_SIZE;
    for (i = 0; (i < (SEQ_MASK_SIZE >> 3)); i++) { //for each mask byte
        if ((mask = fatd->seq_mask[i]) != 0xFF) {  //if mask byte has any bit free
            for (j = 0; j < 8; j++, mask >>= 1) {  //for each bit in byte
                if ((mask & 1) == 0) {             //if free bit found
                    seq = (i << 3) + j;            //set seq value
                    break;                         //break bit loop
                }                                  //ends "if free bit found"
            }                                      //ends "for each bit in byte"
            break;                                 //break byte loop
        }                                          //ends "if mask byte has any bit free"
    }                                              //ends "for each mask byte"

    memset(number, 0, 8);
    sprintf(number, "%d", seq);
    j = strlen(number);

    buf    = sname + 7 - j;
    buf[0] = '~';
    buf++;
    for (i = 0; i < j; i++)
        buf[i] = number[i];

    return 0;
}

//---------------------------------------------------------------------------
/*
  find space where to put the direntry
*/
static int getDirentryStoreOffset(fat_driver* fatd, int entryCount, int direntrySize)
{
    int i;
    int tightIndex;
    int looseIndex;
    int cont;
    int id;
    int slotStart;
    int slotSize;
    int mask_ix, mask_sh;

    //we search for sequence of deleted or empty entries (cleared bits in dir_used_mask)
    //1) search the tight slot (that fits completely. ex: size = 3, and slot space is 3 )
    //2) search the suitable (loose) slot (ex: size = 3, slot space is 5)

    slotStart  = -1;
    slotSize   = 0;
    tightIndex = -1;
    looseIndex = -1;
    cont       = 1;
    //search the entries for entry types
    for (i = 0; i < entryCount && cont; i++) {
        mask_ix = i >> 3;
        mask_sh = i & 7;
        id      = fatd->dir_used_mask[mask_ix] & (1 << mask_sh);
        if (id == 0) { //empty entry
            if (slotStart >= 0) {
                slotSize++;
            } else {
                slotStart = i;
                slotSize  = 1;
                M_DEBUG("*Start slot at index=%d ", slotStart);
            }
        } else { //occupied entry
            if (tightIndex < 0 && slotSize == direntrySize) {
                tightIndex = slotStart;
                M_DEBUG("!Set tight index= %d\n", tightIndex);
            }
            if (looseIndex < 0 && slotSize > direntrySize) {
                looseIndex = slotStart + slotSize - direntrySize;
                M_DEBUG("!Set loose index= %d\n", looseIndex);
            }
            if (tightIndex >= 0 && looseIndex >= 0) {
                cont = 0;
            }
            slotStart = -1;
            slotSize  = 0;
        }
    }
    M_DEBUG("\n");

    // tight index - smaller fragmentation of space, the larger blocks
    //               are left for larger filenames.
    // loose index - more fragmentation of direntry space, but the direntry
    //               name has space for future enlargement (of the name).
    //               i.e. no need to reposition the direntry and / or
    //               to allocate additional fat cluster for direntry space.

    // we prefere tight slot if available, othervise we use the loose slot.
    // if there are no tight and no loose slot then we position new direntry
    // at the end of current direntry space
    if (tightIndex >= 0) {
        return tightIndex;
    }
    if (looseIndex >= 0) {
        return looseIndex;
    }
    //last entry is (most likely) empty - and if so, use it
    mask_ix = (entryCount - 1) >> 3;
    mask_sh = (entryCount - 1) & 7;
    id      = fatd->dir_used_mask[mask_ix] & (1 << mask_sh);
    if (id == 0) {
        return entryCount - 1;
    }

    return entryCount;
}

//---------------------------------------------------------------------------
/*
  scans current directory entries and fills the info records

  lname        - long filename (to test wether existing entry match the long name) (input)
  sname        - short filename ( ditto ^^ ) (input)
  startCluster - valid start cluster of the directory or 0 if we scan the root directory (input)
  directory    - Entry type to check against. 0 = file, >0 = directory, <0 = ignore type.

  if file/directory already exist (return code 0) then:
  retSector    - contains sector number of the direntry (output)
  retOffset    - contains byte offse of the SFN direntry from start of the sector (output)
  the reason is to speed up modification of the SFN (size of the file)
//dlanor: This function has been rewritten to use global bitmask arrays for output
*/
static int fat_fillDirentryInfo(fat_driver* fatd, const char* lname, const char* sname,
    char directory, unsigned int* startCluster,
    unsigned int* retSector, int* retOffset)
{
    fat_direntry_summary dir;
    int i, j;
    unsigned int startSector, dirSector, theSector;
    int cont;
    int ret;
    unsigned int dirPos;
    int seq;
    int mask_ix, mask_sh;

    memset(fatd->dir_used_mask, 0, DIR_MASK_SIZE / 8);
    memset(fatd->seq_mask, 0, SEQ_MASK_SIZE / 8);

    cont = 1;
    //clear name strings
    dir.sname[0] = 0;
    dir.name[0]  = 0;

    j = 0;
    if (directory > 0)
        directory = FAT_ATTR_DIRECTORY;

    fat_getDirentrySectorData(fatd, startCluster, &startSector, &dirSector);

    M_DEBUG("dirCluster=%u startSector=%u (%u) dirSector=%u \n", *startCluster, startSector, startSector * fatd->bd->sectorSize, dirSector);

    //go through first directory sector till the max number of directory sectors
    //or stop when no more direntries detected
    for (i = 0; i < dirSector && cont; i++) {
        unsigned char* sbuf = NULL; //sector buffer

        //At cluster borders, get correct sector from cluster chain buffer
        if ((*startCluster != 0) && (i % fatd->partBpb.clusterSize == 0)) {
            startSector = fat_cluster2sector(&fatd->partBpb, fatd->cbuf[(i / fatd->partBpb.clusterSize)]) - i;
        }
        theSector = startSector + i;
        ret       = READ_SECTOR(fatd, theSector, sbuf);
        if (ret < 0) {
            M_DEBUG("read directory sector failed ! sector=%u\n", theSector);
            return -EIO;
        }
        M_DEBUG("read sector ok, scanning sector for direntries...\n");
        dirPos = 0;

        // go through start of the sector till the end of sector
        while (cont && (dirPos < fatd->partBpb.sectorSize) && (j < DIR_MASK_SIZE)) {
            fat_direntry* dir_entry = (fat_direntry*)(sbuf + dirPos);
            cont                    = fat_getDirentry(fatd->partBpb.fatType, dir_entry, &dir); //get single directory entry from sector buffer
            mask_ix                 = j >> 3;
            mask_sh                 = j & 7;
            switch (cont) {
            case 1: //short name
                fatd->dir_used_mask[mask_ix] |= (1 << mask_sh);
                if (!(dir.attr & FAT_ATTR_VOLUME_LABEL)) { //not volume label
                    if ((strEqual(dir.sname, lname) == 0) || (strEqual(dir.name, lname) == 0)) {
                        //file we want to create already exist - return the cluster of the file
                        if ((directory >= 0) && ((dir.attr & FAT_ATTR_DIRECTORY) != directory)) {
                            //found directory but requested is file (and vice veresa)
                            if (directory)
                                return -ENOTDIR;
                            return -EISDIR;
                        } //ends "if" clause for mismatched file/folder state
                        M_DEBUG("I: entry found! %s, %s = %s\n", dir.name, dir.sname, lname);
                        *retSector               = theSector;
                        *retOffset               = dirPos;
                        *startCluster            = dir.cluster;
                        fatd->deSec[fatd->deIdx] = theSector;
                        fatd->deOfs[fatd->deIdx] = dirPos;
                        fatd->deIdx++;
                        return 0;
                    } //ends "if" clause for matching name
                    seq = getShortNameSequence((char*)dir_entry->sfn.name, (char*)dir_entry->sfn.ext, sname);
                    if (seq < SEQ_MASK_SIZE)
                        fatd->seq_mask[seq >> 3] |= (1 << (seq & 7));
                    fatd->deIdx = 0;
                    //clear name strings
                    dir.sname[0] = 0;
                    dir.name[0]  = 0;
                }      //ends "if(!(dir.attr & FAT_ATTR_VOLUME_LABEL))"
                else { //dlanor: Volume label
                    fatd->deIdx = 0;
                }
                break;
            case 2: //long name
                fatd->dir_used_mask[mask_ix] |= (1 << mask_sh);
                fatd->deSec[fatd->deIdx] = theSector;
                fatd->deOfs[fatd->deIdx] = dirPos;
                fatd->deIdx++;
                break;
            case 3: //empty
                fatd->deIdx = 0;
                break;
            } //ends "switch"
            dirPos += sizeof(fat_direntry);
            j++;
        }
    }
    //indicate inconsistency
    if (j >= DIR_MASK_SIZE) {
        j++;
    }
    return j;
}

//---------------------------------------------------------------------------
/*
  check wether the new direntries (note: one file have at least 2 direntries for 1 SFN and 1..n LFN)
  fit into the current directory space.
  Enlarges the directory space if needed and possible (note: root dirspace can't be enlarged for fat12 and fat16)

  startCluster - valid start cluster of dirpace or 0 for the root directory
  entryCount   - number of direntries of the filename (at least 2)
  entryIndex   - index where to store new direntries
  direntrySize - number of all direntries in the directory
*/

static int enlargeDirentryClusterSpace(fat_driver* fatd, unsigned int startCluster, int entryCount, int entryIndex, int direntrySize)
{
    int ret;
    unsigned int startSector, dirSector;
    int i;
    int maxSector;
    int entriesPerSector;
    int chainSize;
    unsigned int currentCluster;
    unsigned int newCluster;

    i = entryIndex + direntrySize;
    M_DEBUG("cur=%d ecount=%d \n", i, entryCount);
    //we don't need to enlarge directory cluster space
    if (i <= entryCount)
        return 0; //direntry fits into current space

    entriesPerSector = fatd->partBpb.sectorSize / 32;
    maxSector        = i / entriesPerSector;
    if (i % entriesPerSector) {
        maxSector++;
    }

    chainSize = fat_getDirentrySectorData(fatd, &startCluster, &startSector, &dirSector);

    M_DEBUG("maxSector=%u  dirSector=%u\n", maxSector, dirSector);

    if (maxSector <= dirSector)
        return 0;

    //Root directory of FAT12 or FAT16 - space can't be enlarged!
    if (startCluster == 0 && fatd->partBpb.fatType < FAT32) {
        return -EMLINK; //too many direntries in the root directory
    }

    //in the fatd->cbuf we have the cluster chain

    //get last cluster of the cluster chain
    currentCluster = fatd->cbuf[chainSize - 1];
    M_DEBUG("current (last) cluster=%u \n", currentCluster);

    //get 1 cluster from cluster stack and append the chain
    newCluster = fat_getFreeCluster(fatd, currentCluster);
    M_DEBUG("new cluster=%u \n", newCluster);
    fat_invalidateLastChainResult(fatd); //prevent to misuse current (now updated) fatd->cbuf
    //if new cluster cannot be allocated
    if (newCluster == 0) {
        return -ENOSPC;
    }

    // now clean the directory space
    startSector = fat_cluster2sector(&fatd->partBpb, newCluster);
    for (i = 0; i < fatd->partBpb.clusterSize; i++) {
        unsigned char* sbuf = NULL; //sector buffer

        ret = ALLOC_SECTOR(fatd, startSector + i, sbuf);
        memset(sbuf, 0, fatd->partBpb.sectorSize); //fill whole sector with zeros
        ret = WRITE_SECTOR(fatd, startSector + i);
        if (ret < 0)
            return -EIO;
    }
    return 1; // 1 cluster allocated
}

//---------------------------------------------------------------------------
/*
  Create empty directory space with two SFN direntries:
  1) current directory "."
  2) parent directory  ".."

*/
static int createDirectorySpace(fat_driver* fatd, unsigned int dirCluster, unsigned int parentDirCluster)
{
    int i, j;
    int ret;
    unsigned int startSector;

    //we do not mess with root directory
    if (dirCluster < 2) {
        return -EFAULT;
    }

    //we create directory space inside one cluster. No need to worry about
    //large dir space spread on multiple clusters
    startSector = fat_cluster2sector(&fatd->partBpb, dirCluster);
    M_DEBUG("I: create dir space: cluster=%u sector=%u (%u) \n", dirCluster, startSector, startSector * fatd->partBpb.sectorSize);

    //go through all sectors of the cluster
    for (i = 0; i < fatd->partBpb.clusterSize; i++) {
        unsigned char* sbuf = NULL; //sector buffer

        ret = ALLOC_SECTOR(fatd, startSector + i, sbuf);
        if (ret < 0) {
            M_DEBUG("alloc directory sector failed ! sector=%u\n", startSector + i);
            return -EIO;
        }
        memset(sbuf, 0, fatd->partBpb.sectorSize); //clean the sector
        if (i == 0) {
            fat_direntry_sfn* dsfn = (fat_direntry_sfn*)sbuf;
            char name[11];
            for (j = 1; j < 11; j++)
                name[j] = ' ';
            name[0] = '.';
            setSfnEntry(name, 1, dsfn + 0, dirCluster);
            name[1] = '.';
            setSfnEntry(name, 1, dsfn + 1, parentDirCluster);
        }
        ret = WRITE_SECTOR(fatd, startSector + i);
        if (ret < 0) {
            M_DEBUG("write directory sector failed ! sector=%u\n", startSector + i);
            return -EIO;
        }
    }

    return (0);
}

/*
  Update the parent directory ("..") entry cluster number of the specified SFN direntry.
*/
static int updateDirectoryParent(fat_driver* fatd, unsigned int dirCluster, unsigned int parentDirCluster)
{
    int i, j;
    int ret;
    unsigned int startSector;

    //we do not mess with root directory
    if (dirCluster < 2) {
        return -EFAULT;
    }

    //The basic directory space should be within one cluster. No need to worry about
    //large dir space spread on multiple clusters
    startSector = fat_cluster2sector(&fatd->partBpb, dirCluster);
    M_DEBUG("I: update dir parent: cluster=%u sector=%u (%u) \n", dirCluster, startSector, startSector * fatd->partBpb.sectorSize);

    //go through all sectors of the cluster
    for (i = 0; i < fatd->partBpb.clusterSize; i++) {
        unsigned char* sbuf = NULL; //sector buffer

        ret = READ_SECTOR(fatd, startSector + i, sbuf);
        if (ret < 0) {
            M_DEBUG("read directory sector failed ! sector=%u\n", startSector + i);
            return -EIO;
        }
        fat_direntry_sfn* dsfn = (fat_direntry_sfn*)sbuf;
        for (j = 0; j < fatd->partBpb.sectorSize; j += sizeof(fat_direntry_sfn), dsfn++) {
            if (memcmp(dsfn->name, "..      ", sizeof(dsfn->name)) == 0) {
                dsfn->clusterH[0] = (parentDirCluster & 0xFF0000) >> 16;
                dsfn->clusterH[1] = (parentDirCluster & 0xFF000000) >> 24;
                dsfn->clusterL[0] = (parentDirCluster & 0x00FF);
                dsfn->clusterL[1] = (parentDirCluster & 0xFF00) >> 8;

                ret = WRITE_SECTOR(fatd, startSector + i);
                if (ret < 0) {
                    M_DEBUG("write directory sector failed ! sector=%u\n", startSector + i);
                    return -EIO;
                }

                return 0;
            }
        }
    }

    return -1;
}

//---------------------------------------------------------------------------
/*
  Save direntries of the long and short filename to the directory space on the disk

  startCluster - start cluster of the directory space
  lname     : long name
  sname     : short name
  directory : 0-file, 1-directory
  cluster   : start cluster of the file/directory

  entrySize    - number of direntries to store
  entryIndex   - index of the direntry start in the directory space

  retSector    - contains sector number of the direntry (output)
  retOffset    - contains byte offse of the SFN direntry from start of the sector (output)
  the reason is to speed up modification of the SFN (size of the file)

  note: the filesize set in the direntry is 0 (for both directory and file)
*/
static int saveDirentry(fat_driver* fatd, unsigned int startCluster,
    const char* lname, const char* sname, char directory, unsigned int cluster,
    int entrySize, int entryIndex, unsigned int* retSector, int* retOffset, const fat_direntry_sfn* orig_dsfn)
{
    int i, j;
    unsigned int dirSector;
    unsigned int startSector;
    unsigned int theSector;
    int cont;
    int ret;
    unsigned int dirPos;
    int entryEndIndex;
    int writeFlag;
    int part = entrySize - 1;
    int nameSize;
    unsigned char chsum;

    chsum    = computeNameChecksum(sname);
    nameSize = strlen(lname);

    cont = 1;
    //clear name strings
    entryEndIndex = entryIndex + entrySize;

    j = 0;

    fat_getDirentrySectorData(fatd, &startCluster, &startSector, &dirSector);

    M_DEBUG("dirCluster=%u startSector=%u (%u) dirSector=%u \n", startCluster, startSector, startSector * fatd->bd->sectorSize, dirSector);

    //go through first directory sector till the max number of directory sectors
    //or stop when no more direntries detected
    for (i = 0; i < dirSector && cont; i++) {
        unsigned char* sbuf = NULL; //sector buffer

        //At cluster borders, get correct sector from cluster chain buffer
        if ((startCluster != 0) && (i % fatd->partBpb.clusterSize == 0)) {
            startSector = fat_cluster2sector(&fatd->partBpb, fatd->cbuf[(i / fatd->partBpb.clusterSize)]) - i;
        }
        theSector = startSector + i;
        ret       = READ_SECTOR(fatd, theSector, sbuf);
        if (ret < 0) {
            M_DEBUG("read directory sector failed ! sector=%u\n", theSector);
            return -EIO;
        }
        M_DEBUG("read sector ok, scanning sector for direntries...\n");
        dirPos    = 0;
        writeFlag = 0;
        // go through start of the sector till the end of sector
        while (dirPos < fatd->partBpb.sectorSize) {
            if (j >= entryIndex && j < entryEndIndex) {
                fat_direntry* dir_entry = (fat_direntry*)(sbuf + dirPos);
                if (part == 0) {
                    if (orig_dsfn == NULL)
                        setSfnEntry(sname, directory, &dir_entry->sfn, cluster);
                    else
                        setSfnEntryFromOld(sname, &dir_entry->sfn, orig_dsfn);
                } else
                    setLfnEntry(lname, nameSize, chsum, &dir_entry->lfn, part, entrySize - 1);
                part--;
                writeFlag++;
                //SFN is stored
                if (j == entryEndIndex - 1) {
                    *retSector = theSector;
                    *retOffset = dirPos;
                }
            }
            //sbuf + dirPos
            dirPos += sizeof(fat_direntry);
            j++;
        } //ends "while"
        //store modified sector
        if (writeFlag) {
            ret = WRITE_SECTOR(fatd, theSector);
            if (ret < 0) {
                M_DEBUG("write directory sector failed ! sector=%u\n", theSector);
                return -EIO;
            }
        }
        if (j >= entryEndIndex) {
            cont = 0; //do not continue
        }
    }
    return j;
}

//---------------------------------------------------------------------------
/*
  - create/convert long name to short name
  - analyse directory space
  - enlarge directory space
  - save direntries

  lname         - long name
  directory     - 0-> create file  1->create directory
  escapeNoExist - 0->allways create file  1->early exit if file doesn't exist
  startCluster  - directory space start cluster (set to zero for root directory)
  retSector     - SFN sector - sector of the SFN direntry (output)
  retOffset     - byte offset of the SFN direntry counting from the start of the sector (output)
*/

static int fat_modifyDirSpace(fat_driver* fatd, char* lname, char directory, char escapeNotExist, unsigned int startCluster, unsigned int* retSector, int* retOffset, const fat_direntry_sfn* orig_dsfn)
{
    char sname[12]; //short name 8+3 + terminator
    unsigned int newCluster, parentDirCluster_tmp;
    int ret, entryCount, compressShortName, entryIndex, direntrySize;

    //dlanor: Since each filename may need up to 11 directory entries (MAX_FAT_NAME==128)
    //dlanor: I've rewritten the methods of this function to use global bitmasks as this
    //dlanor: allows more effective entry handling than most other methods
    //
    //memo buffer for each direntry - up to 1024 entries in directory
    // 7 6 5 4 3 2 | 1 0
    // ------------+-----
    // SEQ HI/LO   | ID
    // ------------------
    // ID : 0 - entry is empty or deleted
    //      1 - sfn entry
    //      2 - lfn entry
    //      3 - other entry (volume label etc.)
    // SEQ: sequence number of the short filename.
    // if our long filename is "Quitelongname.txt" then
    // seq for existing entry:
    // ABCD.TXT     seq = 0 (no sequence number in name and name doesn't match)
    // ABCDEF~1.TXT seq = 0 (because the short names doesn't match)
    // QUITELON.TXT seq = 0 (again the short name doesn't match)
    // QUITEL~1.JPG seq = 0 (name match but extension desn't match)
    // QUITEL~1.TXT seq = 1 (both name and extension match - sequence number 1)
    // QUITE~85.TXT seq = 85 ( dtto. ^^^^)

    // If the sfn has sequence it means the filename should be long
    // and preceeding entry should be lfn. In this case the preceeding (lfn)
    // entry seq holds the high 6 bites of the whole sequence. If preceding
    // entry is another sfn direntry then we report error (even if it might be
    // (in some rare occasions) correct directory structure).

    sname[11] = 0;

    //create short name from long name
    ret = createShortNameMask(lname, sname);
    if (ret < 0) {
        M_DEBUG("E: short name invalid!\n");
        return ret;
    }
    compressShortName = ret;

    //get information about existing direntries (palcement of the empty/reusable direntries)
    //and sequence numbers of the short filenames
    parentDirCluster_tmp = startCluster;
    ret                  = fat_fillDirentryInfo(fatd, lname, sname, directory,
        &parentDirCluster_tmp, retSector, retOffset);
    if (ret < 0) {
        M_DEBUG("E: direntry data invalid!\n");
        return ret;
    }
    //ret 0 means that exact filename/directory already exist
    if (ret == 0) {
        return ret;
    }

    //exact filename not exist and we want to report it
    if (escapeNotExist) {
        return -ENOENT;
    }

    if (ret > DIR_MASK_SIZE) {
        M_DEBUG("W: Direntry count is larger than number of records!\n");
        ret = DIR_MASK_SIZE;
    }
    entryCount = ret;
    M_DEBUG("I: direntry count=%d\n", entryCount);

    if (compressShortName) {
        setShortNameSequence(fatd, sname);
    }
    M_DEBUG("I: new short name='%s' \n", sname);

    //direntry size for long name + 1 additional direntry for short name
    direntrySize = getDirentrySize(lname) + 1;
    M_DEBUG("Direntry size=%d\n", direntrySize);

    //find the offset (index) of the direntry space where to put this direntry
    entryIndex = getDirentryStoreOffset(fatd, entryCount, direntrySize);
    M_DEBUG("I: direntry store offset=%d\n", entryIndex);

    //if the direntry offset excede current space of directory clusters
    //we have to add one cluster to directory space
    ret = enlargeDirentryClusterSpace(fatd, startCluster, entryCount, entryIndex, direntrySize);
    M_DEBUG("I: enlarge direntry cluster space ret=%d\n", ret);
    if (ret < 0) {
        return ret;
    }

    if (orig_dsfn == NULL) {
        //get new cluster for file/directory
        newCluster = fat_getFreeCluster(fatd, 0);
        if (newCluster == 0) {
            return -ENOSPC;
        }
        M_DEBUG("I: new file/dir cluster=%u\n", newCluster);
    } else {
        newCluster = 0;
    }

    //now store direntries into the directory space
    ret = saveDirentry(fatd, startCluster, lname, sname, directory, newCluster, direntrySize, entryIndex, retSector, retOffset, orig_dsfn);
    M_DEBUG("I: save direntry ret=%d\n", ret);
    if (ret < 0) {
        return ret;
    }

    //create empty directory structure
    if ((orig_dsfn == NULL) && directory) {
        ret = createDirectorySpace(fatd, newCluster, startCluster);
        M_DEBUG("I: create directory space ret=%d\n", ret);
        if (ret < 0) {
            return ret;
        }
    }

    return 1;
}

//---------------------------------------------------------------------------
/*
   Check whether directory space contain any file or directory

   startCluster - start cluster of the directory space

   returns: 0 - false - directory space contains files or directories (except '.' and '..')
            1 - true  - directory space is empty or contains deleted entries
           -X - error
*/
static int checkDirspaceEmpty(fat_driver* fatd, unsigned int startCluster)
{
    int ret;
    int i;
    char sname[12]; //short name 8+3 + terminator
    int entryCount;

    unsigned int retSector;
    int retOffset;

    M_DEBUG("I: checkDirspaceEmpty  directory cluster=%u \n", startCluster);
    if (startCluster < 2) { // do not check root directory!
        return -EFAULT;
    }

    sname[0] = 0;

    ret = fat_fillDirentryInfo(fatd, sname, sname, 1,
        &startCluster, &retSector, &retOffset);
    if (ret > DIR_MASK_SIZE) {
        M_DEBUG("W: Direntry count is larger than number of records! directory space cluster =%u maxRecords=%u\n", startCluster, DIR_MASK_SIZE);
        ret = DIR_MASK_SIZE;
    }
    entryCount = ret;
    //first two records should be '.' & '..', which don't count as real content
    if ((fatd->dir_used_mask[0] & 0xFC) != 0)
        goto non_empty;
    for (i = 1; i < (entryCount / 8); i++) {
        if (fatd->dir_used_mask[i] != 0) {
        non_empty:
            M_DEBUG("I: directory not empty!\n");
            return 0;
        } //ends "if"
    }     //ends "for"
    M_DEBUG("I: directory is empty.\n");
    return 1;
}

//---------------------------------------------------------------------------
static int fat_wipeDirEntries(fat_driver* fatd)
{
    int ret;
    unsigned int i, theSector;
    unsigned char* sbuf = NULL;

    //now mark direntries as deleted
    theSector = 0;
    ret       = 0;
    for (i = 0; i < fatd->deIdx; i++) {
        if (fatd->deSec[i] != theSector) {
            if (theSector > 0) {
                ret = WRITE_SECTOR(fatd, theSector);
                if (ret < 0) {
                    M_DEBUG("write directory sector failed ! sector=%u\n", theSector);
                    ret = -EIO;
                    break;
                }
            }
            theSector = fatd->deSec[i];
            ret       = READ_SECTOR(fatd, theSector, sbuf);
            if (ret < 0) {
                M_DEBUG("read directory sector failed ! sector=%u\n", theSector);
                ret = -EIO;
                break;
            }
        }
        sbuf[fatd->deOfs[i]] = 0xE5; //delete marker
    }
    if (theSector > 0) {
        ret = WRITE_SECTOR(fatd, theSector);
        if (ret < 0) {
            M_DEBUG("write directory sector failed ! sector=%u\n", theSector);
            ret = -EIO;
        }
    }

    return ret;
}

/*
   Remove the name (direntries of the file or directory) from the directory space.

   lname        - long name (without the path)
   directory    - 0->delete file    1-delete directory
   startCluster - start cluster of the directory space
*/

static int fat_clearDirSpace(fat_driver* fatd, char* lname, char directory, unsigned int* startCluster)
{
    int ret;
    char sname[12]; //short name 8+3 + terminator
    unsigned int dirCluster;
    unsigned int sfnSector;
    int sfnOffset;

    sname[0] = 0;

    dirCluster = *startCluster;
    //get information about existing direntries (palcement of the empty/reusable direntries)
    //and find the lname in the directory
    //also fill up the dsSec and dsOfs information
    ret = fat_fillDirentryInfo(fatd, lname, sname, directory,
        startCluster, &sfnSector, &sfnOffset);
    if (ret != 0) {
        M_DEBUG("E: direntry not found!\n");
        return -ENOENT;
    }
    M_DEBUG("clear dir space: dir found at  cluster=%u \n ", *startCluster);

    //Check wether any file or directory exist in te target directory space.
    //We should not delete the directory if files/directories exist
    //because just clearing the dir doesn't free the fat clusters
    //occupied by files.
    if (directory) {
        //check wether sub-directory is empty
        //startCluster now points to subdirectory start cluster
        ret = checkDirspaceEmpty(fatd, *startCluster);
        if (ret == 0) { //directorty contains some files
            return -ENOTEMPTY;
        }
        //read the direntry info again, because we lost it during subdir check
        *startCluster = dirCluster;

        ret = fat_fillDirentryInfo(fatd, lname, sname, directory,
            startCluster, &sfnSector, &sfnOffset);
        if (ret != 0) {
            M_DEBUG("E: direntry not found!\n");
            return -ENOENT;
        }
    }

    //now mark direntries as deleted
    ret = fat_wipeDirEntries(fatd);
    if (ret < 0) {
        M_DEBUG("E: wipe direntries failed!\n");
        return ret;
    }

    //now delete whole cluster chain starting at the file's first cluster
    ret = fat_deleteClusterChain(fatd, *startCluster);
    if (ret < 0) {
        M_DEBUG("E: delete cluster chain failed!\n");
        return ret;
    }
    return 1;
}

/* ===================================================================== */

/*
  cluster  - start cluster of the file
  sfnSector - short filename entry sector
  sfnOffset - short filename entry offset from the sector start
*/
int fat_truncateFile(fat_driver* fatd, unsigned int cluster, unsigned int sfnSector, int sfnOffset)
{
    int ret;
    fat_direntry_sfn* dsfn;
    unsigned char* sbuf = NULL; //sector buffer

    if (cluster == 0 || sfnSector == 0) {
        return -EFAULT;
    }

    //now delete whole cluster chain starting at the file's first cluster
    ret = fat_deleteClusterChain(fatd, cluster);
    if (ret < 0) {
        M_DEBUG("E: delete cluster chain failed!\n");
        return ret;
    }

    //terminate cluster
    ret = fat_createClusterChain(fatd, cluster);
    if (ret < 0) {
        M_DEBUG("E: truncate cluster chain failed!\n");
        return ret;
    }

    ret = READ_SECTOR(fatd, sfnSector, sbuf);
    if (ret < 0) {
        M_DEBUG("read direntry sector failed ! sector=%u\n", sfnSector);
        return -EIO;
    }
    dsfn          = (fat_direntry_sfn*)(sbuf + sfnOffset);
    dsfn->size[0] = 0;
    dsfn->size[1] = 0;
    dsfn->size[2] = 0;
    dsfn->size[3] = 0;

    ret = WRITE_SECTOR(fatd, sfnSector);
    if (ret < 0) {
        M_DEBUG("write directory sector failed ! sector=%u\n", sfnSector);
        return -EIO;
    }
    return 1;
}

//---------------------------------------------------------------------------
/*
  Update size of the SFN entry

  cluster  - start cluster of the file
  sfnSector - short filename entry sector
  sfnOffset - short filename entry offset from the sector start
*/
int fat_updateSfn(fat_driver* fatd, int size, unsigned int sfnSector, int sfnOffset)
{
    int ret;
    fat_direntry_sfn* dsfn;
    unsigned char* sbuf = NULL; //sector buffer

    if (sfnSector == 0) {
        return -EFAULT;
    }

    ret = READ_SECTOR(fatd, sfnSector, sbuf);
    if (ret < 0) {
        M_DEBUG("read direntry sector failed ! sector=%u\n", sfnSector);
        return -EIO;
    }
    dsfn          = (fat_direntry_sfn*)(sbuf + sfnOffset);
    dsfn->size[0] = size & 0xFF;
    dsfn->size[1] = (size & 0xFF00) >> 8;
    dsfn->size[2] = (size & 0xFF0000) >> 16;
    dsfn->size[3] = (size & 0xFF000000) >> 24;

    setSfnDate(dsfn, DATE_MODIFY);

    ret = WRITE_SECTOR(fatd, sfnSector);
    if (ret < 0) {
        M_DEBUG("write directory sector failed ! sector=%u\n", sfnSector);
        return -EIO;
    }
    M_DEBUG("I: sfn updated, file size=%d \n", size);
    return 1;
}

//---------------------------------------------------------------------------
/*
 create file or directory

 fname          - path and filename
 directory      - set to 0 to create file, 1 to create directory
 escapeNotExist - set to 1 if you want to report error if file not exist.
                  Otherwise set to 0 and file/dir will be created.
 cluster        - start cluster of the directory space - default is 0 -> root directory
 sfnSector      - sector of the SFN entry (output) - helps to modify direntry (size, date, time)
 sfnOffset      - offset (in bytes) of the SFN entry (output)
*/

int fat_createFile(fat_driver* fatd, const char* fname, char directory, char escapeNotExist, unsigned int* cluster, unsigned int* sfnSector, int* sfnOffset)
{
    int ret;
    unsigned int startCluster;
    unsigned int directoryCluster;
    char lname[FAT_MAX_NAME], pathToDirent[FAT_MAX_PATH];
    fat_dir fatdir;

    ret = separatePathAndName(fname, pathToDirent, lname);
    if ((ret < 0)           //if name invalid to separation routine
        || ((lname[0] == 0) //or name is empty string
               || ((lname[0] == '.')
                      && ((lname[1] == 0) //or name is single period
                             || ((lname[1] == '.')
                                    && (lname[2] == 0) //or name is two periods
                                    ))))) {
        M_DEBUG("E: file name not exist or not valid!");
        return -ENOENT;
    }

    M_DEBUG("Calling fat_getFileStartCluster from fat_createFile\n");
    //get start cluster of the last sub-directory of the path
    startCluster = 0;
    ret          = fat_getFileStartCluster(fatd, pathToDirent, &startCluster, &fatdir);
    if (ret < 0) {
        M_DEBUG("E: directory not found! \n");
        return ret;
    }

    if (!(fatdir.attr & FAT_ATTR_DIRECTORY)) {
        M_DEBUG("E: directory not found! \n");
        return -ENOENT;
    }

    M_DEBUG("directory=%s name=%s cluster=%u \n", pathToDirent, lname, startCluster);

    if (fatdir.attr & FAT_ATTR_READONLY)
        return -EACCES;

    //modify directory space of the path (cread direntries)
    //and/or create new (empty) directory space if directory creation requested
    directoryCluster = startCluster;
    ret              = fat_modifyDirSpace(fatd, lname, directory, escapeNotExist, startCluster, sfnSector, sfnOffset, NULL);
    if (ret < 0) {
        M_DEBUG("E: modifyDirSpace failed!\n");
        return ret;
    }
    M_DEBUG("I: SFN info: sector=%u (%u)  offset=%u (%u) startCluster=%u\n", *sfnSector, *sfnSector * fatd->partBpb.sectorSize, *sfnOffset, *sfnOffset + (*sfnSector * fatd->partBpb.sectorSize), startCluster);
    *cluster = startCluster;
    //dlanor: I've repatched the stuff below to improve functionality
    //The simple test below was bugged for the case of creating a folder in root
    //if (startCluster != directoryCluster) {
    //That test (on the line above) fails in root because created folders never
    //get a startCluster of 0. That is reserved for root only.
    //The test below replaces this with a more complex test that takes all of this
    //stuff into proper account, but behaves like the old test for other cases.
    //That's mainly because I can't tell how consistent name conflict flagging is
    //for those other cases, and it's not really worth the trouble of finding out :)
    if ((ret == 0)                                   //if we have a directly flagged name conflict
        || ((directoryCluster || !directory)         //OR working in non_root, or making a file
               && (startCluster != directoryCluster) //AND we get an unexpected startCluster
               )) {
        M_DEBUG("I: file already exists at cluster=%u\n", startCluster);
        return 2;
    }
    return 0;
}

//---------------------------------------------------------------------------
int fat_deleteFile(fat_driver* fatd, const char* fname, char directory)
{
    int ret;
    unsigned int startCluster;
    unsigned int directoryCluster;
    char lname[FAT_MAX_NAME], pathToDirent[FAT_MAX_PATH];
    fat_dir fatdir;

    ret = separatePathAndName(fname, pathToDirent, lname);
    if ((ret < 0)           //if name invalid to separation routine
        || ((lname[0] == 0) //or name is empty string
               || ((lname[0] == '.')
                      && ((lname[1] == 0) //or name is single period
                             || ((lname[1] == '.')
                                    && (lname[2] == 0) //or name is two periods
                                    ))))) {
        M_DEBUG("E: file name not exist or not valid!");
        return -ENOENT;
    }

    M_DEBUG("Calling fat_getFileStartCluster from fat_deleteFile\n");
    //get start cluster of the last sub-directory of the path
    startCluster = 0;
    ret          = fat_getFileStartCluster(fatd, pathToDirent, &startCluster, &fatdir);
    if (ret < 0) {
        M_DEBUG("E: directory not found! \n");
        return ret;
    }

    if (!(fatdir.attr & FAT_ATTR_DIRECTORY)) {
        M_DEBUG("E: directory not found! \n");
        return -ENOENT;
    }

    M_DEBUG("directory=%s name=%s cluster=%u \n", pathToDirent, lname, startCluster);

    if (fatdir.attr & FAT_ATTR_READONLY) {
        M_DEBUG("E: directory read only! \n");
        return -EACCES;
    }

    //delete direntries and modify fat
    directoryCluster = startCluster;
    ret              = fat_clearDirSpace(fatd, lname, directory, &startCluster);
    if (ret < 0) {
        M_DEBUG("E: cleanDirSpace failed!\n");
        return ret;
    }
    if (startCluster != directoryCluster) {
        M_DEBUG("I: file/dir removed from cluster=%u\n", startCluster);
    }
    return 0;
}

//---------------------------------------------------------------------------
//Create a new record that points to the file/directory, before deleting the original one.

int fat_renameFile(fat_driver* fatd, fat_dir* fatdir, const char* fname)
{
    int ret;
    unsigned int sDirCluster;
    unsigned int dDirCluster, dParentDirCluster;
    char lname[FAT_MAX_NAME], pathToDirent[FAT_MAX_PATH];
    unsigned int sfnSector, new_sfnSector;
    int sfnOffset, new_sfnOffset;
    char sname[12]; //short name 8+3 + terminator
    unsigned char* sbuf = NULL;
    unsigned char srcIsDirectory;
    fat_direntry_sfn OriginalSFN;

    ret = separatePathAndName(fname, pathToDirent, lname);
    if ((ret < 0)           //if name invalid to separation routine
        || ((lname[0] == 0) //or name is empty string
               || ((lname[0] == '.')
                      && ((lname[1] == 0) //or name is single period
                             || ((lname[1] == '.')
                                    && (lname[2] == 0) //or name is two periods
                                    ))))) {
        M_DEBUG("E: destination file name not exist or not valid!");
        return -ENOENT;
    }

    //Check if the source file exists, and that the new filename is not in use.
    sDirCluster = fatdir->parentDirCluster;
    dDirCluster = 0;
    M_DEBUG("Calling fat_getFileStartCluster from fat_renameFile\n");

    ret = fat_getFileStartCluster(fatd, pathToDirent, &dDirCluster, NULL);
    if (ret < 0) {
        M_DEBUG("E: destination directory not found! \n");
        return ret;
    }
    dParentDirCluster = dDirCluster; //Backup dDirCluster, as every call to fat_filleDirentryInfo will update it to point to the scanned file's first cluster.

    //Get the SFN sector number and offset, so that the SFN record can be read.
    sname[0] = 0;
    ret      = fat_fillDirentryInfo(fatd, fatdir->name, sname, -1, &sDirCluster, &sfnSector, &sfnOffset);
    if (ret != 0) {
        M_DEBUG("E: direntry not found! %d\n", ret);
        return -ENOENT;
    }

    M_DEBUG("fat_renameFile: dir found at  cluster=%u \n ", sDirCluster);

    //Preserve the original SFN entry.
    if ((ret = READ_SECTOR(fatd, sfnSector, sbuf)) < 0) {
        M_DEBUG("E: I/O error! %d\n", ret);
        return ret;
    }
    memcpy(&OriginalSFN, (fat_direntry_sfn*)(sbuf + sfnOffset), sizeof(fat_direntry_sfn));
    srcIsDirectory = ((fat_direntry_sfn*)(sbuf + sfnOffset))->attr & FAT_ATTR_DIRECTORY;

    ret = fat_fillDirentryInfo(fatd, lname, sname, srcIsDirectory, &dDirCluster, &new_sfnSector, &new_sfnOffset);
    if (ret == 0) {
        //Entry is found and the type is the same as the original (both are either directories or files).
        if (srcIsDirectory) {
            //If directory, the rename can still take place if the directory can be deleted (is empty).
            dDirCluster = dParentDirCluster;
            if ((ret = fat_clearDirSpace(fatd, lname, 1, &dDirCluster)) < 0)
                return ret;
        } else {
            //Do not allow a file to be renamed to an existing file.
            return -EEXIST;
        }
    }

    //Insert a new record.
    if ((ret = fat_modifyDirSpace(fatd, lname, srcIsDirectory, 0, dParentDirCluster, &sfnSector, &sfnOffset, &OriginalSFN)) < 0) {
        M_DEBUG("E: fat_modifyDirSpace failed! %d\n", ret);
        return ret;
    }

    //If it is a directory, update the parent directory record.
    if (srcIsDirectory) {
        dDirCluster = (fatd->partBpb.fatType == FAT32) ? getI32_2(OriginalSFN.clusterL, OriginalSFN.clusterH) : getI16(OriginalSFN.clusterL);
        if ((ret = updateDirectoryParent(fatd, dDirCluster, dParentDirCluster)) != 0) {
            M_DEBUG("E: could not update \"..\" entry! %d\n", ret);
            return ret;
        }
    }

    //Wipe the original entry.
    //Fill fatd with a list of the original directory entry records, which can be deleted.
    sDirCluster = fatdir->parentDirCluster;
    ret         = fat_fillDirentryInfo(fatd, fatdir->name, sname, -1, &sDirCluster, &sfnSector, &sfnOffset);
    if (ret != 0) {
        M_DEBUG("E: direntry not found! %d\n", ret);
        return -ENOENT;
    }

    //now mark the original direntries as deleted
    ret = fat_wipeDirEntries(fatd);
    if (ret < 0) {
        M_DEBUG("E: wipe direntries failed!\n");
        return ret;
    }

    fatdir->parentDirCluster = dParentDirCluster;
    strcpy(fatdir->name, lname);

    return 0;
} //ends fat_renameFile

//---------------------------------------------------------------------------
int fat_writeFile(fat_driver* fatd, fat_dir* fatDir, int* updateClusterIndices, unsigned int filePos, unsigned char* buffer, unsigned int size)
{
    int ret;
    int i, j;
    int chainSize;
    int nextChain;
    int startSector;
    unsigned int bufSize;
    int sectorSkip;
    int clusterSkip;
    int dataSkip;

    unsigned int bufferPos;
    unsigned int fileCluster;
    unsigned int clusterPos;
    unsigned int endPosFile;
    unsigned int endPosCluster;
    unsigned int lastCluster;

    int clusterChainStart;

    //check wether we have enough clusters allocated
    i = fatd->partBpb.clusterSize * fatd->partBpb.sectorSize; //the size (in bytes) of the one cluster
    j = fatDir->size / i;
    if (fatDir->size % i) {
        j++;
    }
    if (j == 0)
        j = 1; //the file have allways at least one cluster allocated

    endPosCluster = j * i;
    endPosFile    = filePos + size;

    *updateClusterIndices = 0;

    //allocate additional cluster(s)
    if (endPosFile > endPosCluster) {
        ret = endPosFile - endPosCluster; //additional space needed in bytes
        j   = ret / i;                    //additional space needed (given in number of clusters)
        if (ret % i) {
            j++;
        }
        lastCluster = fatDir->lastCluster;
        M_DEBUG("I: writeFile: last cluster= %u \n", lastCluster);

        if (lastCluster == 0)
            return -ENOSPC; // no more free clusters or data invalid
        for (i = 0; i < j; i++) {
            lastCluster = fat_getFreeCluster(fatd, lastCluster);
            if (lastCluster == 0)
                return -ENOSPC; // no more free clusters
        }
        fatDir->lastCluster   = lastCluster;
        *updateClusterIndices = j;
        fat_invalidateLastChainResult(fatd); //prevent to misuse current (now deleted) fatd->cbuf

        M_DEBUG("I: writeFile: new clusters allocated = %u new lastCluster=%u \n", j, lastCluster);
    }
    M_DEBUG("I: write file: filePos=%d  dataSize=%d \n", filePos, size);

    fat_getClusterAtFilePos(fatd, fatDir, filePos, &fileCluster, &clusterPos);
    sectorSkip  = (filePos - clusterPos) / fatd->partBpb.sectorSize;
    clusterSkip = sectorSkip / fatd->partBpb.clusterSize;
    sectorSkip %= fatd->partBpb.clusterSize;
    dataSkip  = filePos % fatd->partBpb.sectorSize;
    bufferPos = 0;

    M_DEBUG("fileCluster = %u,  clusterPos= %u clusterSkip=%u, sectorSkip=%u dataSkip=%u \n",
        fileCluster, clusterPos, clusterSkip, sectorSkip, dataSkip);

    if (fileCluster < 2) {
        return -EFAULT;
    }

    bufSize           = fatd->bd->sectorSize;
    nextChain         = 1;
    clusterChainStart = 1;

    while (nextChain && size > 0) {
        chainSize         = fat_getClusterChain(fatd, fileCluster, fatd->cbuf, MAX_DIR_CLUSTER, clusterChainStart);
        clusterChainStart = 0;
        if (chainSize >= MAX_DIR_CLUSTER) { //the chain is full, but more chain parts exist
            fileCluster = fatd->cbuf[MAX_DIR_CLUSTER - 1];
        } else { //chain fits in the chain buffer completely - no next chain needed
            nextChain = 0;
        }
        while (clusterSkip >= MAX_DIR_CLUSTER) {
            chainSize         = fat_getClusterChain(fatd, fileCluster, fatd->cbuf, MAX_DIR_CLUSTER, clusterChainStart);
            clusterChainStart = 0;
            if (chainSize >= MAX_DIR_CLUSTER) { //the chain is full, but more chain parts exist
                fileCluster = fatd->cbuf[MAX_DIR_CLUSTER - 1];
            } else { //chain fits in the chain buffer completely - no next chain needed
                nextChain = 0;
            }
            clusterSkip -= MAX_DIR_CLUSTER;
        }

        //process the cluster chain (fatd->cbuf) and skip leading clusters if needed
        for (i = 0 + clusterSkip; i < chainSize && size > 0; i++) {
            //read cluster and save cluster content
            startSector = fat_cluster2sector(&fatd->partBpb, fatd->cbuf[i]);
            //process all sectors of the cluster (and skip leading sectors if needed)
            for (j = 0 + sectorSkip; j < fatd->partBpb.clusterSize && size > 0; j++) {
                unsigned char* sbuf = NULL; //sector buffer

                //compute exact size of transfered bytes
                if (size < bufSize) {
                    bufSize = size + dataSkip;
                }
                if (bufSize > fatd->bd->sectorSize) {
                    bufSize = fatd->bd->sectorSize;
                }

                ret = READ_SECTOR(fatd, startSector + j, sbuf);
                if (ret < 0) {
                    M_DEBUG("Read sector failed ! sector=%u\n", startSector + j);
                    return bufferPos; //return number of bytes already written
                }

                M_DEBUG("memcopy dst=%u, src=%u, size=%u  bufSize=%u \n", dataSkip, bufferPos, bufSize - dataSkip, bufSize);
                memcpy(sbuf + dataSkip, buffer + bufferPos, bufSize - dataSkip);
                ret = WRITE_SECTOR(fatd, startSector + j);
                if (ret < 0) {
                    M_DEBUG("Write sector failed ! sector=%u\n", startSector + j);
                    return bufferPos; //return number of bytes already written
                }

                size -= (bufSize - dataSkip);
                bufferPos += (bufSize - dataSkip);
                dataSkip = 0;
                bufSize  = fatd->bd->sectorSize;
            }
            sectorSkip = 0;
        }
        clusterSkip = 0;
    }

    return bufferPos; //return number of bytes already written
}

//---------------------------------------------------------------------------
int fat_flushSectors(fat_driver* fatd)
{
    FLUSH_SECTORS(fatd);
    return (0);
}

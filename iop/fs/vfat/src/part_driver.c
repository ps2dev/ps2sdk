//---------------------------------------------------------------------------
// File name:    part_driver.c
//---------------------------------------------------------------------------
#include <stdio.h>
#include <errno.h>
#ifdef BUILDING_USBHDFSD
#include <usbhdfsd.h>
#endif /* BUILDING_USBHDFSD */
#include "usbhd_common.h"
#include "scache.h"
#include "part_driver.h"
#ifdef BUILDING_USBHDFSD
#include "mass_stor.h"
#endif /* BUILDING_USBHDFSD */
#ifdef BUILDING_IEEE1394_DISK
#include "sbp2_disk.h"
#endif /* BUILDING_IEEE1394_DISK */
#include "fat_driver.h"

// #define DEBUG  //comment out this line when not debugging

#include "mass_debug.h"

#define READ_SECTOR(d, a, b) scache_readSector((d)->cache, (a), (void **)&b)

typedef struct _part_record
{
    unsigned char sid;  // system id-    4=16bit FAT (16bit sector numbers)
                        //        5=extended partition
                        //        6=16bit FAT (32bit sector numbers)
    unsigned int start; // start sector of the partition
    unsigned int count; // length of the partititon (total number of sectors)
} part_record;

typedef struct _part_table
{
    part_record record[4]; // maximum of 4 primary partitions
} part_table;

typedef struct _part_raw_record
{
    unsigned char active;      // Set to 80h if this partition is active / bootable
    unsigned char startH;      // Partition's starting head.
    unsigned char startST[2];  // Partition's starting sector and track.
    unsigned char sid;         // Partition's system ID number.
    unsigned char endH;        // Partition's ending head.
    unsigned char endST[2];    // Partition's ending sector and track.
    unsigned char startLBA[4]; // Starting LBA (sector)
    unsigned char size[4];     // Partition size in sectors.
} part_raw_record;

//---------------------------------------------------------------------------
#if defined(BUILDING_USBHDFSD)
static USBHD_INLINE int part_getPartitionRecord(mass_dev *dev, part_raw_record *raw, part_record *rec)
#elif defined(BUILDING_IEEE1394_DISK)
static USBHD_INLINE int part_getPartitionRecord(struct SBP2Device *dev, part_raw_record *raw, part_record *rec)
#else
static USBHD_INLINE int part_getPartitionRecord(void *dev, part_raw_record *raw, part_record *rec)
#endif
{
    rec->sid   = raw->sid;
    rec->start = getUI32(raw->startLBA);
    rec->count = getUI32(raw->size);

    // Ignore partitions that have a partition type/system ID set to 0.
    if (rec->sid != 0x00) { /*    Windows appears to check if the start LBA is not 0 and whether the start LBA is within the disk.
                                There may be checks against the size, but I didn't manage to identify a pattern.
                                If the disk has no partition table (i.e. disks with "removable" media), then this check is also one safeguard. */
        if ((rec->start == 0) || (rec->start >= dev->maxLBA))
            return -1;

        return 0;
    }

    return 1;
}

//---------------------------------------------------------------------------
#if defined(BUILDING_USBHDFSD)
static int part_getPartitionTable(mass_dev *dev, part_table *part)
#elif defined(BUILDING_IEEE1394_DISK)
static int part_getPartitionTable(struct SBP2Device *dev, part_table *part)
#else
static int part_getPartitionTable(void *dev, part_table *part)
#endif
{
    int ret;
    unsigned int i;
    unsigned char *sbuf;

    ret = READ_SECTOR(dev, 0, sbuf); // read sector 0 - Disk MBR or boot sector
    if (ret < 0) {
        XPRINTF("part_getPartitionTable read failed %d!\n", ret);
        return -EIO;
    }

    /* A VBR (i.e. diskette-like) also shares this signature, in the same place.
       Hence passing this check only indicates the presence of a VBR/MBR, but does not indicate which it is. */
    printf("USBHDFSD: boot signature %X %X\n", sbuf[0x1FE], sbuf[0x1FF]);
    if (sbuf[0x1FE] == 0x55 && sbuf[0x1FF] == 0xAA) {
        int partitions;

        for (partitions = 0, i = 0; i < 4; i++) {
            part_raw_record *part_raw;

            part_raw = (part_raw_record *)(sbuf + 0x01BE + (i * 16));
            ret      = part_getPartitionRecord(dev, part_raw, &part->record[i]);
            if (ret < 0)
                return 0;      // Invalid record encountered, so the table is probably invalid.
            else if (ret == 0) // Count the number of valid entries.
                partitions++;
        }

        // Check the number of partitions validated. Having all entries set to 0, could mean it is not a MBR.
        return (partitions == 0 ? 0 : 4);
    } else {
        for (i = 0; i < 4; i++) {
            part->record[i].sid = 0;
        }
        return 0;
    }
}

//---------------------------------------------------------------------------
#if defined(BUILDING_USBHDFSD)
int part_connect(mass_dev *dev)
#elif defined(BUILDING_IEEE1394_DISK)
int part_connect(struct SBP2Device *dev)
#else
int part_connect(void *dev)
#endif
{
    part_table partTable;
    int parts;
#ifdef BUILDING_USBHDFSD
    XPRINTF("part_connect devId %i \n", dev->devId);
#endif /* BUILDING_USBHDFSD */
#ifdef BUILDING_IEEE1394_DISK
    XPRINTF("part_connect devId %i \n", dev->nodeID);
#endif /* BUILDING_IEEE1394_DISK */

    if ((parts = part_getPartitionTable(dev, &partTable)) < 0)
        return -1;

    if (parts > 0) {
        unsigned int count = 0, i;
        for (i = 0; i < (unsigned int)parts; i++) {
            if (
                partTable.record[i].sid == 6 ||
                partTable.record[i].sid == 4 ||
                partTable.record[i].sid == 1 || // fat 16, fat 12
                partTable.record[i].sid == 0x0B ||
                partTable.record[i].sid == 0x0C || // fat 32
                partTable.record[i].sid == 0x0E)   // fat 16 LBA
            {
                XPRINTF("mount partition %d id %02x\n", i, partTable.record[i].sid);
                if (fat_mount(dev, partTable.record[i].start, partTable.record[i].count) >= 0)
                    count++;
            }
        }

        if (count == 0) {
            printf("USBHDFSD: error - no mountable partitions.\n");
            return -1;
        }
    } else { /* No partition table detected, so try to use "floppy" option and hope for the best. */
        printf("USBHDFSD: mount drive\n");
        if (fat_mount(dev, 0, dev->maxLBA) < 0)
            return -1;
    }

    return 0;
}

//---------------------------------------------------------------------------
#if defined(BUILDING_USBHDFSD)
void part_disconnect(mass_dev *dev)
#elif defined(BUILDING_IEEE1394_DISK)
void part_disconnect(struct SBP2Device *dev)
#else
int part_connect(void *dev)
#endif
{
#ifdef BUILDING_USBHDFSD
    printf("USBHDFSD: part_disconnect devId %i \n", dev->devId);
#endif /* BUILDING_USBHDFSD */
#ifdef BUILDING_IEEE1394_DISK
    XPRINTF("part_disconnect devId %i \n", dev->nodeID);
#endif /* BUILDING_IEEE1394_DISK */
    fat_forceUnmount(dev);
}

//---------------------------------------------------------------------------
// End of file:  part_driver.c
//---------------------------------------------------------------------------

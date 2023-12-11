#include <errno.h>
#include <intrman.h>
#include <stdio.h>
#include <sysclib.h>
#include <sysmem.h>

// #define DEBUG

#include "iLinkman.h"
#include "sbp2_disk.h"
#include "scsi.h"

#include "usbhd_common.h"
#include "scache.h"
#include "part_driver.h"
#include "mass_debug.h"

/* Function prototypes */
static int scsiTestUnitReady(struct SBP2Device *dev);
static int scsiInquiry(struct SBP2Device *dev, void *buffer, int size);
static int scsiRequestSense(struct SBP2Device *dev, void *buffer, int size);
static int scsiStartStopUnit(struct SBP2Device *dev);
static int scsiReadCapacity(struct SBP2Device *dev, void *buffer, int size);

static int scsiTestUnitReady(struct SBP2Device *dev)
{
    struct CommandDescriptorBlock cdb;

    XPRINTF("scsiTestUnitReady.\n");

    cdb.misc = (u32)(ORB_NOTIFY | ORB_REQUEST_FORMAT(0) | CDB_MAX_PAYLOAD(dev->max_payload) | CDB_DIRECTION(WRITE_TRANSACTION) | CDB_SPEED(dev->speed));

    cdb.DataDescriptor.low = cdb.DataDescriptor.high = 0;
    cdb.DataDescriptor.NodeID                        = dev->InitiatorNodeID;

    cdb.NextOrb.high = cdb.NextOrb.low = 0;
    cdb.NextOrb.reserved               = NULL_POINTER;

    // scsi command packet
    cdb.CDBs[3]  = 0x00; // test unit ready operation code
    cdb.CDBs[2]  = 0;    // reserved
    cdb.CDBs[1]  = 0;    // reserved
    cdb.CDBs[0]  = 0;    // reserved
    cdb.CDBs[7]  = 0;    // reserved
    cdb.CDBs[6]  = 0;    // reserved
    cdb.CDBs[5]  = 0;    // reserved
    cdb.CDBs[4]  = 0;    // reserved
    cdb.CDBs[11] = 0;    // reserved
    cdb.CDBs[10] = 0;    // reserved
    cdb.CDBs[9]  = 0;    // reserved
    cdb.CDBs[8]  = 0;    // reserved

    ieee1394_SendCommandBlockORB(dev, &cdb);
    return (ieee1394_Sync());
}

static int scsiRequestSense(struct SBP2Device *dev, void *buffer, int size)
{
    int ret;
    struct CommandDescriptorBlock cdb;

    XPRINTF("scsiRequestSense.\n");

    cdb.misc = (ORB_NOTIFY | ORB_REQUEST_FORMAT(0) | CDB_MAX_PAYLOAD(dev->max_payload) | CDB_DIRECTION(WRITE_TRANSACTION) | CDB_SPEED(dev->speed) | CDB_DATA_SIZE(size));

    cdb.DataDescriptor.low    = (u32)buffer;
    cdb.DataDescriptor.high   = 0;
    cdb.DataDescriptor.NodeID = dev->InitiatorNodeID;

    cdb.NextOrb.high = cdb.NextOrb.low = 0;
    cdb.NextOrb.reserved               = NULL_POINTER;

    // scsi command packet
    cdb.CDBs[3]  = 0x03; // request sense operation code
    cdb.CDBs[2]  = 0;    // reserved
    cdb.CDBs[1]  = 0;    // reserved
    cdb.CDBs[0]  = 0;    // reserved
    cdb.CDBs[7]  = size; // allocation length
    cdb.CDBs[6]  = 0;    // reserved
    cdb.CDBs[5]  = 0;    // reserved
    cdb.CDBs[4]  = 0;    // reserved
    cdb.CDBs[11] = 0;    // reserved
    cdb.CDBs[10] = 0;    // reserved
    cdb.CDBs[9]  = 0;    // reserved
    cdb.CDBs[8]  = 0;    // reserved

    ieee1394_SendCommandBlockORB(dev, &cdb);
    ret = ieee1394_Sync();

    if (ret != 0) {
        XPRINTF("scsiRequestSense error: %d\n", ret);
    }

    return ret;
}

static int scsiInquiry(struct SBP2Device *dev, void *buffer, int size)
{
    int ret;
    struct CommandDescriptorBlock cdb;

    XPRINTF("scsiInquiry. buffer: %p\n", buffer);

    cdb.misc = (ORB_NOTIFY | ORB_REQUEST_FORMAT(0) | CDB_MAX_PAYLOAD(dev->max_payload) | CDB_DIRECTION(WRITE_TRANSACTION) | CDB_SPEED(dev->speed) | CDB_DATA_SIZE(size));

    cdb.DataDescriptor.low    = (u32)buffer;
    cdb.DataDescriptor.high   = 0;
    cdb.DataDescriptor.NodeID = dev->InitiatorNodeID;

    cdb.NextOrb.high = cdb.NextOrb.low = 0;
    cdb.NextOrb.reserved               = NULL_POINTER;

    // scsi command packet
    cdb.CDBs[3]  = 0x12; // inquiry operation code
    cdb.CDBs[2]  = 0;    // reserved
    cdb.CDBs[1]  = 0;    // page code
    cdb.CDBs[0]  = 0;    // reserved
    cdb.CDBs[7]  = size; // inquiry reply length
    cdb.CDBs[6]  = 0;    // reserved
    cdb.CDBs[5]  = 0;    // reserved
    cdb.CDBs[4]  = 0;    // reserved
    cdb.CDBs[11] = 0;    // reserved
    cdb.CDBs[10] = 0;    // reserved
    cdb.CDBs[9]  = 0;    // reserved
    cdb.CDBs[8]  = 0;    // reserved

    ieee1394_SendCommandBlockORB(dev, &cdb);
    ret = ieee1394_Sync();

    if (ret != 0) {
        XPRINTF("scsiInquiry error %d\n", ret);
    } else {
        unsigned int i;

        for (i = 0; i < (unsigned int)(size / 4); i++)
            ((unsigned int *)buffer)[i] = BSWAP32(((unsigned int *)buffer)[i]);
    }

    return ret;
}

static int scsiStartStopUnit(struct SBP2Device *dev)
{
    struct CommandDescriptorBlock cdb;

    XPRINTF("scsiStartStopUnit.\n");

    cdb.misc = (ORB_NOTIFY | ORB_REQUEST_FORMAT(0) | CDB_MAX_PAYLOAD(dev->max_payload) | CDB_DIRECTION(WRITE_TRANSACTION) | CDB_SPEED(dev->speed));

    cdb.DataDescriptor.low = cdb.DataDescriptor.high = 0;
    cdb.DataDescriptor.NodeID                        = dev->InitiatorNodeID;

    cdb.NextOrb.high = cdb.NextOrb.low = 0;
    cdb.NextOrb.reserved               = NULL_POINTER;

    // scsi command packet
    cdb.CDBs[3]  = 0x1B; // start stop unit operation code
    cdb.CDBs[2]  = 1;    // reserved/immed
    cdb.CDBs[1]  = 0;    // reserved
    cdb.CDBs[0]  = 0;    // reserved
    cdb.CDBs[7]  = 1;    // POWER CONDITIONS/reserved/LoEj/Start "Start the media and acquire the format type"
    cdb.CDBs[6]  = 0;    // reserved
    cdb.CDBs[5]  = 0;    // reserved
    cdb.CDBs[4]  = 0;    // reserved
    cdb.CDBs[11] = 0;    // reserved
    cdb.CDBs[10] = 0;    // reserved
    cdb.CDBs[9]  = 0;    // reserved
    cdb.CDBs[8]  = 0;    // reserved

    ieee1394_SendCommandBlockORB(dev, &cdb);
    return (ieee1394_Sync());
}

static int scsiReadCapacity(struct SBP2Device *dev, void *buffer, int size)
{
    struct CommandDescriptorBlock cdb;

    XPRINTF("scsiReadCapacity.\n");

    cdb.misc = (ORB_NOTIFY | ORB_REQUEST_FORMAT(0) | CDB_MAX_PAYLOAD(dev->max_payload) | CDB_DIRECTION(WRITE_TRANSACTION) | CDB_SPEED(dev->speed) | CDB_DATA_SIZE(size));

    cdb.DataDescriptor.low    = (u32)buffer;
    cdb.DataDescriptor.high   = 0;
    cdb.DataDescriptor.NodeID = dev->InitiatorNodeID;

    cdb.NextOrb.high = cdb.NextOrb.low = 0;
    cdb.NextOrb.reserved               = NULL_POINTER;

    // scsi command packet
    cdb.CDBs[3]  = 0x25; // read capacity operation code
    cdb.CDBs[2]  = 0;    // reserved
    cdb.CDBs[1]  = 0;    // LBA 1
    cdb.CDBs[0]  = 0;    // LBA 2
    cdb.CDBs[7]  = 0;    // LBA 3
    cdb.CDBs[6]  = 0;    // LBA 4
    cdb.CDBs[5]  = 0;    // Reserved
    cdb.CDBs[4]  = 0;    // Reserved
    cdb.CDBs[11] = 0;    // Reserved
    cdb.CDBs[10] = 0;    // reserved
    cdb.CDBs[9]  = 0;    // reserved
    cdb.CDBs[8]  = 0;    // reserved

    ieee1394_SendCommandBlockORB(dev, &cdb);
    return (ieee1394_Sync());
}

#define MAX_ORBS 256

static struct CommandDescriptorBlock cdb[MAX_ORBS];

int scsiReadSector(struct SBP2Device *dev, unsigned long int lba, void *buffer, int sectorCount)
{
    unsigned int nOrbs, OrbsRemaining, i, sectorsToRead, sectorsRemaining, SectorsPerBlock, BlockNum;
    unsigned long int startingLBA;
    void *bufferPtr, *PreviousReqBufferPtr;
    int result;

    XPRINTF("scsiReadSector. buffer: %p, lba: 0x%08lx, numsectors: %d.\n", buffer, lba, sectorCount);
    SectorsPerBlock = XFER_BLOCK_SIZE / dev->sectorSize;
    OrbsRemaining   = sectorCount / SectorsPerBlock;
    if ((sectorCount % SectorsPerBlock) != 0)
        OrbsRemaining++;

    XPRINTF("NumOrbs=%d cdb: %p.\n", OrbsRemaining, cdb);

    startingLBA      = lba;
    sectorsRemaining = sectorCount;
    bufferPtr        = buffer;

    result = 0;
    for (PreviousReqBufferPtr = NULL, BlockNum = 0, sectorsToRead = 0; (OrbsRemaining > 0) && (result == 0); OrbsRemaining -= nOrbs, BlockNum++) {
        nOrbs = (OrbsRemaining > MAX_ORBS) ? MAX_ORBS : OrbsRemaining;

        if (BlockNum > 0) { /* While waiting for the read request that was just sent to be processed, byte-swap the previously read in data. */
            for (i = 0; i < sectorsToRead << 7; i++)
                ((unsigned int *)PreviousReqBufferPtr)[i] = BSWAP32(((unsigned int *)PreviousReqBufferPtr)[i]);
            result = ieee1394_Sync(); /* Only sync if a read request is being processed. Otherwise, it'll wait eternally.. */
        }

        /* Save the current pointer to the buffer, so that the code above can byte-swap the payload of the request below that will be read in,
            when this loop loops around.
        */
        PreviousReqBufferPtr = bufferPtr;

        for (i = 0; i < nOrbs; i++) {
            sectorsToRead = (sectorsRemaining > SectorsPerBlock) ? SectorsPerBlock : sectorsRemaining;

            cdb[i].misc = (ORB_NOTIFY | ORB_REQUEST_FORMAT(0) | CDB_MAX_PAYLOAD(dev->max_payload) | CDB_DIRECTION(WRITE_TRANSACTION) | CDB_SPEED(dev->speed) | CDB_DATA_SIZE((dev->sectorSize * sectorsToRead)));

            cdb[i].DataDescriptor.low    = (u32)bufferPtr;
            cdb[i].DataDescriptor.high   = 0;
            cdb[i].DataDescriptor.NodeID = dev->InitiatorNodeID;

            if (i > 0) {
                cdb[i - 1].NextOrb.high     = 0;
                cdb[i - 1].NextOrb.low      = (u32)&cdb[i];
                cdb[i - 1].NextOrb.reserved = 0;
            }

            // scsi command packet
            cdb[i].CDBs[3]  = 0x28;                             // read operation code
            cdb[i].CDBs[2]  = 0;                                // Reserved
            cdb[i].CDBs[1]  = (startingLBA & 0xFF000000) >> 24; // lba 1 (MSB)
            cdb[i].CDBs[0]  = (startingLBA & 0xFF0000) >> 16;   // lba 2
            cdb[i].CDBs[7]  = (startingLBA & 0xFF00) >> 8;      // lba 3
            cdb[i].CDBs[6]  = (startingLBA & 0xFF);             // lba 4 (LSB)
            cdb[i].CDBs[5]  = 0;                                // Reserved
            cdb[i].CDBs[4]  = (sectorsToRead & 0xFF00) >> 8;    // Transfer length MSB
            cdb[i].CDBs[11] = (sectorsToRead & 0xFF);           // Transfer length LSB
            cdb[i].CDBs[10] = 0;                                // reserved
            cdb[i].CDBs[9]  = 0;                                // reserved
            cdb[i].CDBs[8]  = 0;                                // reserved

            startingLBA += sectorsToRead;
            sectorsRemaining -= sectorsToRead;
            bufferPtr = (void *)((u8 *)bufferPtr + (dev->sectorSize * sectorsToRead));
        }

        /* NULL terminate the last ORB. */
        cdb[i - 1].NextOrb.high = cdb[i - 1].NextOrb.low = 0;
        cdb[i - 1].NextOrb.reserved                      = NULL_POINTER;

        ieee1394_SendCommandBlockORB(dev, cdb);
    }

    /* Sync and byte-swap the last block. */
    result = ieee1394_Sync();
    for (i = 0; i < sectorsToRead << 7; i++)
        ((unsigned int *)PreviousReqBufferPtr)[i] = BSWAP32(((unsigned int *)PreviousReqBufferPtr)[i]);

    XPRINTF("scsiReadSector done.\n");

    return result;
}

int scsiWriteSector(struct SBP2Device *dev, unsigned long int lba, void *buffer, int sectorCount)
{
    unsigned int nOrbs, OrbsRemaining, i, sectorsToRead, sectorsRemaining, SectorsPerBlock;
    unsigned long int startingLBA;
    void *bufferPtr;
    int result, max_payload;

    XPRINTF("scsiWriteSector.\n");
    SectorsPerBlock = XFER_BLOCK_SIZE / dev->sectorSize;
    OrbsRemaining   = sectorCount / SectorsPerBlock;
    if ((sectorCount % SectorsPerBlock) != 0)
        OrbsRemaining++;

    startingLBA      = lba;
    sectorsRemaining = sectorCount;
    bufferPtr        = buffer;
    max_payload      = (dev->max_payload > 7) ? 7 : dev->max_payload; /* Do not allow the maximum payload to exceed 512 bytes (Actually, is this necessary?). */

    result = 0;
    for (; (OrbsRemaining > 0) && (result == 0); OrbsRemaining -= nOrbs) {
        nOrbs = (OrbsRemaining > MAX_ORBS) ? MAX_ORBS : OrbsRemaining;

        for (i = 0; i < nOrbs; i++) {
            sectorsToRead = (sectorsRemaining > SectorsPerBlock) ? SectorsPerBlock : sectorsRemaining;

            cdb[i].misc = (ORB_NOTIFY | ORB_REQUEST_FORMAT(0) | CDB_MAX_PAYLOAD(max_payload) | CDB_DIRECTION(READ_TRANSACTION) | CDB_SPEED(dev->speed) | CDB_DATA_SIZE((dev->sectorSize * sectorsToRead)));

            cdb[i].DataDescriptor.low    = (u32)bufferPtr;
            cdb[i].DataDescriptor.high   = 0;
            cdb[i].DataDescriptor.NodeID = dev->InitiatorNodeID;

            if (i > 0) {
                cdb[i - 1].NextOrb.high     = 0;
                cdb[i - 1].NextOrb.low      = (u32)&cdb[i];
                cdb[i - 1].NextOrb.reserved = 0;
            }

            // scsi command packet
            cdb[i].CDBs[3]  = 0x2A;                             // write operation code
            cdb[i].CDBs[2]  = 0;                                // Reserved
            cdb[i].CDBs[1]  = (startingLBA & 0xFF000000) >> 24; // lba 1 (MSB)
            cdb[i].CDBs[0]  = (startingLBA & 0xFF0000) >> 16;   // lba 2
            cdb[i].CDBs[7]  = (startingLBA & 0xFF00) >> 8;      // lba 3
            cdb[i].CDBs[6]  = (startingLBA & 0xFF);             // lba 4 (LSB)
            cdb[i].CDBs[5]  = 0;                                // Reserved
            cdb[i].CDBs[4]  = (sectorsToRead & 0xFF00) >> 8;    // Transfer length MSB
            cdb[i].CDBs[11] = (sectorsToRead & 0xFF);           // Transfer length LSB
            cdb[i].CDBs[10] = 0;                                // reserved
            cdb[i].CDBs[9]  = 0;                                // reserved
            cdb[i].CDBs[8]  = 0;                                // reserved

            startingLBA += sectorsToRead;
            sectorsRemaining -= sectorsToRead;
            bufferPtr = (void *)((u8 *)bufferPtr + (dev->sectorSize * sectorsToRead));
        }

        /* NULL terminate the last ORB. */
        cdb[i - 1].NextOrb.high = cdb[i - 1].NextOrb.low = 0;
        cdb[i - 1].NextOrb.reserved                      = NULL_POINTER;

        ieee1394_SendCommandBlockORB(dev, cdb);
        result = ieee1394_Sync();
    }

    return result;
}

static inline int InitializeSCSIDevice(struct SBP2Device *dev)
{
    inquiry_data id;
    sense_data sd;
    read_capacity_data rcd;
    int stat;

    XPRINTF("InitializeSCSIDevice.\n");

    memset(&id, 0, sizeof(inquiry_data));
    if ((stat = scsiInquiry(dev, &id, sizeof(inquiry_data))) < 0) {
        XPRINTF("Error - scsiInquiry returned %d.\n", stat);
        return -1;
    }

    printf("Vendor: %.8s\n", id.vendor);
    printf("Product: %.16s\n", id.product);
    printf("Revision: %.4s\n", id.revision);

    if ((stat = scsiReadCapacity(dev, &rcd, sizeof(read_capacity_data))) != 0) {
        XPRINTF("Error - scsiReadCapacity %d\n", stat);
        return -1;
    }

    dev->sectorSize = rcd.block_length;
    dev->maxLBA     = rcd.last_lba;

    printf("sectorSize %lu maxLBA %lu\n", dev->sectorSize, dev->maxLBA);

    while ((stat = scsiTestUnitReady(dev)) != 0) {
        XPRINTF("Error - scsiTestUnitReady %d\n", stat);

        stat = scsiRequestSense(dev, &sd, sizeof(sense_data));
        if (stat != 0) {
            XPRINTF("Error - scsiRequestSense %d\n", stat);
        }

        if ((sd.error_code == 0x70) && (sd.sense_key != 0x00)) {
            XPRINTF("Sense Data key: %02X code: %02X qual: %02X\n", sd.sense_key, sd.add_sense_code, sd.add_sense_qual);

            if ((sd.sense_key == 0x02) && (sd.add_sense_code == 0x04) && (sd.add_sense_qual == 0x02)) {
                XPRINTF("Error - Additional initalization is required for this device!\n");
                if ((stat = scsiStartStopUnit(dev)) != 0) {
                    XPRINTF("Error - scsiStartStopUnit %d\n", stat);
                    return -1;
                }
            }
        }
    }

    return 0;
}

void releaseSBP2Device(struct SBP2Device *dev)
{
    part_disconnect(dev);
    scache_kill(dev->cache);
}

int ConfigureSBP2Device(struct SBP2Device *dev)
{
    int ret;

    if ((ret = InitializeSCSIDevice(dev)) < 0) {
        XPRINTF("Error - failed to warmup device 0x%08x\n", dev->nodeID);
        return ret;
    }

    if ((dev->cache = scache_init(dev, dev->sectorSize)) == NULL) {
        XPRINTF("Error - scache_init failed \n");
        return ret;
    }

    return (part_connect(dev));
}

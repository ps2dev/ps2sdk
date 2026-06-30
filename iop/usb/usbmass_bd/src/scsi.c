#include <errno.h>
#include <stdio.h>
#include <sysclib.h>
#include <thsemap.h>

#include "scsi.h"
#include <bdm.h>

// #define DEBUG  //comment out this line when not debugging
#include "module_debug.h"

#define getBI32(__buf)   ((((u8 *)(__buf))[3] << 0) | (((u8 *)(__buf))[2] << 8) | (((u8 *)(__buf))[1] << 16) | (((u8 *)(__buf))[0] << 24))
#define SCSI_MAX_RETRIES 16

typedef struct _inquiry_data
{
    u8 peripheral_device_type; // 00h - Direct access (Floppy), 1Fh none (no FDD connected)
    u8 removable_media;        // 80h - removeable
    u8 iso_ecma_ansi;
    u8 response_data_format;
    u8 additional_length;
    u8 res[3];
    u8 vendor[8];
    u8 product[16];
    u8 revision[4];
} inquiry_data;

typedef struct _sense_data
{
    u8 error_code;
    u8 res1;
    u8 sense_key;
    u8 information[4];
    u8 add_sense_len;
    u8 res3[4];
    u8 add_sense_code;
    u8 add_sense_qual;
    u8 res4[4];
} sense_data;

typedef struct _read_capacity10_data
{
    u8 last_lba[4];
    u8 block_length[4];
} read_capacity10_data;
static_assert(sizeof(read_capacity10_data) == 8);

typedef struct _read_capacity16_data
{
    u8 last_lba_msb[4];
    u8 last_lba_lsb[4];
    u8 block_length[4];
    u8 args[4];
    u8 reserved[16];
} read_capacity16_data;
static_assert(sizeof(read_capacity16_data) == 32);

#define NUM_DEVICES 2
static struct block_device g_scsi_bd[NUM_DEVICES];

//
// Private Low level SCSI commands
//
static int scsi_cmd(struct block_device *bd, unsigned char cmd, void *buffer, int buf_size, int cmd_size)
{
    unsigned char comData[12]   = {0x00, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
    struct scsi_interface *scsi = (struct scsi_interface *)bd->priv;

    // M_DEBUG("%s\n", __func__);

    comData[0] = cmd;
    comData[4] = cmd_size;

    return scsi->queue_cmd(scsi, comData, sizeof(comData), buffer, buf_size, 0);
}

static inline int scsi_cmd_test_unit_ready(struct block_device *bd)
{
    M_DEBUG("%s\n", __func__);

    return scsi_cmd(bd, TEST_UNIT_READY, NULL, 0, 0);
}

static inline int scsi_cmd_request_sense(struct block_device *bd, void *buffer, int size)
{
    M_DEBUG("%s\n", __func__);

    return scsi_cmd(bd, REQUEST_SENSE, buffer, size, size);
}

static inline int scsi_cmd_inquiry(struct block_device *bd, void *buffer, int size)
{
    M_DEBUG("%s\n", __func__);

    return scsi_cmd(bd, INQUIRY, buffer, size, size);
}

static int scsi_cmd_start_stop_unit(struct block_device *bd, u8 param)
{
    M_DEBUG("%s\n", __func__);

    return scsi_cmd(bd, START_STOP_UNIT, NULL, 0, param);
}

static inline int scsi_cmd_read_capacity10(struct block_device *bd, void *buffer, int buf_size)
{
    M_DEBUG("%s\n", __func__);

    return scsi_cmd(bd, READ_CAPACITY_10, buffer, buf_size, 0);
}

static inline int scsi_cmd_read_capacity16(struct block_device *bd, void *buffer, int buf_size)
{
    unsigned char comData[16]   = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
    struct scsi_interface *scsi = (struct scsi_interface *)bd->priv;

    M_DEBUG("%s\n", __func__);

    comData[0] = SERVICE_ACTION_IN;
    comData[1] = 0x10;
    comData[13] = buf_size;

    return scsi->queue_cmd(scsi, comData, sizeof(comData), buffer, buf_size, 0);
}

static int scsi_cmd_rw_sector(struct block_device *bd, u64 lba, const void *buffer, unsigned short int sectorCount, unsigned int write)
{
    struct scsi_interface *scsi = (struct scsi_interface *)bd->priv;

    DEBUG_U64_2XU32(lba);
    M_DEBUG("scsi_cmd_rw_sector - 0x%08x%08x %p 0x%04x\n", lba_u32[1], lba_u32[0], buffer, sectorCount);

    if (lba <= 0xffffffffull)
    {
        unsigned char comData[12]   = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
        comData[0] = write ? WRITE_10 : READ_10;
        comData[2] = (lba >> 24) & 0xff;        // lba 1 (MSB)
        comData[3] = (lba >> 16) & 0xff;        // lba 2
        comData[4] = (lba >>  8) & 0xff;        // lba 3
        comData[5] = (lba >>  0) & 0xff;        // lba 4 (LSB)
        comData[7] = (sectorCount >> 8) & 0xff; // Transfer length MSB
        comData[8] = (sectorCount >> 0) & 0xff; // Transfer length LSB
        return scsi->queue_cmd(scsi, comData, sizeof(comData), (void *)buffer, bd->sectorSize * sectorCount, write);
    }
    else
    {
        unsigned char comData[16] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
        comData[0] = write ? WRITE_16 : READ_16;
        comData[2] = (lba >> 56) & 0xff;          // lba (MSB)
        comData[3] = (lba >> 48) & 0xff;          // lba
        comData[4] = (lba >> 40) & 0xff;          // lba
        comData[5] = (lba >> 32) & 0xff;          // lba
        comData[6] = (lba >> 24) & 0xff;          // lba
        comData[7] = (lba >> 16) & 0xff;          // lba
        comData[8] = (lba >>  8) & 0xff;          // lba
        comData[9] = (lba >>  0) & 0xff;          // lba (LSB)
        comData[10] = (sectorCount >> 24) & 0xff; // Transfer length MSB
        comData[11] = (sectorCount >> 16) & 0xff; // Transfer length
        comData[12] = (sectorCount >>  8) & 0xff; // Transfer length
        comData[13] = (sectorCount >>  0) & 0xff; // Transfer length LSB
        return scsi->queue_cmd(scsi, comData, sizeof(comData), (void *)buffer, bd->sectorSize * sectorCount, write);
    }
}

//
// Private
//
static int scsi_warmup(struct block_device *bd)
{
    struct scsi_interface *scsi = (struct scsi_interface *)bd->priv;
    inquiry_data id;
    sense_data sd;
    read_capacity10_data rc10d;
    read_capacity16_data rc16d;
    int stat;

    M_DEBUG("%s\n", __func__);

    stat = scsi->get_max_lun(scsi);
    M_DEBUG("scsi->get_max_lun %d\n", stat);

    memset(&id, 0, sizeof(inquiry_data));
    if ((stat = scsi_cmd_inquiry(bd, &id, sizeof(inquiry_data))) < 0) {
        M_PRINTF("ERROR: scsi_cmd_inquiry %d\n", stat);
        return -1;
    }

    M_PRINTF("Vendor: %.8s\n", id.vendor);
    M_PRINTF("Product: %.16s\n", id.product);
    M_PRINTF("Revision: %.4s\n", id.revision);

    while ((stat = scsi_cmd_test_unit_ready(bd)) != 0) {
        M_PRINTF("ERROR: scsi_cmd_test_unit_ready %d\n", stat);

        stat = scsi_cmd_request_sense(bd, &sd, sizeof(sense_data));
        if (stat != 0) {
            M_PRINTF("ERROR: scsi_cmd_request_sense %d\n", stat);
        }

        if ((sd.error_code == 0x70) && (sd.sense_key != 0x00)) {
            M_PRINTF("Sense Data key: %02X code: %02X qual: %02X\n", sd.sense_key, sd.add_sense_code, sd.add_sense_qual);

            if ((sd.sense_key == 0x02) && (sd.add_sense_code == 0x04) && (sd.add_sense_qual == 0x02)) {
                M_PRINTF("ERROR: Additional initalization is required for this device!\n");
                if ((stat = scsi_cmd_start_stop_unit(bd, 1)) != 0) {
                    M_PRINTF("ERROR: scsi_cmd_start_stop_unit %d\n", stat);
                    return -1;
                }
            }
        }
    }

    memset(&rc10d, 0, sizeof(read_capacity10_data));
    if ((stat = scsi_cmd_read_capacity10(bd, &rc10d, sizeof(read_capacity10_data))) != 0) {
        M_PRINTF("ERROR: scsi_cmd_read_capacity10 %d\n", stat);
        return -1;
    }

    bd->sectorCount  = getBI32(&rc10d.last_lba) & 0xffffffff;
    bd->sectorSize   = getBI32(&rc10d.block_length);
    bd->sectorOffset = 0;

    u64 sectorCount = bd->sectorCount;
    U64_2XU32(sectorCount);
    M_PRINTF("rc10 = 0x%08x%08x %u-byte logical blocks: (%lu MB / %lu MiB)\n", sectorCount_u32[1], sectorCount_u32[0], bd->sectorSize,
        (u32)(bd->sectorCount / ((1000 * 1000) / bd->sectorSize)), (u32)(bd->sectorCount / ((1024 * 1024) / bd->sectorSize)));

    // Cannot rely exclusively on READ_CAPACITY16 because it causes issues on smaller/older disks:
    // - Kingmax Super Stick 4GB stops responding after receiving RC16
    // - Kingmax Super Stick 8GB returns success, but the last LBA value is garbage (ff<<48)
    // Fingers crossed - hopefully this approach works
    if (bd->sectorCount == 0xffffffffull)
    {
        memset(&rc16d, 0, sizeof(read_capacity16_data));
        if (scsi_cmd_read_capacity16(bd, &rc16d, sizeof(read_capacity16_data)) == 0) {
            bd->sectorCount  = ((u64)getBI32(&rc16d.last_lba_msb) << 32) | (getBI32(&rc16d.last_lba_lsb) & 0xffffffff);
            bd->sectorSize   = getBI32(&rc16d.block_length);
            bd->sectorOffset = 0;

            sectorCount = bd->sectorCount;
            U64_2XU32(sectorCount);
            M_PRINTF("rc16 = 0x%08x%08x %u-byte logical blocks: (%lu MB / %lu MiB)\n", sectorCount_u32[1], sectorCount_u32[0], bd->sectorSize,
                (u32)(bd->sectorCount / ((1000 * 1000) / bd->sectorSize)), (u32)(bd->sectorCount / ((1024 * 1024) / bd->sectorSize)));
        }
    }

    return 0;
}

//
// Block device interface
//
static int scsi_read(struct block_device *bd, u64 sector, void *buffer, u16 count)
{
    struct scsi_interface *scsi = (struct scsi_interface *)bd->priv;
    u16 sc_remaining            = count;
    int retries;

    DEBUG_U64_2XU32(sector);
    M_DEBUG("%s: sector=0x%08x%08x, count=%d\n", __func__, sector_u32[1], sector_u32[0], count);

    while (sc_remaining > 0) {
        u16 sc = sc_remaining > scsi->max_sectors ? scsi->max_sectors : sc_remaining;

        for (retries = SCSI_MAX_RETRIES; retries > 0; retries--) {
            if (scsi_cmd_rw_sector(bd, sector, buffer, sc, 0) == 0)
                break;
        }

        if (retries == 0) {
            U64_2XU32(sector);
            M_PRINTF("ERROR: unable to read sector after %d tries (sector=0x%08x%08x, count=%d)\n", SCSI_MAX_RETRIES, sector_u32[1], sector_u32[0], count);
            return -EIO;
        }

        sc_remaining -= sc;
        sector += sc;
        buffer = (u8 *)buffer + (sc * bd->sectorSize);
    }

    return count;
}

static int scsi_write(struct block_device *bd, u64 sector, const void *buffer, u16 count)
{
    struct scsi_interface *scsi = (struct scsi_interface *)bd->priv;
    u16 sc_remaining            = count;
    unsigned int sectorSize     = bd->sectorSize;
    void *misalign_buffer       = NULL;
    int retries;

    DEBUG_U64_2XU32(sector);
    M_DEBUG("%s: sector=0x%08x%08x, count=%d\n", __func__, sector_u32[1], sector_u32[0], count);

    if (((uiptr)buffer & 3) != 0) {
        /* Slow misalignment workaround */
        misalign_buffer = __builtin_alloca(sectorSize + 4);

        if (((uiptr)misalign_buffer & 3) != 0)
        {
            misalign_buffer = (u8 *)misalign_buffer + (4 - ((uiptr)misalign_buffer & 3));
        }
    }

    while (sc_remaining > 0) {
        u16 sc = sc_remaining > scsi->max_sectors ? scsi->max_sectors : sc_remaining;
        const void *dst_buffer = buffer;

        if (misalign_buffer != NULL) {
            memcpy(misalign_buffer, buffer, sectorSize);
            dst_buffer = misalign_buffer;
            sc = 1;
        }

        for (retries = SCSI_MAX_RETRIES; retries > 0; retries--) {
            if (scsi_cmd_rw_sector(bd, sector, dst_buffer, sc, 1) == 0)
                break;
        }

        if (retries == 0) {
            U64_2XU32(sector);
            M_PRINTF("ERROR: unable to write sector after %d tries (sector=0x%08x%08x, count=%d)\n", SCSI_MAX_RETRIES, sector_u32[1], sector_u32[0], count);
            return -EIO;
        }

        sc_remaining -= sc;
        sector += sc;
        buffer = (u8 *)buffer + (sc * sectorSize);
    }

    return count;
}

static void scsi_flush(struct block_device *bd)
{
    (void)bd;

    M_DEBUG("%s\n", __func__);

    // Dummy function
}

static int scsi_stop(struct block_device *bd)
{
    int stat;

    M_DEBUG("%s\n", __func__);

    if ((stat = scsi_cmd_start_stop_unit(bd, 0)) != 0) {
        M_PRINTF("ERROR: scsi_cmd_start_stop_unit %d\n", stat);
    }

    return stat;
}

//
// Public functions
//
void scsi_connect(struct scsi_interface *scsi)
{
    int i;

    M_DEBUG("%s\n", __func__);

    for (i = 0; i < NUM_DEVICES; ++i) {
        if (g_scsi_bd[i].priv == NULL) {
            struct block_device *bd = &g_scsi_bd[i];

            bd->priv = scsi;
            bd->name = scsi->name;
            bd->path = "usb";
            bd->devNr = scsi->devNr;
            scsi_warmup(bd);
            bdm_connect_bd(bd);
            break;
        }
    }
}

void scsi_disconnect(struct scsi_interface *scsi)
{
    int i;

    M_DEBUG("%s\n", __func__);

    for (i = 0; i < NUM_DEVICES; ++i) {
        if (g_scsi_bd[i].priv == scsi) {
            struct block_device *bd = &g_scsi_bd[i];
            bdm_disconnect_bd(bd);
            bd->priv = NULL;
            break;
        }
    }
}

int scsi_init(void)
{
    int i;

    M_DEBUG("%s\n", __func__);

    for (i = 0; i < NUM_DEVICES; ++i) {
        g_scsi_bd[i].parNr = 0;
        g_scsi_bd[i].parId = 0x00;

        g_scsi_bd[i].priv  = NULL;
        g_scsi_bd[i].read  = scsi_read;
        g_scsi_bd[i].write = scsi_write;
        g_scsi_bd[i].flush = scsi_flush;
        g_scsi_bd[i].stop  = scsi_stop;
    }

    return 0;
}

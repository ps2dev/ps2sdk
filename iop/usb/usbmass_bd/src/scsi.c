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

typedef struct _read_capacity_data
{
    u8 last_lba[4];
    u8 block_length[4];
} read_capacity_data;

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

    return scsi->queue_cmd(scsi, comData, 12, buffer, buf_size, 0);
}

static inline int scsi_cmd_test_unit_ready(struct block_device *bd)
{
    M_DEBUG("%s\n", __func__);

    return scsi_cmd(bd, 0x00, NULL, 0, 0);
}

static inline int scsi_cmd_request_sense(struct block_device *bd, void *buffer, int size)
{
    M_DEBUG("%s\n", __func__);

    return scsi_cmd(bd, 0x03, buffer, size, size);
}

static inline int scsi_cmd_inquiry(struct block_device *bd, void *buffer, int size)
{
    M_DEBUG("%s\n", __func__);

    return scsi_cmd(bd, 0x12, buffer, size, size);
}

static int scsi_cmd_start_stop_unit(struct block_device *bd, u8 param)
{
    M_DEBUG("%s\n", __func__);

    return scsi_cmd(bd, 0x1b, NULL, 0, param);
}

static inline int scsi_cmd_read_capacity(struct block_device *bd, void *buffer, int size)
{
    M_DEBUG("%s\n", __func__);

    return scsi_cmd(bd, 0x25, buffer, size, 0);
}

static int scsi_cmd_rw_sector(struct block_device *bd, u64 lba, const void *buffer, unsigned short int sectorCount, unsigned int write)
{
    unsigned char comData[12]   = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
    struct scsi_interface *scsi = (struct scsi_interface *)bd->priv;

    M_DEBUG("scsi_cmd_rw_sector - 0x%08x%08x %p 0x%04x\n", U64_2XU32(&lba), buffer, sectorCount);

    // Note: LBA from bdm is 64bit but SCSI commands being used are 32bit. These need to be updated to 64bit LBA SCSI
    // commands to work with large capacity drives. For now the 32bit LBA will only support up to 2TB drives.

    comData[0] = write ? 0x2a : 0x28;
    comData[2] = (lba & 0xFF000000) >> 24;    // lba 1 (MSB)
    comData[3] = (lba & 0xFF0000) >> 16;      // lba 2
    comData[4] = (lba & 0xFF00) >> 8;         // lba 3
    comData[5] = (lba & 0xFF);                // lba 4 (LSB)
    comData[7] = (sectorCount & 0xFF00) >> 8; // Transfer length MSB
    comData[8] = (sectorCount & 0xFF);        // Transfer length LSB
    return scsi->queue_cmd(scsi, comData, 12, (void *)buffer, bd->sectorSize * sectorCount, write);
}

//
// Private
//
static int scsi_warmup(struct block_device *bd)
{
    struct scsi_interface *scsi = (struct scsi_interface *)bd->priv;
    inquiry_data id;
    sense_data sd;
    read_capacity_data rcd;
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

    if ((stat = scsi_cmd_read_capacity(bd, &rcd, sizeof(read_capacity_data))) != 0) {
        M_PRINTF("ERROR: scsi_cmd_read_capacity %d\n", stat);
        return -1;
    }

    bd->sectorSize   = getBI32(&rcd.block_length);
    bd->sectorOffset = 0;
    bd->sectorCount  = getBI32(&rcd.last_lba);
    M_PRINTF("%u %u-byte logical blocks: (%uMB / %uMiB)\n", bd->sectorCount, bd->sectorSize, bd->sectorCount / ((1000 * 1000) / bd->sectorSize), bd->sectorCount / ((1024 * 1024) / bd->sectorSize));

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

    M_DEBUG("%s: sector=0x%08x%08x, count=%d\n", __func__, U64_2XU32(&sector), count);

    while (sc_remaining > 0) {
        u16 sc = sc_remaining > scsi->max_sectors ? scsi->max_sectors : sc_remaining;

        for (retries = SCSI_MAX_RETRIES; retries > 0; retries--) {
            if (scsi_cmd_rw_sector(bd, sector, buffer, sc, 0) == 0)
                break;
        }

        if (retries == 0) {
            M_PRINTF("ERROR: unable to read sector after %d tries (sector=0x%08x%08x, count=%d)\n", SCSI_MAX_RETRIES, U64_2XU32(&sector), count);
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
    int retries;

    M_DEBUG("%s: sector=0x%08x%08x, count=%d\n", __func__, U64_2XU32(&sector), count);

    while (sc_remaining > 0) {
        u16 sc = sc_remaining > scsi->max_sectors ? scsi->max_sectors : sc_remaining;

        for (retries = SCSI_MAX_RETRIES; retries > 0; retries--) {
            if (scsi_cmd_rw_sector(bd, sector, buffer, sc, 1) == 0)
                break;
        }

        if (retries == 0) {
            M_PRINTF("ERROR: unable to write sector after %d tries (sector=0x%08x%08x, count=%d)\n", SCSI_MAX_RETRIES, U64_2XU32(&sector), count);
            return -EIO;
        }

        sc_remaining -= sc;
        sector += sc;
        buffer = (u8 *)buffer + (sc * bd->sectorSize);
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
        g_scsi_bd[i].devNr = i;
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

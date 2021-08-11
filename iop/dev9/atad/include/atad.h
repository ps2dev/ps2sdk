/*
# _____     ___ ____     ___ ____
#  ____|   |    ____|   |        | |____|
# |     ___|   |____ ___|    ____| |    \    PS2DEV Open Source Project.
#-----------------------------------------------------------------------
# Copyright (c) 2003 Marcus R. Brown <mrbrown@0xd6.org>
# Licenced under Academic Free License version 2.0
# Review ps2sdk README & LICENSE files for further details.
*/

/**
 * @file
 * ATA Device Driver definitions and imports.
 */

#ifndef __ATAD_H__
#define __ATAD_H__

#include <types.h>
#include <irx.h>

/* These are used with the dir parameter of ata_device_sector_io().  */
#define ATA_DIR_READ  0
#define ATA_DIR_WRITE 1

typedef struct _ata_devinfo
{
    /** Was successfully probed.  */
    s32 exists;
    /** Supports the PACKET command set.  */
    s32 has_packet;
    /** Total number of user sectors.  */
    u32 total_sectors;
    /** Word 0x100 of the identify info.  */
    u32 security_status;
    /** Supports the 48-bit LBA command set (unofficial).  */
    u32 lba48;
    /** Total number of 48-bit LBA user sectors (unofficial). */
    u32 total_sectors_lba48;
} ata_devinfo_t;

/* Error definitions.  */
#define ATA_RES_ERR_NOTREADY -501
#define ATA_RES_ERR_TIMEOUT  -502
#define ATA_RES_ERR_IO       -503
#define ATA_RES_ERR_NODATA   -504
#define ATA_RES_ERR_NODEV    -505
#define ATA_RES_ERR_CMD      -506
#define ATA_RES_ERR_LOCKED   -509
#define ATA_RES_ERR_ICRC     -510

ata_devinfo_t *ata_get_devinfo(int device);

int ata_reset_devices(void);

int ata_io_start(void *buf, u32 blkcount, u16 feature, u16 nsector, u16 sector, u16 lcyl, u16 hcyl, u16 select, u16 command);
int ata_io_finish(void);

int ata_get_error(void);

#define ata_device_dma_transfer ata_device_sector_io // Backward-compatibility
int ata_device_sector_io(int device, void *buf, u32 lba, u32 nsectors, int dir);

// DRM functions that were meant to keep users from sharing disks (and hence the contained content). Supported by only Sony-modified HDDs (e.g. the SCPH-20400).
int ata_device_sce_sec_set_password(int device, void *password);
int ata_device_sce_sec_unlock(int device, void *password);
int ata_device_sce_sec_erase(int device);

int ata_device_idle(int device, int period);
int ata_device_sce_identify_drive(int device, void *data);
int ata_device_smart_get_status(int device);
int ata_device_smart_save_attr(int device);
int ata_device_flush_cache(int device);
int ata_device_idle_immediate(int device);

#define atad_IMPORTS_start DECLARE_IMPORT_TABLE(atad, 1, 3)
#define atad_IMPORTS_end   END_IMPORT_TABLE

#define I_ata_get_devinfo                 DECLARE_IMPORT(4, ata_get_devinfo)
#define I_ata_reset_devices               DECLARE_IMPORT(5, ata_reset_devices)
#define I_ata_io_start                    DECLARE_IMPORT(6, ata_io_start)
#define I_ata_io_finish                   DECLARE_IMPORT(7, ata_io_finish)
#define I_ata_get_error                   DECLARE_IMPORT(8, ata_get_error)
#define I_ata_device_dma_transfer         I_ata_device_sector_io // Backward-compatibility
#define I_ata_device_sector_io            DECLARE_IMPORT(9, ata_device_sector_io)
#define I_ata_device_sce_sec_set_password DECLARE_IMPORT(10, ata_device_sce_sec_set_password)
#define I_ata_device_sce_sec_unlock       DECLARE_IMPORT(11, ata_device_sce_sec_unlock)
#define I_ata_device_sce_sec_erase        DECLARE_IMPORT(12, ata_device_sce_sec_erase)
#define I_ata_device_idle                 DECLARE_IMPORT(13, ata_device_idle)
#define I_ata_device_sce_identify_drive   DECLARE_IMPORT(14, ata_device_sce_identify_drive)
#define I_ata_device_smart_get_status     DECLARE_IMPORT(15, ata_device_smart_get_status)
#define I_ata_device_smart_save_attr      DECLARE_IMPORT(16, ata_device_smart_save_attr)
#define I_ata_device_flush_cache          DECLARE_IMPORT(17, ata_device_flush_cache)
#define I_ata_device_idle_immediate       DECLARE_IMPORT(18, ata_device_idle_immediate)

#endif /* __ATAD_H__ */

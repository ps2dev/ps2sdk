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

extern ata_devinfo_t *sceAtaInit(int device);

extern int sceAtaSoftReset(void);

extern int sceAtaExecCmd(void *buf, u32 blkcount, u16 feature, u16 nsector, u16 sector, u16 lcyl, u16 hcyl, u16 select, u16 command);
extern int sceAtaWaitResult(void);

extern int sceAtaGetError(void);

extern int sceAtaDmaTransfer(int device, void *buf, u32 lba, u32 nsectors, int dir);

// DRM functions that were meant to keep users from sharing disks (and hence the contained content). Supported by only Sony-modified HDDs (e.g. the SCPH-20400).
extern int sceAtaSecuritySetPassword(int device, void *password);
extern int sceAtaSecurityUnLock(int device, void *password);
extern int sceAtaSecurityEraseUnit(int device);

extern int sceAtaIdle(int device, int period);
extern int sceAtaGetSceId(int device, void *data);
extern int sceAtaSmartReturnStatus(int device);
extern int sceAtaSmartSaveAttr(int device);
extern int sceAtaFlushCache(int device);
extern int sceAtaIdleImmediate(int device);

extern int ata_device_sector_io64(int device, void *buf, u64 lba, u32 nsectors, int dir);

#define atad_IMPORTS_start DECLARE_IMPORT_TABLE(atad, 1, 3)
#define atad_IMPORTS_end   END_IMPORT_TABLE

#define I_sceAtaInit                      DECLARE_IMPORT(4, sceAtaInit)
#define I_sceAtaSoftReset                 DECLARE_IMPORT(5, sceAtaSoftReset)
#define I_sceAtaExecCmd                   DECLARE_IMPORT(6, sceAtaExecCmd)
#define I_sceAtaWaitResult                DECLARE_IMPORT(7, sceAtaWaitResult)
#define I_sceAtaGetError                  DECLARE_IMPORT(8, sceAtaGetError)
#define I_sceAtaDmaTransfer               DECLARE_IMPORT(9, sceAtaDmaTransfer)
#define I_sceAtaSecuritySetPassword       DECLARE_IMPORT(10, sceAtaSecuritySetPassword)
#define I_sceAtaSecurityUnLock            DECLARE_IMPORT(11, sceAtaSecurityUnLock)
#define I_sceAtaSecurityEraseUnit         DECLARE_IMPORT(12, sceAtaSecurityEraseUnit)
#define I_sceAtaIdle                      DECLARE_IMPORT(13, sceAtaIdle)
#define I_sceAtaGetSceId                  DECLARE_IMPORT(14, sceAtaGetSceId)
#define I_sceAtaSmartReturnStatus         DECLARE_IMPORT(15, sceAtaSmartReturnStatus)
#define I_sceAtaSmartSaveAttr             DECLARE_IMPORT(16, sceAtaSmartSaveAttr)
#define I_sceAtaFlushCache                DECLARE_IMPORT(17, sceAtaFlushCache)
#define I_sceAtaIdleImmediate             DECLARE_IMPORT(18, sceAtaIdleImmediate)
#define I_ata_device_sector_io64          DECLARE_IMPORT(19, ata_device_sector_io64)

// Backward-compatibility definitions
#define ata_get_devinfo sceAtaInit
#define ata_reset_devices sceAtaSoftReset
#define ata_io_start sceAtaExecCmd
#define ata_io_finish sceAtaWaitResult
#define ata_get_error sceAtaGetError
#define ata_device_sector_io sceAtaDmaTransfer
#define ata_device_sce_sec_set_password sceAtaSecuritySetPassword
#define ata_device_sce_sec_unlock sceAtaSecurityUnLock
#define ata_device_sce_sec_erase sceAtaSecurityEraseUnit
#define ata_device_idle sceAtaIdle
#define ata_device_sce_identify_drive sceAtaGetSceId
#define ata_device_smart_get_status sceAtaSmartReturnStatus
#define ata_device_smart_save_attr sceAtaSmartSaveAttr
#define ata_device_flush_cache sceAtaFlushCache
#define ata_device_idle_immediate sceAtaIdleImmediate
#define ata_device_dma_transfer sceAtaDmaTransfer

#define I_ata_get_devinfo I_sceAtaInit
#define I_ata_reset_devices I_sceAtaSoftReset
#define I_ata_io_start I_sceAtaExecCmd
#define I_ata_io_finish I_sceAtaWaitResult
#define I_ata_get_error I_sceAtaGetError
#define I_ata_device_sector_io I_sceAtaDmaTransfer
#define I_ata_device_sce_sec_set_password I_sceAtaSecuritySetPassword
#define I_ata_device_sce_sec_unlock I_sceAtaSecurityUnLock
#define I_ata_device_sce_sec_erase I_sceAtaSecurityEraseUnit
#define I_ata_device_idle I_sceAtaIdle
#define I_ata_device_sce_identify_drive I_sceAtaGetSceId
#define I_ata_device_smart_get_status I_sceAtaSmartReturnStatus
#define I_ata_device_smart_save_attr I_sceAtaSmartSaveAttr
#define I_ata_device_flush_cache I_sceAtaFlushCache
#define I_ata_device_idle_immediate I_sceAtaIdleImmediate
#define I_ata_device_dma_transfer I_sceAtaDmaTransfer

#endif /* __ATAD_H__ */

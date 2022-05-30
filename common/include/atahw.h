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
 * ATA hardware types and definitions.
 */

#ifndef __ATAHW_H__
#define __ATAHW_H__

#include <tamtypes.h>
#include <aifregs.h>
#include <speedregs.h>

#define ATA_DEV9_HDD_BASE (SPD_REGBASE + 0x40)
/** AIF on T10Ks - supported with aifatad.  */
#define ATA_AIF_HDD_BASE  (AIF_REGBASE + AIF_ATA)

/** A port contains all of the ATA controller registers.  */
typedef struct _ata_hwport
{
    u16 r_data;  /* 00 */
    u16 r_error; /* 02 */
#define r_feature r_error
    u16 r_nsector; /* 04 */
    u16 r_sector;  /* 06 */
    u16 r_lcyl;    /* 08 */
    u16 r_hcyl;    /* 0a */
    u16 r_select;  /* 0c */
    u16 r_status;  /* 0e */
#define r_command r_status
    u16 pad[6];
    u16 r_control; /* 1c */
} ata_hwport_t;

#define USE_ATA_REGS volatile ata_hwport_t *ata_hwport = \
                         (volatile ata_hwport_t *)ATA_DEV9_HDD_BASE

#define USE_AIF_ATA_REGS volatile ata_hwport_t *ata_hwport = \
                             (volatile ata_hwport_t *)ATA_AIF_HDD_BASE

/* r_error bits.  */
#define ATA_ERR_MARK   0x01
#define ATA_ERR_TRACK0 0x02
#define ATA_ERR_ABORT  0x04
#define ATA_ERR_MCR    0x08
#define ATA_ERR_ID     0x10
#define ATA_ERR_MC     0x20
#define ATA_ERR_ECC    0x40
#define ATA_ERR_ICRC   0x80

/* r_status bits.  */
#define ATA_STAT_ERR   0x01
#define ATA_STAT_INDEX 0x02
#define ATA_STAT_ECC   0x04
#define ATA_STAT_DRQ   0x08
#define ATA_STAT_SEEK  0x10
#define ATA_STAT_WRERR 0x20
#define ATA_STAT_READY 0x40
#define ATA_STAT_BUSY  0x80

/* r_select bits.  */
#define ATA_SEL_LBA 0x40

/** ATA command codes.  */
enum ATA_C_CODES {
    ATA_C_NOP                             = 0x00,
    ATA_C_CFA_REQUEST_EXTENDED_ERROR_CODE = 0x03,
    ATA_C_DEVICE_RESET                    = 0x08,
    ATA_C_READ_SECTOR                     = 0x20,
    ATA_C_READ_SECTOR_EXT                 = 0x24,
    ATA_C_READ_DMA_EXT,
    ATA_C_READ_NATIVE_MAX_ADDRESS_EXT = 0x27,
    ATA_C_READ_MULTIPLE_EXT           = 0x29,
    ATA_C_WRITE_SECTOR                = 0x30,
    ATA_C_WRITE_LONG                  = 0x32,
    ATA_C_WRITE_SECTOR_EXT            = 0x34,
    ATA_C_WRITE_DMA_EXT,
    ATA_C_SET_MAX_ADDRESS_EXT             = 0x37,
    ATA_C_CFA_WRITE_SECTORS_WITHOUT_ERASE = 0x38,
    ATA_C_WRITE_MULTIPLE_EXT              = 0x39,
    ATA_C_READ_VERIFY_SECTOR              = 0x40,
    ATA_C_READ_VERIFY_SECTOR_EXT          = 0x42,
    ATA_C_SEEK                            = 0x70,
    ATA_C_CFA_TRANSLATE_SECTOR            = 0x87,
    ATA_C_SCE_SECURITY_CONTROL            = 0x8e,
    ATA_C_EXECUTE_DEVICE_DIAGNOSTIC       = 0x90,
    ATA_C_INITIALIZE_DEVICE_PARAMETERS    = 0x91,
    ATA_C_DOWNLOAD_MICROCODE              = 0x92,
    ATA_C_PACKET                          = 0xa0,
    ATA_C_IDENTIFY_PACKET_DEVICE,
    ATA_C_SERVICE,
    ATA_C_SMART             = 0xb0,
    ATA_C_CFA_ERASE_SECTORS = 0xc0,
    ATA_C_READ_MULTIPLE     = 0xc4,
    ATA_C_WRITE_MULTIPLE,
    ATA_C_SET_MULTIPLE_MODE,
    ATA_C_READ_DMA_QUEUED,
    ATA_C_READ_DMA,
    ATA_C_WRITE_DMA        = 0xca,
    ATA_C_WRITE_DMA_QUEUED = 0xcc,
    ATA_C_CFA_WRITE_MULTIPLE_WITHOUT_ERASE,
    ATA_C_GET_MEDIA_STATUS = 0xda,
    ATA_C_MEDIA_LOCK       = 0xde,
    ATA_C_MEDIA_UNLOCK,
    ATA_C_STANDBY_IMMEDIATE = 0xe0,
    ATA_C_IDLE_IMMEDIATE,
    ATA_C_STANDBY,
    ATA_C_IDLE,
    ATA_C_READ_BUFFER,
    ATA_C_CHECK_POWER_MODE,
    ATA_C_SLEEP,
    ATA_C_FLUSH_CACHE,
    ATA_C_WRITE_BUFFER,
    ATA_C_FLUSH_CACHE_EXT = 0xea,
    ATA_C_IDENTIFY_DEVICE = 0xec,
    ATA_C_MEDIA_EJECT,

    ATA_C_SET_FEATURES = 0xef,

    ATA_C_SECURITY_SET_PASSWORD = 0xf1,
    ATA_C_SECURITY_UNLOCK,
    ATA_C_SECURITY_ERASE_PREPARE,
    ATA_C_SECURITY_ERASE_UNIT,
    ATA_C_SECURITY_FREEZE_LOCK,
    ATA_C_SECURITY_DISABLE_PASSWORD,

    ATA_C_READ_NATIVE_MAX_ADDRESS = 0xf8,
    ATA_C_SET_MAX_ADDRESS,
};

enum ATA_SCE_SECURITY_CODES {
    ATA_SCE_IDENTIFY_DRIVE = 0xec,

    ATA_SCE_SECURITY_SET_PASSWORD = 0xf1,
    ATA_SCE_SECURITY_UNLOCK,
    ATA_SCE_SECURITY_ERASE_PREPARE,
    ATA_SCE_SECURITY_ERASE_UNIT,
    ATA_SCE_SECURITY_FREEZE_LOCK,
    ATA_SCE_SECURITY_READ_ID  = 0x20,
    ATA_SCE_SECURITY_WRITE_ID = 0x30,
};

enum ATA_S_SMART_CODES {
    ATA_S_SMART_READ_DATA               = 0xd0,
    ATA_S_SMART_ENABLE_DISABLE_AUTOSAVE = 0xd2,
    ATA_S_SMART_SAVE_ATTRIBUTE_VALUES,
    ATA_S_SMART_EXECUTE_OFF_LINE,
    ATA_S_SMART_READ_LOG,
    ATA_S_SMART_WRITE_LOG,
    ATA_S_SMART_ENABLE_OPERATIONS = 0xd8,
    ATA_S_SMART_DISABLE_OPERATIONS,
    ATA_S_SMART_RETURN_STATUS
};

/** Offsets for the data returned from IDENTIFY DEVICE commands.  */
enum _ata_identify_offsets {
    ATA_ID_SECTOTAL_LO            = 60,
    ATA_ID_SECTOTAL_HI            = 61,
    ATA_ID_COMMAND_SETS_SUPPORTED = 83,
    ATA_ID_48BIT_SECTOTAL_LO      = 100,
    ATA_ID_48BIT_SECTOTAL_MI      = 101,
    ATA_ID_48BIT_SECTOTAL_HI      = 102,
    ATA_ID_SECURITY_STATUS        = 128
};

/* Bits in the security status word.  */
#define ATA_F_SEC_ENABLED (1 << 1)
#define ATA_F_SEC_LOCKED  (1 << 2)

#endif /* __ATAHW_H__ */

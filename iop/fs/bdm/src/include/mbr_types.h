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
 * MBR partition definitions.
 */

#ifndef _MBR_TYPES_H
#define _MBR_TYPES_H

#include <types.h>

#define MBR_BOOT_SIGNATURE                      0xAA55

#define MBR_PART_TYPE_GPT_PROTECTIVE_MBR        0xEE    // Indicates the MBR is 'protective' only and the actual format is GPT

// See: https://en.wikipedia.org/wiki/Master_boot_record#PTE
typedef struct __attribute__((packed)) _mbr_partition_entry
{
    /* 0x00 */ u8 status;                           // Status or physical drive (bit 7 set is for active or bootable, old MBRs only accept 0x80, 0x00 means inactive, and 0x01–0x7F stand for invalid)
    /* 0x01 */ u8 chs_first_lba[3];                 // CHS address of first absolute sector in partition
    /* 0x04 */ u8 partition_type;                   // Type of partition, see: https://en.wikipedia.org/wiki/Partition_type
    /* 0x05 */ u8 chs_last_lba[3];                  // CHS address of last absolute sector in partition
    /* 0x08 */ u32 first_lba;                       // LBA of first absolute sector in the partition
    /* 0x0C */ u32 sector_count;                    // Number of sectors in partition
} mbr_partition_entry;

// See: https://en.wikipedia.org/wiki/Master_boot_record
typedef struct __attribute__((packed)) _master_boot_record
{
    /* 0x00 */ u8 bootstrap_code1[218];                     // Bootstrap code area (part 1)
    /* 0xDA */ u8 disk_timestamp[6];                        // Disk timestamp (optional; Windows 95B/98/98SE/ME (MS-DOS 7.1–8.0). Alternatively, can serve as OEM loader signature with NEWLDR)
    /* 0xE0 */ u8 bootstrap_code2[216];                     // Bootstrap code area (part 2, code entry at 0x0000)
    /* 0x1B8 */ u32 disk_signature;                         // Disk signature (optional; UEFI, Linux, Windows NT family and other OSes)
    /* 0x1BC */ u16 copy_protection;                        // 0x0000 (0x5A5A if copy-protected)
    /* 0x1BE */ mbr_partition_entry primary_partitions[4];  // Partition table (for primary partitions)
    /* 0x1FE */ u16 boot_signature;                         // 0xAA55
} master_boot_record;

#endif // _MBR_TYPES_H

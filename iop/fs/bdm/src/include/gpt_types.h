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
 * GPT partition definitions.
 */

#ifndef _GPT_TYPES_H
#define _GPT_TYPES_H

#include <types.h>

static const u8 EFI_PARTITION_SIGNATURE[8] = { 'E', 'F', 'I', ' ', 'P', 'A', 'R', 'T' };

typedef struct __attribute__((packed))  _gpt_partition_table_header
{
    /* 0x00 */ u8 signature[8];             // 'EFI PART'
    /* 0x08 */ u32 revision;                // Revision 1.0 (00h 00h 01h 00h) for UEFI 2.8
    /* 0x0C */ u32 header_size;             // Header size in little endian (in bytes, usually 5Ch 00h 00h 00h or 92 bytes)
    /* 0x10 */ u32 header_checksum;         // CRC32 of header (offset +0 to +0x5b) in little endian, with this field zeroed during calculation
    /* 0x14 */ u32 reserved1;               // Reserved, must be zero
    /* 0x18 */ u64 current_lba;             // Current LBA (location of this header copy)
    /* 0x20 */ u64 backup_lba;              // Backup LBA (location of the other header copy)
    /* 0x28 */ u64 first_lba;               // First usable LBA for partitions (primary partition table last LBA + 1)
    /* 0x30 */ u64 last_lba;                // Last usable LBA (secondary partition table first LBA − 1)
    /* 0x38 */ u8 disk_guid[16];            // Disk GUID in mixed endian
    /* 0x48 */ u64 partition_table_lba;     // Starting LBA of array of partition entries (usually 2 for compatibility)
    /* 0x50 */ u32 partition_count;         // Number of partition entries in array
    /* 0x54 */ u32 partition_entry_size;    // Size of a single partition entry (usually 80h or 128)
    /* 0x58 */ u32 partition_table_checksum;    // CRC32 of partition entries array in little endian
} gpt_partition_table_header;

// Partition type GUIDs:
static const u8 NULL_GUID[16] = { 0 };
static const u8 MS_RESERVED_PARTITION_GUID[16] = { 0x16, 0xE3, 0xC9, 0xE3, 0x5C, 0x0B, 0xB8, 0x4D, 0x81, 0x7D, 0xF9, 0x2D, 0xF0, 0x02, 0x15, 0xAE };
static const u8 EFI_SYSTEM_PARTITION[16] = { 0x28, 0x73, 0x2A, 0xC1, 0x1F, 0xF8, 0xD2, 0x11, 0xBA, 0x4B, 0x00, 0xA0, 0xC9, 0x3E, 0xC9, 0x3B };
static const u8 MS_BASIC_DATA_PARTITION_GUID[16] = { 0xA2, 0xA0, 0xD0, 0xEB, 0xE5, 0xB9, 0x33, 0x44, 0x87, 0xC0, 0x68, 0xB6, 0xB7, 0x26, 0x99, 0xC7 };

typedef struct __attribute__((packed)) _gpt_partition_table_entry
{
    /* 0x00 */ u8 partition_type_guid[16];      // Partition type guid (mixed endian)
    /* 0x10 */ u8 partition_unique_guid[16];    // Unique partition id
    /* 0x20 */ u64 first_lba;                   // First LBA (little endian)
    /* 0x28 */ u64 last_lba;                    // Last LBA (inclusive, usually odd)
    /* 0x30 */ u64 attribute_flags;             // Attribute flags (e.g. bit 60 denotes read-only)
    /* 0x38 */ u16 partition_name[36];          // Partition name (36 UTF-16LE code units)
} gpt_partition_table_entry;

#define GPT_PART_ATTR_PLATFORM_REQUIRED         0x1         // Platform required (required by the computer to function properly, OEM partition for example, disk partitioning utilities must preserve the partition as is) 
#define GPT_PART_ATTR_IGNORE                    0x2         // EFI firmware should ignore the content of the partition and not try to read from it
#define GPT_PART_ATTR_LEGACY_BOOTABLE           0x4         // Legacy BIOS bootable

#define GPT_PART_ATTR_TYPE_SHIFT                48          // Number of partition attribute bits reserved for common attribute
#define GPT_PART_ATTR_TYPE_MASK                 0xFFFF      // Mask for type-specific attribute bits (after shifting)
#define GPT_PART_ATTR_TYPE_SPECIFIC(attr)       ((attr >> GPT_PART_ATTR_TYPE_SHIFT) & GPT_PART_ATTR_TYPE_MASK)

// Basic data partition type-specific attribute flags:
#define GPT_BDP_ATTR_READ_ONLY                  0x1000      // Partition is read only
#define GPT_BDP_ATTR_SHADOW_COPY                0x2000      // Shadow copy (of another partition)
#define GPT_BDP_ATTR_HIDDEN                     0x4000      // Partition is hidden
#define GPT_BDP_ATTR_NO_DRIVE_LETTER            0x8000      // No drive letter (i.e. do not automount)

#endif // _GPT_TYPES_H

/*
# _____     ___ ____     ___ ____
#  ____|   |    ____|   |        | |____|
# |     ___|   |____ ___|    ____| |    \    PS2DEV Open Source Project.
#-----------------------------------------------------------------------
# Copyright 2001-2004, ps2dev - http://www.ps2dev.org
# Licenced under Academic Free License version 2.0
# Review ps2sdk README & LICENSE files for further details.
*/

/**
 * @file
 * ROM file system driver routines.
 * Not available on the protokernel version of ROMDRV.
 */

/*
    ROMFS format:
        ROMDIR section
        EXTINFO section
        File data section
    All files will have an entry in all three sections. The ROMDIR section is terminated by an entry that consists of zeros.

    Required file entries (In this order):
        RESET	(0-byte)
        ROMDIR	(The size of the whole ROMDIR section)
        EXTINFO	(The size of the whole EXTINFO section)

    The EXTINFO section (Extended Information section) contains more information on the file (E.g. date and version numbers) and comments on the file.

    The EXTINFO section is also a file! (In fact, all sections are files)

    All sections and files are aligned to 16-byte boundaries, and all records within each section must be aligned to 4-byte boundaries.
*/

#ifndef _PS2SDK_ROMDRV_H
#define _PS2SDK_ROMDRV_H

struct RomDirEntry
{
    char name[10];
    u16 ExtInfoEntrySize;
    u32 size;
};

struct RomImg
{
    const void *ImageStart;
    const void *RomdirStart;
    const void *RomdirEnd;
};

/* Each ROMDIR entry can have any combination of EXTINFO fields. */
struct ExtInfoFieldEntry
{
    u16 value;    /* Only applicable for the version field type. */
    u8 ExtLength; /* The length of data appended to the end of this entry. */
    u8 type;
};

enum ExtInfoFieldTypes {
    EXTINFO_FIELD_TYPE_DATE = 1,
    EXTINFO_FIELD_TYPE_VERSION,
    EXTINFO_FIELD_TYPE_COMMENT,
    EXTINFO_FIELD_TYPE_FIXED = 0x7F // Must exist at a fixed location.
};

#define ROMDRV_MAX_IMAGES 4
#define ROMDRV_MAX_FILES  8

// Error codes
#define ROMDRV_ADD_FAILED    -160
#define ROMDRV_DEL_FAILED    -161
#define ROMDRV_ADD_BAD_IMAGE -162

#define romdrv_IMPORTS_start DECLARE_IMPORT_TABLE(romdrv, 2, 1)
#define romdrv_IMPORTS_end   END_IMPORT_TABLE

extern int romAddDevice(int unit, const void *image);
#define I_romAddDevice DECLARE_IMPORT(4, romAddDevice)
extern int romDelDevice(int unit);
#define I_romDelDevice DECLARE_IMPORT(5, romDelDevice)

// Functions only available in romdrvX
#define romdrvX_IMPORTS_start DECLARE_IMPORT_TABLE(romdrvX, 1, 1)
#define romdrvX_IMPORTS_end   END_IMPORT_TABLE

extern const struct RomImg *romGetDevice(int unit);
#define I_romGetDevice DECLARE_IMPORT(6, romGetDevice)

// Backwards compatibility definitions
#define romdrv_mount     romAddDevice
#define I_romdrv_mount   I_romAddDevice
#define romdrv_unmount   romDelDevice
#define I_romdrv_unmount I_romDelDevice

#endif

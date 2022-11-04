/*
# _____     ___ ____     ___ ____
#  ____|   |    ____|   |        | |____|
# |     ___|   |____ ___|    ____| |    \    PS2DEV Open Source Project.
#-----------------------------------------------------------------------
# Copyright (c) 2009 jimmikaelkael
# Licenced under Academic Free License version 2.0
# Review ps2sdk README & LICENSE files for further details.
*/

/**
 * @file
 * Definitions and imports for secrman module.
 */

#ifndef __LIBSECR_COMMON_H__
#define __LIBSECR_COMMON_H__

#include <tamtypes.h>

/** Encrypted file Data Block info struct */
typedef struct SecrBitBlockData
{
    /** Size of data block */
    u32 size;
    /** Flags : 0x01 = signed, 0x02 = encrypted. */
    u32 flags;
    u8 checksum[8];
} SecrBitBlockData_t;

typedef struct SecrBitTableHeader
{
    /** KELF header size (same as SecrKELFHeader_t.KELF_header_size) */
    u32 headersize;
    /** Number of blocks in the KELF file */
    u8 block_count;
    u8 pad1;
    u8 pad2;
    u8 pad3;
} SecrBitTableHeader_t;

/** Encrypted file BIT table struct */
typedef struct SecrBitTable
{
    SecrBitTableHeader_t header;
    /** KELF section information. */
    SecrBitBlockData_t blocks[63];
} SecrBitTable_t;

/** Encrypted file header struct */
typedef struct KELF_Header
{
    u8 UserHeader[16];
    /** Size of data blocks = Decrypted elf size */
    u32 ELF_size;
    /** KELF header size */
    u16 KELF_header_size;
    u16 unknown5;
    /** Controls the layout of the KELF header. */
    u16 flags;
    /** Number of entries in the bit table. */
    u16 BIT_count;
    u32 mg_zones;
} SecrKELFHeader_t;

extern int SecrCardBootHeader(int port, int slot, void *buffer, SecrBitTable_t *BitTable, s32 *pSize);
extern int SecrCardBootBlock(void *src, void *dst, unsigned int size);
extern void *SecrCardBootFile(int port, int slot, void *buffer);
extern int SecrDiskBootHeader(void *buffer, SecrBitTable_t *BitTable, s32 *pSize);
extern int SecrDiskBootBlock(void *src, void *dst, unsigned int size);
extern void *SecrDiskBootFile(void *buffer);

/* FOLLOWING EXPORTS ARE ONLY AVAILABLE IN SPECIAL SECRMAN OR FREESECR */
extern int SecrDownloadHeader(int port, int slot, void *buffer, SecrBitTable_t *BitTable, s32 *pSize);
extern int SecrDownloadBlock(void *src, unsigned int size);
extern void *SecrDownloadFile(int port, int slot, void *buffer);
extern int SecrDownloadGetKbit(int port, int slot, void *kbit);
extern int SecrDownloadGetKc(int port, int slot, void *kbit);
extern int SecrDownloadGetICVPS2(void *icvps2);

#endif /* __LIBSECR_COMMON_H__ */

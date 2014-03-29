/*
# _____     ___ ____     ___ ____
#  ____|   |    ____|   |        | |____|
# |     ___|   |____ ___|    ____| |    \    PS2DEV Open Source Project.
#-----------------------------------------------------------------------
# Copyright (c) 2009 jimmikaelkael
# Licenced under Academic Free License version 2.0
# Review ps2sdk README & LICENSE files for further details.
#
# $Id: secrman.h 1410 2009-01-18 15:24:54Z jimmikaelkael $
# Definitions and imports for secrman module.
*/

#ifndef LIBSECR_COMMON_H
#define LIBSECR_COMMON_H

#include <tamtypes.h>

// Encrypted file Data Block info struct
typedef struct SecrBitBlockData{
	u32 size;		// Size of data block
	u32 flags;		// Flags : 0x01 = signed, 0x02 = encrypted.
	u8 checksum[8];
} SecrBitBlockData_t;

typedef struct SecrBitTableHeader{
	u32 headersize;	// KELF header size (same as SecrKELFHeader_t.KELF_header_size)
	u8 block_count;	// Number of blocks in the KELF file
	u8 pad1;
	u8 pad2;
	u8 pad3;
} SecrBitTableHeader_t;

// Encrypted file BIT table struct
typedef struct SecrBitTable {
	SecrBitTableHeader_t header;
	SecrBitBlockData_t blocks[63];	// KELF section information.
} SecrBitTable_t;

// Encrypted file header struct
typedef struct KELF_Header{
	u8 UserHeader[16];
	u32 ELF_size;		// Size of data blocks = Decrypted elf size
	u16 KELF_header_size;	// KELF header size
	u16 unknown5;
	u16 flags;		// Controls the layout of the KELF header.
	u16 BIT_count;		// Number of entries in the bit table.
	u32 mg_zones;
} SecrKELFHeader_t;

int SecrCardBootHeader(int port, int slot, void *buffer, SecrBitTable_t *BitTable, s32 *pSize);
int SecrCardBootBlock(void *src, void *dst, unsigned int size);
void *SecrCardBootFile(int port, int slot, void *buffer);
int SecrDiskBootHeader(void *buffer, SecrBitTable_t *BitTable, s32 *pSize);
int SecrDiskBootBlock(void *src, void *dst, unsigned int size);
void *SecrDiskBootFile(void *buffer);

/* FOLLOWING EXPORTS ARE ONLY AVAILABLE IN SPECIAL SECRMAN OR FREESECR */
int SecrDownloadHeader(int port, int slot, void *buffer, SecrBitTable_t *BitTable, s32 *pSize);
int SecrDownloadBlock(void *src, unsigned int size);
void *SecrDownloadFile(int port, int slot, void *buffer);
int SecrDownloadGetKbit(int port, int slot, void *kbit);
int SecrDownloadGetKc(int port, int slot, void *kbit);
int SecrDownloadGetICVPS2(void *icvps2);

#endif /* LIBSECR_COMMON_H */

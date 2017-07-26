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

#ifndef __SECRMAN_H__
#define __SECRMAN_H__

#include <types.h>
#include <irx.h>
#include <sio2man.h>
#include <libsecr-common.h>

typedef int (*McCommandHandler_t)(int port, int slot, sio2_transfer_data_t *sio2_trans_data);
typedef int (*McDevIDHandler_t)(int port, int slot);

void SecrSetMcCommandHandler(McCommandHandler_t handler);
void SecrSetMcDevIDHandler(McDevIDHandler_t handler);

int SecrAuthCard(int port, int slot, int cnum);
void SecrResetAuthCard(int port, int slot, int cnum);

#define secrman_IMPORTS_start DECLARE_IMPORT_TABLE(secrman, 1, 3)
#define secrman_IMPORTS_end END_IMPORT_TABLE

#define I_SecrSetMcCommandHandler DECLARE_IMPORT(4, SecrSetMcCommandHandler)
#define I_SecrSetMcDevIDHandler DECLARE_IMPORT(5, SecrSetMcDevIDHandler)
#define I_SecrAuthCard DECLARE_IMPORT(6, SecrAuthCard)
#define I_SecrResetAuthCard DECLARE_IMPORT(7, SecrResetAuthCard)

#define I_SecrCardBootHeader DECLARE_IMPORT(8, SecrCardBootHeader)
#define I_SecrCardBootBlock DECLARE_IMPORT(9, SecrCardBootBlock)
#define I_SecrCardBootFile DECLARE_IMPORT(10, SecrCardBootFile)
#define I_SecrDiskBootHeader DECLARE_IMPORT(11, SecrDiskBootHeader)
#define I_SecrDiskBootBlock DECLARE_IMPORT(12, SecrDiskBootBlock)
#define I_SecrDiskBootFile DECLARE_IMPORT(13, SecrDiskBootFile)

/* FOLLOWING EXPORTS ARE ONLY AVAILABLE IN SPECIAL SECRMAN OR FREESECR */
#define I_SecrDownloadHeader DECLARE_IMPORT(14, SecrDownloadHeader)
#define I_SecrDownloadBlock DECLARE_IMPORT(15, SecrDownloadBlock)
#define I_SecrDownloadFile DECLARE_IMPORT(16, SecrDownloadFile)
#define I_SecrDownloadGetKbit DECLARE_IMPORT(17, SecrDownloadGetKbit)
#define I_SecrDownloadGetKc DECLARE_IMPORT(18, SecrDownloadGetKc)
#define I_SecrDownloadGetICVPS2 DECLARE_IMPORT(19, SecrDownloadGetICVPS2)

#endif /* IOP_SECRMAN_H */

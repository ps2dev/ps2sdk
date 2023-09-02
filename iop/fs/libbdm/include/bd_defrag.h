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
 * Fragmented Block Device to Block Device
 */

#ifndef __BDM_DEFRAG_H__
#define __BDM_DEFRAG_H__


#include <tamtypes.h>
#include <bdm.h>
#include <usbhdfsd-common.h>


int bd_defrag_read(struct block_device* bd, u32 fragcount, struct bd_fragment* fraglist, u64 sector, void* buffer, u16 count);
int bd_defrag_write(struct block_device* bd, u32 fragcount, struct bd_fragment* fraglist, u64 sector, const void* buffer, u16 count);

// For backwards compatibility:
#define bd_defrag bd_defrag_read

#endif

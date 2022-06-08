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


int bd_defrag(struct block_device* bd, struct bd_fraglist* fl, u32 sector, void* buffer, u16 count);



#endif

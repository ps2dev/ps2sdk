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
 * Simple cache for Block Devices
 */

#ifndef __BDM_CACHE_H__
#define __BDM_CACHE_H__


#include <tamtypes.h>
#include <bdm.h>


/* Create a new cached block device */
struct block_device *bd_cache_create(struct block_device *bd);

/* Destroy a cached block device */
void bd_cache_destroy(struct block_device *cbd);


#endif

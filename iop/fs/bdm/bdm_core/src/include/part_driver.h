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
 * Common partition driver definitions.
 */

#ifndef _PART_DRIVER_H
#define _PART_DRIVER_H

#include <bdm.h>

struct partition
{
    struct block_device *bd;
};

#define MAX_PARTITIONS 10
extern struct partition g_part[MAX_PARTITIONS];
extern struct block_device g_part_bd[MAX_PARTITIONS];

int GetNextFreePartitionIndex();

int part_connect_mbr(struct block_device *bd);
int part_connect_gpt(struct block_device *bd);

#endif
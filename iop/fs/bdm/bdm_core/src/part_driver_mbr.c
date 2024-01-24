#include "part_driver.h"
#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <sysmem.h>

#include <bdm.h>
#include "mbr_types.h"

#include "module_debug.h"

int part_connect_mbr(struct block_device *bd)
{
    master_boot_record* pMbrBlock = NULL;
    int rval = -1;
    int ret;
    int mountCount = 0;
    int partIndex;

    M_DEBUG("%s\n", __func__);
    
    // Filter out any block device where sectorOffset != 0, as this will be a block device for a file system partition and not
    // the raw device.
    if (bd->sectorOffset != 0)
        return rval;

    // Allocate memory for MBR partition sector.
    pMbrBlock = AllocSysMemory(ALLOC_FIRST, sizeof(master_boot_record), NULL);
    if (pMbrBlock == NULL)
    {
        // Failed to allocate memory for mbr block.
        M_DEBUG("Failed to allocate memory for MBR block\n");
        return rval;
    }

    // Read the MBR block from the block device.
    ret = bd->read(bd, 0, pMbrBlock, 1);
    if (ret < 0)
    {
        // Failed to read MBR block from the block device.
        M_DEBUG("Failed to read MBR sector from block device %d\n", ret);
        return rval;
    }

    // Check the MBR boot signature.
    if (pMbrBlock->boot_signature != MBR_BOOT_SIGNATURE)
    {
        // Boot signature is invalid, device is not valid MBR.
        M_DEBUG("MBR boot signature is invalid, device is not MBR\n");
        FreeSysMemory(pMbrBlock);
        return rval;
    }

    // Check the first primary partition to see if this is a GPT protective MBR.
    if (pMbrBlock->primary_partitions[0].partition_type == MBR_PART_TYPE_GPT_PROTECTIVE_MBR)
    {
        // We explicitly fail to connect GPT protective MBRs in order to let the GPT driver handle it.
        FreeSysMemory(pMbrBlock);
        return rval;
    }
    
    // Loop and parse the primary partition entries in the MBR block.
    printf("Found MBR disk\n");
    for (int i = 0; i < 4; i++)
    {
        // Check if the partition is active, checking the status bit is not reliable so check if the sector_count is greater than zero instead.
        if (pMbrBlock->primary_partitions[i].sector_count == 0)
            continue;

        printf("Found partition type 0x%02x\n", pMbrBlock->primary_partitions[i].partition_type);
        
        // TODO: Filter out unsupported partition types.

        if ((partIndex = GetNextFreePartitionIndex()) == -1)
        {
            // No more free partition slots.
            printf("Can't mount partition, no more free partition slots!\n");
            continue;
        }

        // Create the pseudo block device for the partition.
        g_part[partIndex].bd              = bd;
        g_part_bd[partIndex].name         = bd->name;
        g_part_bd[partIndex].devNr        = bd->devNr;
        g_part_bd[partIndex].parNr        = i + 1;
        g_part_bd[partIndex].parId        = pMbrBlock->primary_partitions[i].partition_type;
        g_part_bd[partIndex].sectorSize   = bd->sectorSize;
        g_part_bd[partIndex].sectorOffset = bd->sectorOffset + (u64)pMbrBlock->primary_partitions[i].first_lba;
        g_part_bd[partIndex].sectorCount  = pMbrBlock->primary_partitions[i].sector_count;
        bdm_connect_bd(&g_part_bd[partIndex]);
        mountCount++;
    }

    // If one or more partitions were mounted then return success.
    rval = mountCount > 0 ? 0 : -1;

    FreeSysMemory(pMbrBlock);
    return rval;
}
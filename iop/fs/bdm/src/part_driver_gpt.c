#include "part_driver.h"
#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <sysmem.h>

#include <bdm.h>
#include "gpt_types.h"

#include "module_debug.h"

#define U64_2XU32(val)  ((u32*)val)[1], ((u32*)val)[0]

void GetGPTPartitionNameAscii(gpt_partition_table_entry* pPartition, char* pAsciiBuffer)
{
    // Loop and perform the world's worst unicode -> ascii string conversion.
    for (int i = 0; i < sizeof(pPartition->partition_name) / sizeof(u16); i++)
        pAsciiBuffer[i] = (char)pPartition->partition_name[i];
}

int part_connect_gpt(struct block_device *bd)
{
    int ret;
    void* buffer = NULL;
    gpt_partition_table_header* pGptHeader;
    gpt_partition_table_entry* pGptPartitionEntry;
    int entriesPerSector;
    int endOfTable = 0;
    char partName[37] = { 0 };
    int partIndex;
    int mountCount = 0;

    M_DEBUG("%s\n", __func__);

    // Allocate scratch memory for parsing the partition table.
    buffer = AllocSysMemory(ALLOC_FIRST, 512 * 2, NULL);
    if (buffer == NULL)
    {
        M_DEBUG("Failed to allocate memory\n");
        return 0;
    }

    pGptHeader = (gpt_partition_table_header*)buffer;
    pGptPartitionEntry = (gpt_partition_table_entry*)((u8*)buffer + 512);

    // Read the GPT partition table header from the block device.
    ret = bd->read(bd, 1, pGptHeader, 1);
    if (ret < 0)
    {
        // Failed to read gpt partition table header.
        M_DEBUG("Failed to read GPT partition table header %d\n", ret);
        FreeSysMemory(buffer);
        return -1;
    }

    // Check the partition table header signature.
    if (memcmp(pGptHeader->signature, EFI_PARTITION_SIGNATURE, sizeof(EFI_PARTITION_SIGNATURE)) != 0)
    {
        // GPT partition table header signature is invalid.
        M_DEBUG("GPT partition table header signature is invalid: %s\n", pGptHeader->signature);
        FreeSysMemory(buffer);
        return -1;
    }

    // TODO: we might want to check the header revision and size for compatibility for newer/older GPT layouts. There's
    // also a few CRC checksums in the header that may be useful to validate, but is probably not needed.

    // Calculate how many partition entries there are per sector.
    entriesPerSector = bd->sectorSize / sizeof(gpt_partition_table_entry);

    // Loop through all the partition table entries and attempt to mount each one.
    printf("Found GPT disk '%08x...'\n", *(u32*)&pGptHeader->disk_guid);
    for (int i = 0; i < pGptHeader->partition_count && endOfTable == 0; )
    {
        // Check if we need to buffer more data, GPT usually uses LBA 2-33 for partition table entries. Typically there will
        // only be a couple partitions at most, so we buffer one sector at a time to avoid making needless allocations for all sectors at once.
        if (i % entriesPerSector == 0)
        {
            // Read the next sector from the block device.
            ret = bd->read(bd, pGptHeader->partition_table_lba + (i / entriesPerSector), pGptPartitionEntry, 1);
            if (ret < 0)
            {
                // Failed to read the next sector from the drive.
#ifdef DEBUG
                u64 lba = pGptHeader->partition_table_lba + (i / entriesPerSector);
                M_DEBUG("Failed to read next partition table entry sector lba=0x%08x%08x\n", U64_2XU32(&lba));
#endif
                FreeSysMemory(buffer);
                return -1;
            }

            // Parse the two partition table entries in the structure.
            for (int x = 0; x < entriesPerSector; x++, i++)
            {
                // Check if the partition type guid is valid, the header will list the maximum number of partitions that can fit into the table, so
                // we need to check if the entries are actually valid.
                if (memcmp(pGptPartitionEntry[x].partition_type_guid, NULL_GUID, sizeof(NULL_GUID)) == 0)
                {
                    // Stop scanning for partitions.
                    endOfTable = 1;
                    break;
                }

                // Perform some sanity checks on the partition.
                if (pGptPartitionEntry[x].first_lba < pGptHeader->first_lba || pGptPartitionEntry[x].last_lba > pGptHeader->last_lba)
                {
                    // Partition entry data appears to be corrupt.
                    M_DEBUG("Partition entry %d appears to be corrupt (lba bounds incorrect)\n", i);
                    continue;
                }

                // Print the partition info and create a pseudo block device for it.
                GetGPTPartitionNameAscii(&pGptPartitionEntry[x], partName);
                printf("Found partition '%s' type=%08x unique=%08x start=0x%08x%08x end=0x%08x%08x attr=0x%08x%08x\n", partName, *(u32*)&pGptPartitionEntry[x].partition_type_guid,
                    *(u32*)&pGptPartitionEntry[x].partition_unique_guid, U64_2XU32(&pGptPartitionEntry[x].first_lba), U64_2XU32(&pGptPartitionEntry[x].last_lba), U64_2XU32(&pGptPartitionEntry[x].attribute_flags));

                // Check for specific GPT partition types we should ignore.
                if (memcmp(pGptPartitionEntry[x].partition_type_guid, MS_RESERVED_PARTITION_GUID, sizeof(MS_RESERVED_PARTITION_GUID)) == 0 ||
                    memcmp(pGptPartitionEntry[x].partition_type_guid, EFI_SYSTEM_PARTITION, sizeof(EFI_SYSTEM_PARTITION)) == 0)
                    continue;

                // Check if the partition should be ignored.
                if ((pGptPartitionEntry[x].attribute_flags & GPT_PART_ATTR_IGNORE) != 0)
                    continue;

                // TODO: Check type specific partition flags: read-only, hidden, etc.

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
                g_part_bd[partIndex].parId        = 0;
                g_part_bd[partIndex].sectorSize   = bd->sectorSize;
                g_part_bd[partIndex].sectorOffset = bd->sectorOffset + pGptPartitionEntry[x].first_lba;
                g_part_bd[partIndex].sectorCount  = pGptPartitionEntry[x].last_lba - pGptPartitionEntry[x].first_lba;
                bdm_connect_bd(&g_part_bd[partIndex]);
                mountCount++;
            }
        }
    }

    // Free our scratch buffer.
    FreeSysMemory(buffer);
    return mountCount > 0 ? 0 : -1;
}
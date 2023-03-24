#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <sysmem.h>
#include <errno.h>

#include <bdm.h>
#include "mbr_types.h"
#include "gpt_types.h"

// #define DEBUG  //comment out this line when not debugging
#include "module_debug.h"

#define U64_2XU32(val)  ((u32*)val)[1], ((u32*)val)[0]

struct partition
{
    struct block_device *bd;
};

#define MAX_PARTITIONS 10
static struct partition g_part[MAX_PARTITIONS];
static struct block_device g_part_bd[MAX_PARTITIONS];

static struct file_system g_mbr_fs;

int GetNextFreePartitionIndex()
{
    // Loop and find the next free partition index.
    for (int i = 0; i < MAX_PARTITIONS; i++)
    {
        if (g_part[i].bd == NULL)
            return i;
    }

    // No more free partitions.
    return -1;
}

void GetGPTPartitionNameAscii(gpt_partition_table_entry* pPartition, char* pAsciiBuffer)
{
    // Loop and perform the world's worst unicode -> ascii string conversion.
    for (int i = 0; i < sizeof(pPartition->partition_name) / sizeof(u16); i++)
        pAsciiBuffer[i] = (char)pPartition->partition_name[i];
}

int ParseGPTPartitionTable(struct block_device* bd)
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
        return 0;
    }

    // Check the partition table header signature.
    if (memcmp(pGptHeader->signature, EFI_PARTITION_SIGNATURE, sizeof(EFI_PARTITION_SIGNATURE)) != 0)
    {
        // GPT partition table header signature is invalid.
        M_DEBUG("GPT partition table header signature is invalid: %s\n", pGptHeader->signature);
        FreeSysMemory(buffer);
        return 0;
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
                return 0;
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
                    // TODO: If one partition is borked we should bail out from mounting any partitions.

                    // Partition entry data appears to be corrupt.
                    M_DEBUG("Partition entry %d appears to be corrupt (lba bounds incorrect)\n", i);
                    continue;
                }

                // Print the partition info and create a pseudo block device for it.
                GetGPTPartitionNameAscii(&pGptPartitionEntry[x], partName);
                printf("Found partition '%s' type=%08x unique=%08x start=0x%08x%08x end=0x%08x%08x attr=0x%08x%08x\n", partName, *(u32*)&pGptPartitionEntry[x].partition_type_guid,
                    *(u32*)&pGptPartitionEntry[x].partition_unique_guid, U64_2XU32(&pGptPartitionEntry[x].first_lba), U64_2XU32(&pGptPartitionEntry[x].last_lba), U64_2XU32(&pGptPartitionEntry[x].attribute_flags));

                // Check for specific GPT partition types we should ignore.
                if (memcmp(pGptPartitionEntry[x].partition_type_guid, MS_RESERVED_PARTITION_GUID, sizeof(MS_RESERVED_PARTITION_GUID)) == 0)
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
    printf("Done parsing GPT disk\n");

    // Free our scratch buffer.
    FreeSysMemory(buffer);
    return mountCount;
}

int ParseMBRPartitionTable(struct block_device *bd, master_boot_record* pMbrBlock)
{
    int mountCount = 0;
    int partIndex;

    M_DEBUG("%s\n", __func__);

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

    return mountCount;
}

//---------------------------------------------------------------------------
int part_connect(struct block_device *bd)
{
    master_boot_record* pMbrBlock = NULL;
    int rval = -1;
    int ret;

    M_DEBUG("%s\n", __func__);

    // Filter for supported partition IDs:
    // - First sector of disk can be MBR partition
    // - 0x05 = extended MBR partition with CHS addressing
    // - 0x0f = extended MBR partition with LBA addressing
    //if (bd->sectorOffset != 0 && bd->parId != 0x05 && bd->parId != 0x0f)
    //    return rval;

    // I don't really understand what the filter above is trying to accomplish...
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
        // Parse GPT partition table.
        if ((rval = ParseGPTPartitionTable(bd)) < 0)
        {
            FreeSysMemory(pMbrBlock);
            return rval;
        }

        // If one or more partitions were mounted then return success.
        rval = rval > 0 ? 0 : -1;
    }
    else
    {
        // Parse the MBR partition table.
        if ((rval = ParseMBRPartitionTable(bd, pMbrBlock)) < 0)
        {
            FreeSysMemory(pMbrBlock);
            return rval;
        }

        // If one or more partitions were mounted then return success.
        rval = rval > 0 ? 0 : -1;
    }

    FreeSysMemory(pMbrBlock);
    return rval;
}

//---------------------------------------------------------------------------
void part_disconnect(struct block_device *bd)
{
    int i;

    M_DEBUG("%s\n", __func__);

    for (i = 0; i < MAX_PARTITIONS; ++i) {
        if (g_part[i].bd == bd) {
            bdm_disconnect_bd(&g_part_bd[i]);
            g_part[i].bd = NULL;
        }
    }
}

//
// Block device interface
//
static int part_read(struct block_device *bd, u64 sector, void *buffer, u16 count)
{
    struct partition *part = (struct partition *)bd->priv;

    if ((part == NULL) || (part->bd == NULL))
        return -1;

    u64 finalSector = sector + bd->sectorOffset;
    M_DEBUG("%s (%s %d %d) 0x%08x%08x %d\n", __func__, bd->name, bd->devNr, bd->parNr, U64_2XU32(&finalSector), count);

    return part->bd->read(part->bd, sector + bd->sectorOffset, buffer, count);
}

static int part_write(struct block_device *bd, u64 sector, const void *buffer, u16 count)
{
    struct partition *part = (struct partition *)bd->priv;

    M_DEBUG("%s\n", __func__);

    if ((part == NULL) || (part->bd == NULL))
        return -1;

    return part->bd->write(part->bd, sector + bd->sectorOffset, buffer, count);
}

static void part_flush(struct block_device *bd)
{
    struct partition *part = (struct partition *)bd->priv;

    M_DEBUG("%s\n", __func__);

    if ((part == NULL) || (part->bd == NULL))
        return;

    return part->bd->flush(part->bd);
}

static int part_stop(struct block_device *bd)
{
    struct partition *part = (struct partition *)bd->priv;

    M_DEBUG("%s\n", __func__);

    if ((part == NULL) || (part->bd == NULL))
        return -1;

    return part->bd->stop(part->bd);
}

static int part_ioctl(struct block_device *bd, int ioctl, void* inp, u32 inpsize, void* outp, u32 outpsize)
{
    struct partition *part = (struct partition *)bd->priv;

    M_DEBUG("%s\n", __func__);

    if ((part == NULL) || (part->bd == NULL))
        return -1;

    if (part->bd->ioctl == NULL)
        return -ENOTSUP;

    return part->bd->ioctl(part->bd, ioctl, inp, inpsize, outp, outpsize);
}

void part_init()
{
    int i;

    M_DEBUG("%s\n", __func__);

    for (i = 0; i < MAX_PARTITIONS; ++i) {
        g_part[i].bd = NULL;

        g_part_bd[i].priv  = &g_part[i];
        g_part_bd[i].read  = part_read;
        g_part_bd[i].write = part_write;
        g_part_bd[i].flush = part_flush;
        g_part_bd[i].stop  = part_stop;
        g_part_bd[i].ioctl = part_ioctl;
    }

    // Setup MBR/GPT file system driver:
    g_mbr_fs.priv          = NULL;
    g_mbr_fs.name          = "MBR/GPT";
    g_mbr_fs.connect_bd    = part_connect;
    g_mbr_fs.disconnect_bd = part_disconnect;
    bdm_connect_fs(&g_mbr_fs);
}

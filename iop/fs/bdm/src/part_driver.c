#include "part_driver.h"
#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <sysmem.h>

#include <bdm.h>
#include "mbr_types.h"
#include "gpt_types.h"

#include "module_debug.h"

#define U64_2XU32(val)  ((u32*)val)[1], ((u32*)val)[0]

struct partition g_part[MAX_PARTITIONS];
struct block_device g_part_bd[MAX_PARTITIONS];

static struct file_system g_mbr_fs;
static struct file_system g_gpt_fs;

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

#ifdef DEBUG
    u64 finalSector = sector + bd->sectorOffset;
    M_DEBUG("%s (%s %d %d) 0x%08x%08x %d\n", __func__, bd->name, bd->devNr, bd->parNr, U64_2XU32(&finalSector), count);
#endif

    return part->bd->read(part->bd, sector + bd->sectorOffset, buffer, count);
}

static int part_write(struct block_device *bd, u64 sector, const void *buffer, u16 count)
{
    struct partition *part = (struct partition *)bd->priv;

    if ((part == NULL) || (part->bd == NULL))
        return -1;

#ifdef DEBUG
    u64 finalSector = sector + bd->sectorOffset;
    M_DEBUG("%s (%s %d %d) 0x%08x%08x %d\n", __func__, bd->name, bd->devNr, bd->parNr, U64_2XU32(&finalSector), count);
#endif

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
    }

    // Setup MBR file system driver:
    g_mbr_fs.priv          = NULL;
    g_mbr_fs.name          = "MBR";
    g_mbr_fs.connect_bd    = part_connect_mbr;
    g_mbr_fs.disconnect_bd = part_disconnect;
    bdm_connect_fs(&g_mbr_fs);

    // Setup GPT file system driver:
    g_gpt_fs.priv          = NULL;
    g_gpt_fs.name          = "GPT";
    g_gpt_fs.connect_bd    = part_connect_gpt;
    g_gpt_fs.disconnect_bd = part_disconnect;
    bdm_connect_fs(&g_gpt_fs);
}

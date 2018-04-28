#include <errno.h>
#include <stdio.h>

#include <bdm.h>

//#define DEBUG  //comment out this line when not debugging
#include "module_debug.h"

#define getI32(buf) ((int)(((u8*)(buf))[0] + (((u8*)(buf))[1] << 8) + (((u8*)(buf))[2] << 16) + (((u8*)(buf))[3] << 24)))

typedef struct _part_record {
    unsigned char sid;  //system id-	4=16bit FAT (16bit sector numbers)
                        //		5=extended partition
                        //		6=16bit FAT (32bit sector numbers)
    unsigned int start; // start sector of the partition
    unsigned int count; // length of the partititon (total number of sectors)
} part_record;

typedef struct _part_table {
    part_record record[4]; //maximum of 4 primary partitions
} part_table;

typedef struct _part_raw_record {
    unsigned char active;      //Set to 80h if this partition is active / bootable
    unsigned char startH;      //Partition's starting head.
    unsigned char startST[2];  //Partition's starting sector and track.
    unsigned char sid;         //Partition's system ID number.
    unsigned char endH;        //Partition's ending head.
    unsigned char endST[2];    //Partition's ending sector and track.
    unsigned char startLBA[4]; //Starting LBA (sector)
    unsigned char size[4];     //Partition size in sectors.
} part_raw_record;

struct partition {
    struct block_device* bd;
};

#define MAX_PARTITIONS 10
static struct partition g_part[MAX_PARTITIONS];
static struct block_device g_part_bd[MAX_PARTITIONS];
static struct file_system g_part_fs;

//---------------------------------------------------------------------------
static int part_getPartitionRecord(struct block_device* bd, part_raw_record* raw, part_record* rec)
{
    M_DEBUG("%s\n", __func__);

    rec->sid   = raw->sid;
    rec->start = getI32(raw->startLBA);
    rec->count = getI32(raw->size);

    if (rec->sid != 0x00) { /*	Windows appears to check if the start LBA is not 0 and whether the start LBA is within the disk.
			There may be checks against the size, but I didn't manage to identify a pattern.
			If the disk has no partition table (i.e. disks with "removable" media), then this check is also one safeguard. */
        if ((rec->start == 0) || (rec->start >= bd->sectorCount))
            return 1;
    }

    return 0;
}

//---------------------------------------------------------------------------
// FIXME: use malloc/free
static unsigned char sbuf[512];
static int part_getPartitionTable(struct block_device* bd, part_table* part)
{
    part_raw_record* part_raw;
    int ret;
    unsigned int i;

    M_DEBUG("%s\n", __func__);

    ret = bd->read(bd, bd->sectorOffset, sbuf, 1); // read sector 0 - Disk MBR or boot sector
    if (ret < 0) {
        M_DEBUG("ERROR: part_getPartitionTable read failed %d!\n", ret);
        return -EIO;
    }

    M_DEBUG("boot signature %X %X\n", sbuf[0x1FE], sbuf[0x1FF]);
    if (sbuf[0x1FE] == 0x55 && sbuf[0x1FF] == 0xAA) {
        for (i = 0; i < 4; i++) {
            part_raw = (part_raw_record*)(sbuf + 0x01BE + (i * 16));
            if (part_getPartitionRecord(bd, part_raw, &part->record[i]) != 0)
                return 0; //Invalid record encountered, so the table is probably invalid.
        }
        return 4;
    } else {
        for (i = 0; i < 4; i++) {
            part->record[i].sid = 0;
        }
        return 0;
    }
}

//---------------------------------------------------------------------------
void part_create(struct block_device* bd, part_record* rec, unsigned int parNr)
{
    int i;

    M_DEBUG("%s\n", __func__);

    for (i = 0; i < MAX_PARTITIONS; ++i) {
        if (g_part[i].bd == NULL) {
            g_part[i].bd              = bd;
            g_part_bd[i].name         = bd->name;
            g_part_bd[i].devNr        = bd->devNr;
            g_part_bd[i].parNr        = parNr + 1;
            g_part_bd[i].sectorSize   = bd->sectorSize;
            g_part_bd[i].sectorOffset = bd->sectorOffset + rec->start;
            g_part_bd[i].sectorCount  = rec->count;
            bdm_connect_bd(&g_part_bd[i]);
            break;
        }
    }
}

//---------------------------------------------------------------------------
int part_connect(struct block_device* bd)
{
    part_table partTable;
    unsigned int i;
    int parts;
    int rval = -1;

    M_DEBUG("%s\n", __func__);

    if ((parts = part_getPartitionTable(bd, &partTable)) <= 0)
        return rval;

    for (i = 0; i < parts; i++) {
        if (partTable.record[i].sid != 0x00) {
            M_DEBUG("mount partition %d id %02x\n", i, partTable.record[i].sid);
            part_create(bd, &partTable.record[i], i);
            rval = 0;
        }
    }

    return rval;
}

//---------------------------------------------------------------------------
void part_disconnect(struct block_device* bd)
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
static int part_read(struct block_device* bd, u32 sector, void* buffer, u16 count)
{
    struct partition* part = (struct partition*)bd->priv;

    M_DEBUG("%s\n", __func__);

    if ((part == NULL) || (part->bd == NULL))
        return -1;

    return part->bd->read(part->bd, sector, buffer, count);
}

static int part_write(struct block_device* bd, u32 sector, const void* buffer, u16 count)
{
    struct partition* part = (struct partition*)bd->priv;

    M_DEBUG("%s\n", __func__);

    if ((part == NULL) || (part->bd == NULL))
        return -1;

    return part->bd->write(part->bd, sector, buffer, count);
}

static void part_flush(struct block_device* bd)
{
    struct partition* part = (struct partition*)bd->priv;

    M_DEBUG("%s\n", __func__);

    if ((part == NULL) || (part->bd == NULL))
        return;

    return part->bd->flush(part->bd);
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
    }

    g_part_fs.priv          = NULL;
    g_part_fs.name          = "MBR";
    g_part_fs.connect_bd    = part_connect;
    g_part_fs.disconnect_bd = part_disconnect;
    bdm_connect_fs(&g_part_fs);
}

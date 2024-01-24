/*-----------------------------------------------------------------------*/
/* Low level disk I/O module SKELETON for FatFs     (C)ChaN, 2019        */
/*-----------------------------------------------------------------------*/
/* If a working storage control module is available, it should be        */
/* attached to the FatFs via a glue function rather than modifying it.   */
/* This is an example of glue functions to attach various exsisting      */
/* storage control modules to the FatFs module with a defined API.       */
/*-----------------------------------------------------------------------*/

#include <bdm.h>
#include <cdvdman.h>

#include "ff.h"     /* Obtains integer types */
#include "diskio.h" /* Declarations of disk functions */

#include "fs_driver.h"

/*-----------------------------------------------------------------------*/
/* Get Drive Status                                                      */
/*-----------------------------------------------------------------------*/

DSTATUS disk_status(
    BYTE pdrv /* Physical drive number to identify the drive */
)
{
    int result;

    result = (fatfs_fs_driver_get_mounted_bd_from_index(pdrv) == NULL) ? (STA_NOINIT | STA_NODISK) : 0;

    return result;
}



/*-----------------------------------------------------------------------*/
/* Inidialize a Drive                                                    */
/*-----------------------------------------------------------------------*/

DSTATUS disk_initialize(
    BYTE pdrv /* Physical drive nmuber to identify the drive */
)
{
    int result;

    result = (fatfs_fs_driver_get_mounted_bd_from_index(pdrv) == NULL) ? (STA_NOINIT | STA_NODISK) : 0;

    return result;
}



/*-----------------------------------------------------------------------*/
/* Read Sector(s)                                                        */
/*-----------------------------------------------------------------------*/

DRESULT disk_read(
    BYTE pdrv,    /* Physical drive nmuber to identify the drive */
    BYTE *buff,   /* Data buffer to store read data */
    LBA_t sector, /* Start sector in LBA */
    UINT count    /* Number of sectors to read */
)
{
    DRESULT res;
    struct block_device *mounted_bd;

    mounted_bd = fatfs_fs_driver_get_mounted_bd_from_index(pdrv);

    if (mounted_bd == NULL) {
        return RES_NOTRDY;
    }

    res = mounted_bd->read(mounted_bd, sector, buff, count);

    return (res == count) ? RES_OK : RES_ERROR;
}



/*-----------------------------------------------------------------------*/
/* Write Sector(s)                                                       */
/*-----------------------------------------------------------------------*/

#if FF_FS_READONLY == 0

DRESULT disk_write(
    BYTE pdrv,        /* Physical drive nmuber to identify the drive */
    const BYTE *buff, /* Data to be written */
    LBA_t sector,     /* Start sector in LBA */
    UINT count        /* Number of sectors to write */
)
{
    DRESULT res;
    struct block_device *mounted_bd;

    mounted_bd = fatfs_fs_driver_get_mounted_bd_from_index(pdrv);

    if (mounted_bd == NULL) {
        return RES_NOTRDY;
    }

    res = mounted_bd->write(mounted_bd, sector, buff, count);

    return (res == count) ? RES_OK : RES_ERROR;
}

#endif


/*-----------------------------------------------------------------------*/
/* Miscellaneous Functions                                               */
/*-----------------------------------------------------------------------*/

DRESULT disk_ioctl(
    BYTE pdrv, /* Physical drive nmuber (0..) */
    BYTE cmd,  /* Control code */
    void *buff /* Buffer to send/receive control data */
)
{
    struct block_device *mounted_bd;

    mounted_bd = fatfs_fs_driver_get_mounted_bd_from_index(pdrv);

    if (mounted_bd == NULL) {
        return RES_NOTRDY;
    }

    switch (cmd) {
        case CTRL_SYNC:
            mounted_bd->flush(mounted_bd);
            break;
        case GET_SECTOR_COUNT:
            *(unsigned int *)buff = mounted_bd->sectorCount;
            break;
        case GET_SECTOR_SIZE:
            *(unsigned int *)buff = mounted_bd->sectorSize;
            break;
        case GET_BLOCK_SIZE:
            *(unsigned int *)buff = 0;
            break;
    }

    return RES_OK;
}

DWORD get_fattime(void)
{
    // ps2 specific routine to get time and date
    int year, month, day, hour, minute, sec;
    sceCdCLOCK cdtime;

    if (sceCdReadClock(&cdtime) != 0 && cdtime.stat == 0) {
        sec = btoi(cdtime.second);
        minute = btoi(cdtime.minute);
        hour = btoi(cdtime.hour);
        day = btoi(cdtime.day);
        month = btoi(cdtime.month & 0x7F); // Ignore century bit (when an old CDVDMAN is used).
        year = btoi(cdtime.year) + 2000;
    } else {
        year = 2005;
        month = 1;
        day = 6;
        hour = 14;
        minute = 12;
        sec = 10;
    }

    /* Pack date and time into a DWORD variable */
    return ((DWORD)(year - 1980) << 25) | ((DWORD)month << 21) | ((DWORD)day << 16) | ((DWORD)hour << 11) | ((DWORD)minute << 5) | ((DWORD)sec >> 1);
}
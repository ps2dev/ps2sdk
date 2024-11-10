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
 * IOP BDM (Block Device Manager) definitions.
 */

#ifndef __BDM_H__
#define __BDM_H__

#include <irx.h>
#include <types.h>

/* Event flag bits */
#define BDM_EVENT_CB_MOUNT     0x01
#define BDM_EVENT_CB_UMOUNT    0x02
#define BDM_EVENT_MOUNT        0x04
#define BDM_EVENT_DEVICE_READY 0x08

typedef void (*bdm_cb)(int event);

struct block_device
{
    // Private driver data
    void *priv;

    // Device name + number + partition number
    // Can be used to create device names like:
    //  - mass0p1
    char *name;
    unsigned int devNr;
    unsigned int parNr;
    unsigned char parId;

    unsigned int sectorSize;        // Size of a sector in bytes
    u64 sectorOffset;               // Starting sector number
    u64 sectorCount;                // Maximum number of sectors usable

    int (*read)(struct block_device *bd, u64 sector, void *buffer, u16 count);
    int (*write)(struct block_device *bd, u64 sector, const void *buffer, u16 count);
    void (*flush)(struct block_device *bd);
    int (*stop)(struct block_device *bd);

    bdm_cb device_ready_callback;
};

struct file_system
{
    // Private driver data
    void *priv;

    // Filesystem (/partition) name
    char *name;

    // mount / umount block device
    int (*connect_bd)(struct block_device *bd);
    void (*disconnect_bd)(struct block_device *bd);
};

// Exported functions
void bdm_connect_bd(struct block_device *bd);
void bdm_disconnect_bd(struct block_device *bd);
void bdm_connect_fs(struct file_system *fs);
void bdm_disconnect_fs(struct file_system *fs);
void bdm_get_bd(struct block_device **pbd, unsigned int count);
void bdm_RegisterCallback(bdm_cb cb);
void bdm_on_device_ready(int event);

#define bdm_IMPORTS_start DECLARE_IMPORT_TABLE(bdm, 1, 1)
#define bdm_IMPORTS_end   END_IMPORT_TABLE

#define I_bdm_connect_bd       DECLARE_IMPORT(4, bdm_connect_bd)
#define I_bdm_disconnect_bd    DECLARE_IMPORT(5, bdm_disconnect_bd)
#define I_bdm_connect_fs       DECLARE_IMPORT(6, bdm_connect_fs)
#define I_bdm_disconnect_fs    DECLARE_IMPORT(7, bdm_disconnect_fs)
#define I_bdm_get_bd           DECLARE_IMPORT(8, bdm_get_bd)
#define I_bdm_RegisterCallback DECLARE_IMPORT(9, bdm_RegisterCallback)
#define I_bdm_on_device_ready  DECLARE_IMPORT(10, bdm_on_device_ready)

#endif

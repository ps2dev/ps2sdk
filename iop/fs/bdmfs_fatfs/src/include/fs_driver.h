#ifndef FS_DRIVER_H
#define FS_DRIVER_H

#include <bdm.h>
#include "ff.h"

typedef struct fatfs_fs_driver_mount_info_
{
    FATFS fatfs;
    struct block_device *mounted_bd;
    int mount_status;
} fatfs_fs_driver_mount_info;

extern fatfs_fs_driver_mount_info fs_driver_mount_info[FF_VOLUMES];

extern int InitFS(void);
extern int connect_bd(struct block_device *bd);
extern void disconnect_bd(struct block_device *bd);
extern struct block_device *fatfs_fs_driver_get_mounted_bd_from_index(int mount_info_index);

#endif

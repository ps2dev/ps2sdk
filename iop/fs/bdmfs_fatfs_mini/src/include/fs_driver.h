#ifndef FS_DRIVER_H
#define FS_DRIVER_H

#include <bdm.h>
#include "ff.h"

extern FATFS fatfs;
extern struct block_device *mounted_bd;

extern int InitFS(void);
extern int connect_bd();
extern void disconnect_bd();

#endif
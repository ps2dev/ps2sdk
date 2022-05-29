#ifndef FS_DRIVER_H
#define FS_DRIVER_H

#include <bdm.h>
#include "ff.h"

extern FATFS fatfs;
extern struct block_device *mounted_bd;

#endif
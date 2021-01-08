/*
# _____     ___ ____     ___ ____
#  ____|   |    ____|   |        | |____|
# |     ___|   |____ ___|    ____| |    \    PS2DEV Open Source Project.
#-----------------------------------------------------------------------
# Copyright 2001-2004, ps2dev - http://www.ps2dev.org
# Licenced under Academic Free License version 2.0
# Review ps2sdk README & LICENSE files for further details.
*/

#ifndef __PS2SDKAPI_H__
#define __PS2SDKAPI_H__

#include <dirent.h>
#include <inttypes.h>
#include <sys/stat.h>

/** Inter-library helpers */
extern int (*_ps2sdk_close)(int);
extern int (*_ps2sdk_open)(const char*, int, ...);
extern int (*_ps2sdk_read)(int, void*, int);
extern int (*_ps2sdk_lseek)(int, int, int);
extern int64_t (*_ps2sdk_lseek64)(int, int64_t, int);
extern int (*_ps2sdk_write)(int, const void*, int);
extern int (*_ps2sdk_ioctl)(int, int, void*);
extern int (*_ps2sdk_remove)(const char*);
extern int (*_ps2sdk_rename)(const char*, const char*);
extern int (*_ps2sdk_mkdir)(const char*, int);
extern int (*_ps2sdk_rmdir)(const char*);

extern int (*_ps2sdk_stat)(const char *path, struct stat *buf);

extern DIR * (*_ps2sdk_opendir)(const char *path);
extern struct dirent * (*_ps2sdk_readdir)(DIR *dir);
extern void (*_ps2sdk_rewinddir)(DIR *dir);
extern int (*_ps2sdk_closedir)(DIR *dir);

#define PS2_CLOCKS_PER_SEC (147456000 / 256) // 576.000
#define PS2_CLOCKS_PER_MSEC (PS2_CLOCKS_PER_SEC / 1000) // 576

typedef uint64_t ps2_clock_t;
ps2_clock_t ps2_clock(void);

extern void _ps2sdk_timezone_update();

// The newlib port does not support 64bit
// this should have been defined in unistd.h
typedef int64_t off64_t;
off64_t lseek64(int fd, off64_t offset, int whence);

#endif /* __PS2SDKAPI_H__ */

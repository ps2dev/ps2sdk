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
#include <sys/stat.h>

/** Inter-library helpers */
extern int (*_ps2sdk_close)(int) __attribute__((section("data")));
extern int (*_ps2sdk_open)(const char*, int, ...) __attribute__((section("data")));
extern int (*_ps2sdk_read)(int, void*, int) __attribute__((section("data")));
extern int (*_ps2sdk_lseek)(int, int, int) __attribute__((section("data")));
extern int (*_ps2sdk_write)(int, const void*, int) __attribute__((section("data")));
extern int (*_ps2sdk_ioctl)(int, int, void*) __attribute__((section("data")));
extern int (*_ps2sdk_remove)(const char*) __attribute__((section("data")));
extern int (*_ps2sdk_rename)(const char*, const char*) __attribute__((section("data")));
extern int (*_ps2sdk_mkdir)(const char*, int) __attribute__((section("data")));
extern int (*_ps2sdk_rmdir)(const char*) __attribute__((section("data")));

extern int (*_ps2sdk_stat)(const char *path, struct stat *buf) __attribute__((section("data")));

extern DIR * (*_ps2sdk_opendir)(const char *path) __attribute__((section("data")));
extern struct dirent * (*_ps2sdk_readdir)(DIR *dir) __attribute__((section("data")));
extern void (*_ps2sdk_rewinddir)(DIR *dir) __attribute__((section("data")));
extern int (*_ps2sdk_closedir)(DIR *dir) __attribute__((section("data")));

#define PS2_CLOCKS_PER_SEC (147456000 / 256) // 576.000
#define PS2_CLOCKS_PER_MSEC (PS2_CLOCKS_PER_SEC / 1000) // 576

clock_t ps2_clock(void);

#endif /* __PS2SDKAPI_H__ */

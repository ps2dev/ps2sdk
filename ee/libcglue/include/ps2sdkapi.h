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
#include <timer.h>

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
extern int (*_ps2sdk_readlink)(const char *path, char *buf, size_t bufsiz);
extern int (*_ps2sdk_symlink)(const char *target, const char *linkpath);

extern int (*_ps2sdk_dopen)(const char *path);
extern int (*_ps2sdk_dread)(int fd, struct dirent *dir);
extern int (*_ps2sdk_dclose)(int fd);

#define PS2_CLOCKS_PER_SEC kBUSCLKBY256 // 576.000
#define PS2_CLOCKS_PER_MSEC (PS2_CLOCKS_PER_SEC / 1000) // 576

/* Disable the auto start of pthread on init for reducing binary size if not used. */
#define PS2_DISABLE_AUTOSTART_PTHREAD() \
	void __libpthreadglue_init() {} \
    void __libpthreadglue_deinit() {}

typedef uint64_t ps2_clock_t;
static inline ps2_clock_t ps2_clock(void) {
    // DEPRECATED VERSION USE INSTEAD GetTimerSystemTime
    return (ps2_clock_t)(GetTimerSystemTime() >> 8);
}

extern s64 _ps2sdk_rtc_offset_from_busclk;
extern void _libcglue_rtc_update();

// The newlib port does not support 64bit
// this should have been defined in unistd.h
typedef int64_t off64_t;
off64_t lseek64(int fd, off64_t offset, int whence);

// Functions to be used related to timezone
extern void _libcglue_timezone_update();

void ps2sdk_setTimezone(int timezone);
void ps2sdk_setDaylightSaving(int daylightSaving);

/* The fd we provide to final user aren't actually the same than IOP's fd
* so this function allow you to get actual IOP's fd from public fd
*/
int ps2sdk_get_iop_fd(int fd);
 
#endif /* __PS2SDKAPI_H__ */

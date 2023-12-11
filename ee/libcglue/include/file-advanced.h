/*
# _____     ___ ____     ___ ____
#  ____|   |    ____|   |        | |____|
# |     ___|   |____ ___|    ____| |    \    PS2DEV Open Source Project.
#-----------------------------------------------------------------------
# Copyright 2001-2004, ps2dev - http://www.ps2dev.org
# Licenced under Academic Free License version 2.0
# Review ps2sdk README & LICENSE files for further details.
*/

#ifndef __FILE_ADVANCED_H__
#define __FILE_ADVANCED_H__

#define NEWLIB_PORT_AWARE
#include <fileio.h>
#include <fileXio_rpc.h>
#include "iox_stat.h"

extern uint8_t __usingFileXIO;


static inline void swapToFileXio() {
    fileXioInit();
    __usingFileXIO = 1;
}

static inline void swapToFileIO() {
    fileXioExit();
    __usingFileXIO = 0;
}

#define FILE_SWAP_PS2SDK_FUNCTIONS() \
    uint8_t __usingFileXIO = 0; \
    int __fileXioGetstatHelper(const char *path, struct stat *buf); \
    int __fileXioDreadHelper(int fd, struct dirent *dir); \
    int __fioOpenHelper(const char* path, int flags, ...); \
    off64_t __fioLseek64Helper(int fd, off64_t offset, int whence); \
    int __fioRenameHelper(const char *oldpath, const char *newpath); \
    int __fioMkdirHelper(const char *path, int mode); \
    int __fioGetstatHelper(const char *path, struct stat *buf); \
    int __fioReadlinkHelper(const char *path, char *buf, size_t bufsize); \
    int __fioSymlinkHelper(const char *oldpath, const char *newpath); \
    int __fioDreadHelper(int fd, struct dirent *dir); \
    void _predefined_ps2sdk_close() { \
        _ps2sdk_close = __usingFileXIO ? fileXioClose : fioClose; \
    } \
    void _predefined_ps2sdk_open() { \
        _ps2sdk_open = __usingFileXIO ? fileXioOpen : __fioOpenHelper; \
    } \
    void _predefined_ps2sdk_read() { \
        _ps2sdk_read = __usingFileXIO ? fileXioRead : fioRead; \
    } \
    void _predefined_ps2sdk_lseek() { \
        _ps2sdk_lseek = __usingFileXIO ? fileXioLseek : fioLseek; \
    } \
    void _predefined_ps2sdk_lseek64() { \
        _ps2sdk_lseek64 = __usingFileXIO ? fileXioLseek64 : __fioLseek64Helper; \
    } \
    void _predefined_ps2sdk_write() { \
        _ps2sdk_write = __usingFileXIO ? fileXioWrite : fioWrite; \
    } \
    void _predefined_ps2sdk_ioctl() { \
        _ps2sdk_ioctl = __usingFileXIO ? fileXioIoctl : fioIoctl; \
    } \
    void _predefined_ps2sdk_remove() { \
        _ps2sdk_remove = __usingFileXIO ? fileXioRemove : fioRemove; \
    } \
    void _predefined_ps2sdk_rename() { \
        _ps2sdk_rename = __usingFileXIO ? fileXioRename : __fioRenameHelper; \
    } \
    void _predefined_ps2sdk_mkdir() { \
        _ps2sdk_mkdir = __usingFileXIO ? fileXioMkdir : __fioMkdirHelper; \
    } \
    void _predefined_ps2sdk_rmdir() { \
        _ps2sdk_rmdir = __usingFileXIO ? fileXioRmdir : fioRmdir; \
    } \
    void _predefined_ps2sdk_stat() { \
        _ps2sdk_stat = __usingFileXIO ? __fileXioGetstatHelper : __fioGetstatHelper; \
    } \
    void _predefined_ps2sdk_readlink() { \
        _ps2sdk_readlink = __usingFileXIO ? fileXioReadlink : __fioReadlinkHelper; \
    } \
    void _predefined_ps2sdk_symlink() { \
        _ps2sdk_symlink = __usingFileXIO ? fileXioSymlink : __fioSymlinkHelper; \
    } \
    void _predefined_ps2sdk_dopen() { \
        _ps2sdk_dopen = __usingFileXIO ? fileXioDopen : fioDopen; \
    } \
    void _predefined_ps2sdk_dclose() { \
        _ps2sdk_dclose = __usingFileXIO ? fileXioDclose : fioDclose; \
    } \
    void _predefined_ps2sdk_dread() { \
        _ps2sdk_dread = __usingFileXIO ? __fileXioDreadHelper : __fioDreadHelper; \
    } \

 
#endif /* __FILE_ADVANCED_H__ */

/*
# _____     ___ ____     ___ ____
#  ____|   |    ____|   |        | |____|
# |     ___|   |____ ___|    ____| |    \    PS2DEV Open Source Project.
#-----------------------------------------------------------------------
# Copyright 2001-2004, ps2dev - http://www.ps2dev.org
# Licenced under Academic Free License version 2.0
# Review ps2sdk README & LICENSE files for further details.
*/

/**
 * @file
 * fileXio RPC client header file
 */

#ifndef __FILEXIO_RPC_H__
#define __FILEXIO_RPC_H__

#include <fileXio.h>
#include <sys/stat.h>

#ifndef NEWLIB_PORT_AWARE
#error "Using fio/fileXio functions directly in the newlib port will lead to problems."
#error "Use posix function calls instead."
#endif

#define FXIO_WAIT		0
#define FXIO_NOWAIT		1

#define FXIO_COMPLETE	1
#define FXIO_INCOMPLETE	0

#ifdef __cplusplus
extern "C" {
#endif

extern int fileXioInit(void);
extern void fileXioExit(void);
extern void fileXioSetBlockMode(int blocking);
extern int fileXioWaitAsync(int mode, int *retVal);

extern void fileXioStop();
extern int fileXioGetDeviceList(struct fileXioDevice deviceEntry[], unsigned int req_entries);
extern int fileXioGetdir(const char* pathname, struct fileXioDirEntry DirEntry[], unsigned int req_entries);
extern int fileXioMount(const char* mountpoint, const char* blockdev, int flag);
extern int fileXioUmount(const char* mountpoint);
extern int fileXioCopyfile(const char* source, const char* dest, int mode);
extern int fileXioMkdir(const char* pathname, int mode);
extern int fileXioRmdir(const char* pathname);
extern int fileXioRemove(const char* pathname);
extern int fileXioRename(const char* source, const char* dest);
extern int fileXioSymlink(const char* source, const char* dest);
extern int fileXioReadlink(const char* source, char* buf, unsigned int buflen);
extern int fileXioChdir(const char* pathname);
extern int fileXioOpen(const char* source, int flags, ...);
extern int fileXioClose(int fd);
extern int fileXioRead(int fd, void *buf, int size);
extern int fileXioWrite(int fd, const void *buf, int size);
extern int fileXioLseek(int fd, int offset, int whence);
extern s64 fileXioLseek64(int fd, s64 offset, int whence);
extern int fileXioChStat(const char *name, iox_stat_t *stat, int mask);
extern int fileXioGetStat(const char *name, iox_stat_t *stat);
extern int fileXioFormat(const char *dev, const char *blockdev, const void *args, int arglen);
extern int fileXioSync(const char *devname, int flag);
extern int fileXioDopen(const char *name);
extern int fileXioDclose(int fd);
extern int fileXioDread(int fd, iox_dirent_t *dirent);
extern int fileXioDevctl(const char *name, int cmd, void *arg, unsigned int arglen, void *buf,unsigned int buflen);
extern int fileXioIoctl(int fd, int cmd, void *arg);
extern int fileXioIoctl2(int fd, int command, void *arg, unsigned int arglen, void *buf, unsigned int buflen);
extern int fileXioSetRWBufferSize(int size);

#ifdef __cplusplus
}
#endif

#endif /* __FILEXIO_RPC_H__ */

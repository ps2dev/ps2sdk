/*
 * fileXio_rpc.c - fileXio RPC client header file
 *
 * Copyright (c) 2003 adresd <adresd_ps2dev@yahoo.com>
 *
 * See the file LICENSE included with this distribution for licensing terms.
 */
#ifndef _FILEXIO_RPC_H
#define _FILEXIO_RPC_H

// include the common definitions
#include "fileXio.h"
#include "sys/stat.h"

#ifdef __cplusplus
extern "C" {
#endif

#define FXIO_WAIT		0
#define FXIO_NOWAIT		1

#define FXIO_COMPLETE	1
#define FXIO_INCOMPLETE	0

int fileXioInit();
void fileXioSetBlockMode(int blocking);
int fileXioWaitAsync(int mode, int *retVal);

void fileXioStop();
int fileXioGetdir(const char* pathname, struct fileXioDirEntry DirEntry[], unsigned int req_entries);
int fileXioMount(const char* mountpoint, const char* blockdev, int flag);
int fileXioUmount(const char* mountpoint);
int fileXioCopyfile(const char* source, const char* dest, int mode);
int fileXioMkdir(const char* pathname, int mode);
int fileXioRmdir(const char* pathname);
int fileXioRemove(const char* pathname);
int fileXioRename(const char* source, const char* dest);
int fileXioSymlink(const char* source, const char* dest);
int fileXioReadlink(const char* source, char* buf, int buflen);
int fileXioChdir(const char* pathname);
int fileXioOpen(const char* source, int flags, int modes);
int fileXioClose(int fd);
int fileXioRead(int fd, unsigned char *buf, int size);
int fileXioWrite(int fd, unsigned char *buf, int size);
int fileXioLseek(int fd, long offset, int whence);
int fileXioLseek64(int fd, long long offset, int whence);
int fileXioChStat(const char *name, iox_stat_t *stat, int mask);
int fileXioGetStat(const char *name, iox_stat_t *stat);
int fileXioFormat(const char *dev, const char *blockdev, const char *args, int arglen);
int fileXioSync(const char *devname, int flag);
int fileXioDopen(const char *name);
int fileXioDclose(int fd);
int fileXioDread(int fd, iox_dirent_t *dirent);
int fileXioDevctl(const char *name, int cmd, void *arg, unsigned int arglen, void *buf,unsigned int buflen);
int fileXioIoctl(int fd, int cmd, void *arg);
int fileXioIoctl2(int fd, int command, void *arg, unsigned int arglen, void *buf, unsigned int buflen);

#ifdef __cplusplus
}
#endif


#endif // _FILEXIO_H

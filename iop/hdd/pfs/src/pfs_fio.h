/*
# _____     ___ ____     ___ ____
#  ____|   |    ____|   |        | |____|
# |     ___|   |____ ___|    ____| |    \    PS2DEV Open Source Project.
#-----------------------------------------------------------------------
# Copyright 2001-2004, ps2dev - http://www.ps2dev.org
# Licenced under Academic Free License version 2.0
# Review ps2sdk README & LICENSE files for further details.
*/

#ifndef _PFS_FIO_H
#define _PFS_FIO_H

#define	PFS_FDIRO		(0x0008)  /* internal use for dopen */

///////////////////////////////////////////////////////////////////////////////
//	Function declarations

int pfsFioCheckForLastError(pfs_mount_t *pfsMount, int rv);
int pfsFioCheckFileSlot(pfs_file_slot_t *fileSlot);
pfs_mount_t *pfsFioGetMountedUnit(int unit);
void pfsFioCloseFileSlot(pfs_file_slot_t *fileSlot);

///////////////////////////////////////////////////////////////////////////////
//	I/O functions

int	pfsFioInit(iomanX_iop_device_t *f);
int	pfsFioDeinit(iomanX_iop_device_t *f);
int	pfsFioFormat(iomanX_iop_file_t *, const char *dev, const char *blockdev, void *arg, int arglen);
int	pfsFioOpen(iomanX_iop_file_t *f, const char *name, int flags, int mode);
int	pfsFioClose(iomanX_iop_file_t *f);
int	pfsFioRead(iomanX_iop_file_t *f, void *buf, int size);
int	pfsFioWrite(iomanX_iop_file_t *f, void *buf, int size);
int	pfsFioLseek(iomanX_iop_file_t *f, int pos, int whence);
int	pfsFioRemove(iomanX_iop_file_t *f, const char *name);
int	pfsFioMkdir(iomanX_iop_file_t *f, const char *path, int mode);
int	pfsFioRmdir(iomanX_iop_file_t *f, const char *path);
int	pfsFioDopen(iomanX_iop_file_t *f, const char *name);
int	pfsFioDclose(iomanX_iop_file_t *f);
int	pfsFioDread(iomanX_iop_file_t *f, iox_dirent_t *buf);
int	pfsFioGetstat(iomanX_iop_file_t *f, const char *name, iox_stat_t *stat);
int	pfsFioChstat(iomanX_iop_file_t *f, const char *name, iox_stat_t *stat, unsigned int statmask);
int pfsFioRename(iomanX_iop_file_t *f, const char *old, const char *new);
int pfsFioChdir(iomanX_iop_file_t *f, const char *name);
int pfsFioSync(iomanX_iop_file_t *f, const char *dev, int flag);
int pfsFioMount(iomanX_iop_file_t *f, const char *fsname, const char *devname, int flag, void *arg, int arglen);
int pfsFioUmount(iomanX_iop_file_t *f, const char *fsname);
s64 pfsFioLseek64(iomanX_iop_file_t *f, s64 offset, int whence);
int pfsFioSymlink(iomanX_iop_file_t *f, const char *old, const char *new);
int pfsFioReadlink(iomanX_iop_file_t *f, const char *path, char *buf, unsigned int buflen);

#endif /* _PFS_FIO_H */

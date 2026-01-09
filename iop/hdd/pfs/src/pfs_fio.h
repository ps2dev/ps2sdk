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
//  Function declarations

extern int pfsFioCheckForLastError(pfs_mount_t *pfsMount, int rv);
extern int pfsFioCheckFileSlot(pfs_file_slot_t *fileSlot);
extern pfs_mount_t *pfsFioGetMountedUnit(int unit);
extern void pfsFioCloseFileSlot(pfs_file_slot_t *fileSlot);

///////////////////////////////////////////////////////////////////////////////
//  I/O functions

extern int pfsFioInit(iomanX_iop_device_t *f);
extern int pfsFioDeinit(iomanX_iop_device_t *f);
extern int pfsFioFormat(iomanX_iop_file_t *, const char *dev, const char *blockdev, void *arg, int arglen);
extern int pfsFioOpen(iomanX_iop_file_t *f, const char *name, int flags, int mode);
extern int pfsFioClose(iomanX_iop_file_t *f);
extern int pfsFioRead(iomanX_iop_file_t *f, void *buf, int size);
extern int pfsFioWrite(iomanX_iop_file_t *f, void *buf, int size);
extern int pfsFioLseek(iomanX_iop_file_t *f, int pos, int whence);
extern int pfsFioRemove(iomanX_iop_file_t *f, const char *name);
extern int pfsFioMkdir(iomanX_iop_file_t *f, const char *path, int mode);
extern int pfsFioRmdir(iomanX_iop_file_t *f, const char *path);
extern int pfsFioDopen(iomanX_iop_file_t *f, const char *name);
extern int pfsFioDclose(iomanX_iop_file_t *f);
extern int pfsFioDread(iomanX_iop_file_t *f, iox_dirent_t *buf);
extern int pfsFioGetstat(iomanX_iop_file_t *f, const char *name, iox_stat_t *stat);
extern int pfsFioChstat(iomanX_iop_file_t *f, const char *name, iox_stat_t *stat, unsigned int statmask);
extern int pfsFioRename(iomanX_iop_file_t *f, const char *old, const char *new_);
extern int pfsFioChdir(iomanX_iop_file_t *f, const char *name);
extern int pfsFioSync(iomanX_iop_file_t *f, const char *dev, int flag);
extern int pfsFioMount(iomanX_iop_file_t *f, const char *fsname, const char *devname, int flag, void *arg, int arglen);
extern int pfsFioUmount(iomanX_iop_file_t *f, const char *fsname);
extern s64 pfsFioLseek64(iomanX_iop_file_t *f, s64 offset, int whence);
extern int pfsFioSymlink(iomanX_iop_file_t *f, const char *old, const char *new_);
extern int pfsFioReadlink(iomanX_iop_file_t *f, const char *path, char *buf, unsigned int buflen);

#endif /* _PFS_FIO_H */

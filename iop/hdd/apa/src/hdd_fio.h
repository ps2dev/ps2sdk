/*
# _____     ___ ____     ___ ____
#  ____|   |    ____|   |        | |____|
# |     ___|   |____ ___|    ____| |    \    PS2DEV Open Source Project.
#-----------------------------------------------------------------------
# Copyright 2001-2004, ps2dev - http://www.ps2dev.org
# Licenced under Academic Free License version 2.0
# Review ps2sdk README & LICENSE files for further details.
*/

#ifndef _HDD_FIO_H
#define _HDD_FIO_H

// I/O functions
extern int hddInit(iomanX_iop_device_t *f);
extern int hddDeinit(iomanX_iop_device_t *f);
extern int hddFormat(iomanX_iop_file_t *f, const char *dev, const char *blockdev, void *arg, int arglen);
extern int hddOpen(iomanX_iop_file_t *f, const char *name, int flags, int mode);
extern int hddClose(iomanX_iop_file_t *f);
extern int hddRead(iomanX_iop_file_t *f, void *buf, int size);
extern int hddWrite(iomanX_iop_file_t *f, void *buf, int size);
extern int hddLseek(iomanX_iop_file_t *f, int post, int whence);
extern int hddIoctl2(iomanX_iop_file_t *f, int request, void *argp, unsigned int arglen, void *bufp, unsigned int buflen);
extern int hddRemove(iomanX_iop_file_t *f, const char *name);
extern int hddDopen(iomanX_iop_file_t *f, const char *name);
extern int hddDread(iomanX_iop_file_t *f, iox_dirent_t *dirent);
extern int hddGetStat(iomanX_iop_file_t *f, const char *name, iox_stat_t *stat);
extern int hddReName(iomanX_iop_file_t *f, const char *oldname, const char *newname);
extern int hddDevctl(iomanX_iop_file_t *f, const char *devname, int cmd, void *arg, unsigned int arglen, void *bufp, unsigned int buflen);
#ifdef APA_USE_IOMANX
extern int hddMount(iomanX_iop_file_t *f, const char *fsname, const char *devname, int flag, void *arg, int arglen);
extern int hddUmount(iomanX_iop_file_t *f, const char *fsname);
#else
#define hddMount IOMANX_RETURN_VALUE(EPERM)
#define hddUmount IOMANX_RETURN_VALUE(EPERM)
#endif

#endif /* _HDD_FIO_H */

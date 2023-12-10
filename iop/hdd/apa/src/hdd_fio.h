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
int hddInit(iomanX_iop_device_t *f);
int hddDeinit(iomanX_iop_device_t *f);
int hddFormat(iomanX_iop_file_t *f, const char *dev, const char *blockdev, void *arg, int arglen);
int hddOpen(iomanX_iop_file_t *f, const char *name, int flags, int mode);
int hddClose(iomanX_iop_file_t *f);
int hddRead(iomanX_iop_file_t *f, void *buf, int size);
int hddWrite(iomanX_iop_file_t *f, void *buf, int size);
int hddLseek(iomanX_iop_file_t *f, int post, int whence);
int hddIoctl2(iomanX_iop_file_t *f, int request, void *argp, unsigned int arglen, void *bufp, unsigned int buflen);
int hddRemove(iomanX_iop_file_t *f, const char *name);
int hddDopen(iomanX_iop_file_t *f, const char *name);
int hddDread(iomanX_iop_file_t *f, iox_dirent_t *dirent);
int hddGetStat(iomanX_iop_file_t *f, const char *name, iox_stat_t *stat);
int hddReName(iomanX_iop_file_t *f, const char *oldname, const char *newname);
int hddDevctl(iomanX_iop_file_t *f, const char *devname, int cmd, void *arg, unsigned int arglen, void *bufp, unsigned int buflen);
#ifdef APA_USE_IOMANX
int hddMount(iomanX_iop_file_t *f, const char *fsname, const char *devname, int flag, void *arg, int arglen);
int hddUmount(iomanX_iop_file_t *f, const char *fsname);
#else
#define hddMount ((void*)&hddUnsupported)
#define hddUmount ((void*)&hddUnsupported)
#endif

int hddUnsupported(iomanX_iop_file_t *f);

#endif /* _HDD_FIO_H */

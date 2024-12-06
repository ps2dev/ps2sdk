/*
# _____     ___ ____     ___ ____
#  ____|   |    ____|   |        | |____|
# |     ___|   |____ ___|    ____| |    \    PS2DEV Open Source Project.
#-----------------------------------------------------------------------
# (C)2001, Gustavo Scotti (gustavo@scotti.com)
# (c) 2003 Marcus R. Brown (mrbrown@0xd6.org)
# Licenced under Academic Free License version 2.0
# Review ps2sdk README & LICENSE files for further details.
*/

/**
 * @file
 * EE FILE IO handling
 */

#ifndef __FILEIO_H__
#define __FILEIO_H__

#include <io_common.h>

#ifndef NEWLIB_PORT_AWARE
#error "Using fio/fileXio functions directly in the newlib port will lead to problems."
#error "Use posix function calls instead."
#endif

#define FIO_WAIT   0
#define FIO_NOWAIT 1

#define FIO_COMPLETE   1
#define FIO_INCOMPLETE 0

#define EOF (-1)

#ifdef __cplusplus
extern "C" {
#endif

extern int fioInit(void);
extern void fioExit(void);
extern int fioOpen(const char *fname, int mode);
extern int fioClose(int fd);
extern int fioRead(int fd, void *buff, int buff_size);
extern int fioWrite(int fd, const void *buff, int buff_size);
extern int fioLseek(int fd, int offset, int whence);
extern int fioMkdir(const char *dirname);
extern int fioPutc(int fd, int c);
extern int fioGetc(int fd);
extern int fioGets(int fd, char *buff, int n);
extern void fioSetBlockMode(int blocking);
extern int fioSync(int mode, int *retVal);
extern int fioIoctl(int fd, int request, void *data);
extern int fioDopen(const char *name);
extern int fioDclose(int fd);
extern int fioDread(int fd, io_dirent_t *buf);           // Warning! (*)
extern int fioGetstat(const char *name, io_stat_t *buf); // Warning! (*)
extern int fioChstat(const char *name, io_stat_t *buf, unsigned int cbit);
extern int fioRemove(const char *name); // Warning! (**)
extern int fioFormat(const char *name);
extern int fioRmdir(const char *dirname);

/*	* Function is unstable; does not suspend interrupts prior to performing DMA transfers on the IOP side.
    ** Function is broken; falls through to the next case (mkdir) upon completion.

    Patches are available to fix these issues. Otherwise, please use fileXio instead.	*/

#ifdef __cplusplus
}
#endif

#endif /* __FILEIO_H__ */

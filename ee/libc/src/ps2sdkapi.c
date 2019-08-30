/*
# _____     ___ ____     ___ ____
#  ____|   |    ____|   |        | |____|
# |     ___|   |____ ___|    ____| |    \    PS2DEV Open Source Project.
#-----------------------------------------------------------------------
# Copyright 2001-2004, ps2dev - http://www.ps2dev.org
# Licenced under Academic Free License version 2.0
# Review ps2sdk README & LICENSE files for further details.
*/

#include <stdarg.h>
#include <errno.h>
#include <string.h>
#include <fileio.h>
#include <kernel.h>

#include <time.h>
#include <sys/time.h>
#include <sys/times.h>
#include <sys/stat.h>
#include <sys/types.h>


extern void *ps2_sbrk(size_t increment);
extern clock_t ps2_clock(void);


/* the present working directory variable. */
char __direct_pwd[256] = "";


int fioRename(const char *old, const char *new) {
  return -ENOSYS;
}

int fioMkdirHelper(const char *path, int mode) {
  // Old fio mkdir has no mode argument
  return fioMkdir(path);
}

int (*_ps2sdk_close)(int) = fioClose;
int (*_ps2sdk_open)(const char*, int) = fioOpen;
int (*_ps2sdk_read)(int, void*, int) = fioRead;
int (*_ps2sdk_lseek)(int, int, int) = fioLseek;
int (*_ps2sdk_write)(int, const void*, int) = fioWrite;
int (*_ps2sdk_remove)(const char*) = fioRemove;
int (*_ps2sdk_rename)(const char*, const char*) = fioRename;
int (*_ps2sdk_mkdir)(const char*, int) = fioMkdirHelper;

#define IOP_O_RDONLY       0x0001
#define IOP_O_WRONLY       0x0002
#define IOP_O_RDWR         0x0003
#define IOP_O_DIROPEN      0x0008  // Internal use for dopen
#define IOP_O_NBLOCK       0x0010
#define IOP_O_APPEND       0x0100
#define IOP_O_CREAT        0x0200
#define IOP_O_TRUNC        0x0400
#define IOP_O_EXCL         0x0800
#define IOP_O_NOWAIT       0x8000

int open(const char *buf, int flags, ...) {
	int iop_flags = 0;

	// newlib frags differ from iop flags
	if ((flags & 3) == O_RDONLY) iop_flags |= IOP_O_RDONLY;
	if ((flags & 3) == O_WRONLY) iop_flags |= IOP_O_WRONLY;
	if ((flags & 3) == O_RDWR  ) iop_flags |= IOP_O_RDWR;
	if (flags & O_NONBLOCK)      iop_flags |= IOP_O_NBLOCK;
	if (flags & O_APPEND)        iop_flags |= IOP_O_APPEND;
	if (flags & O_CREAT)         iop_flags |= IOP_O_CREAT;
	if (flags & O_TRUNC)         iop_flags |= IOP_O_TRUNC;
	if (flags & O_EXCL)          iop_flags |= IOP_O_EXCL;
	//if (flags & O_???)           iop_flags |= IOP_O_NOWAIT;

	return _ps2sdk_open(buf, iop_flags);
}

int close(int fd) {
	return _ps2sdk_close(fd);
}

int read(int fd, void *buf, size_t nbytes) {
	return _ps2sdk_read(fd, buf, nbytes);
}

int write(int fd, const void *buf, size_t nbytes) {
	return _ps2sdk_write(fd, buf, nbytes);
}

int isatty(int fd) {
	return 1;
}

int fstat(int fd, struct stat *buf) {
	buf->st_mode = S_IFCHR;       /* Always pretend to be a tty */
	buf->st_blksize = 0;

	return 0;
}

off_t lseek(int fd, off_t offset, int whence)
{
	return _ps2sdk_lseek(fd, offset, whence);
}

int chdir(const char *path) {
    strcpy(__direct_pwd, path);
    return 0;
}

int mkdir(const char *path, mode_t mode) {
    return _ps2sdk_mkdir(path, mode);
}

int link(const char *old, const char *new) {
    return _ps2sdk_rename(old, new);
}

int unlink(const char *path) {
    return _ps2sdk_remove(path);
}

char *getcwd(char *buf, size_t len) {
	strncpy(buf, __direct_pwd, len);
	return buf;
}

int getpid(void) {
	return GetThreadId();
}

int kill(int pid, int sig) {
#if 0 // needs to be tested first
	// null signal: do error checking on pid only
	if (sig == 0)
		return pid == getpid() ? 0 : -1;

	if (pid == getpid()) {
		ExitDeleteThread();
		// NOTE: ExitDeleteThread does not return
		return 0;
	}
#endif
	// FIXME: set errno
	return -1;
}

void * sbrk(ptrdiff_t incr) {
	return ps2_sbrk(incr);
}

int gettimeofday(struct timeval *__p, struct timezone *__z) {
	return -1;
}

clock_t times(struct tms *buffer) {
	clock_t clk = ps2_clock();

	if (buffer != NULL) {
		buffer->tms_utime  = clk;
		buffer->tms_stime  = 0;
		buffer->tms_cutime = clk;
		buffer->tms_cutime = 0;
	}

	return clk;
}


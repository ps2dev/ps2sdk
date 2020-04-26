/*
# _____     ___ ____     ___ ____
#  ____|   |    ____|   |        | |____|
# |     ___|   |____ ___|    ____| |    \    PS2DEV Open Source Project.
#-----------------------------------------------------------------------
# Copyright 2001-2004, ps2dev - http://www.ps2dev.org
# Licenced under Academic Free License version 2.0
# Review ps2sdk README & LICENSE files for further details.
*/

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <errno.h>
#include <string.h>
#include <kernel.h>
#include <malloc.h>
#include <sio.h>

#include <time.h>
#include <sys/time.h>
#include <sys/times.h>
#include <sys/stat.h>
#include <sys/types.h>

// Include all integer types for compile time checking of:
// - compiler (gcc)
// - libc (newlib)
#include <stdint.h>
#include <inttypes.h>
#include <tamtypes.h>

#define NEWLIB_PORT_AWARE
#include "fileio.h"
#include "io_common.h"
#include "iox_stat.h"
#include "ps2sdkapi.h"


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

static time_t io_to_posix_time(const unsigned char *ps2time)
{
        struct tm tim;
        tim.tm_sec  = ps2time[1];
        tim.tm_min  = ps2time[2];
        tim.tm_hour = ps2time[3];
        tim.tm_mday = ps2time[4];
        tim.tm_mon  = ps2time[5] - 1;
        tim.tm_year = ((u16)ps2time[6] | ((u16)ps2time[7] << 8)) - 1900;
        return mktime(&tim);
}

static mode_t io_to_posix_mode(unsigned int ps2mode)
{
        mode_t posixmode = 0;
        if (ps2mode & FIO_SO_IFREG) posixmode |= S_IFREG;
        if (ps2mode & FIO_SO_IFDIR) posixmode |= S_IFDIR;
        if (ps2mode & FIO_SO_IROTH) posixmode |= S_IRUSR|S_IRGRP|S_IROTH;
        if (ps2mode & FIO_SO_IWOTH) posixmode |= S_IWUSR|S_IWGRP|S_IWOTH;
        if (ps2mode & FIO_SO_IXOTH) posixmode |= S_IXUSR|S_IXGRP|S_IXOTH;
        return posixmode;
}

static int fioGetstatHelper(const char *path, struct stat *buf) {
        io_stat_t fiostat;

        if (fioGetstat(path, &fiostat) < 0) {
                //errno = ENOENT;
                return -1;
        }

        buf->st_dev = 0;
        buf->st_ino = 0;
        buf->st_mode = io_to_posix_mode(fiostat.mode);
        buf->st_nlink = 0;
        buf->st_uid = 0;
        buf->st_gid = 0;
        buf->st_rdev = 0;
        buf->st_size = ((off_t)fiostat.hisize << 32) | (off_t)fiostat.size;
        buf->st_atime = io_to_posix_time(fiostat.atime);
        buf->st_mtime = io_to_posix_time(fiostat.mtime);
        buf->st_ctime = io_to_posix_time(fiostat.ctime);
        buf->st_blksize = 16*1024;
        buf->st_blocks = buf->st_size / 512;

        return 0;
}

static DIR *fioOpendirHelper(const char *path)
{
	int dd;
	DIR *dir;

	dd = fioDopen(path);
	if (dd < 0) {
		//errno = ENOENT;
		return NULL;
	}

	dir = malloc(sizeof(DIR));
        dir->dd_fd = dd;
        dir->dd_loc = 0;
        dir->dd_size = 0;
        dir->dd_buf = malloc(sizeof(struct dirent) + 255);
        dir->dd_len = 0;
        dir->dd_seek = 0;

	return dir;
}

static struct dirent *fioReaddirHelper(DIR *dir)
{
	int rv;
        struct dirent *de;
        io_dirent_t fiode;

	if(dir == NULL) {
		//errno = EBADF;
		return NULL;
	}

        de = (struct dirent *)dir->dd_buf;
        rv = fioDread(dir->dd_fd, &fiode);
	if (rv <= 0) {
		return NULL;
	}

        de->d_ino = 0;
        de->d_off = 0;
        de->d_reclen = 0;
	strncpy(de->d_name, fiode.name, 255);
	de->d_name[255] = 0;

	return de;
}

static void fioRewinddirHelper(DIR *dir)
{
	printf("rewinddir not implemented\n");
}

static int fioClosedirHelper(DIR *dir)
{
	int rv;

	if(dir == NULL) {
		//errno = EBADF;
		return -1;
	}

	rv = fioDclose(dir->dd_fd); // Check return value?
        free(dir->dd_buf);
        free(dir);
	return 0;
}

int (*_ps2sdk_close)(int) = fioClose;
int (*_ps2sdk_open)(const char*, int, ...) = (void *)fioOpen;
int (*_ps2sdk_read)(int, void*, int) = fioRead;
int (*_ps2sdk_lseek)(int, int, int) = fioLseek;
int (*_ps2sdk_write)(int, const void*, int) = fioWrite;
int (*_ps2sdk_ioctl)(int, int, void*) = fioIoctl;
int (*_ps2sdk_remove)(const char*) = fioRemove;
int (*_ps2sdk_rename)(const char*, const char*) = fioRename;
int (*_ps2sdk_mkdir)(const char*, int) = fioMkdirHelper;
int (*_ps2sdk_rmdir)(const char*) = fioRmdir;

int (*_ps2sdk_stat)(const char *path, struct stat *buf) = fioGetstatHelper;

DIR * (*_ps2sdk_opendir)(const char *path) = fioOpendirHelper;
struct dirent * (*_ps2sdk_readdir)(DIR *dir) = fioReaddirHelper;
void (*_ps2sdk_rewinddir)(DIR *dir) = fioRewinddirHelper;
int (*_ps2sdk_closedir)(DIR *dir) = fioClosedirHelper;

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

#if INT_MAX != 0x7fffffffL
	#error "INT_MAX != 0x7fffffffL"
#endif
#ifndef LONG_MAX
	#error "LONG_MAX not defined"
#endif
#if LONG_MAX == 0x7fffffffL
	#error "LONG_MAX == 0x7fffffffL"
#endif
#if LONG_MAX != 9223372036854775807L
	#error "LONG_MAX != 9223372036854775807L"
#endif

#define ct_assert(e) {enum { ct_assert_value = 1/(!!(e)) };}
void compile_time_check() {
	// Compiler
	ct_assert(sizeof(unsigned char)==1);
	ct_assert(sizeof(unsigned short)==2);
	ct_assert(sizeof(unsigned int)==4);
	ct_assert(sizeof(unsigned long)==8);
	ct_assert(sizeof(unsigned int __attribute__(( mode(TI) )))==16);
	// Defined in tamtypes.h (ps2sdk)
	ct_assert(sizeof(u8)==1);
	ct_assert(sizeof(u16)==2);
	ct_assert(sizeof(u32)==4);
	ct_assert(sizeof(u64)==8);
	ct_assert(sizeof(u128)==16);
	// Defined in inttypes.h/stdint.h (newlib)
	ct_assert(sizeof(uint8_t)==1);
	ct_assert(sizeof(uint16_t)==2);
	ct_assert(sizeof(uint32_t)==4);
	ct_assert(sizeof(uint64_t)==8);
}

int _open(const char *buf, int flags, ...) {
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

int _close(int fd) {
	return _ps2sdk_close(fd);
}

int _read(int fd, void *buf, size_t nbytes) {
	return _ps2sdk_read(fd, buf, nbytes);
}

int _write(int fd, const void *buf, size_t nbytes) {
	// HACK: stdout and strerr to serial
	//if ((fd==1) || (fd==2))
	//	return sio_write((void *)buf, nbytes);

	return _ps2sdk_write(fd, buf, nbytes);
}

int _fstat(int fd, struct stat *buf) {
	if (fd >=0 && fd <= 1) {
		// Character device
		buf->st_mode = S_IFCHR;
		buf->st_blksize = 0;
	}
	else {
		// Block device
		buf->st_mode = S_IFBLK;
		buf->st_blksize = 16*1024;
	}

	return 0;
}

int _stat(const char *path, struct stat *buf) {
        return _ps2sdk_stat(path, buf);
}

int access(const char *fn, int flags) {
	struct stat s;
	if (stat(fn, &s))
		return -1;
	if (s.st_mode & S_IFDIR)
		return 0;
	if (flags & W_OK) {
		if (s.st_mode & S_IWRITE)
			return 0;
		return -1;
	}
	return 0;
}

DIR *opendir(const char *path)
{
    return _ps2sdk_opendir(path);
}

struct dirent *readdir(DIR *dir)
{
    return _ps2sdk_readdir(dir);
}

void rewinddir(DIR *dir)
{
    return _ps2sdk_rewinddir(dir);
}

int closedir(DIR *dir)
{
    return _ps2sdk_closedir(dir);
}

int isatty(int fd) {
	struct stat buf;

	if (_fstat (fd, &buf) < 0) {
		//errno = EBADF;
		return 0;
	}
	if (S_ISCHR (buf.st_mode))
		return 1;
	//errno = ENOTTY;
	return 0;
}

off_t _lseek(int fd, off_t offset, int whence)
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

int rmdir(const char *path) {
    return _ps2sdk_rmdir(path);
}

int _link(const char *old, const char *new) {
    return _ps2sdk_rename(old, new);
}

int _unlink(const char *path) {
    return _ps2sdk_remove(path);
}

char *getcwd(char *buf, size_t len) {
	strncpy(buf, __direct_pwd, len);
	return buf;
}

int _getpid(void) {
	return GetThreadId();
}

int _kill(int pid, int sig) {
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

void * _sbrk(size_t incr) {
	return ps2_sbrk(incr);
}

/*
 * newlib function, unfortunately depends on the 'cdvd' library.
 * In libc there is a dummy   'time' function declared as WEAK.
 * In cdvd there is a working 'time' function declared as STRONG
 * Include libcdvd if you need to use the time function.
 */
time_t time(time_t *t) __attribute__((weak));
time_t time(time_t *t)
{
        printf("ERROR: include libcdvd when using the time function\n");

        if(t != NULL)
                *t = -1;
	return -1;
}

/*
 * Implement in terms of time, which means we can't
 * return the microseconds.
 */
int _gettimeofday(struct timeval *tv, struct timezone *tz) {
        if (tz) {
                /* Timezone not supported for gettimeofday */
                tz->tz_minuteswest = 0;
                tz->tz_dsttime = 0;
        }

        tv->tv_usec = 0;
        tv->tv_sec = time(0);

        return 0;
}

clock_t _times(struct tms *buffer) {
	clock_t clk = ps2_clock() / (PS2_CLOCKS_PER_SEC / CLOCKS_PER_SEC);

	if (buffer != NULL) {
		buffer->tms_utime  = clk;
		buffer->tms_stime  = 0;
		buffer->tms_cutime = 0;
		buffer->tms_cutime = 0;
	}

	return clk;
}

long int random(void)
{
        return rand();
}

void srandom(unsigned int seed)
{
        srand(seed);
}

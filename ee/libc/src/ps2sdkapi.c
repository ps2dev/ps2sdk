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
#include <limits.h>
#include <inttypes.h>
#include <tamtypes.h>

#define NEWLIB_PORT_AWARE
#include "fileio.h"
#include "io_common.h"
#include "iox_stat.h"
#include "ps2sdkapi.h"


extern void * _end;

#ifdef F___direct_pwd
/* the present working directory variable. */
char __direct_pwd[256] = "";
#else
extern char __direct_pwd[256];
#endif

#ifdef F___transform_errno
int __transform_errno(int res) {
	/* On error, -1 is returned and errno is set to indicate the error. */
	if (res < 0) {
		errno = -res;
		return -1;
	}
	return res;
}
#else
int __transform_errno(int errorCode);
#endif

#ifdef F___fill_stat
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

void __fill_stat(struct stat *stat, const io_stat_t *fiostat)
{
        stat->st_dev = 0;
        stat->st_ino = 0;
        stat->st_mode = io_to_posix_mode(fiostat->mode);
        stat->st_nlink = 0;
        stat->st_uid = 0;
        stat->st_gid = 0;
        stat->st_rdev = 0;
        stat->st_size = ((off_t)fiostat->hisize << 32) | (off_t)fiostat->size;
        stat->st_atime = io_to_posix_time(fiostat->atime);
        stat->st_mtime = io_to_posix_time(fiostat->mtime);
        stat->st_ctime = io_to_posix_time(fiostat->ctime);
        stat->st_blksize = 16*1024;
        stat->st_blocks = stat->st_size / 512;
}
#else
void __fill_stat(struct stat *stat, const io_stat_t *fiostat);
#endif

#ifdef F__ps2sdk_ioctl
int (*_ps2sdk_ioctl)(int, int, void*) = fioIoctl;
#endif

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
#if LONG_MAX != 0x7fffffffL
	#error "LONG_MAX != 0x7fffffffL"
#endif

#define ct_assert(e) {enum { ct_assert_value = 1/(!!(e)) };}
#ifdef F_compile_time_check
void compile_time_check() {
	// Compiler (ABI n32)
	ct_assert(sizeof(unsigned char)==1);
	ct_assert(sizeof(unsigned short)==2);
	ct_assert(sizeof(unsigned int)==4);
	ct_assert(sizeof(unsigned long)==4);
	ct_assert(sizeof(unsigned long long)==8);
	ct_assert(sizeof(unsigned int __attribute__(( mode(TI) )))==16);
	ct_assert(sizeof(void *)==4);

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
#endif

#ifdef F__open
/* Normalize a pathname by removing . and .. components, duplicated /, etc. */
static char* normalize_path(const char *path_name)
{
	int i, j;
	int first, next;
	static char out[255];

	/* First copy the path into our temp buffer */
	strcpy(out, path_name);
	/* Then append "/" to make the rest easier */
	strcat(out,"/");

	/* Convert "//" to "/" */
	for(i=0; out[i+1]; i++) {
		if(out[i]=='/' && out[i+1]=='/') {
			for(j=i+1; out[j]; j++)
					out[j] = out[j+1];
			i--;
		;}
	}

	/* Convert "/./" to "/" */
	for(i=0; out[i] && out[i+1] && out[i+2]; i++) {
		if(out[i]=='/' && out[i+1]=='.' && out[i+2]=='/') {
			for(j=i+1; out[j]; j++)
					out[j] = out[j+2];
			i--;
		}
	}

	/* Convert "/path/../" to "/" until we can't anymore.  Also convert leading
	 * "/../" to "/" */
	first = next = 0;
	while(1) {
		/* If a "../" follows, remove it and the parent */
		if(out[next+1] && out[next+1]=='.' &&
			out[next+2] && out[next+2]=='.' &&
			out[next+3] && out[next+3]=='/') {
			for(j=0; out[first+j+1]; j++)
				out[first+j+1] = out[next+j+4];
			first = next = 0;
			continue;
		}

		/* Find next slash */
		first = next;
		for(next=first+1; out[next] && out[next] != '/'; next++)
			continue;
		if(!out[next]) break;
	}

	/* Remove trailing "/" */
	for(i=1; out[i]; i++)
		continue;
	if(i >= 1 && out[i-1] == '/')
		out[i-1] = 0;

	return (char*)out;
}

static int isCdromPath(const char *path)
{
	return !strncmp(path, "cdrom0:", 7) || !strncmp(path, "cdrom:", 6);
}

int (*_ps2sdk_open)(const char*, int, ...) = (void *)fioOpen;

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

	char *t_fname = normalize_path(buf);
	char b_fname[FILENAME_MAX];

	if (!strchr(buf, ':')) { // filename doesn't contain device
		t_fname = b_fname;
		if (buf[0] == '/' || buf[0] == '\\') {   // does it contain root ?
			char *device_end = strchr(__direct_pwd, ':');
			if (device_end) {      // yes, let's strip pwd a bit to keep device only
				strncpy(b_fname, __direct_pwd, device_end - __direct_pwd);
				strcpy(b_fname + (device_end - __direct_pwd), buf);
			} else {               // but pwd doesn't contain any device, let's default to host
				strcpy(b_fname, "host:");
				strcpy(b_fname + 5, buf);
			}
		} else {                 // otherwise, it's relative directory, let's copy pwd straight
			int b_fname_len = strlen(__direct_pwd);
			if (!strchr(__direct_pwd, ':')) { // check if pwd contains device name
				strcpy(b_fname, "host:");
				strcpy(b_fname + 5, __direct_pwd);
				if (!(__direct_pwd[b_fname_len - 1] == '/' || __direct_pwd[b_fname_len - 1] == '\\')) { // does it has trailing slash ?
					if(isCdromPath(b_fname)) {
						b_fname[b_fname_len + 5] = '\\';
						b_fname_len++;
					} else {
						b_fname[b_fname_len + 5] = '/';
						b_fname_len++;
					}
				}
				b_fname_len += 5;
				strcpy(b_fname + b_fname_len, buf);
			} else {                          // device name is here
				if (b_fname_len) {
				strcpy(b_fname, __direct_pwd);
				if (!(b_fname[b_fname_len - 1] == '/' || b_fname[b_fname_len - 1] == '\\')) {
					if(isCdromPath(b_fname)) {
						b_fname[b_fname_len] = '\\';
						b_fname_len++;
					} else {
						b_fname[b_fname_len] = '/';
						b_fname_len++;
					}
				}
				strcpy(b_fname + b_fname_len, buf);
				}
			}
		}
	}

	return __transform_errno(_ps2sdk_open(t_fname, iop_flags));
}
#endif

#ifdef F__close
int (*_ps2sdk_close)(int) = fioClose;

int _close(int fd) {
	return __transform_errno(_ps2sdk_close(fd));
}
#endif

#ifdef F__read
int (*_ps2sdk_read)(int, void*, int) = fioRead;

int _read(int fd, void *buf, size_t nbytes) {
	return __transform_errno(_ps2sdk_read(fd, buf, nbytes));
}
#endif

#ifdef F__write
int (*_ps2sdk_write)(int, const void*, int) = fioWrite;

int _write(int fd, const void *buf, size_t nbytes) {
	// HACK: stdout and strerr to serial
	//if ((fd==1) || (fd==2))
	//	return sio_write((void *)buf, nbytes);

	return __transform_errno(_ps2sdk_write(fd, buf, nbytes));
}
#endif

#ifdef F__fstat
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
#else
int _fstat(int fd, struct stat *buf);
#endif

#ifdef F__stat
static int fioGetstatHelper(const char *path, struct stat *buf) {
        io_stat_t fiostat;

        if (fioGetstat(path, &fiostat) < 0) {
			errno = ENOENT;
			return -1;
        }

        __fill_stat(buf, &fiostat);

        return 0;
}

int (*_ps2sdk_stat)(const char *path, struct stat *buf) = fioGetstatHelper;

int _stat(const char *path, struct stat *buf) {
    return _ps2sdk_stat(path, buf);
}
#endif

#ifdef F_access
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
#endif

#ifdef F_opendir
static DIR *fioOpendirHelper(const char *path)
{
	int dd;
	DIR *dir;

	dd = fioDopen(path);
	if (dd < 0) {
		errno = ENOENT;
		return NULL;
	}

	dir = malloc(sizeof(DIR));
        dir->dd_fd = dd;
        dir->dd_buf = malloc(sizeof(struct dirent));

	return dir;
}

DIR * (*_ps2sdk_opendir)(const char *path) = fioOpendirHelper;

DIR *opendir(const char *path)
{
    return _ps2sdk_opendir(path);
}
#endif

#ifdef F_readdir
static struct dirent *fioReaddirHelper(DIR *dir)
{
	int rv;
        struct dirent *de;
        io_dirent_t fiode;

	if(dir == NULL) {
		errno = EBADF;
		return NULL;
	}

        de = (struct dirent *)dir->dd_buf;
        rv = fioDread(dir->dd_fd, &fiode);
	if (rv <= 0) {
		return NULL;
	}

	__fill_stat(&de->d_stat, &fiode.stat);
	strncpy(de->d_name, fiode.name, 255);
	de->d_name[255] = 0;

	return de;
}

struct dirent * (*_ps2sdk_readdir)(DIR *dir) = fioReaddirHelper;

struct dirent *readdir(DIR *dir)
{
    return _ps2sdk_readdir(dir);
}
#endif

#ifdef F_rewinddir
static void fioRewinddirHelper(DIR *dir)
{
	printf("rewinddir not implemented\n");
}

void (*_ps2sdk_rewinddir)(DIR *dir) = fioRewinddirHelper;

void rewinddir(DIR *dir)
{
    return _ps2sdk_rewinddir(dir);
}
#endif

#ifdef F_closedir
static int fioClosedirHelper(DIR *dir)
{
	if(dir == NULL) {
		errno = EBADF;
		return -1;
	}

	fioDclose(dir->dd_fd); // Check return value?
	free(dir->dd_buf);
	free(dir);

	return 0;
}

int (*_ps2sdk_closedir)(DIR *dir) = fioClosedirHelper;

int closedir(DIR *dir)
{
    return _ps2sdk_closedir(dir);
}
#endif

#ifdef F__isatty
int _isatty(int fd) {
	struct stat buf;

	if (_fstat (fd, &buf) < 0) {
		errno = EBADF;
		return 0;
	}
	if (S_ISCHR (buf.st_mode))
		return 1;
	errno = ENOTTY;
	return 0;
}
#endif

#ifdef F__lseek
int (*_ps2sdk_lseek)(int, int, int) = fioLseek;

off_t _lseek(int fd, off_t offset, int whence)
{
	return __transform_errno(_ps2sdk_lseek(fd, offset, whence));
}
#endif

#ifdef F_lseek64
int64_t (*_ps2sdk_lseek64)(int, int64_t, int) = NULL;

off64_t lseek64(int fd, off64_t offset, int whence)
{
    if (_ps2sdk_lseek64 == NULL)
        return EOVERFLOW;

    return _ps2sdk_lseek64(fd, offset, whence);
}
#endif

#ifdef F_chdir
int chdir(const char *path) {
    strcpy(__direct_pwd, path);
    return 0;
}
#endif

#ifdef F_mkdir
int fioMkdirHelper(const char *path, int mode) {
  // Old fio mkdir has no mode argument
  return fioMkdir(path);
}

int (*_ps2sdk_mkdir)(const char*, int) = fioMkdirHelper;

int mkdir(const char *path, mode_t mode) {
    return __transform_errno(_ps2sdk_mkdir(path, mode));
}
#endif

#ifdef F_rmdir
int (*_ps2sdk_rmdir)(const char*) = fioRmdir;

int rmdir(const char *path) {
    return __transform_errno(_ps2sdk_rmdir(path));
}
#endif

#ifdef F__link
int fioRename(const char *old, const char *new) {
  return -ENOSYS;
}

int (*_ps2sdk_rename)(const char*, const char*) = fioRename;

int _link(const char *old, const char *new) {
    return _ps2sdk_rename(old, new);
}
#endif

#ifdef F__unlink
int (*_ps2sdk_remove)(const char*) = fioRemove;

int _unlink(const char *path) {
    return __transform_errno(_ps2sdk_remove(path));
}
#endif

#ifdef F_getcwd
char *getcwd(char *buf, size_t len) {
	strncpy(buf, __direct_pwd, len);
	return buf;
}
#endif

#ifdef F__getpid
int _getpid(void) {
	return GetThreadId();
}
#endif

#ifdef F__kill
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
#endif

#ifdef F__sbrk
void * _sbrk(size_t incr) {
	static void * _heap_ptr = &_end;
	void *mp, *ret = (void *)-1;

	if (incr == 0)
		return _heap_ptr;

	/* If the area we want to allocated is past the end of our heap, we have a problem. */
	mp = _heap_ptr + incr;
	if (mp <= EndOfHeap()) {
		ret = _heap_ptr;
		_heap_ptr = mp;
	}

	return ret;
}
#endif

#ifdef F_time
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
#endif

#ifdef F__gettimeofday
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
#endif

#ifdef F__times
clock_t _times(struct tms *buffer) {
	clock_t clk = ps2_clock() / (PS2_CLOCKS_PER_SEC / CLOCKS_PER_SEC);

	if (buffer != NULL) {
		buffer->tms_utime  = clk;
		buffer->tms_stime  = 0;
		buffer->tms_cutime = clk;
		buffer->tms_cstime = 0;
	}

	return clk;
}
#endif

#ifdef F_random
long int random(void)
{
    return rand();
}
#endif

#ifdef F_srandom
void srandom(unsigned int seed)
{
    srand(seed);
}
#endif

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
#include <timer.h>
#include <stdlib.h>
#include <stdarg.h>
#include <sio.h>

#include <pwd.h>
#include <time.h>
#include <sys/time.h>
#include <sys/timeb.h>
#include <sys/times.h>
#include <sys/utime.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/random.h>

// Include all integer types for compile time checking of:
// - compiler (gcc)
// - libc (newlib)
#include <stdint.h>
#include <limits.h>
#include <inttypes.h>
#include <tamtypes.h>

#define NEWLIB_PORT_AWARE
#include "io_common.h"
#include "ps2sdkapi.h"
#include "timer_alarm.h"
#include "fdman.h"

/* Functions from cwd.c */
extern char __cwd[MAXNAMLEN + 1];
int __path_absolute(const char *in, char *out, int len);

extern void * _end;

#ifdef F___dummy_passwd
/* the present working directory variable. */
struct passwd __dummy_passwd = { "ps2_user", "xxx", 1000, 1000, "", "", "/", "" };
#else
extern struct passwd __dummy_passwd;
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
int __transform_errno(int res);
#endif

#ifdef F___transform64_errno
int64_t __transform64_errno(int64_t res) {
	/* On error, -1 is returned and errno is set to indicate the error. */
	if (res < 0) {
		errno = -res;
		return -1;
	}
	return res;
}
#else
int64_t __transform64_errno(int64_t res);
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
int _open(const char *buf, int flags, ...) {
	int iop_flags = 0;
	int is_dir = 0;
	int iop_fd, fd;
	char t_fname[MAXNAMLEN + 1];

	if(__path_absolute(buf, t_fname, MAXNAMLEN) < 0) {
		errno = ENAMETOOLONG;
		return -1;
	}

	// Set ps2sdk functions
	_glue_ps2sdk_dopen();
	_glue_ps2sdk_open();
	_glue_ps2sdk_dclose();
	_glue_ps2sdk_close();

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
	if (flags & O_DIRECTORY) {
		iop_flags |= IOP_O_DIROPEN;
		is_dir = 1;
	}

	iop_fd = is_dir ? _ps2sdk_dopen(t_fname) : _ps2sdk_open(t_fname, iop_flags);
	if (iop_fd >= 0) {
		fd = __fdman_get_new_descriptor();
		if (fd != -1) {
			__descriptormap[fd]->descriptor = iop_fd;
			__descriptormap[fd]->type = is_dir ? __DESCRIPTOR_TYPE_FOLDER : __DESCRIPTOR_TYPE_FILE;
			__descriptormap[fd]->flags = flags;
			__descriptormap[fd]->filename = strdup(t_fname);
			return fd;
		}
		else {
			is_dir ? _ps2sdk_dclose(iop_fd) : _ps2sdk_close(iop_fd);
			errno = ENOMEM;
			return -1;
		}
	} 
	else {
		return __transform_errno(iop_fd);
	}
}
#endif

#ifdef F__close
int _close(int fd) {
	int ret = 0;

	if (!__IS_FD_VALID(fd)) {
		errno = EBADF;
		return -1;
	}

	// Set ps2sdk functions
	_glue_ps2sdk_close();

	switch(__descriptormap[fd]->type)
	{
		case __DESCRIPTOR_TYPE_FILE:
		case __DESCRIPTOR_TYPE_TTY:
			if (__descriptormap[fd]->ref_count == 1) {
				ret = __transform_errno(_ps2sdk_close(__descriptormap[fd]->descriptor));
			}
			__fdman_release_descriptor(fd);
			return ret;
			break;
		case __DESCRIPTOR_TYPE_FOLDER:
			if (__descriptormap[fd]->ref_count == 1) {
				ret = __transform_errno(_ps2sdk_dclose(__descriptormap[fd]->descriptor));
			}
			__fdman_release_descriptor(fd);
			return ret;
			break;
		case __DESCRIPTOR_TYPE_PIPE:
			// Not supported yet
			break;
		case __DESCRIPTOR_TYPE_SOCKET:
			// Not supported yet
			break;
		default:
			break;
	}

	errno = EBADF;
	return -1;
}
#endif

#ifdef F__read
int _read(int fd, void *buf, size_t nbytes) {
	if (!__IS_FD_VALID(fd)) {
		errno = EBADF;
		return -1;
	}

	// Set ps2sdk functions
	_glue_ps2sdk_read();

	switch(__descriptormap[fd]->type)
	{
		case __DESCRIPTOR_TYPE_FILE:
		case __DESCRIPTOR_TYPE_TTY:
			return __transform_errno(_ps2sdk_read(__descriptormap[fd]->descriptor, buf, nbytes));
			break;
		case __DESCRIPTOR_TYPE_PIPE:
			break;
		case __DESCRIPTOR_TYPE_SOCKET:
			break;
		default:
			break;
	}

	errno = EBADF;
	return -1;
}
#endif

#ifdef F__write
int _write(int fd, const void *buf, size_t nbytes) {
	if (!__IS_FD_VALID(fd)) {
		errno = EBADF;
		return -1;
	}

	// Set ps2sdk functions
	_glue_ps2sdk_write();

	switch(__descriptormap[fd]->type)
	{
		case __DESCRIPTOR_TYPE_FILE:
		case __DESCRIPTOR_TYPE_TTY:
			return __transform_errno(_ps2sdk_write(__descriptormap[fd]->descriptor, buf, nbytes));
			break;
		case __DESCRIPTOR_TYPE_PIPE:
			break;
		case __DESCRIPTOR_TYPE_SOCKET:
			break;
		default:
			break;
	}

	errno = EBADF;
	return -1;
}
#endif

#ifdef F__stat
int _stat(const char *path, struct stat *buf) {
	char dest[MAXNAMLEN + 1];

	if(__path_absolute(path, dest, MAXNAMLEN) < 0) {
		errno = ENAMETOOLONG;
		return -1;
	}

	// Set ps2sdk functions
	_glue_ps2sdk_stat();

	return __transform_errno(_ps2sdk_stat(dest, buf));
}
#endif

#ifdef F_lstat
int lstat(const char *path, struct stat *buf) {
	char dest[MAXNAMLEN + 1];

	if(__path_absolute(path, dest, MAXNAMLEN) < 0) {
		errno = ENAMETOOLONG;
		return -1;
	}
	
	return __transform_errno(stat(dest, buf));
}
#endif

#ifdef F__fstat
int _fstat(int fd, struct stat *buf) {
	int ret;
	if (!__IS_FD_VALID(fd)) {
		errno = EBADF;
		return -1;
	}

	switch(__descriptormap[fd]->type)
	{
		case __DESCRIPTOR_TYPE_TTY:
			memset(buf, '\0', sizeof(struct stat));
			buf->st_mode = S_IFCHR;
			return 0;
			break;		
		case __DESCRIPTOR_TYPE_FILE:
			if (__descriptormap[fd]->filename != NULL) {
				ret = stat(__descriptormap[fd]->filename, buf);
				return ret;
			}
			break;
		case __DESCRIPTOR_TYPE_PIPE:
		case __DESCRIPTOR_TYPE_SOCKET:
		default:
			break;
	}

	errno = EBADF;
	return -1;
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
		errno = EACCES;
		return -1;
	}
	return 0;
}
#endif

#ifdef F__fcntl
int _fcntl(int fd, int cmd, ...)
{
	if (!__IS_FD_VALID(fd)) {
		errno = EBADF;
		return -1;
	}

	switch (cmd)
	{
		case F_DUPFD:
		{
			return __fdman_get_dup_descriptor(fd);
			break;
		}
		case F_GETFL:
		{
			return __descriptormap[fd]->flags;
			break;
		}
		case F_SETFL:
		{
			int newfl;
			va_list args;
	
			va_start (args, cmd);         /* Initialize the argument list. */
			newfl =  va_arg(args, int);
			va_end (args);                /* Clean up. */

			__descriptormap[fd]->flags = newfl;

			switch(__descriptormap[fd]->type)
			{
				case __DESCRIPTOR_TYPE_FILE:
					break;
				case __DESCRIPTOR_TYPE_PIPE:
					break;
				case __DESCRIPTOR_TYPE_SOCKET:
					break;
				default:
					break;
			}
			return 0;
			break;
		}
	}

	errno = EBADF;
	return -1;
}
#endif /* F__fcntl */

#ifdef F_getdents
int getdents(int fd, void *dd_buf, int count)
{
	struct dirent *dirp;
	int rv, read;

	read = 0;
	dirp = (struct dirent *)dd_buf;

	// Set ps2sdk functions
	_glue_ps2sdk_dread();

	rv = _ps2sdk_dread(__descriptormap[fd]->descriptor, dirp);
	if (rv < 0) {
		return __transform_errno(rv);
	} else if (rv == 0) {
		return read;
	}

	read += sizeof(struct dirent);	
	dirp->d_reclen = count;

	return read;
}
#endif



#ifdef F__lseek
static off_t _lseekDir(int fd, off_t offset, int whence)
{
	int i;
	int uid;
	struct dirent dir;

	if (whence != SEEK_SET || __descriptormap[fd]->filename == NULL) {
		errno = EINVAL;
		return -1;
	}

	// Set ps2sdk functions
	_glue_ps2sdk_close();
	_glue_ps2sdk_dopen();
	_glue_ps2sdk_dread();

	_ps2sdk_dclose(__descriptormap[fd]->descriptor);
	uid = _ps2sdk_dopen(__descriptormap[fd]->filename);
	__descriptormap[fd]->descriptor = uid;
	for (i = 0; i < offset; i++) {
		_ps2sdk_dread(uid, &dir);
	}

	return offset;
}

off_t _lseek(int fd, off_t offset, int whence)
{
	if (!__IS_FD_VALID(fd)) {
		errno = EBADF;
		return -1;
	}

	// Set ps2sdk functions
	_glue_ps2sdk_lseek();

	switch(__descriptormap[fd]->type)
	{
		case __DESCRIPTOR_TYPE_FILE:
			return __transform_errno(_ps2sdk_lseek(__descriptormap[fd]->descriptor, offset, whence));
			break;
		case __DESCRIPTOR_TYPE_FOLDER:
			return _lseekDir(fd, offset, whence);
			break;
		case __DESCRIPTOR_TYPE_PIPE:
			break;
		case __DESCRIPTOR_TYPE_SOCKET:
			break;
		default:
			break;
	}

	errno = EBADF;
	return -1;
}
#endif

#ifdef F_lseek64
off64_t lseek64(int fd, off64_t offset, int whence)
{
	if (!__IS_FD_VALID(fd)) {
		errno = EBADF;
		return -1;
	}

	// Set ps2sdk functions
	_glue_ps2sdk_lseek64();

	switch(__descriptormap[fd]->type)
	{
		case __DESCRIPTOR_TYPE_FILE:
			return __transform64_errno(_ps2sdk_lseek64(__descriptormap[fd]->descriptor, offset, whence));
			break;
		case __DESCRIPTOR_TYPE_FOLDER:
			break;
		case __DESCRIPTOR_TYPE_PIPE:
			break;
		case __DESCRIPTOR_TYPE_SOCKET:
			break;
		default:
			break;
	}

	errno = EBADF;
	return -1;
}
#endif

#ifdef F_chdir
int chdir(const char *path) {
	char dest[MAXNAMLEN + 1];

	if(__path_absolute(path, dest, MAXNAMLEN) < 0) {
		errno = ENAMETOOLONG;
		return -1;
	}

	strcpy(__cwd, dest);
	return 0;
}
#endif

#ifdef F_mkdir
int mkdir(const char *path, mode_t mode) {
	char dest[MAXNAMLEN + 1];

	if(__path_absolute(path, dest, MAXNAMLEN) < 0) {
		errno = ENAMETOOLONG;
		return -1;
	}

	// Set ps2sdk functions
	_glue_ps2sdk_mkdir();

	return __transform_errno(_ps2sdk_mkdir(dest, mode));
}
#endif

#ifdef F_rmdir
int rmdir(const char *path) {
	char dest[MAXNAMLEN + 1];

	if(__path_absolute(path, dest, MAXNAMLEN) < 0) {
		errno = ENAMETOOLONG;
		return -1;
	}

	// Set ps2sdk functions
	_glue_ps2sdk_rmdir();

	return __transform_errno(_ps2sdk_rmdir(dest));
}
#endif

#ifdef F__link
int _link(const char *old, const char *new) {
	errno = ENOSYS;
	return -1; /* not supported */
}
#endif

#ifdef F__unlink
int _unlink(const char *path) {
	char dest[MAXNAMLEN + 1];
	if(__path_absolute(path, dest, MAXNAMLEN) < 0) {
		errno = ENAMETOOLONG;
		return -1;
	}

	// Set ps2sdk functions
	_glue_ps2sdk_remove();

	return __transform_errno(_ps2sdk_remove(dest));
}
#endif

#ifdef F__rename
int _rename(const char *old, const char *new) {
	char oldname[MAXNAMLEN + 1];
	char newname[MAXNAMLEN + 1];

	if(__path_absolute(old, oldname, MAXNAMLEN) < 0) {
		errno = ENAMETOOLONG;
		return -1;
	}

	if(__path_absolute(new, newname, MAXNAMLEN) < 0) {
		errno = ENAMETOOLONG;
		return -1;
	}

	// Set ps2sdk functions
	_glue_ps2sdk_rename();

	return __transform_errno(_ps2sdk_rename(oldname, newname));
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
	(void)pid;
	(void)sig;
	errno = ENOSYS;
	return 1; /* not supported */
}
#endif

#ifdef F__fork
pid_t _fork(void) {
	errno = ENOSYS;
	return (pid_t) -1; /* not supported */
}
#endif

#ifdef F__wait
pid_t _wait(int *unused) {
	errno = ENOSYS;
	return (pid_t) -1; /* not supported */
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

#ifdef F__gettimeofday
int _gettimeofday(struct timeval *tv, struct timezone *tz)
{
	if (tv == NULL)
	{
		errno = EFAULT;
		return -1;
	}

	{
		u32 busclock_sec;
		u32 busclock_usec;

		TimerBusClock2USec(GetTimerSystemTime(), &busclock_sec, &busclock_usec);
		tv->tv_sec = (time_t)(_ps2sdk_rtc_offset_from_busclk + ((s64)busclock_sec));
		tv->tv_usec = busclock_usec;
	}

	if (tz != NULL)
	{
		tz->tz_minuteswest = _timezone / 60;
		tz->tz_dsttime = 0;
	}

	return 0;
}
#endif

#ifdef F__times
clock_t _times(struct tms *buffer) {
	clock_t clk = GetTimerSystemTime() / (kBUSCLK / CLOCKS_PER_SEC);

	if (buffer != NULL) {
		buffer->tms_utime  = clk;
		buffer->tms_stime  = 0;
		buffer->tms_cutime = 0;
		buffer->tms_cstime = 0;
	}

	return clk;
}
#endif

#ifdef F_ftime
int ftime(struct timeb *tb) {
	struct timeval tv;
	struct timezone tz;

	gettimeofday(&tv, &tz);

	tb->time = tv.tv_sec;
	tb->millitm = tv.tv_usec / 1000;
	tb->timezone = tz.tz_minuteswest;
	tb->dstflag = tz.tz_dsttime;

	return 0;
}
#endif

#ifdef F_clock_getres
int clock_getres(clockid_t clk_id, struct timespec *res) {
	struct timeval tv;
	int ret;

	ret = gettimeofday(&tv, NULL);

	/* Return the actual time since epoch */
	res->tv_sec = tv.tv_sec;
	res->tv_nsec = tv.tv_usec * 1000;

	return ret;
}
#endif

#ifdef F_clock_gettime
int clock_gettime(clockid_t clk_id, struct timespec *tp) {
	struct timeval tv;
	int res;

	res = gettimeofday(&tv, NULL);

	/* Return the actual time since epoch */
	tp->tv_sec = tv.tv_sec;
	tp->tv_nsec = tv.tv_usec * 1000;

	return res;
}
#endif

#ifdef F_clock_settime
int clock_settime(clockid_t clk_id, const struct timespec *tp) {
	// TODO: implement using sceCdWriteClock
	errno = EPERM;
	return -1;
}
#endif

#ifdef F_truncate
int truncate(const char *path, off_t length)
{
	ssize_t bytes_read;
	int fd, res;
	char buff[length];

	fd = open(path, O_RDONLY);
	if (fd < 0) {
		errno = ENOENT;
		return -1;
	}

	bytes_read = read(fd, &buff, length);
	close(fd);
	if (bytes_read < length) {
		errno = EFBIG;
		return -1;
	}

	fd = open (path, O_TRUNC|O_WRONLY);
	if (fd < 0) {
		errno = ENOENT;
		return -1;
	}

	res = write(fd, &buff, length);
	close(fd);
	return res;
}
#endif

#ifdef F_symlink
int symlink(const char *target, const char *linkpath)
{
	char dest_target[MAXNAMLEN + 1];
	char dest_linkpath[MAXNAMLEN + 1];

	if(__path_absolute(target, dest_target, MAXNAMLEN) < 0) {
		errno = ENAMETOOLONG;
		return -1;
	}

	if(__path_absolute(linkpath, dest_linkpath, MAXNAMLEN) < 0) {
		errno = ENAMETOOLONG;
		return -1;
	}

	// Set ps2sdk functions
	_glue_ps2sdk_symlink();

	return __transform_errno(_ps2sdk_symlink(dest_target, dest_linkpath));
}
#endif

#ifdef F_readlink
ssize_t readlink(const char *path, char *buf, size_t bufsiz)
{
	char dest[MAXNAMLEN + 1];

	if(__path_absolute(path, dest, MAXNAMLEN) < 0) {
		errno = ENAMETOOLONG;
		return -1;
	}

	// Set ps2sdk functions
	_glue_ps2sdk_readlink();

	return 	__transform_errno(_ps2sdk_readlink(dest, buf, bufsiz));
}
#endif

#ifdef F_utime
int utime(const char *pathname, const struct utimbuf *times)
{
	// TODO: implement in terms of chstat
	errno = ENOSYS;
	return -1; /* not supported */
}
#endif

#ifdef F_fchown
int fchown(int fd, uid_t owner, gid_t group)
{
	errno = ENOSYS;
	return -1; /* not supported */
}
#endif

#ifdef F_getrandom
ssize_t getrandom(void *buf, size_t buflen, unsigned int flags)
{
	(void)flags;

	arc4random_buf(buf, buflen);
	return buflen;
}
#endif

#ifdef F_getentropy
int getentropy(void *buf, size_t buflen)
{
	u8 *buf_cur = buf;
	int i;
	// Restrict buffer size as documented in the man page
	if (buflen > 256)
	{
		errno = EIO;
		return -1;
	}
	// TODO: get proper entropy from e.g.
	// * RTC
	// * uninitialized memory
	// * Mechacon temperature
	for (i = 0; i < buflen; i += 1)
	{
		// Performance counter low buts should be changed for each call to cpu_ticks
		buf_cur[i] = (u8)(cpu_ticks() & 0xff);
	}
	return 0;
}
#endif

#ifdef F__isatty
int _isatty(int fd)
{
	errno = ENOSYS;
	return -1; /* not supported */
}
#endif

#ifdef F_chmod
int chmod(const char *pathname, mode_t mode)
{
	errno = ENOSYS;
	return -1; /* not supported */
}
#endif

#ifdef F_fchmod
int fchmod(int fd, mode_t mode)
{
	errno = ENOSYS;
	return -1; /* not supported */
}
#endif

#ifdef F_fchmodat
int fchmodat(int fd, const char *path, mode_t mode, int flag)
{
	return chmod(path, mode);
}
#endif

#ifdef F_pathconf
long int pathconf(const char *path, int name)
{
	errno = ENOSYS;
	return -1; /* not supported */
}
#endif

#ifdef F_fsync
int fsync(int fd) {
	// TODO: implement in terms of sync
	return 0;
}
#endif

#ifdef F_getuid
uid_t getuid(void) {
	return __dummy_passwd.pw_uid;
}
#endif

#ifdef F_geteuid
uid_t geteuid(void) {
	return __dummy_passwd.pw_uid;
}
#endif

#ifdef F_getpwuid
struct passwd *getpwuid(uid_t uid) {
	/* There's no support for users */
	return &__dummy_passwd;
}
#endif

#ifdef F_getpwnam
struct passwd *getpwnam(const char *name) {
	/* There's no support for users */
	return &__dummy_passwd;
}
#endif

#ifdef F_ps2sdk_get_iop_fd
int ps2sdk_get_iop_fd(int fd) {
	if (!__IS_FD_VALID(fd)) {
		errno = EBADF;
		return -1;
	}

	return __descriptormap[fd]->descriptor;
}
#endif

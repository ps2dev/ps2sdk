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
	int iop_fd, fd;
	int mode;
	_libcglue_fdman_fd_info_t *info;
	va_list alist;
	char t_fname[MAXNAMLEN + 1];

	va_start(alist, flags);
	mode = va_arg(alist, int);	// Retrieve the mode argument, regardless of whether it is expected or not.
	va_end(alist);

	if(__path_absolute(buf, t_fname, MAXNAMLEN) < 0) {
		errno = ENAMETOOLONG;
		return -1;
	}

	if (_libcglue_fdman_path_ops == NULL || _libcglue_fdman_path_ops->open == NULL)
	{
		errno = ENOSYS;
		return -1;
	}

	fd = __fdman_get_new_descriptor();
	if (fd == -1)
	{
		errno = ENOMEM;
		return -1;
	}

	info = &(__descriptormap[fd]->info);
	iop_fd = _libcglue_fdman_path_ops->open(info, t_fname, flags, mode);
	if (iop_fd < 0)
	{
		__fdman_release_descriptor(fd);
		return __transform_errno(iop_fd);
	}
	__descriptormap[fd]->flags = flags;
	return fd;
}
#endif

#ifdef F__close
int _close(int fd) {
	int ret = 0;

	if (!__IS_FD_VALID(fd)) {
		errno = EBADF;
		return -1;
	}

	if (__descriptormap[fd]->ref_count == 1)
	{
		_libcglue_fdman_fd_info_t *fdinfo;

		fdinfo = &(__descriptormap[fd]->info);
		if (fdinfo->ops != NULL && fdinfo->ops->close != NULL)
		{
			ret = __transform_errno(fdinfo->ops->close(fdinfo->userdata));
		}
	}
	__fdman_release_descriptor(fd);
	return ret;
}
#endif

#ifdef F__read
int _read(int fd, void *buf, size_t nbytes) {
	if (!__IS_FD_VALID(fd)) {
		errno = EBADF;
		return -1;
	}

	_libcglue_fdman_fd_info_t *fdinfo;

	fdinfo = &(__descriptormap[fd]->info);
	if (fdinfo->ops == NULL || fdinfo->ops->read == NULL)
	{
		errno = ENOSYS;
		return -1;
	}
	return __transform_errno(fdinfo->ops->read(fdinfo->userdata, buf, nbytes));
}
#endif

#ifdef F__write
int _write(int fd, const void *buf, size_t nbytes) {
	if (!__IS_FD_VALID(fd)) {
		errno = EBADF;
		return -1;
	}

	_libcglue_fdman_fd_info_t *fdinfo;

	fdinfo = &(__descriptormap[fd]->info);
	if (fdinfo->ops == NULL || fdinfo->ops->write == NULL)
	{
		errno = ENOSYS;
		return -1;
	}
	return __transform_errno(fdinfo->ops->write(fdinfo->userdata, buf, nbytes));
}
#endif

#ifdef F__stat
int _stat(const char *path, struct stat *buf) {
	char dest[MAXNAMLEN + 1];

	if(__path_absolute(path, dest, MAXNAMLEN) < 0) {
		errno = ENAMETOOLONG;
		return -1;
	}

	if (_libcglue_fdman_path_ops == NULL || _libcglue_fdman_path_ops->stat == NULL)
	{
		errno = ENOSYS;
		return -1;
	}

	return __transform_errno(_libcglue_fdman_path_ops->stat(dest, buf));
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
	if (!__IS_FD_VALID(fd)) {
		errno = EBADF;
		return -1;
	}

	_libcglue_fdman_fd_info_t *fdinfo;
	char *filename;

	fdinfo = &(__descriptormap[fd]->info);
	if (fdinfo->ops == NULL || fdinfo->ops->getfilename == NULL)
	{
		errno = ENOSYS;
		return -1;
	}
	filename = fdinfo->ops->getfilename(fdinfo->userdata);
	if (filename == NULL)
	{
		errno = ENOENT;
		return -1;
	}
	return stat(filename, buf);
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
			int newfl, rv;
			va_list args;
	
			rv = 0;

			va_start (args, cmd);         /* Initialize the argument list. */
			newfl =  va_arg(args, int);
			va_end (args);                /* Clean up. */

			__descriptormap[fd]->flags = newfl;

			{
				_libcglue_fdman_fd_info_t *fdinfo;

				fdinfo = &(__descriptormap[fd]->info);
				
				if (fdinfo->ops != NULL && fdinfo->ops->fcntl_f_setfl != NULL)
				{
					rv = __transform_errno(fdinfo->ops->fcntl_f_setfl(fdinfo->userdata, newfl));
				}
			}
			return rv;
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

	{
		_libcglue_fdman_fd_info_t *fdinfo;

		fdinfo = &(__descriptormap[fd]->info);
		rv = -ENOSYS;
		if (fdinfo->ops != NULL && fdinfo->ops->dread != NULL)
		{
			rv = __transform_errno(fdinfo->ops->dread(fdinfo->userdata, dirp));
		}
	}
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
off_t _lseek(int fd, off_t offset, int whence)
{
	if (!__IS_FD_VALID(fd)) {
		errno = EBADF;
		return -1;
	}

	_libcglue_fdman_fd_info_t *fdinfo;

	fdinfo = &(__descriptormap[fd]->info);
	if (fdinfo->ops == NULL || fdinfo->ops->lseek == NULL)
	{
		errno = ENOSYS;
		return -1;
	}
	return __transform_errno(fdinfo->ops->lseek(fdinfo->userdata, offset, whence));
}
#endif

#ifdef F_lseek64
off64_t lseek64(int fd, off64_t offset, int whence)
{
	if (!__IS_FD_VALID(fd)) {
		errno = EBADF;
		return -1;
	}

	_libcglue_fdman_fd_info_t *fdinfo;

	fdinfo = &(__descriptormap[fd]->info);
	if (fdinfo->ops == NULL || fdinfo->ops->lseek64 == NULL)
	{
		errno = ENOSYS;
		return -1;
	}
	return __transform64_errno(fdinfo->ops->lseek64(fdinfo->userdata, offset, whence));
}
#endif

#ifdef F_chdir
int chdir(const char *path) {
	char dest[MAXNAMLEN + 1];

	if(__path_absolute(path, dest, MAXNAMLEN) < 0) {
		errno = ENAMETOOLONG;
		return -1;
	}

	strncpy(__cwd, dest, sizeof(__cwd));
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

	if (_libcglue_fdman_path_ops == NULL || _libcglue_fdman_path_ops->mkdir == NULL)
	{
		errno = ENOSYS;
		return -1;
	}

	return __transform_errno(_libcglue_fdman_path_ops->mkdir(dest, mode));
}
#endif

#ifdef F_rmdir
int rmdir(const char *path) {
	char dest[MAXNAMLEN + 1];

	if(__path_absolute(path, dest, MAXNAMLEN) < 0) {
		errno = ENAMETOOLONG;
		return -1;
	}

	if (_libcglue_fdman_path_ops == NULL || _libcglue_fdman_path_ops->rmdir == NULL)
	{
		errno = ENOSYS;
		return -1;
	}

	return __transform_errno(_libcglue_fdman_path_ops->rmdir(dest));
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

	if (_libcglue_fdman_path_ops == NULL || _libcglue_fdman_path_ops->remove == NULL)
	{
		errno = ENOSYS;
		return -1;
	}

	return __transform_errno(_libcglue_fdman_path_ops->remove(dest));
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

	if (_libcglue_fdman_path_ops == NULL || _libcglue_fdman_path_ops->rename == NULL)
	{
		errno = ENOSYS;
		return -1;
	}

	return __transform_errno(_libcglue_fdman_path_ops->rename(oldname, newname));
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

	if (_libcglue_fdman_path_ops == NULL || _libcglue_fdman_path_ops->symlink == NULL)
	{
		errno = ENOSYS;
		return -1;
	}

	return __transform_errno(_libcglue_fdman_path_ops->symlink(dest_target, dest_linkpath));
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

	if (_libcglue_fdman_path_ops == NULL || _libcglue_fdman_path_ops->readlink == NULL)
	{
		errno = ENOSYS;
		return -1;
	}

	return __transform_errno(_libcglue_fdman_path_ops->readlink(dest, buf, bufsiz));
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

#ifdef F__getentropy
int _getentropy(void *buf, size_t buflen)
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

#ifdef F_libcglue_get_fd_info
_libcglue_fdman_fd_info_t *libcglue_get_fd_info(int fd) {
	if (!__IS_FD_VALID(fd)) {
		errno = EBADF;
		return NULL;
	}

	return &(__descriptormap[fd]->info);
}
#endif

#ifdef F_ps2sdk_get_iop_fd
int ps2sdk_get_iop_fd(int fd)
{
	_libcglue_fdman_fd_info_t *fdinfo;
	fdinfo = libcglue_get_fd_info(fd);
	if (fdinfo == NULL)
	{
		return -EBADF;
	}
	if (fdinfo->ops == NULL || fdinfo->ops->getfd == NULL)
	{
		return -ENOSYS;
	}
	return fdinfo->ops->getfd(fdinfo->userdata);
}
#endif

#ifdef F_ps2sdk_get_iop_filename
char *ps2sdk_get_iop_filename(int fd)
{
	_libcglue_fdman_fd_info_t *fdinfo;
	fdinfo = libcglue_get_fd_info(fd);
	if (fdinfo == NULL)
	{
		return NULL;
	}
	if (fdinfo->ops == NULL || fdinfo->ops->getfilename == NULL)
	{
		return NULL;
	}
	return fdinfo->ops->getfilename(fdinfo->userdata);
}
#endif

#ifdef F__ps2sdk_close
int _ps2sdk_close(int fd)
{
	_libcglue_fdman_fd_info_t *fdinfo;
	fdinfo = libcglue_get_fd_info(fd);
	if (fdinfo == NULL)
	{
		return -EBADF;
	}
	if (fdinfo->ops == NULL || fdinfo->ops->close == NULL)
	{
		return -ENOSYS;
	}
	return fdinfo->ops->close(fdinfo->userdata);
}
#endif

#ifdef F__ps2sdk_dclose
int _ps2sdk_dclose(int fd)
{
	return _ps2sdk_close(fd);
}
#endif

#ifdef F__ps2sdk_read
int _ps2sdk_read(int fd, void *buf, int nbytes)
{
	_libcglue_fdman_fd_info_t *fdinfo;
	fdinfo = libcglue_get_fd_info(fd);
	if (fdinfo == NULL)
	{
		return -EBADF;
	}
	if (fdinfo->ops == NULL || fdinfo->ops->read == NULL)
	{
		return -ENOSYS;
	}
	return fdinfo->ops->read(fdinfo->userdata, buf, nbytes);
}
#endif

#ifdef F__ps2sdk_lseek
int _ps2sdk_lseek(int fd, int offset, int whence)
{
	_libcglue_fdman_fd_info_t *fdinfo;
	fdinfo = libcglue_get_fd_info(fd);
	if (fdinfo == NULL)
	{
		return -EBADF;
	}
	if (fdinfo->ops == NULL || fdinfo->ops->lseek == NULL)
	{
		return -ENOSYS;
	}
	return fdinfo->ops->lseek(fdinfo->userdata, offset, whence);
}
#endif

#ifdef F__ps2sdk_lseek64
int64_t _ps2sdk_lseek64(int fd, int64_t offset, int whence)
{
	_libcglue_fdman_fd_info_t *fdinfo;
	fdinfo = libcglue_get_fd_info(fd);
	if (fdinfo == NULL)
	{
		return -EBADF;
	}
	if (fdinfo->ops == NULL || fdinfo->ops->lseek64 == NULL)
	{
		return -ENOSYS;
	}
	return fdinfo->ops->lseek64(fdinfo->userdata, offset, whence);
}
#endif

#ifdef F__ps2sdk_write
int _ps2sdk_write(int fd, const void *buf, int nbytes)
{
	_libcglue_fdman_fd_info_t *fdinfo;
	fdinfo = libcglue_get_fd_info(fd);
	if (fdinfo == NULL)
	{
		return -EBADF;
	}
	if (fdinfo->ops == NULL || fdinfo->ops->write == NULL)
	{
		return -ENOSYS;
	}
	return fdinfo->ops->write(fdinfo->userdata, buf, nbytes);
}
#endif

#ifdef F__ps2sdk_ioctl
int _ps2sdk_ioctl(int fd, int request, void *data)
{
	_libcglue_fdman_fd_info_t *fdinfo;
	fdinfo = libcglue_get_fd_info(fd);
	if (fdinfo == NULL)
	{
		return -EBADF;
	}
	if (fdinfo->ops == NULL || fdinfo->ops->ioctl == NULL)
	{
		return -ENOSYS;
	}
	return fdinfo->ops->ioctl(fdinfo->userdata, request, data);
}
#endif

#ifdef F__ps2sdk_dread
int _ps2sdk_dread(int fd, struct dirent *dir)
{
	_libcglue_fdman_fd_info_t *fdinfo;
	fdinfo = libcglue_get_fd_info(fd);
	if (fdinfo == NULL)
	{
		return -EBADF;
	}
	if (fdinfo->ops == NULL || fdinfo->ops->dread == NULL)
	{
		return -ENOSYS;
	}
	return fdinfo->ops->dread(fdinfo->userdata, dir);
}
#endif

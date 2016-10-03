/*
# _____     ___ ____     ___ ____
#  ____|   |    ____|   |        | |____|
# |     ___|   |____ ___|    ____| |    \    PS2DEV Open Source Project.
#-----------------------------------------------------------------------
# Copyright 2001-2005, ps2dev - http://www.ps2dev.org
# Licenced under Academic Free License version 2.0
# Review ps2sdk README & LICENSE files for further details.
#
# $Id$
# unistd implementation
*/
#include <sys/stat.h>
#include <unistd.h>
#include <string.h>
#include <fileio.h>

#ifdef F_open
extern int (*_ps2sdk_open)(const char*, int);
int open(const char *fname, int flags, ...)
{
  return _ps2sdk_open(fname, flags);
}
#endif

#ifdef F_close
extern int (*_ps2sdk_close)(int);
int close(int handle) 
{
  return _ps2sdk_close(handle);
}
#endif

#ifdef F_read
extern int (*_ps2sdk_read)(int, void*, int);
ssize_t read(int handle, void * buffer, size_t size)
{
  return _ps2sdk_read(handle, buffer, size);
}
#endif

#ifdef F_write
extern int (*_ps2sdk_write)(int, const void*, int);
ssize_t write(int handle, const void * buffer, size_t size)
{
  return _ps2sdk_write(handle, buffer, size);
}
#endif

#ifdef F_lseek
extern int (*_ps2sdk_lseek)(int, int, int);
off_t lseek(int handle, off_t position, int wheel)
{
  return _ps2sdk_lseek(handle, position, wheel);
}
#endif

#ifdef F_mkdir
extern int (*_ps2sdk_mkdir)(const char*, int mode);
int mkdir(const char *path, mode_t mode)
{
  return _ps2sdk_mkdir(path, mode);
}
#endif

#ifdef F_rmdir
extern int (*_ps2sdk_rmdir)(const char*);
int rmdir(const char *path)
{
  return _ps2sdk_rmdir(path);
}
#endif

#ifdef F_getcwd
extern char __direct_pwd[256];

char *getcwd(char *buf, int len)
{
	strncpy(buf, __direct_pwd, len);
	return buf;
}
#endif

#ifdef F_access
int access(const char *path, int mode)
{
	printf("access() unimplemented\n");
	return -1;
}
#endif

#ifdef F_stat
int stat(const char *path, struct stat *sbuf)
{
	printf("stat() unimplemented\n");
	return -1;
}
#endif

#ifdef F_fstat
int fstat(int filedes, struct stat *buf)
{
	printf("fstat() unimplemented\n");
	return -1;
}
#endif

#ifdef F_unlink
extern int (*_ps2sdk_remove)(const char*);
int unlink(const char *path)
{
	return _ps2sdk_remove(path);
}
#endif

#ifdef F_sleep
#include <kernel.h>
#include <time.h>

#define HSYNC_COUNT 600

struct sleep_data
{
    s32	 s;
    clock_t wait;
};

static void _sleep_waker(s32 alarm_id, u16 time, void *arg2)
{
    struct sleep_data *sd = (struct sleep_data *) arg2;
    if (clock() >= sd->wait)
        iSignalSema(sd->s);
    else
        iSetAlarm(HSYNC_COUNT, _sleep_waker, arg2);
}

unsigned int sleep(unsigned int seconds)
{
    ee_sema_t sema;
    struct sleep_data sd;

	sema.init_count = 0;
	sema.max_count  = 1;
	sema.option     = 0;

    sd.wait = clock() + seconds * CLOCKS_PER_SEC;
    sd.s = CreateSema(&sema);
    SetAlarm(HSYNC_COUNT, _sleep_waker, &sd);
    WaitSema(sd.s);
    DeleteSema(sd.s);

	return 0;
}
#endif

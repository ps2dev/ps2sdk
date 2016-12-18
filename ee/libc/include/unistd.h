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
# Miscellaneous Unix symbolic constants, types, and declarations
*/
#ifndef __UNISTD_H__
#define __UNISTD_H__

#include <sys/types.h>

#define __need_size_t
#define __need_NULL
#include <stddef.h>

#include <stdio.h>

/** does file exist */
#define	F_OK     0
/** is it executable or searchable */
#define X_OK     1
/** is it writeable */
#define	W_OK     2
/** is it readable */
#define	R_OK     4

#define	SEEK_SET	0
#define	SEEK_CUR	1
#define	SEEK_END	2

#define STDIN_FILENO	0
#define STDOUT_FILENO	1
#define STDERR_FILENO	2

#ifdef __cplusplus
extern "C" {
#endif

/* access is unimplemented */
int          access(const char *path, int mode);

int          close(int handle);
char        *getcwd(char *buf, int len);
off_t        lseek(int handle, off_t position, int wheel);
ssize_t      read(int handle, void * buffer, size_t size);
int          rmdir(const char *path);
unsigned int sleep(unsigned int seconds);
int          unlink(const char *path);
ssize_t      write(int handle, const void * buffer, size_t size);

#ifdef __cplusplus
}
#endif

#endif

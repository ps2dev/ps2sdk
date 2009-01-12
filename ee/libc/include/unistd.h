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
# Trap the unistd.h include and redirect it to stdio.h
*/
#ifndef __UNISTD_H__
#define __UNISTD_H__

#include <stdio.h>
#include <sys/stat.h>

/** does file exist */
#define	F_OK     0 
/** is it executable or searchable */
#define X_OK     1
/** is it writeable */
#define	W_OK     2
/** is it readable */
#define	R_OK     4

#ifdef __cplusplus
extern "C" {
#endif

int    stat(const char *path, struct stat *sbuf);
int    fstat(int filedes, struct stat *sbuf);
int    access(const char *path, int mode);
char  *getcwd(char *buf, int len);
int    unlink(const char *path);

unsigned int sleep(unsigned int seconds);

#ifdef __cplusplus
}
#endif

#endif

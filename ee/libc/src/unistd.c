/*
# _____     ___ ____     ___ ____
#  ____|   |    ____|   |        | |____|
# |     ___|   |____ ___|    ____| |    \    PS2DEV Open Source Project.
#-----------------------------------------------------------------------
# Copyright 2001-2005, ps2dev - http://www.ps2dev.org
# Licenced under Academic Free License version 2.0
# Review ps2sdk README & LICENSE files for further details.
#
# $Id: unistd.c $
# unistd implementation
*/
#include <unistd.h>
#include <sys/stat.h>

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
int unlink(const char *path)
{
	return fioRemove(path);
}
#endif


/*
# _____     ___ ____     ___ ____
#  ____|   |    ____|   |        | |____|
# |     ___|   |____ ___|    ____| |    \    PS2DEV Open Source Project.
#-----------------------------------------------------------------------
# Copyright 2001-2004, ps2dev - http://www.ps2dev.org
# Licenced under Academic Free License version 2.0
# Review ps2sdk README & LICENSE files for further details.
#
# $Id$
# EE file control definitions.
*/
#ifndef __FCTNL_H__
#define __FCNTL_H__

#include <sys/fcntl.h>
#include <sys/stat.h>

#include <unistd.h>

#ifdef __cplusplus
extern "C" {
#endif

int open(const char *fname, int flags, ...);

#ifdef __cplusplus
}
#endif

#endif /*__FCNTL_H__*/

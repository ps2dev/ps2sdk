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
# Small, compatibility-useful assert.
*/


#ifndef __UNISTD_H__
#define __UNISTD_H__

#include <stdio.h>
#include <kernel.h>

#ifdef NDEBUG
#define assert(cond)
#else
#define assert(cond) { \
    if (!cond) { \
        fprintf(stderr, "assert failed at line %i of file %s:\n %s is false.\n", __LINE__, __FILE__, #cond); \
	SleepThread(); \
    } \
}
#endif


#endif

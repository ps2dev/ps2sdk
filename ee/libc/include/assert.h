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


#ifndef __ASSERT_H__
#define __ASSERT_H__

#include <stdio.h>
#include <kernel.h>

#ifdef NDEBUG
#define assert(cond)
#else
void __assert_fail (const char *assertion, const char *file, unsigned int line, const char *function) __attribute__((noreturn));
#define assert(cond) { \
    if (!cond) \
	__assert_fail(#cond, __FILE__, __LINE__, __FUNCTION__); \
}
#endif


#endif

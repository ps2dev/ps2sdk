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
# A few ctype's functions. They should be macros but...
*/

#ifndef __CTYPE_H__
#define __CTYPE_H__

#ifdef __cplusplus
extern "C" {
#endif

int isalnum(int);
int isalpha(int);
int iscntrl(int);
int isdigit(int);
int isgraph(int);
int islower(int);
int isprint(int);
int ispunct(int);
int isspace(int);
int isupper(int);
int isxdigit(int);
int tolower(int);
int toupper(int);

#ifdef __cplusplus
}
#endif


/* To be compatible with C++'s ctype_base.h */

namespace std {
    enum {
	_U =	01,
	_L =	02,
	_N =	04,
	_S =	010,
	_P =	020,
	_C =	040,
	_X =	0100,
	_B =	0200,
    };
};

#endif

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
# Very minimalistic ctype macros for IOP
*/

#ifndef IOP__CTYPES_H__
#define IOP__CTYPES_H__

#define isascii(c) ((c >= 0) && (c <= 127))
#define islower(c) ((c >= 'a') && (c <= 'z'))
#define isupper(c) ((c >= 'A') && (c <= 'Z'))
#define isdigit(c) ((c >= '0') && (c <= '9'))
#define iscntrl(c) ((c >= 0) && (c < 32))
#define isblank(c) ((c == ' ') || (c == '\t'))
#define isprint(c) ((c >= 32) && (c <= 127))

#define isspace(c) (isblank(c) || (c == '\f') || (c == '\n') || (c == '\r') || (c == '\v'))
#define isxdigit(c) (isdigit(c) || ((c >= 'A') && (c <= 'F')))

#define isalnum(c) (salpha(c) || isdigit(c))
#define isalpha(c) (isupper(c) || islower(c))
#define isgraph(c) (isprint(c) && !isspace(c))
#define ispunct(c) (isprint(c) && !(isspace(c) || isalnum(c)))


#endif

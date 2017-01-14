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
# ctype functions for the IOP
*/

#ifndef IOP__CTYPES_H__
#define IOP__CTYPES_H__

#define isascii(c)	((unsigned int)(c) <= 127)
#define toascii(c)	((unsigned char)(c) & 127)

#define isblank(c)	((c == ' ') || (c == '\t'))

#ifdef USE_IOP_CTYPE_MACRO

#define islower(c)	((c >= 'a') && (c <= 'z'))
#define isupper(c)	((c >= 'A') && (c <= 'Z'))
#define isdigit(c)	((c >= '0') && (c <= '9'))
#define iscntrl(c)	((c >= 0) && (c < 32))
#define isprint(c)	((c >= 32) && (c <= 127))

#define isspace(c)	(isblank(c) || (c == '\f') || (c == '\n') || (c == '\r') || (c == '\v'))
#define isxdigit(c)	(isdigit(c) || ((c >= 'A') && (c <= 'F')))

#define isalnum(c)	(isalpha(c) || isdigit(c))
#define isalpha(c)	(isupper(c) || islower(c))
#define isgraph(c)	(isprint(c) && !isspace(c))
#define ispunct(c)	(isprint(c) && !(isspace(c) || isalnum(c)))

#define toupper(c)	((unsigned char)(c) + ('a' - 'A'))
#define tolower(c)	((unsigned char)(c) - ('a' - 'A'))

#else

#define	_U		0x01	/* Uppercase letter */
#define	_L		0x02	/* Lowercase letter */
#define	_N		0x04	/* Digit (0-9) */
#define	_S		0x08	/* Space, tab, newline, vertical tab, formfeed or carriage return */
#define _P		0x10	/* Punctuation character */
#define _C		0x20	/* Control character or delete */
#define _X		0x40	/* Hexadecimal digit (0-9, a-f, A-F) */
#define	_B		0x80	/* Blank (space) */

#define	isalpha(c)	(look_ctype_table((unsigned int)(c)) & (_U|_L))
#define	isupper(c)	(look_ctype_table((unsigned int)(c)) & (_U))
#define	islower(c)	(look_ctype_table((unsigned int)(c)) & (_L))
#define	isdigit(c)	(look_ctype_table((unsigned int)(c)) & (_N))
#define	isxdigit(c)	(look_ctype_table((unsigned int)(c)) & (_X|_N))
#define	isspace(c)	(look_ctype_table((unsigned int)(c)) & (_S))
#define ispunct(c)	(look_ctype_table((unsigned int)(c)) & (_P))
#define isalnum(c)	(look_ctype_table((unsigned int)(c)) & (_U|_L|_N))
#define isprint(c)	(look_ctype_table((unsigned int)(c)) & (_P|_U|_L|_N|_B))
#define isgraph(c)	(look_ctype_table((unsigned int)(c)) & (_P|_U|_L|_N))
#define iscntrl(c)	(look_ctype_table((unsigned int)(c)) & (_C))

#endif

#endif	/* IOP__CTYPES_H__ */

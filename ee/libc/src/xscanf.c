/*
# _____     ___ ____     ___ ____
#  ____|   |    ____|   |        | |____|
# |     ___|   |____ ___|    ____| |    \    PS2DEV Open Source Project.
#-----------------------------------------------------------------------
# Copyright (c) 2005, ps2dev - http://www.ps2dev.org
# Licenced under Academic Free License version 2.0
# Review ps2sdk README & LICENSE files for further details.
#
# $Id$
# Various *scanf functions.
*/

/* This code is modified from avr libc.
   Original (C) text and license of this code follows */
   

/* Copyright (c) 2002,2004 Joerg Wunsch
   All rights reserved.

   Redistribution and use in source and binary forms, with or without
   modification, are permitted provided that the following conditions are met:

   * Redistributions of source code must retain the above copyright
     notice, this list of conditions and the following disclaimer.

   * Redistributions in binary form must reproduce the above copyright
     notice, this list of conditions and the following disclaimer in
     the documentation and/or other materials provided with the
     distribution.

   * Neither the name of the copyright holders nor the names of
     contributors may be used to endorse or promote products derived
     from this software without specific prior written permission.

  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
  AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
  IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
  ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE
  LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
  CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
  SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
  INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
  CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
  ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
  POSSIBILITY OF SUCH DAMAGE.
*/

#include <tamtypes.h>
#include <ctype.h>
#include <limits.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/*
 * This file can be compiled into more than one flavour.  The default
 * is to offer the usual modifiers and integer formatting support
 * (level 2).  Level 1 maintains a minimal version that just offers
 * integer formatting, but no modifier support except "h" and "l".
 * Level 3 is intented for floating point support, and also adds the
 * %[ conversion.
 */

#define SCANF_MIN 1
#define SCANF_STD 2
#define SCANF_FLT 3

#ifndef SCANF_LEVEL
#  define SCANF_LEVEL SCANF_FLT
#endif

#if SCANF_LEVEL == SCANF_MIN || SCANF_LEVEL == SCANF_STD \
|| SCANF_LEVEL == SCANF_FLT
/* OK */
#else
#  error "Not a known scanf level."
#endif

#if SCANF_LEVEL >= SCANF_MIN
#define FLHASPERCENT	0x01	/* first % found */
#define	FLUNSIGNED	0x02	/* arg is unsinged (long) * */
#define FLLONG		0x04	/* arg is long int * */
#define FLMINUS		0x08	/* minus sign scanned */
#define FLSHORT		0x10    /* arg is short int * */
#endif

#if SCANF_LEVEL >= SCANF_STD
#define FLSTAR		0x10	/* suppress assingment (* fmt) */
#endif

#if SCANF_LEVEL >= SCANF_FLT
#define FLBRACKET	0x20	/* %[ found, now collecting the set */
#define FLNEGATE	0x40	/* negate %[ set */

/* bit set macros for %[ format */
#define xscanf_set_bit(i) \
	buf[(unsigned char)(i) / 8] |= (1 << (unsigned char)(i) % 8)
#define xscanf_bit_is_set(i) \
	(buf[(unsigned char)(i) / 8] & (1 << (unsigned char)(i) % 8))

/*
 * 40 Bytes is sufficient for 32-bit floating point values in `f'
 * style notation.  Note that this buffer is also used to record the
 * bit vector for %[ formats, so it must be at least 256/8 = 32 bytes
 * long.
 */
#define FLTBUF	40
#endif

#ifdef F_vxscanf
#define w_xgetc(s) xgetc(s), clen++

int vxscanf(int (*xgetc)(void *), void (*xungetc)(int, void *), void *stream, const char *fmt, va_list ap) {
	union {
#if SCANF_LEVEL >= SCANF_FLT
		double	d;
#endif
		unsigned long ul;
		long	l;
		char	*cp;
	} a;
	char	c;	/* holds a char from the format string */
	u8	base = 0;
	int	nconvs, i, j = 0, olen, clen;
#if SCANF_LEVEL > SCANF_MIN
	s8	width = 0;
#endif
	u8 flags;
#if SCANF_LEVEL >= SCANF_FLT
	char	*buf, *bp;
	char	fltchars[] = "0123456789Ee.";
#endif

	flags = 0;
	nconvs = 0;
	i = 0;
	olen = clen = 0;
#if SCANF_LEVEL >= SCANF_FLT
	buf = 0;
#endif

	while ((c = *(fmt++))) {

		fmt++;

#if SCANF_LEVEL >= SCANF_FLT
		if (flags & FLBRACKET) {
			if (c == '^' && i == 0 && !(flags & FLNEGATE)) {
				flags |= FLNEGATE; /* negate set */
				continue; /* without bumping i */
			}
			if (c == '-') {
				if (i == 0) {
				  addbit:
					xscanf_set_bit(c);
					i++;
					continue;
				}
				flags |= FLMINUS;
				i++;
				continue;
			}
			if (c == ']') {
				if (i == 0)
					goto addbit;
				if (flags & FLMINUS) /* trailing - before ] */
					xscanf_set_bit('-');
				if (flags & FLNEGATE)
					for (i = 0; i < 256 / 8; i++)
						buf[i] = ~buf[i];
				if (!(flags & FLSTAR))
					a.cp = va_arg(ap, char *);
				while (width-- > 0) {
					if ((i = w_xgetc(stream)) == EOF)
						break;
					if (!xscanf_bit_is_set(i)) {
						xungetc(i, stream);
						break;
					}
					if (!(flags & FLSTAR))
						*a.cp++ = i;
				}
				if (!(flags & FLSTAR))
					*a.cp = '\0';
				goto nextconv;
			}
			if (flags & FLMINUS) {
				flags &= ~FLMINUS;
				while ((unsigned char)j < (unsigned char)c) {
					xscanf_set_bit(j);
					j++;
				}
			}
			j = (unsigned char)c; /* remember for x-y range */
			goto addbit;
		}
#endif /* SCANF_LEVEL >= SCANF_FLT */
		if (flags & FLHASPERCENT) {
			if (c == '%') {
				flags &= ~FLHASPERCENT;
#if SCANF_LEVEL > SCANF_MIN
				goto literal;
#else
				continue;
#endif
			}

			/*
			 * Modifiers go first.  They all end up in a
			 * "continue" statement so to fetch the next
			 * char from the format string.
			 */
#if SCANF_LEVEL > SCANF_MIN
			if (c >= '0' && c <= '9') {
				c -= '0';
				if (width == SCHAR_MAX)
					width = 0;
				else
					width *= 10;
				width += c;
				continue;
			}
#endif /* SCANF_LEVEL > SCANF_MIN */

			c = tolower(c);
#if SCANF_LEVEL >= SCANF_FLT
			if ((c == '[' || c == 'e' || c == 'f' || c == 'g') &&
			    buf == 0) {
				if ((buf = malloc(FLTBUF)) == 0)
					return EOF;
			}
#endif /* SCANF_LEVEL >= SCANF_FLT */

			switch (c) {
#if SCANF_LEVEL > SCANF_MIN
			case '*':
				flags |= FLSTAR;
				continue;
#endif /* SCANF_LEVEL > SCANF_MIN */

			case 'h':
#if SHRT_MAX != INT_MAX
				flags |= FLSHORT;
#endif
				/*
				 * short int and int are identical on
				 * our target platform, ignore.
				 */
				continue;

			case 'l':
				flags |= FLLONG;
				continue;

				/*
				 * Actual conversion specifications go
				 * here.
				 */
			case 'c':
#if SCANF_LEVEL > SCANF_MIN
				if (!(flags & FLSTAR))
#endif /* SCANF_LEVEL > SCANF_MIN */
					a.cp = va_arg(ap, char *);
#if SCANF_LEVEL > SCANF_MIN
				if (width == SCHAR_MAX)
					width = 1;
				while (width-- > 0) {
#endif /* SCANF_LEVEL > SCANF_MIN */
					if ((i = w_xgetc(stream)) == EOF)
						goto leave;
#if SCANF_LEVEL > SCANF_MIN
					if (!(flags & FLSTAR))
#endif /* SCANF_LEVEL > SCANF_MIN */
						*a.cp++ = i;
#if SCANF_LEVEL > SCANF_MIN
				}
#endif /* SCANF_LEVEL > SCANF_MIN */
				break;

			case 's':
#if SCANF_LEVEL > SCANF_MIN
				if (!(flags & FLSTAR))
#endif /* SCANF_LEVEL > SCANF_MIN */
					a.cp = va_arg(ap, char *);
				do {
					i = w_xgetc(stream);
				} while (isspace(i));
				if (i == EOF)
					goto leave;

#if SCANF_LEVEL > SCANF_MIN
				while (width-- > 0)
#else
				for (;;)
#endif /* SCANF_LEVEL > SCANF_MIN */
				{
					if (isspace(i)) {
						xungetc(i, stream);
						break;
					}
#if SCANF_LEVEL > SCANF_MIN
					if (!(flags & FLSTAR))
#endif /* SCANF_LEVEL > SCANF_MIN */
						*a.cp++ = i;
					if ((i = w_xgetc(stream)) == EOF)
						break;
				}
#if SCANF_LEVEL > SCANF_MIN
				if (!(flags & FLSTAR))
#endif /* SCANF_LEVEL > SCANF_MIN */
					*a.cp = '\0';
				break;

			case 'o':
				base = 8;
				flags |= FLUNSIGNED;
				goto dointeger;

			case 'p':
				/*
				 * Handle pointers as plain unsigned
				 * integers.  This assumes that
				 * sizeof(void *) == sizeof(unsigned int).
				 */
			case 'x':
				base = 16;
				/* FALLTHROUGH */

			case 'u':
				flags |= FLUNSIGNED;
				/* FALLTHROUGH */

			case 'd':
			case 'i':
			  dointeger:
				do {
					i = w_xgetc(stream);
				} while (isspace(i));
				if (i == EOF)
					goto leave;

				if ((char)i == '-' || (char)i == '+') {
#if SCANF_LEVEL > SCANF_MIN
					if (--width <= 0)
						/*
						 * Incomplete conversion
						 * due to field width
						 * truncation.
						 */
						goto leave;
#endif /* SCANF_LEVEL > SCANF_MIN */
					if ((char)i == '-')
						flags |= FLMINUS;
					if ((i = w_xgetc(stream)) == EOF)
						goto leave;
				}

				if ((char)i == '0') {
					/*
					 * %i conversions default to base
					 * 10, but allow for base 8
					 * indicated by a leading 0 in
					 * input, or base 16 indicated by
					 * leading 0x/0X.
					 *
					 * For %x (and %p) conversions, the
					 * leading 0x/0X is explicitly
					 * allowable.
					 *
					 * If we fail the conversion here,
					 * it is a mismatch condition, but
					 * since we already saw a zero,
					 * this means the current
					 * conversion succeeded, assigning
					 * 0.
					 */
					a.ul = 0;

#if SCANF_LEVEL > SCANF_MIN
					if (--width <= 0)
						goto intdone;
#endif /* SCANF_LEVEL > SCANF_MIN */
					if ((i = w_xgetc(stream)) == EOF)
						goto intdone;
					if ((char)tolower(i) == 'x') {
						if (c == 'o' ||
						    c == 'd' || c == 'u') {
							/*
							 * Invalid 0x in
							 * %d/%u/%o
							 */
							xungetc(i, stream);
							goto intdone;
						}
						base = 16;
						if ((i = w_xgetc(stream)) == EOF)
							goto intdone;
					} else if (c == 'i')
						base = 8;
				}

				a.ul = 0;
				for (;;) {
					j = tolower(i);
					/*
					 * First, assume it is a decimal
					 * digit.
					 */
					j -= '0';
					if (j > 9) {
						/*
						 * Not a decimal digit.
						 * Try hex next.
						 */
						j += '0'; /* undo "- '0'"
							   * above */
						j -= 'a'; /* 'a' is first
							   * hex digit */
						if (j >= 0)
							/* 'a' has value
							 * 10 */
							j += 10;
						/*
						 * else: not a hex digit,
						 * gets caught below.
						 */
					}
					if (j < 0 || j >= base) {
						xungetc(i, stream);
						break;
					}
					a.ul *= base;
					a.ul += j;
#if SCANF_LEVEL > SCANF_MIN
					if (--width <= 0)
						break;
#endif /* SCANF_LEVEL > SCANF_MIN */
					if ((i = w_xgetc(stream)) == EOF)
						break;
				}
				/*
				 * This is a bit of a hack: while we
				 * collect all integer digits in an
				 * unsigned long number in order to be
				 * safe for unsigned conversions, the
				 * standard allows for optional signs.
				 * We are thus faced to the concept of
				 * possibly negating an unsigned
				 * number. :-/  We rely here on union a
				 * mapping the signed and unsigned
				 * fields suitably.
				 */
				if (flags & FLMINUS)
					a.l = -a.l;
			  intdone:
#if SCANF_LEVEL > SCANF_MIN
				if (!(flags & FLSTAR)) {
#endif /* SCANF_LEVEL > SCANF_MIN */
					if ((flags & (FLLONG | FLUNSIGNED)) == (FLLONG | FLUNSIGNED))
						*(va_arg(ap, unsigned long *)) = a.ul;
					else if ((flags & (FLSHORT | FLUNSIGNED)) == (FLSHORT | FLUNSIGNED))
						*(va_arg(ap, unsigned short *)) = (unsigned short)a.ul;
					else if (flags & (FLUNSIGNED))
						*(va_arg(ap, unsigned *)) = (unsigned)a.ul;
					else if (flags & FLLONG)
						*(va_arg(ap, long *)) = a.l;
					else if (flags & FLSHORT)
						*(va_arg(ap, short *)) = (short)a.l;
					else
						*(va_arg(ap, int *)) = (int)a.l;
#if SCANF_LEVEL > SCANF_MIN
				}
#endif /* SCANF_LEVEL > SCANF_MIN */
				break;

#if SCANF_LEVEL > SCANF_MIN
			case 'n':
				if (!(flags & FLSTAR))
					*(va_arg(ap, int *)) = clen;
				break;
#endif /* SCANF_LEVEL > SCANF_MIN */

#if SCANF_LEVEL >= SCANF_FLT
			case 'e':
			case 'f':
			case 'g':
				do {
					i = w_xgetc(stream);
				} while (isspace(i));
				if (i == EOF)
					goto leave;

				if ((char)i == '-' || (char)i == '+') {
					if ((char)i == '-')
						flags |= FLMINUS;
					if ((i = w_xgetc(stream)) == EOF)
						goto leave;
				}

				a.d = 0.0;
				for (bp = buf;
				     bp < buf + FLTBUF - 1 && width > 0;
				     width--) {
					if (strchr(fltchars, i) == 0) {
						xungetc(i, stream);
						break;
					}
					if ((char)i == 'e' ||
					    (char)i == 'E') {
						/*
						 * Prevent another 'E'
						 * from being recognized.
						 */
						fltchars[10] = 0;
						*bp++ = i;
						if ((i = w_xgetc(stream)) == EOF)
							break;
						if ((char)i != '-' &&
						    (char)i != '+')
							continue;
					} else if ((char)i == '.')
						/*
						 * Prevent another dot from
						 * being recognized.  If we
						 * already saw an 'E'
						 * above, we could not get
						 * here at all.
						 */
						fltchars[12] = 0;
					*bp++ = i;
					if ((i = w_xgetc(stream)) == EOF)
						break;
				}
				*bp++ = 0;
				a.d = strtod(buf, 0);
				if (flags & FLMINUS)
					a.d = -a.d;
				*(va_arg(ap, double *)) = a.d;
				/*
				 * Restore the 'E' and '.' chars that
				 * might have been clobbered above.
				 */
				fltchars[10] = 'E';
				fltchars[12] = '.';
				break;

			case '[':
				flags |= FLBRACKET;
				i = j = 0;
				memset(buf, 0, 256 / 8);
				continue;
#endif /* SCANF_LEVEL >= SCANF_FLT */
			}

#if SCANF_LEVEL >= SCANF_FLT
		  nextconv:
#endif
			flags = 0;
			if (clen > olen)
				nconvs++;
			else if (c != 'n' || i == EOF)
				/*
				 * If one conversion failed completely,
				 * punt.
				 */
				goto leave;
		} else if (c == '%') {
			flags = FLHASPERCENT;
			base = 10;
			olen = clen;
#if SCANF_LEVEL > SCANF_MIN
			width = SCHAR_MAX;
		} else if (isspace(c)) {
			/* match against any whitspace */
			do {
				i = w_xgetc(stream);
			} while (isspace(i));
			if (i == EOF)
				goto leave;
			xungetc(i, stream);
		} else {
			/* literal character in format, match it */
		  literal:
			if ((i = w_xgetc(stream)) == EOF)
				goto leave;
			if (i != c)
				goto leave;
#endif /* SCANF_LEVEL > SCANF_MIN */
		}
	}
leave:
	/*
	 * If a conversion was aborted (usually due to input failure
	 * or end-of-file), adjust the total number of conversions
	 * done if at least one char could be read from the stream.
	 */
	if ((flags & FLHASPERCENT) && clen > olen)
		nconvs++;
	/*
	 * If an error occurs before the first successful conversion,
	 * we ought to return EOF.  Before getting here, all
	 * conversions maintain the last character read from the
	 * stream (or EOF) within variable `i'.
	 */
	if (nconvs == 0 && i == EOF)
		nconvs = EOF;

#if SCANF_LEVEL >= SCANF_FLT
	free(buf);
#endif
	return nconvs;
}
#endif

#ifdef F_xscanf
int xscanf(int (*xgetc)(void *), void (*xungetc)(int, void *), void *stream, const char *fmt, ...) {
	int r;
        va_list args;
        va_start(args, fmt);
	r = vxscanf(xgetc, xungetc, stream, fmt, args);
        va_end(args);
	return r;
}
#endif

int _f_xgetc(void *);
void _f_xungetc(int, void *);
int _s_xgetc(void *);
void _s_xungetc(int, void *);

#ifdef F__xscanf_internals
int _f_xgetc(void * stream) {
    return fgetc((FILE *) stream);
}

void _f_xungetc(int c, void * stream) {
    ungetc(c, (FILE *) stream);
}

int _s_xgetc(void * p_str) {
    char c = *((*((char **)p_str))++);
    return c ? c : EOF;
}

void _s_xungetc(int c, void * p_str) {
    *(--(*((char **)p_str))) = c;
}
#endif

#ifdef F_scanf
int scanf(const char * fmt, ...) {
	int r;
        va_list args;
        va_start(args, fmt);
	r = vxscanf(_f_xgetc, _f_xungetc, stdin, fmt, args);
        va_end(args);
	return r;
}
#endif

#ifdef F_sscanf
int sscanf(const char * str, const char * fmt, ...) {
	int r;
        va_list args;
        va_start(args, fmt);
	r = vxscanf(_s_xgetc, _s_xungetc, (void *) str, fmt, args);
        va_end(args);
	return r;
}
#endif

#ifdef F_fscanf
int fscanf(FILE * stream, const char * fmt, ...) {
	int r;
        va_list args;
        va_start(args, fmt);
	r = vxscanf(_f_xgetc, _f_xungetc, stream, fmt, args);
        va_end(args);
	return r;
}
#endif

#ifdef F_vscanf
int vscanf(const char * fmt, va_list args) {
	return vxscanf(_f_xgetc, _f_xungetc, stdin, fmt, args);
}
#endif

#ifdef F_vsscanf
int vsscanf(const char * str, const char * fmt, va_list args) {
	return vxscanf(_s_xgetc, _s_xungetc, (void *) str, fmt, args);
}
#endif

#ifdef F_vfscanf
int vfscanf(FILE * stream, const char * fmt, va_list args) {
	return vxscanf(_f_xgetc, _f_xungetc, stream, fmt, args);
}
#endif

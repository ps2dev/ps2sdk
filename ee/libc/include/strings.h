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
# Strings function prototypes
*/
#ifndef __STRINGS_H__
#define __STRINGS_H__

#define __need_size_t
#include <stddef.h>

#include <string.h>

#ifdef __cplusplus
extern "C" {
#endif

// copies ascii string to sjis string
//
// args:    dest sjis string buffer
//          source ascii string buffer
// returns: length of ascii string copied
int strcpy_sjis(short* sjis_buff, const char* ascii_buff);

// copies sjis string to ascii string
//
// args:    dest ascii string buffer
//          source sjis string buffer
// returns: length of sjis string copied
int strcpy_ascii(char* ascii_buff, const short* sjis_buff);

/* Try to make gnu89, C89, C99, C11, gnu99 compatible inline. */
#ifndef INLINE
# if __GNUC__ && !__GNUC_STDC_INLINE__
#  define INLINE static __inline__
# else
#  define INLINE static inline
# endif
#endif

INLINE void bzero(void * p, size_t n) { memset(p, 0, n); }
INLINE void bcopy(const void * s, void * d, size_t n) { memcpy(d, s, n); }
INLINE int bcmp(const void * s1, const void * s2, size_t n) { return memcmp(s1, s2, n); }
INLINE char * index(const char * s, int c) { return strchr(s, c); }
INLINE char * rindex(const char * s, int c) { return strrchr(s, c); }

#undef INLINE

/* C String functions by Hiryu (A.Lee) */
#define stricmp strcasecmp
#define strnicmp strncasecmp

int	 strcasecmp(const char *, const char *);
int	 strncasecmp(const char *, const char *, size_t);

#ifdef __cplusplus
}
#endif

#endif /*__STRINGS_H__*/
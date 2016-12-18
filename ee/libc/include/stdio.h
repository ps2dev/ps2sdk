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
# LIBC (stdio) type, constant and function declarations.
*/
#ifndef __STDIO_H__
#define __STDIO_H__

#define __need__va_list
#include <stdarg.h>

#define __need_size_t
#define __need_NULL
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

#define BUFSIZ                         1024

#define _NFILE                         16

#define _IOFBF                         0x0000
#define _IOLBF                         0x0100
#define _IONBF                         0x0004

/* ensure EOF is defined. */
#ifndef EOF
#define EOF                            (-1)
#endif


#define FOPEN_MAX                      _NFILE
#define FILENAME_MAX                   1024


/* ensure fpos_t is defined. */
#ifndef __FPOS_T_DEFINED
#define __FPOS_T_DEFINED
typedef long fpos_t;
#endif // __FPOS_T_DEFINED


/* ensure FILE is defined. */
#ifndef __FILE_DEFINED
#define __FILE_DEFINED
typedef struct {
  int  type;
  int  fd;
  int  cnt;
  int  flag;
  int  has_putback;
  int  putback;
} FILE;
#endif // __FILE_DEFINED


extern FILE __iob[_NFILE];

#define stdin                          (&__iob[0])
#define stdout                         (&__iob[1])
#define stderr                         (&__iob[2])

#define	SEEK_SET	0
#define	SEEK_CUR	1
#define	SEEK_END	2

/* function declarations. */
void   clearerr(FILE *);
int    feof(FILE *);
int    ferror(FILE *);
FILE   *fopen(const char *, const char *);
FILE   *fdopen(int, const char *);
int    fclose(FILE *);
int    fflush(FILE *);
int    fgetc(FILE *);
int    fgetpos(FILE *, fpos_t *);
char   *fgets(char *, int, FILE *);
int    fileno(FILE *);
int    fputc(int, FILE *);
int    fputs(const char *, FILE *);
size_t fread(void *, size_t, size_t, FILE *);
FILE   *freopen(const char *, const char *, FILE *);
int    fscanf(FILE *, const char *, ...);
int    fseek(FILE *, long, int);
int    fsetpos(FILE *, const fpos_t *);
long   ftell(FILE *);
size_t fwrite(const void *, size_t, size_t, FILE *);
int    getc(FILE *);
int    getchar(void);
char   *gets(char *);
void   perror(const char *);
int    putc(int, FILE *);
int    puts(const char *);
int    rename(const char *, const char *);
int    remove(const char *);
void   rewind(FILE *);
int    scanf(const char *, ...);
int    setbuf(FILE *, char *);
int    setvbuf(FILE *, char *, int, size_t);
int    sscanf(const char *, const char *, ...);
FILE   *tmpfile(void);
char   *tmpnam(char *);
int    vfscanf(FILE *, const char *, va_list);
int    vscanf(const char *, va_list);
int    vsscanf(const char *, const char *, va_list);
int    vxscanf(int (*xgetc)(void **), void (*xungetc)(int, void **), void *stream, const char *, va_list);
int    xscanf(int (*xgetc)(void **), void (*xungetc)(int, void **), void *stream, const char *, ...);
int    ungetc(int, FILE *);

int    _fcloseall(void);
int    _fflushall(void);

int    chdir(const char *path);

/* from xprintf */
int vxprintf(void (*func)(char*, int, void *), void * arg, const char * format, va_list ap);
int vsnprintf(char *buf, size_t n, const char *fmt, va_list ap);
int vsprintf(char *buf, const char *fmt, va_list ap);
char *vmprintf(const char *zFormat, va_list ap);
int vfprintf(FILE *pOut, const char *zFormat, va_list ap);
int vprintf(const char *format, va_list ap);
int vasprintf(char **strp, const char *format, va_list ap);

int xprintf(void (*func)(char*, int, void *), void * arg, const char * format, ...)
    __attribute__((format(printf,3,4)));
int snprintf(char *str, size_t sz, const char *format, ...)
    __attribute__((format(printf,3,4)));
int sprintf(char *buf, const char *fmt, ...)
    __attribute__((format(printf,2,3)));
char *mprintf(const char *zFormat, ...)
    __attribute__((format(printf,1,2)));
int fprintf(FILE *pOut, const char *zFormat, ...)
    __attribute__((format(printf,2,3)));
int printf(const char *format, ...)
    __attribute__((format(printf,1,2)));
int asprintf(char **strp, const char *format, ...)
    __attribute__((format(printf,2,3)));

int putchar(int);

int sio_printf(const char *format, ...);

#ifdef __cplusplus
}
#endif


#endif // __STDIO_H__

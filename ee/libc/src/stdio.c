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
# Simple standard C library implementation.
*/
	   
#include <stdio.h>
#include <tamtypes.h>
#include <kernel.h>
#include <sio.h>
#include <fileio.h>
#include <string.h>
#include <limits.h>


/* std I/O buffer type constants. */
#define STD_IOBUF_TYPE_NONE            0
#define STD_IOBUF_TYPE_GS              1
#define STD_IOBUF_TYPE_SIO             2
#define STD_IOBUF_TYPE_CDROM           4
#define STD_IOBUF_TYPE_MC              8
#define STD_IOBUF_TYPE_HOST           16
#define STD_IOBUF_TYPE_STDOUTHOST     32


#ifdef F_clearerr
/*
**
**  [func] - clearerr.
**  [desc] - clears the stream file stream error condition.
**  [entr] - FILE *stream; the pointer to the FILE stream.
**  [exit] - none.
**  [prec] - stream is a valid FILE pointer.
**  [post] - the stream file stream error condition is cleared.
**
*/
void clearerr(FILE *stream)
{
  stream->flag &= (~_IOERR);
}
#endif


#ifdef F_fclose
/*
**
**  [func] - fclose.
**  [desc] - if stream is a valid FILE stream and able to close the stream file
**           then returns 0. else returns EOF.
**  [entr] - FILE *stream; the pointer to the FILE stream.
**  [exit] - int; 0 if able to close the stream file. else EOF.
**  [prec] - stream is a valid FILE pointer.
**  [post] - the stream file is closed.
**
*/
int fclose(FILE *stream)
{
  int ret;

  /* test the file stream type. */
  switch(stream->type) {
    case STD_IOBUF_TYPE_NONE:
    case STD_IOBUF_TYPE_GS:
    case STD_IOBUF_TYPE_SIO:
    case STD_IOBUF_TYPE_STDOUTHOST:
      /* cannot close stdin, stdout, or stderr. */
      // duh.. this is wrong. One SHOULD be able to close
      // std*. That's a common unix doing. However, I doubt
      // allowing this madness could be a good idea.
      ret = EOF;
      break;
    default:
      if ((stream->fd >= 0) && (fioClose(stream->fd) >= 0)) {
        stream->type = STD_IOBUF_TYPE_NONE;
        stream->fd = -1;
        stream->cnt = 0;
        stream->flag = 0;
        ret = 0;
      }
      else ret = EOF;
  }
  return (ret);
}
#endif


#ifdef F_fcloseall
/*
**
**  [func] - fcloseall.
**  [desc] - attempts to close all the open files. if able to close all the open
**           files then returns the number of files closed. else returns -1.
**  [entr] - none.
**  [exit] - int; the number of files closed if successful. else -1.
**  [prec] - none.
**  [post] - all open non-system files are closed.
**
*/
int fcloseall(void)
{
  int  i, ret = 0;
  FILE *iob;

  /* process all open files except for stdout, stdin and stderr. */
  for (i = 3, iob = &__iob[3]; i < _NFILE; ++i, ++iob) {
    if (iob->fd >= 0) {
      /* attempt to close the current file. */
      if ((fclose(iob) == 0) && (ret >= 0)) ++ret;
      else ret = EOF;
    }
  }
  return (ret);
}
#endif


#ifdef F_feof
/*
**
**  [func] - feof.
**  [desc] - if the stream file stream has reached the end of the file then
**           returns non-zero. else returns 0.
**  [entr] - FILE *stream; the pointer to the FILE stream.
**  [exit] - int; non-zero if the stream file has reached EOF. else 0.
**  [prec] - stream is a valid FILE pointer.
**  [post] - none.
**
*/
int feof(FILE *stream)
{
  return ((stream->flag & _IOEOF) != 0);
}
#endif


#ifdef F_ferror
/*
**
**  [func] - ferror.
**  [desc] - if an error has occured for the stream file stream then returns
**           non-zero. else returns 0.
**  [entr] - FILE *stream; the pointer to the FILE stream.
**  [exit] - int; non-zero if error has occured for the stream file. else 0.
**  [prec] - stream is a valid FILE pointer.
**  [post] - none.
**
*/
int ferror(FILE *stream)
{
  return ((stream->flag & _IOERR) != 0);
}
#endif


#ifdef F_fflush
int mcFlush(int fd);
int mcSync(int mode, int *cmd, int *result);

/*
**
**  [func] - fflush.
**  [desc] - if the stream file is opened as read-only then returns 0. else
**           if stream is a valid FILE stream and able to flush the stream
**           file write buffer then returns 0. else returns EOF.
**  [entr] - FILE *stream; the pointer to the FILE stream.
**  [exit] - int; 0 if able to flush the write buffer or file is read-only. else EOF.
**  [prec] - stream is a valid FILE pointer.
**  [post] - the stream FILE stream write buffer is flushed.
**
*/
int fflush(FILE *stream)
{
  int ret;

  switch(stream->type) {
    case STD_IOBUF_TYPE_GS:
    case STD_IOBUF_TYPE_SIO:
    case STD_IOBUF_TYPE_STDOUTHOST:
      /* stdout & stderr are never buffered. */
    case STD_IOBUF_TYPE_CDROM:
      /* cd-rom files are read-only so no write buffer to flush. */
      ret = 0;
      break;
    case STD_IOBUF_TYPE_MC:
      if (stream->flag & (_IOWRT | _IORW)) {
        /* flush memory card file write buffer. */
        mcFlush(stream->fd);
        mcSync(0, NULL, &ret);
        if (ret != 0) ret = EOF;
      }
      else ret = 0;
      break;
    case STD_IOBUF_TYPE_HOST:
      /* flush host file write buffer. */
      if (stream->flag & (_IOWRT | _IORW)) ret = 0;
      else ret = 0;
      break;
    default:
      /* unknown/invalid I/O buffer type. */
      ret = EOF;
  }
  return (ret);
}
#endif


#ifdef F_fflushall
/*
**
**  [func] - _fcloseall.
**  [desc] - attempts to flush all the open files with write-access. if able
**           to flush all the open files with write-access then returns the
**           number of files flushed. else returns -1.
**  [entr] - none.
**  [exit] - int; the number of files flushed if successful. else -1.
**  [prec] - none.
**  [post] - all open non-system files with write-access are flushed.
**
*/
int _fflushall(void)
{
  int  i, ret = 0;
  FILE *iob;

  /* process all open files except for stdout, stdin and stderr. */
  for (i = 3, iob = &__iob[3]; i < _NFILE; ++i, ++iob) {
    if (iob->fd >= 0) {
      /* attempt to flush the current file. */
      if ((fflush(iob) == 0) && (ret >= 0)) ++ret;
      else ret = EOF;
    }
  }
  return (ret);
}
#endif


#ifdef F_fgetc
/*
**
**  [func] - fgetc.
**  [desc] - attempts to read one character from the stream file. if able to
**           read one character from the file then returns the chaaracter
**           read. else EOF.
**  [entr] - FILE *stream; the pointer to the FILE stream.
**  [exit] - int; the character read from the stream file. else -1.
**  [prec] - stream is a valid FILE pointer.
**  [post] - the stream file is modified.
**
*/
int fgetc(FILE *stream)
{
  char c;
  int  ret;

  if (stream->has_putback) {
    stream->has_putback = 0;
    return stream->putback;
  }

  switch(stream->type) {
    case STD_IOBUF_TYPE_GS:
    case STD_IOBUF_TYPE_SIO:
    case STD_IOBUF_TYPE_STDOUTHOST:
      /* cannot read from stdout or stderr. */
      ret = EOF;
      break;
    default:
      ret = ((fread(&c, 1, 1, stream) == 1) ? (int)c : EOF);
  }
  return (ret);
}
#endif


#ifdef F_fgetpos
/*
**
**  [func] - fgetpos.
**  [desc] - attempts to retrieve the stream file stream pointer position.
**           if able to retrieve the stream file stream pointer position
**           then stores the position to pos and returns 0. else returns -1.
**  [entr] - FILE *stream; the pointer to the file stream.
**           fpos_t *pos; the pointer to the destination file position buffer.
**  [exit] - int; 0 if able to retrieve the stream file pointer position. else -1.
**  [prec] - stream is a valid FILE pointer and pos is a valid fpos_t pointer.
**  [post] - the memory pointed to by pos is modified.
**
*/
int fgetpos(FILE *stream, fpos_t *pos)
{
  long n;

  if ((n = ftell(stream)) >= 0) *pos = (fpos_t)n;
  return ((n >= 0) ? 0 : -1);
}
#endif


#ifdef F_fgets
/*
**
**  [func] - fgets.
**  [desc] - attempts to read a string from the stream file. if able to read
**           a string from the stream file stdin then stores the string up to
**           n characters to the memory pointed by buf and returns buf. else
**           returns NULL.
**  [entr] - char *buf; the pointer to the destination string buffer.
**           int n; the maximum number of characters to write to buf.
**           FILE *stream; the pointer to the FILE stream.
**  [exit] - char *; buf if the string is read successfully. else NULL.
**  [prec] - buf is a valid memory pointer of n size in bytes and stream is a
**           valid FILE pointer.
**  [post] - the memory pointed to by buf is modified and the stream file
**           pointer is modified.
**
*/
char *fgets(char *buf, int n, FILE *stream)
{
  char *ret = buf;
  int  c, done;

  switch(stream->type) {
    case STD_IOBUF_TYPE_GS:
    case STD_IOBUF_TYPE_SIO:
    case STD_IOBUF_TYPE_STDOUTHOST:
      /* cannot read from stdout or stderr. */
      ret = NULL;
      break;
    default:
      for (done = 0; (!done); ) {
        switch(c = fgetc(stream)) {
          case '\r':
          case '\n':
            if (n > 0) {
              /* newline terminates fgets. */
              *buf++ = '\0';
              --n;
              done = 1;
              break;
            }
            break;
          case EOF:
            /* end of file or error. */
            ret = NULL;
            done = 1;
            break;
          default:
            if (n > 0) {
              /* store the current character to buf. */
              *buf++ = (char)c;
              --n;
            }
        }
      }
  }
  return (ret);
}
#endif


#ifdef F_fopen
/* std I/O internal function. */
int __stdio_get_fd_type(const char *);


/*
**
**  [func] - fopen.
**  [desc] - attempts to open the fname file using the mode file mode. if able
**           open the fname then returns the pointer to the FILE stream. else
**           returns NULL.
**  [entr] - const char *fname; the filename string pointer.
**           const char *mode; the file mode string pointer.
**  [exit] - FILE *; the pointer the fname FILE stream. else NULL.
**  [prec] - fname and mode are valid string pointers.
**  [post] - the fname file is opened.
**
*/
FILE *fopen(const char *fname, const char *mode)
{
  FILE *ret = NULL;
  int  fd, flag = 0, i, iomode = 0;

  /* ensure file name and mode are not NULL strings. */
  if ((fname != NULL) && (*fname != '\0')) {
    if ((mode != NULL) && (*mode != '\0')) {
      /* test the file mode. */
      switch(*mode++) {
        case 'r':
          flag = _IOREAD;
          iomode = O_RDONLY;
          break;
        case 'w':
          flag = _IOWRT;
          iomode = (O_WRONLY | O_CREAT);
          break;
        case 'a':
          flag = _IORW;
          iomode = O_APPEND;
          break;
      }
      /* test the extended file mode. */
      for (; (*mode++ != '\0'); ) {
        switch(*mode) {
          case 'b':
            continue;
          case '+':
            flag |= (_IOREAD | _IOWRT);
            iomode |= (O_RDWR | O_CREAT | O_TRUNC);
            continue;
          default:
            break;
        }
      }
      /* search for an available fd slot. */
      for (i = 2; i < _NFILE; ++i) if (__iob[i].fd < 0) break;
      if (i < _NFILE) {
        /* attempt to open the fname file. */
        if ((fd = fioOpen((char *)fname, iomode)) >= 0) {
          __iob[i].type = __stdio_get_fd_type(fname);
          __iob[i].fd = fd;
          __iob[i].cnt = 0;
          __iob[i].flag = flag;
          __iob[i].has_putback = 0;
          ret = (__iob + i);
        }
      }
    }
  }
  return (ret);
}
#endif

#ifdef F_fdopen
/*
**
**  [func] - fdopen.
**  [desc] - produces a file descriptor of type `FILE *', from a 
**           descriptor for an already-open file (returned, for 
**           example, by the system subroutine `open' rather than by `fopen').
**           The MODE argument has the same meanings as in `fopen'.
**  [entr] - int fd; file descriptor returned by 'open'.
**           const char *mode; the file mode string pointer.
**  [exit] - file pointer or `NULL', as for `fopen'.
**
*/
FILE *fdopen(int fd, const char *mode)
{
  FILE *ret = NULL;
  int  flag = 0, i, iomode = 0;

  /* ensure valid descriptor, and that mode is not a NULL string. */
  if (fd >= 0) {
    if ((mode != NULL) && (*mode != '\0')) {
      /* test the file mode. */
      switch(*mode++) {
        case 'r':
          flag = _IOREAD;
          iomode = O_RDONLY;
          break;
        case 'w':
          flag = _IOWRT;
          iomode = (O_WRONLY | O_CREAT);
          break;
        case 'a':
          flag = _IORW;
          iomode = O_APPEND;
          break;
      }
      /* test the extended file mode. */
      for (; (*mode++ != '\0'); ) {
        switch(*mode) {
          case 'b':
            continue;
          case '+':
            flag |= (_IOREAD | _IOWRT);
            iomode |= (O_RDWR | O_CREAT | O_TRUNC);
            continue;
          default:
            break;
        }
      }
      /* search for an available fd slot. */
      for (i = 2; i < _NFILE; ++i) if (__iob[i].fd < 0) break;
      if (i < _NFILE) {
        /* attempt to open the fname file. */
        __iob[i].type = STD_IOBUF_TYPE_NONE;
        __iob[i].fd = fd;
        __iob[i].cnt = 0;
        __iob[i].flag = flag;
        __iob[i].has_putback = 0;
        ret = (__iob + i);
      }
    }
  }
  return (ret);
}
#endif

#ifdef F_fputc
/*
**
**  [func] - fputc.
**  [desc] - attempts to write the c character to the stream file. if able to
**           write the character to the stream file then returns the character
**           written. else returns -1.
**  [entr] - int c; the character to write to the file.
**           FILE *stream; the pointer to the FILE stream.
**  [exit] - int; the character written to the file if successful. else -1.
**  [prec] - stream is a valid FILE pointer.
**  [post] - the stream file is modified.
**
*/
int fputc(int c, FILE *stream)
{
  char ch;

  ch = (char)c;
  return ((fwrite(&ch, 1, 1, stream) == 1) ? 0 : EOF);
}
#endif


#ifdef F_fputs
/*
**
**  [func] - fputs.
**  [desc] - attempts to write the s string to the stream file. if able to
**           successfully write the string to the stream file then returns
**           the number of characters written to the file. else returns -1.
**  [entr] - const char *s; the source string pointer.
**  [exit] - int; the number of chars. written to file if successful. else -1.
**  [prec] - stream is a valid FILE pointer and s is a valid string pointer.
**  [post] - the stream file is modified.
**
*/
int fputs(const char *s, FILE *stream)
{
  size_t len;

  len = ((fwrite(s, 1, (len = strlen(s)), stream) == len) ? (int)len : EOF);
  
  if (len != EOF) {
    fputc('\n', stream);
  }
  return len + 1;
}
#endif


#ifdef F_fread
/*
**
**  [func] - fread.
**  [desc] - attempts to read n number of records of r size to the stream file
**           and returns the number of records successfully read from the file.
**  [entr] - void *buf; the pointer to the destination data buffer.
**           size_t r; the size of the records to read.
**           size_t n; the number of records to read.
**           FILE *stream; the pointer to the FILE stream.
**  [exit] - size_t; the number of records successfully read from the stream file.
**  [prec] - buf is a valid memory pointer of (r * n) size in bytes and stream
**           is a valid FILE pointer.
**  [post] - the stream file is modified.
**
*/
size_t fread(void *buf, size_t r, size_t n, FILE *stream)
{
  size_t ret;

  switch(stream->type) {
    case STD_IOBUF_TYPE_NONE:
    case STD_IOBUF_TYPE_GS:
    case STD_IOBUF_TYPE_SIO:
    case STD_IOBUF_TYPE_STDOUTHOST:
      /* cannot read from stdout or stderr. */
      ret = 0;
      break;
    default:
      /* attempt to read from the stream file. */
      ret = (fioRead(stream->fd, buf, (int)(r * n)) / (int)r);
  }
  return (ret);
}
#endif


#ifdef F_fseek
/*
**
**  [func] - fseek.
**  [desc] - attempts to seek the stream file pointer to offset from origin.
**           if able to seek the stream file pointer to offset from origin
**           returns 0. else returns -1.
**  [entr] - FILE *stream; the pointer to the FILE stream.
**           long offset; the seek offset.
**           int origin; the seek origin.
**  [exit] - int; 0 if able to seek to offset from origin successfully. else -1.
**  [prec] - stream is a valid FILE pointer and origin is a valid seek origin
**           type.
**  [post] - the stream file pointer position is modified.
**
*/
int fseek(FILE *stream, long offset, int origin)
{
  int ret;

  switch(stream->type) {
    case STD_IOBUF_TYPE_NONE:
    case STD_IOBUF_TYPE_GS:
    case STD_IOBUF_TYPE_SIO:
    case STD_IOBUF_TYPE_STDOUTHOST:
      /* cannot seek stdout or stderr. */
      ret = -1;
      break;
    default:
      /* attempt to seek to offset from origin. */
      ret = ((fioLseek(stream->fd, (int)offset, origin) >= 0) ? 0 : -1);
  }
  return (ret);
}
#endif


#ifdef F_fsetpos
/*
**
**  [func] - fsetpos.
**  [desc] - attempts to set the stream file pointer position to the pos offset.
**           if able to set the stream file pointer position to pos then returns
**           0. else returns -1.
**  [entr] - FILE *stream; the pointer to the FILE stream.
**           const fpos_t *pos; the pointer to the source file position buffer.
**  [exit] - 0 if able to set the stream file pointer position. else -1.
**  [prec] - stream is a valid FILE pointer and pos is a valid fpos_t pointer.
**  [post] - the stream file pointer position is modified.
**
*/
int fsetpos(FILE *stream, const fpos_t *pos)
{
  return (fseek(stream, (long)*pos, SEEK_SET));
}
#endif


#ifdef F_ftell
/*
**
**  [func] - ftell.
**  [desc] - attempts to retrieve the stream file stream pointer position.
**           if able to retrieve the stream file stream pointer position
**           then returns the position. else sets error code and returns -1.
**  [entr] - FILE *stream; the pointer to the FILE stream.
**  [exit] - long; the stream file pointer position if successful. else -1.
**  [prec] - stream is a valid FILE pointer.
**  [post] - none.
**
*/
long ftell(FILE *stream)
{
  long n, ret;

  switch(stream->type) {
    case STD_IOBUF_TYPE_NONE:
    case STD_IOBUF_TYPE_GS:
    case STD_IOBUF_TYPE_SIO:
    case STD_IOBUF_TYPE_STDOUTHOST:
      /* stdout or stderr is an invalid seek stream argument. */
      errno = EINVAL;
      ret = -1L;
      break;
    default:
      if (stream->fd < 0) {
        /* file is not open. */
        errno = EBADF;
        ret = -1L;
      }
      else ret = (((n = fioLseek(stream->fd, 0, SEEK_CUR)) >= 0) ? (long)n : -1L);
  }
  return (ret);
}
#endif


#ifdef F_fwrite
/*
**
**  [func] - fwrite.
**  [desc] - attempts to write n number of records of r size to the stream file
**           and returns the number of records successfully written to the file.
**  [entr] - const void *buf; the pointer to the source data buffer.
**           size_t r; the size of the records to write.
**           size_t n the number of records to write.
**           FILE *stream; the pointer to the FILE stream.
**  [exit] - size_t; the number of records successfully written to the stream file.
**  [prec] - buf is a valid memory pointer of (r * n) size in bytes and stream
**           is a valid FILE pointer.
**  [post] - the stream file is modified.
**
*/
size_t fwrite(const void *buf, size_t r, size_t n, FILE *stream)
{
  size_t i, len, ret;

  switch(stream->type) {
    case STD_IOBUF_TYPE_GS:
      /* write to stdout. */
      for (i = 0, len = (r * n); i < len; ++i) putchar((int)((char *)buf)[i]);
      ret = r;
      break;
    case STD_IOBUF_TYPE_STDOUTHOST:
      ret = (fioWrite(1, (void *) buf, (int)(r * n)) / (int)r);
      break;
    case STD_IOBUF_TYPE_SIO:
      for (i = 0, len = (r * n); i < len; ++i) sio_putc((int)((char *)buf)[i]);
      ret = r;
      break;
    default:
      /* attempt to write the stream file. */
      ret = (fioWrite(stream->fd, (void *)buf, (int)(r * n)) / (int)r);
  }
  return (ret);
}
#endif


#ifdef F_getc
/*
**
**  [func] - getc.
**  [desc] - attempts to read one character from the stream file. if able to
**           read one character from the file then returns the chaaracter
**           read. else EOF.
**  [entr] - FILE *stream; the pointer to the FILE stream.
**  [exit] - int; the character read from the stream file. else -1.
**  [prec] - stream is a valid FILE pointer.
**  [post] - the stream file is modified.
**
*/
int getc(FILE *stream)
{
  char c;
  int  ret;

  switch(stream->type) {
    case STD_IOBUF_TYPE_NONE:
    case STD_IOBUF_TYPE_GS:
    case STD_IOBUF_TYPE_SIO:
    case STD_IOBUF_TYPE_STDOUTHOST:
      /* cannot read from stdout or stderr. */
      ret = EOF;
      break;
    default:
      ret = ((fread(&c, 1, 1, stream) == 1) ? (int)c : EOF);
  }
  return (ret);
}
#endif


#ifdef F_getchar
/*
**
**  [func] - getchar.
**  [desc] - attempts to read one character from the stdin file stream. if able
**           to read one character from the stdin file stream then returns the
**           character read. else returns -1.
**  [entr] - none.
**  [exit] - int; the character read from stdin if successful. else -1.
**  [prec] - none.
**  [post] - the stdin file stream is modified.
**
*/
int getchar(void)
{
  return (getc(stdin));
}
#endif


#ifdef F_getfdtype
/* the present working directory variable. */
#if 0
char __direct_pwd[256];
#endif

/*
**
**  [func] - __stdio_get_fd_type.
**  [desc] - if s or the present working directory begins with a valid file
**           device name then returns the corresponding file descriptor type.
**           else returns -1.
**  [entr] - const char *s; the source string pointer.
**  [exit] - int; the device name file descriptor type if determined. else -1.
**  [prec] - s is a valid string pointer.
**  [post] - none.
**
*/
int __stdio_get_fd_type(const char *s)
{
  int ret;

  if (strncmp(s, "cdrom0:", 7) == 0) ret = STD_IOBUF_TYPE_CDROM;
  else if (strncmp(s, "mc0:", 4) == 0) ret = STD_IOBUF_TYPE_MC;
  else if (strncmp(s, "mc1:", 4) == 0) ret = STD_IOBUF_TYPE_MC;
  else if (strncmp(s, "host0:", 6) == 0) ret = STD_IOBUF_TYPE_HOST;
  else if (strncmp(s, "cdrom0:", 7) == 0) ret = STD_IOBUF_TYPE_CDROM;
#if 0
  else if (strncmp(__direct_pwd, "mc0:", 4) == 0) ret = STD_IOBUF_TYPE_MC;
  else if (strncmp(__direct_pwd, "mc1:", 4) == 0) ret = STD_IOBUF_TYPE_MC;
  else if (strncmp(__direct_pwd, "host0:", 6) == 0) ret = STD_IOBUF_TYPE_HOST;
  else if (strncmp(__direct_pwd, "cdrom0:", 7) == 0) ret = STD_IOBUF_TYPE_CDROM;
#endif
  else ret = -1;
  return (ret);
}
#endif


#ifdef F_gets
/*
**
**  [func] - gets.
**  [desc] - attempts to read a string from stdin. if able to read a string
**           from stdin then stores the string to the memory pointed by buf
**           and returns buf. else returns NULL.
**  [entr] - char *buf; the pointer to the destination string buffer.
**  [exit] - char *; buf if string is read successfully. else NULL.
**  [prec] - buf is a valid memory pointer.
**  [post] - the memory pointed to by buf is modified.
**
*/
char *gets(char *buf)
{
  return (fgets(buf, INT_MAX, stdin));
}
#endif


#ifdef F_strerror
#define E_USE_NAMES
#include "errno.h"
char * strerror(int err) {
    return error_to_string(err);
}
#endif


#ifdef F_perror
/*
**
**  [func] - perror.
**  [desc] - if there is a current error then prints the corresponding error
**           and then prints s to stderr. else prints s to stderr.
**  [entr] - const char *s; the error string pointer.
**  [exit] - none.
**  [prec] - s is a valid string pointer.
**  [post] - none.
**
*/
void perror(const char *s)
{
  char *err;

  /* print to stderr output. */
  if ((err = strerror(errno)) != NULL) fprintf(stderr, "%s : ", err);
  fputs(s, stderr);
}
#endif


#ifdef F_putc
/* std I/O data variable. */
#ifdef USE_GS
extern int __stdio_stdout_xy[2];


/* stdio internal function. */
void __stdio_update_stdout_xy(int, int);
#endif

/*
**
**  [func] - putc.
**  [desc] - attempts to write the c character to the stream file. if able to
**           write the character to the stream file then returns the character
**           written. else returns -1.
**  [entr] - int c; the character to write to the file.
**           FILE *stream; the pointer to the FILE stream.
**  [exit] - int; the character written to the file if successful. else -1.
**  [prec] - stream is a valid FILE pointer.
**  [post] - the stream file is modified.
**
*/
int putc(int c, FILE *stream)
{
  char ch;
  int  ret = 0;

  switch(stream->type) {
    case STD_IOBUF_TYPE_GS:
#ifdef USE_GS
      /* print one character to stdout. */
      switch((char)c) {
        case '\a':
          ret = (int)c;
          break;
        case '\b':
          ret = (int)c;
          break;
        case '\n':
          /* newline. */
        case '\r':
          /* carriage return. */
          __stdio_update_stdout_xy(0, (__stdio_stdout_xy[1] + 1));
          ret = (int)c;
          break;
        case '\t':
          __stdio_update_stdout_xy((__stdio_stdout_xy[0] + 2), __stdio_stdout_xy[1]);
          ret = (int)c;
          break;
        case '\v':
          __stdio_update_stdout_xy(__stdio_stdout_xy[0], (__stdio_stdout_xy[1] + 2));
          ret = (int)c;
          break;
        default:
          /* write one character to the screen. */
          gsCharOut((__stdio_stdout_xy[0] * 16), (__stdio_stdout_xy[1] * 16), c);
          __stdio_update_stdout_xy((__stdio_stdout_xy[0] + 1), __stdio_stdout_xy[1]);
          ret = (int)c;
      }
#endif
      break;
    default:
      /* write one character to the stream file. */
      ch = (char)c;
      ret = ((fwrite(&ch, 1, 1, stream) == 1) ? c : EOF);
  }
  return (ret);
}
#endif


#ifdef F_putchar
/*
**
**  [func] - putchar.
**  [desc] - attempts to write the c character to stdout. if able to write
**           the character to stdout then returns the character written.
**           else returns -1.
**  [entr] - int c; the character to write to stdout.
**  [exit] - int; the character written to stdout. else -1.
**  [prec] - none.
**  [post] - the stdout file stream is modified.
**
*/
int putchar(int c)
{
  return (putc(c, stdout));
}
#endif


#ifdef F_puts
/*
**
**  [func] - puts.
**  [desc] - attempts to write the s string to stdout. if able to write the s
**           string to stdout then returns the number of characters written.
**           else returns -1.
**  [entr] - const char *s; the source string pointer.
**  [exit] - int; the number of characters written to stdout. else -1.
**  [prec] - s is a valid string pointer.
**  [post] - the stdout file stream is modified.
**
*/
int puts(const char *s)
{
  int ret;

  for (ret = 0; (*s != '\0'); ++s) {
    /* attempt to print the current character to stdout. */
    if ((putchar(*s) == (int)*s) && (ret >= 0)) ++ret;
    else ret = EOF;
  }
  if ((putchar('\n') == '\n') && (ret >= 0)) ++ret;
  else ret = EOF;
  return (ret);
}
#endif


#ifdef F_remove
/*
**
**  [func] - remove.
**  [desc] - if the s named file exists then deletes the s named file and
**           returns 0. else returns -1.
**  [entr] - const char *s; the filename string pointer.
**  [exit] - int; 0 if able to delete the s file. else -1.
**  [prec] - s is a valid string pointer.
**  [post] - the s file is deleted.
**
*/
int remove(const char *s)
{
  return (0);
}
#endif


#ifdef F_rename
/*
**
**  [func] - rename.
**  [desc] -
**  [entr] - const char *name; the filename string pointer.
**           const char *newname; the new filename string pointer.
**  [exit] - int;
**  [prec] - name and newname are valid string pointers.
**  [post] - the name filen name is modified.
**
*/
int rename(const char *name, const char *newname)
{
  int ret = 0;
  
  return (ret);
}
#endif


#ifdef F_rewind
/*
**
**  [func] - rewind.
**  [desc] - resets the stream file pointer to 0.
**  [entr] - FILE *stream; the pointer to the FILE stream.
**  [exit] - none.
**  [prec] - stream is a valid FILE pointer.
**  [post] - the stream file pointer is modified.
**
*/
void rewind(FILE *stream)
{
  fseek(stream, 0, SEEK_SET);
}
#endif


#ifdef F_skipatoi
/*
**
**  [func] - __stdio_skip_atoi.
**  [desc] -
**  [entr] - const char **s; the pointer to the source string pointer.
**  [exit] - int;
**  [prec] - s is a valid pointer to string pointer.
**  [post] - the memory pointed to by s is modified.
**
*/
int __stdio_skip_atoi(const char **s)
{
  int ret = 0;

  for (; (isdigit(**s) != 0); ) ret = ((ret * 10) + (*((*s)++) - '0'));
  return (ret);
}
#endif


#ifdef F_sscanf
/*
**
**  [func] - sscanf.
**  [desc] -
**  [entr] - const char *buf;
**           const char *format; the format string pointer.
**           ...;
**  [exit] - int;
**  [prec] - buf and format are valid string pointers.
**  [post] - the memory pointed to by format string arguments are
**
*/
int sscanf(const char *buf, const char *format, ...)
{
  int     ret;
  va_list va;

  va_start(va, format);
  ret = vsscanf(buf, format, va);
  va_end(va);
  return (ret);
}
#endif


#ifdef F_stdio
/* stdio data variables. */
FILE __iob[_NFILE] = {
  { -1,                 0, 0, 0 },     // stdin
#ifdef USE_GS
  { STD_IOBUF_TYPE_GS,  0, 0, 0 },     // stdout
#else
  { STD_IOBUF_TYPE_STDOUTHOST, 0, 0, 0 }, // stdout
#endif
#ifdef USE_SIO
  { STD_IOBUF_TYPE_SIO, 0, 0, 0 },     // stderr
#else
  { STD_IOBUF_TYPE_STDOUTHOST, 0, 0, 0 }, // stdout
#endif
  { 0, -1, 0, 0 },
  { 0, -1, 0, 0 },
  { 0, -1, 0, 0 },
  { 0, -1, 0, 0 },
  { 0, -1, 0, 0 },
  { 0, -1, 0, 0 },
  { 0, -1, 0, 0 },
  { 0, -1, 0, 0 },
  { 0, -1, 0, 0 },
  { 0, -1, 0, 0 },
  { 0, -1, 0, 0 },
  { 0, -1, 0, 0 },
  { 0, -1, 0, 0 }
};
char __stdio_tmpnam[256];
#ifdef USE_GS
int  __stdio_stdout_xy[2];
#endif
#endif


#ifdef F_tmpfile
/* stdio temp name variable. */
extern char __stdio_tmpnam[256];


/*
**
**  [func] - tmpfile.
**  [desc] - attempts to create a temporary file. if able to create a temporary
**           file then returns the pointer to the FILE stream. else returns NULL.
**  [entr] - none.
**  [exit] - FILE *; the ptr. to the opened temp. file if successful. else NULL.
**  [prec] - none.
**  [post] - a temporary is opened.
**
*/
FILE *tmpfile(void)
{
  return ((tmpnam(NULL) != NULL) ?  fopen(__stdio_tmpnam, "rw+") : NULL);
}
#endif


#ifdef F_tmpnam
/* stdio temp name variable. */
extern char __stdio_tmpnam[256];


/*
**
**  [func] - tmpnam.
**  [desc] - creates a temporary filename string, 
**  [entr] - char *name; the pointer to the destination string pointer.
**  [exit] - char *;
**  [prec] -
**  [post] -
**
*/
char *tmpnam(char *name)
{
  char *ret = NULL;

  return (ret);
}
#endif


#ifdef F_ungetc
/*
**
**  [func] - ungetc.
**  [desc] -
**  [entr] - int c;
**           FILE *stream; the pointer to the FILE stream.
**  [exit] - int;
**  [prec] - stream is a valid FILE pointer.
**  [post] - the stream FILE stream is modified.
**
*/
int ungetc(int c, FILE *stream)
{
  // int ret = EOF;

  if (c == EOF || stream->has_putback) {
    /* invalid input, or putback queue full */
    return EOF;
  }

  stream->putback = (u8)c;
  stream->has_putback = 1;
  return c;
}
#endif


#ifdef F_updatestdoutxy
/* std I/O data variable. */
#ifdef USE_GS
extern int __stdio_stdout_xy[2];
#endif

/*
**
**  [func] - __stdio_update_stdout_xy.
**  [desc] - updates the stdout (x, y) screen coordinates.
**  [entr] - int x; the x screen coordinate.
**           int y; the y screen coordinate.
**  [exit] - none.
**  [prec] - none.
**  [post] - the stdout screen coordinates are modified.
**
*/
void __stdio_update_stdout_xy(int x, int y)
{
#ifdef USE_GS
  if ((x * 16) >= gsGetDisplayWidth()) {
    x = 0;
    ++y;
  }
  if ((y * 16) >= gsGetDisplayHeight()) y = 0;
  __stdio_stdout_xy[0] = x;
  __stdio_stdout_xy[1] = y;
#endif
}
#endif


#ifdef F___stdio_internals
void _ps2sdk_stdio_init()
{
}

void _ps2sdk_stdio_deinit()
{
}
#endif

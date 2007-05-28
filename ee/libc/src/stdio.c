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
	   
#ifdef F_strerror
#define E_USE_NAMES
#include "errno.h"
#endif

#include <stdio.h>
#include <tamtypes.h>
#include <kernel.h>
#include <sio.h>
#include <fileio.h>
#include <string.h>
#include <limits.h>

extern int (*_ps2sdk_close)(int);
extern int (*_ps2sdk_open)(const char*, int);
extern int (*_ps2sdk_read)(int, void*, int);
extern int (*_ps2sdk_lseek)(int, int, int);
extern int (*_ps2sdk_write)(int, const void*, int);
extern int (*_ps2sdk_remove)(const char*);
extern int (*_ps2sdk_rename)(const char*, const char*);
extern int (*_ps2sdk_mkdir)(const char*, int);
extern int (*_ps2sdk_rmdir)(const char*);

void _ps2sdk_stdio_init();

/* std I/O buffer type constants. */
#define STD_IOBUF_TYPE_NONE            0
#define STD_IOBUF_TYPE_GS              1
#define STD_IOBUF_TYPE_SIO             2
#define STD_IOBUF_TYPE_CDROM           4
#define STD_IOBUF_TYPE_MC              8
#define STD_IOBUF_TYPE_HOST           16
#define STD_IOBUF_TYPE_STDOUTHOST     32
#define STD_IOBUF_TYPE_MASS           64
#define STD_IOBUF_TYPE_PFS           128

extern char __direct_pwd[256];
extern int __stdio_initialised;

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
      if ((stream->fd >= 0) && (_ps2sdk_close(stream->fd) >= 0)) {
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
**  [func] - _fcloseall.
**  [desc] - attempts to close all the open files. if able to close all the open
**           files then returns the number of files closed. else returns -1.
**  [entr] - none.
**  [exit] - int; the number of files closed if successful. else -1.
**  [prec] - none.
**  [post] - all open non-system files are closed.
**
*/
int _fcloseall(void)
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
__attribute__((weak))
int mcFlush(int fd)
{
	return 0;
}

__attribute__((weak))
int mcSync(int mode, int *cmd, int *result)
{
	return 0;
}

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
**  [func] - _fflushall.
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
  unsigned char c;
  int ret;

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

  n = (ftell(stream) - (stream->has_putback ? 1 : 0));
  if (n >= 0) *pos = (fpos_t)n;
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
            if (n > 1) {
              /* newline terminates fgets. */
              *buf++ = (char)c;
              *buf++ = '\0';
              n -= 2;
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


 /* Normalize a pathname by removing
  . and .. components, duplicated /, etc. */
char* __ps2_normalize_path(char *path_name)
{
        int i, j;
        int first, next;
        static char out[255];

        /* First copy the path into our temp buffer */
        strcpy(out, path_name);
        /* Then append "/" to make the rest easier */
        strcat(out,"/");

        /* Convert "//" to "/" */
        for(i=0; out[i+1]; i++) {
                if(out[i]=='/' && out[i+1]=='/') {
                        for(j=i+1; out[j]; j++)
                                out[j] = out[j+1];
                        i--;
                ;}
        }

        /* Convert "/./" to "/" */
        for(i=0; out[i] && out[i+1] && out[i+2]; i++) {
                if(out[i]=='/' && out[i+1]=='.' && out[i+2]=='/') {
                        for(j=i+1; out[j]; j++)
                                out[j] = out[j+2];
                        i--;
                }
        }

        /* Convert "/path/../" to "/" until we can't anymore.  Also
         * convert leading "/../" to "/" */
        first = next = 0;
        while(1) {
                /* If a "../" follows, remove it and the parent */
                if(out[next+1] && out[next+1]=='.' && 
                   out[next+2] && out[next+2]=='.' &&
                   out[next+3] && out[next+3]=='/') {
                        for(j=0; out[first+j+1]; j++)
                                out[first+j+1] = out[next+j+4];
                        first = next = 0;
                        continue;
                }

                /* Find next slash */
                first = next;
                for(next=first+1; out[next] && out[next] != '/'; next++)
                        continue;
                if(!out[next]) break;
        }

        /* Remove trailing "/" */
        for(i=1; out[i]; i++)
                continue;
        if(i >= 1 && out[i-1] == '/') 
                out[i-1] = 0;

        return (char*)out;
}

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
  
  // some people won't use our crt0...
  if (!__stdio_initialised)
    _ps2sdk_stdio_init();

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
          iomode = (O_WRONLY | O_CREAT | O_TRUNC);
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
            iomode |= (O_RDWR);
            continue;
          default:
            break;
        }
      }
      /* search for an available fd slot. */
      for (i = 3; i < _NFILE; ++i) if (__iob[i].fd < 0) break;
      if (i < _NFILE) {
        char * t_fname = __ps2_normalize_path((char *)fname);
        char b_fname[FILENAME_MAX];
        __iob[i].type = __stdio_get_fd_type(fname);
        if (!strchr(fname, ':')) { // filename doesn't contain device
          t_fname = b_fname;
          if (fname[0] == '/' || fname[0] == '\\' ) {   // does it contain root ?
            char * device_end = strchr(__direct_pwd, ':');
            if (device_end) {      // yes, let's strip pwd a bit to keep device only
              strncpy(b_fname, __direct_pwd, device_end - __direct_pwd);
              strcpy(b_fname + (device_end - __direct_pwd), fname);
            } else {               // but pwd doesn't contain any device, let's default to host
              strcpy(b_fname, "host:");
              strcpy(b_fname + 5, fname);
            }
          } else {                 // otherwise, it's relative directory, let's copy pwd straight
            int b_fname_len = strlen(__direct_pwd);
            if (!strchr(__direct_pwd, ':')) { // check if pwd contains device name
              strcpy(b_fname, "host:");
              strcpy(b_fname + 5, __direct_pwd);
              if (!(__direct_pwd[b_fname_len - 1] == '/' || __direct_pwd[b_fname_len - 1] == '\\')) { // does it has trailing slash ?
                if(__iob[i].type == STD_IOBUF_TYPE_CDROM)
              	 b_fname[b_fname_len + 5] = '\\';
            else 
             b_fname[b_fname_len + 5] = '/';
                // b_fname[b_fname_len + 6] = 0; 
                    // b_fname_len += 2;
                    b_fname_len++;
              }
              b_fname_len += 5;
              strcpy(b_fname + b_fname_len, fname);
            } else {                          // device name is here
              if (b_fname_len) {
                strcpy(b_fname, __direct_pwd);
                if (!(b_fname[b_fname_len - 1] == '/' || b_fname[b_fname_len - 1] == '\\')) {
                  if(__iob[i].type == STD_IOBUF_TYPE_CDROM)
                  	b_fname[b_fname_len] = '\\';
                  else
                    b_fname[b_fname_len] = '/';
                  // b_fname[b_fname_len + 1] = 0; // #neofar
                  // b_fname_len += 2;
                  b_fname_len++;
                }
                strcpy(b_fname + b_fname_len, fname);
              }
            }
          }
        }
        if ((fd = _ps2sdk_open((char *)t_fname, iomode)) >= 0) {
          __iob[i].fd = fd;
          __iob[i].cnt = 0;
          __iob[i].flag = flag;
          __iob[i].has_putback = 0;
          ret = (__iob + i);
        } else if ((__iob[i].type == STD_IOBUF_TYPE_CDROM)) {
          int fname_len = strlen(t_fname);
          if (!((t_fname[fname_len - 2] == ';') && (t_fname[fname_len - 1] == '1'))) {
            char cd_fname[fname_len + 3];
            strcpy(cd_fname, t_fname);
            cd_fname[fname_len + 0] = ';';
            cd_fname[fname_len + 1] = '1';
            cd_fname[fname_len + 2] = 0;
            if ((fd = _ps2sdk_open((char *)cd_fname, iomode)) >= 0) {
              __iob[i].fd = fd;
              __iob[i].cnt = 0;
              __iob[i].flag = flag;
              __iob[i].has_putback = 0;
              ret = (__iob + i);
            }
          }
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

#ifdef F_fileno
int fileno(FILE * f) {
    return f->fd;
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
  unsigned char ch = (unsigned char)c;
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

  int temp = strlen(s);

  len = ((fwrite(s, 1, temp, stream) == temp) ? temp : EOF);
  
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
  size_t ret = 0, read_len = r * n;

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
      if (stream->has_putback) {
        unsigned char *ptr = (unsigned char *)buf;
        *ptr = stream->putback;
        buf = ptr + 1;
        stream->has_putback = 0;
        ret++;
        /* subtract 1 to read_len to avoid buffer overflow */
        read_len--;
      }
      ret += _ps2sdk_read(stream->fd, buf, read_len) / r;
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
  
  stream->has_putback = 0;

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
      ret = _ps2sdk_lseek(stream->fd, (int)offset, origin);
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
  stream->has_putback = 0;
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
      else {
        ret = (((n = _ps2sdk_lseek(stream->fd, 0, SEEK_CUR)) >= 0) ? (long)n : -1L);
        if ((n >= 0) && stream->has_putback) ret--;
      }
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
  
  if (stream->has_putback) 
  {
      fseek(stream, -1, SEEK_CUR);
      stream->has_putback = 0;
  }

  switch(stream->type) {
    case STD_IOBUF_TYPE_GS:
      /* write to stdout. */
      for (i = 0, len = (r * n); i < len; ++i) putchar((int)((char *)buf)[i]);
      ret = r;
      break;
    case STD_IOBUF_TYPE_STDOUTHOST:
      ret = (_ps2sdk_write(1, (void *) buf, (int)(r * n)) / (int)r);
      break;
    case STD_IOBUF_TYPE_SIO:
      for (i = 0, len = (r * n); i < len; ++i) sio_putc((int)((char *)buf)[i]);
      ret = r;
      break;
    default:
      /* attempt to write the stream file. */
      ret = (_ps2sdk_write(stream->fd, (void *)buf, (int)(r * n)) / (int)r);
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
  unsigned char c;
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
char __direct_pwd[256] = "";

static struct {
  char * prefix;
  int len;
  int ret;
} __prefix_types[] = {
    { "cdrom0:", 7, STD_IOBUF_TYPE_CDROM },
    { "cdrom:",  6, STD_IOBUF_TYPE_CDROM },
    { "mc0:",    4, STD_IOBUF_TYPE_MC },
    { "mc1:",    4, STD_IOBUF_TYPE_MC },
    { "host0:",  6, STD_IOBUF_TYPE_HOST },
    { "host:",   5, STD_IOBUF_TYPE_HOST },
    { "mass0:",  6, STD_IOBUF_TYPE_MASS },
    { "mass:",   5, STD_IOBUF_TYPE_MASS },
    { "pfs0:",   5, STD_IOBUF_TYPE_PFS },
    { "pfs1:",   5, STD_IOBUF_TYPE_PFS },
    { "pfs2:",   5, STD_IOBUF_TYPE_PFS },
    { "pfs3:",   5, STD_IOBUF_TYPE_PFS },
    { 0, 0 }
};

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
  int i;
  
  for (i = 0; __prefix_types[i].prefix; i++) {
    if (!strncmp(s, __prefix_types[i].prefix, __prefix_types[i].len))
      return __prefix_types[i].ret;
  }

  for (i = 0; __prefix_types[i].prefix; i++) {
    if (!strncmp(__direct_pwd, __prefix_types[i].prefix, __prefix_types[i].len))
      return __prefix_types[i].ret;
  }
  
  return -1;
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
  char *str = NULL;
  char c = 0;

  if ((str = fgets(buf, INT_MAX, stdin)) != NULL) {
    /* remove the trailing new line (if it exists) */
    c = str[strlen(str) - 1];
    if (c == '\n' || c == '\r')
      str[strlen(str) - 1] = '\0';
  }
  return (str);
}
#endif


#ifdef F_strerror
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
  return _ps2sdk_remove(s);
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
  int ret = -1;
  if (_ps2sdk_rename)
    ret = _ps2sdk_rename(name, newname);
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


#ifdef F_stdio
/* stdio data variables. */
int __stdio_initialised = 0;

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
static int fioMkdirHelper(const char* source, int mode)
{
    /* mode not used */
	return fioMkdir(source);
}

int (*_ps2sdk_close)(int) = fioClose;
int (*_ps2sdk_open)(const char*, int) = fioOpen;
int (*_ps2sdk_read)(int, void*, int) = fioRead;
int (*_ps2sdk_lseek)(int, int, int) = fioLseek;
int (*_ps2sdk_write)(int, const void*, int) = fioWrite;
int (*_ps2sdk_remove)(const char*) = fioRemove;
int (*_ps2sdk_rename)(const char*, const char*) = 0;
int (*_ps2sdk_mkdir)(const char*, int) = fioMkdirHelper;
int (*_ps2sdk_rmdir)(const char*) = fioRmdir;

void _ps2sdk_stdio_init()
{
    int i;
    
    for (i = 3; i < _NFILE; i++) {
	__iob[i].type = 0;
	__iob[i].fd = -1;
	__iob[i].cnt = 0;
	__iob[i].flag = 0;
	__iob[i].has_putback = 0;
	__iob[i].putback = 0;
    }
    
    __stdio_initialised = 1;
}

void _ps2sdk_stdio_deinit()
{
//    _fflushall();  will require libmc...
    _fcloseall();
    
    __stdio_initialised = 0;
}
#endif

#ifdef F_chdir
int chdir(const char *path) {
    strcpy(__direct_pwd, path);
    return 0;
}
#endif

#ifdef F_mkdir
int mkdir(const char *path, int mode)
{
	return _ps2sdk_mkdir(path, mode);
}
#endif

#ifdef F_rmdir
int rmdir(const char *path)
{
	return _ps2sdk_rmdir(path);
}
#endif

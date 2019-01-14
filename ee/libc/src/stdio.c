/*
# _____     ___ ____     ___ ____
#  ____|   |    ____|   |        | |____|
# |     ___|   |____ ___|    ____| |    \    PS2DEV Open Source Project.
#-----------------------------------------------------------------------
# Copyright 2001-2004, ps2dev - http://www.ps2dev.org
# Licenced under Academic Free License version 2.0
# Review ps2sdk README & LICENSE files for further details.
*/

/**
 * @file
 * Simple standard C library implementation.
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
void clearerr(FILE *stream)
{
  stream->flag &= ~(_IOERR|_IOEOF);
}
#endif


#ifdef F_fclose
int fclose(FILE *stream)
{
  int ret = EOF;

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
      if ((stream->fd >= 0) && (ret=_ps2sdk_close(stream->fd) >= 0)) {
        stream->type = STD_IOBUF_TYPE_NONE;
        stream->fd = -1;
        stream->cnt = 0;
        stream->flag = 0;
        ret = 0;
      }
      else {
        errno = (ret * -1);
        ret = EOF;
      }
  }
  return (ret);
}
#endif


#ifdef F_fcloseall
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
int feof(FILE *stream)
{
  return ((stream->flag & _IOEOF) != 0);
}
#endif


#ifdef F_ferror
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
        if (ret != 0) {
          errno = (ret * -1);
		  stream->flag |= _IOERR;
          ret = EOF;
        }
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
      errno = EBADFD;
      ret = EOF;
  }
  return (ret);
}
#endif


#ifdef F_fflushall
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
int fgetc(FILE *stream)
{
  unsigned char c;
  int ret;

  switch(stream->type) {
    case STD_IOBUF_TYPE_GS:
    case STD_IOBUF_TYPE_SIO:
    case STD_IOBUF_TYPE_STDOUTHOST:
      /* cannot read from stdout or stderr. */
      errno = EINVAL;
	  stream->flag |= _IOERR;
      ret = EOF;
      break;
    default:
	  if(fread(&c, 1, 1, stream) == 1){
		  ret = (int)c;
	  }
	  else{
		  stream->flag |= _IOEOF;
		  ret = EOF;
	  }
  }
  return (ret);
}
#endif


#ifdef F_fgetpos
int fgetpos(FILE *stream, fpos_t *pos)
{
  long n;

  n = (ftell(stream) - (stream->has_putback ? 1 : 0));
  if (n >= 0) *pos = (fpos_t)n;
  return ((n >= 0) ? 0 : -1);
}
#endif


#ifdef F_fgets
char *fgets(char *buf, int n, FILE *stream)
{
  char *ret = buf;
  int  c, done;

  switch(stream->type) {
    case STD_IOBUF_TYPE_GS:
    case STD_IOBUF_TYPE_SIO:
    case STD_IOBUF_TYPE_STDOUTHOST:
      /* cannot read from stdout or stderr. */
      errno = EINVAL;
      ret = NULL;
      break;
    default:
      for (done = 0; (!done) && (n >= 2); ) {	//Read until either a newline character is encountered or n-1 characters are read.
        switch(c = fgetc(stream)) {
          case '\r':	//Newline character translation: do nothing
		break;
          case '\n':
            /* newline terminates fgets. */
            *buf++ = (char)c;
            n --;
            done = 1;
            break;
          case EOF:
            /* end of file or error. */
            if(buf == ret)
              ret = NULL;	//If no characters could be read, then return NULL.
            done = 1;
            break;
          default:
            /* store the current character to buf. */
            *buf++ = (char)c;
            --n;
        }
      }

      if(ret != NULL && n >= 1)
        *buf = '\0';	//Only if there was at least one character read and if there is enough space.
  }
  return (ret);
}
#endif


#ifdef F_fopen
/** std I/O internal function. */
int __stdio_get_fd_type(const char *);


/** Normalize a pathname by removing . and .. components, duplicated /, etc. */
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

FILE *fopen(const char *fname, const char *mode)
{
  FILE *ret = NULL;
  int  fd = 0, flag = 0, i, iomode = 0;

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
          iomode = O_APPEND|O_WRONLY;
          break;
      } // switch
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
        } // switch
      } // for
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
                if(__iob[i].type == STD_IOBUF_TYPE_CDROM) {
              	  b_fname[b_fname_len + 5] = '\\';
                  b_fname_len++;
                } else if(__iob[i].type == STD_IOBUF_TYPE_HOST) {
                  //Do nothing for the host device, as adding a leading slash (where there is none) after the device name will change the origin of the path.
                } else {
                  b_fname[b_fname_len + 5] = '/';
                  // b_fname[b_fname_len + 6] = 0;
                  // b_fname_len += 2;
                  b_fname_len++;
                }
              }
              b_fname_len += 5;
              strcpy(b_fname + b_fname_len, fname);
            } else {                          // device name is here
              if (b_fname_len) {
                strcpy(b_fname, __direct_pwd);
                if (!(b_fname[b_fname_len - 1] == '/' || b_fname[b_fname_len - 1] == '\\')) {
                  if(__iob[i].type == STD_IOBUF_TYPE_CDROM) {
                    b_fname[b_fname_len] = '\\';
                    b_fname_len++;
                  } else if(__iob[i].type == STD_IOBUF_TYPE_HOST) {
                    //Do nothing for the host device, as adding a leading slash (where there is none) after the device name will change the origin of the path.
                  } else {
                    b_fname[b_fname_len] = '/';
                    // b_fname[b_fname_len + 1] = 0; // #neofar
                    // b_fname_len += 2;
                    b_fname_len++;
                  }
                }
                strcpy(b_fname + b_fname_len, fname);
              }
            }
          }
        }
        if ((fd = _ps2sdk_open((char *)t_fname, iomode, 0666)) >= 0) {
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
            if ((fd = _ps2sdk_open((char *)cd_fname, iomode, 0666)) >= 0) {
              __iob[i].fd = fd;
              __iob[i].cnt = 0;
              __iob[i].flag = flag;
              __iob[i].has_putback = 0;
              ret = (__iob + i);
            }
          }
        }
      } else {
        errno = ENFILE;
      }
    } else {
      errno = EINVAL;
    }
  } else {
    errno = EINVAL;
  }
  if (fd < 0) {
	errno = (fd * -1);
  }
  return (ret);
}
#endif

#ifdef F_fdopen
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
      } else {
        errno = ENFILE;
      }
    } else {
      errno = EINVAL;
    }
  } else {
    errno = EBADF;
  }

  return (ret);
}
#endif

#ifdef F_fileno
int fileno(FILE * f) {
    if (f->fd > 0)
      return f->fd;
    else
      errno = EBADF;
      return -1;
}
#endif

#ifdef F_fputc
int fputc(int c, FILE *stream)
{
  unsigned char ch = (unsigned char)c;
  return ((fwrite(&ch, 1, 1, stream) == 1) ? 0 : EOF);
}
#endif


#ifdef F_fputs
int fputs(const char *s, FILE *stream)
{
  size_t len;

  int temp = strlen(s);

  len = ((fwrite(s, 1, temp, stream) == temp) ? temp : EOF);

  return len;
}
#endif


#ifdef F_fread
size_t fread(void *buf, size_t r, size_t n, FILE *stream)
{
  size_t ret = 0, read_len = r * n;
  int read;

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
      read = _ps2sdk_read(stream->fd, buf, read_len);
      if (read < 0)
        read = errno = -read;
      else if(read<read_len)
        stream->flag |= _IOEOF;

      ret +=  read / r;
  }
  return (ret);
}
#endif


#ifdef F_fseek
int fseek(FILE *stream, long offset, int origin)
{
  int ret;

  stream->has_putback = 0;
  stream->flag &= ~_IOEOF;

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
  if (ret >= 0)
    return 0;
  else {
    errno = (ret * -1);
    return -1;
  }
}
#endif


#ifdef F_fsetpos
int fsetpos(FILE *stream, const fpos_t *pos)
{
  stream->has_putback = 0;
  return (fseek(stream, (long)*pos, SEEK_SET));
}
#endif


#ifdef F_ftell
long ftell(FILE *stream)
{
  long n, ret = -1L;

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
        if ((n = _ps2sdk_lseek(stream->fd, 0, SEEK_CUR)) >= 0)
             ret = (long)n;
        else if (n < 0) {
              errno = (n * -1);
              ret = -1L;
        }
        if ((n >= 0) && stream->has_putback) ret--;
      }
  }
  return (ret);
}
#endif


#ifdef F_fwrite
size_t fwrite(const void *buf, size_t r, size_t n, FILE *stream)
{
  size_t i, len, ret;
  int written = 0;

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
      written = (_ps2sdk_write(1, (void *) buf, (int)(r * n)) / (int)r);
      break;
    case STD_IOBUF_TYPE_SIO:
      for (i = 0, len = (r * n); i < len; ++i) sio_putc((int)((char *)buf)[i]);
      ret = r;
      break;
    default:
      /* attempt to write the stream file. */
      written = (_ps2sdk_write(stream->fd, (void *)buf, (int)(r * n)) / (int)r);
  }
  if (written < 0) {
    /* _ps2sdk_write returns negative errno on error */
    errno = (written * -1);
    ret = 0;
  }
  else {
    ret = written;
  }
  return (ret);
}
#endif


#ifdef F_getc
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
int getchar(void)
{
  return (getc(stdin));
}
#endif


#ifdef F_getfdtype
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
 * Determines if s or the present working directory begins with a valid file device name
 *  @param s the source string pointer which is valid.
 *  @return the device name file descriptor type if determined. else -1.
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


/** stdio internal function. */
void __stdio_update_stdout_xy(int, int);
#endif

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
int putchar(int c)
{
  return (putc(c, stdout));
}
#endif


#ifdef F_puts
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
int remove(const char *s)
{
  int ret = _ps2sdk_remove(s);

  return (ret * -1);
}
#endif


#ifdef F_rename
int rename(const char *oldfn, const char *newfn)
{
  int ret = _ps2sdk_rename(oldfn, newfn);
  if (ret < 0) {
    errno = (ret * -1);
    return -1;
  }
  else
    return 0;
}
#endif


#ifdef F_rewind
void rewind(FILE *stream)
{
  fseek(stream, 0, SEEK_SET);
}
#endif


#ifdef F_skipatoi
/**
 * @param s the pointer to the source string pointer.
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

/* the present working directory variable. */
char __direct_pwd[256] = "";

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

FILE *tmpfile(void)
{
  return ((tmpnam(NULL) != NULL) ?  fopen(__stdio_tmpnam, "rw+") : NULL);
}
#endif


#ifdef F_tmpnam
/* stdio temp name variable. */
extern char __stdio_tmpnam[256];

char *tmpnam(char *name)
{
  char *ret = NULL;

  return (ret);
}
#endif


#ifdef F_ungetc
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

/* Updates the stdout (x, y) screen coordinates.
 * @param x the x screen coordinate.
 * @param y the y screen coordinate.
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


#ifdef F___stdio_helper_internals
static int fioOpenHelper(const char* source, int flags, ...);

int (*_ps2sdk_close)(int) = fioClose;
int (*_ps2sdk_open)(const char*, int, ...) = fioOpenHelper;
int (*_ps2sdk_read)(int, void*, int) = fioRead;
int (*_ps2sdk_lseek)(int, int, int) = fioLseek;
int (*_ps2sdk_write)(int, const void*, int) = fioWrite;
int (*_ps2sdk_remove)(const char*) = fioRemove;

static int fioOpenHelper(const char* pathname, int flags, ...)
{
	int mode;
	va_list alist;

	va_start(alist, flags);
	mode = va_arg(alist, int);	//Retrieve the mode argument, regardless of whether it is expected or not.
	va_end(alist);

	return fioOpen(pathname, flags);
}

int fioRename(const char *old, const char *new)
{
  return -ENOSYS;
}

int (*_ps2sdk_rename)(const char*, const char*) = fioRename;
#endif

#ifdef F___stdio_internals
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
    __stdio_initialised = 0;
}
#endif

#ifdef F_chdir
int chdir(const char *path) {
    strcpy(__direct_pwd, path);
    return 0;
}
#endif

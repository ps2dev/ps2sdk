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
# Stdlib's functions, without allocs (see alloc.c)
*/

#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <kernel.h>
#include <string.h>
#include <limits.h>

extern void (* __stdlib_exit_func[32])(void);
extern int     __stdlib_exit_index;

// This function is missing...
char *__stdlib_ecvt(double, size_t, int *, int *);

#ifndef __ENVIRONVARIABLE_T_DEFINED
#define __ENVIRONVARIABLE_T_DEFINED
typedef struct {
   char name[256];
   char value[256];
} environvariable_t;
#endif // __ENVIRONVARIABLE_T_DEFINED

extern environvariable_t __stdlib_env[32];
extern int __stdlib_mb_shift;
extern unsigned int __stdlib_rand_seed;


#ifdef F_abs
/*
**
**  [func] - abs.
**  [desc] - returns the absolute value of the integer c.
**  [entr] - int c; the integer value.
**  [exit] - int; the absolute value of the integer c.
**  [prec] - none.
**  [post] - none.
**
*/

// shouldn't we rather put that as a macro... ?
int abs(int c)
{
  return ((c >= 0) ? c : -c);
}
#endif


#ifdef F_atexit
/*
**
**  [func] - atexit.
**  [desc] - if the current amount of registered exit() functions has not
**           been reached then registers the func parameter function to the
**           list and returns 0. else returns non-zero.
**  [entr] - void (*func)(void); the pointer to the exit function.
**  [exit] - int; 0 if albe to register the func exit() function. else non-zero.
**  [prec] - func is a valid function pointer.
**  [post] - the atexit() function list is modified.
**
*/
int atexit(void (*func)(void))
{
  int ret;

  if (__stdlib_exit_index < 32) {
    /* register func to the exit() function list. */
    __stdlib_exit_func[__stdlib_exit_index++] = func;
    ret = 0;
  }
  else ret = -1;
  return (ret);
}
#endif


#ifdef F_atof
/*
**
**  [func] - atof.
**  [desc] - if the string s begins with a valid floating point string then
**           returns the floating point value of the string s. else returns 0.**  
**  [entr] - const char *s; the source string pointer.
**  [exit] - double; the floating point value of the string s. else 0.
**  [prec] - s is a valid string pointer.
**  [post] - none.
*
*/

// macro... maybe ? :)
double atof(const char *s)
{
  return (strtod(s, NULL));
}
#endif


#ifdef F__atoi
/*
**
**  [func] - _atoi.
**  [desc] - if the string s begins with a valid integer then returns the
**           integer value of the string s. else returns 0.
**  [entr] - const char *s; the source string pointer.
**  [exit] - int; the integer value of the string s. else 0.
**  [prec] - s is a valid string pointer.
**  [post] - none.
**
*/
int _atoi(const char *s)
{
  int neg = 0, ret = 0;

  for (;;++s) {
    switch(*s) {
      case ' ':
      case '\t':
        continue;
      case '-':
        ++neg;
      case '+':
        ++s;
        break;
    }
  }
  /* calculate the integer value. */
  for (; ((*s >= '0') && (*s <= '9')); ) ret = ((ret * 10) + (int)(*s++ - '0'));
  return ((neg == 0) ? ret : -ret);
}
#endif


#ifdef F_atol
/*
**
**  [func] - atol.
**  [desc] - if the string s begins with a valid long integer then returns the
**           long integer value of the string s. else returns 0.
**  [entr] - const char *s; the source string pointer.
**  [exit] - int; the integer value of the string s. else 0.
**  [prec] - s is a valid string pointer.
**  [post] - none.
**
*/
long atol(const char *s)
{
  int  neg = 0;
  long ret = 0;

  for (;;++s) {
    switch(*s) {
      case ' ':
      case '\t':
        continue;
      case '-':
        ++neg;
      case '+':
        ++s;
        break;
    }
  }
  /* calculate the integer value. */
  for (; ((*s >= '0') && (*s <= '9')); ) ret = ((ret * 10) + (long)(*s++ - '0'));
  return ((neg == 0) ? ret : -ret);
}
#endif


#ifdef F_bsearch
/*
**
**  [func] - bsearch.
**  [desc] -
**  [entr] - const void *key; the pointer to the search key object.
**           const void *base; the pointer to the base of the search data.
**           size_t count; the number of elements in the search data.
**           size_t size; the size of the search elements.
**           int (* compare)(const void *, const void *); the pointer to the compare function.
**  [exit] - void *;
**  [prec] -
**  [post] -
**
*/
void *bsearch(const void *key, const void *base, size_t count, size_t size, int (* compare)(const void *, const void *))
{
  int        comparison;
  size_t     l, u, idx;
  void       *ret = NULL;
  const void *p;

  /* perform a binary search of a sorted array. */
  for (l = 0, u = count; l < u; ) {
    idx = ((l + u) / 2);
    /* calculate the pointer index. */
    p = (const void *)((const char *)base + (idx * size));
    comparison = (*compare)(key, p);
    if (comparison < 0) u = idx;
    else if (comparison > 0) l = (idx + 1);
    else {
      /* return the pointer. */
      ret = (void *)p;
      break;
    }
  }
  return (ret);
}
#endif


#ifdef F_div
/*
**
**  [func] - div.
**  [desc] -
**  [entr] - int n; the integer numerator.
**           int d; the integer divisor.
**  [exit] - div_t;
**  [prec] - none.
**  [post] - none.
**
*/
div_t div(int n, int d)
{
  div_t ret;

  /* calculate the quotient and remainder. */
  // duh... can't this be written with some asm "mfhi/mflo" ?
  ret.quot = (n / d);
  ret.quot = (n % d);
  return (ret);
}
#endif


#if 0
/*
**
**  [func] - exit.
**  [desc] - calls all the register exit() functions and returns to PlayStation2
**           OSD.
**  [entr] - int status; the exit status code.
**  [exit] - this function deos not return.
**  [prec] - none.
**  [post] - none.
**
*/
void exit(int status)
{
  int i;

  for (i = (__stdlib_exit_index - 1); i <= 0; --i) (__stdlib_exit_func[i])();
  // wrong... have to do _exit rather... see abort.c
  // but we should also provide a __process_atexit so the crt0 could call
  // it when main() returns.
  Exit(status);
}
#endif


#if 0
/*
**
**  [func] - _gcvt.c
**  [desc] -
**  [entr] - double x;
**           size_t n;
**           char *buf;
**  [exit] - char *;
**  [prec] -
**  [post] -
**
*/

// why the underscore ? win32 or what ?
// won't link anyway.
char *_gcvt(double x, size_t n, char *buf)
{
  int  decpt, i, sign;
  char *p1, *p2;

  p1 = __stdlib_ecvt(x, n, &decpt, &sign);
  p2 = buf;
  if (sign) *p2++ = '-';
  for (i = (n - 1); ((i > 0) && (p1[i] == '0')); --i) --n;
  i = (int)n;
  if (((decpt >= 0) && (decpt - i > 4)) || ((decpt < 0) && (decpt < -3))) {
    --decpt;
    *p2++ = *p1++;
    *p2++ = '.';
    for (i = 1; i < n; ++i) *p2++ = *p1++;
    *p2++ = 'e';
    if (decpt < 0) {
      decpt = -decpt;
      *p2++ = '-';
    }
    else *p2++ = '+';
    if ((decpt / 100) > 0) *p2++ = ((decpt / 100) + '0');
    if ((decpt / 10) > 0) *p2++ = (((decpt % 100) / 10) + '0');
    *p2++ = decpt%10 + '0';
  }
  else {
    if (decpt <= 0) {
      if (*p1!='0') *p2++ = '.';
      while (decpt < 0) {
        ++decpt;
        *p2++ = '0';
      }
    }
    for (i = 1; i <= n; ++i) {
      *p2++ = *p1++;
      if (i == decpt) *p2++ = '.';
    }
    if (n < decpt) {
      while (n++ < decpt) *p2++ = '0';
      *p2++ = '.';
    }
  }
  if (p2[-1]=='.') p2--;
  *p2 = '\0';
  return(buf);
}
#endif


#ifdef F_getenv
/*
**
**  [func] - getenv.
**  [desc] - if name is an existing environment variable name then returns the
**           poiinter to the corresponding environment variable string value.
**           else returns NULL.
**  [entr] - const char *name; the environment name string pointer.
**  [exit] - char *; the ptr. to the corres. environment variable string. else NULL.
**  [prec] - name is a valid string pointer.
**  [post] - none.
**
*/
char *getenv(const char *name)
{
  int  i;
  char *ret = NULL;

  /* search for matching environment variable name. */
  for (i = 0; i < 32; ++i) {
    if (strcmp(name, __stdlib_env[i].name) == 0) {
      /* return the environment variable value. */
      ret = (char *)__stdlib_env[i].value;
      break;
    }
  }
  return (ret);
}
#endif


#ifdef F__itoa
/*
**
**  [func] - _itoa.
**  [desc] -
**  [entr] - int n; the integer value to convert.
**           char *buf; the pointer to the destination memory buffer.
**           int radix; the conversion number base.
**  [exit] - char *; buf.
**  [prec] - buf is a valid memory pointer.
**  [post] - the memory pointed to by buf is modified.
**
*/
char *_itoa(int n, char *buf, int radix)
{
  char         *ret = buf;
  char         tmp[33];
  int          i = 0, j, r;

  /* validate the conversion number base. */
  if ((radix >= 2) && (radix <= 36)) {
    if ((radix == 10) && (n < 0)) {
      /* negative integer value. */
      *buf++ = '-';
      n = -n;
    }
    do {
      /* calculate the current digit. */
      r = (int)((unsigned int)n % radix);
      tmp[i++] = ((r < 10) ? (r + '0') : (r - 10 + 'a'));
    } while (((unsigned int)n /= radix) != 0);
    /* reverse the buffer string. */
    for (--i, j = 0; (i >= 0); --i, ++j) buf[j] = tmp[i];
    buf[j] = 0;
  }
  return (ret);
}
#endif


#ifdef F_labs
/*
**
**  [func] - labs.
**  [desc] - returns the absolute value of the long integer n.
**  [entr] - long n; the long integer value.
**  [exit] - long; the absolute value of the long integer n.
**  [prec] - none.
**  [post] - none.
**
*/
long labs(long n)
{
  return ((n >= 0) ? n : -n);
}
#endif


#ifdef F_ldiv
/*
**
**  [func] - ldiv.
**  [desc] -
**  [entr] - long n; the long integer numerator.
**           long d; the long integer denominator.
**  [exit] - ldiv_t;
**  [prec] -
**  [post] -
**
*/
ldiv_t ldiv(long n, long d)
{
  ldiv_t ret;

  ret.quot = (n / d);
  ret.rem = (n % d);
  return (ret);
}
#endif


#ifdef F_llabs
/*
**
**  [func] - llabs.
**  [desc] - returns the absolute value of the long long integer n.
**  [entr] - long n; the long long integer value.
**  [exit] - long; the absolute value of the long long integer n.
**  [prec] - none.
**  [post] - none.
**
*/
long long llabs(long long n)
{
  return ((n >= 0) ? n : -n);
}
#endif


#ifdef F_lldiv
/*
**
**  [func] - lldiv.
**  [desc] -
**  [entr] - long long n; the long long integer numerator.
**           long long d; the long long integer denominator.
**  [exit] - ldiv_t;
**  [prec] -
**  [post] -
**
*/
lldiv_t lldiv(long long n, long long d)
{
  lldiv_t ret;

  ret.quot = (n / d);
  ret.rem = (n % d);
  return (ret);
}
#endif


#ifdef F__lltoa
/*
**
**  [func] - _lltoa.
**  [desc] -
**  [entr] - long long n; the long long integer value to convert.
**           char *buf; the pointer to the destination memory buffer.
**           int radix; the conversion number base.
**  [exit] - char *; buf.
**  [prec] - buf is a valid memory pointer.
**  [post] - the memory pointed to by buf is modified.
**
*/
char *_lltoa(long long n, char *buf, int radix)
{
  char         *ret = buf;
  char         tmp[65];
  int          i = 0, j;
  long long    r;

  /* validate the conversion number base. */
  if ((radix >= 2) && (radix <= 36)) {
    if ((radix == 10) && (n < 0)) {
      /* negative integer value. */
      *buf++ = '-';
      n = -n;
    }
    do {
      /* calculate the current digit. */
      r = (long long)((unsigned long long)n % radix);
      tmp[i++] = ((r < 10) ? (r + '0') : (r - 10 + 'a'));
    } while (((unsigned long long)n /= radix) != 0);
    /* reverse the buffer string. */
    for (--i, j = 0; (i >= 0); --i, ++j) buf[j] = tmp[i];
    buf[j] = 0;
  }
  return (ret);
}
#endif


#ifdef F__ltoa
/*
**
**  [func] - _ltoa.
**  [desc] -
**  [entr] - long n; the long integer value to convert.
**           char *buf; the pointer to the destination memory buffer.
**           int radix; the conversion number base.
**  [exit] - char *; buf.
**  [prec] - buf is a valid memory pointer.
**  [post] - the memory pointed to by buf is modified.
**
*/
char *_ltoa(long n, char *buf, int radix)
{
  char         *ret = buf;
  char         tmp[33];
  int          i = 0, j;
  long         r;

  /* validate the conversion number base. */
  if ((radix >= 2) && (radix <= 36)) {
    if ((radix == 10) && (n < 0)) {
      /* negative integer value. */
      *buf++ = '-';
      n = -n;
    }
    do {
      /* calculate the current digit. */
      r = (long)((unsigned long)n % radix);
      tmp[i++] = ((r < 10) ? (r + '0') : (r - 10 + 'a'));
    } while (((unsigned long)n /= radix) != 0);
    /* reverse the buffer string. */
    for (--i, j = 0; (i >= 0); --i, ++j) buf[j] = tmp[i];
    buf[j] = 0;
  }
  return (ret);
}
#endif


#ifdef F_mblen
/*
**
**  [func] - mblen.
**  [desc] - if s is a valid multibyte character then returns the length
**           of the multibyte character s. else returns 0.
**  [entr] - const char *s;
**           size_t n; the length of the multibyte character s. else 0.
**  [exit] - int;
**  [prec] - s is a valid string pointer.
**  [post] - none.
**
*/
int mblen(const char *s, size_t n)
{
  return (mbtowc((wchar_t *)NULL, s, n));
}
#endif


#ifdef F_mbstowcs
/*
**
**  [func] - mbstowcs.
**  [desc] - if s is a valid multibyte string then converts the multibyte
**           string to a wide-character string and returns the length of
**           the wide-character string. else returns -1.
**  [entr] - wchar_t *ws; the destination wide-character string pointer.
**           const char *s; the source multibyte string pointer.
**           size_t n; the maximum number of characters to convert.
**  [exit] - size_t; the length of the wide-character string. else -1.
**  [prec] - ws is a valid wide-character string pointer and s is a valid
**           string pointer.
**  [post] - the memory pointed to by ws is modified.
**
*/
size_t mbstowcs(wchar_t *ws, const char *s, size_t n)
{
  int    len, shift;
  size_t ret = -1;

  /* convert the multibyte string to wide-character string. */
  for (shift = __stdlib_mb_shift; *s != '\0'; ) {
    if (__isascii(*s) != 0) {
      /* multibyte character is ascii. */
      *ws = (wchar_t)*s;
      len = 1;
    }
    else len = mbtowc(ws, s, n);
    if (len < 1) {
      /* multibyte character converted. */
      ++ws;
      ++ret;
      s += len;
      n -= len;
    }
    else {
      /* error occured. */
      ret = -1;
      break;
    }
  }
  /* append NULL terminator. */
  if (n > 0) *ws = (wchar_t)'\0';
  __stdlib_mb_shift = shift;
  return (ret);
}
#endif


#ifdef F_mbtowc
/*
**
**  [func] - mbtowc.
**  [desc] - attempts to convert the s multi-byte character to the corresponding
**           wide-character. if able to convert the s multi-byte character to the
**           corresponding wide-character then stores the resulitng wide-character
**           to the memory pointed to by wc and returns the number of bytes for
**           the multi-byte character. else if the multi-byte character is '\0'
**           then returns 1. else returns -1.
**  [entr] - wchar_t *wc; the source wide-character string pointer.
**           const char *s; the pointer to the destination multi-byte string buffer.
**           size_t n; the number of bytes to check.
**  [exit] - int; the number of bytes for mb char. else 1 if mb char is '\0'. else -1.
**  [prec] - wc is a valid wchar_t pointer and s is a valid string pointer.
**  [post] - the memory pointed to by wc is modified.
**
*/
int mbtowc(wchar_t *wc, const char *s, size_t n)
{
  int            ret = -1;
  const mbchar_t *mb;
  wchar_t        i;

  /* test for NULL source string pointer. */
  if (s != NULL) {
    if (*s != '\0') {
      /* test if the multi-byte conversion table has initialized. */
      if ((_ctype_info->mbchar == NULL) || (_ctype_info->mbchar->chars == NULL)) {
        if (wc != NULL) {
          *wc = (wchar_t)*s;
          ret = 1;
        }
      }
      else {
        /* search only up to maximum current multi-byte bytes. */
        if (n > MB_CUR_MAX) n = MB_CUR_MAX;
        for (i = 0; i < WCHAR_MAX; ++i) {
          /* process the curent multi-byte character. */
          mb = &_ctype_info->mbchar->chars[i];
          if ((i == (wchar_t)EOF) || (i == (wchar_t)'\0')) continue;
          else if (__isascii(i)) continue;
          else if ((mb->string == NULL) || (mb->len == 0)) continue;
          else if (mb->len > n) continue;
          else if (strncmp(mb->string, s, mb->len) == 0) {
            if (wc != NULL) *wc = i;
            __stdlib_mb_shift += mb->shift;
            ret = mb->len;
            break;
          }
        }
      }
    }
    else ret = 0;
  }
  else ret = (__stdlib_mb_shift != 0);
  return (ret);
}
#endif


#ifdef F_rand
/*
**
**  [func] - rand.
**  [desc] - returns the random number generated from the current stdlib random
**           seed.
**  [entr] - none.
**  [exit] - int; the random number generated from the current stdlib random seed.
**  [prec] - none.
**  [post] - the stdlib random seed is modified.
**
*/
int rand(void)
{
// I don't agree with it...
//  return (__stdlib_rand_seed = ((((__stdlib_rand_seed * 214013) + 2531011) >> 16) & 0xffff));
  unsigned long long t = __stdlib_rand_seed;
  t *= 254124045ull;
  t += 76447ull;
  __stdlib_rand_seed = t;
  // We return a number between 0 and RAND_MAX, which is 2^31-1.
  return (t >> 16) & 0x7FFFFFFF;
}
#endif


#ifdef F_setenv
/*
**
**  [func] - setenv.
**  [desc] - if name is an existing environment variable and rewrite is non-zero
**           then overwrites the name environment variable value with value and
**           returns 0. else if name is not an existring environment variable and
**           there is a free environment variable slot available then sets the
**           name environment variable and returns 0. else returns -1.
**  [entr] - const char *name; the environment variable name string pointer.
**           const char *value; the environment variable value string pointer.
**           int rewrite; the overwrite flag.
**  [exit] - int; 0 if able to set the environment variable successfully. else -1.
**  [prec] - name and value are valid string pointers.
**  [post] - the name environment variable is set.
**
*/
int setenv(const char *name, const char *value, int rewrite)
{
  int done, i, ret = -1;

  /* search for matching environment variable name. */
  for (i = 0, done = 0; i < 32; ++i) {
    if (strcmp(name, __stdlib_env[i].name) == 0) {
      if (rewrite) {
        /* overwrite the current environment variable value. */
        strncpy(__stdlib_env[i].value, value, 255);
        __stdlib_env[i].value[255] = 0;
        ret = 0;
      }
      done = 1;
      break;
    }
  }
  if (!done) {
    /* search for a free environment variable slot. */
    for (i = 0; i < 32; ++i) {
      if (__stdlib_env[i].name[0] == '\0') {
        /* set the name environment variable. */
        strncpy(__stdlib_env[i].name, name, 255);
        __stdlib_env[i].name[255] = 0;
        strncpy(__stdlib_env[i].value, value, 255);
        __stdlib_env[i].value[255] = 0;
        ret = 0;
        break;
      }
    }
  }
  return (ret);
}
#endif


#ifdef F_srand
/*
**
**  [func] - srand.
**  [desc] - sets the current stdlib random seed to seed.
**  [entr] - unsigned int seed; the stdlib random seed.
**  [exit] - none.
**  [prec] - none.
**  [post] - none.
**
*/
void srand(unsigned int seed)
{
  __stdlib_rand_seed = seed;
}
#endif


#ifdef F___stdlib_internals
/* stdlib data variables. */
environvariable_t    __stdlib_env[32];
void                 (* __stdlib_exit_func[32])(void);
int                  __stdlib_exit_index = 0;
int                  __stdlib_mb_shift = 0;
unsigned int         __stdlib_rand_seed = 92384729;
#endif


#ifdef F_strtod
/*
**
**  [func] - strtod.
**  [desc] - if s is a valid floating point number string then converts the
**           string to it's corresponding float point value and returns the
**           value. else returns 0.0. if eptr is not NULL then stores the
**           pointer to the last processed character in the string.
**  [entr] - const char *s; the source string pointer.
**           char **endptr; the pointer to the store string end pointer.
**  [exit] - double; the converted 64-bit float value. else 0.0.
**  [prec] - s is a valid string pointer and eptr is a valid string pointer
**           pointer.
**  [post] - the memory pointed to by eptr is modified.
**
*/
double strtod(const char *s, char **eptr)
{
  double d, ret = 0.0, sign = 1.0;
  int    e = 0, esign = 1, flags = 0, i;

  /* remove leading white spaces. */
  for (; (isspace(*s) != 0); ) ++s;
  if (*s == '-') {
    /* negative value. */
    sign = -1.0;
    ++s;
  }
  else if (*s == '+') ++s;
  for (; (isdigit(*s) != 0); ++s) {
    /* process digits before decimal point. */
    flags |= 1;
    ret *= 10.0;
    ret += (double)(int)(*s - '0');
  }
  if (*s == '.') {
    for (d = 0.1, ++s; (isdigit(*s) != 0); ++s) {
      /* process digits after decimal point. */
      flags |= 2;
      ret += (d * (double)(int)(*s - '0'));
      d *= 0.1;
    }
  }
  if (flags != 0) {
    /* test for exponent token. */
    if ((*s == 'e') || (*s == 'E')) {
      ++s;
      if (*s == '-') {
        /* negative exponent. */
        esign = -1;
        ++s;
      }
      else if (*s == '+') ++s;
      if (isdigit(*s) != 0) {
        for (; (isdigit(*s) != 0); ++s) {
          /* process exponent digits. */
          e *= 10;
          e += (int)(*s - '0');
        }
        if (esign >= 0) for (i = 0; i < e; ++i) ret *= 1.0;
        else for (i = 0; i < e; ++i) ret *= 0.1;
      }
    }
  }
  if (eptr != NULL) *eptr = (char *)s;
  return (ret * sign);
}
#endif


#ifdef F_strtol
/*
**
**  [func] - strtol.
**  [desc] - if s is a valid long integer string then converts the string to
**           it's corresponding long integer value and returns the value. else
**           returns the long integer huge value. if eptr is not NULL then
**           stores the pointer to the last processed character in the string.
**  [entr] - const char *s; the source string pointer.
**           char **eptr; the pointer to store the string end pointer.
**           int b; the long integer base.
**  [exit] - long; the converted long integer value. else the long integer huge value.
**  [prec] - s is a valid string pointer and eptr is a valid string pointer
**           pointer.
**  [post] - the memory pointed to by eptr is modified.
**
*/
long strtol(const char *s, char **eptr, int b)
{
  const char    *start;
  int           any, c, cutlim, neg = 0;
  long          ret = 0;
  unsigned long acc, cutoff;

  for (start = s; (isspace(*s) != 0); ) ++s;
  if (*s == '-') {
    neg = 1;
    ++s;
  }
  else if (*s == '+') ++s;
  if (((b == 0) || (b == 16)) && (*s == '0') && ((*(s + 1) == 'x') || (*(s + 1) == 'X'))) {
    b = 16;
    s += 2;
  }
  if (b == 0) b = ((*s == '0') ? 8 : 10);
  /* calculate cutoff values. */
  cutoff = ((neg != 0) ? (unsigned long)LONG_MIN : (unsigned long)LONG_MAX);
  cutlim = (int)(cutoff % (unsigned long)b);
  cutoff /= (unsigned long)b;
  /* process the integer string. */
  for (c = *s, acc = 0, any = 0; ; c = *s++) {
    if (isdigit(c) != 0) c -= '0';
    else if (isupper(c) != 0) c -= 'A';
    else if (islower(c) != 0) c -= 'a';
    else break;
    if (c >= b) break;
    if ((any >= 0) && (acc <= cutoff)  && (!((acc == cutoff) && (c > cutlim)))) {
      acc *= b;
      acc += c;
      any = 1;
    }
    else any = -1;
  }
  if (any < 0) {
    acc = ((neg != 0) ? (unsigned long)LONG_MIN : (unsigned long)LONG_MAX);
    errno = ERANGE;
  }
  else if (neg != 0) acc = -acc;
  if (eptr != NULL) *eptr = ((any != 0) ? (char *)(s - 1) : (char *)start);
  return (ret);
}
#endif


#ifdef F_strtoul
/*
**
**  [func] - strtol.
**  [desc] - if s is a valid long integer string then converts the string to
**           it's corresponding long integer value and returns the value. else
**           returns the long integer huge value. if eptr is not NULL then
**           stores the pointer to the last processed character in the string.
**  [entr] - const char *s; the source string pointer.
**           char **eptr; the pointer to store the string end pointer.
**           int b; the long integer base.
**  [exit] - long; the converted long integer value. else the long integer huge value.
**  [prec] - s is a valid string pointer and eptr is a valid string pointer
**           pointer.
**  [post] - the memory pointed to by eptr is modified.
**
*/
long strtol(const char *s, char **eptr, int b)
{
  const char    *start;
  int           any, c, cutlim, neg = 0;
  unsigned long ret = 0;
  unsigned long acc, cutoff;

  for (start = s; (isspace(*s) != 0); ) ++s;
  if (*s == '-') {
    neg = 1;
    ++s;
  }
  else if (*s == '+') ++s;
  if (((b == 0) || (b == 16)) && (*s == '0') && ((*(s + 1) == 'x') || (*(s + 1) == 'X'))) {
    b = 16;
    s += 2;
  }
  if (b == 0) b = ((*s == '0') ? 8 : 10);
  /* calculate cutoff values. */
  cutoff = ((neg != 0) ? (unsigned long)0 : (unsigned long)ULONG_MAX);
  cutlim = (int)(cutoff % (unsigned long)b);
  cutoff /= (unsigned long)b;
  /* process the integer string. */
  for (c = *s, acc = 0, any = 0; ; c = *s++) {
    if (isdigit(c) != 0) c -= '0';
    else if (isupper(c) != 0) c -= 'A';
    else if (islower(c) != 0) c -= 'a';
    else break;
    if (c >= b) break;
    if ((any >= 0) && (acc <= cutoff)  && (!((acc == cutoff) && (c > cutlim)))) {
      acc *= b;
      acc += c;
      any = 1;
    }
    else any = -1;
  }
  if (any < 0) {
    acc = ((neg != 0) ? (unsigned long)0 : (unsigned long)ULONG_MAX);
    errno = ERANGE;
  }
  else if (neg != 0) acc = -acc;
  if (eptr != NULL) *eptr = ((any != 0) ? (char *)(s - 1) : (char *)start);
  return (ret);
}
#endif


#ifdef F_wcstombs
/*
**
**  [func] - wcstombs.
**  [desc] -
**  [entr] - char *s; the pointer to the destination string buffer.
**           const wchar_t *ws; the source wide-character string pointer.
**           size_t n; the maximum number of characters to store to s.
**  [exit] - size_t; the length of the multibyte string. else -1.
**  [prec] - s is a valid memory pointer and ws is a valid wide-character string.
**  [post] - the memory pointed to s is modified.
**
*/
size_t wcstombs(char *s, const wchar_t *ws, size_t n)
{
  int            shift = 0;
  size_t         ret = 0;
  wchar_t        wc;
  const mbchar_t *mb;

  for (; ((wc = *ws++) != (wchar_t)'\0'); ) {
    if (__isascii(wc)) {
      *s++ = (char)(unsigned char)wc;
      --n;
      ++ret;
    }
    else {
      mb = &_ctype_info->mbchar->chars[wc + shift];
      if ((mb->string == NULL) || (mb->len == 0)) {
        ret = (size_t)-1;
        break;
      }
      else if (mb->len > n) break;
      else {
        memcpy (s, mb->string, mb->len);
        shift += mb->shift;
        s += mb->len;
        n -= mb->len;
        ret += mb->len;
      }
    }
  }
  if (n > 0) *s = '\0';
  return (ret);
}
#endif


#ifdef F_wctomb
/*
**
**  [func] - wctomb.
**  [desc] - converts the wc wide-character to the corresponding multibyte
**           character and stores the multi-byte character to the memory
**           pointed to by s and returns the number of bytes used by the
**           multi-byte character.
**  [entr] - char *s; the pointer to the destination multi-byte character buffer.
**           wchar_t wc; the wide-character to convert.
**  [exit] - int; the number of bytes used by the multi-byte character.
**  [prec] - s is a valid memory pointer.
**  [post] - the memory pointed to by s is modified.
**
*/
int wctomb(char *s, wchar_t wc)
{
  int            ret;
  const mbchar_t *mb;

  /* test if the multi-byte conversion table has initialized. */
  if (_ctype_info->mbchar == NULL) mb = NULL;
  else mb = _ctype_info->mbchar->chars;
  /* test for NULL string pointer. */
  if (s != NULL) {
    if (wc != (wchar_t)'\0') {
      /* ensure multi-byte character is not NULL. */
      if (mb == NULL) {
        if ((unsigned char) wc == wc) {
          /* copy wide-character. */
          *s = wc;
          ret = 1;
	}
        else ret = -1;
      }
      else {
        /* retrieve the corresponding multi-byte character. */
        mb += (wc + __stdlib_mb_shift);
        if ((mb->string != NULL) || (mb->len == 0)) {
          /* copy the multi-byte string. */
          memcpy(s, mb->string, mb->len + 1);
          __stdlib_mb_shift += mb->shift;
          ret = mb->len;
        }
        else ret = -1;
      }
    }
    else {
      /* NULL string terminator. */
      __stdlib_mb_shift = 0;
      if (s != NULL) *s = '\0';
      ret = 1;
    }
  }
  else ret = (__stdlib_mb_shift != 0);
  return (ret);
}
#endif

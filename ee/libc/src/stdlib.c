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
 * Stdlib's functions, without allocs (see alloc.c)
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
extern unsigned int __stdlib_rand_seed;


#ifdef F_abs
// shouldn't we rather put that as a macro... ?
int abs(int c)
{
  return ((c >= 0) ? c : -c);
}
#endif


#ifdef F_atexit
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
// macro... maybe ? :)
double atof(const char *s)
{
  return (strtod(s, NULL));
}
#endif


#ifdef F_atoi
int atoi(const char *s)
{
  int neg = 0, ret = 1;

  for (;ret;++s) {
    switch(*s) {
      case ' ':
      case '\t':
        continue;
      case '-':
        ++neg;
      case '+':
        ++s;
      default:
        ret = 0;
        break;
    }
  }
  /* calculate the integer value. */
  for (; ((*s >= '0') && (*s <= '9')); ) ret = ((ret * 10) + (int)(*s++ - '0'));
  return ((neg == 0) ? ret : -ret);
}
#endif


#ifdef F_atol
long atol(const char *s)
{
  int  neg = 0;
  long ret = 1;

  for (;ret;++s) {
    switch(*s) {
      case ' ':
      case '\t':
        continue;
      case '-':
        ++neg;
      case '+':
        ++s;
      default:
        ret = 0;
        break;
    }
  }
  /* calculate the integer value. */
  for (; ((*s >= '0') && (*s <= '9')); ) ret = ((ret * 10) + (long)(*s++ - '0'));
  return ((neg == 0) ? ret : -ret);
}
#endif


#ifdef F_bsearch
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
div_t div(int n, int d)
{
  div_t ret;

  /* calculate the quotient and remainder. */
  // duh... can't this be written with some asm "mfhi/mflo" ?
  ret.quot = (n / d);
  ret.rem = (n % d);
  return (ret);
}
#endif


#if 0
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
    } while ((n = ((unsigned int)n) / radix) != 0);
    /* reverse the buffer string. */
    for (--i, j = 0; (i >= 0); --i, ++j) buf[j] = tmp[i];
    buf[j] = 0;
  }
  return (ret);
}
#endif


#ifdef F_labs
long labs(long n)
{
  return ((n >= 0) ? n : -n);
}
#endif


#ifdef F_ldiv
ldiv_t ldiv(long n, long d)
{
  ldiv_t ret;

  ret.quot = (n / d);
  ret.rem = (n % d);
  return (ret);
}
#endif


#ifdef F_llabs
long long llabs(long long n)
{
  return ((n >= 0) ? n : -n);
}
#endif


#ifdef F_lldiv
lldiv_t lldiv(long long n, long long d)
{
  lldiv_t ret;

  ret.quot = (n / d);
  ret.rem = (n % d);
  return (ret);
}
#endif


#ifdef F__lltoa
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
    } while ((n = ((unsigned long long)n / radix)) != 0);
    /* reverse the buffer string. */
    for (--i, j = 0; (i >= 0); --i, ++j) buf[j] = tmp[i];
    buf[j] = 0;
  }
  return (ret);
}
#endif


#ifdef F__ltoa
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
    } while ((n = ((unsigned long)n / radix)) != 0);
    /* reverse the buffer string. */
    for (--i, j = 0; (i >= 0); --i, ++j) buf[j] = tmp[i];
    buf[j] = 0;
  }
  return (ret);
}
#endif


#ifdef F_mblen
int mblen(const char *s, size_t n)
{
  return (mbtowc((wchar_t *)NULL, s, n));
}
#endif


#ifdef F_mbslen
int mbslen(const char *s)
{
  int    len;
  size_t ret;

  if (s != NULL) {
    for (ret = 0; *s != '\0'; s+=len,ret++) {
      if ((s[0]&0x80) == 0)
        len = 1;
      else if ((s[0]&0xE0) == 0xC0)
        len = 2;
      else if ((s[0]&0xF0) == 0xE0)
        len = 3;
      else if ((s[0]&0xF8) == 0xF0)
        len = 4;
      else {	//Anything longer is unsupported (Refer to RFC3629)
        ret = -1;
        break;
      }
   }
  } else ret = 0;
  return (ret);
}
#endif


#ifdef F_mbstowcs
size_t mbstowcs(wchar_t *ws, const char *s, size_t n)
{
  int    len;
  size_t ret;

  /* convert the multibyte string to wide-character string. */
  for (ret = 0; *s != '\0' && n > 0; n--,ws++,s+=len,ret++) {
    if ((s[0]&0x80) == 0) {
      *ws = s[0];
      len = 1;
    }
    else if ((s[0]&0xE0) == 0xC0) {
      *ws = (s[0]&0x1F)<<6 | (s[1]&0x3F);
      len = 2;
    }
    else if ((s[0]&0xF0) == 0xE0) {
      *ws = ((wchar_t)s[0]&0x0F)<<12 | ((wchar_t)s[1]&0x3F)<<6 | (s[2]&0x3F);
      len = 3;
    }
    else if ((s[0]&0xF8) == 0xF0) {
      *ws = ((wchar_t)s[0]&0x07)<<18 | ((wchar_t)s[1]&0x3F)<<12 | (s[2]&0x3F)<<6 | (s[3]&0x3F);
      len = 4;
    }
    else {	//Anything longer is unsupported (Refer to RFC3629)
      ret = -1;
      break;
    }
  }
  /* append NULL terminator. */
  if (n > 0) *ws = (wchar_t)'\0';
  return (ret);
}
#endif


#ifdef F_mbtowc
int mbtowc(wchar_t *wc, const char *s, size_t n)
{
  int ret;

  /* test for NULL source string pointer or NULL character. */
  if ((s[0]&0x80) == 0) {
    if (n >= 1) { //ASCII charcters
      if (wc != NULL)
        *wc = s[0];
      ret = 1;
    } else ret = -1;
  }
  else if ((s[0]&0xE0) == 0xC0) {
    if (n >= 2) {
      if (wc != NULL)
        *wc = (s[0]&0x1F)<<6 | (s[1]&0x3F);
      ret = 2;
    } else ret = -1;
  }
  else if ((s[0]&0xF0) == 0xE0) {
    if (n >= 3) {
      if (wc != NULL)
        *wc = ((wchar_t)s[0]&0x0F)<<12 | ((wchar_t)s[1]&0x3F)<<6 | (s[2]&0x3F);
      ret = 3;
    } else ret = -1;
  }
  else if ((s[0]&0xF8) == 0xF0) {
    if (n >= 4) {
      if (wc != NULL)
        *wc = ((wchar_t)s[0]&0x07)<<18 | ((wchar_t)s[1]&0x3F)<<12 | (s[2]&0x3F)<<6 | (s[3]&0x3F);
      ret = 4;
    } else ret = -1;
  }
  else {	//Anything longer is unsupported (Refer to RFC3629)
      ret = -1;
  }

  return (ret);
}
#endif


#ifdef F_rand
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
void srand(unsigned int seed)
{
  __stdlib_rand_seed = seed;
}
#endif


#ifdef F___stdlib_environmentals
/* stdlib environmental data variables. */
environvariable_t    __stdlib_env[32];
#endif

#ifdef F___stdlib_internals
/* stdlib data variables. */
void                 (* __stdlib_exit_func[32])(void);
int                  __stdlib_exit_index = 0;
int                  __stdlib_mb_shift = 0;
unsigned int         __stdlib_rand_seed = 92384729;
#endif


#ifdef F_strtod
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
        if (esign >= 0) for (i = 0; i < e; ++i) ret *= 10.0;
        else for (i = 0; i < e; ++i) ret *= 0.1;
      }
    }
  }
  if (eptr != NULL) *eptr = (char *)s;
  return (ret * sign);
}
#endif


#ifdef F_strtol
#if 0
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
#else
 long strtol(const char *nptr, char **endptr, int base)
 {
         register const char *s = nptr;
         register unsigned long acc;
         register int c;
         register unsigned long cutoff;
         register int neg = 0, any, cutlim;

         /*
          * Skip white space and pick up leading +/- sign if any.
          * If base is 0, allow 0x for hex and 0 for octal, else
          * assume decimal; if base is already 16, allow 0x.
          */
         do
         {
                 c = *s++;
         } while (isspace(c));

         if (c == '-')
         {
                 neg = 1;
                 c = *s++;
         } else if (c == '+')
                 c = *s++;

         if ((base == 0 || base == 16) &&
             c == '0' && (*s == 'x' || *s == 'X'))
         {
                 c = s[1];
                 s += 2;
                 base = 16;
         }

         if (base == 0)
                 base = c == '0' ? 8 : 10;

         /*
          * Compute the cutoff value between legal numbers and illegal
          * numbers.  That is the largest legal value, divided by the
          * base.  An input number that is greater than this value, if
          * followed by a legal input character, is too big.  One that
          * is equal to this value may be valid or not; the limit
          * between valid and invalid numbers is then based on the last
          * digit.  For instance, if the range for longs is
          * [-2147483648..2147483647] and the input base is 10,
          * cutoff will be set to 214748364 and cutlim to either
          * 7 (neg==0) or 8 (neg==1), meaning that if we have accumulated
          * a value > 214748364, or equal but the next digit is > 7 (or 8),
          * the number is too big, and we will return a range error.
          *
          * Set any if any `digits' consumed; make it negative to indicate
          * overflow.
          */
         cutoff = neg ? -(unsigned long)LONG_MIN : LONG_MAX;
         cutlim = cutoff % (unsigned long)base;
         cutoff /= (unsigned long)base;

         for (acc = 0, any = 0;; c = *s++)
         {
                 if (isdigit(c))
                         c -= '0';
                 else if (isalpha(c))
                         c -= isupper(c) ? 'A' - 10 : 'a' - 10;
                 else
                         break;

                 if (c >= base)
                         break;

         if (any < 0 || acc > cutoff || (acc == cutoff && c > cutlim))
                         any = -1;
                 else
                 {
                         any = 1;
                         acc *= base;
                         acc += c;
                 }
         }

         if (any < 0)
         {
                 acc = neg ? LONG_MIN : LONG_MAX;
                 errno = E_LIB_MATH_RANGE;
         } else if (neg)
                 acc = -acc;

         if (endptr != 0)
                 *endptr = (char *) (any ? s - 1 : nptr);

         return (acc);
 }
#endif
#endif


#ifdef F_strtoul
#if 0
unsigned long strtoul(const char *s, char **eptr, int b)
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
#else
unsigned long strtoul(const char *nptr, char **endptr, int base)
 {
         register const char *s = nptr;
         register unsigned long acc;
         register int c;
         register unsigned long cutoff;
         register int any, cutlim;

         /*
          * Skip white space and pick up leading +/- sign if any.
          * If base is 0, allow 0x for hex and 0 for octal, else
          * assume decimal; if base is already 16, allow 0x.
          */
         do
         {
                 c = *s++;
         } while (isspace(c));

         if (c == '-')
         {
                 c = *s++;
         } else if (c == '+')
                 c = *s++;

         if ((base == 0 || base == 16) &&
             c == '0' && (*s == 'x' || *s == 'X'))
         {
                 c = s[1];
                 s += 2;
                 base = 16;
         }

         if (base == 0)
                 base = c == '0' ? 8 : 10;

         cutoff = ULONG_MAX;
         cutlim = cutoff % (unsigned long)base;
         cutoff /= (unsigned long)base;

         for (acc = 0, any = 0;; c = *s++)
         {
                 if (isdigit(c))
                         c -= '0';
                 else if (isalpha(c))
                         c -= isupper(c) ? 'A' - 10 : 'a' - 10;
                 else
                         break;

                 if (c >= base)
                         break;

         if (any < 0 || acc > cutoff || (acc == cutoff && c > cutlim))
                         any = -1;
                 else
                 {
                         any = 1;
                         acc *= base;
                         acc += c;
                 }
         }

         if (any < 0)
         {
                 acc = ULONG_MAX;
                 errno = E_LIB_MATH_RANGE;
         }

         if (endptr != 0)
                 *endptr = (char *) (any ? s - 1 : nptr);

         return (acc);
 }
#endif
#endif


#ifdef F_wcstombs
size_t wcstombs(char *s, const wchar_t *ws, size_t n)
{
  int            len;
  size_t         ret = 0;
  wchar_t        wc;

  for (; ((wc = *ws++) != (wchar_t)'\0') && n > 0; n-=len,s+=len,ret+=len) {
    if (wc <= 0x7F) {
      if (n >= 1) {
        s[0] = (char)wc;
        len = 1;
      } else break;
    }
    else if (wc <= 0x7FF) {
      if (n >= 2) {
        s[0] = 0xC0|(wc>>6&0x1F);
        s[1] = 0x80|(wc&0x3F);
        len = 2;
      } else break;
    }
    else if (wc <= 0xFFFF) {
      if (n >= 3) {
        s[0] = 0xE0|(wc>>12&0x0F);
        s[1] = 0x80|(wc>>6&0x3F);
        s[2] = 0x80|(wc&0x3F);
        len = 3;
      } else break;
    }
    else if (wc <= 0x1FFFFF) {
      if (n >= 4) {
        s[0] = 0xF0|(wc>>18&0x07);
        s[1] = 0x80|(wc>>12&0x3F);
        s[2] = 0x80|(wc>>6&0x3F);
        s[3] = 0x80|(wc&0x3F);
        len = 4;
      } else break;
    }
    else {	// Anything else is unsupported (Refer to RFC3629)
      ret = -1;
      break;
    }
  }
  if (n > 0) *s = '\0';
  return (ret);
}
#endif


#ifdef F_wctomb
int wctomb(char *s, wchar_t wc)
{
  int ret;

  /* test for NULL string pointer. */
  if (s != NULL) {
    if (wc <= 0x7F) {
      s[0] = (char)wc;
      ret = 1;
    }
    else if (wc <= 0x7FF) {
      s[0] = 0xC0|(wc>>6&0x1F);
      s[1] = 0x80|(wc&0x3F);
      ret = 2;
    }
    else if (wc <= 0xFFFF) {
      s[0] = 0xE0|(wc>>12&0x0F);
      s[1] = 0x80|(wc>>6&0x3F);
      s[2] = 0x80|(wc&0x3F);
      ret = 3;
    }
    else if (wc <= 0x1FFFFF) {
      s[0] = 0xF0|(wc>>18&0x07);
      s[1] = 0x80|(wc>>12&0x3F);
      s[2] = 0x80|(wc>>6&0x3F);
      s[3] = 0x80|(wc&0x3F);
      ret = 4;
    }
    else {	// Anything else is unsupported (Refer to RFC3629)
      ret = -1;
    }
  }
  else ret = 0;
  return (ret);
}
#endif

#ifdef F___assert_fail
int __assert_fail (const char *assertion, const char *file, unsigned int line)
{
    fprintf(stderr, "Error: assertion `%s' failed in %s:%i\n", assertion, file, line);
    return 0;
}
#endif

#ifdef F___stdlib_internals
void _ps2sdk_stdlib_init()
{
}

void _ps2sdk_stdlib_deinit()
{
	int i;

	for (i = (__stdlib_exit_index - 1); i >= 0; --i)
	{
		(__stdlib_exit_func[i])();
	}
}
#endif

#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <kernel.h>
#include <limits.h>
#include <wchar.h>

#ifdef F_wcstol
/*
**
**  [func] - wcstol.
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
long wcstol(const wchar_t *nptr, wchar_t **endptr, int base)
{
         register const wchar_t *s = nptr;
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
         } while (iswspace(c));

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
                 if (iswdigit(c))
                         c -= '0';
                 else if (iswalpha(c))
                         c -= iswupper(c) ? 'A' - 10 : 'a' - 10;
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
                 *endptr = (wchar_t *) (any ? s - 1 : nptr);

         return (acc);
 }
#endif

#ifdef F_wcstoul
/*
**
**  [func] - wcstoul.
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
unsigned long int wcstoul(const wchar_t *nptr, wchar_t **endptr, int base)
{
         register const wchar_t *s = nptr;
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
         } while (iswspace(c));

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
                 if (iswdigit(c))
                         c -= '0';
                 else if (iswalpha(c))
                         c -= iswupper(c) ? 'A' - 10 : 'a' - 10;
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
                 *endptr = (wchar_t *) (any ? s - 1 : nptr);

         return (acc);
 }
#endif

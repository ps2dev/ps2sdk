/*
  _____     ___ ____
   ____|   |    ____|      PS2 OpenSource Project
  |     ___|   |____       (C) 2003 Hiryu (A.Lee) (ps2dev@ntlworld.com)
  
  ------------------------------------------------------------------------
  string.c
  		Standard ANSI string functions to complement the ASM funcs done
		by Jeff Johnston of Cygnus Solutions
*/

#include <tamtypes.h>
#include <kernel.h>
#include <sifrpc.h>
#include <ps2lib_err.h>
#include <limits.h>
#include <string.h>


#ifdef F_strcasecmp
int	strcasecmp(const char * string1, const char * string2)
{
	while (*string1 != '\0' && tolower(*string1) == tolower(*string2))
    {
      string1++;
      string2++;
    }

	return tolower(*(unsigned char *) string1) - tolower(*(unsigned char *) string2);
}
#endif


#ifdef F_strncasecmp
int	 strncasecmp(const char * string1, const char * string2, size_t n)
{
	if (n == 0)
		return 0;

	while ((n-- != 0) && (tolower(*string1) == tolower(*string2)))
    {
		if ((n == 0) || (*string1 == '\0') || (*string2 == '\0'))
			break;

		string1++;
		string2++;
	}

	return tolower(*(unsigned char *) string1) - tolower(*(unsigned char *) string2);
}
#endif

#ifdef F_strtok
char* strtok(char * strToken, const char * strDelimit)
{
	static char* start;
	static char* end;

	if (strToken != NULL)
		start = strToken;
	else
	{
		if (*end == 0)
			return 0;

		start=end;
	}

	// Strip out any leading delimiters
	while (strchr(strDelimit, *start))
	{
		// If a character from the delimiting string
		// then skip past it

		start++;

		if (*start == 0)
			return 0;
	}

	if (*start == 0)
		return 0;

	end=start;


	while (*end != 0)
	{
		if (strchr(strDelimit, *end))
		{
			// if we find a delimiting character
			// before the end of the string, then
			// terminate the token and move the end
			// pointer to the next character
			*end = 0;
			end++;
			return start;
		}
		
		end++;
	}


	// reached the end of the string before finding a delimiter
	// so dont move on to the next character
	return start;
}
#endif

#ifdef F_strrchr
char* strrchr(const char * string, int c)
{
	/* use the asm strchr to do strrchr */
	char* lastmatch;
	char* result;

	/* if char is never found then this will return 0 */
	/* if char is found then this will return the last matched location
	   before strchr returned 0 */

	lastmatch = 0;
	result = strchr(string,c);

	while ((int)result != 0)
	{
		lastmatch=result;
		result = strchr(lastmatch+1,c);
	}

	return lastmatch;
}
#endif

#ifdef F_strstr
char *	strstr(const char * string, const char * substring)
{
	char* strpos;

	if (string == 0)
		return 0;
	
	if (strlen(substring)==0)
		return (char*)string;

	strpos = (char*)string;

	while (*strpos != 0)
	{
		if (strncmp(strpos, substring, strlen(substring)) == 0)
		{
			return strpos;
		}

		strpos++;
	}

	return 0;
}
#endif


#ifdef F_strtol
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

#ifdef F_strupr
char * strupr(char *str)
{
	char * strptr = str;
	
	// loop thru each char in string
	while (*strptr != '\0')
    {
      // if char is lowercase, convert to uppercase
      if(islower(*strptr))
      	*strptr = toupper(*strptr);
      strptr++;
    }
	
	return str;
}
#endif

#ifdef F_strlwr
char * strlwr(char *str)
{
	char * strptr = str;
	
	// loop thru each char in string
	while (*strptr != '\0')
    {
      // if char is uppercase, convert to lowercase
      if(isupper(*strptr))
      	*strptr = tolower(*strptr);
      strptr++;
    }
	
	return str;
}
#endif


#ifdef F_ctype
int tolower(int c)
{
	if (isupper(c))
		c+=32;

	return c;
}

int toupper(int c)
{
	if (islower(c))
		c-=32;

	return c;
}


int isupper(int c)
{
	if (c < 'A')
		return 0;

	if (c > 'Z')
		return 0;

	// passed both criteria, so it
	// is an upper case alpha char
	return 1;
}

int islower(int c)
{
	if (c < 'a')
		return 0;

	if (c > 'z')
		return 0;

	// passed both criteria, so it
	// is a lower case alpha char
	return 1;
}

int isalpha(int c)
{
	if (islower(c) || isupper(c))
		return 1;

	return 0;
}

int isdigit(int c)
{
	if (c < '0')
		return 0;

	if (c > '9')
		return 0;

	// passed both criteria, so it
	// is a numerical char
	return 1;
}

int isalnum(int c)
{
	if (isalpha(c) || isdigit(c))
		return 1;

	return 0;
}

int iscntrl(int c)
{
	if (c < 0x20)
		return 1;

	if (c == 0x7F)
		return 1;

	return 0;
}


int isgraph(int c)
{
	if (iscntrl(c))
		return 0;

	if (isspace(c))
		return 0;

	return 1;
}


int isprint(int c)
{
	if (iscntrl(c))
		return 0;

	return 1;
}

int ispunct(int c)
{
	if (iscntrl(c))
		return 0;
	
	if (isalnum(c))
		return 0;

	if (isspace(c))
		return 0;

	// It's a printable character
	// thats not alpha-numeric, or a space
	// so its a punctuation character
	return 1;
}

int isspace(int c)
{
	if ((c>=0x09) && (c<=0x0D))
		return 1;

	if (c==0x20)
		return 1;

	return 0;
}



int isxdigit(int c)
{
	if (isdigit(c))
		return 1;

	if ((c>='a') && (c<='f'))
		return 1;

	if ((c>='A') && (c<='F'))
		return 1;

	return 0;
}
#endif

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
 * Standard wide-character string functions.
 */

#include <tamtypes.h>
#include <kernel.h>
#include <limits.h>
#include <malloc.h>
#include <wchar.h>

#ifdef F_wcsdup
wchar_t *wcsdup(const wchar_t* wcs)
{
	return wcscpy(malloc((wcslen(wcs)+1)*sizeof(wchar_t)), wcs);
}
#endif

#ifdef F_wcscasecmp
int wcscasecmp(const wchar_t* ws1, const wchar_t* ws2)
{
	while (*ws1 != '\0' && towlower(*ws1) == towlower(*ws2))
	{
		ws1++;
		ws2++;
	}

	return towlower(*ws1) - towlower(*ws2);
}
#endif

#ifdef F_wcsncasecmp
int wcsncasecmp(const wchar_t* ws1, const wchar_t* ws2, size_t n)
{
	if (n == 0)
		return 0;

	while ((n-- != 0) && (towlower(*ws1) == towlower(*ws2)))
	{
		if ((n == 0) || (*ws1 == '\0') || (*ws2 == '\0'))
			break;

		ws1++;
		ws2++;
	}

	return towlower(*ws1) - towlower(*ws2);
}
#endif

#ifdef F_wcstok
wchar_t* wcstok(wchar_t* wcs, const wchar_t* delimiters)
{
	static wchar_t* start;
	static wchar_t* end;

	if (wcs != NULL)
		start = wcs;
	else
	{
		if (*end == 0)
			return 0;

		start=end;
	}

	if(*start == 0)
	{
		return 0;
	}

	// Strip out any leading delimiters
	while (wcschr(delimiters, *start))
	{
		// If a character from the delimiting wcs
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
		if (wcschr(delimiters, *end))
		{
			// if we find a delimiting character
			// before the end of the wcs, then
			// terminate the token and move the end
			// pointer to the next character
			*end = 0;
			end++;
			return start;
		}

		end++;
	}


	// reached the end of the wcs before finding a delimiter
	// so dont move on to the next character
	return start;
}
#endif

#ifdef F_wcsrchr
wchar_t* wcsrchr(const wchar_t* ws, wint_t wc)
{
	/* use the asm wcschr to do wcsrchr */
	wchar_t* lastmatch;
	wchar_t* result;

	/*	If string is never found then this will return 0
		If string is found then this will return the last matched location before wcschr returned 0 */

	lastmatch = 0;
	result = wcschr(ws, wc);

	while (result != NULL)
	{
		lastmatch=result;
		result = wcschr(lastmatch+1, wc);
	}

	return lastmatch;
}
#endif

#ifdef F_wcswcs
wchar_t* wcswcs(const wchar_t* ws1, const wchar_t* ws2)
{
	wchar_t* wcspos;

	if (ws1 == 0)
		return 0;

	if (wcslen(ws2)==0)
		return (wchar_t*)ws1;

	wcspos = (wchar_t*)ws1;

	while (*wcspos != 0)
	{
		if (wcsncmp(wcspos, ws2, wcslen(ws2)) == 0)
		{
			return wcspos;
		}

		wcspos++;
	}

	return NULL;
}
#endif

#ifdef F_wcsupr
wchar_t* wcsupr(wchar_t* wcs)
{
	wchar_t* wcsptr = wcs;

	// loop thru each wchar_t in wcs
	while (*wcsptr != '\0')
	{
		// If character is lowercase, convert to uppercase
		if(iswlower(*wcsptr)) *wcsptr = towupper(*wcsptr);
		wcsptr++;
	}

	return wcs;
}
#endif

#ifdef F_wcslwr
wchar_t* wcslwr(wchar_t* wcs)
{
	wchar_t* wcsptr = wcs;

	// loop thru each wchar_t in wcs
	while (*wcsptr != '\0')
	{
		// If character is uppercase, convert to lowercase
		if(iswupper(*wcsptr)) *wcsptr = towlower(*wcsptr);
		wcsptr++;
	}

	return wcs;
}
#endif

#ifdef F_towlower
wint_t towlower(wint_t wc)
{
	if (iswupper(wc)) wc+=32;

	return wc;
}
#endif

#ifdef F_towupper
wint_t towupper(wint_t wc)
{
	if (iswlower(wc)) wc-=32;

	return wc;
}
#endif

#ifdef F_iswupper
int iswupper(wint_t wc)
{
	if (wc < 'A' || wc > 'Z')
		return 0;

	// passed both criteria, so it
	// is an upper case alpha char
	return 1;
}
#endif

#ifdef F_iswlower
int iswlower(wint_t wc)
{
	if (wc < 'a' || wc > 'z')
		return 0;

	// passed both criteria, so it
	// is a lower case alpha char
	return 1;
}
#endif

#ifdef F_iswalpha
int iswalpha(wint_t wc)
{
	return((iswlower(wc) || iswupper(wc))?1:0);
}
#endif

#ifdef F_iswdigit
int iswdigit(wint_t wc)
{
	if (wc < '0' || wc > '9')
		return 0;

	// passed both criteria, so it
	// is a numerical char
	return 1;
}
#endif

#ifdef F_iswalnum
int iswalnum(wint_t wc)
{
	return((iswalpha(wc) || iswdigit(wc))?1:0);
}
#endif

#ifdef F_iswcntrl
int iswcntrl(wint_t wc)
{
	return((wc < 0x20 || wc == 0x7F)?1:0);
}
#endif

#ifdef F_iswgraph
int iswgraph(wint_t wc)
{
	return((iswcntrl(wc) || iswspace(wc))?0:1);
}
#endif

#ifdef F_iswprint
int iswprint(wint_t wc)
{
	return(iswcntrl(wc)?0:1);
}
#endif

#ifdef F_iswpunct
int iswpunct(wint_t wc)
{
	if (iswcntrl(wc) || iswalnum(wc) || iswspace(wc))
		return 0;

	// It's a printable character
	// thats not alpha-numeric, or a space
	// so its a punctuation character
	return 1;
}
#endif

#ifdef F_iswspace
int iswspace(wint_t wc)
{
	return(((wc>=0x09 && wc<=0x0D) || wc==0x20)?1:0);
}
#endif

#ifdef F_iswxdigit
int iswxdigit(wint_t wc)
{
	return((iswdigit(wc) || (wc>='a' && wc<='f') || (wc>='A' && wc<='F'))?1:0);
}
#endif

#ifdef F_wcscpy
wchar_t* wcscpy(wchar_t* destination, const wchar_t* source)
{
	wchar_t *ptr;

	ptr=destination;
	while(*source!='\0'){
		*ptr=*source;
		ptr++;
		source++;
	}
	*ptr='\0';

	return destination;
}
#endif

#ifdef F_wcsncpy
wchar_t* wcsncpy(wchar_t* destination, const wchar_t* source, size_t num)
{
	wchar_t *ptr;
	size_t i;

	ptr=destination;
	for(i=0; *source!='\0' && i<num; i++){
		*ptr=*source;
		ptr++;
		source++;
	}
	*ptr='\0';

	return destination;
}
#endif

#ifdef F_wcspbrk
wchar_t* wcspbrk(const wchar_t* wcs1, const wchar_t* wcs2)
{
    const wchar_t *needle;
    for (; *wcs1; wcs1++) {
        for (needle = wcs2; *needle; needle++) {
            if (*wcs1 == *needle)
                return (wchar_t *)wcs1;
        }
    }

    return NULL;
}
#endif

#ifdef F_wcsspn
size_t wcsspn(const wchar_t* ws1, const wchar_t* ws2)
{
    const wchar_t *c;

    for (c = ws1; *ws1; c++) {
        if (!wcschr(ws2, *c))
            return c - ws1;
    }

    return c - ws1;
}
#endif

#ifdef F_wcscspn
size_t wcscspn(const wchar_t* wcs1, const wchar_t* wcs2)
{
    const wchar_t *c;

    for (c = wcs1; *c; c++) {
        if (wcschr(wcs2, *c))
            return c - wcs1;
    }

    return c - wcs1;
}
#endif


#ifdef F_wcscmp
int wcscmp(const wchar_t *s1, const wchar_t *s2)
{
	const wchar_t *us1 = (const wchar_t *)s1;
	const wchar_t *us2 = (const wchar_t *)s2;
	int c1a, c1b;

	do
	{
		c1a = *us1++;
		c1b = *us2++;
	}
	while (c1a != '\0' && c1a == c1b);

	return c1a - c1b;
}
#endif

#ifdef F_wcsncmp
int wcsncmp(const wchar_t *s1, const wchar_t *s2, int len)
{
	const wchar_t *us1 = (const wchar_t *)s1;
	const wchar_t *us2 = (const wchar_t *)s2;
	int c1a, c1b, i;

	i=0;
	do
	{
		c1a = *us1++;
		c1b = *us2++;
	}
	while (c1a != '\0' && c1a == c1b && ++i<len);

	return c1a - c1b;
}
#endif

#ifdef F_wcslen
int wcslen(const wchar_t *string){
	int i;

	for(i=0; string[i]!='\0'; i++);

	return i;
}
#endif

#ifdef F_wcschr
wchar_t *wcschr(const wchar_t *string, wint_t character)
{
	int i;
	wchar_t *result;

	for(result=NULL,i=0; string[i]!='\0'; i++){
		if(string[i]==character){
			result=(wchar_t*)&string[i];
			break;
		}
	}

	return result;
}
#endif

#ifdef F_wcscat
wchar_t* wcscat(wchar_t* destination, const wchar_t* source)
{
	wchar_t *ptr;

	ptr=&destination[wcslen(destination)];
	while(*source!='\0'){
		*ptr=*source;
		ptr++;
		source++;
	}
	*ptr='\0';

	return destination;
}
#endif

#ifdef F_wcsncat
wchar_t* wcsncat(wchar_t* destination, const wchar_t* source, size_t num)
{
	wchar_t *ptr;
	size_t i;

	ptr=&destination[wcslen(destination)];
	for(i=0; *source!='\0' && i<num; i++){
		*ptr=*source;
		ptr++;
		source++;
	}
	*ptr='\0';

	return destination;
}
#endif

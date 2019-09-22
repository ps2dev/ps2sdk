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
 * sjis<->ascii conversion routines by Peter Sandström
 */

#include <string.h>
#include <sjis.h>

struct charmap_t {
	unsigned short sjis;
	unsigned char ascii;
};

#ifdef F__sjis_internals
struct charmap_t sjis_conversion[] = {
    { 0x4081, ' ' },
    { 0x6d81, '[' },
    { 0x6e81, ']' },
    { 0x7c81, '-' },
    { 0x5b81, '°' },
    { 0x4581, '¥' },
    { 0x4481, '.' },
    { 0x7B81, '+' },
    { 0x9681, '*' },
    { 0x5E81, '/' },
    { 0x4981, '!' },
    { 0x6881, '"' },
    { 0x9481, '#' },
    { 0x9081, '$' },
    { 0x9381, '%' },
    { 0x9581, '&' },
    { 0x6681, '\'' },
    { 0x6981, '(' },
    { 0x6a81, ')' },
    { 0x8181, '=' },
    { 0x6281, '|' },
    { 0x8f81, '\\' },
    { 0x4881, '?' },
    { 0x5181, '_' },
    { 0x6f81, '{' },
    { 0x7081, '}' },
    { 0x9781, '@' },
    { 0x4781, ';' },
    { 0x4681, ':' },
    { 0x8381, '<' },
    { 0x8481, '>' },
    { 0x4d81, '`' },
    { 0, 0 }
};

unsigned char isSpecialSJIS(short sjis)
{
    struct charmap_t *s = &sjis_conversion[0];
    do {
	if (s->sjis == sjis) return s->ascii;
 	s++;
    } while (s->sjis != 0);
    return 0;
}

short isSpecialASCII(unsigned char ascii)
{
    struct charmap_t *s = &sjis_conversion[0];
    do {
	if (s->ascii == ascii) return s->sjis;
 	s++;
    } while (s->ascii != 0);
    return 0;
}
#else
extern struct charmap_t * sjis_conversion;
unsigned char isSpecialSJIS(short sjis);
short isSpecialASCII(unsigned char ascii);
#endif

#ifdef F_strcpy_ascii
int strcpy_ascii(char* ascii_buff, const short* sjis_buff)
{
    int i;
    short ascii, sjis;

    int len = strlen((const char *)sjis_buff)/2;

    for (i=0;i<len;i++) {
	sjis = sjis_buff[i];
	if ((ascii = isSpecialSJIS(sjis)) != 0) {
	} else {
	    ascii = ((sjis & 0xFF00) >> 8) - 0x1f;
	    if (ascii>96) ascii--;
	}
	ascii_buff[i] = ascii;
    }
    ascii_buff[i]=0;
    return len;
}
#endif

#ifdef F_strcpy_sjis
int strcpy_sjis(short* sjis_buff, const char* ascii_buff)
{
    int i;
    short ascii, sjis;

    int len = strlen(ascii_buff);

    for (i=0;i<len;i++)	{
	ascii = ascii_buff[i];
	if ((sjis = isSpecialASCII(ascii)) != 0) {
	} else {
	    if (ascii>96) ascii++;
	    sjis = ((ascii + 0x1f) << 8) | 0x82;
	}
        sjis_buff[i] = sjis;
    }
    sjis_buff[i]=0;
    return len;
}
#endif


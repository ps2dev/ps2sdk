/*
# _____     ___ ____     ___ ____
#  ____|   |    ____|   |        | |____|
# |     ___|   |____ ___|    ____| |    \    PS2DEV Open Source Project.
#-----------------------------------------------------------------------
# Copyright 2001-2004, ps2dev - http://www.ps2dev.org
# Licenced under Academic Free License version 2.0
# Review ps2sdk README & LICENSE files for further details.
*/

#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <ctype.h>
#include <sys/param.h>
#include <dirent.h>
#include <errno.h>

#ifdef F___cwd
/* the present working directory variable. */
char __cwd[MAXNAMLEN + 1] = { 0 };
#else
extern char __cwd[MAXNAMLEN + 1];
#endif

#ifdef F___cwd_len
/* the length of the present working directory variable. */
size_t __cwd_len = 0;
#else
extern size_t __cwd_len;
#endif

#define defaultCWD "host:"

enum SeparatorType {
	SeparatorTypeNone,
	SeparatorTypePOSIX,
	SeparatorTypeWindows
};

#define isnum(c) ((c) >= '0' && (c) <= '9')

#ifdef F___get_drive
/* Return the number of bytes taken up by the "drive:" prefix,
   or -1 if it's not found */
int __get_drive(const char *dev, enum SeparatorType *usePOSIXSeparator)
{
	const char *tail;
	const char *d;
	int devname_len;

	/* Skip leading spaces */
	d = dev;
	while (*d == ' ')
	{
		d += 1;
	}

	/* Get colon position */
	tail = strchr(d, ':');
	if (tail == NULL)
	{
		return -1;
	}

	devname_len = (int)(tail - d);

	/* Reduce length to not include index */
	while (isnum(d[devname_len - 1]))
	{
		devname_len -= 1;
	}

	*usePOSIXSeparator = SeparatorTypePOSIX;
	switch (devname_len)
	{
	case 3:
		/* These drivers don't have separator */
		if ((memcmp(d, "rom", devname_len) == 0) || (memcmp(d, "hdd", devname_len) == 0))
			*usePOSIXSeparator = SeparatorTypeNone;
		break;
	case 5:
		/* These drivers use \ as separator */
		if ((memcmp(d, "cdrom", devname_len) == 0))
			*usePOSIXSeparator = SeparatorTypeWindows;
		break;
	case 6:
		/* These drivers don't have separator */
		if ((memcmp(d, "usbkbd", devname_len) == 0))
			*usePOSIXSeparator = SeparatorTypeNone;
		break;
	default:
		break;
	}

	/* Return the length of the whole device name portion, including:
	 * - leading spaces
	 * - device name
	 * - device index
	 * - colon
	 */
	return (tail - dev) + 1;
}
#else 
int __get_drive(const char *dev, enum SeparatorType *usePOSIXSeparator);
#endif

#ifdef F_getcwd
char *getcwd(char *buf, size_t size)
{
	if(!buf) {
		errno = EINVAL;
		return NULL;
	}		

	if(__cwd_len >= size) {
		errno = ERANGE;
		return NULL;
	}

	memcpy(buf, __cwd, __cwd_len);
	buf[__cwd_len] = '\x00';
	return buf;
}
#endif

#ifdef F___path_absolute
/* Normalize a pathname (without leading "drive:") by removing
   . and .. components, duplicated /, etc. */
static int __path_normalize(char *out, int len, int posixSeparator)
{
	int i, j;
	int first, next;
	char separator = posixSeparator ? '/' : '\\';
	size_t out_len = strnlen(out, len);

	out_len += 2;
	if (out_len >= len) return -10;

	/* First append "/" to make the rest easier */
	out[out_len - 1] = separator;
	out[out_len] = 0;

	// Convert separators to the specified one
    for (size_t i = 0; i < out_len; i++) {
        if (out[i] == '/' || out[i] == '\\') {
            out[i] = separator;
        }
    }

	/* Convert "//" to "/" */
	for(i = 0; (i < out_len) && out[i+1]; i++) {
		if(out[i] == separator && out[i+1] == separator) {
			for(j=i+1; out[j]; j++)
				out[j] = out[j+1];
			i--;
		}
	}

	/* Convert "/./" to "/" */
	for(i = 0; (i < out_len) && out[i] && out[i+1] && out[i+2]; i++) {
		if(out[i] == separator && out[i+1] == '.' && out[i+2] == separator) {
			for(j = i+1; out[j]; j++)
				out[j] = out[j+2];
			i--;
		}
	}

	/* Convert "/asdf/../" to "/" until we can't anymore.  Also
	 * convert leading "/../" to "/" */
	first = next = 0;
	while((first < out_len) && (next < out_len)) {
		/* If a "../" follows, remove it and the parent */
		if(out[next+1] && out[next+1] == '.' && 
		   out[next+2] && out[next+2] == '.' &&
		   out[next+3] && out[next+3] == separator) {
			for(j=0; out[first+j+1]; j++)
				out[first+j+1] = out[next+j+4];
			first = next = 0;
			continue;
		}

		/* Find next slash */
		first = next;
		for(next= first+1; out[next] && out[next] != separator; next++)
			continue;
		if(!out[next]) break;
	}

	/* Remove trailing "/" just if it's not the root */
	for(i = 1; (i < out_len) && out[i]; i++)
		continue;
	if(i > 1 && out[i-1] == separator) 
		out[i-1] = 0;

	return 0;
}

/* Convert relative path to absolute path. */
int __path_absolute(const char *in, char *out, int len)
{
	int dr;
	enum SeparatorType separatorType;
	size_t in_len;

	in_len = strlen(in);
	/* See what the relative URL starts with */
	dr = __get_drive(in, &separatorType);
	char separator = separatorType == SeparatorTypePOSIX ? '/' : '\\';

	if(dr > 0 && (separatorType == SeparatorTypeNone || in[dr] == separator)) {
		/* It starts with "drive:" and has no separator or "drive:/", so it's already absolute */
		if (in_len >= len) return -1;
		strncpy(out, in, len);
	} else if(dr > 0 && in[dr - 1] == ':') {
		/* It starts with "drive:", so it's already absoulte, however it misses the "/" after unit */
		if (in_len + 1 >= len) return -2;
		strncpy(out, in, dr);
		out[dr] = separator;
		strncpy(out + dr + 1, in + dr, len - dr - 1);
	} else if(in[0] == '\\' || in[0] == '/') {
		/* It's absolute, but missing the drive, so use cwd's drive */
		if(__cwd_len + in_len >= len) return -3;
		memcpy(out, __cwd, __cwd_len);
		out[__cwd_len] = '\x00';
		dr = __get_drive(out, &separatorType);
		if(dr < 0) dr = 0;
		out[dr] = 0;
		strncat(out, in, len);
	} else {
		/* It's not absolute, so append it to the current cwd */
		if(__cwd_len + 1 + in_len >= len) return -5;
		memcpy(out, __cwd, __cwd_len);
		out[__cwd_len] = separator;
		out[__cwd_len + 1] = '\x00';
		strncat(out, in, len);
	}

	/* Now normalize the pathname portion */
	dr = __get_drive(out, &separatorType);
	if(dr < 0) dr = 0;
	return __path_normalize(out + dr, len - dr, separatorType == SeparatorTypePOSIX);
}
#endif

#ifdef F___init_cwd
/* Set the current working directory (CWD) to the path where the module was launched. */
void __init_cwd(int argc, char ** argv)
{
	if (argc == 0) {
		chdir("host:");
		return;
	}
    
	char * p, * s = NULL;
	// let's find the last slash, or at worst, the :
	for (p = argv[0]; *p; p++) {
	    if ((*p == '/') || (*p == '\\') || (*p == ':')) {
			s = p;
	    }
	}
	if (s == NULL)
	{
		chdir("host:");
		return;
	}
    char backup = *(++s);
	*s = 0;
	chdir(argv[0]);
	*s = backup;
}
#endif
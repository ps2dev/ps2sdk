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
#include <sys/param.h>
#include <dirent.h>
#include <errno.h>

#ifdef F___cwd
/* the present working directory variable. */
char __cwd[MAXNAMLEN + 1] = { 0 };
#else
extern char __cwd[MAXNAMLEN + 1];
#endif

#define defaultCWD "host:"

enum SeparatorType {
	SeparatorTypeNone,
	SeparatorTypePOSIX,
	SeparatorTypeWindows
};

#ifdef F___get_drive
/* Return the number of bytes taken up by the "drive:" prefix,
   or -1 if it's not found */
int __get_drive(const char *d, enum SeparatorType *usePOSIXSeparator)
{
	int i;
	for(i=0; d[i]; i++) {
		if(! ((d[i] >= 'a' && d[i] <= 'z') ||
			  (d[i] >= 'A' && d[i] <= 'Z') ||
		      (d[i] >= '0' && d[i] <= '9') ))
			break;
	}
	/* We need to check if driver is cdrom: cdrom0: ... cdrom9: because those one use \ as separator */
	if ((i >= 5 && strncmp(d, "cdrom", 5) == 0) &&
		((d[5] == ':') ||
		(d[5] >= '0' && d[5] <= '9' && d[6] == ':'))) {
		*usePOSIXSeparator = SeparatorTypeWindows;
	/* We need to check if drive is rom: rom0: ... rom9: because those one don't have separator */
	} else if ((i >= 3 && strncmp(d, "rom", 3) == 0) &&
		((d[3] == ':') ||
		(d[3] >= '0' && d[3] <= '9' && d[4] == ':'))) {
		*usePOSIXSeparator = SeparatorTypeNone;
	} else {
		*usePOSIXSeparator = 1;
	}

	if(d[i] == ':') return i+1;
	return -1;
}
#else 
int __get_drive(const char *d, enum SeparatorType *usePOSIXSeparator);
#endif

#ifdef F_getcwd
char *getcwd(char *buf, size_t size)
{
	if(!buf) {
		errno = EINVAL;
		return NULL;
	}		

	if(strlen(__cwd) >= size) {
		errno = ERANGE;
		return NULL;
	}

	strncpy(buf, __cwd, size);
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

	// Convert separators to the specified one
    for (size_t i = 0; i < len; i++) {
        if (out[i] == '/' || out[i] == '\\') {
            out[i] = separator;
        }
    }

	/* First append "/" to make the rest easier */
	if (strlen(out) + 1 >= len) return -10;
	out[strlen(out)] = separator;
	out[strlen(out)] = 0;


	/* Convert "//" to "/" */
	for(i = 0; out[i+1]; i++) {
		if(out[i] == separator && out[i+1] == separator) {
			for(j=i+1; out[j]; j++)
				out[j] = out[j+1];
			i--;
		}
	}

	/* Convert "/./" to "/" */
	for(i = 0; out[i] && out[i+1] && out[i+2]; i++) {
		if(out[i] == separator && out[i+1] == '.' && out[i+2] == separator) {
			for(j = i+1; out[j]; j++)
				out[j] = out[j+2];
			i--;
		}
	}

	/* Convert "/asdf/../" to "/" until we can't anymore.  Also
	 * convert leading "/../" to "/" */
	first = next = 0;
	while(1) {
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
	for(i = 1; out[i]; i++)
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

	/* See what the relative URL starts with */
	dr = __get_drive(in, &separatorType);
	char separator = separatorType == SeparatorTypePOSIX ? '/' : '\\';

	if(dr > 0 && (separatorType == SeparatorTypeNone || in[dr] == separator)) {
		/* It starts with "drive:" and has no separator or "drive:/", so it's already absolute */
		if (strlen(in) >= len) return -1;
		strncpy(out, in, len);
	} else if(dr > 0 && in[dr - 1] == ':') {
		/* It starts with "drive:", so it's already absoulte, however it misses the "/" after unit */
		if (strlen(in) + 1 >= len) return -2;
		strncpy(out, in, dr);
		out[dr] = separator;
		strncpy(out + dr + 1, in + dr, len - dr - 1);
	} else if(in[0] == '\\' || in[0] == '/') {
		/* It's absolute, but missing the drive, so use cwd's drive */
		if(strlen(__cwd) + strlen(in) >= len) return -3;
		strncpy(out, __cwd, len);
		dr = __get_drive(out, &separatorType);
		out[dr] = 0;
		strncat(out, in, len);
	} else {
		/* It's not absolute, so append it to the current cwd */
		if(strlen(__cwd) + 1 + strlen(in) >= len) return -5;
		strncpy(out, __cwd, len);
		strncat(out, &separator, 1); 
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
    
	char * p, * s = 0;
	// let's find the last slash, or at worst, the :
	for (p = argv[0]; *p; p++) {
	    if ((*p == '/') || (*p == '\\') || (*p == ':')) {
			s = p;
	    }
	}
    char backup = *(++s);
	*s = 0;
	chdir(argv[0]);
	*s = backup;
}
#endif
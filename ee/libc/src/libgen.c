/*
# _____     ___ ____     ___ ____
#  ____|   |    ____|   |        | |____|
# |     ___|   |____ ___|    ____| |    \    PS2DEV Open Source Project.
#-----------------------------------------------------------------------
# Copyright 2001-2005, ps2dev - http://www.ps2dev.org
# Licenced under Academic Free License version 2.0
# Review ps2sdk README & LICENSE files for further details.
#
# $Id: libgen.c $
# libgen implementation
*/
#include <string.h>
#include <libgen.h>

#ifndef PATH_MAX
#define PATH_MAX 255
#endif

#ifdef F_basename
const char *basename(const char *path)
{
	char *last_slash;

	last_slash = strrchr(path, '/');
	if (last_slash != NULL)
	{
		return (last_slash + 1);
	}
	else
	{
		return path;
	}
}
#endif

#ifdef F_dirname
const char *dirname(const char *path)
{
	static char result[PATH_MAX];
	char *last_slash;

	if (path == NULL || path[0] == '\0')
	{
		/* return local directory if no file specified */
		return ".";
	}

	last_slash = strrchr(path, '/');
	if (last_slash != NULL)
	{
		/* strip up until the last slash */
		int len = last_slash - path;
		if (len >= PATH_MAX)
		{ 
			/* not enough memory */
			return NULL;
		}

		memcpy(result, path, len);
		result[len] = '\0';
		return result;
	}

	/* no slashes at all */
	return ".";
}
#endif

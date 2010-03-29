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
char *basename(char *path)
{
    static char base[PATH_MAX];
    char *end;

    strcpy(base,".");

    if (path == NULL || *path == '\0')
        return base;

    if ((end = strrchr(path, '/')) != NULL);
    else if ((end = strrchr(path, '\\')) != NULL);
    else if ((end = strrchr(path, ':')) != NULL);
    else { strcpy(base,path); return base; }

    end++;

    if (*end != '\0')
        strcpy(base,end);

    return base;
}
#endif

#ifdef F_dirname
char *dirname(char *path)
{
    static char dir[PATH_MAX];
    char *end;

    strcpy(dir,".");

    if (path == NULL || *path == '\0')
        return dir;

    if ((end = strrchr(path, '/')) != NULL);
    else if ((end = strrchr(path, '\\')) != NULL);
    else if ((end = strrchr(path, ':')) != NULL);
    else return dir;    

    if (*end == ':') end++;

    memcpy(dir,path,end-path);
    dir[end-path] = '\0';

    return dir;
}
#endif

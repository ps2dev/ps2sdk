/*
# _____     ___ ____     ___ ____
#  ____|   |    ____|   |        | |____|
# |     ___|   |____ ___|    ____| |    \    PS2DEV Open Source Project.
#-----------------------------------------------------------------------
# Copyright 2001-2005, ps2dev - http://www.ps2dev.org
# Licenced under Academic Free License version 2.0
# Review ps2sdk README & LICENSE files for further details.
#
# $Id: libgen.h $
# basename and dirname
*/
#ifndef __LIBGEN_H__
#define __LIBGEN_H__

#ifdef __cplusplus
extern "C" {
#endif

/** Extract the base portion of a pathname
    @param path
    @return last component of path
*/
char *basename (char *path);

/** Extract the directory portion of a pathname
    @param path
    @return directory location of file
*/
char *dirname (char *path);

#ifdef __cplusplus
}
#endif

#endif //LIBGEN

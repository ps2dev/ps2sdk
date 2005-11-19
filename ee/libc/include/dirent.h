/*
# _____     ___ ____     ___ ____
#  ____|   |    ____|   |        | |____|
# |     ___|   |____ ___|    ____| |    \    PS2DEV Open Source Project.
#-----------------------------------------------------------------------
# Copyright 2001-2005, ps2dev - http://www.ps2dev.org
# Licenced under Academic Free License version 2.0
# Review ps2sdk README & LICENSE files for further details.
#
# $Id: dirent.h $
# dirent
*/
#ifndef __DIRENT_H__
#define __DIRENT_H__

struct dirent
{
	/** relative filename */
	char d_name[256];
};

typedef struct DIR
{
	/** handle used against fio */
	int  d_fd;

	/** entry returned at readdir */
	struct dirent *d_entry;
} DIR;

#ifdef __cplusplus
extern "C" {
#endif

/** Open a directory
    @param path
    @return DIR struct to be used for rest of dirent functions
*/
DIR *opendir (const char *path);

/** Reads an entry from handle opened previously by opendir
    @param d
    @return
*/
struct dirent *readdir (DIR *d);

/** Rewinds
    @param d
*/
void rewinddir (DIR *d);

/** Release DIR handle
    @param d
    @return Zero on sucess
*/
int closedir (DIR *d);

#ifdef __cplusplus
}
#endif

#endif //DIRENT

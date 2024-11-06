/*
# _____     ___ ____     ___ ____
#  ____|   |    ____|   |        | |____|
# |     ___|   |____ ___|    ____| |    \    PS2DEV Open Source Project.
#-----------------------------------------------------------------------
# Copyright 2001-2004, ps2dev - http://www.ps2dev.org
# Licenced under Academic Free License version 2.0
# Review ps2sdk README & LICENSE files for further details.
*/

#ifndef _FDMAN_H_

#include <sys/types.h>
#include <ps2sdkapi.h>

#define _FDMAN_H_

#define __FILENO_MAX 64

#define __IS_FD_VALID(FD) \
		( (FD >= 0) && (FD < __FILENO_MAX) && (__descriptormap[FD] != NULL) )

typedef struct {
	uint32_t flags;
	uint32_t ref_count;
	_libcglue_fdman_fd_info_t info;
} __descriptormap_type;
	
extern __descriptormap_type *__descriptormap[__FILENO_MAX];

void __fdman_init();
void __fdman_deinit();
int  __fdman_get_new_descriptor();
int  __fdman_get_dup_descriptor(int fd);
int __fdman_get_dup2_descriptor(int fd, int newfd);
void __fdman_release_descriptor(int fd);

#endif

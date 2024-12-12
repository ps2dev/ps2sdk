/*
# _____     ___ ____     ___ ____
#  ____|   |    ____|   |        | |____|
# |     ___|   |____ ___|    ____| |    \    PS2DEV Open Source Project.
#-----------------------------------------------------------------------
# Copyright ps2dev - http://www.ps2dev.org
# Licenced under Academic Free License version 2.0
# Review ps2sdk README & LICENSE files for further details.
*/

#ifndef _SYS_UIO_H
#define _SYS_UIO_H

// libsmb2 defines its own
#if !defined(NEED_READV) && !defined(NEED_WRITEV)
#include <sys/types.h>

__BEGIN_DECLS

/*
 * Per POSIX
 */

struct iovec {
	void   *iov_base;
	size_t  iov_len;
};

extern ssize_t readv(int fd, const struct iovec *iov, int iovcnt);
extern ssize_t writev(int fd, const struct iovec *iov, int iovcnt);

__END_DECLS
#endif

#endif

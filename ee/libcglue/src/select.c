/*
# _____     ___ ____     ___ ____
#  ____|   |    ____|   |        | |____|
# |     ___|   |____ ___|    ____| |    \    PS2DEV Open Source Project.
#-----------------------------------------------------------------------
# Copyright ps2dev - http://www.ps2dev.org
# Licenced under Academic Free License version 2.0
# Review ps2sdk README & LICENSE files for further details.
*/

#define LIBCGLUE_SYS_SOCKET_NO_ALIASES
#define LIBCGLUE_ARPA_INET_NO_ALIASES
#include <sys/socket.h>
#include <ps2sdkapi.h>
#include <errno.h>

#include "ps2sdkapi.h"

#ifdef F_select
/* FIXME: This function currently cannot handle heterogeneous fds */
int	select(int n, fd_set *readfds, fd_set *writefds, fd_set *exceptfds, struct timeval *timeout)
{
	int fd;
	int ret;
	fd_set ready_readfds, ready_writefds, ready_exceptfds;
	fd_set iop_readfds, iop_writefds, iop_exceptfds;

	if (_libcglue_fdman_socket_ops == NULL || _libcglue_fdman_socket_ops->select == NULL)
	{
		errno = ENOSYS;
		return -1;
	}

	FD_ZERO(&ready_readfds);
	FD_ZERO(&ready_writefds);
	FD_ZERO(&ready_exceptfds);

	FD_ZERO(&iop_readfds);
	FD_ZERO(&iop_writefds);
	FD_ZERO(&iop_exceptfds);

	for (fd = 0; fd < n; fd += 1)
	{
		int iop_fd;
		iop_fd = ps2sdk_get_iop_fd(fd);
		if (iop_fd < 0)
		{
			errno = EBADF;
			return -1;
		}
		if (readfds && FD_ISSET(fd, readfds))
		{
			FD_SET(iop_fd, &iop_readfds);
		}
		if (writefds && FD_ISSET(fd, writefds))
		{
			FD_SET(iop_fd, &iop_writefds);
		}
		if (exceptfds && FD_ISSET(fd, exceptfds))
		{
			FD_SET(iop_fd, &iop_exceptfds);
		}
	}

	ret = _libcglue_fdman_socket_ops->select(n, &iop_readfds, &iop_writefds, &iop_exceptfds, timeout);

	for (fd = 0; fd < n; fd += 1)
	{
		int iop_fd;
		iop_fd = ps2sdk_get_iop_fd(fd);
		if (FD_ISSET(iop_fd, &iop_readfds))
		{
			FD_SET(fd, &ready_readfds);
		}
		if (FD_ISSET(iop_fd, &iop_writefds))
		{
			FD_SET(fd, &ready_writefds);
		}
		if (FD_ISSET(iop_fd, &iop_exceptfds))
		{
			FD_SET(fd, &ready_exceptfds);
		}
	}

	if (readfds)
	{
		*readfds = ready_readfds;
	}
	if (writefds)
	{
		*writefds = ready_writefds;
	}
	if (exceptfds)
	{
		*exceptfds = ready_exceptfds;
	}

	return ret;
}
#endif

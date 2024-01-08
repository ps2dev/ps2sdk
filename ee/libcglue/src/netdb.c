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
#include <ps2sdkapi.h>
#include <netdb.h>

#include "ps2sdkapi.h"

#ifdef F_gethostbyname
struct hostent *gethostbyname(const char *name)
{
	if (_libcglue_fdman_socket_ops == NULL || _libcglue_fdman_socket_ops->gethostbyname == NULL)
	{
		return NULL;
	}

	return _libcglue_fdman_socket_ops->gethostbyname(name);
}
#endif

#ifdef F_gethostbyname_r
int gethostbyname_r(const char *name, struct hostent *ret, char *buf, size_t buflen, struct hostent **result, int *h_errnop)
{
	if (_libcglue_fdman_socket_ops == NULL || _libcglue_fdman_socket_ops->gethostbyname_r == NULL)
	{
		if (h_errnop != NULL)
		{
			*h_errnop = NO_RECOVERY;
		}
		return -1;
	}

	return _libcglue_fdman_socket_ops->gethostbyname_r(name, ret, buf, buflen, result, h_errnop);
}
#endif

#ifdef F_freeaddrinfo
void freeaddrinfo(struct addrinfo *ai)
{
	if (_libcglue_fdman_socket_ops == NULL || _libcglue_fdman_socket_ops->freeaddrinfo == NULL)
	{
		return;
	}

	return _libcglue_fdman_socket_ops->freeaddrinfo(ai);
}
#endif

#ifdef F_getaddrinfo
int getaddrinfo(const char *nodename, const char *servname, const struct addrinfo *hints, struct addrinfo **res)
{
	if (_libcglue_fdman_socket_ops == NULL || _libcglue_fdman_socket_ops->getaddrinfo == NULL)
	{
		return EAI_FAIL;
	}

	return _libcglue_fdman_socket_ops->getaddrinfo(nodename, servname, hints, res);
}
#endif

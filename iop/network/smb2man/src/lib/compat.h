/* -*-  mode:c; tab-width:8; c-basic-offset:8; indent-tabs-mode:nil;  -*- */
/*
   Copyright (C) 2020 by Ronnie Sahlberg <ronniesahlberg@gmail.com>

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU Lesser General Public License as published by
   the Free Software Foundation; either version 2.1 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public License
   along with this program; if not, see <http://www.gnu.org/licenses/>.
*/

#ifndef _COMPAT_H_
#define _COMPAT_H_

#ifdef _XBOX

/* XBOX Defs begin */
#include <xtl.h>
#include <winsockx.h>

#ifdef XBOX_PLATFORM /* MSVC 2003 Doesn´t have stdint.h header */
typedef char int8_t;
typedef short int16_t;
typedef short int_least16_t;
typedef int int32_t;
typedef long long int64_t;
typedef int intptr_t;

typedef unsigned char uint8_t;
typedef unsigned short uint16_t;
typedef unsigned int uint32_t;
typedef unsigned long long uint64_t;
typedef unsigned int uint_t;
typedef unsigned int uintptr_t;
#endif

#include <errno.h>

#ifndef ENETRESET
#define ENETRESET WSAENETRESET
#endif

#ifndef ECONNREFUSED
#define ECONNREFUSED WSAECONNREFUSED
#endif

#ifndef ETIMEDOUT
#define ETIMEDOUT WSAETIMEDOUT
#endif

#ifndef ECONNRESET
#define ECONNRESET WSAECONNRESET
#endif

#ifndef ENODATA
#define ENODATA WSANO_DATA
#endif

#ifndef ETXTBSY 
#define ETXTBSY         139
#endif

#ifndef ENOLINK
#define ENOLINK         121
#endif

#ifndef EWOULDBLOCK 
#define EWOULDBLOCK     WSAEWOULDBLOCK
#endif

#define snprintf _snprintf

#define EAI_AGAIN EAGAIN
#define EAI_FAIL        4
#define EAI_MEMORY      6
#define EAI_NONAME      8
#define EAI_SERVICE     9

typedef int socklen_t;

#ifndef POLLIN
#define POLLIN      0x0001    /* There is data to read */
#endif
#ifndef POLLPRI
#define POLLPRI     0x0002    /* There is urgent data to read */
#endif
#ifndef POLLOUT
#define POLLOUT     0x0004    /* Writing now will not block */
#endif
#ifndef POLLERR
#define POLLERR     0x0008    /* Error condition */
#endif
#ifndef POLLHUP
#define POLLHUP     0x0010    /* Hung up */
#endif

#ifndef SO_ERROR
#define SO_ERROR 0x1007
#endif

struct sockaddr_storage {
#ifdef HAVE_SOCKADDR_SA_LEN
	unsigned char ss_len;
#endif /* HAVE_SOCKADDR_SA_LEN */
	unsigned char ss_family;
	unsigned char fill[127];
};

struct addrinfo {
	int	ai_flags;	/* AI_PASSIVE, AI_CANONNAME */
	int	ai_family;	/* PF_xxx */
	int	ai_socktype;	/* SOCK_xxx */
	int	ai_protocol;	/* 0 or IPPROTO_xxx for IPv4 and IPv6 */
	size_t	ai_addrlen;	/* length of ai_addr */
	char	*ai_canonname;	/* canonical name for hostname */
	struct sockaddr *ai_addr;	/* binary address */
	struct addrinfo *ai_next;	/* next structure in linked list */
};

/* XBOX Defs end */
struct pollfd {
        int fd;
        short events;
        short revents;
};

#define SOL_TCP 6

#define inline __inline 

int poll(struct pollfd *fds, unsigned int nfds, int timo);

int smb2_getaddrinfo(const char *node, const char*service,
                const struct addrinfo *hints,
                struct addrinfo **res);
void smb2_freeaddrinfo(struct addrinfo *res);

#define getaddrinfo smb2_getaddrinfo
#define freeaddrinfo smb2_freeaddrinfo

void srandom(unsigned int seed);
int random(void);

/* just pretend they are the same so we compile */
#define sockaddr_in6 sockaddr_in

int getlogin_r(char *buf, size_t size);

int getpid();

#endif /* _XBOX */

#if defined(_MSC_VER) && defined(_WINDOWS)
void srandom(unsigned int seed);
int random(void);
#include <stddef.h>
int getlogin_r(char *buf, size_t size);	
int getpid();
#endif /* _MSC_VER */

#ifdef PICO_PLATFORM

#include "lwip/netdb.h"
#include "lwip/sockets.h"

#define EAI_AGAIN EAGAIN
long long int be64toh(long long int x);
int getlogin_r(char *buf, size_t size);

#endif /* PICO_PLATFORM */

#ifdef DC_KOS_PLATFORM

#include <netdb.h>
#include <netinet/in.h>
#include <stdlib.h>
#include <string.h>
#include <sys/errno.h>
#include <unistd.h>

#define TCP_NODELAY     1  /* Don't delay send to coalesce packets  */
#define SOL_TCP IPPROTO_TCP

ssize_t writev(int fd, const struct iovec *iov, int iovcnt);
ssize_t readv(int fd, const struct iovec *iov, int iovcnt);

int getlogin_r(char *buf, size_t size);

#endif /* DC_KOS_PLATFORM */

#ifdef PS2_EE_PLATFORM

#include <unistd.h>

int getlogin_r(char *buf, size_t size);

#define POLLIN      0x0001    /* There is data to read */
#define POLLPRI     0x0002    /* There is urgent data to read */
#define POLLOUT     0x0004    /* Writing now will not block */
#define POLLERR     0x0008    /* Error condition */
#define POLLHUP     0x0010    /* Hung up */

struct pollfd {
        int fd;
        short events;
        short revents;
};

int poll(struct pollfd *fds, unsigned int nfds, int timo);

struct iovec {
  void  *iov_base;
  size_t iov_len;
};

ssize_t writev(int fd, const struct iovec *iov, int iovcnt);
ssize_t readv(int fd, const struct iovec *iov, int iovcnt);

long long int be64toh(long long int x);

#define SOL_TCP IPPROTO_TCP
#define EAI_AGAIN EAGAIN

/* just pretend they are the same so we compile */
#define sockaddr_in6 sockaddr_in

#endif /* PS2_EE_PLATFORM */

#ifdef PS2_IOP_PLATFORM
#ifndef PS2SDK_IOP
#include <alloc.h>
#endif
#include <stdint.h>
#include <ps2ip.h>
#include <loadcore.h>
#ifdef PS2SDK_IOP
#include "sysclib.h"
#include "sysmem.h"
#include "intrman.h"
#endif
typedef uint32_t UWORD32;
typedef size_t ssize_t;

long long int be64toh(long long int x);
char *strdup(const char *s);

int random(void);
void srandom(unsigned int seed);
time_t time(time_t *tloc);
int asprintf(char **strp, const char *fmt, ...);

int getlogin_r(char *buf, size_t size);
int getpid();

#define close(x) lwip_close(x)
#define snprintf(format, n, ...) sprintf(format, __VA_ARGS__)
#define fcntl(a,b,c) lwip_fcntl(a,b,c)

#define POLLIN      0x0001    /* There is data to read */
#define POLLPRI     0x0002    /* There is urgent data to read */
#define POLLOUT     0x0004    /* Writing now will not block */
#define POLLERR     0x0008    /* Error condition */
#define POLLHUP     0x0010    /* Hung up */

struct pollfd {
        int fd;
        short events;
        short revents;
};

int poll(struct pollfd *fds, unsigned int nfds, int timo);

struct iovec {
  void  *iov_base;
  size_t iov_len;
};

#undef connect
#define connect(a,b,c) iop_connect(a,b,c)
int iop_connect(int sockfd, struct sockaddr *addr, socklen_t addrlen);

#define write(a,b,c) lwip_send(a,b,c,MSG_DONTWAIT)
#define read(a,b,c) lwip_recv(a,b,c,MSG_DONTWAIT)

ssize_t writev(int fd, const struct iovec *iov, int iovcnt);
ssize_t readv(int fd, const struct iovec *iov, int iovcnt);
#ifdef PS2SDK_IOP
void *malloc(int size);

void free(void *ptr);

void *calloc(size_t nmemb, size_t size);
#endif

#define SOL_TCP IPPROTO_TCP
#define EAI_AGAIN EAGAIN

#define strerror(x) "Unknown"

/* just pretend they are the same so we compile */
#define sockaddr_in6 sockaddr_in

#endif /* PS2_IOP_PLATFORM */

#ifdef PS3_PPU_PLATFORM

#include <sys/time.h>
#include <netdb.h>
#include <net/poll.h>

int getlogin_r(char *buf, size_t size);
void srandom(unsigned int seed);
int random(void);
#define getaddrinfo smb2_getaddrinfo
#define freeaddrinfo smb2_freeaddrinfo

#define TCP_NODELAY     1  /* Don't delay send to coalesce packets  */

#define EAI_FAIL        4
#define EAI_MEMORY      6
#define EAI_NONAME      8
#define EAI_SERVICE     9

int smb2_getaddrinfo(const char *node, const char*service,
                const struct addrinfo *hints,
                struct addrinfo **res);
void smb2_freeaddrinfo(struct addrinfo *res);

ssize_t writev(int fd, const struct iovec *iov, int iovcnt);
ssize_t readv(int fd, const struct iovec *iov, int iovcnt);

#define SOL_TCP IPPROTO_TCP
#define EAI_AGAIN EAGAIN

#if !defined(HAVE_SOCKADDR_STORAGE)
/*
 * RFC 2553: protocol-independent placeholder for socket addresses
 */
#define _SS_MAXSIZE	128
#define _SS_ALIGNSIZE	(sizeof(double))
#define _SS_PAD1SIZE	(_SS_ALIGNSIZE - sizeof(unsigned char) * 2)
#define _SS_PAD2SIZE	(_SS_MAXSIZE - sizeof(unsigned char) * 2 - \
				_SS_PAD1SIZE - _SS_ALIGNSIZE)

struct sockaddr_storage {
#ifdef HAVE_SOCKADDR_LEN
	unsigned char ss_len;		/* address length */
	unsigned char ss_family;	/* address family */
#else
	unsigned short ss_family;
#endif
	char	__ss_pad1[_SS_PAD1SIZE];
	double	__ss_align;	/* force desired structure storage alignment */
	char	__ss_pad2[_SS_PAD2SIZE];
};
#endif


/* just pretend they are the same so we compile */
#define sockaddr_in6 sockaddr_in

#endif

#ifdef PS4_PLATFORM

#include <netdb.h>
#include <poll.h>
#include <sys/uio.h>

#define TCP_NODELAY     1  /* Don't delay send to coalesce packets  */

#endif /* PS4_PLATFORM */

#ifdef ESP_PLATFORM
#include <stddef.h>
void srandom(unsigned int seed);
int random(void);
int getlogin_r(char *buf, size_t size);
#endif

#ifdef __ANDROID__
#include <stddef.h>
/* getlogin_r() was added in API 28 */
#if __ANDROID_API__ < 28
int getlogin_r(char *buf, size_t size);
#endif
#endif /* __ANDROID__ */

#endif /* _COMPAT_H_ */

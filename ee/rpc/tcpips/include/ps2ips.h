/*
# _____     ___ ____     ___ ____
#  ____|   |    ____|   |        | |____|
# |     ___|   |____ ___|    ____| |    \    PS2DEV Open Source Project.
#-----------------------------------------------------------------------
# Copyright 2001-2004, ps2dev - http://www.ps2dev.org
# Licenced under Academic Free License version 2.0
# Review ps2sdk README & LICENSE files for further details.
#
# $Id$
*/

#ifndef _EE_PS2IP_H
#define _EE_PS2IP_H

#include "tcpip.h"

#ifdef __cplusplus
extern "C" {
#endif

int ps2ip_init();
int accept(int s, struct sockaddr *addr, int *addrlen);
int bind(int s, struct sockaddr *name, int namelen);
int disconnect(int s);
int connect(int s, struct sockaddr *name, int namelen);
int listen(int s, int backlog);
int recv(int s, void *mem, int len, unsigned int flags);
int recvfrom(int s, void *mem, int len, unsigned int flags, struct sockaddr *from, int *fromlen);
int send(int s, void *dataptr, int size, unsigned int flags);
int sendto(int s, void *dataptr, int size, unsigned int flags, struct sockaddr *to, int tolen);
int socket(int domain, int type, int protocol);
int ps2ip_setconfig(t_ip_info *ip_info);
int ps2ip_getconfig(char *netif_name, t_ip_info *ip_info);
int select(int maxfdp1, struct fd_set *readset, struct fd_set *writeset, struct fd_set *exceptset, struct timeval *timeout);
int ioctlsocket(int s, long cmd, void *argp);
int getsockname(int s, struct sockaddr* name, int* namelen);
int getpeername(int s, struct sockaddr *name, int *namelen);
int getsockopt(int s, int level, int optname, void* optval, socklen_t* optlen);
int setsockopt(int s, int level, int optname, const void *optval, socklen_t optlen);
int gethostbyname(char *name, struct in_addr *ip);
int ps2ip_dnslookup(char *name, struct in_addr *ip);

#ifdef __cplusplus
}
#endif

#endif /* _EE_PS2IP_H */

/*
# _____     ___ ____     ___ ____
#  ____|   |    ____|   |        | |____|
# |     ___|   |____ ___|    ____| |    \    PS2DEV Open Source Project.
#-----------------------------------------------------------------------
# Copyright 2001-2004, ps2dev - http://www.ps2dev.org
# Licenced under Academic Free License version 2.0
# Review ps2sdk README & LICENSE files for further details.
*/

/**
 * @file
 * PS2IP library.
 */

#ifndef __PS2IPS_H__
#define __PS2IPS_H__

#include <tcpip.h>
#include <sys/time.h>

#ifdef __cplusplus
extern "C" {
#endif

int ps2ip_init(void);
void ps2ip_deinit(void);
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
struct hostent *gethostbyname(const char *name);
void dns_setserver(u8 numdns, ip_addr_t *dnsserver);
const ip_addr_t *dns_getserver(u8 numdns);

#ifdef __cplusplus
}
#endif

#endif /* __PS2IPS_H__ */

/*
# _____     ___ ____     ___ ____
#  ____|   |    ____|   |        | |____|
# |     ___|   |____ ___|    ____| |    \    PS2DEV Open Source Project.
#-----------------------------------------------------------------------
# Copyright ps2dev - http://www.ps2dev.org
# Licenced under Academic Free License version 2.0
# Review ps2sdk README & LICENSE files for further details.
*/

#ifndef __PS2IPC_H__
#define __PS2IPC_H__

#include <ps2ips.h>

#ifdef __cplusplus
extern "C" {
#endif

int ps2ipc_accept(int s, struct sockaddr *addr, int *addrlen);
int ps2ipc_bind(int s, const struct sockaddr *name, int namelen);
int ps2ipc_disconnect(int s);
int ps2ipc_connect(int s, const struct sockaddr *name, int namelen);
int ps2ipc_listen(int s, int backlog);
int ps2ipc_recv(int s, void *mem, int len, unsigned int flags);
int ps2ipc_recvfrom(int s, void *mem, int len, unsigned int flags, struct sockaddr *from, int *fromlen);
int ps2ipc_send(int s, const void *dataptr, int size, unsigned int flags);
int ps2ipc_sendto(int s, const void *dataptr, int size, unsigned int flags, const struct sockaddr *to, int tolen);
int ps2ipc_socket(int domain, int type, int protocol);
int ps2ipc_ps2ip_setconfig(const t_ip_info *ip_info);
int ps2ipc_ps2ip_getconfig(char *netif_name, t_ip_info *ip_info);
int ps2ipc_select(int maxfdp1, struct fd_set *readset, struct fd_set *writeset, struct fd_set *exceptset, struct timeval *timeout);
int ps2ipc_ioctl(int s, long cmd, void *argp);
int ps2ipc_getsockname(int s, struct sockaddr* name, int* namelen);
int ps2ipc_getpeername(int s, struct sockaddr *name, int *namelen);
int ps2ipc_getsockopt(int s, int level, int optname, void* optval, socklen_t* optlen);
int ps2ipc_setsockopt(int s, int level, int optname, const void *optval, socklen_t optlen);
struct hostent *ps2ipc_gethostbyname(const char *name);
void ps2ipc_dns_setserver(u8 numdns, const ip_addr_t *dnsserver);
const ip_addr_t *ps2ipc_dns_getserver(u8 numdns);

#ifdef __cplusplus
}
#endif

#endif /* __PS2IPC_H__ */

/*
# _____     ___ ____     ___ ____
#  ____|   |    ____|   |        | |____|
# |     ___|   |____ ___|    ____| |    \    PS2DEV Open Source Project.
#-----------------------------------------------------------------------
# Copyright ps2dev - http://www.ps2dev.org
# Licenced under Academic Free License version 2.0
# Review ps2sdk README & LICENSE files for further details.
*/

#ifndef __PS2IPEE_H__
#define	__PS2IPEE_H__

#include <ps2ip.h>

#ifdef __cplusplus
extern "C" {
#endif

/* From include/lwip/sockets.h:  */
extern int lwip_accept(int s, struct sockaddr *addr, socklen_t *addrlen);
extern int lwip_bind(int s, const struct sockaddr *name, socklen_t namelen);
extern int lwip_shutdown(int s, int how);
extern int lwip_getpeername (int s, struct sockaddr *name, socklen_t *namelen);
extern int lwip_getsockname (int s, struct sockaddr *name, socklen_t *namelen);
extern int lwip_getsockopt (int s, int level, int optname, void *optval, socklen_t *optlen);
extern int lwip_setsockopt (int s, int level, int optname, const void *optval, socklen_t optlen);
extern int lwip_close(int s);
extern int lwip_connect(int s, const struct sockaddr *name, socklen_t namelen);
extern int lwip_listen(int s, int backlog);
extern int lwip_recv(int s, void *mem, size_t len, int flags);
extern int lwip_read(int s, void *mem, size_t len);
extern int lwip_recvfrom(int s, void *mem, size_t len, int flags, struct sockaddr *from, socklen_t *fromlen);
extern int lwip_send(int s, const void *dataptr, size_t size, int flags);
extern int lwip_sendto(int s, const void *dataptr, size_t size, int flags, const struct sockaddr *to, socklen_t tolen);
extern int lwip_socket(int domain, int type, int protocol);
extern int lwip_write(int s, const void *dataptr, size_t size);
extern int lwip_select(int maxfdp1, fd_set *readset, fd_set *writeset, fd_set *exceptset, struct timeval *timeout);
extern int lwip_ioctl(int s, long cmd, void *argp);
extern int lwip_fcntl(int s, int cmd, int val);

extern int ps2ip_setconfig(const t_ip_info* ip_info);
extern int ps2ip_getconfig(char* netif_name,t_ip_info* ip_info);

/* From include/lwip/inet.h:  */
extern u32        ipaddr_addr(const char *cp);
extern int        ip4addr_aton(const char *cp, ip4_addr_t *addr);
/** returns ptr to static buffer; not reentrant! */
extern char       *ip4addr_ntoa(const ip4_addr_t *addr);
extern char       *ip4addr_ntoa_r(const ip4_addr_t *addr, char *buf, int buflen);

#ifdef PS2IP_DNS
/* From include/lwip/netdb.h:  */
extern struct hostent *lwip_gethostbyname(const char *name);
extern int lwip_gethostbyname_r(const char *name, struct hostent *ret, char *buf,
                size_t buflen, struct hostent **result, int *h_errnop);
extern void lwip_freeaddrinfo(struct addrinfo *ai);
extern int lwip_getaddrinfo(const char *nodename,
       const char *servname,
       const struct addrinfo *hints,
       struct addrinfo **res);

/* From include/lwip/dns.h:  */
extern void           dns_setserver(u8 numdns, const ip_addr_t *dnsserver);
extern const ip_addr_t* dns_getserver(u8 numdns);
#endif

#ifdef __cplusplus
}
#endif

#endif	// !defined(IOP_PS2IP_INTERNAL_H)

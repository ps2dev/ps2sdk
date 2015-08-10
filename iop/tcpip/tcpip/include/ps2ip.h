/*
# _____     ___ ____     ___ ____
#  ____|   |    ____|   |        | |____|
# |     ___|   |____ ___|    ____| |    \    PS2DEV Open Source Project.
#-----------------------------------------------------------------------
# Copyright (c) 2003 Marcus R. Brown <mrbrown@0xd6.org>
# Licenced under Academic Free License version 2.0
# Review ps2sdk README & LICENSE files for further details.
#
# $
# Imports and definitions for ps2ip.
*/

#ifndef IOP_PS2IP_H
#define IOP_PS2IP_H

#include <types.h>
#include <irx.h>

#include "tcpip.h"

#define ps2ip_IMPORTS_start	DECLARE_IMPORT_TABLE(ps2ip, 2, 4)
#define ps2ip_IMPORTS_end	END_IMPORT_TABLE

/* From include/lwip/sockets.h:  */

int       lwip_accept(int s, struct sockaddr *addr, socklen_t *addrlen);
#define I_lwip_accept DECLARE_IMPORT(4, lwip_accept)
int       lwip_bind(int s, struct sockaddr *name, socklen_t namelen);
#define I_lwip_bind DECLARE_IMPORT(5, lwip_bind)
int       lwip_close(int s);
#define I_lwip_close DECLARE_IMPORT(6, lwip_close)
int       lwip_connect(int s, struct sockaddr *name, socklen_t namelen);
#define I_lwip_connect DECLARE_IMPORT(7, lwip_connect)
int       lwip_listen(int s, int backlog);
#define I_lwip_listen DECLARE_IMPORT(8, lwip_listen)
int       lwip_recv(int s, void *mem, int len, unsigned int flags);
#define I_lwip_recv DECLARE_IMPORT(9, lwip_recv)
int       lwip_recvfrom(int s, void *mem, int len, unsigned int flags,
                        struct sockaddr *from, socklen_t *fromlen);
#define I_lwip_recvfrom DECLARE_IMPORT(10, lwip_recvfrom)
int       lwip_send(int s, void *dataptr, int size, unsigned int flags);
#define I_lwip_send DECLARE_IMPORT(11, lwip_send)
int       lwip_sendto(int s, void *dataptr, int size, unsigned int flags,
                      struct sockaddr *to, socklen_t tolen);
#define I_lwip_sendto DECLARE_IMPORT(12, lwip_sendto)
int       lwip_socket(int domain, int type, int protocol);
#define I_lwip_socket DECLARE_IMPORT(13, lwip_socket)
int       lwip_select(int maxfdp1, fd_set *readset, fd_set *writeset, fd_set *exceptset,
                      struct timeval *timeout);
#define I_lwip_select DECLARE_IMPORT(14, lwip_select)
int       lwip_ioctl(int s, long cmd, void *argp);
#define I_lwip_ioctl DECLARE_IMPORT(15, lwip_ioctl)
int       lwip_getpeername (int s, struct sockaddr *name, socklen_t *namelen);
#define I_lwip_getpeername DECLARE_IMPORT(16, lwip_getpeername)
int       lwip_getsockname (int s, struct sockaddr *name, socklen_t *namelen);
#define I_lwip_getsockname DECLARE_IMPORT(17, lwip_getsockname)
int       lwip_getsockopt (int s, int level, int optname, void *optval, socklen_t *optlen);
#define I_lwip_getsockopt DECLARE_IMPORT(18, lwip_getsockopt)
int       lwip_setsockopt (int s, int level, int optname, const void *optval, socklen_t optlen);
#define I_lwip_setsockopt DECLARE_IMPORT(19, lwip_setsockopt)

int       ps2ip_setconfig(t_ip_info* ip_info);
#define I_ps2ip_setconfig DECLARE_IMPORT(20, ps2ip_setconfig)
int       ps2ip_getconfig(char* netif_name,t_ip_info* ip_info);
#define I_ps2ip_getconfig DECLARE_IMPORT(21, ps2ip_getconfig)
void	ps2ip_input(struct pbuf *p, struct netif *inp);
#define I_ps2ip_input DECLARE_IMPORT(22, ps2ip_input)

int lwip_shutdown(int s, int how);
#define I_lwip_shutdown DECLARE_IMPORT(46, lwip_shutdown)
int lwip_fcntl(int s, int cmd, int val);
#define I_lwip_fcntl DECLARE_IMPORT(47, lwip_fcntl)

/* From include/netif/etharp.h:  */
err_t	etharp_output(struct netif *netif, struct pbuf *q, ip_addr_t *ipaddr);
#define I_etharp_output DECLARE_IMPORT(23, etharp_output)

/* From include/lwip/tcpip.h:  */
err_t     tcpip_input(struct pbuf *p, struct netif *inp);
#define I_tcpip_input DECLARE_IMPORT(25, tcpip_input)

/* From include/lwip/netif.h:  */
struct netif*    netif_add(struct netif *netif, struct ip_addr *ipaddr, struct ip_addr *netmask,
                           struct ip_addr *gw,void *state,err_t (* init)(struct netif *netif),
                           err_t (* input)(struct pbuf *p, struct netif *netif));
#define        I_netif_add DECLARE_IMPORT(26, netif_add)

/* Returns a network interface given its name. The name is of the form
   "et0", where the first two letters are the "name" field in the
   netif structure, and the digit is in the num field in the same
   structure. */
struct netif*    netif_find(char *name);
#define        I_netif_find DECLARE_IMPORT(27, netif_find)
void             netif_set_default(struct netif *netif);
#define        I_netif_set_default DECLARE_IMPORT(28, netif_set_default)
void             netif_set_ipaddr(struct netif *netif, struct ip_addr *ipaddr);
#define        I_netif_set_ipaddr DECLARE_IMPORT(29, netif_set_ipaddr)
void             netif_set_netmask(struct netif *netif, struct ip_addr *netmast);
#define        I_netif_set_netmask DECLARE_IMPORT(30, netif_set_netmask)
void             netif_set_gw(struct netif *netif, struct ip_addr *gw);
#define        I_netif_set_gw DECLARE_IMPORT(31, netif_set_gw)
void		netif_set_up(struct netif *netif);
#define        I_netif_set_up DECLARE_IMPORT(32, netif_set_up)
void		netif_set_down(struct netif *netif);
#define        I_netif_set_down DECLARE_IMPORT(33, netif_set_down)

struct pbuf*     pbuf_alloc(pbuf_layer l, u16 size, pbuf_type type);
#define        I_pbuf_alloc DECLARE_IMPORT(34, pbuf_alloc)
void             pbuf_realloc(struct pbuf *p, u16 size);
#define        I_pbuf_realloc DECLARE_IMPORT(35, pbuf_realloc)
u8               pbuf_header(struct pbuf *p, s16 header_size);
#define        I_pbuf_header DECLARE_IMPORT(36, pbuf_header)
void             pbuf_ref(struct pbuf *p);
#define        I_pbuf_ref DECLARE_IMPORT(37, pbuf_ref)
u8               pbuf_free(struct pbuf *p);
#define        I_pbuf_free DECLARE_IMPORT(38, pbuf_free)
u8               pbuf_clen(struct pbuf *p);
#define        I_pbuf_clen DECLARE_IMPORT(39, pbuf_clen)
void             pbuf_chain(struct pbuf *h, struct pbuf *t);
#define        I_pbuf_chain DECLARE_IMPORT(40, pbuf_chain)
struct pbuf*     pbuf_dechain(struct pbuf *p);
#define        I_pbuf_dechain DECLARE_IMPORT(41, pbuf_dechain)
struct pbuf*     pbuf_take(struct pbuf *f);
#define        I_pbuf_take DECLARE_IMPORT(42, pbuf_take)

/* From include/ipv4/lwip/inet.h:  */
/* directly map this to the lwip internal functions */
#define inet_addr(cp)         ipaddr_addr(cp)
#define inet_aton(cp, addr)   ipaddr_aton(cp, (ip_addr_t*)addr)
#define inet_ntoa(addr)       ipaddr_ntoa((ip_addr_t*)&(addr))
#define inet_ntoa_r(addr, buf, buflen) ipaddr_ntoa_r((ip_addr_t*)&(addr), buf, buflen)

u32        ipaddr_addr(const char *cp);
#define  I_ipaddr_addr DECLARE_IMPORT(24, ipaddr_addr)
#define  I_inet_addr DECLARE_IMPORT(24, ipaddr_addr)
int        ipaddr_aton(const char *cp, ip_addr_t *addr);
#define  I_ipaddr_aton DECLARE_IMPORT(43, ipaddr_aton)
#define  I_inet_aton DECLARE_IMPORT(43, ipaddr_aton)
/** returns ptr to static buffer; not reentrant! */
char       *ipaddr_ntoa(const ip_addr_t *addr);
#define  I_ipaddr_ntoa DECLARE_IMPORT(44, ipaddr_ntoa)
#define  I_inet_ntoa DECLARE_IMPORT(44, ipaddr_ntoa)
char       *ipaddr_ntoa_r(const ip_addr_t *addr, char *buf, int buflen);
#define  I_ipaddr_ntoa_r DECLARE_IMPORT(45, ipaddr_ntoa_r)
#define  I_inet_ntoa_r DECLARE_IMPORT(45, ipaddr_ntoa_r)

#ifdef PS2IP_DNS
/* From include/ipv4/lwip/netdb.h:  */
struct hostent *lwip_gethostbyname(const char *name);
#define  I_lwip_gethostbyname DECLARE_IMPORT(48, lwip_gethostbyname)
int lwip_gethostbyname_r(const char *name, struct hostent *ret, char *buf,
                size_t buflen, struct hostent **result, int *h_errnop);
#define  I_lwip_gethostbyname_r DECLARE_IMPORT(49, lwip_gethostbyname_r)
void lwip_freeaddrinfo(struct addrinfo *ai);
#define  I_lwip_freeaddrinfo DECLARE_IMPORT(50, lwip_freeaddrinfo)
int lwip_getaddrinfo(const char *nodename,
       const char *servname,
       const struct addrinfo *hints,
       struct addrinfo **res);
#define  I_lwip_getaddrinfo DECLARE_IMPORT(51, lwip_getaddrinfo)

void           dns_setserver(u8 numdns, ip_addr_t *dnsserver);
#define  I_dns_setserver DECLARE_IMPORT(52, dns_setserver)
ip_addr_t      dns_getserver(u8 numdns);
#define  I_dns_getserver DECLARE_IMPORT(53, dns_getserver)
#endif

/* Compatibility macros.  */

#define	accept			lwip_accept
#define	bind			lwip_bind
#define	disconnect		lwip_close
#define closesocket		lwip_close
#define	shutdown		lwip_shutdown
#define	connect			lwip_connect
#define	listen			lwip_listen
#define	recv			lwip_recv
#define	recvfrom		lwip_recvfrom
#define	send			lwip_send
#define	sendto			lwip_sendto
#define	socket			lwip_socket
#define	select			lwip_select
#define	ioctlsocket		lwip_ioctl
#define	fcntlsocket		lwip_fcntl
#define	gethostbyname		lwip_gethostbyname
#define	gethostbyname_r		lwip_gethostbyname_r
#define	freeaddrinfo		lwip_freeaddrinfo
#define	getaddrinfo		lwip_getaddrinfo

// ntba2
#define getsockname		lwip_getsockname
#define getpeername		lwip_getpeername
#define getsockopt		lwip_getsockopt
#define setsockopt		lwip_setsockopt

#endif /* IOP_PS2IP_H */

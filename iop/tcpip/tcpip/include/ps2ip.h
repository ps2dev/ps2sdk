/*
# _____     ___ ____     ___ ____
#  ____|   |    ____|   |        | |____|
# |     ___|   |____ ___|    ____| |    \    PS2DEV Open Source Project.
#-----------------------------------------------------------------------
# Copyright (c) 2003 Marcus R. Brown <mrbrown@0xd6.org>
# Licenced under Academic Free License version 2.0
# Review ps2sdk README & LICENSE files for further details.
*/

/**
 * @file
 * Imports and definitions for ps2ip.
 */

#ifndef __PS2IP_H__
#define __PS2IP_H__

#include <types.h>
#include <irx.h>

#include <tcpip.h>

#include <sys/time.h>

/* From include/lwip/sockets.h:  */

extern int       lwip_accept(int s, struct sockaddr *addr, socklen_t *addrlen);
extern int       lwip_bind(int s, struct sockaddr *name, socklen_t namelen);
extern int       lwip_close(int s);
extern int       lwip_connect(int s, struct sockaddr *name, socklen_t namelen);
extern int       lwip_listen(int s, int backlog);
extern int       lwip_recv(int s, void *mem, int len, unsigned int flags);
extern int       lwip_recvfrom(int s, void *mem, int len, unsigned int flags,
                        struct sockaddr *from, socklen_t *fromlen);
extern int       lwip_send(int s, void *dataptr, int size, unsigned int flags);
extern int       lwip_sendto(int s, void *dataptr, int size, unsigned int flags,
                      struct sockaddr *to, socklen_t tolen);
extern int       lwip_socket(int domain, int type, int protocol);
extern int       lwip_select(int maxfdp1, fd_set *readset, fd_set *writeset, fd_set *exceptset,
                      struct timeval *timeout);
extern int       lwip_ioctl(int s, long cmd, void *argp);
extern int       lwip_getpeername (int s, struct sockaddr *name, socklen_t *namelen);
extern int       lwip_getsockname (int s, struct sockaddr *name, socklen_t *namelen);
extern int       lwip_getsockopt (int s, int level, int optname, void *optval, socklen_t *optlen);
extern int       lwip_setsockopt (int s, int level, int optname, const void *optval, socklen_t optlen);

extern int       ps2ip_setconfig(const t_ip_info* ip_info);
extern int       ps2ip_getconfig(char* netif_name,t_ip_info* ip_info);
extern void    ps2ip_input(struct pbuf *p, struct netif *inp);

extern int lwip_shutdown(int s, int how);
extern int lwip_fcntl(int s, int cmd, int val);

/* From include/netif/etharp.h:  */
extern err_t   etharp_output(struct netif *netif, struct pbuf *q, const ip_addr_t *ipaddr);

/* From include/lwip/tcpip.h:  */
extern err_t     tcpip_input(struct pbuf *p, struct netif *inp);

/** Function prototype for functions passed to tcpip_callback() */
typedef void (*tcpip_callback_fn)(void *ctx);

extern err_t  tcpip_callback_with_block(tcpip_callback_fn function, void *ctx, u8 block);

/**
 * @ingroup lwip_os
 * @see tcpip_callback_with_block
 */
#define tcpip_callback(f, ctx)  tcpip_callback_with_block(f, ctx, 1)

/* From include/lwip/netif.h:  */
extern struct netif *netif_add(struct netif *netif,
#if LWIP_IPV4
                        const ip4_addr_t *ipaddr, const ip4_addr_t *netmask, const ip4_addr_t *gw,
#endif /* LWIP_IPV4 */
                        void *state, netif_init_fn init, netif_input_fn input);

/* Returns a network interface given its name. The name is of the form
   "et0", where the first two letters are the "name" field in the
   netif structure, and the digit is in the num field in the same
   structure. */
extern struct netif*    netif_find(const char *name);
extern void             netif_set_default(struct netif *netif);
extern void             netif_set_ipaddr(struct netif *netif, const ip4_addr_t *ipaddr);
extern void             netif_set_netmask(struct netif *netif, const ip4_addr_t *netmask);
extern void             netif_set_gw(struct netif *netif, const ip4_addr_t *gw);
extern void        netif_set_up(struct netif *netif);
extern void        netif_set_down(struct netif *netif);

extern void netif_set_link_up(struct netif *netif);
extern void netif_set_link_down(struct netif *netif);

/* From include/lwip/pbuf.h:  */
extern struct pbuf*     pbuf_alloc(pbuf_layer l, u16 size, pbuf_type type);
extern void             pbuf_realloc(struct pbuf *p, u16 size);
extern u8               pbuf_header(struct pbuf *p, s16 header_size);
extern void             pbuf_ref(struct pbuf *p);
extern u8               pbuf_free(struct pbuf *p);
extern u8               pbuf_clen(struct pbuf *p);
extern void             pbuf_chain(struct pbuf *h, struct pbuf *t);
extern struct pbuf*     pbuf_dechain(struct pbuf *p);
extern struct pbuf*     pbuf_take(struct pbuf *f);
extern struct pbuf*     pbuf_coalesce(struct pbuf *p, pbuf_layer layer);

/* From include/lwip/inet.h:  */
/* directly map this to the lwip internal functions */
#define inet_addr(cp)                   ipaddr_addr(cp)
#define inet_aton(cp, addr)             ip4addr_aton(cp, (ip4_addr_t*)addr)
#define inet_ntoa(addr)                 ip4addr_ntoa((const ip4_addr_t*)&(addr))
#define inet_ntoa_r(addr, buf, buflen)  ip4addr_ntoa_r((const ip4_addr_t*)&(addr), buf, buflen)

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

#define ipaddr4_aton(...) ip4addr_aton(__VA_ARGS__)

#define ps2ip_IMPORTS_start DECLARE_IMPORT_TABLE(ps2ip, 2, 6)
#define ps2ip_IMPORTS_end END_IMPORT_TABLE

#define I_lwip_accept DECLARE_IMPORT(4, lwip_accept)
#define I_lwip_bind DECLARE_IMPORT(5, lwip_bind)
#define I_lwip_close DECLARE_IMPORT(6, lwip_close)
#define I_lwip_connect DECLARE_IMPORT(7, lwip_connect)
#define I_lwip_listen DECLARE_IMPORT(8, lwip_listen)
#define I_lwip_recv DECLARE_IMPORT(9, lwip_recv)
#define I_lwip_recvfrom DECLARE_IMPORT(10, lwip_recvfrom)
#define I_lwip_send DECLARE_IMPORT(11, lwip_send)
#define I_lwip_sendto DECLARE_IMPORT(12, lwip_sendto)
#define I_lwip_socket DECLARE_IMPORT(13, lwip_socket)
#define I_lwip_select DECLARE_IMPORT(14, lwip_select)
#define I_lwip_ioctl DECLARE_IMPORT(15, lwip_ioctl)
#define I_lwip_getpeername DECLARE_IMPORT(16, lwip_getpeername)
#define I_lwip_getsockname DECLARE_IMPORT(17, lwip_getsockname)
#define I_lwip_getsockopt DECLARE_IMPORT(18, lwip_getsockopt)
#define I_lwip_setsockopt DECLARE_IMPORT(19, lwip_setsockopt)
#define I_ps2ip_setconfig DECLARE_IMPORT(20, ps2ip_setconfig)
#define I_ps2ip_getconfig DECLARE_IMPORT(21, ps2ip_getconfig)
#define I_ps2ip_input DECLARE_IMPORT(22, ps2ip_input)
#define I_lwip_shutdown DECLARE_IMPORT(46, lwip_shutdown)
#define I_lwip_fcntl DECLARE_IMPORT(47, lwip_fcntl)
#define I_etharp_output DECLARE_IMPORT(23, etharp_output)
#define I_tcpip_input DECLARE_IMPORT(25, tcpip_input)
#define I_tcpip_callback_with_block DECLARE_IMPORT(56, tcpip_callback_with_block)
#define I_netif_add DECLARE_IMPORT(26, netif_add)
#define I_netif_find DECLARE_IMPORT(27, netif_find)
#define I_netif_set_default DECLARE_IMPORT(28, netif_set_default)
#define I_netif_set_ipaddr DECLARE_IMPORT(29, netif_set_ipaddr)
#define I_netif_set_netmask DECLARE_IMPORT(30, netif_set_netmask)
#define I_netif_set_gw DECLARE_IMPORT(31, netif_set_gw)
#define I_netif_set_up DECLARE_IMPORT(32, netif_set_up)
#define I_netif_set_down DECLARE_IMPORT(33, netif_set_down)
#define I_netif_set_link_up DECLARE_IMPORT(54, netif_set_link_up)
#define I_netif_set_link_down DECLARE_IMPORT(55, netif_set_link_down)
#define I_pbuf_alloc DECLARE_IMPORT(34, pbuf_alloc)
#define I_pbuf_realloc DECLARE_IMPORT(35, pbuf_realloc)
#define I_pbuf_header DECLARE_IMPORT(36, pbuf_header)
#define I_pbuf_ref DECLARE_IMPORT(37, pbuf_ref)
#define I_pbuf_free DECLARE_IMPORT(38, pbuf_free)
#define I_pbuf_clen DECLARE_IMPORT(39, pbuf_clen)
#define I_pbuf_chain DECLARE_IMPORT(40, pbuf_chain)
#define I_pbuf_dechain DECLARE_IMPORT(41, pbuf_dechain)
#define I_pbuf_take DECLARE_IMPORT(42, pbuf_take)
#define I_pbuf_coalesce DECLARE_IMPORT(57, pbuf_coalesce)
#define I_ipaddr_addr DECLARE_IMPORT(24, ipaddr_addr)
#define I_ip4addr_aton DECLARE_IMPORT(43, ip4addr_aton)
#define I_ip4addr_ntoa DECLARE_IMPORT(44, ip4addr_ntoa)
#define I_ip4addr_ntoa_r DECLARE_IMPORT(45, ip4addr_ntoa_r)
#define I_lwip_gethostbyname DECLARE_IMPORT(48, lwip_gethostbyname)
#define I_lwip_gethostbyname_r DECLARE_IMPORT(49, lwip_gethostbyname_r)
#define I_lwip_freeaddrinfo DECLARE_IMPORT(50, lwip_freeaddrinfo)
#define I_lwip_getaddrinfo DECLARE_IMPORT(51, lwip_getaddrinfo)
#define I_dns_setserver DECLARE_IMPORT(52, dns_setserver)
#define I_dns_getserver DECLARE_IMPORT(53, dns_getserver)

#define I_inet_addr I_ipaddr_addr
#define I_ipaddr4_aton I_ip4addr_aton
#define I_inet_aton I_ip4addr_aton
#define I_inet_ntoa I_ip4addr_ntoa
#define I_inet_ntoa_r I_ip4addr_ntoa_r

#endif /* __PS2IP_H__ */

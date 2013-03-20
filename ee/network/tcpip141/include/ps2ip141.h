/*
# _____     ___ ____     ___ ____
#  ____|   |    ____|   |        | |____|
# |     ___|   |____ ___|    ____| |    \    PS2DEV Open Source Project.
#-----------------------------------------------------------------------
# Copyright (c) 2003 Marcus R. Brown <mrbrown@0xd6.org>
# Licenced under Academic Free License version 2.0
# Review ps2sdk README & LICENSE files for further details.
#
# $Id: ps2ip.h 1423 2007-07-07 12:21:26Z radad $
# Imports and definitions for ps2ip.
*/

#ifndef IOP_PS2IP_H
#define IOP_PS2IP_H

#include <tcpip141.h>

int InitPS2IP(struct ip_addr *ip_address, struct ip_addr *subnet_mask, struct ip_addr *gateway);
void DeinitPS2IP(void);

/* From include/lwip/sockets.h:  */

int       lwip_accept(int s, struct sockaddr *addr, socklen_t *addrlen);
int       lwip_bind(int s, struct sockaddr *name, socklen_t namelen);
int       lwip_close(int s);
int       lwip_connect(int s, struct sockaddr *name, socklen_t namelen);
int       lwip_listen(int s, int backlog);
int       lwip_recv(int s, void *mem, int len, unsigned int flags);
int       lwip_recvfrom(int s, void *mem, int len, unsigned int flags,
                        struct sockaddr *from, socklen_t *fromlen);
int       lwip_send(int s, void *dataptr, int size, unsigned int flags);
int       lwip_sendto(int s, void *dataptr, int size, unsigned int flags,
                      struct sockaddr *to, socklen_t tolen);
int       lwip_socket(int domain, int type, int protocol);
int       lwip_select(int maxfdp1, fd_set *readset, fd_set *writeset, fd_set *exceptset,
                      struct timeval *timeout);
int       lwip_ioctl(int s, long cmd, void *argp);
int       lwip_getpeername (int s, struct sockaddr *name, socklen_t *namelen);
int       lwip_getsockname (int s, struct sockaddr *name, socklen_t *namelen);
int       lwip_getsockopt (int s, int level, int optname, void *optval, socklen_t *optlen);
int       lwip_setsockopt (int s, int level, int optname, const void *optval, socklen_t optlen);

int       ps2ip_setconfig(t_ip_info* ip_info);
int       ps2ip_getconfig(char* netif_name,t_ip_info* ip_info);
err_t	ps2ip_input(struct pbuf *p, struct netif *inp);

/* From include/netif/etharp.h:  */
err_t	etharp_output(struct netif *netif, struct pbuf *q, ip_addr_t *ipaddr);

/* From include/ipv4/lwip/inet.h:  */
u32        inet_addr(const char *cp);

/* From include/lwip/tcpip.h:  */
err_t     tcpip_input(struct pbuf *p, struct netif *inp);

/* From include/lwip/netif.h:  */
struct netif*    netif_add(struct netif *netif, struct ip_addr *ipaddr, struct ip_addr *netmask,
                           struct ip_addr *gw,void *state,err_t (* init)(struct netif *netif),
                           err_t (* input)(struct pbuf *p, struct netif *netif));

/* Returns a network interface given its name. The name is of the form
   "et0", where the first two letters are the "name" field in the
   netif structure, and the digit is in the num field in the same
   structure. */
struct netif*    netif_find(char *name);
void             netif_set_default(struct netif *netif);
void             netif_set_ipaddr(struct netif *netif, struct ip_addr *ipaddr);
void             netif_set_netmask(struct netif *netif, struct ip_addr *netmast);
void             netif_set_gw(struct netif *netif, struct ip_addr *gw);
void		netif_set_up(struct netif *netif);
void		netif_set_down(struct netif *netif);

struct pbuf*     pbuf_alloc(pbuf_layer l, u16 size, pbuf_type type);
void             pbuf_realloc(struct pbuf *p, u16 size);
u8               pbuf_header(struct pbuf *p, s16 header_size);
void             pbuf_ref(struct pbuf *p);
u8               pbuf_free(struct pbuf *p);
u8               pbuf_clen(struct pbuf *p);
void             pbuf_chain(struct pbuf *h, struct pbuf *t);
struct pbuf*     pbuf_dechain(struct pbuf *p);
struct pbuf*     pbuf_take(struct pbuf *f);

/* Compatibility macros.  */

#define	accept			lwip_accept
#define	bind			lwip_bind
#define	disconnect		lwip_close
#define	connect			lwip_connect
#define	listen			lwip_listen
#define	recv			lwip_recv
#define	recvfrom		lwip_recvfrom
#define	send			lwip_send
#define	sendto			lwip_sendto
#define	socket			lwip_socket
#define	select			lwip_select
#define	ioctlsocket		lwip_ioctl

// ntba2
#define getsockname		lwip_getsockname
#define getpeername		lwip_getpeername
#define getsockopt		lwip_getsockopt
#define setsockopt		lwip_setsockopt

#endif /* IOP_PS2IP_H */

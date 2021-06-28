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

#include <stddef.h>
#include <tcpip.h>
#include <time.h>

/** Initializes PS2IP. Specify a dummy address like "169.254.0.1" if DHCP is to be used, before enabling DHCP via ps2ip_setconfig(). */
int ps2ipInit(struct ip4_addr *ip_address, struct ip4_addr *subnet_mask, struct ip4_addr *gateway);
void ps2ipDeinit(void);
/**
 * Use to specify the number of H-sync ticks per milisecond (Default: 16). 
 * Use this function to keep timings accurate, if a mode like 480P (~31KHz H-sync) is used instead of NTSC/PAL (~16KHz H-sync).	
 */
void ps2ipSetHsyncTicksPerMSec(unsigned char ticks);

/* From include/lwip/sockets.h:  */
int lwip_accept(int s, struct sockaddr *addr, socklen_t *addrlen);
int lwip_bind(int s, const struct sockaddr *name, socklen_t namelen);
int lwip_shutdown(int s, int how);
int lwip_getpeername (int s, struct sockaddr *name, socklen_t *namelen);
int lwip_getsockname (int s, struct sockaddr *name, socklen_t *namelen);
int lwip_getsockopt (int s, int level, int optname, void *optval, socklen_t *optlen);
int lwip_setsockopt (int s, int level, int optname, const void *optval, socklen_t optlen);
int lwip_close(int s);
int lwip_connect(int s, const struct sockaddr *name, socklen_t namelen);
int lwip_listen(int s, int backlog);
int lwip_recv(int s, void *mem, size_t len, int flags);
int lwip_read(int s, void *mem, size_t len);
int lwip_recvfrom(int s, void *mem, size_t len, int flags, struct sockaddr *from, socklen_t *fromlen);
int lwip_send(int s, const void *dataptr, size_t size, int flags);
int lwip_sendto(int s, const void *dataptr, size_t size, int flags, const struct sockaddr *to, socklen_t tolen);
int lwip_socket(int domain, int type, int protocol);
int lwip_write(int s, const void *dataptr, size_t size);
int lwip_select(int maxfdp1, fd_set *readset, fd_set *writeset, fd_set *exceptset, struct timeval *timeout);
int lwip_ioctl(int s, long cmd, void *argp);
int lwip_fcntl(int s, int cmd, int val);

#define accept(a,b,c)		lwip_accept(a,b,c)
#define bind(a,b,c)		lwip_bind(a,b,c)
#define shutdown(a,b)		lwip_shutdown(a,b)
#define disconenct(s)		lwip_close(s)
#define closesocket(s)		lwip_close(s)
#define connect(a,b,c)		lwip_connect(a,b,c)
#define getsockname(a,b,c)	lwip_getsockname(a,b,c)
#define getpeername(a,b,c)	lwip_getpeername(a,b,c)
#define setsockopt(a,b,c,d,e)	lwip_setsockopt(a,b,c,d,e)
#define getsockopt(a,b,c,d,e)	lwip_getsockopt(a,b,c,d,e)
#define listen(a,b)		lwip_listen(a,b)
#define recv(a,b,c,d)		lwip_recv(a,b,c,d)
#define recvfrom(a,b,c,d,e,f)	lwip_recvfrom(a,b,c,d,e,f)
#define send(a,b,c,d)		lwip_send(a,b,c,d)
#define sendto(a,b,c,d,e,f)	lwip_sendto(a,b,c,d,e,f)
#define socket(a,b,c)		lwip_socket(a,b,c)
#define select(a,b,c,d,e)	lwip_select(a,b,c,d,e)
#define ioctlsocket(a,b,c)	lwip_ioctl(a,b,c)

int	ps2ip_setconfig(const t_ip_info* ip_info);
int	ps2ip_getconfig(char* netif_name,t_ip_info* ip_info);
err_t	ps2ip_input(struct pbuf *p, struct netif *inp);

/* From include/netif/etharp.h:  */
err_t	etharp_output(struct netif *netif, struct pbuf *q, const ip_addr_t *ipaddr);

/* From include/lwip/inet.h:  */
/* directly map this to the lwip internal functions */
#define inet_addr(cp)                   ipaddr_addr(cp)
#define inet_aton(cp, addr)             ip4addr_aton(cp, (ip4_addr_t*)addr)
#define inet_ntoa(addr)                 ip4addr_ntoa((const ip4_addr_t*)&(addr))
#define inet_ntoa_r(addr, buf, buflen)  ip4addr_ntoa_r((const ip4_addr_t*)&(addr), buf, buflen)

u32        ipaddr_addr(const char *cp);
int        ip4addr_aton(const char *cp, ip4_addr_t *addr);
/** returns ptr to static buffer; not reentrant! */
char       *ip4addr_ntoa(const ip4_addr_t *addr);
char       *ip4addr_ntoa_r(const ip4_addr_t *addr, char *buf, int buflen);

/* From include/lwip/tcpip.h:  */
err_t     tcpip_input(struct pbuf *p, struct netif *inp);

/* From include/lwip/netif.h:  */
struct netif *netif_add(struct netif *netif,
#if LWIP_IPV4
                        const ip4_addr_t *ipaddr, const ip4_addr_t *netmask, const ip4_addr_t *gw,
#endif /* LWIP_IPV4 */
                        void *state, netif_init_fn init, netif_input_fn input);

/** Returns a network interface given its name. 
 * The name is of the form "et0", where the first two letters are the "name" field in the
 * netif structure, and the digit is in the num field in the same structure. 
 */
struct netif *netif_find(const char *name);
void netif_set_default(struct netif *netif);
void netif_set_ipaddr(struct netif *netif, const ip4_addr_t *ipaddr);
void netif_set_netmask(struct netif *netif, const ip4_addr_t *netmask);
void netif_set_gw(struct netif *netif, const ip4_addr_t *gw);
void netif_set_up(struct netif *netif);
void netif_set_down(struct netif *netif);

struct pbuf*     pbuf_alloc(pbuf_layer l, u16 size, pbuf_type type);
void             pbuf_realloc(struct pbuf *p, u16 size);
u8               pbuf_header(struct pbuf *p, s16 header_size);
void             pbuf_ref(struct pbuf *p);
u8               pbuf_free(struct pbuf *p);
u8               pbuf_clen(struct pbuf *p);
void             pbuf_chain(struct pbuf *h, struct pbuf *t);
struct pbuf*     pbuf_dechain(struct pbuf *p);
struct pbuf*     pbuf_take(struct pbuf *f);
struct pbuf*     pbuf_coalesce(struct pbuf *p, pbuf_layer layer);

#ifdef PS2IP_DNS
/* From include/lwip/netdb.h:  */
struct hostent *lwip_gethostbyname(const char *name);
int lwip_gethostbyname_r(const char *name, struct hostent *ret, char *buf,
                size_t buflen, struct hostent **result, int *h_errnop);
void lwip_freeaddrinfo(struct addrinfo *ai);
int lwip_getaddrinfo(const char *nodename,
       const char *servname,
       const struct addrinfo *hints,
       struct addrinfo **res);

/* From include/lwip/dns.h:  */
void           dns_setserver(u8 numdns, const ip_addr_t *dnsserver);
const ip_addr_t* dns_getserver(u8 numdns);
#endif

#endif /* __PS2IP_H__ */

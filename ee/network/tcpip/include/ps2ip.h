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

#ifndef LIBCGLUE_SYS_SOCKET_ALIASES
#define LIBCGLUE_SYS_SOCKET_ALIASES 1
#endif

#include <stddef.h>
#include <sys/socket.h>
#include <time.h>

/** Initializes PS2IP. Specify a dummy address like "169.254.0.1" if DHCP is to be used, before enabling DHCP via ps2ip_setconfig(). */
int ps2ipInit(struct ip4_addr *ip_address, struct ip4_addr *subnet_mask, struct ip4_addr *gateway);
void ps2ipDeinit(void);
/**
 * Use to specify the number of H-sync ticks per milisecond (Default: 16). 
 * Use this function to keep timings accurate, if a mode like 480P (~31KHz H-sync) is used instead of NTSC/PAL (~16KHz H-sync).	
 * This function is obsolete, so it is stubbed for compatibility purposes.
 */
void ps2ipSetHsyncTicksPerMSec(unsigned char ticks);

err_t	ps2ip_input(struct pbuf *p, struct netif *inp);

/* From include/netif/etharp.h:  */
err_t	etharp_output(struct netif *netif, struct pbuf *q, const ip_addr_t *ipaddr);

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

#endif /* __PS2IP_H__ */

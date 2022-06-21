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
 * Common definitions file for the LWIP v2.0.0 port.
 */

#ifndef __TCPIP_H__
#define __TCPIP_H__

#include <tamtypes.h>

/* Some portions of this header fall under the following copyright.  The license
   is compatible with that of ps2sdk.

   This port of LWIP has LWIP_DHCP defined by default.    */

#ifndef PS2IP_DNS
#define PS2IP_DNS
#endif
#ifndef PS2IP_DHCP
#define PS2IP_DHCP 1
#endif
#ifndef LWIP_IPV4
#define LWIP_IPV4 1
#endif
#ifndef LWIP_IPV6
#define LWIP_IPV6 0
#endif
#ifndef LWIP_IPV6_MLD
#define LWIP_IPV6_MLD 0
#endif
#ifndef LWIP_CHECKSUM_CTRL_PER_NETIF
#define LWIP_CHECKSUM_CTRL_PER_NETIF 0
#endif
#ifndef LWIP_NETIF_REMOVE_CALLBACK
#define LWIP_NETIF_REMOVE_CALLBACK 0
#endif
#ifndef LWIP_IPV6_AUTOCONFIG
#define LWIP_IPV6_AUTOCONFIG 0
#endif
#ifndef LWIP_MULTICAST_TX_OPTIONS
#define LWIP_MULTICAST_TX_OPTIONS 0
#endif
#ifndef LWIP_DHCP
#define LWIP_DHCP 1
#endif
#ifndef LWIP_TCP
#define LWIP_TCP 1
#endif
#ifndef LWIP_UDP
#define LWIP_UDP 1
#endif
#ifndef LWIP_UDPLITE
#define LWIP_UDPLITE 0
#endif
#ifndef MIB2_STATS
#define MIB2_STATS 0
#endif
#ifndef ENABLE_LOOPBACK
#define ENABLE_LOOPBACK 0
#endif
#ifndef DNS_LOCAL_HOSTLIST
#define DNS_LOCAL_HOSTLIST 0
#endif

/*** Taken from src/include/lwip/opt.h. If changes were made to lwipopts.h, please update this section.
    Some settings affect the fields present in structures like struct netif! ****/
#define MEMP_NUM_UDP_PCB              4
#define MEMP_NUM_TCP_PCB              5
#define MEMP_NUM_NETCONN              (MEMP_NUM_TCP_PCB + MEMP_NUM_UDP_PCB)
#define LWIP_NETIF_STATUS_CALLBACK    0
#define LWIP_NETIF_LINK_CALLBACK      0
#define LWIP_AUTOIP                   0
#define LWIP_NETIF_HOSTNAME           0
#define LWIP_SNMP                     0
#define LWIP_IGMP                     0
#define LWIP_NETIF_HWADDRHINT         0
#define LWIP_LOOPBACK_MAX_PBUFS       0
#define LWIP_NUM_NETIF_CLIENT_DATA    0
#define LWIP_SOCKET_OFFSET            0
#define LWIP_IPV6_SEND_ROUTER_SOLICIT 1
#define DNS_MAX_SERVERS               2

/*
 * Copyright (c) 2001-2003 Swedish Institute of Computer Science.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without modification,
 * are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice,
 *    this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
 * 3. The name of the author may not be used to endorse or promote products
 *    derived from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR IMPLIED
 * WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT
 * SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT
 * OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING
 * IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY
 * OF SUCH DAMAGE.
 *
 * This file is part of the lwIP TCP/IP stack.
 *
 * Author: Adam Dunkels <adam@sics.se>
 *
 */

typedef signed char err_t; /* lwIP error type.  */

/* From src/include/lwip/pbuf.h:  */

#define PBUF_TRANSPORT_HLEN 20
#if LWIP_IPV6
#define PBUF_IP_HLEN 40
#else
#define PBUF_IP_HLEN 20
#endif

/**
 * @ingroup pbuf
 * Enumeration of pbuf layers
 */
typedef enum {
    /** Includes spare room for transport layer header, e.g. UDP header.
     * Use this if you intend to pass the pbuf to functions like udp_send().
     */
    PBUF_TRANSPORT,
    /** Includes spare room for IP header.
     * Use this if you intend to pass the pbuf to functions like raw_send().
     */
    PBUF_IP,
    /** Includes spare room for link layer header (ethernet header).
     * Use this if you intend to pass the pbuf to functions like ethernet_output().
     * @see PBUF_LINK_HLEN
     */
    PBUF_LINK,
    /** Includes spare room for additional encapsulation header before ethernet
     * headers (e.g. 802.11).
     * Use this if you intend to pass the pbuf to functions like netif->linkoutput().
     * @see PBUF_LINK_ENCAPSULATION_HLEN
     */
    PBUF_RAW_TX,
    /** Use this for input packets in a netif driver when calling netif->input()
     * in the most common case - ethernet-layer netif driver. */
    PBUF_RAW
} pbuf_layer;

/**
 * @ingroup pbuf
 * Enumeration of pbuf types
 */
typedef enum {
    /** pbuf data is stored in RAM, used for TX mostly, struct pbuf and its payload
        are allocated in one piece of contiguous memory (so the first payload byte
        can be calculated from struct pbuf).
        pbuf_alloc() allocates PBUF_RAM pbufs as unchained pbufs (although that might
        change in future versions).
        This should be used for all OUTGOING packets (TX).*/
    PBUF_RAM,
    /** pbuf data is stored in ROM, i.e. struct pbuf and its payload are located in
        totally different memory areas. Since it points to ROM, payload does not
        have to be copied when queued for transmission. */
    PBUF_ROM,
    /** pbuf comes from the pbuf pool. Much like PBUF_ROM but payload might change
        so it has to be duplicated when queued before transmitting, depending on
        who has a 'ref' to it. */
    PBUF_REF,
    /** pbuf payload refers to RAM. This one comes from a pool and should be used
        for RX. Payload can be chained (scatter-gather RX) but like PBUF_RAM, struct
        pbuf and its payload are allocated in one piece of contiguous memory (so
        the first payload byte can be calculated from struct pbuf).
        Don't use this for TX, if the pool becomes empty e.g. because of TCP queuing,
        you are unable to receive TCP acks! */
    PBUF_POOL
} pbuf_type;

/** Main packet buffer struct */
struct pbuf
{
    /** next pbuf in singly linked pbuf chain */
    struct pbuf *next;

    /** pointer to the actual data in the buffer */
    void *payload;

    /**
     * total length of this buffer and all next buffers in chain
     * belonging to the same packet.
     *
     * For non-queue packet chains this is the invariant:
     * p->tot_len == p->len + (p->next? p->next->tot_len: 0)
     */
    u16 tot_len;

    /** length of this buffer */
    u16 len;

    /** pbuf_type as u8 instead of enum to save space */
    u8 /*pbuf_type*/ type;

    /** misc flags */
    u8 flags;

    /**
     * the reference count always equals the number of pointers
     * that refer to this pbuf. This can be pointers from an application,
     * the stack itself, or pbuf->next pointers from a chain.
     */
    u16 ref;
};

/* From include/lwip/ip4_addr.h:  */
/** This is the aligned version of ip4_addr_t,
   used as local variable, on the stack, etc. */
struct ip4_addr
{
    u32 addr;
};

/** ip4_addr_t uses a struct for convenience only, so that the same defines can
 * operate both on ip4_addr_t as well as on ip4_addr_p_t. */
typedef struct ip4_addr ip4_addr_t;
typedef struct ip4_addr_packed ip4_addr_p_t;

/** 255.255.255.255 */
#define IPADDR_NONE      ((u32)0xffffffffUL)
/** 127.0.0.1 */
#define IPADDR_LOOPBACK  ((u32)0x7f000001UL)
/** 0.0.0.0 */
#define IPADDR_ANY       ((u32)0x00000000UL)
/** 255.255.255.255 */
#define IPADDR_BROADCAST ((u32)0xffffffffUL)

/* Definitions of the bits in an Internet address integer.

   On subnets, host and network parts are found according to
   the subnet mask, not these masks.  */
#define IP_CLASSA(a)     ((((u32)(a)) & 0x80000000UL) == 0)
#define IP_CLASSA_NET    0xff000000
#define IP_CLASSA_NSHIFT 24
#define IP_CLASSA_HOST   (0xffffffff & ~IP_CLASSA_NET)
#define IP_CLASSA_MAX    128

#define IP_CLASSB(a)     ((((u32)(a)) & 0xc0000000UL) == 0x80000000UL)
#define IP_CLASSB_NET    0xffff0000
#define IP_CLASSB_NSHIFT 16
#define IP_CLASSB_HOST   (0xffffffff & ~IP_CLASSB_NET)
#define IP_CLASSB_MAX    65536

#define IP_CLASSC(a)     ((((u32)(a)) & 0xe0000000UL) == 0xc0000000UL)
#define IP_CLASSC_NET    0xffffff00
#define IP_CLASSC_NSHIFT 8
#define IP_CLASSC_HOST   (0xffffffff & ~IP_CLASSC_NET)

#define IP_CLASSD(a)     (((u32)(a)&0xf0000000UL) == 0xe0000000UL)
#define IP_CLASSD_NET    0xf0000000 /* These ones aren't really */
#define IP_CLASSD_NSHIFT 28         /*   net and host fields, but */
#define IP_CLASSD_HOST   0x0fffffff /*   routing needn't know. */
#define IP_MULTICAST(a)  IP_CLASSD(a)

#define IP_EXPERIMENTAL(a) (((u32)(a)&0xf0000000UL) == 0xf0000000UL)
#define IP_BADCLASS(a)     (((u32)(a)&0xf0000000UL) == 0xf0000000UL)

#define IP_LOOPBACKNET 127 /* official! */


/** Set an IP address given by the four byte-parts.
    Little-endian version that prevents the use of lwip_htonl. */
#define IP4_ADDR(ipaddr, a, b, c, d)           \
    (ipaddr)->addr = ((u32)((d)&0xff) << 24) | \
                     ((u32)((c)&0xff) << 16) | \
                     ((u32)((b)&0xff) << 8) |  \
                     (u32)((a)&0xff)

/** MEMCPY-like copying of IP addresses where addresses are known to be
 * 16-bit-aligned if the port is correctly configured (so a port could define
 * this to copying 2 u16's) - no NULL-pointer-checking needed. */
#ifndef IPADDR2_COPY
#define IPADDR2_COPY(dest, src) memcpy(dest, src, sizeof(ip4_addr_t))
#endif

/** Copy IP address - faster than ip4_addr_set: no NULL check */
#define ip4_addr_copy(dest, src) ((dest).addr = (src).addr)
/** Safely copy one IP address to another (src may be NULL) */
#define ip4_addr_set(dest, src)  ((dest)->addr =          \
                                     ((src) == NULL ? 0 : \
                                                      (src)->addr))
/** Set complete address to zero */
#define ip4_addr_set_zero(ipaddr)     ((ipaddr)->addr = 0)
/** Set address to IPADDR_ANY (no need for lwip_htonl()) */
#define ip4_addr_set_any(ipaddr)      ((ipaddr)->addr = IPADDR_ANY)
/** Set address to loopback address */
#define ip4_addr_set_loopback(ipaddr) ((ipaddr)->addr = PP_HTONL(IPADDR_LOOPBACK))
/** Check if an address is in the loopback region */
#define ip4_addr_isloopback(ipaddr)   (((ipaddr)->addr & PP_HTONL(IP_CLASSA_NET)) == PP_HTONL(((u32)IP_LOOPBACKNET) << 24))
/** Safely copy one IP address to another and change byte order
 * from host- to network-order. */
#define ip4_addr_set_hton(dest, src)  ((dest)->addr =          \
                                          ((src) == NULL ? 0 : \
                                                           lwip_htonl((src)->addr)))
/** IPv4 only: set the IP address given as an u32 */
#define ip4_addr_set_u32(dest_ipaddr, src_u32) ((dest_ipaddr)->addr = (src_u32))
/** IPv4 only: get the IP address as an u32 */
#define ip4_addr_get_u32(src_ipaddr)           ((src_ipaddr)->addr)

/** Get the network address by combining host address with netmask */
#define ip4_addr_get_network(target, host, netmask)            \
    do {                                                       \
        ((target)->addr = ((host)->addr) & ((netmask)->addr)); \
    } while (0)

/**
 * Determine if two address are on the same network.
 *
 * @arg addr1 IP address 1
 * @arg addr2 IP address 2
 * @arg mask network identifier mask
 * @return !0 if the network identifiers of both address match
 */
#define ip4_addr_netcmp(addr1, addr2, mask) (((addr1)->addr &  \
                                              (mask)->addr) == \
                                             ((addr2)->addr &  \
                                              (mask)->addr))
#define ip4_addr_cmp(addr1, addr2) ((addr1)->addr == (addr2)->addr)

#define ip4_addr_isany_val(addr1) ((addr1).addr == IPADDR_ANY)
#define ip4_addr_isany(addr1)     ((addr1) == NULL || ip4_addr_isany_val(*(addr1)))

#define ip4_addr_isbroadcast(addr1, netif) ip4_addr_isbroadcast_u32((addr1)->addr, netif)

#define ip_addr_netmask_valid(netmask) ip4_addr_netmask_valid((netmask)->addr)

#define ip4_addr_ismulticast(addr1) (((addr1)->addr & PP_HTONL(0xf0000000UL)) == PP_HTONL(0xe0000000UL))

#define ip4_addr_islinklocal(addr1) (((addr1)->addr & PP_HTONL(0xffff0000UL)) == PP_HTONL(0xa9fe0000UL))

#define ip4_addr_debug_print_parts(debug, a, b, c, d) \
    LWIP_DEBUGF(debug, ("%" U16_F ".%" U16_F ".%" U16_F ".%" U16_F, a, b, c, d))
#define ip4_addr_debug_print(debug, ipaddr)                                        \
    ip4_addr_debug_print_parts(debug,                                              \
                               (u16)((ipaddr) != NULL ? ip4_addr1_16(ipaddr) : 0), \
                               (u16)((ipaddr) != NULL ? ip4_addr2_16(ipaddr) : 0), \
                               (u16)((ipaddr) != NULL ? ip4_addr3_16(ipaddr) : 0), \
                               (u16)((ipaddr) != NULL ? ip4_addr4_16(ipaddr) : 0))
#define ip4_addr_debug_print_val(debug, ipaddr)         \
    ip4_addr_debug_print_parts(debug,                   \
                               ip4_addr1_16(&(ipaddr)), \
                               ip4_addr2_16(&(ipaddr)), \
                               ip4_addr3_16(&(ipaddr)), \
                               ip4_addr4_16(&(ipaddr)))

/* Get one byte from the 4-byte address */
#define ip4_addr1(ipaddr)    (((const u8 *)(&(ipaddr)->addr))[0])
#define ip4_addr2(ipaddr)    (((const u8 *)(&(ipaddr)->addr))[1])
#define ip4_addr3(ipaddr)    (((const u8 *)(&(ipaddr)->addr))[2])
#define ip4_addr4(ipaddr)    (((const u8 *)(&(ipaddr)->addr))[3])
/* These are cast to u16, with the intent that they are often arguments
 * to printf using the U16_F format from cc.h. */
#define ip4_addr1_16(ipaddr) ((u16)ip4_addr1(ipaddr))
#define ip4_addr2_16(ipaddr) ((u16)ip4_addr2(ipaddr))
#define ip4_addr3_16(ipaddr) ((u16)ip4_addr3(ipaddr))
#define ip4_addr4_16(ipaddr) ((u16)ip4_addr4(ipaddr))

#define IP4ADDR_STRLEN_MAX 16

/* From include/lwip/ip6_addr.h:  */
/** This is the aligned version of ip6_addr_t,
    used as local variable, on the stack, etc. */
struct ip6_addr
{
    u32 addr[4];
};

/** IPv6 address */
typedef struct ip6_addr ip6_addr_t;
typedef struct ip6_addr_packed ip6_addr_p_t;

/** Set an IPv6 partial address given by byte-parts.
Little-endian version, stored in network order (no lwip_htonl). */
#define IP6_ADDR_PART(ip6addr, index, a, b, c, d)      \
    (ip6addr)->addr[index] = ((u32)((d)&0xff) << 24) | \
                             ((u32)((c)&0xff) << 16) | \
                             ((u32)((b)&0xff) << 8) |  \
                             (u32)((a)&0xff)

/** Set a full IPv6 address by passing the 4 u32 indices in network byte order
    (use PP_HTONL() for constants) */
#define IP6_ADDR(ip6addr, idx0, idx1, idx2, idx3) \
    do {                                          \
        (ip6addr)->addr[0] = idx0;                \
        (ip6addr)->addr[1] = idx1;                \
        (ip6addr)->addr[2] = idx2;                \
        (ip6addr)->addr[3] = idx3;                \
    } while (0)

/** Access address in 16-bit block */
#define IP6_ADDR_BLOCK1(ip6addr) ((u16)((lwip_htonl((ip6addr)->addr[0]) >> 16) & 0xffff))
/** Access address in 16-bit block */
#define IP6_ADDR_BLOCK2(ip6addr) ((u16)((lwip_htonl((ip6addr)->addr[0])) & 0xffff))
/** Access address in 16-bit block */
#define IP6_ADDR_BLOCK3(ip6addr) ((u16)((lwip_htonl((ip6addr)->addr[1]) >> 16) & 0xffff))
/** Access address in 16-bit block */
#define IP6_ADDR_BLOCK4(ip6addr) ((u16)((lwip_htonl((ip6addr)->addr[1])) & 0xffff))
/** Access address in 16-bit block */
#define IP6_ADDR_BLOCK5(ip6addr) ((u16)((lwip_htonl((ip6addr)->addr[2]) >> 16) & 0xffff))
/** Access address in 16-bit block */
#define IP6_ADDR_BLOCK6(ip6addr) ((u16)((lwip_htonl((ip6addr)->addr[2])) & 0xffff))
/** Access address in 16-bit block */
#define IP6_ADDR_BLOCK7(ip6addr) ((u16)((lwip_htonl((ip6addr)->addr[3]) >> 16) & 0xffff))
/** Access address in 16-bit block */
#define IP6_ADDR_BLOCK8(ip6addr) ((u16)((lwip_htonl((ip6addr)->addr[3])) & 0xffff))

/** Copy IPv6 address - faster than ip6_addr_set: no NULL check */
#define ip6_addr_copy(dest, src)        \
    do {                                \
        (dest).addr[0] = (src).addr[0]; \
        (dest).addr[1] = (src).addr[1]; \
        (dest).addr[2] = (src).addr[2]; \
        (dest).addr[3] = (src).addr[3]; \
    } while (0)
/** Safely copy one IPv6 address to another (src may be NULL) */
#define ip6_addr_set(dest, src)                               \
    do {                                                      \
        (dest)->addr[0] = (src) == NULL ? 0 : (src)->addr[0]; \
        (dest)->addr[1] = (src) == NULL ? 0 : (src)->addr[1]; \
        (dest)->addr[2] = (src) == NULL ? 0 : (src)->addr[2]; \
        (dest)->addr[3] = (src) == NULL ? 0 : (src)->addr[3]; \
    } while (0)

/** Set complete address to zero */
#define ip6_addr_set_zero(ip6addr) \
    do {                           \
        (ip6addr)->addr[0] = 0;    \
        (ip6addr)->addr[1] = 0;    \
        (ip6addr)->addr[2] = 0;    \
        (ip6addr)->addr[3] = 0;    \
    } while (0)

/** Set address to ipv6 'any' (no need for lwip_htonl()) */
#define ip6_addr_set_any(ip6addr) ip6_addr_set_zero(ip6addr)
/** Set address to ipv6 loopback address */
#define ip6_addr_set_loopback(ip6addr)               \
    do {                                             \
        (ip6addr)->addr[0] = 0;                      \
        (ip6addr)->addr[1] = 0;                      \
        (ip6addr)->addr[2] = 0;                      \
        (ip6addr)->addr[3] = PP_HTONL(0x00000001UL); \
    } while (0)
/** Safely copy one IPv6 address to another and change byte order
 * from host- to network-order. */
#define ip6_addr_set_hton(dest, src)                                      \
    do {                                                                  \
        (dest)->addr[0] = (src) == NULL ? 0 : lwip_htonl((src)->addr[0]); \
        (dest)->addr[1] = (src) == NULL ? 0 : lwip_htonl((src)->addr[1]); \
        (dest)->addr[2] = (src) == NULL ? 0 : lwip_htonl((src)->addr[2]); \
        (dest)->addr[3] = (src) == NULL ? 0 : lwip_htonl((src)->addr[3]); \
    } while (0)


/**
 * Determine if two IPv6 address are on the same network.
 *
 * @arg addr1 IPv6 address 1
 * @arg addr2 IPv6 address 2
 * @return !0 if the network identifiers of both address match
 */
#define ip6_addr_netcmp(addr1, addr2) (((addr1)->addr[0] == (addr2)->addr[0]) && \
                                       ((addr1)->addr[1] == (addr2)->addr[1]))

#define ip6_addr_cmp(addr1, addr2) (((addr1)->addr[0] == (addr2)->addr[0]) && \
                                    ((addr1)->addr[1] == (addr2)->addr[1]) && \
                                    ((addr1)->addr[2] == (addr2)->addr[2]) && \
                                    ((addr1)->addr[3] == (addr2)->addr[3]))

#define ip6_get_subnet_id(ip6addr) (lwip_htonl((ip6addr)->addr[2]) & 0x0000ffffUL)

#define ip6_addr_isany_val(ip6addr) (((ip6addr).addr[0] == 0) && \
                                     ((ip6addr).addr[1] == 0) && \
                                     ((ip6addr).addr[2] == 0) && \
                                     ((ip6addr).addr[3] == 0))
#define ip6_addr_isany(ip6addr) (((ip6addr) == NULL) || ip6_addr_isany_val(*(ip6addr)))

#define ip6_addr_isloopback(ip6addr) (((ip6addr)->addr[0] == 0UL) && \
                                      ((ip6addr)->addr[1] == 0UL) && \
                                      ((ip6addr)->addr[2] == 0UL) && \
                                      ((ip6addr)->addr[3] == PP_HTONL(0x00000001UL)))

#define ip6_addr_isglobal(ip6addr) (((ip6addr)->addr[0] & PP_HTONL(0xe0000000UL)) == PP_HTONL(0x20000000UL))

#define ip6_addr_islinklocal(ip6addr) (((ip6addr)->addr[0] & PP_HTONL(0xffc00000UL)) == PP_HTONL(0xfe800000UL))

#define ip6_addr_issitelocal(ip6addr) (((ip6addr)->addr[0] & PP_HTONL(0xffc00000UL)) == PP_HTONL(0xfec00000UL))

#define ip6_addr_isuniquelocal(ip6addr) (((ip6addr)->addr[0] & PP_HTONL(0xfe000000UL)) == PP_HTONL(0xfc000000UL))

#define ip6_addr_isipv6mappedipv4(ip6addr) (((ip6addr)->addr[0] == 0) && ((ip6addr)->addr[1] == 0) && (((ip6addr)->addr[2]) == PP_HTONL(0x0000FFFFUL)))

#define ip6_addr_ismulticast(ip6addr)               (((ip6addr)->addr[0] & PP_HTONL(0xff000000UL)) == PP_HTONL(0xff000000UL))
#define ip6_addr_multicast_transient_flag(ip6addr)  ((ip6addr)->addr[0] & PP_HTONL(0x00100000UL))
#define ip6_addr_multicast_prefix_flag(ip6addr)     ((ip6addr)->addr[0] & PP_HTONL(0x00200000UL))
#define ip6_addr_multicast_rendezvous_flag(ip6addr) ((ip6addr)->addr[0] & PP_HTONL(0x00400000UL))
#define ip6_addr_multicast_scope(ip6addr)           ((lwip_htonl((ip6addr)->addr[0]) >> 16) & 0xf)
#define IP6_MULTICAST_SCOPE_RESERVED                0x0
#define IP6_MULTICAST_SCOPE_RESERVED0               0x0
#define IP6_MULTICAST_SCOPE_INTERFACE_LOCAL         0x1
#define IP6_MULTICAST_SCOPE_LINK_LOCAL              0x2
#define IP6_MULTICAST_SCOPE_RESERVED3               0x3
#define IP6_MULTICAST_SCOPE_ADMIN_LOCAL             0x4
#define IP6_MULTICAST_SCOPE_SITE_LOCAL              0x5
#define IP6_MULTICAST_SCOPE_ORGANIZATION_LOCAL      0x8
#define IP6_MULTICAST_SCOPE_GLOBAL                  0xe
#define IP6_MULTICAST_SCOPE_RESERVEDF               0xf
#define ip6_addr_ismulticast_iflocal(ip6addr)       (((ip6addr)->addr[0] & PP_HTONL(0xff8f0000UL)) == PP_HTONL(0xff010000UL))
#define ip6_addr_ismulticast_linklocal(ip6addr)     (((ip6addr)->addr[0] & PP_HTONL(0xff8f0000UL)) == PP_HTONL(0xff020000UL))
#define ip6_addr_ismulticast_adminlocal(ip6addr)    (((ip6addr)->addr[0] & PP_HTONL(0xff8f0000UL)) == PP_HTONL(0xff040000UL))
#define ip6_addr_ismulticast_sitelocal(ip6addr)     (((ip6addr)->addr[0] & PP_HTONL(0xff8f0000UL)) == PP_HTONL(0xff050000UL))
#define ip6_addr_ismulticast_orglocal(ip6addr)      (((ip6addr)->addr[0] & PP_HTONL(0xff8f0000UL)) == PP_HTONL(0xff080000UL))
#define ip6_addr_ismulticast_global(ip6addr)        (((ip6addr)->addr[0] & PP_HTONL(0xff8f0000UL)) == PP_HTONL(0xff0e0000UL))

/* @todo define get/set for well-know multicast addresses, e.g. ff02::1 */
#define ip6_addr_isallnodes_iflocal(ip6addr) (((ip6addr)->addr[0] == PP_HTONL(0xff010000UL)) && \
                                              ((ip6addr)->addr[1] == 0UL) &&                    \
                                              ((ip6addr)->addr[2] == 0UL) &&                    \
                                              ((ip6addr)->addr[3] == PP_HTONL(0x00000001UL)))

#define ip6_addr_isallnodes_linklocal(ip6addr) (((ip6addr)->addr[0] == PP_HTONL(0xff020000UL)) && \
                                                ((ip6addr)->addr[1] == 0UL) &&                    \
                                                ((ip6addr)->addr[2] == 0UL) &&                    \
                                                ((ip6addr)->addr[3] == PP_HTONL(0x00000001UL)))
#define ip6_addr_set_allnodes_linklocal(ip6addr)     \
    do {                                             \
        (ip6addr)->addr[0] = PP_HTONL(0xff020000UL); \
        (ip6addr)->addr[1] = 0;                      \
        (ip6addr)->addr[2] = 0;                      \
        (ip6addr)->addr[3] = PP_HTONL(0x00000001UL); \
    } while (0)

#define ip6_addr_isallrouters_linklocal(ip6addr) (((ip6addr)->addr[0] == PP_HTONL(0xff020000UL)) && \
                                                  ((ip6addr)->addr[1] == 0UL) &&                    \
                                                  ((ip6addr)->addr[2] == 0UL) &&                    \
                                                  ((ip6addr)->addr[3] == PP_HTONL(0x00000002UL)))
#define ip6_addr_set_allrouters_linklocal(ip6addr)   \
    do {                                             \
        (ip6addr)->addr[0] = PP_HTONL(0xff020000UL); \
        (ip6addr)->addr[1] = 0;                      \
        (ip6addr)->addr[2] = 0;                      \
        (ip6addr)->addr[3] = PP_HTONL(0x00000002UL); \
    } while (0)

#define ip6_addr_issolicitednode(ip6addr) (((ip6addr)->addr[0] == PP_HTONL(0xff020000UL)) && \
                                           ((ip6addr)->addr[2] == PP_HTONL(0x00000001UL)) && \
                                           (((ip6addr)->addr[3] & PP_HTONL(0xff000000UL)) == PP_HTONL(0xff000000UL)))

#define ip6_addr_set_solicitednode(ip6addr, if_id)               \
    do {                                                         \
        (ip6addr)->addr[0] = PP_HTONL(0xff020000UL);             \
        (ip6addr)->addr[1] = 0;                                  \
        (ip6addr)->addr[2] = PP_HTONL(0x00000001UL);             \
        (ip6addr)->addr[3] = (PP_HTONL(0xff000000UL) | (if_id)); \
    } while (0)

#define ip6_addr_cmp_solicitednode(ip6addr, sn_addr) (((ip6addr)->addr[0] == PP_HTONL(0xff020000UL)) && \
                                                      ((ip6addr)->addr[1] == 0) &&                      \
                                                      ((ip6addr)->addr[2] == PP_HTONL(0x00000001UL)) && \
                                                      ((ip6addr)->addr[3] == (PP_HTONL(0xff000000UL) | (sn_addr)->addr[3])))

/* IPv6 address states. */
#define IP6_ADDR_INVALID     0x00
#define IP6_ADDR_TENTATIVE   0x08
#define IP6_ADDR_TENTATIVE_1 0x09 /* 1 probe sent */
#define IP6_ADDR_TENTATIVE_2 0x0a /* 2 probes sent */
#define IP6_ADDR_TENTATIVE_3 0x0b /* 3 probes sent */
#define IP6_ADDR_TENTATIVE_4 0x0c /* 4 probes sent */
#define IP6_ADDR_TENTATIVE_5 0x0d /* 5 probes sent */
#define IP6_ADDR_TENTATIVE_6 0x0e /* 6 probes sent */
#define IP6_ADDR_TENTATIVE_7 0x0f /* 7 probes sent */
#define IP6_ADDR_VALID       0x10 /* This bit marks an address as valid (preferred or deprecated) */
#define IP6_ADDR_PREFERRED   0x30
#define IP6_ADDR_DEPRECATED  0x10 /* Same as VALID (valid but not preferred) */

#define IP6_ADDR_TENTATIVE_COUNT_MASK 0x07 /* 1-7 probes sent */

#define ip6_addr_isinvalid(addr_state)    (addr_state == IP6_ADDR_INVALID)
#define ip6_addr_istentative(addr_state)  (addr_state & IP6_ADDR_TENTATIVE)
#define ip6_addr_isvalid(addr_state)      (addr_state & IP6_ADDR_VALID) /* Include valid, preferred, and deprecated. */
#define ip6_addr_ispreferred(addr_state)  (addr_state == IP6_ADDR_PREFERRED)
#define ip6_addr_isdeprecated(addr_state) (addr_state == IP6_ADDR_DEPRECATED)

#define ip6_addr_debug_print_parts(debug, a, b, c, d, e, f, g, h)                                               \
    LWIP_DEBUGF(debug, ("%" X16_F ":%" X16_F ":%" X16_F ":%" X16_F ":%" X16_F ":%" X16_F ":%" X16_F ":%" X16_F, \
                        a, b, c, d, e, f, g, h))
#define ip6_addr_debug_print(debug, ipaddr)                                           \
    ip6_addr_debug_print_parts(debug,                                                 \
                               (u16)((ipaddr) != NULL ? IP6_ADDR_BLOCK1(ipaddr) : 0), \
                               (u16)((ipaddr) != NULL ? IP6_ADDR_BLOCK2(ipaddr) : 0), \
                               (u16)((ipaddr) != NULL ? IP6_ADDR_BLOCK3(ipaddr) : 0), \
                               (u16)((ipaddr) != NULL ? IP6_ADDR_BLOCK4(ipaddr) : 0), \
                               (u16)((ipaddr) != NULL ? IP6_ADDR_BLOCK5(ipaddr) : 0), \
                               (u16)((ipaddr) != NULL ? IP6_ADDR_BLOCK6(ipaddr) : 0), \
                               (u16)((ipaddr) != NULL ? IP6_ADDR_BLOCK7(ipaddr) : 0), \
                               (u16)((ipaddr) != NULL ? IP6_ADDR_BLOCK8(ipaddr) : 0))
#define ip6_addr_debug_print_val(debug, ipaddr)            \
    ip6_addr_debug_print_parts(debug,                      \
                               IP6_ADDR_BLOCK1(&(ipaddr)), \
                               IP6_ADDR_BLOCK2(&(ipaddr)), \
                               IP6_ADDR_BLOCK3(&(ipaddr)), \
                               IP6_ADDR_BLOCK4(&(ipaddr)), \
                               IP6_ADDR_BLOCK5(&(ipaddr)), \
                               IP6_ADDR_BLOCK6(&(ipaddr)), \
                               IP6_ADDR_BLOCK7(&(ipaddr)), \
                               IP6_ADDR_BLOCK8(&(ipaddr)))

#define IP6ADDR_STRLEN_MAX 46

/* From include/lwip/ip_addr.h:  */

/** @ingroup ipaddr
 * IP address types for use in ip_addr_t.type member.
 * @see tcp_new_ip_type(), udp_new_ip_type(), raw_new_ip_type().
 */
enum lwip_ip_addr_type {
    /** IPv4 */
    IPADDR_TYPE_V4 = 0U,
    /** IPv6 */
    IPADDR_TYPE_V6 = 6U,
    /** IPv4+IPv6 ("dual-stack") */
    IPADDR_TYPE_ANY = 46U
};

#if LWIP_IPV4 && LWIP_IPV6
/**
 * @ingroup ipaddr
 * A union struct for both IP version's addresses.
 * ATTENTION: watch out for its size when adding IPv6 address scope!
 */
typedef struct _ip_addr
{
    union
    {
        ip6_addr_t ip6;
        ip4_addr_t ip4;
    } uaddr;
    /** @ref lwip_ip_addr_type */
    u8 type;
} ip_addr_t;

extern const ip_addr_t ip_addr_any_type;

/** @ingroup ip4addr */
#define IPADDR4_INIT(u32val)                        \
    {                                               \
        {{{u32val, 0ul, 0ul, 0ul}}}, IPADDR_TYPE_V4 \
    }
/** @ingroup ip6addr */
#define IPADDR6_INIT(a, b, c, d)         \
    {                                    \
        {{{a, b, c, d}}}, IPADDR_TYPE_V6 \
    }

/** @ingroup ipaddr */
#define IP_IS_ANY_TYPE_VAL(ipaddr) (IP_GET_TYPE(&ipaddr) == IPADDR_TYPE_ANY)
/** @ingroup ipaddr */
#define IPADDR_ANY_TYPE_INIT                      \
    {                                             \
        {{{0ul, 0ul, 0ul, 0ul}}}, IPADDR_TYPE_ANY \
    }

/** @ingroup ip4addr */
#define IP_IS_V4_VAL(ipaddr) (IP_GET_TYPE(&ipaddr) == IPADDR_TYPE_V4)
/** @ingroup ip6addr */
#define IP_IS_V6_VAL(ipaddr) (IP_GET_TYPE(&ipaddr) == IPADDR_TYPE_V6)
/** @ingroup ip4addr */
#define IP_IS_V4(ipaddr)     (((ipaddr) == NULL) || IP_IS_V4_VAL(*(ipaddr)))
/** @ingroup ip6addr */
#define IP_IS_V6(ipaddr)     (((ipaddr) != NULL) && IP_IS_V6_VAL(*(ipaddr)))

#define IP_SET_TYPE_VAL(ipaddr, iptype) \
    do {                                \
        (ipaddr).type = (iptype);       \
    } while (0)
#define IP_SET_TYPE(ipaddr, iptype)             \
    do {                                        \
        if ((ipaddr) != NULL) {                 \
            IP_SET_TYPE_VAL(*(ipaddr), iptype); \
        }                                       \
    } while (0)
#define IP_GET_TYPE(ipaddr) ((ipaddr)->type)

#define IP_ADDR_PCB_VERSION_MATCH_EXACT(pcb, ipaddr) (IP_GET_TYPE(&pcb->local_ip) == IP_GET_TYPE(ipaddr))
#define IP_ADDR_PCB_VERSION_MATCH(pcb, ipaddr)       (IP_IS_ANY_TYPE_VAL(pcb->local_ip) || IP_ADDR_PCB_VERSION_MATCH_EXACT(pcb, ipaddr))

/** @ingroup ip6addr
 * Convert generic ip address to specific protocol version
 */
#define ip_2_ip6(ipaddr) (&((ipaddr)->uaddr.ip6))
/** @ingroup ip4addr
 * Convert generic ip address to specific protocol version
 */
#define ip_2_ip4(ipaddr) (&((ipaddr)->uaddr.ip4))

/** @ingroup ip4addr */
#define IP_ADDR4(ipaddr, a, b, c, d)                \
    do {                                            \
        IP4_ADDR(ip_2_ip4(ipaddr), a, b, c, d);     \
        IP_SET_TYPE_VAL(*(ipaddr), IPADDR_TYPE_V4); \
    } while (0)
/** @ingroup ip6addr */
#define IP_ADDR6(ipaddr, i0, i1, i2, i3)            \
    do {                                            \
        IP6_ADDR(ip_2_ip6(ipaddr), i0, i1, i2, i3); \
        IP_SET_TYPE_VAL(*(ipaddr), IPADDR_TYPE_V6); \
    } while (0)

/** @ingroup ipaddr */
#define ip_addr_copy(dest, src)                                   \
    do {                                                          \
        IP_SET_TYPE_VAL(dest, IP_GET_TYPE(&src));                 \
        if (IP_IS_V6_VAL(src)) {                                  \
            ip6_addr_copy(*ip_2_ip6(&(dest)), *ip_2_ip6(&(src))); \
        } else {                                                  \
            ip4_addr_copy(*ip_2_ip4(&(dest)), *ip_2_ip4(&(src))); \
        }                                                         \
    } while (0)
/** @ingroup ip6addr */
#define ip_addr_copy_from_ip6(dest, src)        \
    do {                                        \
        ip6_addr_copy(*ip_2_ip6(&(dest)), src); \
        IP_SET_TYPE_VAL(dest, IPADDR_TYPE_V6);  \
    } while (0)
/** @ingroup ip4addr */
#define ip_addr_copy_from_ip4(dest, src)        \
    do {                                        \
        ip4_addr_copy(*ip_2_ip4(&(dest)), src); \
        IP_SET_TYPE_VAL(dest, IPADDR_TYPE_V4);  \
    } while (0)
/** @ingroup ip4addr */
#define ip_addr_set_ip4_u32(ipaddr, val)             \
    do {                                             \
        if (ipaddr) {                                \
            ip4_addr_set_u32(ip_2_ip4(ipaddr), val); \
            IP_SET_TYPE(ipaddr, IPADDR_TYPE_V4);     \
        }                                            \
    } while (0)
/** @ingroup ip4addr */
#define ip_addr_get_ip4_u32(ipaddr) (((ipaddr) && IP_IS_V4(ipaddr)) ?         \
                                         ip4_addr_get_u32(ip_2_ip4(ipaddr)) : \
                                         0)
/** @ingroup ipaddr */
#define ip_addr_set(dest, src)                           \
    do {                                                 \
        IP_SET_TYPE(dest, IP_GET_TYPE(src));             \
        if (IP_IS_V6(src)) {                             \
            ip6_addr_set(ip_2_ip6(dest), ip_2_ip6(src)); \
        } else {                                         \
            ip4_addr_set(ip_2_ip4(dest), ip_2_ip4(src)); \
        }                                                \
    } while (0)
/** @ingroup ipaddr */
#define ip_addr_set_ipaddr(dest, src) ip_addr_set(dest, src)
/** @ingroup ipaddr */
#define ip_addr_set_zero(ipaddr)             \
    do {                                     \
        ip6_addr_set_zero(ip_2_ip6(ipaddr)); \
        IP_SET_TYPE(ipaddr, 0);              \
    } while (0)
/** @ingroup ip5addr */
#define ip_addr_set_zero_ip4(ipaddr)         \
    do {                                     \
        ip6_addr_set_zero(ip_2_ip6(ipaddr)); \
        IP_SET_TYPE(ipaddr, IPADDR_TYPE_V4); \
    } while (0)
/** @ingroup ip6addr */
#define ip_addr_set_zero_ip6(ipaddr)         \
    do {                                     \
        ip6_addr_set_zero(ip_2_ip6(ipaddr)); \
        IP_SET_TYPE(ipaddr, IPADDR_TYPE_V6); \
    } while (0)
/** @ingroup ipaddr */
#define ip_addr_set_any(is_ipv6, ipaddr)         \
    do {                                         \
        if (is_ipv6) {                           \
            ip6_addr_set_any(ip_2_ip6(ipaddr));  \
            IP_SET_TYPE(ipaddr, IPADDR_TYPE_V6); \
        } else {                                 \
            ip4_addr_set_any(ip_2_ip4(ipaddr));  \
            IP_SET_TYPE(ipaddr, IPADDR_TYPE_V4); \
        }                                        \
    } while (0)
/** @ingroup ipaddr */
#define ip_addr_set_loopback(is_ipv6, ipaddr)        \
    do {                                             \
        if (is_ipv6) {                               \
            ip6_addr_set_loopback(ip_2_ip6(ipaddr)); \
            IP_SET_TYPE(ipaddr, IPADDR_TYPE_V6);     \
        } else {                                     \
            ip4_addr_set_loopback(ip_2_ip4(ipaddr)); \
            IP_SET_TYPE(ipaddr, IPADDR_TYPE_V4);     \
        }                                            \
    } while (0)
/** @ingroup ipaddr */
#define ip_addr_set_hton(dest, src)                     \
    do {                                                \
        if (IP_IS_V6(src)) {                            \
            ip6_addr_set_hton(ip_2_ip6(ipaddr), (src)); \
            IP_SET_TYPE(dest, IPADDR_TYPE_V6);          \
        } else {                                        \
            ip4_addr_set_hton(ip_2_ip4(ipaddr), (src)); \
            IP_SET_TYPE(dest, IPADDR_TYPE_V4);          \
        }                                               \
    } while (0)
/** @ingroup ipaddr */
#define ip_addr_get_network(target, host, netmask)                                     \
    do {                                                                               \
        if (IP_IS_V6(host)) {                                                          \
            ip4_addr_set_zero(ip_2_ip4(target));                                       \
            IP_SET_TYPE(target, IPADDR_TYPE_V6);                                       \
        } else {                                                                       \
            ip4_addr_get_network(ip_2_ip4(target), ip_2_ip4(host), ip_2_ip4(netmask)); \
            IP_SET_TYPE(target, IPADDR_TYPE_V4);                                       \
        }                                                                              \
    } while (0)
/** @ingroup ipaddr */
#define ip_addr_netcmp(addr1, addr2, mask) ((IP_IS_V6(addr1) && IP_IS_V6(addr2)) ? \
                                                0 :                                \
                                                ip4_addr_netcmp(ip_2_ip4(addr1), ip_2_ip4(addr2), mask))
/** @ingroup ipaddr */
#define ip_addr_cmp(addr1, addr2) ((IP_GET_TYPE(addr1) != IP_GET_TYPE(addr2)) ? 0 : (IP_IS_V6_VAL(*(addr1)) ? ip6_addr_cmp(ip_2_ip6(addr1), ip_2_ip6(addr2)) : ip4_addr_cmp(ip_2_ip4(addr1), ip_2_ip4(addr2))))
/** @ingroup ipaddr */
#define ip_addr_isany(ipaddr)     ((IP_IS_V6(ipaddr)) ?               \
                                   ip6_addr_isany(ip_2_ip6(ipaddr)) : \
                                   ip4_addr_isany(ip_2_ip4(ipaddr)))
/** @ingroup ipaddr */
#define ip_addr_isany_val(ipaddr) ((IP_IS_V6_VAL(ipaddr)) ?                       \
                                       ip6_addr_isany_val(*ip_2_ip6(&(ipaddr))) : \
                                       ip4_addr_isany_val(*ip_2_ip4(&(ipaddr))))
/** @ingroup ipaddr */
#define ip_addr_isbroadcast(ipaddr, netif) ((IP_IS_V6(ipaddr)) ? \
                                                0 :              \
                                                ip4_addr_isbroadcast(ip_2_ip4(ipaddr), netif))
/** @ingroup ipaddr */
#define ip_addr_ismulticast(ipaddr) ((IP_IS_V6(ipaddr)) ?                         \
                                         ip6_addr_ismulticast(ip_2_ip6(ipaddr)) : \
                                         ip4_addr_ismulticast(ip_2_ip4(ipaddr)))
/** @ingroup ipaddr */
#define ip_addr_isloopback(ipaddr) ((IP_IS_V6(ipaddr)) ?                        \
                                        ip6_addr_isloopback(ip_2_ip6(ipaddr)) : \
                                        ip4_addr_isloopback(ip_2_ip4(ipaddr)))
/** @ingroup ipaddr */
#define ip_addr_islinklocal(ipaddr) ((IP_IS_V6(ipaddr)) ?                         \
                                         ip6_addr_islinklocal(ip_2_ip6(ipaddr)) : \
                                         ip4_addr_islinklocal(ip_2_ip4(ipaddr)))
#define ip_addr_debug_print(debug, ipaddr)                 \
    do {                                                   \
        if (IP_IS_V6(ipaddr)) {                            \
            ip6_addr_debug_print(debug, ip_2_ip6(ipaddr)); \
        } else {                                           \
            ip4_addr_debug_print(debug, ip_2_ip4(ipaddr)); \
        }                                                  \
    } while (0)
#define ip_addr_debug_print_val(debug, ipaddr)                     \
    do {                                                           \
        if (IP_IS_V6_VAL(ipaddr)) {                                \
            ip6_addr_debug_print_val(debug, *ip_2_ip6(&(ipaddr))); \
        } else {                                                   \
            ip4_addr_debug_print_val(debug, *ip_2_ip4(&(ipaddr))); \
        }                                                          \
    } while (0)
/** @ingroup ipaddr */
#define ipaddr_ntoa(addr) (((addr) == NULL) ? "NULL" : \
                                              ((IP_IS_V6(addr)) ? ip6addr_ntoa(ip_2_ip6(addr)) : ip4addr_ntoa(ip_2_ip4(addr))))
/** @ingroup ipaddr */
#define ipaddr_ntoa_r(addr, buf, buflen) (((addr) == NULL) ? "NULL" : \
                                                             ((IP_IS_V6(addr)) ? ip6addr_ntoa_r(ip_2_ip6(addr), buf, buflen) : ip4addr_ntoa_r(ip_2_ip4(addr), buf, buflen)))
int ipaddr_aton(const char *cp, ip_addr_t *addr);

/** @ingroup ipaddr */
#define IPADDR_STRLEN_MAX IP6ADDR_STRLEN_MAX

#else /* LWIP_IPV4 && LWIP_IPV6 */

#define IP_ADDR_PCB_VERSION_MATCH(addr, pcb)         1
#define IP_ADDR_PCB_VERSION_MATCH_EXACT(pcb, ipaddr) 1

#if LWIP_IPV4

typedef ip4_addr_t ip_addr_t;
#define IPADDR4_INIT(u32val) \
    {                        \
        u32val               \
    }
#define IP_IS_V4_VAL(ipaddr)       1
#define IP_IS_V6_VAL(ipaddr)       0
#define IP_IS_V4(ipaddr)           1
#define IP_IS_V6(ipaddr)           0
#define IP_IS_ANY_TYPE_VAL(ipaddr) 0
#define IP_SET_TYPE_VAL(ipaddr, iptype)
#define IP_SET_TYPE(ipaddr, iptype)
#define IP_GET_TYPE(ipaddr)          IPADDR_TYPE_V4
#define ip_2_ip4(ipaddr)             (ipaddr)
#define IP_ADDR4(ipaddr, a, b, c, d) IP4_ADDR(ipaddr, a, b, c, d)

#define ip_addr_copy(dest, src)                 ip4_addr_copy(dest, src)
#define ip_addr_copy_from_ip4(dest, src)        ip4_addr_copy(dest, src)
#define ip_addr_set_ip4_u32(ipaddr, val)        ip4_addr_set_u32(ip_2_ip4(ipaddr), val)
#define ip_addr_get_ip4_u32(ipaddr)             ip4_addr_get_u32(ip_2_ip4(ipaddr))
#define ip_addr_set(dest, src)                  ip4_addr_set(dest, src)
#define ip_addr_set_ipaddr(dest, src)           ip4_addr_set(dest, src)
#define ip_addr_set_zero(ipaddr)                ip4_addr_set_zero(ipaddr)
#define ip_addr_set_zero_ip4(ipaddr)            ip4_addr_set_zero(ipaddr)
#define ip_addr_set_any(is_ipv6, ipaddr)        ip4_addr_set_any(ipaddr)
#define ip_addr_set_loopback(is_ipv6, ipaddr)   ip4_addr_set_loopback(ipaddr)
#define ip_addr_set_hton(dest, src)             ip4_addr_set_hton(dest, src)
#define ip_addr_get_network(target, host, mask) ip4_addr_get_network(target, host, mask)
#define ip_addr_netcmp(addr1, addr2, mask)      ip4_addr_netcmp(addr1, addr2, mask)
#define ip_addr_cmp(addr1, addr2)               ip4_addr_cmp(addr1, addr2)
#define ip_addr_isany(ipaddr)                   ip4_addr_isany(ipaddr)
#define ip_addr_isany_val(ipaddr)               ip4_addr_isany_val(ipaddr)
#define ip_addr_isloopback(ipaddr)              ip4_addr_isloopback(ipaddr)
#define ip_addr_islinklocal(ipaddr)             ip4_addr_islinklocal(ipaddr)
#define ip_addr_isbroadcast(addr, netif)        ip4_addr_isbroadcast(addr, netif)
#define ip_addr_ismulticast(ipaddr)             ip4_addr_ismulticast(ipaddr)
#define ip_addr_debug_print(debug, ipaddr)      ip4_addr_debug_print(debug, ipaddr)
#define ip_addr_debug_print_val(debug, ipaddr)  ip4_addr_debug_print_val(debug, ipaddr)
#define ipaddr_ntoa(ipaddr)                     ip4addr_ntoa(ipaddr)
#define ipaddr_ntoa_r(ipaddr, buf, buflen)      ip4addr_ntoa_r(ipaddr, buf, buflen)
#define ipaddr_aton(cp, addr)                   ip4addr_aton(cp, addr)

#define IPADDR_STRLEN_MAX IP4ADDR_STRLEN_MAX

#else /* LWIP_IPV4 */

typedef ip6_addr_t ip_addr_t;
#define IPADDR6_INIT(a, b, c, d) \
    {                            \
        {                        \
            a, b, c, d           \
        }                        \
    }
#define IP_IS_V4_VAL(ipaddr)       0
#define IP_IS_V6_VAL(ipaddr)       1
#define IP_IS_V4(ipaddr)           0
#define IP_IS_V6(ipaddr)           1
#define IP_IS_ANY_TYPE_VAL(ipaddr) 0
#define IP_SET_TYPE_VAL(ipaddr, iptype)
#define IP_SET_TYPE(ipaddr, iptype)
#define IP_GET_TYPE(ipaddr)              IPADDR_TYPE_V6
#define ip_2_ip6(ipaddr)                 (ipaddr)
#define IP_ADDR6(ipaddr, i0, i1, i2, i3) IP6_ADDR(ipaddr, i0, i1, i2, i3)

#define ip_addr_copy(dest, src)                 ip6_addr_copy(dest, src)
#define ip_addr_copy_from_ip6(dest, src)        ip6_addr_copy(dest, src)
#define ip_addr_set(dest, src)                  ip6_addr_set(dest, src)
#define ip_addr_set_ipaddr(dest, src)           ip6_addr_set(dest, src)
#define ip_addr_set_zero(ipaddr)                ip6_addr_set_zero(ipaddr)
#define ip_addr_set_zero_ip6(ipaddr)            ip6_addr_set_zero(ipaddr)
#define ip_addr_set_any(is_ipv6, ipaddr)        ip6_addr_set_any(ipaddr)
#define ip_addr_set_loopback(is_ipv6, ipaddr)   ip6_addr_set_loopback(ipaddr)
#define ip_addr_set_hton(dest, src)             ip6_addr_set_hton(dest, src)
#define ip_addr_get_network(target, host, mask) ip6_addr_set_zero(target)
#define ip_addr_netcmp(addr1, addr2, mask)      0
#define ip_addr_cmp(addr1, addr2)               ip6_addr_cmp(addr1, addr2)
#define ip_addr_isany(ipaddr)                   ip6_addr_isany(ipaddr)
#define ip_addr_isany_val(ipaddr)               ip6_addr_isany_val(ipaddr)
#define ip_addr_isloopback(ipaddr)              ip6_addr_isloopback(ipaddr)
#define ip_addr_islinklocal(ipaddr)             ip6_addr_islinklocal(ipaddr)
#define ip_addr_isbroadcast(addr, netif)        0
#define ip_addr_ismulticast(ipaddr)             ip6_addr_ismulticast(ipaddr)
#define ip_addr_debug_print(debug, ipaddr)      ip6_addr_debug_print(debug, ipaddr)
#define ip_addr_debug_print_val(debug, ipaddr)  ip6_addr_debug_print_val(debug, ipaddr)
#define ipaddr_ntoa(ipaddr)                     ip6addr_ntoa(ipaddr)
#define ipaddr_ntoa_r(ipaddr, buf, buflen)      ip6addr_ntoa_r(ipaddr, buf, buflen)
#define ipaddr_aton(cp, addr)                   ip6addr_aton(cp, addr)

#define IPADDR_STRLEN_MAX IP6ADDR_STRLEN_MAX

#endif /* LWIP_IPV4 */
#endif /* LWIP_IPV4 && LWIP_IPV6 */

#if LWIP_IPV4

extern const ip_addr_t ip_addr_any;
extern const ip_addr_t ip_addr_broadcast;

/**
 * @ingroup ip4addr
 * Provided for compatibility. Use IP4_ADDR_ANY for better readability.
 */
#define IP_ADDR_ANY   IP4_ADDR_ANY
/**
 * @ingroup ip4addr
 * Can be used as a fixed/const ip_addr_t
 * for the IPv4 wildcard and the broadcast address
 */
#define IP4_ADDR_ANY  (&ip_addr_any)
/**
 * @ingroup ip4addr
 * Can be used as a fixed/const ip4_addr_t
 * for the wildcard and the broadcast address
 */
#define IP4_ADDR_ANY4 (ip_2_ip4(&ip_addr_any))

/** @ingroup ip4addr */
#define IP_ADDR_BROADCAST  (&ip_addr_broadcast)
/** @ingroup ip4addr */
#define IP4_ADDR_BROADCAST (ip_2_ip4(&ip_addr_broadcast))

#endif /* LWIP_IPV4*/

#if LWIP_IPV6

extern const ip_addr_t ip6_addr_any;

/**
 * @ingroup ip6addr
 * IP6_ADDR_ANY can be used as a fixed ip_addr_t
 * for the IPv6 wildcard address
 */
#define IP6_ADDR_ANY  (&ip6_addr_any)
/**
 * @ingroup ip6addr
 * IP6_ADDR_ANY6 can be used as a fixed ip6_addr_t
 * for the IPv6 wildcard address
 */
#define IP6_ADDR_ANY6 (ip_2_ip6(&ip6_addr_any))

#if !LWIP_IPV4
/** Just a little upgrade-helper for IPv6-only configurations: */
#define IP_ADDR_ANY IP6_ADDR_ANY
#endif /* !LWIP_IPV4 */

#endif

#if LWIP_IPV4 && LWIP_IPV6
/** @ingroup ipaddr */
#define IP_ANY_TYPE (&ip_addr_any_type)
#else
#define IP_ANY_TYPE IP_ADDR_ANY
#endif

/* From src/include/lwip/netif.h:  */


/* Throughout this file, IP addresses are expected to be in
 * the same byte order as in IP_PCB. */

/** Must be the maximum of all used hardware address lengths
    across all types of interfaces in use.
    This does not have to be changed, normally. */
#ifndef NETIF_MAX_HWADDR_LEN
#define NETIF_MAX_HWADDR_LEN 6U
#endif

/**
 * @defgroup netif_flags Flags
 * @ingroup netif
 * @{
 */

/** Whether the network interface is 'up'. This is
 * a software flag used to control whether this network
 * interface is enabled and processes traffic.
 * It must be set by the startup code before this netif can be used
 * (also for dhcp/autoip).
 */
#define NETIF_FLAG_UP        0x01U
/** If set, the netif has broadcast capability.
 * Set by the netif driver in its init function. */
#define NETIF_FLAG_BROADCAST 0x02U
/** If set, the interface has an active link
 *  (set by the network interface driver).
 * Either set by the netif driver in its init function (if the link
 * is up at that time) or at a later point once the link comes up
 * (if link detection is supported by the hardware). */
#define NETIF_FLAG_LINK_UP   0x04U
/** If set, the netif is an ethernet device using ARP.
 * Set by the netif driver in its init function.
 * Used to check input packet types and use of DHCP. */
#define NETIF_FLAG_ETHARP    0x08U
/** If set, the netif is an ethernet device. It might not use
 * ARP or TCP/IP if it is used for PPPoE only.
 */
#define NETIF_FLAG_ETHERNET  0x10U
/** If set, the netif has IGMP capability.
 * Set by the netif driver in its init function. */
#define NETIF_FLAG_IGMP      0x20U
/** If set, the netif has MLD6 capability.
 * Set by the netif driver in its init function. */
#define NETIF_FLAG_MLD6      0x40U

/**
 * @}
 */

enum lwip_internal_netif_client_data_index {
#if LWIP_DHCP
    LWIP_NETIF_CLIENT_DATA_INDEX_DHCP,
#endif
#if LWIP_AUTOIP
    LWIP_NETIF_CLIENT_DATA_INDEX_AUTOIP,
#endif
#if LWIP_IGMP
    LWIP_NETIF_CLIENT_DATA_INDEX_IGMP,
#endif
#if LWIP_IPV6_MLD
    LWIP_NETIF_CLIENT_DATA_INDEX_MLD6,
#endif
    LWIP_NETIF_CLIENT_DATA_INDEX_MAX
};

#if LWIP_CHECKSUM_CTRL_PER_NETIF
#define NETIF_CHECKSUM_GEN_IP      0x0001
#define NETIF_CHECKSUM_GEN_UDP     0x0002
#define NETIF_CHECKSUM_GEN_TCP     0x0004
#define NETIF_CHECKSUM_GEN_ICMP    0x0008
#define NETIF_CHECKSUM_GEN_ICMP6   0x0010
#define NETIF_CHECKSUM_CHECK_IP    0x0100
#define NETIF_CHECKSUM_CHECK_UDP   0x0200
#define NETIF_CHECKSUM_CHECK_TCP   0x0400
#define NETIF_CHECKSUM_CHECK_ICMP  0x0800
#define NETIF_CHECKSUM_CHECK_ICMP6 0x1000
#define NETIF_CHECKSUM_ENABLE_ALL  0xFFFF
#define NETIF_CHECKSUM_DISABLE_ALL 0x0000
#endif /* LWIP_CHECKSUM_CTRL_PER_NETIF */

struct netif;

/** MAC Filter Actions, these are passed to a netif's igmp_mac_filter or
 * mld_mac_filter callback function. */
enum netif_mac_filter_action {
    /** Delete a filter entry */
    NETIF_DEL_MAC_FILTER = 0,
    /** Add a filter entry */
    NETIF_ADD_MAC_FILTER = 1
};

/** Function prototype for netif init functions. Set up flags and output/linkoutput
 * callback functions in this function.
 *
 * @param netif The netif to initialize
 */
typedef err_t (*netif_init_fn)(struct netif *netif);
/** Function prototype for netif->input functions. This function is saved as 'input'
 * callback function in the netif struct. Call it when a packet has been received.
 *
 * @param p The received packet, copied into a pbuf
 * @param inp The netif which received the packet
 */
typedef err_t (*netif_input_fn)(struct pbuf *p, struct netif *inp);

#if LWIP_IPV4
/** Function prototype for netif->output functions. Called by lwIP when a packet
 * shall be sent. For ethernet netif, set this to 'etharp_output' and set
 * 'linkoutput'.
 *
 * @param netif The netif which shall send a packet
 * @param p The packet to send (p->payload points to IP header)
 * @param ipaddr The IP address to which the packet shall be sent
 */
typedef err_t (*netif_output_fn)(struct netif *netif, struct pbuf *p,
                                 const ip4_addr_t *ipaddr);
#endif /* LWIP_IPV4*/

#if LWIP_IPV6
/** Function prototype for netif->output_ip6 functions. Called by lwIP when a packet
 * shall be sent. For ethernet netif, set this to 'ethip6_output' and set
 * 'linkoutput'.
 *
 * @param netif The netif which shall send a packet
 * @param p The packet to send (p->payload points to IP header)
 * @param ipaddr The IPv6 address to which the packet shall be sent
 */
typedef err_t (*netif_output_ip6_fn)(struct netif *netif, struct pbuf *p,
                                     const ip6_addr_t *ipaddr);
#endif /* LWIP_IPV6 */

/** Function prototype for netif->linkoutput functions. Only used for ethernet
 * netifs. This function is called by ARP when a packet shall be sent.
 *
 * @param netif The netif which shall send a packet
 * @param p The packet to send (raw ethernet packet)
 */
typedef err_t (*netif_linkoutput_fn)(struct netif *netif, struct pbuf *p);
/** Function prototype for netif status- or link-callback functions. */
typedef void (*netif_status_callback_fn)(struct netif *netif);
#if LWIP_IPV4 && LWIP_IGMP
/** Function prototype for netif igmp_mac_filter functions */
typedef err_t (*netif_igmp_mac_filter_fn)(struct netif *netif,
                                          const ip4_addr_t *group, enum netif_mac_filter_action action);
#endif /* LWIP_IPV4 && LWIP_IGMP */
#if LWIP_IPV6 && LWIP_IPV6_MLD
/** Function prototype for netif mld_mac_filter functions */
typedef err_t (*netif_mld_mac_filter_fn)(struct netif *netif,
                                         const ip6_addr_t *group, enum netif_mac_filter_action action);
#endif /* LWIP_IPV6 && LWIP_IPV6_MLD */

#if LWIP_DHCP || LWIP_AUTOIP || LWIP_IGMP || LWIP_IPV6_MLD || (LWIP_NUM_NETIF_CLIENT_DATA > 0)
u8 netif_alloc_client_data_id(void);
/** @ingroup netif_cd
 * Set client data. Obtain ID from netif_alloc_client_data_id().
 */
#define netif_set_client_data(netif, id, data) netif_get_client_data(netif, id) = (data)
/** @ingroup netif_cd
 * Get client data. Obtain ID from netif_alloc_client_data_id().
 */
#define netif_get_client_data(netif, id)       (netif)->client_data[(id)]
#endif /* LWIP_DHCP || LWIP_AUTOIP || (LWIP_NUM_NETIF_CLIENT_DATA > 0) */

/** Generic data structure used for all lwIP network interfaces.
 *  The following fields should be filled in by the initialization
 *  function for the device driver: hwaddr_len, hwaddr[], mtu, flags */
struct netif
{
    /** pointer to next in linked list */
    struct netif *next;

#if LWIP_IPV4
    /** IP address configuration in network byte order */
    ip_addr_t ip_addr;
    ip_addr_t netmask;
    ip_addr_t gw;
#endif /* LWIP_IPV4 */
#if LWIP_IPV6
    /** Array of IPv6 addresses for this netif. */
    ip_addr_t ip6_addr[LWIP_IPV6_NUM_ADDRESSES];
    /** The state of each IPv6 address (Tentative, Preferred, etc).
     * @see ip6_addr.h */
    u8 ip6_addr_state[LWIP_IPV6_NUM_ADDRESSES];
#endif /* LWIP_IPV6 */
    /** This function is called by the network device driver
     *  to pass a packet up the TCP/IP stack. */
    netif_input_fn input;
#if LWIP_IPV4
    /** This function is called by the IP module when it wants
     *  to send a packet on the interface. This function typically
     *  first resolves the hardware address, then sends the packet.
     *  For ethernet physical layer, this is usually etharp_output() */
    netif_output_fn output;
#endif /* LWIP_IPV4 */
    /** This function is called by ethernet_output() when it wants
     *  to send a packet on the interface. This function outputs
     *  the pbuf as-is on the link medium. */
    netif_linkoutput_fn linkoutput;
#if LWIP_IPV6
    /** This function is called by the IPv6 module when it wants
     *  to send a packet on the interface. This function typically
     *  first resolves the hardware address, then sends the packet.
     *  For ethernet physical layer, this is usually ethip6_output() */
    netif_output_ip6_fn output_ip6;
#endif /* LWIP_IPV6 */
#if LWIP_NETIF_STATUS_CALLBACK
    /** This function is called when the netif state is set to up or down
     */
    netif_status_callback_fn status_callback;
#endif /* LWIP_NETIF_STATUS_CALLBACK */
#if LWIP_NETIF_LINK_CALLBACK
    /** This function is called when the netif link is set to up or down
     */
    netif_status_callback_fn link_callback;
#endif /* LWIP_NETIF_LINK_CALLBACK */
#if LWIP_NETIF_REMOVE_CALLBACK
    /** This function is called when the netif has been removed */
    netif_status_callback_fn remove_callback;
#endif /* LWIP_NETIF_REMOVE_CALLBACK */
    /** This field can be set by the device driver and could point
     *  to state information for the device. */
    void *state;
#ifdef netif_get_client_data
    void *client_data[LWIP_NETIF_CLIENT_DATA_INDEX_MAX + LWIP_NUM_NETIF_CLIENT_DATA];
#endif
#if LWIP_IPV6_AUTOCONFIG
    /** is this netif enabled for IPv6 autoconfiguration */
    u8 ip6_autoconfig_enabled;
#endif /* LWIP_IPV6_AUTOCONFIG */
#if LWIP_IPV6_SEND_ROUTER_SOLICIT
    /** Number of Router Solicitation messages that remain to be sent. */
    u8 rs_count;
#endif /* LWIP_IPV6_SEND_ROUTER_SOLICIT */
#if LWIP_NETIF_HOSTNAME
    /* the hostname for this netif, NULL is a valid value */
    const char *hostname;
#endif /* LWIP_NETIF_HOSTNAME */
#if LWIP_CHECKSUM_CTRL_PER_NETIF
    u16 chksum_flags;
#endif /* LWIP_CHECKSUM_CTRL_PER_NETIF*/
    /** maximum transfer unit (in bytes) */
    u16 mtu;
    /** number of bytes used in hwaddr */
    u8 hwaddr_len;
    /** link level hardware address of this interface */
    u8 hwaddr[NETIF_MAX_HWADDR_LEN];
    /** flags (@see @ref netif_flags) */
    u8 flags;
    /** descriptive abbreviation */
    char name[2];
    /** number of this interface */
    u8 num;
#if MIB2_STATS
    /** link type (from "snmp_ifType" enum from snmp_mib2.h) */
    u8 link_type;
    /** (estimate) link speed */
    u32 link_speed;
    /** timestamp at last change made (up/down) */
    u32 ts;
    /** counters */
    struct stats_mib2_netif_ctrs mib2_counters;
#endif /* MIB2_STATS */
#if LWIP_IPV4 && LWIP_IGMP
    /** This function could be called to add or delete an entry in the multicast
        filter table of the ethernet MAC.*/
    netif_igmp_mac_filter_fn igmp_mac_filter;
#endif /* LWIP_IPV4 && LWIP_IGMP */
#if LWIP_IPV6 && LWIP_IPV6_MLD
    /** This function could be called to add or delete an entry in the IPv6 multicast
        filter table of the ethernet MAC. */
    netif_mld_mac_filter_fn mld_mac_filter;
#endif /* LWIP_IPV6 && LWIP_IPV6_MLD */
#if LWIP_NETIF_HWADDRHINT
    u8 *addr_hint;
#endif /* LWIP_NETIF_HWADDRHINT */
#if ENABLE_LOOPBACK
    /* List of packets to be queued for ourselves. */
    struct pbuf *loop_first;
    struct pbuf *loop_last;
#if LWIP_LOOPBACK_MAX_PBUFS
    u16 loop_cnt_current;
#endif /* LWIP_LOOPBACK_MAX_PBUFS */
#endif /* ENABLE_LOOPBACK */
};

#if LWIP_CHECKSUM_CTRL_PER_NETIF
#define NETIF_SET_CHECKSUM_CTRL(netif, chksumflags) \
    do {                                            \
        (netif)->chksum_flags = chksumflags;        \
    } while (0)
#define IF__NETIF_CHECKSUM_ENABLED(netif, chksumflag) if (((netif) == NULL) || (((netif)->chksum_flags & (chksumflag)) != 0))
#else /* LWIP_CHECKSUM_CTRL_PER_NETIF */
#define NETIF_SET_CHECKSUM_CTRL(netif, chksumflags)
#define IF__NETIF_CHECKSUM_ENABLED(netif, chksumflag)
#endif /* LWIP_CHECKSUM_CTRL_PER_NETIF */

/** The list of network interfaces. */
extern struct netif *netif_list;
/** The default network interface. */
extern struct netif *netif_default;

#if LWIP_IPV4
/** @ingroup netif_ip4 */
#define netif_ip4_addr(netif)    ((const ip4_addr_t *)ip_2_ip4(&((netif)->ip_addr)))
/** @ingroup netif_ip4 */
#define netif_ip4_netmask(netif) ((const ip4_addr_t *)ip_2_ip4(&((netif)->netmask)))
/** @ingroup netif_ip4 */
#define netif_ip4_gw(netif)      ((const ip4_addr_t *)ip_2_ip4(&((netif)->gw)))
/** @ingroup netif_ip4 */
#define netif_ip_addr4(netif)    ((const ip_addr_t *)&((netif)->ip_addr))
/** @ingroup netif_ip4 */
#define netif_ip_netmask4(netif) ((const ip_addr_t *)&((netif)->netmask))
/** @ingroup netif_ip4 */
#define netif_ip_gw4(netif)      ((const ip_addr_t *)&((netif)->gw))
#endif /* LWIP_IPV4 */

/** @ingroup netif
 * Ask if an interface is up
 */
#define netif_is_up(netif) (((netif)->flags & NETIF_FLAG_UP) ? (u8)1 : (u8)0)

/** Ask if a link is up */
#define netif_is_link_up(netif) (((netif)->flags & NETIF_FLAG_LINK_UP) ? (u8)1 : (u8)0)

#if LWIP_NETIF_HOSTNAME
/** @ingroup netif */
#define netif_set_hostname(netif, name) \
    do {                                \
        if ((netif) != NULL) {          \
            (netif)->hostname = name;   \
        }                               \
    } while (0)
/** @ingroup netif */
#define netif_get_hostname(netif) (((netif) != NULL) ? ((netif)->hostname) : NULL)
#endif /* LWIP_NETIF_HOSTNAME */

#if LWIP_IGMP
/** @ingroup netif */
#define netif_set_igmp_mac_filter(netif, function) \
    do {                                           \
        if ((netif) != NULL) {                     \
            (netif)->igmp_mac_filter = function;   \
        }                                          \
    } while (0)
#define netif_get_igmp_mac_filter(netif) (((netif) != NULL) ? ((netif)->igmp_mac_filter) : NULL)
#endif /* LWIP_IGMP */

#if LWIP_IPV6 && LWIP_IPV6_MLD
/** @ingroup netif */
#define netif_set_mld_mac_filter(netif, function) \
    do {                                          \
        if ((netif) != NULL) {                    \
            (netif)->mld_mac_filter = function;   \
        }                                         \
    } while (0)
#define netif_get_mld_mac_filter(netif) (((netif) != NULL) ? ((netif)->mld_mac_filter) : NULL)
#define netif_mld_mac_filter(netif, addr, action)               \
    do {                                                        \
        if ((netif) && (netif)->mld_mac_filter) {               \
            (netif)->mld_mac_filter((netif), (addr), (action)); \
        }                                                       \
    } while (0)
#endif /* LWIP_IPV6 && LWIP_IPV6_MLD */

#if LWIP_IPV6
/** @ingroup netif_ip6 */
#define netif_ip_addr6(netif, i) ((const ip_addr_t *)(&((netif)->ip6_addr[i])))
/** @ingroup netif_ip6 */
#define netif_ip6_addr(netif, i) ((const ip6_addr_t *)ip_2_ip6(&((netif)->ip6_addr[i])))
void netif_ip6_addr_set(struct netif *netif, s8_t addr_idx, const ip6_addr_t *addr6);
void netif_ip6_addr_set_parts(struct netif *netif, s8_t addr_idx, u32 i0, u32 i1, u32 i2, u32 i3);
#define netif_ip6_addr_state(netif, i) ((netif)->ip6_addr_state[i])
#define netif_set_ip6_autoconfig_enabled(netif, action) \
    do {                                                \
        if (netif) {                                    \
            (netif)->ip6_autoconfig_enabled = (action); \
        }                                               \
    } while (0)
#endif /* LWIP_IPV6 */

#if LWIP_NETIF_HWADDRHINT
#define NETIF_SET_HWADDRHINT(netif, hint) ((netif)->addr_hint = (hint))
#else /* LWIP_NETIF_HWADDRHINT */
#define NETIF_SET_HWADDRHINT(netif, hint)
#endif /* LWIP_NETIF_HWADDRHINT */

/* From include/lwip/prot/dhcp.h: */
/* DHCP client states */
typedef enum {
    DHCP_STATE_OFF         = 0,
    DHCP_STATE_REQUESTING  = 1,
    DHCP_STATE_INIT        = 2,
    DHCP_STATE_REBOOTING   = 3,
    DHCP_STATE_REBINDING   = 4,
    DHCP_STATE_RENEWING    = 5,
    DHCP_STATE_SELECTING   = 6,
    DHCP_STATE_INFORMING   = 7,
    DHCP_STATE_CHECKING    = 8,
    DHCP_STATE_PERMANENT   = 9, /* not yet implemented */
    DHCP_STATE_BOUND       = 10,
    DHCP_STATE_RELEASING   = 11, /* not yet implemented */
    DHCP_STATE_BACKING_OFF = 12
} dhcp_state_enum_t;

/* From include/lwip/inet.h: */

struct in_addr
{
    u32 s_addr;
};

struct in6_addr
{
    union
    {
        u32 u32_addr[4];
        u8 u8_addr[16];
    } un;
#define s6_addr un.u8_addr
};

/** 255.255.255.255 */
#define INADDR_NONE      IPADDR_NONE
/** 127.0.0.1 */
#define INADDR_LOOPBACK  IPADDR_LOOPBACK
/** 0.0.0.0 */
#define INADDR_ANY       IPADDR_ANY
/** 255.255.255.255 */
#define INADDR_BROADCAST IPADDR_BROADCAST

/* If your port already typedef's sa_family_t, define SA_FAMILY_T_DEFINED
   to prevent this code from redefining it. */
#if !defined(sa_family_t) && !defined(SA_FAMILY_T_DEFINED)
typedef u8 sa_family_t;
#endif
/* If your port already typedef's in_port_t, define IN_PORT_T_DEFINED
   to prevent this code from redefining it. */
#if !defined(in_port_t) && !defined(IN_PORT_T_DEFINED)
typedef u16 in_port_t;
#endif

#if LWIP_IPV4
/* members are in network byte order */
struct sockaddr_in
{
    u8 sin_len;
    sa_family_t sin_family;
    in_port_t sin_port;
    struct in_addr sin_addr;
#define SIN_ZERO_LEN 8
    char sin_zero[SIN_ZERO_LEN];
};
#endif /* LWIP_IPV4 */

#if LWIP_IPV6
struct sockaddr_in6
{
    u8 sin6_len;               /* length of this structure    */
    sa_family_t sin6_family;   /* AF_INET6                    */
    in_port_t sin6_port;       /* Transport layer port #      */
    u32 sin6_flowinfo;         /* IPv6 flow information       */
    struct in6_addr sin6_addr; /* IPv6 address                */
    u32 sin6_scope_id;         /* Set of interfaces for scope */
};
#endif /* LWIP_IPV6 */

struct sockaddr
{
    u8 sa_len;
    sa_family_t sa_family;
    char sa_data[14];
};

struct sockaddr_storage
{
    u8 s2_len;
    sa_family_t ss_family;
    char s2_data1[2];
    u32 s2_data2[3];
#if LWIP_IPV6
    u32 s2_data3[3];
#endif /* LWIP_IPV6 */
};

/* Define SOCKLEN_T_DEFINED to prevent other code from redefining socklen_t. */
#define SOCKLEN_T_DEFINED 1
typedef int socklen_t;

/* Socket protocol types (TCP/UDP/RAW) */
#define SOCK_STREAM 1
#define SOCK_DGRAM  2
#define SOCK_RAW    3

/*
 * Option flags per-socket. These must match the SOF_ flags in ip.h (checked in init.c)
 */
#define SO_REUSEADDR 0x0004 /* Allow local address reuse */
#define SO_KEEPALIVE 0x0008 /* keep connections alive */
#define SO_BROADCAST 0x0020 /* permit to send and to receive broadcast messages (see IP_SOF_BROADCAST option) */


/*
 * Additional options, not kept in so_options.
 */
#define SO_DEBUG       0x0001 /* Unimplemented: turn on debugging info recording */
#define SO_ACCEPTCONN  0x0002 /* socket has had listen() */
#define SO_DONTROUTE   0x0010 /* Unimplemented: just use interface addresses */
#define SO_USELOOPBACK 0x0040 /* Unimplemented: bypass hardware when possible */
#define SO_LINGER      0x0080 /* linger on close if data present */
#define SO_DONTLINGER  ((int)(~SO_LINGER))
#define SO_OOBINLINE   0x0100 /* Unimplemented: leave received OOB data in line */
#define SO_REUSEPORT   0x0200 /* Unimplemented: allow local address & port reuse */
#define SO_SNDBUF      0x1001 /* Unimplemented: send buffer size */
#define SO_RCVBUF      0x1002 /* receive buffer size */
#define SO_SNDLOWAT    0x1003 /* Unimplemented: send low-water mark */
#define SO_RCVLOWAT    0x1004 /* Unimplemented: receive low-water mark */
#define SO_SNDTIMEO    0x1005 /* send timeout */
#define SO_RCVTIMEO    0x1006 /* receive timeout */
#define SO_ERROR       0x1007 /* get error status and clear */
#define SO_TYPE        0x1008 /* get socket type */
#define SO_CONTIMEO    0x1009 /* Unimplemented: connect timeout */
#define SO_NO_CHECK    0x100a /* don't create UDP checksum */

/*
 * Level number for (get/set)sockopt() to apply to socket itself.
 */
#define SOL_SOCKET 0xfff /* options for socket level */

#define AF_UNSPEC 0
#define AF_INET   2
#if LWIP_IPV6
#define AF_INET6 10
#else /* LWIP_IPV6 */
#define AF_INET6 AF_UNSPEC
#endif /* LWIP_IPV6 */
#define PF_INET   AF_INET
#define PF_INET6  AF_INET6
#define PF_UNSPEC AF_UNSPEC

#define IPPROTO_IP   0
#define IPPROTO_ICMP 1
#define IPPROTO_TCP  6
#define IPPROTO_UDP  17
#if LWIP_IPV6
#define IPPROTO_IPV6   41
#define IPPROTO_ICMPV6 58
#endif /* LWIP_IPV6 */
#define IPPROTO_UDPLITE 136
#define IPPROTO_RAW     255

/* Flags we can use with send and recv. */
#define MSG_PEEK     0x01 /* Peeks at an incoming message */
#define MSG_WAITALL  0x02 /* Unimplemented: Requests that the function block until the full amount of data requested can be returned */
#define MSG_OOB      0x04 /* Unimplemented: Requests out-of-band data. The significance and semantics of out-of-band data are protocol-specific */
#define MSG_DONTWAIT 0x08 /* Nonblocking i/o for this operation only */
#define MSG_MORE     0x10 /* Sender will send more */

/*
 * Options for level IPPROTO_IP
 */
#define IP_TOS 1
#define IP_TTL 2

#if LWIP_TCP
/*
 * Options for level IPPROTO_TCP
 */
#define TCP_NODELAY   0x01 /* don't delay send to coalesce packets */
#define TCP_KEEPALIVE 0x02 /* send KEEPALIVE probes when idle for pcb->keep_idle milliseconds */
#define TCP_KEEPIDLE  0x03 /* set pcb->keep_idle  - Same as TCP_KEEPALIVE, but use seconds for get/setsockopt */
#define TCP_KEEPINTVL 0x04 /* set pcb->keep_intvl - Use seconds for get/setsockopt */
#define TCP_KEEPCNT   0x05 /* set pcb->keep_cnt   - Use number of probes sent for get/setsockopt */
#endif                     /* LWIP_TCP */

#if LWIP_IPV6
/*
 * Options for level IPPROTO_IPV6
 */
#define IPV6_CHECKSUM 7  /* RFC3542: calculate and insert the ICMPv6 checksum for raw sockets. */
#define IPV6_V6ONLY   27 /* RFC3493: boolean control to restrict AF_INET6 sockets to IPv6 communications only. */
#endif                   /* LWIP_IPV6 */

#if LWIP_UDP && LWIP_UDPLITE
/*
 * Options for level IPPROTO_UDPLITE
 */
#define UDPLITE_SEND_CSCOV 0x01 /* sender checksum coverage */
#define UDPLITE_RECV_CSCOV 0x02 /* minimal receiver checksum coverage */
#endif                          /* LWIP_UDP && LWIP_UDPLITE*/


#if LWIP_MULTICAST_TX_OPTIONS
/*
 * Options and types for UDP multicast traffic handling
 */
#define IP_MULTICAST_TTL  5
#define IP_MULTICAST_IF   6
#define IP_MULTICAST_LOOP 7
#endif /* LWIP_MULTICAST_TX_OPTIONS */

#if LWIP_IGMP
/*
 * Options and types related to multicast membership
 */
#define IP_ADD_MEMBERSHIP  3
#define IP_DROP_MEMBERSHIP 4

typedef struct ip_mreq
{
    struct in_addr imr_multiaddr; /* IP multicast address of group */
    struct in_addr imr_interface; /* local IP address of interface */
} ip_mreq;
#endif /* LWIP_IGMP */

/*
 * The Type of Service provides an indication of the abstract
 * parameters of the quality of service desired.  These parameters are
 * to be used to guide the selection of the actual service parameters
 * when transmitting a datagram through a particular network.  Several
 * networks offer service precedence, which somehow treats high
 * precedence traffic as more important than other traffic (generally
 * by accepting only traffic above a certain precedence at time of high
 * load).  The major choice is a three way tradeoff between low-delay,
 * high-reliability, and high-throughput.
 * The use of the Delay, Throughput, and Reliability indications may
 * increase the cost (in some sense) of the service.  In many networks
 * better performance for one of these parameters is coupled with worse
 * performance on another.  Except for very unusual cases at most two
 * of these three indications should be set.
 */
#define IPTOS_TOS_MASK    0x1E
#define IPTOS_TOS(tos)    ((tos)&IPTOS_TOS_MASK)
#define IPTOS_LOWDELAY    0x10
#define IPTOS_THROUGHPUT  0x08
#define IPTOS_RELIABILITY 0x04
#define IPTOS_LOWCOST     0x02
#define IPTOS_MINCOST     IPTOS_LOWCOST

/*
 * The Network Control precedence designation is intended to be used
 * within a network only.  The actual use and control of that
 * designation is up to each network. The Internetwork Control
 * designation is intended for use by gateway control originators only.
 * If the actual use of these precedence designations is of concern to
 * a particular network, it is the responsibility of that network to
 * control the access to, and use of, those precedence designations.
 */
#define IPTOS_PREC_MASK            0xe0
#define IPTOS_PREC(tos)            ((tos)&IPTOS_PREC_MASK)
#define IPTOS_PREC_NETCONTROL      0xe0
#define IPTOS_PREC_INTERNETCONTROL 0xc0
#define IPTOS_PREC_CRITIC_ECP      0xa0
#define IPTOS_PREC_FLASHOVERRIDE   0x80
#define IPTOS_PREC_FLASH           0x60
#define IPTOS_PREC_IMMEDIATE       0x40
#define IPTOS_PREC_PRIORITY        0x20
#define IPTOS_PREC_ROUTINE         0x00


/*
 * Commands for ioctlsocket(),  taken from the BSD file fcntl.h.
 * lwip_ioctl only supports FIONREAD and FIONBIO, for now
 *
 * Ioctl's have the command encoded in the lower word,
 * and the size of any in or out parameters in the upper
 * word.  The high 2 bits of the upper word are used
 * to encode the in/out status of the parameter; for now
 * we restrict parameters to at most 128 bytes.
 */
#if !defined(FIONREAD) || !defined(FIONBIO)
#define IOCPARM_MASK 0x7fU        /* parameters must be < 128 bytes */
#define IOC_VOID     0x20000000UL /* no parameters */
#define IOC_OUT      0x40000000UL /* copy out parameters */
#define IOC_IN       0x80000000UL /* copy in parameters */
#define IOC_INOUT    (IOC_IN | IOC_OUT)
/* 0x20000000 distinguishes new &
   old ioctl's */
#define _IO(x, y)    (IOC_VOID | ((x) << 8) | (y))

#define _IOR(x, y, t) (IOC_OUT | (((long)sizeof(t) & IOCPARM_MASK) << 16) | ((x) << 8) | (y))

#define _IOW(x, y, t) (IOC_IN | (((long)sizeof(t) & IOCPARM_MASK) << 16) | ((x) << 8) | (y))
#endif /* !defined(FIONREAD) || !defined(FIONBIO) */

#ifndef FIONREAD
#define FIONREAD _IOR('f', 127, unsigned long) /* get # bytes to read */
#endif
#ifndef FIONBIO
#define FIONBIO _IOW('f', 126, unsigned long) /* set/clear non-blocking i/o */
#endif

/* Socket I/O Controls: unimplemented */
#ifndef SIOCSHIWAT
#define SIOCSHIWAT _IOW('s', 0, unsigned long) /* set high watermark */
#define SIOCGHIWAT _IOR('s', 1, unsigned long) /* get high watermark */
#define SIOCSLOWAT _IOW('s', 2, unsigned long) /* set low watermark */
#define SIOCGLOWAT _IOR('s', 3, unsigned long) /* get low watermark */
#define SIOCATMARK _IOR('s', 7, unsigned long) /* at oob mark? */
#endif

/* commands for fnctl */
#ifndef F_GETFL
#define F_GETFL 3
#endif
#ifndef F_SETFL
#define F_SETFL 4
#endif

/* File status flags and file access modes for fnctl,
   these are bits in an int. */
#ifndef O_NONBLOCK
#define O_NONBLOCK 1 /* nonblocking I/O */
#endif
#ifndef O_NDELAY
#define O_NDELAY 1 /* same as O_NONBLOCK, for compatibility */
#endif

#ifndef SHUT_RD
#define SHUT_RD   0
#define SHUT_WR   1
#define SHUT_RDWR 2
#endif

/* FD_SET used for lwip_select */
#ifndef FD_SET
#undef FD_SETSIZE
/* Make FD_SETSIZE match NUM_SOCKETS in socket.c */
#define FD_SETSIZE MEMP_NUM_NETCONN
#define FDSETSAFESET(n, code)                                                                      \
    do {                                                                                           \
        if (((n)-LWIP_SOCKET_OFFSET < MEMP_NUM_NETCONN) && (((int)(n)-LWIP_SOCKET_OFFSET) >= 0)) { \
            code;                                                                                  \
        }                                                                                          \
    } while (0)
#define FDSETSAFEGET(n, code) (((n)-LWIP_SOCKET_OFFSET < MEMP_NUM_NETCONN) && (((int)(n)-LWIP_SOCKET_OFFSET) >= 0) ? \
                                   (code) :                                                                          \
                                   0)
#define FD_SET(n, p)   FDSETSAFESET(n, (p)->fd_bits[((n)-LWIP_SOCKET_OFFSET) / 8] |= (1 << (((n)-LWIP_SOCKET_OFFSET) & 7)))
#define FD_CLR(n, p)   FDSETSAFESET(n, (p)->fd_bits[((n)-LWIP_SOCKET_OFFSET) / 8] &= ~(1 << (((n)-LWIP_SOCKET_OFFSET) & 7)))
#define FD_ISSET(n, p) FDSETSAFEGET(n, (p)->fd_bits[((n)-LWIP_SOCKET_OFFSET) / 8] & (1 << (((n)-LWIP_SOCKET_OFFSET) & 7)))
#define FD_ZERO(p)     memset((void *)(p), 0, sizeof(*(p)))

typedef struct fd_set
{
    unsigned char fd_bits[(FD_SETSIZE + 7) / 8];
} fd_set;

#elif LWIP_SOCKET_OFFSET
#error LWIP_SOCKET_OFFSET does not work with external FD_SET!
#elif FD_SETSIZE < MEMP_NUM_NETCONN
#error "external FD_SETSIZE too small for number of sockets"
#endif /* FD_SET */

#ifdef PS2IP_DNS
/** DNS timer period */
#define DNS_TMR_INTERVAL 1000

/* DNS resolve types: */
#define LWIP_DNS_ADDRTYPE_IPV4      0
#define LWIP_DNS_ADDRTYPE_IPV6      1
#define LWIP_DNS_ADDRTYPE_IPV4_IPV6 2 /* try to resolve IPv4 first, try IPv6 if IPv4 fails only */
#define LWIP_DNS_ADDRTYPE_IPV6_IPV4 3 /* try to resolve IPv6 first, try IPv4 if IPv6 fails only */
#if LWIP_IPV4 && LWIP_IPV6
#ifndef LWIP_DNS_ADDRTYPE_DEFAULT
#define LWIP_DNS_ADDRTYPE_DEFAULT LWIP_DNS_ADDRTYPE_IPV4_IPV6
#endif
#elif defined(LWIP_IPV4)
#define LWIP_DNS_ADDRTYPE_DEFAULT LWIP_DNS_ADDRTYPE_IPV4
#else
#define LWIP_DNS_ADDRTYPE_DEFAULT LWIP_DNS_ADDRTYPE_IPV6
#endif

#if DNS_LOCAL_HOSTLIST
/** struct used for local host-list */
struct local_hostlist_entry
{
    /** static hostname */
    const char *name;
    /** static host address in network byteorder */
    ip_addr_t addr;
    struct local_hostlist_entry *next;
};
#if DNS_LOCAL_HOSTLIST_IS_DYNAMIC
#ifndef DNS_LOCAL_HOSTLIST_MAX_NAMELEN
#define DNS_LOCAL_HOSTLIST_MAX_NAMELEN DNS_MAX_NAME_LENGTH
#endif
#define LOCALHOSTLIST_ELEM_SIZE ((sizeof(struct local_hostlist_entry) + DNS_LOCAL_HOSTLIST_MAX_NAMELEN + 1))
#endif /* DNS_LOCAL_HOSTLIST_IS_DYNAMIC */
#endif /* DNS_LOCAL_HOSTLIST */

/** Callback which is invoked when a hostname is found.
 * A function of this type must be implemented by the application using the DNS resolver.
 * @param name pointer to the name that was looked up.
 * @param ipaddr pointer to an ip_addr_t containing the IP address of the hostname,
 *        or NULL if the name could not be found (or on any other error).
 * @param callback_arg a user-specified callback argument passed to dns_gethostbyname
 */
typedef void (*dns_found_callback)(const char *name, const ip_addr_t *ipaddr, void *callback_arg);
#endif

/* From src/include/lwip/netdb.h:  */
/* some rarely used options */
#ifndef LWIP_DNS_API_DECLARE_H_ERRNO
#define LWIP_DNS_API_DECLARE_H_ERRNO 1
#endif

#ifndef LWIP_DNS_API_DEFINE_ERRORS
#define LWIP_DNS_API_DEFINE_ERRORS 1
#endif

#ifndef LWIP_DNS_API_DEFINE_FLAGS
#define LWIP_DNS_API_DEFINE_FLAGS 1
#endif

#ifndef LWIP_DNS_API_DECLARE_STRUCTS
#define LWIP_DNS_API_DECLARE_STRUCTS 1
#endif

#if LWIP_DNS_API_DEFINE_ERRORS
/** Errors used by the DNS API functions, h_errno can be one of them */
#define EAI_NONAME  200
#define EAI_SERVICE 201
#define EAI_FAIL    202
#define EAI_MEMORY  203
#define EAI_FAMILY  204

#define HOST_NOT_FOUND 210
#define NO_DATA        211
#define NO_RECOVERY    212
#define TRY_AGAIN      213
#endif /* LWIP_DNS_API_DEFINE_ERRORS */

#if LWIP_DNS_API_DEFINE_FLAGS
/* input flags for struct addrinfo */
#define AI_PASSIVE     0x01
#define AI_CANONNAME   0x02
#define AI_NUMERICHOST 0x04
#define AI_NUMERICSERV 0x08
#define AI_V4MAPPED    0x10
#define AI_ALL         0x20
#define AI_ADDRCONFIG  0x40
#endif /* LWIP_DNS_API_DEFINE_FLAGS */

#if LWIP_DNS_API_DECLARE_STRUCTS
struct hostent
{
    char *h_name;             /* Official name of the host. */
    char **h_aliases;         /* A pointer to an array of pointers to alternative host names,
                                 terminated by a null pointer. */
    int h_addrtype;           /* Address type. */
    int h_length;             /* The length, in bytes, of the address. */
    char **h_addr_list;       /* A pointer to an array of pointers to network addresses (in
                                 network byte order) for the host, terminated by a null pointer. */
#define h_addr h_addr_list[0] /* for backward compatibility */
};

struct addrinfo
{
    int ai_flags;             /* Input flags. */
    int ai_family;            /* Address family of socket. */
    int ai_socktype;          /* Socket type. */
    int ai_protocol;          /* Protocol of socket. */
    socklen_t ai_addrlen;     /* Length of socket address. */
    struct sockaddr *ai_addr; /* Socket address of socket. */
    char *ai_canonname;       /* Canonical name of service location. */
    struct addrinfo *ai_next; /* Pointer to next in list. */
};
#endif /* LWIP_DNS_API_DECLARE_STRUCTS */

#define NETDB_ELEM_SIZE (sizeof(struct addrinfo) + sizeof(struct sockaddr_storage) + DNS_MAX_NAME_LENGTH + 1)

/* From src/include/lwip/def.h:  */
#define htons(x) lwip_htons(x)
#define ntohs(x) lwip_ntohs(x)
#define htonl(x) lwip_htonl(x)
#define ntohl(x) lwip_ntohl(x)

#define lwip_htons(x) PP_HTONS(x)
#define lwip_ntohs(x) PP_NTOHS(x)
#define lwip_htonl(x) PP_HTONL(x)
#define lwip_ntohl(x) PP_NTOHL(x)

/* These macros should be calculated by the preprocessor and are used
   with compile-time constants only (so that there is no little-endian
   overhead at runtime). */
#define PP_HTONS(x) ((((x)&0xff) << 8) | (((x)&0xff00) >> 8))
#define PP_NTOHS(x) PP_HTONS(x)
#define PP_HTONL(x) ((((x)&0xff) << 24) |      \
                     (((x)&0xff00) << 8) |     \
                     (((x)&0xff0000UL) >> 8) | \
                     (((x)&0xff000000UL) >> 24))
#define PP_NTOHL(x) PP_HTONL(x)

/** The structure that contains the network interface state ***/
typedef struct
{
    char netif_name[4];
    struct in_addr ipaddr;
    struct in_addr netmask;
    struct in_addr gw;
    u32 dhcp_enabled;
    u32 dhcp_status;
    u8 hw_addr[8];
} t_ip_info;

struct timeval;

#endif /* __TCPIP_H__ */

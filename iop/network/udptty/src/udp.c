/*
# _____     ___ ____     ___ ____
#  ____|   |    ____|   |        | |____|
# |     ___|   |____ ___|    ____| |    \    PS2DEV Open Source Project.
#-----------------------------------------------------------------------
# Copyright (c) 2003  Marcus R. Brown <mrbrown@0xd6.org>
# Licenced under Academic Free License version 2.0
# Review ps2sdk README & LICENSE files for further details.
#
# $Id$
# UDP library for UDPTTY.
*/

#include "udptty.h"
#include "sysclib.h"

typedef struct { u8 addr[4]; } ip_addr_t __attribute__((packed));

typedef struct {
	/* Ethernet header (14).  */
	u8	eth_addr_dst[6];
	u8	eth_addr_src[6];
	u16	eth_type;

	/* IP header (20).  */
	u8	ip_hlen;
	u8	ip_tos;
	u16	ip_len;
	u16	ip_id;
	u8	ip_flags;
	u8	ip_frag_offset;
	u8	ip_ttl;
	u8	ip_proto;
	u16	ip_csum;
	ip_addr_t ip_addr_src;
	ip_addr_t ip_addr_dst;

	/* UDP header (8).  */
	u16	udp_port_src;
	u16	udp_port_dst;
	u16	udp_len;
	u16	udp_csum;

	/* Data goes here.  */
} udp_pkt_t __attribute__((packed));

/* The max we'll send is a MTU, 1514 bytes.  */
static u8 pktbuf[1514];

/* UDP */

static inline u16 htons(u16 n)
{ 
	return ((n & 0xff) << 8) | ((n & 0xff00) >> 8);
}

static u16 checksum(void *buf, size_t size)
{
	u16 *data = (u16 *)buf;
	u32 csum;

	for (csum = 0; size > 1; size -= 2, data++)
		csum += *data;

	if (size == 1)
		csum += htons((u16)((*(u8 *)data) & 0xff) << 8);

	csum = (csum >> 16) + (csum & 0xffff);
	if (csum & 0xffff0000)
		csum = (csum >> 16) + (csum & 0xffff);

	return (u16)csum;
}

int udp_init()
{
	udp_pkt_t *udp_pkt;

	/* Initialize the static elements of our UDP packet.  */
	udp_pkt = (udp_pkt_t *)pktbuf;

	memcpy(udp_pkt->eth_addr_dst, udptty_param.eth_addr_dst, 12);
	udp_pkt->eth_type = 0x0008;	/* Network byte order: 0x800 */

	udp_pkt->ip_hlen = 0x45;
	udp_pkt->ip_tos = 0;
	udp_pkt->ip_id = 0;
	udp_pkt->ip_flags = 0;
	udp_pkt->ip_frag_offset = 0;
	udp_pkt->ip_ttl = 64;
	udp_pkt->ip_proto = 0x11;
	memcpy(&udp_pkt->ip_addr_src.addr, &udptty_param.ip_addr_src, 4);
	memcpy(&udp_pkt->ip_addr_dst.addr, &udptty_param.ip_addr_dst, 4);

	udp_pkt->udp_port_src = udptty_param.ip_port_src;
	udp_pkt->udp_port_dst = udptty_param.ip_port_dst;

	return 0;
}

/* Copy the data into place, calculate the various checksums, and send the
   final packet.  */
int udp_send(void *buf, size_t size)
{
	udp_pkt_t *udp_pkt;
	size_t pktsize, udpsize;
	u32 csum;

	if ((size + sizeof(udp_pkt_t)) > sizeof(pktbuf))
		size = sizeof(pktbuf) - sizeof(udp_pkt_t);

	/* See if we can update our pointers into the TX FIFO.  */
	smap_txbd_check();

	udp_pkt = (udp_pkt_t *)pktbuf;
	pktsize = size + sizeof(udp_pkt_t);

	udp_pkt->ip_len = htons(pktsize - 14);	/* Subtract the ethernet header.  */

	udp_pkt->ip_csum = 0;
	csum = checksum(&udp_pkt->ip_hlen, 20);	/* Checksum the IP header (20 bytes).  */
	while (csum >> 16)
		csum = (csum & 0xffff) + (csum >> 16);
	udp_pkt->ip_csum = ~(csum & 0xffff);

	udpsize = htons(size + 8);		/* Size of the UDP header + data.  */
	udp_pkt->udp_len = udpsize;
	memcpy(pktbuf + sizeof(udp_pkt_t), buf, size);

	udp_pkt->udp_csum = 0;			/* Don't bother.  */

	return smap_transmit(pktbuf, pktsize);
}

/*
# _____     ___ ____     ___ ____
#  ____|   |    ____|   |        | |____|
# |     ___|   |____ ___|    ____| |    \    PS2DEV Open Source Project.
#-----------------------------------------------------------------------
# Copyright 2001-2004, ps2dev - http://www.ps2dev.org
# Licenced under Academic Free License version 2.0
# Review ps2sdk README & LICENSE files for further details.
*/

#ifndef IOP_PS2IP_INTERNAL_H
#define	IOP_PS2IP_INTERNAL_H

#include "lwip/sockets.h"

#define netif_dhcp_data(netif) ((struct dhcp*)(netif)->client_data[LWIP_NETIF_CLIENT_DATA_INDEX_DHCP])

typedef struct
{
	char		netif_name[4];
	struct in_addr	ipaddr;
	struct in_addr	netmask;
	struct in_addr	gw;
	u32		dhcp_enabled;
	u32		dhcp_status;
	u8		hw_addr[8];
} t_ip_info;

#ifdef DEBUG
#define	dbgprintf(args...)	printf(args)
#else
#define	dbgprintf(args...)
#endif

err_t ps2ip_input(struct pbuf *p, struct netif *inp);

#endif	// !defined(IOP_PS2IP_INTERNAL_H)

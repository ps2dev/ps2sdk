/*
# _____     ___ ____     ___ ____
#  ____|   |    ____|   |        | |____|
# |     ___|   |____ ___|    ____| |    \    PS2DEV Open Source Project.
#-----------------------------------------------------------------------
# Copyright 2001-2004, ps2dev - http://www.ps2dev.org
# Licenced under Academic Free License version 2.0
# Review ps2sdk README & LICENSE files for further details.
#
# $Id$
# PS2 TCP/IP STACK FOR IOP
*/

#include <types.h>
#include <stdio.h>
#include <intrman.h>
#include <loadcore.h>
#include <thbase.h>
#include <vblank.h>
#include <modload.h>
#include <sysclib.h>
#include <thevent.h>
#include <libsd.h>
#include <sysmem.h>
#include <lwip/memp.h>

#include <lwip/sys.h>
#include <lwip/tcpip.h>
#include <lwip/netif.h>
#include <lwip/dhcp.h>
#include <netif/loopif.h>
#include <netif/etharp.h>

#include "ps2ip_internal.h"


#if		defined(DEBUG)
#define	dbgprintf(args...)	printf(args)
#else
#define	dbgprintf(args...)	((void)0)
#endif


typedef struct pbuf		PBuf;
typedef struct netif		NetIF;
typedef struct ip_addr	IPAddr;

#define MODNAME	"TCP/IP Stack"
IRX_ID(MODNAME,1,3);

extern struct irx_export_table	_exp_ps2ip;

#if		defined(PS2IP_DHCP)
static int		iTimerDHCP=0;
#endif	//defined(PS2IP_DHCP)

//static NetIF	LoopIF;

int
ps2ip_getconfig(char* pszName,t_ip_info* pInfo)
{
	NetIF*	pNetIF=netif_find(pszName);

	if (pNetIF==NULL)
	{

		//Net interface not found.

		memset(pInfo,0,sizeof(*pInfo));
		return	0;
	}
	strcpy(pInfo->netif_name,pszName);
	pInfo->ipaddr.s_addr=pNetIF->ip_addr.addr;
	pInfo->netmask.s_addr=pNetIF->netmask.addr;
	pInfo->gw.s_addr=pNetIF->gw.addr;

	memcpy(pInfo->hw_addr,pNetIF->hwaddr,sizeof(pInfo->hw_addr));

#if		LWIP_DHCP

	if (pNetIF->dhcp)
	{
		pInfo->dhcp_enabled=1;
		pInfo->dhcp_status=pNetIF->dhcp->state;
	}
	else
	{
		pInfo->dhcp_enabled=0;
		pInfo->dhcp_status=0;
	}

#else

	pInfo->dhcp_enabled=0;

#endif

	return	1;
}


int
ps2ip_setconfig(t_ip_info* pInfo)
{
	NetIF*	pNetIF=netif_find(pInfo->netif_name);

	if	(pNetIF==NULL)
	{
		return	0;
	}
	netif_set_ipaddr(pNetIF,(IPAddr*)&pInfo->ipaddr);
	netif_set_netmask(pNetIF,(IPAddr*)&pInfo->netmask);
	netif_set_gw(pNetIF,(IPAddr*)&pInfo->gw);

#if	LWIP_DHCP

	//Enable dhcp here

	if (pInfo->dhcp_enabled)
	{
		if (!pNetIF->dhcp)
		{

			//Start dhcp client

			dhcp_start(pNetIF);
		}
	}
	else
	{
		if (pNetIF->dhcp)
		{

			//Stop dhcp client

			dhcp_stop(pNetIF);
		}
	}

#endif

	return	1;
}


static void
InitDone(void* pvArg)
{
	sys_sem_t*	pSem=(sys_sem_t*)pvArg;

	dbgprintf("InitDone: TCPIP initialized\n");
	sys_sem_signal(*pSem);
}

void
ps2ip_Stub(void)
{
}


int
ps2ip_ShutDown(void)
{
//	printf("ps2ip_ShutDown: Shutting down ps2ip-module\n");
	return(1); // return "not resident"!
}

/* static void
AddLoopIF(void)
{
	IPAddr	IP;
	IPAddr	NM;
	IPAddr	GW;

	IP4_ADDR(&IP,127,0,0,1);
	IP4_ADDR(&NM,255,0,0,0);
	IP4_ADDR(&GW,127,0,0,1);

	netif_add(&LoopIF,&IP,&NM,&GW,NULL,loopif_init,tcpip_input);
} */

err_t ps2ip_input(struct pbuf* pInput, struct netif* pNetIF)
{
	switch(htons(((struct eth_hdr*)(pInput->payload))->type))
	{
	case	ETHTYPE_IP:
		//IP-packet. Update ARP table, obtain first queued packet.
		etharp_ip_input(pNetIF, pInput);
		pbuf_header(pInput, (int)-sizeof(struct eth_hdr));
		pNetIF->input(pInput, pNetIF);
		break;
	case	ETHTYPE_ARP:
		//ARP-packet. Pass pInput to ARP module, get ARP reply or ARP queued packet.
		//Pass to network layer.
		etharp_arp_input(pNetIF, (struct eth_addr*)&pNetIF->hwaddr, pInput);
		break;
	default:
		//Unsupported ethernet packet-type. Free pInput.
		pbuf_free(pInput);
	}

	return	ERR_OK;
}

int
_start(int argc,char** argv)
{
	sys_sem_t	Sema;
	int			iRet;

	dbgprintf("PS2IP: Module Loaded.\n");

	if ((iRet=RegisterLibraryEntries(&_exp_ps2ip))!=0)
	{
		printf("PS2IP: RegisterLibraryEntries returned: %d\n",iRet);
	}

	sys_init();
	mem_init();
	memp_init();
	pbuf_init();

	dbgprintf("PS2IP: sys_init, mem_init, memp_init, pbuf_init called\n");

	netif_init();

	dbgprintf("PS2IP: netif_init called\n");

	Sema=sys_sem_new(0);
	dbgprintf("PS2IP: Calling tcpip_init\n");
	tcpip_init(InitDone,&Sema);

	sys_arch_sem_wait(Sema,0);
	sys_sem_free(Sema);

	dbgprintf("PS2IP: tcpip_init called\n");

//	AddLoopIF();

	dbgprintf("PS2IP: System Initialised\n");

	return	iRet; 
}

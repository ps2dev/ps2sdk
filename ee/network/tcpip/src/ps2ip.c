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
 * PS2 TCP/IP STACK FOR EE
 */

#include <stdio.h>
#include <string.h>
#include <netman.h>
#include <lwip/memp.h>

#include "lwip/sys.h"
#include "lwip/tcp.h"
#include "lwip/tcpip.h"
#include "lwip/netif.h"
#include "lwip/dhcp.h"
#include "lwip/prot/dhcp.h"
#include "lwip/inet.h"
#include "netif/etharp.h"

#include "ps2ip_internal.h"

typedef struct pbuf	PBuf;
typedef struct netif	NetIF;
typedef struct ip4_addr	IPAddr;

static struct netif NIF;
static struct pbuf *TxHead, *TxTail;

unsigned short int hsyncTicksPerMSec	= 16;

int ps2ip_getconfig(char* pszName, t_ip_info* pInfo)
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

#if LWIP_DHCP
	struct dhcp *dhcp = netif_dhcp_data(pNetIF);

	if ((dhcp != NULL) && (dhcp->state != DHCP_STATE_OFF))
	{
		pInfo->dhcp_enabled=1;
		pInfo->dhcp_status=dhcp->state;
	}
	else
	{
		pInfo->dhcp_enabled=0;
		pInfo->dhcp_status=DHCP_STATE_OFF;
	}

#else
	pInfo->dhcp_enabled=0;
#endif

	return	1;
}

int ps2ip_setconfig(const t_ip_info* pInfo)
{
	NetIF*	pNetIF=netif_find(pInfo->netif_name);

	if	(pNetIF==NULL)
	{
		return	0;
	}

#if	LWIP_DHCP
	struct dhcp *dhcp = netif_dhcp_data(pNetIF);

	//Enable dhcp here

	if (pInfo->dhcp_enabled)
	{
		if ((dhcp == NULL) || (dhcp->state == DHCP_STATE_OFF))
		{
			//Set initial IP address
			netif_set_ipaddr(pNetIF,(IPAddr*)&pInfo->ipaddr);
			netif_set_netmask(pNetIF,(IPAddr*)&pInfo->netmask);
			netif_set_gw(pNetIF,(IPAddr*)&pInfo->gw);

			//Start DHCP client
			dhcp_start(pNetIF);
		}
	}
	else
	{
		if ((dhcp != NULL) && (dhcp->state != DHCP_STATE_OFF))
		{
			//Release DHCP lease
			dhcp_release(pNetIF);

			//Stop DHCP client
			dhcp_stop(pNetIF);
		}

		netif_set_ipaddr(pNetIF,(IPAddr*)&pInfo->ipaddr);
		netif_set_netmask(pNetIF,(IPAddr*)&pInfo->netmask);
		netif_set_gw(pNetIF,(IPAddr*)&pInfo->gw);
	}

#else
	netif_set_ipaddr(pNetIF,(IPAddr*)&pInfo->ipaddr);
	netif_set_netmask(pNetIF,(IPAddr*)&pInfo->netmask);
	netif_set_gw(pNetIF,(IPAddr*)&pInfo->gw);
#endif

	return	1;
}

static void EnQTxPacket(struct pbuf *tx)
{
	DI();

	if(TxHead != NULL)
		TxHead->next = tx;

	TxHead = tx;
	tx->next = NULL;

	if(TxTail == NULL)	//Queue empty
		TxTail = tx;

	EI();
}

static err_t SMapLowLevelOutput(struct netif* pNetIF, struct pbuf* pOutput)
{
	err_t result;

	(void)pNetIF;

	result = ERR_OK;
	if(pOutput->tot_len > pOutput->len)
	{
		struct pbuf* pbuf;

		pbuf_ref(pOutput);	//Increment reference count because LWIP must free the PBUF, not the driver!
		if((pbuf = pbuf_coalesce(pOutput, PBUF_RAW)) != pOutput)
		{	//No need to increase reference count because pbuf_coalesce() does it.
			EnQTxPacket(pbuf);
			NetManNetIFXmit();
		} else
			result = ERR_MEM;
	} else {
		pbuf_ref(pOutput);	//This will be freed later.
		EnQTxPacket(pOutput);
		NetManNetIFXmit();
	}

	return result;
}

static void LinkStateUp(void)
{
	tcpip_callback((void*)&netif_set_link_up, &NIF);
}

static void LinkStateDown(void)
{
	tcpip_callback((void*)&netif_set_link_down, &NIF);
}

static void *AllocRxPacket(unsigned int size, void **payload)
{
	struct pbuf* pbuf;

	if((pbuf = pbuf_alloc(PBUF_RAW, size, PBUF_POOL)) != NULL)
		*payload = pbuf->payload;

	return pbuf;
}

static void ReallocRxPacket(void *packet, unsigned int size)
{
	pbuf_realloc((struct pbuf *)packet, size);
}

static void FreeRxPacket(void *packet)
{
	pbuf_free(packet);
}

static void EnQRxPacket(void *packet)
{
	ps2ip_input(packet, &NIF);
}

static int NextTxPacket(void **payload)
{
	int len;

	if(TxTail != NULL)
	{
		*payload = TxTail->payload;
		len = TxTail->len;
	} else
		len = 0;

	return len;
}

static void DeQTxPacket(void)
{
	struct pbuf *toFree;

	toFree = NULL;

	DI();
	if(TxTail != NULL)
	{
		toFree = TxTail;

		if(TxTail == TxHead) {
			//Last in queue.
			TxTail = NULL;
			TxHead = NULL;
		} else {
			TxTail = TxTail->next;
		}
	}
	EI();

	if(toFree != NULL)
	{
		toFree->next = NULL;
		pbuf_free(toFree);
	}
}

static int AfterTxPacket(void **payload)
{
	int len;

	if(TxTail != NULL && TxTail->next != NULL)
	{
		*payload = TxTail->next->payload;
		len = TxTail->next->len;
	} else
		len = 0;

	return len;
}

static void InitDone(void* pvArg)
{
	dbgprintf("InitDone: TCPIP initialized\n");
	sys_sem_signal((sys_sem_t*)pvArg);
}

/** Should be called at the beginning of the program to set up the network interface. */
static err_t SMapIFInit(struct netif* pNetIF)
{
	TxHead = NULL;
	TxTail = NULL;

	pNetIF->name[0]='s';
	pNetIF->name[1]='m';
	pNetIF->output=&etharp_output;	// For LWIP 1.3.0 and later.
	pNetIF->linkoutput=&SMapLowLevelOutput;
	pNetIF->hwaddr_len=NETIF_MAX_HWADDR_LEN;
	pNetIF->flags|=(NETIF_FLAG_ETHARP|NETIF_FLAG_BROADCAST);	// For LWIP v1.3.0 and later.
	pNetIF->mtu=1500;

	//Get MAC address.
	NetManIoctl(NETMAN_NETIF_IOCTL_ETH_GET_MAC, NULL, 0, pNetIF->hwaddr, sizeof(pNetIF->hwaddr));
	dbgprintf("MAC address : %02d:%02d:%02d:%02d:%02d:%02d\n",pNetIF->hwaddr[0],pNetIF->hwaddr[1],pNetIF->hwaddr[2],
				 pNetIF->hwaddr[3],pNetIF->hwaddr[4],pNetIF->hwaddr[5]);

	return	ERR_OK;
}

err_t ps2ip_input(PBuf* pInput, NetIF* pNetIF)
{
	err_t result;

	if((result = pNetIF->input(pInput, pNetIF)) != ERR_OK)
		pbuf_free(pInput);

	return result;
}

static inline void InitializeLWIP(void)
{
	sys_sem_t Sema;

	dbgprintf("PS2IP: Module Loaded.\n");

	sys_sem_new(&Sema, 0);
	dbgprintf("PS2IP: Calling tcpip_init\n");
	tcpip_init(InitDone,&Sema);

	sys_arch_sem_wait(&Sema, 0);
	sys_sem_free(&Sema);

	dbgprintf("PS2IP: System Initialised\n");
}

int ps2ipInit(struct ip4_addr *ip_address, struct ip4_addr *subnet_mask, struct ip4_addr *gateway){
	static struct NetManNetProtStack stack={
		&LinkStateUp,
		&LinkStateDown,
		&AllocRxPacket,
		&FreeRxPacket,
		&EnQRxPacket,
		&NextTxPacket,
		&DeQTxPacket,
		&AfterTxPacket,
		&ReallocRxPacket
	};

	NetManInit();

	InitializeLWIP();

	netif_add(&NIF, ip_address, subnet_mask, gateway, NULL, &SMapIFInit, tcpip_input);
	netif_set_default(&NIF);

	NetManRegisterNetworkStack(&stack);
	netif_set_up(&NIF);

	return 0;
}

void ps2ipDeinit(void){
	NetManUnregisterNetworkStack();
}

// Stubbed for compatibility purposes
void ps2ipSetHsyncTicksPerMSec(unsigned char ticks) {}

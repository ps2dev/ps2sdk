/*
# _____     ___ ____     ___ ____
#  ____|   |    ____|   |        | |____|
# |     ___|   |____ ___|    ____| |    \    PS2DEV Open Source Project.
#-----------------------------------------------------------------------
# Copyright 2001-2004, ps2dev - http://www.ps2dev.org
# Licenced under Academic Free License version 2.0
# Review ps2sdk README & LICENSE files for further details.
#
# $Id: ps2ip.c 1680 2010-05-17 22:47:17Z jim $
# PS2 TCP/IP STACK FOR IOP
*/

#include <types.h>
#include <stdio.h>
#include <intrman.h>
#include <netman.h>
#include <loadcore.h>
#include <thbase.h>
#include <sysclib.h>
#include <thevent.h>
#include <sysmem.h>
#include <lwip/memp.h>

#include "lwip/sys.h"
#include "lwip/tcp.h"
#include "lwip/tcpip.h"
#include "lwip/netif.h"
#include "lwip/dhcp.h"
#include "lwip/inet.h"
#include "lwip/tcp_impl.h"
#include "netif/etharp.h"

#include "ipconfig.h"
#include "ps2ip_internal.h"

typedef struct pbuf	PBuf;
typedef struct netif	NetIF;
typedef struct ip_addr	IPAddr;

#define MODNAME	"TCP/IP Stack"
IRX_ID(MODNAME, 2, 1);

extern struct irx_export_table	_exp_ps2ip;

#if NOSYS
static int		iTimerARP=0;

#if		defined(PS2IP_DHCP)
static int		iTimerDHCP=0;
#endif	//defined(PS2IP_DHCP)
#endif

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

	if ((pNetIF->dhcp != NULL) && (pNetIF->dhcp->state != DHCP_OFF))
	{
		pInfo->dhcp_enabled=1;
		pInfo->dhcp_status=pNetIF->dhcp->state;
	}
	else
	{
		pInfo->dhcp_enabled=0;
		pInfo->dhcp_status=DHCP_OFF;
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
		if ((pNetIF->dhcp == NULL) || (pNetIF->dhcp->state == DHCP_OFF))
		{
			//Start dhcp client
			dhcp_start(pNetIF);
		}
	}
	else
	{
		if ((pNetIF->dhcp != NULL) && (pNetIF->dhcp->state != DHCP_OFF))
		{
			//Stop dhcp client
			dhcp_stop(pNetIF);
		}
	}

#endif

	return	1;
}


static void InitDone(void* pvArg)
{
	dbgprintf("InitDone: TCPIP initialized\n");
	sys_sem_signal((sys_sem_t*)pvArg);
}

#if NOSYS
static void TimerThread(void* pvArg)
{
	while (1)
	{
		//TCP timer.
		tcp_tmr();

		//ARP timer.
		iTimerARP+=TCP_TMR_INTERVAL;
		if	(iTimerARP>=ARP_TMR_INTERVAL)
		{
			iTimerARP-=ARP_TMR_INTERVAL;
			etharp_tmr();
		}

#if		defined(PS2IP_DHCP)

		//DHCP timer.

		iTimerDHCP+=TCP_TMR_INTERVAL;
		if ((iTimerDHCP-TCP_TMR_INTERVAL)/DHCP_FINE_TIMER_MSECS!=iTimerDHCP/DHCP_FINE_TIMER_MSECS)
		{
			dhcp_fine_tmr();
		}

		if (iTimerDHCP>=DHCP_COARSE_TIMER_SECS*1000)
		{
			iTimerDHCP-=DHCP_COARSE_TIMER_SECS*1000;
			dhcp_coarse_tmr();
		}
#endif

		DelayThread(TCP_TMR_INTERVAL*250);
	}
}

static inline void InitTimer(void)
{
	iop_thread_t	Thread={TH_C, 0, TimerThread, 0x300, 0x16};
	int		iTimerThreadID=CreateThread(&Thread);

	if (iTimerThreadID<0)
	{
		printf("InitTimer: Fatal error - Failed to create tcpip timer-thread!\n");
	}

	//Start timer-thread
	StartThread(iTimerThreadID, NULL);
}
#endif

err_t
ps2ip_input(PBuf* pInput,NetIF* pNetIF)
{
	switch	(htons(((struct eth_hdr*)(pInput->payload))->type))
	{
	case	ETHTYPE_IP:
	case	ETHTYPE_ARP:
		//IP-packet. Update ARP table, obtain first queued packet.
		//ARP-packet. Pass pInput to ARP module, get ARP reply or ARP queued packet.
		//Pass to network layer.

		if(pNetIF->input(pInput, pNetIF)!=ERR_OK)
		{
			dbgprintf("PS2IP: IP input error\n");
			goto error;
		}
		break;
	default:
error:
		//Unsupported ethernet packet-type. Free pInput.
		pbuf_free(pInput);
	}

	return	ERR_OK;
}

void ps2ip_Stub(void)
{
}

int _exit(int argc, char** argv)
{
//	printf("ps2ip_ShutDown: Shutting down ps2ip-module\n");
	return MODULE_NO_RESIDENT_END; // return "not resident"!
}

static struct netif NIF;

static void LinkStateUp(void){
	tcpip_callback((void*)&netif_set_link_up, &NIF);
}

static void LinkStateDown(void){
	tcpip_callback((void*)&netif_set_link_down, &NIF);
}

#define LWIP_STACK_MAX_RX_PBUFS	16

struct NetManPacketBuffer pbufs[LWIP_STACK_MAX_RX_PBUFS];
static unsigned short int NetManRxPacketBufferPoolFreeStart, RxPBufsFree;
static struct NetManPacketBuffer *recv_pbuf_queue_start, *recv_pbuf_queue_end;

static struct NetManPacketBuffer *AllocRxPacket(unsigned int size)
{
	struct pbuf* pBuf;
	struct NetManPacketBuffer *result;

	if(RxPBufsFree>0 && ((pBuf=pbuf_alloc(PBUF_RAW, size, PBUF_POOL))!=NULL))
	{
		result=&pbufs[NetManRxPacketBufferPoolFreeStart];
		NetManRxPacketBufferPoolFreeStart = (NetManRxPacketBufferPoolFreeStart + 1) % LWIP_STACK_MAX_RX_PBUFS;

		result->payload=pBuf->payload;
		result->handle=pBuf;
		result->length=size;

		RxPBufsFree--;
	}
	else result=NULL;

	return result;
}

static void FreeRxPacket(struct NetManPacketBuffer *packet)
{
	pbuf_free(packet->handle);
}

static int EnQRxPacket(struct NetManPacketBuffer *packet)
{
	ps2ip_input(packet->handle, &NIF);
	RxPBufsFree++;

	return 0;
}

static int FlushInputQueue(void)
{
	RxPBufsFree = LWIP_STACK_MAX_RX_PBUFS;
	return 0;
}

static err_t
SMapLowLevelOutput(struct netif *pNetIF, struct pbuf* pOutput)
{
	static u8 buffer[1518];
	struct pbuf* pbuf;
	unsigned char *buffer_ptr;
	unsigned short int TotalLength;

	pbuf=pOutput;
	buffer_ptr=buffer;
	TotalLength=0;
	while(pbuf!=NULL)
	{
		memcpy(buffer_ptr, pbuf->payload, pbuf->len);
		TotalLength+=pbuf->len;
		buffer_ptr+=pbuf->len;
		pbuf=pbuf->next;
	}

	NetManNetIFSendPacket(buffer, TotalLength);

	return ERR_OK;
}

//SMapOutput():

//This function is called by the TCP/IP stack when an IP packet should be sent. It'll be invoked in the context of the
//tcpip-thread, hence no synchronization is required.
// For LWIP versions before v1.3.0.
#ifdef PRE_LWIP_130_COMPAT
static err_t
SMapOutput(struct netif *pNetIF, struct pbuf *pOutput, IPAddr* pIPAddr)
{
	struct pbuf *pBuf=etharp_output(pNetIF,pIPAddr,pOutput);

	return	pBuf!=NULL ? SMapLowLevelOutput(pNetIF, pBuf):ERR_OK;
}
#endif

//Should be called at the beginning of the program to set up the network interface.
static err_t SMapIFInit(struct netif* pNetIF)
{
	static unsigned char MAC_buffer[64];

	pNetIF->name[0]='s';
	pNetIF->name[1]='m';
#ifdef PRE_LWIP_130_COMPAT
	pNetIF->output=&SMapOutput;	// For LWIP versions before v1.3.0.
#else
	pNetIF->output=&etharp_output;	// For LWIP 1.3.0 and later.
#endif
	pNetIF->linkoutput=&SMapLowLevelOutput;
	pNetIF->hwaddr_len=NETIF_MAX_HWADDR_LEN;
#ifdef PRE_LWIP_130_COMPAT
	pNetIF->flags|=(NETIF_FLAG_LINK_UP|NETIF_FLAG_BROADCAST);	// For LWIP versions before v1.3.0.
#else
	pNetIF->flags|=(NETIF_FLAG_ETHARP|NETIF_FLAG_BROADCAST);	// For LWIP v1.3.0 and later.
#endif
	pNetIF->mtu=1500;

	//Get MAC address.
	NetManIoctl(NETMAN_NETIF_IOCTL_ETH_GET_MAC, NULL, 0, MAC_buffer, sizeof(pNetIF->hwaddr));
	memcpy(pNetIF->hwaddr, MAC_buffer, sizeof(pNetIF->hwaddr));
//	DEBUG_PRINTF("MAC address : %02d:%02d:%02d:%02d:%02d:%02d\n",pNetIF->hwaddr[0],pNetIF->hwaddr[1],pNetIF->hwaddr[2],
//				 pNetIF->hwaddr[3],pNetIF->hwaddr[4],pNetIF->hwaddr[5]);

	return	ERR_OK;
}

static inline int InitializeLWIP(void){
	sys_sem_t	Sema;
	int		iRet;

	dbgprintf("PS2IP: Module Loaded.\n");

	if ((iRet=RegisterLibraryEntries(&_exp_ps2ip))!=0)
	{
		printf("PS2IP: RegisterLibraryEntries returned: %d\n", iRet);
	}

	RxPBufsFree=LWIP_STACK_MAX_RX_PBUFS;
	NetManRxPacketBufferPoolFreeStart=0;
	recv_pbuf_queue_start=recv_pbuf_queue_end=NULL;

	sys_sem_new(&Sema, 0);
	dbgprintf("PS2IP: Calling tcpip_init\n");
	tcpip_init(InitDone,&Sema);

	sys_arch_sem_wait(&Sema, 0);
	sys_sem_free(&Sema);

	dbgprintf("PS2IP: tcpip_init called\n");
#if NOSYS
	InitTimer();
#endif

	dbgprintf("PS2IP: System Initialised\n");

	return 0;
}

static inline int InitLWIPStack(struct ip_addr *IP, struct ip_addr *NM, struct ip_addr *GW){
	static struct NetManNetProtStack stack={
		&LinkStateUp,
		&LinkStateDown,
		&AllocRxPacket,
		&FreeRxPacket,
		&EnQRxPacket,
		&FlushInputQueue
	};

	InitializeLWIP();

	netif_add(&NIF, IP, NM, GW, &NIF, &SMapIFInit, tcpip_input);
	netif_set_default(&NIF);

	NetManRegisterNetworkStack(&stack);
	netif_set_up(&NIF);

	return 0;
}

int _start(int argc, char *argv[]){
	struct ip_addr IP, NM, GW;

	//Parse IP address arguments.
	if(argc>=4)
	{
		dbgprintf("SMAP: %s %s %s\n", argv[1],argv[2],argv[3]);
		IP.addr=inet_addr(argv[1]);
		NM.addr=inet_addr(argv[2]);
		GW.addr=inet_addr(argv[3]);
	}
	else
	{
		//Set some defaults.
		IP4_ADDR(&IP,192,168,0,80);
		IP4_ADDR(&NM,255,255,255,0);
		IP4_ADDR(&GW,192,168,0,1);
	}

	return InitLWIPStack(&IP, &NM, &GW)==0?MODULE_RESIDENT_END:MODULE_NO_RESIDENT_END;
}

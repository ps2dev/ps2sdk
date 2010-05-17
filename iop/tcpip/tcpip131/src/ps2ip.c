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
#include <lwip/tcp.h>
#include <lwip/tcpip.h>
#include <lwip/netif.h>
#include <lwip/dhcp.h>
#include <lwip/inet.h>
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


static int		iTimerARP=0;

#if		defined(PS2IP_DHCP)
static int		iTimerDHCP=0;
#endif	//defined(PS2IP_DHCP)

static NetIF	LoopIF;

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


static void
Timer(void* pvArg)
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
}


static void
TimerThread(void* pvArg)
{
	while (1)
	{
		tcpip_callback(Timer,NULL);
		DelayThread(TCP_TMR_INTERVAL*1000);
	}
}


static void
InitTimer(void)
{
	iop_thread_t	Thread={TH_C,0,TimerThread,0x800,0x22};
	int				iTimerThreadID=CreateThread(&Thread);

	if (iTimerThreadID<0)
	{
		printf("InitTimer: Fatal error - Failed to create tcpip timer-thread!\n");
		ExitDeleteThread();
	}

	//Start timer-thread

	StartThread(iTimerThreadID,NULL);
}


static void
SendARPReply(NetIF* pNetIF,PBuf* pBuf)
{

	//Send out the ARP reply or ARP queued packet.

	if	(pBuf!=NULL)
	{
		pNetIF->linkoutput(pNetIF,pBuf);
		pbuf_free(pBuf);
	}
}


typedef struct InputMSG
{
	PBuf*		pInput;
	NetIF*	pNetIF;
} InputMSG;

#define	MSG_QUEUE_SIZE		16

static InputMSG	aMSGs[MSG_QUEUE_SIZE];
static u8_t			u8FirstMSG=0;
static u8_t			u8LastMSG=0;


static u8_t
GetNextMSGQueueIndex(u8_t u8Index)
{
	return	(u8Index+1)%MSG_QUEUE_SIZE;
}


static int
IsMSGQueueFull(void)
{
	return	GetNextMSGQueueIndex(u8LastMSG)==u8FirstMSG;
}


static void
InputCB(void* pvArg)
{

	//TCPIP input callback. This function has been registered by ps2ip_input and will be invoked in the context of the
	//tcpip-thread. Hence, only synchronization for the message-queue is required.

	InputMSG*	pMSG=(InputMSG*)pvArg;
	PBuf*			pInput=pMSG->pInput;
	NetIF*		pNetIF=pMSG->pNetIF;
	int			iFlags;

	//Remove the first message in the message-queue. BTW: pMSG == &aMSGs[u8FirstMSG].

	CpuSuspendIntr(&iFlags);
	u8FirstMSG=GetNextMSGQueueIndex(u8FirstMSG);
	CpuResumeIntr(iFlags);

	//What kind of package is it?

	switch	(htons(((struct eth_hdr*)(pInput->payload))->type))
	{
	case	ETHTYPE_IP:

		//IP-packet. Update ARP table, obtain first queued packet.

		etharp_ip_input(pNetIF,pInput);

		//Skip Ethernet header.

		pbuf_header(pInput,-14);

		//Pass to network layer.

		ip_input(pInput,pNetIF);

		//Send out the ARP reply or ARP queued packet.

		SendARPReply(pNetIF,NULL);
		break;
	case	ETHTYPE_ARP:

		//ARP-packet. Pass pInput to ARP module, get ARP reply or ARP queued packet.

		etharp_arp_input(pNetIF,(struct eth_addr*)&pNetIF->hwaddr,pInput);

		//Send out the ARP reply or ARP queued packet.

		SendARPReply(pNetIF,NULL);
		break;
	default:

		//Unsupported ethernet packet-type. Free pInput.

		pbuf_free(pInput);
	}
}


//Added in lwip/src/api/tcpip.c. It returns a pointer to the messagebox of the tcpip-thread.
//
//sys_mbox_t tcpip_getmbox(void) {return mbox;}

extern sys_mbox_t		tcpip_getmbox(void);


err_t
ps2ip_input(PBuf* pInput,NetIF* pNetIF)
{

	//When ps2smap receive data, it invokes this function. It'll be called directly by the interrupthandler, which means we are
	//running in an interrupt-context. We'll pass on the data to the tcpip message-thread by adding a callback message. If the
	//messagebox is full, we can't wait for the tcpip-thread to process a message to make room for our message, since we're in an
	//interrupt-context. If the messagebox or messagequeue is full, drop the packet.

	InputMSG*				pIMSG;
	struct tcpip_msg*		pMSG;
	sys_mbox_t				pMBox=tcpip_getmbox();
	arch_message *          msg;

	//Is the messagequeue full?

	if(IsMSGQueueFull() || ((msg = alloc_msg()) == NULL))
	{

		//Yes, silently drop the packet!

		dbgprintf("ps2ip_input: MessageQueue or MessagePool full, dropping packet\n");
		pbuf_free(pInput);
		return	ERR_OK;
	}

	//Allocate messagequeue entry.

	pIMSG=&aMSGs[u8LastMSG];
	u8LastMSG=GetNextMSGQueueIndex(u8LastMSG);

	//Initialize the InputMSG.

	pIMSG->pInput=pInput;
	pIMSG->pNetIF=pNetIF;

	//Allocate memory for the tcpip-message.

	pMSG=(struct tcpip_msg*)memp_malloc(MEMP_TCPIP_MSG_INPKT);
	if	(pMSG==NULL)
	{
		//Memory allocation failed, drop packet!

		dbgprintf("ps2ip_input: Failed to allocate memory for tcpip_msg, dropping packet\n");
		pbuf_free(pInput);
		free_msg(msg);
		return	ERR_MEM;  
	}
	pMSG->type=TCPIP_MSG_CALLBACK;
	pMSG->msg.cb.f=InputCB;
	pMSG->msg.cb.ctx=pIMSG;

	//Post the message in the tcpip-thread's messagebox.

    msg->sys_msg = pMSG;
	PostInputMSG(pMBox, msg);
	return	ERR_OK;
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


static void
AddLoopIF(void)
{
	IPAddr	IP;
	IPAddr	NM;
	IPAddr	GW;

	IP4_ADDR(&IP,127,0,0,1);
	IP4_ADDR(&NM,255,0,0,0);
	IP4_ADDR(&GW,127,0,0,1);

	netif_add(&LoopIF,&IP,&NM,&GW,NULL,loopif_init,tcpip_input);
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

	AddLoopIF();
	InitTimer();

	dbgprintf("PS2IP: System Initialised\n");

	return	iRet; 
}

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
 * PS2 TCP/IP STACK FOR IOP
 */

#include <types.h>
#include <stdio.h>
#include <intrman.h>
#include <loadcore.h>
#include <thbase.h>
#include <sysclib.h>
#include <thevent.h>
#include <sysmem.h>
#include <lwip/memp.h>

#include "lwip/sys.h"
#include "lwip/tcp.h"
#include "lwip/tcpip.h"
#include "lwip/prot/dhcp.h"
#include "lwip/netif.h"
#include "lwip/dhcp.h"
#include "lwip/inet.h"
#include "netif/etharp.h"

#include "ps2ip_internal.h"

typedef struct pbuf	PBuf;
typedef struct netif	NetIF;
typedef struct ip4_addr	IPAddr;

#define MODNAME	"TCP/IP Stack"
IRX_ID(MODNAME, 2, 3);

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


int
ps2ip_setconfig(const t_ip_info* pInfo)
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
			netif_set_ipaddr(pNetIF,(const IPAddr*)&pInfo->ipaddr);
			netif_set_netmask(pNetIF,(const IPAddr*)&pInfo->netmask);
			netif_set_gw(pNetIF,(const IPAddr*)&pInfo->gw);

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

		netif_set_ipaddr(pNetIF,(const IPAddr*)&pInfo->ipaddr);
		netif_set_netmask(pNetIF,(const IPAddr*)&pInfo->netmask);
		netif_set_gw(pNetIF,(const IPAddr*)&pInfo->gw);
	}
#else
	netif_set_ipaddr(pNetIF,(const IPAddr*)&pInfo->ipaddr);
	netif_set_netmask(pNetIF,(const IPAddr*)&pInfo->netmask);
	netif_set_gw(pNetIF,(const IPAddr*)&pInfo->gw);
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
	iop_thread_t	Thread={TH_C, (u32)"PS2IP-timer", TimerThread, 0x300, 0x16};
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
	err_t result;

	if((result = pNetIF->input(pInput, pNetIF)) != ERR_OK)
		pbuf_free(pInput);

	return result;
}

int _exit(int argc, char** argv)
{
	return MODULE_NO_RESIDENT_END; // return "not resident"!
}

int _start(int argc, char *argv[]){
	sys_sem_t	Sema;
	int		result;

	dbgprintf("PS2IP: Module Loaded.\n");

	if ((result = RegisterLibraryEntries(&_exp_ps2ip)) != 0)
	{
		printf("PS2IP: RegisterLibraryEntries returned: %d\n", result);
		return MODULE_NO_RESIDENT_END;
	} else {
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
	}

	return MODULE_RESIDENT_END;
}

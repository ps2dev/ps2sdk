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

#include "ps2ip_internal.h"

typedef struct pbuf PBuf;
typedef struct netif NetIF;
typedef struct ip_addr IPAddr;

#define MODNAME "TCP/IP Stack"
IRX_ID(MODNAME, 2, 1);

extern struct irx_export_table _exp_ps2ip;

#if NOSYS
static int iTimerARP = 0;

#if defined(PS2IP_DHCP)
static int iTimerDHCP = 0;
#endif  //defined(PS2IP_DHCP)
#endif

int ps2ip_getconfig(char *pszName, t_ip_info *pInfo)
{
    NetIF *pNetIF = netif_find(pszName);

    if (pNetIF == NULL) {

        //Net interface not found.

        memset(pInfo, 0, sizeof(*pInfo));
        return 0;
    }
    strcpy(pInfo->netif_name, pszName);
    pInfo->ipaddr.s_addr = pNetIF->ip_addr.addr;
    pInfo->netmask.s_addr = pNetIF->netmask.addr;
    pInfo->gw.s_addr = pNetIF->gw.addr;

    memcpy(pInfo->hw_addr, pNetIF->hwaddr, sizeof(pInfo->hw_addr));

#if LWIP_DHCP

    if (pNetIF->dhcp) {
        pInfo->dhcp_enabled = 1;
        pInfo->dhcp_status = pNetIF->dhcp->state;
    } else {
        pInfo->dhcp_enabled = 0;
        pInfo->dhcp_status = 0;
    }

#else

    pInfo->dhcp_enabled = 0;

#endif

    return 1;
}


int ps2ip_setconfig(t_ip_info *pInfo)
{
    NetIF *pNetIF = netif_find(pInfo->netif_name);

    if (pNetIF == NULL) {
        return 0;
    }
    netif_set_ipaddr(pNetIF, (IPAddr *)&pInfo->ipaddr);
    netif_set_netmask(pNetIF, (IPAddr *)&pInfo->netmask);
    netif_set_gw(pNetIF, (IPAddr *)&pInfo->gw);

#if LWIP_DHCP

    //Enable dhcp here

    if (pInfo->dhcp_enabled) {
        if (!pNetIF->dhcp) {

            //Start dhcp client

            dhcp_start(pNetIF);
        }
    } else {
        if (pNetIF->dhcp) {

            //Stop dhcp client

            dhcp_stop(pNetIF);
        }
    }

#endif

    return 1;
}


static void InitDone(void *pvArg)
{
    dbgprintf("InitDone: TCPIP initialized\n");
    sys_sem_signal((sys_sem_t *)pvArg);
}

#if NOSYS
static void TimerThread(void *pvArg)
{
    while (1) {
        //TCP timer.
        tcp_tmr();

        //ARP timer.
        iTimerARP += TCP_TMR_INTERVAL;
        if (iTimerARP >= ARP_TMR_INTERVAL) {
            iTimerARP -= ARP_TMR_INTERVAL;
            etharp_tmr();
        }

#if defined(PS2IP_DHCP)

        //DHCP timer.

        iTimerDHCP += TCP_TMR_INTERVAL;
        if ((iTimerDHCP - TCP_TMR_INTERVAL) / DHCP_FINE_TIMER_MSECS != iTimerDHCP / DHCP_FINE_TIMER_MSECS) {
            dhcp_fine_tmr();
        }

        if (iTimerDHCP >= DHCP_COARSE_TIMER_SECS * 1000) {
            iTimerDHCP -= DHCP_COARSE_TIMER_SECS * 1000;
            dhcp_coarse_tmr();
        }
#endif

        DelayThread(TCP_TMR_INTERVAL * 250); /* Note: The IOP's DelayThread() function isn't accurate, and the actual timming accuracy is about 25% of the specified value. */
    }
}

static inline void InitTimer(void)
{
    iop_thread_t Thread = {TH_C, 0, TimerThread, 0x300, 0x16};
    int iTimerThreadID = CreateThread(&Thread);

    if (iTimerThreadID < 0) {
        printf("InitTimer: Fatal error - Failed to create tcpip timer-thread!\n");
    }

    //Start timer-thread
    StartThread(iTimerThreadID, NULL);
}
#endif

err_t ps2ip_input(PBuf *pInput, NetIF *pNetIF)
{
    switch (htons(((struct eth_hdr *)(pInput->payload))->type)) {
        case ETHTYPE_IP:
        case ETHTYPE_ARP:
            //IP-packet. Update ARP table, obtain first queued packet.
            //ARP-packet. Pass pInput to ARP module, get ARP reply or ARP queued packet.
            //Pass to network layer.

            if (pNetIF->input(pInput, pNetIF) != ERR_OK) {
                dbgprintf("PS2IP: IP input error\n");
                goto error;
            }
            break;
        default:
        error:
            //Unsupported ethernet packet-type. Free pInput.
            pbuf_free(pInput);
    }

    return ERR_OK;
}

void ps2ip_Stub(void)
{
}

int _exit(int argc, char **argv)
{
    //	printf("ps2ip_ShutDown: Shutting down ps2ip-module\n");
    return MODULE_NO_RESIDENT_END;  // return "not resident"!
}

static inline int InitLWIPStack(struct ip_addr *IP, struct ip_addr *NM, struct ip_addr *GW)
{
    sys_sem_t Sema;
    int iRet;

    dbgprintf("PS2IP: Module Loaded.\n");

    if ((iRet = RegisterLibraryEntries(&_exp_ps2ip)) != 0) {
        printf("PS2IP: RegisterLibraryEntries returned: %d\n", iRet);
    } else {
        sys_sem_new(&Sema, 0);
        dbgprintf("PS2IP: Calling tcpip_init\n");
        tcpip_init(InitDone, &Sema);

        sys_arch_sem_wait(&Sema, 0);
        sys_sem_free(&Sema);

        dbgprintf("PS2IP: tcpip_init called\n");
#if NOSYS
        InitTimer();
#endif

        dbgprintf("PS2IP: System Initialised\n");
    }

    return iRet;
}

int _start(int argc, char *argv[])
{
    struct ip_addr IP, NM, GW;

    //Parse IP address arguments.
    if (argc >= 4) {
        dbgprintf("SMAP: %s %s %s\n", argv[1], argv[2], argv[3]);
        IP.addr = inet_addr(argv[1]);
        NM.addr = inet_addr(argv[2]);
        GW.addr = inet_addr(argv[3]);
    } else {
        //Set some defaults.
        IP4_ADDR(&IP, 192, 168, 0, 80);
        IP4_ADDR(&NM, 255, 255, 255, 0);
        IP4_ADDR(&GW, 192, 168, 0, 1);
    }

    return InitLWIPStack(&IP, &NM, &GW) == 0 ? MODULE_RESIDENT_END : MODULE_NO_RESIDENT_END;
}

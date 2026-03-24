/*
# _____     ___ ____     ___ ____
#  ____|   |    ____|   |        | |____|
# |     ___|   |____ ___|    ____| |    \    PS2DEV Open Source Project.
#-----------------------------------------------------------------------
# Copyright ps2dev - http://www.ps2dev.org
# Licenced under Academic Free License version 2.0
# Review ps2sdk README & LICENSE files for further details.
*/

#ifndef _NETDEV_H
#define _NETDEV_H

#include <tamtypes.h>

#define sceInetBus_Unknown 0
#define sceInetBus_USB 1
#define sceInetBus_1394 2
#define sceInetBus_PCMCIA 3
#define sceInetBus_PSEUDO 4
#define sceInetBus_NIC 5

#define sceInetDevProtVer 2

#define sceInetDevF_Up 0x0001
#define sceInetDevF_Running 0x0002
#define sceInetDevF_Broadcast 0x0004
#define sceInetDevF_ARP 0x0010
#define sceInetDevF_DHCP 0x0020
#define sceInetDevF_PPP 0x0040
#define sceInetDevF_NIC 0x0080
#define sceInetDevF_Error 0x0100
#define sceInetDevF_PPPoE 0x0200
#define sceInetDevF_Multicast 0x0400

#define sceInetDevEFP_StartDone 0x00000001
#define sceInetDevEFP_PlugOut 0x00000002
#define sceInetDevEFP_Recv 0x00000004
#define sceInetDevEFP_Error 0x00000010
#define sceInetDevEFP_TimeOut 0x00000020
#define sceInetDevEFP_InetUse 0xffff0000

#define sceInetDevDHCP_RelOnStop 0x00000001

#define sceInetNDCC_GET_THPRI 0x80000000
#define sceInetNDCC_SET_THPRI 0x81000000
#define sceInetNDCC_GET_IF_TYPE 0x80000100
#define sceInetNDCC_GET_RX_PACKETS 0x80010000
#define sceInetNDCC_GET_TX_PACKETS 0x80010001
#define sceInetNDCC_GET_RX_BYTES 0x80010002
#define sceInetNDCC_GET_TX_BYTES 0x80010003
#define sceInetNDCC_GET_RX_ERRORS 0x80010004
#define sceInetNDCC_GET_TX_ERRORS 0x80010005
#define sceInetNDCC_GET_RX_DROPPED 0x80010006
#define sceInetNDCC_GET_TX_DROPPED 0x80010007
#define sceInetNDCC_GET_RX_BROADCAST_PACKETS 0x80010008
#define sceInetNDCC_GET_TX_BROADCAST_PACKETS 0x80010009
#define sceInetNDCC_GET_RX_BROADCAST_BYTES 0x8001000a
#define sceInetNDCC_GET_TX_BROADCAST_BYTES 0x8001000b
#define sceInetNDCC_GET_RX_MULTICAST_PACKETS 0x8001000c
#define sceInetNDCC_GET_TX_MULTICAST_PACKETS 0x8001000d
#define sceInetNDCC_GET_RX_MULTICAST_BYTES 0x8001000e
#define sceInetNDCC_GET_TX_MULTICAST_BYTES 0x8001000f

#define sceInetNDIFT_GENERIC 0x00000000
#define sceInetNDIFT_ETHERNET 0x00000001
#define sceInetNDIFT_PPP 0x00000002

#define sceInetNDCC_GET_MULTICAST 0x80011000
#define sceInetNDCC_GET_COLLISIONS 0x80011001
#define sceInetNDCC_GET_RX_LENGTH_ER 0x80011002
#define sceInetNDCC_GET_RX_OVER_ER 0x80011003
#define sceInetNDCC_GET_RX_CRC_ER 0x80011004
#define sceInetNDCC_GET_RX_FRAME_ER 0x80011005
#define sceInetNDCC_GET_RX_FIFO_ER 0x80011006
#define sceInetNDCC_GET_RX_MISSED_ER 0x80011007
#define sceInetNDCC_GET_TX_ABORTED_ER 0x80011008
#define sceInetNDCC_GET_TX_CARRIER_ER 0x80011009
#define sceInetNDCC_GET_TX_FIFO_ER 0x8001100a
#define sceInetNDCC_GET_TX_HEARTBEAT_ER 0x8001100b
#define sceInetNDCC_GET_TX_WINDOW_ER 0x8001100c
#define sceInetNDCC_GET_NEGO_MODE 0x80020000
#define sceInetNDCC_SET_NEGO_MODE 0x81020000
#define sceInetNDCC_GET_NEGO_STATUS 0x80020001
#define sceInetNDCC_GET_LINK_STATUS 0x80030000
#define sceInetNDCC_SET_MULTICAST_LIST 0x81040000

#define sceInetNDNEGO_10 0x0001
#define sceInetNDNEGO_10_FD 0x0002
#define sceInetNDNEGO_TX 0x0004
#define sceInetNDNEGO_TX_FD 0x0008
#define sceInetNDNEGO_PAUSE 0x0040
#define sceInetNDNEGO_AUTO 0x0080

typedef struct sceInetPkt
{
	struct sceInetPkt *forw;
	struct sceInetPkt *back;
#ifdef NETDEV_OPAQUE
	int reserved[2];
#else
	void *m_reserved1;
	void *m_reserved2;
#endif
	u8 *rp;
	u8 *wp;
} sceInetPkt_t;

typedef struct sceInetPktQ
{
	struct sceInetPkt *head;
	struct sceInetPkt *tail;
} sceInetPktQ_t;

typedef struct sceInetDevOps
{
	struct sceInetDevOps *forw;
	struct sceInetDevOps *back;
	char interface[9];
	char *module_name;
	char *vendor_name;
	char *device_name;
	u8 bus_type;
	u8 bus_loc[31];
	u16 prot_ver;
	u16 impl_ver;
	void *priv;
	int flags;
	int evfid;
	struct sceInetPktQ rcvq;
	struct sceInetPktQ sndq;
	int (*start)(void *priv, int flags);
	int (*stop)(void *priv, int flags);
	int (*xmit)(void *priv, int flags);
	int (*control)(void *priv, int code, void *ptr, int len);
	unsigned int ip_addr;
	unsigned int ip_mask;
	unsigned int broad_addr;
	unsigned int gw_addr;
	unsigned int ns_addr1;
	int mtu;
	u8 hw_addr[16];
	u8 dhcp_hostname[256];
	int dhcp_hostname_len;
	int dhcp_flags;
	void *reserved[4];
	unsigned int ns_addr2;
	void *pppoe_priv;
} sceInetDevOps_t;

extern int sceInetRegisterNetDevice(sceInetDevOps_t *ops);
extern int sceInetUnregisterNetDevice(sceInetDevOps_t *ops);
extern void *sceInetAllocMem(sceInetDevOps_t *ops, int siz);
extern void sceInetFreeMem(sceInetDevOps_t *ops, void *ptr);
extern void sceInetPktEnQ(sceInetPktQ_t *que, sceInetPkt_t *pkt);
extern sceInetPkt_t *sceInetPktDeQ(sceInetPktQ_t *que);
extern unsigned int sceInetRand(void);
extern int sceInetPrintf(const char *fmt, ...);
extern sceInetPkt_t *sceInetAllocPkt(sceInetDevOps_t *ops, int siz);
extern void sceInetFreePkt(sceInetDevOps_t *ops, sceInetPkt_t *pkt);
// TODO: sceInetRegisterPPPoE
// TODO: sceInetUnregisterPPPoE

#define netdev_IMPORTS_start DECLARE_IMPORT_TABLE(netdev, 1, 1)
#define netdev_IMPORTS_end END_IMPORT_TABLE

#define I_sceInetRegisterNetDevice DECLARE_IMPORT(4, sceInetRegisterNetDevice)
#define I_sceInetUnregisterNetDevice DECLARE_IMPORT(5, sceInetUnregisterNetDevice)
#define I_sceInetAllocMem DECLARE_IMPORT(6, sceInetAllocMem)
#define I_sceInetFreeMem DECLARE_IMPORT(7, sceInetFreeMem)
#define I_sceInetPktEnQ DECLARE_IMPORT(8, sceInetPktEnQ)
#define I_sceInetPktDeQ DECLARE_IMPORT(9, sceInetPktDeQ)
#define I_sceInetRand DECLARE_IMPORT(10, sceInetRand)
#define I_sceInetPrintf DECLARE_IMPORT(11, sceInetPrintf)
#define I_sceInetAllocPkt DECLARE_IMPORT(12, sceInetAllocPkt)
#define I_sceInetFreePkt DECLARE_IMPORT(13, sceInetFreePkt)
#define I_sceInetRegisterPPPoE DECLARE_IMPORT(14, sceInetRegisterPPPoE)
#define I_sceInetUnregisterPPPoE DECLARE_IMPORT(15, sceInetUnregisterPPPoE)

#endif

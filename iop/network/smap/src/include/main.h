
#ifndef MAIN_H
#define MAIN_H

#include <thbase.h>

#ifdef BUILDING_SMAP_NETMAN
#include <netman.h>
#endif
#ifdef BUILDING_SMAP_PS2IP
#include <ps2ip.h>
#endif
#ifdef BUILDING_SMAP_NETDEV
#include <netdev.h>
#endif
#ifdef BUILDING_SMAP_MODULAR
#include <smap_modular.h>
#endif

#ifndef BUILDING_SMAP_NETDEV
#define sceInetPrintf(...) printf(__VA_ARGS__)
#endif

#define DEBUG_PRINTF(args...) sceInetPrintf("SMAP: "args)

#ifdef BUILDING_SMAP_NETDEV
struct RuntimeStats_NetDev
{
    u32 m_RxErrorVarious[16];
    u32 m_TxErrorVarious[16];
    u32 m_Rx_Packets;
    u32 m_Tx_Packets;
    u32 m_Rx_Bytes;
    u32 m_Tx_Bytes;
    u32 m_Rx_Errors;
    u32 m_Tx_Errors;
    u32 m_Rx_Dropped;
    u32 m_Tx_Dropped;
    u32 m_Rx_Broadcast_Packets;
    u32 m_Tx_Broadcast_Packets;
    u32 m_Rx_Broadcast_Bytes;
    u32 m_Tx_Broadcast_Bytes;
    u32 m_Rx_Multicast_Packets;
    u32 m_Tx_Multicast_Packets;
    u32 m_Rx_Multicast_Bytes;
    u32 m_Tx_Multicast_Bytes;
    u32 m_Multicast;
    u32 m_Collisions;
    u32 m_Rx_Length_Er;
    u32 m_Rx_Over_Er;
    u32 m_Rx_Crc_Er;
    u32 m_Rx_Frame_Er;
    u32 m_Rx_Fifo_Er;
    u32 m_Rx_Missed_Er;
    u32 m_Tx_Aborted_Er;
    u32 m_Tx_Carrier_Er;
    u32 m_Tx_Fifo_Er;
    u32 m_Tx_Heartbeat_Er;
    u32 m_Tx_Window_Er;
};
#endif

// This struct needs to be the exact same layout as struct NetManEthRuntimeStats!
struct RuntimeStats
{
    u32 RxDroppedFrameCount;
    u32 RxErrorCount;
    u16 RxFrameOverrunCount;
    u16 RxFrameBadLengthCount;
    u16 RxFrameBadFCSCount;
    u16 RxFrameBadAlignmentCount;
    u32 TxDroppedFrameCount;
    u32 TxErrorCount;
    u16 TxFrameLOSSCRCount;
    u16 TxFrameEDEFERCount;
    u16 TxFrameCollisionCount;
    u16 TxFrameUnderrunCount;
    u16 RxAllocFail;
};

struct SmapDriverData
{
    volatile u8 *smap_regbase;
    volatile u8 *emac3_regbase;
    unsigned int TxBufferSpaceAvailable;
    unsigned char NumPacketsInTx;
    unsigned char TxBDIndex;
    unsigned char TxDNVBDIndex;
    unsigned char RxBDIndex;
    void *packetToSend;
    int Dev9IntrEventFlag;
    int IntrHandlerThreadID;
    unsigned char SmapDriverStarting; // SMAP driver is starting.
    unsigned char SmapIsInitialized; // SMAP driver is initialized (software)
    unsigned char NetDevStopFlag;
    unsigned char EnableLinkCheckTimer;
    unsigned char LinkStatus; // Ethernet link is initialized (hardware)
    unsigned char LinkMode;
    iop_sys_clock_t LinkCheckTimer;
#ifdef SMAP_RX_PACKETS_POLLING_MODE
    iop_sys_clock_t RxIntrPollingTimer;
#endif
    struct RuntimeStats RuntimeStats;
#ifdef BUILDING_SMAP_NETDEV
    struct RuntimeStats_NetDev RuntimeStats_NetDev;
#endif
#ifdef BUILDING_SMAP_NETMAN
    int NetIFID;
#endif
#ifdef BUILDING_SMAP_NETDEV
    sceInetDevOps_t m_devops;
#endif
#ifdef BUILDING_SMAP_MODULAR
    const SmapModularHookTable_t *HookTable[1];
#endif
};

/* Event flag bits */
#define SMAP_EVENT_START      0x01
#define SMAP_EVENT_STOP       0x02
#define SMAP_EVENT_INTR       0x04
#define SMAP_EVENT_XMIT       0x08
#define SMAP_EVENT_LINK_CHECK 0x10

/* Function prototypes */
extern int DisplayBanner(void);
extern int smap_init(int argc, char *argv[]);
#ifdef BUILDING_SMAP_NETDEV
extern int smap_deinit(void);
#endif
#ifdef BUILDING_SMAP_PS2IP
extern int SMAPInitStart(void);
#endif
extern void SMAPXmit(void);
extern int SMAPGetMACAddress(u8 *buffer);
#ifdef BUILDING_SMAP_PS2IP
extern void PS2IPLinkStateUp(void);
extern void PS2IPLinkStateDown(void);

extern void SMapLowLevelInput(struct pbuf *pBuf);
extern int SMapTxPacketNext(void **payload);
extern void SMapTxPacketDeQ(void);
#endif

/* Data prototypes */
extern struct SmapDriverData SmapDriverData;

#endif

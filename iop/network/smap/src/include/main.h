
#ifndef MAIN_H
#define MAIN_H

#include <thbase.h>

#ifdef BUILDING_SMAP_NETMAN
#include <netman.h>
#endif
#ifdef BUILDING_SMAP_PS2IP
#include <ps2ip.h>
#endif
#ifdef BUILDING_SMAP_MODULAR
#include <smap_modular.h>
#endif

// In the SONY original, all the calls to DEBUG_PRINTF() were to sceInetPrintf().
#define DEBUG_PRINTF(args...) printf("SMAP: "args)

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
    unsigned char SmapDriverStarted; // SMAP driver is started.
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
#ifdef BUILDING_SMAP_NETMAN
    int NetIFID;
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
int DisplayBanner(void);
int smap_init(int argc, char *argv[]);
#ifdef BUILDING_SMAP_PS2IP
int SMAPInitStart(void);
#endif
int SMAPStart(void);
void SMAPStop(void);
void SMAPXmit(void);
int SMAPGetMACAddress(u8 *buffer);
#ifdef BUILDING_SMAP_PS2IP
void PS2IPLinkStateUp(void);
void PS2IPLinkStateDown(void);

void SMapLowLevelInput(struct pbuf *pBuf);
int SMapTxPacketNext(void **payload);
void SMapTxPacketDeQ(void);
#endif

/* Data prototypes */
extern struct SmapDriverData SmapDriverData;

#endif

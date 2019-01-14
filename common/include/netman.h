/**
 * @file
 * Netman common definitions
 */

#ifndef __NETMAN_H__
#define __NETMAN_H__

//Common structures
#define NETMAN_NETIF_NAME_MAX_LEN	4
#define NETMAN_NETIF_FRAME_SIZE		1514
#define NETMAN_FRAME_GROUP_SIZE		8	//The actual number of DMA transfer tags is twice this. The total number presented to sceSifSetDma must never exceed 32.

struct NetManNetProtStack{
	void (*LinkStateUp)(void);
	void (*LinkStateDown)(void);
	void *(*AllocRxPacket)(unsigned int size, void **payload);
	void (*FreeRxPacket)(void *packet);
	void (*EnQRxPacket)(void *packet);
	int (*NextTxPacket)(void **payload);
	void (*DeQTxPacket)(void);
	int (*AfterTxPacket)(void **payload);				//For EE only, peek at the packet after the current packet.
	void (*ReallocRxPacket)(void *packet, unsigned int size);	//For EE only, update the size of the Rx packet (size will be always smaller than NETMAN_NETIF_FRAME_SIZE).
};

struct NetManEthRuntimeStats{
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

struct NetManEthStatus{
	u8 LinkStatus, LinkMode;
	struct NetManEthRuntimeStats stats;
};

/** Flow-control */
#define NETMAN_NETIF_ETH_LINK_DISABLE_PAUSE	0x40

enum NETMAN_NETIF_ETH_LINK_MODE{
	/** Auto negotiation cannot be reflected by NETMAN_NETIF_IOCTL_ETH_GET_LINK_MODE. */
	NETMAN_NETIF_ETH_LINK_MODE_AUTO		= 0,
	/** 10Mbit Half-DupleX */
	NETMAN_NETIF_ETH_LINK_MODE_10M_HDX,
	/** 10Mbit Full-DupleX */
	NETMAN_NETIF_ETH_LINK_MODE_10M_FDX,
	/** 100Mbit Half-DupleX */
	NETMAN_NETIF_ETH_LINK_MODE_100M_HDX,
	/** 100Mbit Full-DupleX */
	NETMAN_NETIF_ETH_LINK_MODE_100M_FDX,

	NETMAN_NETIF_ETH_LINK_MODE_COUNT
};

enum NETMAN_NETIF_ETH_LINK_STATE{
	NETMAN_NETIF_ETH_LINK_STATE_DOWN	= 0,
	NETMAN_NETIF_ETH_LINK_STATE_UP
};

enum NETMAN_NETIF_IOCTL_CODES{
	// Ethernet I/F-only IOCTL codes
	/** Output = 6 bytes of MAC address. */
	NETMAN_NETIF_IOCTL_ETH_GET_MAC	= 0x1000,
	//Function codes with no input and no output; the result is in the return value.
	NETMAN_NETIF_IOCTL_ETH_GET_LINK_MODE,
	NETMAN_NETIF_IOCTL_ETH_GET_RX_EOVERRUN_CNT,
	NETMAN_NETIF_IOCTL_ETH_GET_RX_EBADLEN_CNT,
	NETMAN_NETIF_IOCTL_ETH_GET_RX_EBADFCS_CNT,
	NETMAN_NETIF_IOCTL_ETH_GET_RX_EBADALIGN_CNT,
	NETMAN_NETIF_IOCTL_ETH_GET_TX_ELOSSCR_CNT,
	NETMAN_NETIF_IOCTL_ETH_GET_TX_EEDEFER_CNT,
	NETMAN_NETIF_IOCTL_ETH_GET_TX_ECOLL_CNT,
	NETMAN_NETIF_IOCTL_ETH_GET_TX_EUNDERRUN_CNT,
	//Returns struct NetManEthStatus
	NETMAN_NETIF_IOCTL_ETH_GET_STATUS,

	/** Input = struct NetManIFLinkModeParams. Note: does not wait for the IF to finish. Use NetManSetLinkMode() instead. */
	NETMAN_NETIF_IOCTL_ETH_SET_LINK_MODE,

	// Dial-up I/F-only IOCTL codes
	// 0x2000

	// Common IOCTL codes
	NETMAN_NETIF_IOCTL_GET_LINK_STATUS	= 0x3000,
	NETMAN_NETIF_IOCTL_GET_TX_DROPPED_COUNT,
	NETMAN_NETIF_IOCTL_GET_RX_DROPPED_COUNT,
};

//*** Higher-level services, for the running user program ***
//Initialization and deinitialization functions, for the EE side.
#ifdef _EE

int NetManInit(void);
void NetManDeinit(void);

#endif

//Network Interface (IF) control.
int NetManGetGlobalNetIFLinkState(void);
int NetManSetMainIF(const char *name);
int NetManQueryMainIF(char *name);

//*** System functions for either the Network IF driver or the network protocol stack ***
//For the network protocol stack to initialize/deinitialize NETMAN.
int NetManRegisterNetworkStack(const struct NetManNetProtStack *stack);
void NetManUnregisterNetworkStack(void);

/* Common network Interface (IF) management functions. Used by the user program and the protocol stack. */
int NetManIoctl(unsigned int command, void *args, unsigned int args_len, void *output, unsigned int length);
int NetManSetLinkMode(int mode);

/* Network Interface (IF) management functions. Used by the protocol stack. */
void NetManNetIFXmit(void);	//Notify the interface of available packets. May be called from the interrupt context.

/* Network protocol stack management functions. Used by the Network InterFace (IF) driver. */
void *NetManNetProtStackAllocRxPacket(unsigned int length, void **payload);
void NetManNetProtStackFreeRxPacket(void *packet);
void NetManNetProtStackEnQRxPacket(void *packet);
int NetManTxPacketNext(void **payload);
void NetManTxPacketDeQ(void);

int NetManTxPacketAfter(void **payload);					//For EE only, for NETMAN's internal use.
void NetManNetProtStackReallocRxPacket(void *packet, unsigned int length);	//For EE only, for NETMAN's internal use.

/* NETIF flags. */
/** Set internally by NETMAN. Do not set externally. */
#define	NETMAN_NETIF_IN_USE	0x80
/** Set = network IF is an Ethernet IF. */
#define	NETMAN_NETIF_ETHERNET	1
/** Set = network IF is a dailup modem. */
#define	NETMAN_NETIF_DIALUP	2
/** Set = network IF has a link up status. */
#define	NETMAN_NETIF_LINK_UP	4

struct NetManNetIF{
	char name[NETMAN_NETIF_NAME_MAX_LEN];
	unsigned short int flags;
	/** Used internally by NETMAN. Do not use. */
	short int id;
	int (*init)(void);
	void (*deinit)(void);
	void (*xmit)(void);
	int (*ioctl)(unsigned int command, void *args, unsigned int args_len, void *output, unsigned int length);
	int EventFlagID;
};

//IF event flag bits, set by NETMAN
#define NETMAN_NETIF_EVF_UP	0x01
#define NETMAN_NETIF_EVF_DOWN	0x02

#define NETMAN_MAX_NETIF_COUNT	2

/* Network InterFace (IF) management functions. Used by the network InterFace (IF). */
int NetManRegisterNetIF(struct NetManNetIF *NetIF);
void NetManUnregisterNetIF(const char *name);
void NetManToggleNetIFLinkState(int NetIFID, unsigned char state);	//Also toggles NETMAN_NETIF_EVF_UP and NETMAN_NETIF_EVF_DOWN

#ifdef _IOP

#define netman_IMPORTS_start DECLARE_IMPORT_TABLE(netman, 3, 1)
#define netman_IMPORTS_end END_IMPORT_TABLE

#define I_NetManRegisterNetworkStack DECLARE_IMPORT(4, NetManRegisterNetworkStack)
#define I_NetManUnregisterNetworkStack DECLARE_IMPORT(5, NetManUnregisterNetworkStack)

#define I_NetManNetIFXmit DECLARE_IMPORT(6, NetManNetIFXmit)
#define I_NetManIoctl DECLARE_IMPORT(7, NetManIoctl)

#define I_NetManNetProtStackAllocPacket DECLARE_IMPORT(8, NetManNetProtStackAllocRxPacket)
#define I_NetManNetProtStackFreePacket DECLARE_IMPORT(9, NetManNetProtStackFreeRxPacket)
#define I_NetManNetProtStackEnQPacket DECLARE_IMPORT(10, NetManNetProtStackEnQRxPacket)

#define I_NetManRegisterNetIF DECLARE_IMPORT(11, NetManRegisterNetIF)
#define I_NetManUnregisterNetIF DECLARE_IMPORT(12, NetManUnregisterNetIF)
#define I_NetManToggleNetIFLinkState DECLARE_IMPORT(13, NetManToggleNetIFLinkState)
#define I_NetManGetGlobalNetIFLinkState DECLARE_IMPORT(14, NetManGetGlobalNetIFLinkState)

#define I_NetManSetMainIF DECLARE_IMPORT(15, NetManSetMainIF)
#define I_NetManQueryMainIF DECLARE_IMPORT(16, NetManQueryMainIF)

#define I_NetManSetLinkMode DECLARE_IMPORT(17, NetManSetLinkMode)

#define I_NetManTxPacketNext DECLARE_IMPORT(18, NetManTxPacketNext)
#define I_NetManTxPacketDeQ DECLARE_IMPORT(19, NetManTxPacketDeQ)

#endif

#endif /* __NETMAN_H__ */

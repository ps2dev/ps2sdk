//Common structures
#define NETMAN_NETIF_NAME_MAX_LEN	4

struct NetManPacketBuffer{
	void *handle;
	void *payload;
	unsigned int length;
};

struct NetManNetProtStack{
	void (*LinkStateUp)(void);
	void (*LinkStateDown)(void);
	struct NetManPacketBuffer *(*AllocRxPacket)(unsigned int size);
	void (*FreeRxPacket)(struct NetManPacketBuffer *packet);
	int (*EnQRxPacket)(struct NetManPacketBuffer *packet);
	int (*FlushInputQueue)(void);
};

#define NETMAN_NETIF_ETH_LINK_MODE_PAUSE	0x40	//Flow-control

enum NETMAN_NETIF_ETH_LINK_MODE{
	NETMAN_NETIF_ETH_LINK_MODE_AUTO		= 0,	//Auto negotiation cannot be reflected by NETMAN_NETIF_IOCTL_ETH_GET_LINK_MODE.
	NETMAN_NETIF_ETH_LINK_MODE_10M_HDX,		//10Mbit Half-DupleX
	NETMAN_NETIF_ETH_LINK_MODE_10M_FDX,		//10Mbit Full-DupleX
	NETMAN_NETIF_ETH_LINK_MODE_100M_HDX,		//100Mbit Half-DupleX
	NETMAN_NETIF_ETH_LINK_MODE_100M_FDX,		//100Mbit Full-DupleX
	NETMAN_NETIF_ETH_LINK_MODE_1000M,		//1000Mbit

	NETMAN_NETIF_ETH_LINK_MODE_COUNT
};

enum NETMAN_NETIF_ETH_LINK_STATE{
	NETMAN_NETIF_ETH_LINK_STATE_DOWN	= 0,
	NETMAN_NETIF_ETH_LINK_STATE_UP
};

enum NETMAN_NETIF_IOCTL_CODES{
	// Ethernet I/F-only IOCTL codes
	NETMAN_NETIF_IOCTL_ETH_GET_MAC	= 0x1000,	//Output = 6 bytes of MAC address.
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

	NETMAN_NETIF_IOCTL_ETH_SET_LINK_MODE,	//Input = struct NetManIFLinkModeParams. Note: does not wait for the IF to finish. Use NetManSetLinkMode() instead.

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
int NetManNetIFSendPacket(const void *packet, unsigned int length);

/* Network protocol stack management functions. Used by the Network InterFace (IF) driver. */
struct NetManPacketBuffer *NetManNetProtStackAllocRxPacket(unsigned int length);
void NetManNetProtStackFreeRxPacket(struct NetManPacketBuffer *packet);
int NetManNetProtStackEnQRxPacket(struct NetManPacketBuffer *packet);
int NetManNetProtStackFlushInputQueue(void);

/* NETIF flags. */
#define	NETMAN_NETIF_IN_USE	0x80	// Set internally by NETMAN. Do not set externally.
#define	NETMAN_NETIF_ETHERNET	1	// Set = network IF is an Ethernet IF.
#define	NETMAN_NETIF_DIALUP	2	// Set = network IF is a dailup modem.
#define	NETMAN_NETIF_LINK_UP	4	// Set = network IF has a link up status.

struct NetManNetIF{
	char name[NETMAN_NETIF_NAME_MAX_LEN];
	unsigned short int flags;
	short int id;	// Used internally by NETMAN. Do not use.
	int (*init)(void);
	void (*deinit)(void);
	int (*xmit)(const void *packet, unsigned int size);
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

#define netman_IMPORTS_start DECLARE_IMPORT_TABLE(netman, 1, 2)
#define netman_IMPORTS_end END_IMPORT_TABLE

#define I_NetManRegisterNetworkStack DECLARE_IMPORT(4, NetManRegisterNetworkStack)
#define I_NetManUnregisterNetworkStack DECLARE_IMPORT(5, NetManUnregisterNetworkStack)

#define I_NetManNetIFSendPacket DECLARE_IMPORT(6, NetManNetIFSendPacket)
#define I_NetManIoctl DECLARE_IMPORT(7, NetManIoctl)

#define I_NetManNetProtStackAllocPacket DECLARE_IMPORT(8, NetManNetProtStackAllocRxPacket)
#define I_NetManNetProtStackFreePacket DECLARE_IMPORT(9, NetManNetProtStackFreeRxPacket)
#define I_NetManNetProtStackEnQPacket DECLARE_IMPORT(10, NetManNetProtStackEnQRxPacket)
#define I_NetManNetProtStackFlushInputQueue DECLARE_IMPORT(11, NetManNetProtStackFlushInputQueue)

#define I_NetManRegisterNetIF DECLARE_IMPORT(12, NetManRegisterNetIF)
#define I_NetManUnregisterNetIF DECLARE_IMPORT(13, NetManUnregisterNetIF)
#define I_NetManToggleNetIFLinkState DECLARE_IMPORT(14, NetManToggleNetIFLinkState)
#define I_NetManGetGlobalNetIFLinkState DECLARE_IMPORT(15, NetManGetGlobalNetIFLinkState)

#define I_NetManSetMainIF DECLARE_IMPORT(16, NetManSetMainIF)
#define I_NetManQueryMainIF DECLARE_IMPORT(17, NetManQueryMainIF)

#define I_NetManSetLinkMode DECLARE_IMPORT(18, NetManSetLinkMode)

#endif

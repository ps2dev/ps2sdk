struct NetManPacketBuffer{
	void *handle;
	void *payload;
	unsigned int length;
	struct NetManPacketBuffer *next;
};

struct NetManNetProtStack{
	void (*LinkStateUp)(void);
	void (*LinkStateDown)(void);
	struct NetManPacketBuffer *(*AllocRxPacket)(unsigned int size);
	void (*FreeRxPacket)(struct NetManPacketBuffer *packet);
	int (*EnQRxPacket)(struct NetManPacketBuffer *packet);
	int (*FlushInputQueue)(void);
};

enum NETMAN_NETIF_ETH_LINK_MODE{
	NETMAN_NETIF_ETH_LINK_MODE_10M_HDX	= 0,	//10Mbit Half-DupleX
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
	NETMAN_NETIF_IOCTL_ETH_GET_MAC	= 0x1000,
	NETMAN_NETIF_IOCTL_ETH_GET_LINK_MODE,
	NETMAN_NETIF_IOCTL_ETH_GET_RX_EOVERRUN_CNT,
	NETMAN_NETIF_IOCTL_ETH_GET_RX_EBADLEN_CNT,
	NETMAN_NETIF_IOCTL_ETH_GET_RX_EBADFCS_CNT,
	NETMAN_NETIF_IOCTL_ETH_GET_RX_EBADALIGN_CNT,
	NETMAN_NETIF_IOCTL_ETH_GET_TX_ELOSSCR_CNT,
	NETMAN_NETIF_IOCTL_ETH_GET_TX_EEDEFER_CNT,
	NETMAN_NETIF_IOCTL_ETH_GET_TX_ECOLL_CNT,
	NETMAN_NETIF_IOCTL_ETH_GET_TX_EUNDERRUN_CNT,

	// Dial-up I/F-only IOCTL codes
	// 0x2000

	// Common IOCTL codes
	NETMAN_NETIF_IOCTL_GET_LINK_STATUS	= 0x3000,
	NETMAN_NETIF_IOCTL_GET_TX_DROPPED_COUNT,
	NETMAN_NETIF_IOCTL_GET_RX_DROPPED_COUNT,
};

int NetManInit(const struct NetManNetProtStack *stack);
void NetManDeinit(void);

/* Network protocol stack management functions. Used by the user's program. */
int NetManNetIFSendPacket(const void *packet, unsigned int length);
int NetManIoctl(unsigned int command, void *args, unsigned int args_len, void *output, unsigned int length);

/* Network protocol stack management functions. Used by the Network InterFace (IF). */
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
	char name[8];
	unsigned int flags;
	int id;	// Used internally by NETMAN. Do not use.
	int (*init)(void);
	void (*deinit)(void);
	int (*xmit)(const void *packet, unsigned int size);
	int (*ioctl)(unsigned int command, void *args, unsigned int args_len, void *output, unsigned int length);
};

#define NETMAN_MAX_NETIF_COUNT	4

/* Network InterFace (IF) management functions. Used by the network protocol stack. */
int NetManRegisterNetIF(const struct NetManNetIF *NetIF);
void NetManUnregisterNetIF(const char *name);
void NetManToggleNetIFLinkState(int NetIFID, unsigned char state);

#ifdef _IOP

#define netman_IMPORTS_start DECLARE_IMPORT_TABLE(netman, 1, 0)
#define netman_IMPORTS_end END_IMPORT_TABLE

#define I_NetManInit DECLARE_IMPORT(4, NetManInit)
#define I_NetManDeinit DECLARE_IMPORT(5, NetManDeinit)

#define I_NetManNetIFSendPacket DECLARE_IMPORT(6, NetManNetIFSendPacket)
#define I_NetManIoctl DECLARE_IMPORT(7, NetManIoctl)

#define I_NetManNetProtStackAllocPacket DECLARE_IMPORT(8, NetManNetProtStackAllocRxPacket)
#define I_NetManNetProtStackFreePacket DECLARE_IMPORT(9, NetManNetProtStackFreeRxPacket)
#define I_NetManNetProtStackEnQPacket DECLARE_IMPORT(10, NetManNetProtStackEnQRxPacket)
#define I_NetManNetProtStackFlushInputQueue DECLARE_IMPORT(11, NetManNetProtStackFlushInputQueue)

#define I_NetManRegisterNetIF DECLARE_IMPORT(12, NetManRegisterNetIF)
#define I_NetManUnregisterNetIF DECLARE_IMPORT(13, NetManUnregisterNetIF)
#define I_NetManToggleNetIFLinkState DECLARE_IMPORT(14, NetManToggleNetIFLinkState)

#endif

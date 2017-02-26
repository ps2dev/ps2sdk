/**
 * @file
 * Netman RPC common definitions
 */

#define NETMAN_RPC_NUMBER	0x00004239
#define NETMAN_RPC_BUFF_SIZE	128

enum NETMAN_EE_RPC_FUNC_NUMS{
	NETMAN_EE_RPC_FUNC_INIT=0x00,
	NETMAN_EE_RPC_FUNC_DEINIT,
	NETMAN_EE_RPC_FUNC_HANDLE_PACKETS,
	NETMAN_EE_RPC_FUNC_HANDLE_LINK_STATUS_CHANGE,
};

enum NETMAN_IOP_RPC_FUNC_NUMS{
	NETMAN_IOP_RPC_FUNC_INIT=0x00,
	NETMAN_IOP_RPC_FUNC_DEINIT,
	NETMAN_IOP_RPC_FUNC_REG_NETWORK_STACK,
	NETMAN_IOP_RPC_FUNC_UNREG_NETWORK_STACK,
	NETMAN_IOP_RPC_FUNC_IOCTL,
	NETMAN_IOP_RPC_FUNC_SEND_PACKETS,
	NETMAN_IOP_RPC_FUNC_SET_MAIN_NETIF,
	NETMAN_IOP_RPC_FUNC_QUERY_MAIN_NETIF,
	NETMAN_IOP_RPC_FUNC_SET_LINK_MODE,
};

struct NetManEEInitResult{
	s32 result;
	void *FrameBuffer;
};

struct NetManRegNetworkStackResult{
	s32 result;
	void *FrameBuffer;
};

struct NetManQueryMainNetIFResult{
	s32 result;
	char name[NETMAN_NETIF_NAME_MAX_LEN];
};

#define NETMAN_MAX_FRAME_SIZE	1520
#define NETMAN_RPC_BLOCK_SIZE	63

struct PacketReqs{
	u16 count;
	u16 length[NETMAN_RPC_BLOCK_SIZE];
};

struct NetManIoctl{
	u32 command;
	u8 args[64];
	u8 args_len;
	void *output;
	u32 length;
};

struct NetManIoctlResult{
	s32 result;
	u8 output[64];
};

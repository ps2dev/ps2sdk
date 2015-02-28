#define NETMAN_RPC_NUMBER 0x00004239

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
};

struct AlignmentData{
	void *buffer1Address;
	void *buffer2Address;
	unsigned int buffer1_len;
	unsigned int buffer2_len;

	unsigned char buffer1[64];
	unsigned char buffer2[64];
};

struct NetManEEInitResult{
	int result;
	void *FrameBuffer;
};

struct NetManRegNetworkStackResult{
	int result;
	void *FrameBuffer;
};

struct NetManQueryMainNetIFResult{
	int result;
	char name[NETMAN_NETIF_NAME_MAX_LEN];
};

#define MAX_FRAME_SIZE		1518
#define NETMAN_RPC_BLOCK_SIZE	31

struct PacketTag{
	unsigned int length;
	unsigned int offset;
};

struct PacketReqs{
	unsigned int NumPackets;
	unsigned int TotalLength;
	struct PacketTag tags[NETMAN_RPC_BLOCK_SIZE];
};

struct NetManIoctl{
	unsigned int command;
	unsigned char args[64];
	unsigned char args_len;
	void *output;
	unsigned int length;
};

struct NetManIoctlResult{
	int result;
	unsigned char output[64];
};

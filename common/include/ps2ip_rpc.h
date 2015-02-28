#ifndef _PS2IP_RPC_H
#define _PS2IP_RPC_H

#define PS2IP_IRX 0xB0125F2

enum PS2IPS_RPC_ID{
	PS2IPS_ID_ACCEPT	= 1,
	PS2IPS_ID_BIND,
	PS2IPS_ID_DISCONNECT,
	PS2IPS_ID_CONNECT,
	PS2IPS_ID_LISTEN,
	PS2IPS_ID_RECV,
	PS2IPS_ID_RECVFROM,
	PS2IPS_ID_SEND,
	PS2IPS_ID_SENDTO,
	PS2IPS_ID_SOCKET,
	PS2IPS_ID_SETCONFIG,
	PS2IPS_ID_GETCONFIG,
	PS2IPS_ID_SELECT,
	PS2IPS_ID_IOCTL,
	PS2IPS_ID_GETSOCKNAME,
	PS2IPS_ID_GETPEERNAME,
	PS2IPS_ID_GETSOCKOPT,
	PS2IPS_ID_SETSOCKOPT,
	PS2IPS_ID_GETHOSTBYNAME,

	PS2IPS_ID_COUNT
};

typedef struct {
	int  ssize;
	int  esize;
	u8 *sbuf;
	u8 *ebuf;
	u8 sbuffer[16];
	u8 ebuffer[16];
} rests_pkt; // sizeof = 48

typedef struct {
	int socket;
	int length;
	int flags;
	void *ee_addr;
	struct sockaddr sockaddr; // sizeof = 16
	int malign;
	unsigned char malign_buff[16]; // buffer for sending misaligned portion
} send_pkt;

typedef struct {
	int socket;
	int length;
	int flags;
	void *ee_addr;
	void *intr_data;
} s_recv_pkt;

typedef struct {
	int ret;
	struct sockaddr sockaddr;
} r_recv_pkt;

typedef struct {
	int socket;
	struct sockaddr sockaddr;
	int len;
} cmd_pkt;

typedef struct {
	int retval;
	struct sockaddr sockaddr;
} ret_pkt;

typedef struct {
	int s;
	int level;
	int optname;
} getsockopt_pkt;

typedef struct {
	int result;
	int optlen;
	u8 buffer[128];
} getsockopt_res_pkt;

typedef struct {
	int s;
	int level;
	int optname;
	int optlen;
	unsigned char buffer[128];
} setsockopt_pkt;

#endif	//_PS2IP_RPC_H

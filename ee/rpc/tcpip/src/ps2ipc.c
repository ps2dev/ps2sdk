/*
# _____     ___ ____     ___ ____
#  ____|   |    ____|   |        | |____|
# |     ___|   |____ ___|    ____| |    \    PS2DEV Open Source Project.
#-----------------------------------------------------------------------
# Copyright 2001-2004, ps2dev - http://www.ps2dev.org
# Licenced under Academic Free License version 2.0
# Review ps2sdk README & LICENSE files for further details.
#
# $Id$
# ps2ip ee client to iop ps2ip.
*/

#include <tamtypes.h>
#include <string.h>
#include <kernel.h>
#include <sifrpc.h>

#include <ps2ip.h>


#define PS2IP_IRX	0xB0125F2

#define ID_ACCEPT 1
#define ID_BIND   2
#define ID_DISCONNECT  3
#define ID_CONNECT 4
#define ID_LISTEN  5
#define ID_RECV    6
#define ID_RECVFROM 8
#define ID_SEND   9
#define ID_SENDTO  10
#define ID_SOCKET  11
#define ID_SETCONFIG  12
#define ID_GETCONFIG  13
#define ID_SELECT  14
#define ID_IOCTL   15

static int _init_check = 0;
static SifRpcClientData_t _ps2ip;
static int _rpc_buffer[512] __attribute__((aligned(64)));
static int _intr_data[32] __attribute__((aligned(64)));

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

int ps2ip_init()
{
	int i;

    memset(&_ps2ip, 0, sizeof(_ps2ip));

	while(1)
	{
		if(SifBindRpc(&_ps2ip, PS2IP_IRX, 0) < 0)
			return -1;

		if(_ps2ip.server != 0) 
			break;

		i = 0x10000;
		while(i--);
	}
 
	_init_check = 1;
	
	return 0;
}

int accept(int s, struct sockaddr *addr, int *addrlen)
{
	cmd_pkt *pkt = (cmd_pkt *)_rpc_buffer;

	if(!_init_check) return -1;

	pkt->socket = s;

	SifCallRpc(&_ps2ip, ID_ACCEPT, 0, (void*)_rpc_buffer, 4, (void*)_rpc_buffer, sizeof(cmd_pkt), 0, 0);

	if(pkt->len < *addrlen) *addrlen = pkt->len;
	memcpy((void *)addr, (void *)&pkt->sockaddr, *addrlen);

	return pkt->socket;
}

int bind(int s, struct sockaddr *name, int namelen)
{
	cmd_pkt *pkt = (cmd_pkt *)_rpc_buffer;

	if(!_init_check) return -1;

	pkt->socket = s;
	pkt->len = namelen;
	memcpy((void *)&pkt->sockaddr, (void *)name, sizeof(struct sockaddr));

	SifCallRpc(&_ps2ip, ID_BIND, 0, (void*)_rpc_buffer, sizeof(cmd_pkt), (void*)_rpc_buffer, 4, 0, 0);

	return _rpc_buffer[0];
}

int disconnect(int s)
{
	if(!_init_check) return -1;

	_rpc_buffer[0] = s;

	SifCallRpc(&_ps2ip, ID_DISCONNECT, 0, (void*)_rpc_buffer, 4, (void*)_rpc_buffer, 4, 0, 0);

	return _rpc_buffer[0];
}

int connect(int s, struct sockaddr *name, int namelen)
{
	cmd_pkt *pkt = (cmd_pkt *)_rpc_buffer;

	if(!_init_check) return -1;

	pkt->socket = s;
	pkt->len = namelen;
	memcpy((void *)&pkt->sockaddr, (void *)name, sizeof(struct sockaddr));

	SifCallRpc(&_ps2ip, ID_CONNECT, 0, (void*)_rpc_buffer, sizeof(cmd_pkt), (void*)_rpc_buffer, 4, 0, 0);

	return _rpc_buffer[0];
}

int listen(int s, int backlog)
{
	if(!_init_check) return -1;
	
	_rpc_buffer[0] = s;
	_rpc_buffer[1] = backlog;

	SifCallRpc(&_ps2ip, ID_LISTEN, 0, (void*)_rpc_buffer, 8, (void*)_rpc_buffer, 4, 0, 0);

	return _rpc_buffer[0];
}

static void recv_intr(void *data_raw)
{
	rests_pkt *rests = UNCACHED_SEG(data_raw);
	int i;

	if(rests->ssize)
		for(i = 0; i < rests->ssize; i++)
			rests->sbuf[i] = rests->sbuffer[i];

	if(rests->esize)
		for(i = 0; i < rests->esize; i++)
			rests->ebuf[i] = rests->ebuffer[i];
}
			

int recv(int s, void *mem, int len, unsigned int flags)
{
	s_recv_pkt *send_pkt = (s_recv_pkt *)_rpc_buffer;
	r_recv_pkt *recv_pkt = (r_recv_pkt *)_rpc_buffer;

	if(!_init_check) return -1;

	send_pkt->socket = s;
	send_pkt->length = len;
	send_pkt->flags = flags;
	send_pkt->ee_addr = mem;
	send_pkt->intr_data = _intr_data;

	if( !IS_UNCACHED_SEG(mem))
		SifWriteBackDCache(mem, len);
	SifWriteBackDCache(_intr_data, 128);
	SifWriteBackDCache(_rpc_buffer, sizeof(s_recv_pkt));

	SifCallRpc(&_ps2ip, ID_RECV, 0, (void*)_rpc_buffer, sizeof(s_recv_pkt), 
				(void*)_rpc_buffer, sizeof(r_recv_pkt), recv_intr, _intr_data);

	return recv_pkt->ret;
}

int recvfrom(int s, void *mem, int len, unsigned int flags,
		  struct sockaddr *from, int *fromlen)
{
	s_recv_pkt *send_pkt = (s_recv_pkt *)_rpc_buffer;
	r_recv_pkt *recv_pkt = (r_recv_pkt *)_rpc_buffer;

	if(!_init_check) return -1;

	send_pkt->socket = s;
	send_pkt->length = len;
	send_pkt->flags = flags;
	send_pkt->ee_addr = mem;
	send_pkt->intr_data = _intr_data;

	if( !IS_UNCACHED_SEG(mem))
		SifWriteBackDCache(mem, len);
	SifWriteBackDCache(_intr_data, 128);
	SifWriteBackDCache(_rpc_buffer, sizeof(s_recv_pkt));

	SifCallRpc(&_ps2ip, ID_RECVFROM, 0, (void*)_rpc_buffer, sizeof(s_recv_pkt), 
				(void*)_rpc_buffer, sizeof(r_recv_pkt), recv_intr, _intr_data);

	memcpy((void *)from, (void *)&recv_pkt->sockaddr, sizeof(struct sockaddr));
	*fromlen = sizeof(struct sockaddr);

	return recv_pkt->ret;
}

int send(int s, void *dataptr, int size, unsigned int flags)
{
	send_pkt *pkt = (send_pkt *)_rpc_buffer;
	int miss;

	pkt->socket = s;
	pkt->length = size;
	pkt->flags = flags;
	pkt->ee_addr = dataptr;

	if((u32)dataptr & 0xf)
	{
		miss = 16 - ((u32)dataptr & 0xf);
		if(miss > size) miss = size;

	} else {

		miss = 0;
	}

	pkt->malign = miss;

	if( !IS_UNCACHED_SEG(dataptr))
		SifWriteBackDCache(dataptr, size);

	memcpy((void *)pkt->malign_buff, UNCACHED_SEG(dataptr), miss);

	SifCallRpc(&_ps2ip, ID_SEND, 0, (void*)_rpc_buffer, sizeof(send_pkt), 
				(void*)_rpc_buffer, 4, 0, 0);

	return _rpc_buffer[0];
}

int sendto(int s, void *dataptr, int size, unsigned int flags,
		struct sockaddr *to, int tolen)
{
	send_pkt *pkt = (send_pkt *)_rpc_buffer;
	int miss;

	pkt->socket = s;
	pkt->length = size;
	pkt->flags = flags;
	pkt->ee_addr = dataptr;
	memcpy((void *)&pkt->sockaddr, (void *)to, sizeof(struct sockaddr));

	if((u32)dataptr & 0xf)
	{
		miss = 16 - ((u32)dataptr & 0xf);
		if(miss > size) miss = size;

	} else {

		miss = 0;
	}

	pkt->malign = miss;

	if( !IS_UNCACHED_SEG(dataptr))
		SifWriteBackDCache(dataptr, size);

	memcpy((void *)pkt->malign_buff, UNCACHED_SEG(dataptr), miss);

	SifCallRpc(&_ps2ip, ID_SENDTO, 0, (void*)_rpc_buffer, sizeof(send_pkt), 
				(void*)_rpc_buffer, 4, 0, 0);

	return _rpc_buffer[0];
}

int socket(int domain, int type, int protocol)
{
	if(!_init_check) return -1;

	_rpc_buffer[0] = domain;
	_rpc_buffer[1] = type;
	_rpc_buffer[2] = protocol;

	SifCallRpc(&_ps2ip, ID_SOCKET, 0, (void*)_rpc_buffer, 12, (void*)_rpc_buffer, 4, 0, 0);

	return _rpc_buffer[0];
}

int ps2ip_setconfig(t_ip_info *ip_info)
{
	if(!_init_check) return -1;

    // return config    
    memcpy(_rpc_buffer, ip_info, sizeof(t_ip_info));

	SifCallRpc(&_ps2ip, ID_SETCONFIG, 0, (void*)_rpc_buffer, sizeof(t_ip_info), (void*)_rpc_buffer, 4, 0, 0);
    return _rpc_buffer[0];
}

int ps2ip_getconfig(char *netif_name, t_ip_info *ip_info)
{
	if(!_init_check) return -1;
    
    // call with netif name
    memcpy(_rpc_buffer, netif_name, 8);
    
	SifCallRpc(&_ps2ip, ID_GETCONFIG, 0, (void*)_rpc_buffer, 8, (void*)_rpc_buffer, sizeof(t_ip_info), 0, 0);

    // return config    
    memcpy(ip_info, _rpc_buffer, sizeof(t_ip_info));
    
    return 1;
}

int select(int maxfdp1, struct fd_set *readset, struct fd_set *writeset, struct fd_set *exceptset, struct timeval *timeout)
{
	if(!_init_check) return -1;

	_rpc_buffer[0] = (int)maxfdp1;
	_rpc_buffer[1] = (int)readset;
	_rpc_buffer[2] = (int)writeset;
	_rpc_buffer[3] = (int)exceptset;
	_rpc_buffer[4] = (int)timeout;
	if( timeout )
	{
		((long*)_rpc_buffer)[3] = timeout->tv_sec;
		((long*)_rpc_buffer)[4] = timeout->tv_usec;
	}
	if( readset )
	{
		((char*)_rpc_buffer)[40] = readset->fd_bits[0];
		((char*)_rpc_buffer)[41] = readset->fd_bits[1];
	}
	if( writeset )
	{
		((char*)_rpc_buffer)[42] = writeset->fd_bits[0];
		((char*)_rpc_buffer)[43] = writeset->fd_bits[1];
	}
	if( exceptset )
 	{
		((char*)_rpc_buffer)[44] = exceptset->fd_bits[0];
		((char*)_rpc_buffer)[45] = exceptset->fd_bits[1];
	}

	SifCallRpc(&_ps2ip, ID_SELECT, 0, (void*)_rpc_buffer, 46, (void*)_rpc_buffer, 4, 0, 0);

	return _rpc_buffer[0];
}

int ioctlsocket(int s, long cmd, void *argp)
{
	((int*)_rpc_buffer)[0] = s;
	((int*)_rpc_buffer)[1] = (int)cmd;
	((int*)_rpc_buffer)[2] = (int)argp;
	if( argp )
		((int*)_rpc_buffer)[3] = *(int*)argp;

	SifCallRpc(&_ps2ip, ID_IOCTL, 0, (void*)_rpc_buffer, 16, (void*)_rpc_buffer, 4, 0, 0);

	return _rpc_buffer[0];
}

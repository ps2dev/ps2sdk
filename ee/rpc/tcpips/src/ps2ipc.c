/*
# _____     ___ ____     ___ ____
#  ____|   |    ____|   |        | |____|
# |     ___|   |____ ___|    ____| |    \    PS2DEV Open Source Project.
#-----------------------------------------------------------------------
# Copyright 2001-2004, ps2dev - http://www.ps2dev.org
# Licenced under Academic Free License version 2.0
# Review ps2sdk README & LICENSE files for further details.
*/

/**
 * @file
 * ps2ip ee client to iop ps2ip.
 */

#include <tamtypes.h>
#include <string.h>
#include <kernel.h>
#include <sifrpc.h>

#include <ps2ips.h>
#include <ps2ip_rpc.h>

static int _init_check = 0;
static SifRpcClientData_t _ps2ip;
static int _rpc_buffer[512] __attribute__((aligned(64)));
static int _intr_data[32] __attribute__((aligned(64)));

int ps2ip_init(void)
{
	int i;

	memset(&_ps2ip, 0, sizeof(_ps2ip));

	while(1)
	{
		if(SifBindRpc(&_ps2ip, PS2IP_IRX, 0) < 0)
			return -1;

		if(_ps2ip.server != 0)
			break;

		for(i = 0x10000; i > 0; i--);
	}

	_init_check = 1;

	return 0;
}

int accept(int s, struct sockaddr *addr, int *addrlen)
{
	cmd_pkt *pkt = (cmd_pkt *)_rpc_buffer;

	if(!_init_check) return -1;

	pkt->socket = s;

	SifCallRpc(&_ps2ip, PS2IPS_ID_ACCEPT, 0, (void*)_rpc_buffer, 4, (void*)_rpc_buffer, sizeof(cmd_pkt), NULL, NULL);

	if(addr != NULL)
	{
		if(pkt->len < *addrlen) *addrlen = pkt->len;
		memcpy((void *)addr, (void *)&pkt->sockaddr, *addrlen);
	}

	return pkt->socket;
}

int bind(int s, struct sockaddr *name, int namelen)
{
	cmd_pkt *pkt = (cmd_pkt *)_rpc_buffer;

	if(!_init_check) return -1;

	pkt->socket = s;
	pkt->len = namelen;
	memcpy((void *)&pkt->sockaddr, (void *)name, sizeof(struct sockaddr));

	SifCallRpc(&_ps2ip, PS2IPS_ID_BIND, 0, (void*)_rpc_buffer, sizeof(cmd_pkt), (void*)_rpc_buffer, 4, NULL, NULL);

	return _rpc_buffer[0];
}

int disconnect(int s)
{
	if(!_init_check) return -1;

	_rpc_buffer[0] = s;

	SifCallRpc(&_ps2ip, PS2IPS_ID_DISCONNECT, 0, (void*)_rpc_buffer, 4, (void*)_rpc_buffer, 4, NULL, NULL);

	return _rpc_buffer[0];
}

int connect(int s, struct sockaddr *name, int namelen)
{
	cmd_pkt *pkt = (cmd_pkt *)_rpc_buffer;

	if(!_init_check) return -1;

	pkt->socket = s;
	pkt->len = namelen;
	memcpy((void *)&pkt->sockaddr, (void *)name, sizeof(struct sockaddr));

	SifCallRpc(&_ps2ip, PS2IPS_ID_CONNECT, 0, (void*)_rpc_buffer, sizeof(cmd_pkt), (void*)_rpc_buffer, 4, NULL, NULL);

	return _rpc_buffer[0];
}

int listen(int s, int backlog)
{
	if(!_init_check) return -1;

	_rpc_buffer[0] = s;
	_rpc_buffer[1] = backlog;

	SifCallRpc(&_ps2ip, PS2IPS_ID_LISTEN, 0, (void*)_rpc_buffer, 8, (void*)_rpc_buffer, 4, NULL, NULL);

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

	SifCallRpc(&_ps2ip, PS2IPS_ID_RECV, 0, (void*)_rpc_buffer, sizeof(s_recv_pkt),
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

	SifCallRpc(&_ps2ip, PS2IPS_ID_RECVFROM, 0, (void*)_rpc_buffer, sizeof(s_recv_pkt),
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

	SifCallRpc(&_ps2ip, PS2IPS_ID_SEND, 0, (void*)_rpc_buffer, sizeof(send_pkt),
				(void*)_rpc_buffer, 4, NULL, NULL);

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

	SifCallRpc(&_ps2ip, PS2IPS_ID_SENDTO, 0, (void*)_rpc_buffer, sizeof(send_pkt),
				(void*)_rpc_buffer, 4, NULL, NULL);

	return _rpc_buffer[0];
}

int socket(int domain, int type, int protocol)
{
	if(!_init_check) return -1;

	_rpc_buffer[0] = domain;
	_rpc_buffer[1] = type;
	_rpc_buffer[2] = protocol;

	SifCallRpc(&_ps2ip, PS2IPS_ID_SOCKET, 0, (void*)_rpc_buffer, 12, (void*)_rpc_buffer, 4, NULL, NULL);

	return _rpc_buffer[0];
}

int ps2ip_setconfig(t_ip_info *ip_info)
{
	if(!_init_check) return -1;

	// return config
	memcpy(_rpc_buffer, ip_info, sizeof(t_ip_info));

	SifCallRpc(&_ps2ip, PS2IPS_ID_SETCONFIG, 0, (void*)_rpc_buffer, sizeof(t_ip_info), (void*)_rpc_buffer, 4, NULL, NULL);
	return _rpc_buffer[0];
}

int ps2ip_getconfig(char *netif_name, t_ip_info *ip_info)
{
	if(!_init_check) return -1;

	// call with netif name
	memcpy(_rpc_buffer, netif_name, 8);

	SifCallRpc(&_ps2ip, PS2IPS_ID_GETCONFIG, 0, (void*)_rpc_buffer, 8, (void*)_rpc_buffer, sizeof(t_ip_info), NULL, NULL);

	// return config
	memcpy(ip_info, _rpc_buffer, sizeof(t_ip_info));

	return 1;
}

int select(int maxfdp1, struct fd_set *readset, struct fd_set *writeset, struct fd_set *exceptset, struct timeval *timeout)
{
	select_pkt *pkt = (select_pkt*)_rpc_buffer;

	if(!_init_check) return -1;

	pkt->maxfdp1 = maxfdp1;
	pkt->readset_p = readset;
	pkt->writeset_p = writeset;
	pkt->exceptset_p = exceptset;
	pkt->timeout_p = timeout;
	if( timeout )
		pkt->timeout = *timeout;

	if( readset )
		pkt->readset = *readset;

	if( writeset )
		pkt->writeset = *writeset;

	if( exceptset )
		pkt->exceptset = *exceptset;

	SifCallRpc(&_ps2ip, PS2IPS_ID_SELECT, 0, (void*)_rpc_buffer, sizeof(select_pkt), (void*)_rpc_buffer, sizeof(select_pkt), NULL, NULL);

	if( timeout )
		*timeout = pkt->timeout;

	if( readset )
		*readset = pkt->readset;

	if( writeset )
		*writeset = pkt->writeset;

	if( exceptset )
		*exceptset = pkt->exceptset;

	return pkt->result;
}

int ioctlsocket(int s, long cmd, void *argp)
{
	ioctl_pkt *pkt = (ioctl_pkt*)_rpc_buffer;
	pkt->s = s;
	pkt->cmd = (s32)cmd;
	pkt->argp = argp;
	if( argp )
		pkt->value = *(s32*)argp;

	SifCallRpc(&_ps2ip, PS2IPS_ID_IOCTL, 0, (void*)_rpc_buffer, 16, (void*)_rpc_buffer, 4, NULL, NULL);

	if( argp )
		*(s32*)argp = pkt->value;

	return pkt->result;
}

int getsockname(int s, struct sockaddr *name, int *namelen)
{
	cmd_pkt *pkt = (cmd_pkt *)_rpc_buffer;

	if(!_init_check) return -1;

	pkt->socket = s;

	SifCallRpc(&_ps2ip, PS2IPS_ID_GETSOCKNAME, 0, (void*)_rpc_buffer, 4, (void*)_rpc_buffer, sizeof(cmd_pkt), NULL, NULL);

	if(pkt->len < *namelen) *namelen = pkt->len;
	memcpy((void *)name, (void *)&pkt->sockaddr, *namelen);

	return pkt->socket;
}

int getpeername(int s, struct sockaddr *name, int *namelen)
{
	cmd_pkt *pkt = (cmd_pkt *)_rpc_buffer;

	if(!_init_check) return -1;

	pkt->socket = s;

	SifCallRpc(&_ps2ip, PS2IPS_ID_GETPEERNAME, 0, (void*)_rpc_buffer, 4, (void*)_rpc_buffer, sizeof(cmd_pkt), NULL, NULL);

	if(pkt->len < *namelen) *namelen = pkt->len;
	memcpy((void *)name, (void *)&pkt->sockaddr, *namelen);

	return pkt->socket;
}

int getsockopt(int s, int level, int optname, void* optval, socklen_t* optlen)
{
	((getsockopt_pkt*)_rpc_buffer)->s = s;
	((getsockopt_pkt*)_rpc_buffer)->level = level;
	((getsockopt_pkt*)_rpc_buffer)->optname = optname;

	SifCallRpc(&_ps2ip, PS2IPS_ID_GETSOCKOPT, 0, (void*)_rpc_buffer, sizeof(getsockopt_pkt), (void*)_rpc_buffer, sizeof(getsockopt_res_pkt), NULL, NULL);

	if(((getsockopt_res_pkt*)_rpc_buffer)->optlen < *optlen) *optlen = ((getsockopt_res_pkt*)_rpc_buffer)->optlen;
	memcpy((void*)optval, ((getsockopt_res_pkt*)_rpc_buffer)->buffer, *optlen);

	return ((getsockopt_res_pkt*)_rpc_buffer)->result;
}

int setsockopt(int s, int level, int optname, const void *optval, socklen_t optlen)
{
	((setsockopt_pkt*)_rpc_buffer)->s = s;
	((setsockopt_pkt*)_rpc_buffer)->s = level;
	((setsockopt_pkt*)_rpc_buffer)->optname = optname;
	((setsockopt_pkt*)_rpc_buffer)->optlen = optlen;

	memcpy(((setsockopt_pkt*)_rpc_buffer)->buffer, optval, optlen);

	SifCallRpc(&_ps2ip, PS2IPS_ID_SETSOCKOPT, 0, (void*)_rpc_buffer, sizeof(setsockopt_pkt), (void*)_rpc_buffer, sizeof(int), NULL, NULL);

	return _rpc_buffer[0];
}

#ifdef PS2IP_DNS
struct hostent *gethostbyname(const char *name)
{
	struct hostent *result;
	static ip_addr_t addr;
	static ip_addr_t *addr_list[2];
	static struct hostent hostent;

	result = NULL;
	memcpy(_rpc_buffer, name, 256);
	if(SifCallRpc(&_ps2ip, PS2IPS_ID_GETHOSTBYNAME, 0, (void*)_rpc_buffer, 256, (void*)_rpc_buffer, sizeof(gethostbyname_res_pkt), NULL, NULL) >=0)
	{
		if(((gethostbyname_res_pkt*)_rpc_buffer)->result == 0)
		{
			hostent.h_addrtype = ((gethostbyname_res_pkt*)_rpc_buffer)->hostent.h_addrtype;
			hostent.h_length = ((gethostbyname_res_pkt*)_rpc_buffer)->hostent.h_length;
			hostent.h_name = (char*)name;
			hostent.h_aliases = NULL;
			memcpy(&addr, &((gethostbyname_res_pkt*)_rpc_buffer)->hostent.h_addr, sizeof(addr));
			addr_list[0] = &addr;
			addr_list[1] = NULL;
			hostent.h_addr_list = (char**)&addr_list;
			result = &hostent;
		}
	}

	return result;
}

void dns_setserver(u8 numdns, ip_addr_t *dnsserver)
{
	((dns_setserver_pkt*)_rpc_buffer)->numdns = numdns;
	((dns_setserver_pkt*)_rpc_buffer)->dnsserver = *dnsserver;

	SifCallRpc(&_ps2ip, PS2IPS_ID_DNS_SETSERVER, 0, (void*)_rpc_buffer, sizeof(dns_setserver_pkt), NULL, 0, NULL, NULL);
}

ip_addr_t dns_getserver(u8 numdns)
{
	ip_addr_t info;

	info.addr = 0;
	*(u8*)_rpc_buffer = numdns;

	if(SifCallRpc(&_ps2ip, PS2IPS_ID_DNS_GETSERVER, 0, (void*)_rpc_buffer, sizeof(u8), (void*)_rpc_buffer, sizeof(dns_getserver_res_pkt), NULL, NULL) >=0)
		info = ((dns_getserver_res_pkt*)_rpc_buffer)->dnsserver;

	return info;
}
#endif

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
#include <iopcontrol.h>

#include <ps2ips.h>
#include <ps2ip_rpc.h>

static int lock_sema;
static SifRpcClientData_t _ps2ip;
static struct {
	union {
		s32 result;
		s32 s;			//Generic socket parameter.
		cmd_pkt cmd_pkt;	//Generic command packet, used by multiple functions.
		listen_pkt listen_pkt;
		s_recv_pkt s_recv_pkt;
		r_recv_pkt r_recv_pkt;
		send_pkt send_pkt;
		socket_pkt socket_pkt;
		t_ip_info ip_info;
		char netif_name[8];
		select_pkt select_pkt;
		ioctl_pkt ioctl_pkt;
		getsockopt_pkt getsockopt_pkt;
		getsockopt_res_pkt getsockopt_res_pkt;
		setsockopt_pkt setsockopt_pkt;
		char hostname[256];
		gethostbyname_res_pkt gethostbyname_res_pkt;
		dns_setserver_pkt dns_setserver_pkt;
		dns_getserver_res_pkt dns_getserver_res_pkt;
		u8 numdns;
		u8 buffer[512];
	};
} _rpc_buffer __attribute__((aligned(64)));
static int _intr_data[32] __attribute__((aligned(64)));

static ip_addr_t dns_servers[DNS_MAX_SERVERS];

//Copied from LWIP, to be independent of the full LWIP source.
/* used by IP4_ADDR_ANY and IP_ADDR_BROADCAST in ip_addr.h */
const ip_addr_t ip_addr_any = IPADDR4_INIT(IPADDR_ANY);

/* The following are defined in ps2ipc_ps2sdk.c */
extern void _ps2sdk_ps2ipc_init(void);
extern void _ps2sdk_ps2ipc_deinit(void);

int ps2ip_init(void)
{
	ee_sema_t sema;
	if (HasIopRebootedSinceLastCall())
		ps2ip_deinit();

	if (_ps2ip.server)
		return 0;

	while(1)
	{
		if(sceSifBindRpc(&_ps2ip, PS2IP_IRX, 0) < 0)
			return -1;

		if(_ps2ip.server != NULL)
			break;

		nopdelay();
	}

	sema.init_count = 1;
	sema.max_count = 1;
	sema.option = (u32)"ps2ipc";
	sema.attr = 0;
	lock_sema = CreateSema(&sema);

	_ps2sdk_ps2ipc_init();

	return 0;
}

void ps2ip_deinit(void)
{
	_ps2sdk_ps2ipc_deinit();

	if (lock_sema > 0)
		DeleteSema(lock_sema);
	lock_sema = 0;

	memset(&_ps2ip, 0, sizeof(_ps2ip));
}

int ps2ipc_accept(int s, struct sockaddr *addr, int *addrlen)
{
	int result;
	cmd_pkt *pkt = &_rpc_buffer.cmd_pkt;

	if(!_ps2ip.server) return -1;

	WaitSema(lock_sema);

	pkt->socket = s;

	if (sceSifCallRpc(&_ps2ip, PS2IPS_ID_ACCEPT, 0, (void*)pkt, sizeof(s32), (void*)pkt, sizeof(cmd_pkt), NULL, NULL) < 0)
	{
		SignalSema(lock_sema);
		return -1;
	}

	if(addr != NULL)
	{
		if(pkt->len < *addrlen) *addrlen = pkt->len;
		memcpy((void *)addr, (void *)&pkt->sockaddr, *addrlen);
	}

	result = pkt->socket;

	SignalSema(lock_sema);

	return result;
}

int ps2ipc_bind(int s, const struct sockaddr *name, int namelen)
{
	cmd_pkt *pkt = &_rpc_buffer.cmd_pkt;
	int result;

	if(!_ps2ip.server) return -1;

	WaitSema(lock_sema);

	pkt->socket = s;
	pkt->len = namelen;
	pkt->sockaddr = *name;

	if (sceSifCallRpc(&_ps2ip, PS2IPS_ID_BIND, 0, (void*)pkt, sizeof(cmd_pkt), (void*)&_rpc_buffer.result, sizeof(s32), NULL, NULL) < 0)
	{
		SignalSema(lock_sema);
		return -1;
	}

	result = _rpc_buffer.result;

	SignalSema(lock_sema);

	return result;
}

int ps2ipc_disconnect(int s)
{
	int result;

	if(!_ps2ip.server) return -1;

	WaitSema(lock_sema);

	_rpc_buffer.s = s;

	if (sceSifCallRpc(&_ps2ip, PS2IPS_ID_DISCONNECT, 0, (void*)&_rpc_buffer.s, sizeof(s32), (void*)&_rpc_buffer.result, sizeof(s32), NULL, NULL) < 0)
	{
		SignalSema(lock_sema);
		return -1;
	}

	result = _rpc_buffer.result;

	SignalSema(lock_sema);

	return result;
}

int ps2ipc_connect(int s, const struct sockaddr *name, int namelen)
{
	int result;
	cmd_pkt *pkt = &_rpc_buffer.cmd_pkt;

	if(!_ps2ip.server) return -1;

	WaitSema(lock_sema);

	pkt->socket = s;
	pkt->len = namelen;
	pkt->sockaddr = *name;

	if (sceSifCallRpc(&_ps2ip, PS2IPS_ID_CONNECT, 0, (void*)pkt, sizeof(cmd_pkt), (void*)&_rpc_buffer.result, sizeof(s32), NULL, NULL) < 0)
	{
		SignalSema(lock_sema);
		return -1;
	}

	result = _rpc_buffer.result;

	SignalSema(lock_sema);

	return result;
}

int ps2ipc_listen(int s, int backlog)
{
	int result;
	listen_pkt *pkt = &_rpc_buffer.listen_pkt;

	if(!_ps2ip.server) return -1;

	WaitSema(lock_sema);

	pkt->s = s;
	pkt->backlog = backlog;

	if (sceSifCallRpc(&_ps2ip, PS2IPS_ID_LISTEN, 0, (void*)pkt, sizeof(listen_pkt), (void*)&_rpc_buffer.result, sizeof(s32), NULL, NULL) < 0)
	{
		SignalSema(lock_sema);
		return -1;
	}

	result = _rpc_buffer.result;

	SignalSema(lock_sema);

	return result;
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


int ps2ipc_recv(int s, void *mem, int len, unsigned int flags)
{
	int result;
	s_recv_pkt *send_pkt = &_rpc_buffer.s_recv_pkt;
	r_recv_pkt *recv_pkt = &_rpc_buffer.r_recv_pkt;

	if(!_ps2ip.server) return -1;

	WaitSema(lock_sema);

	send_pkt->socket = s;
	send_pkt->length = len;
	send_pkt->flags = flags;
	send_pkt->ee_addr = mem;
	send_pkt->intr_data = _intr_data;

	if( !IS_UNCACHED_SEG(mem))
		sceSifWriteBackDCache(mem, len);

	if (sceSifCallRpc(&_ps2ip, PS2IPS_ID_RECV, 0, (void*)send_pkt, sizeof(s_recv_pkt),
				(void*)recv_pkt, sizeof(r_recv_pkt), recv_intr, _intr_data) < 0)
	{
		SignalSema(lock_sema);
		return -1;
	}

	result = recv_pkt->ret;

	SignalSema(lock_sema);

	return result;
}

int ps2ipc_recvfrom(int s, void *mem, int len, unsigned int flags,
		  struct sockaddr *from, int *fromlen)
{
	int result;
	s_recv_pkt *send_pkt = &_rpc_buffer.s_recv_pkt;
	r_recv_pkt *recv_pkt = &_rpc_buffer.r_recv_pkt;

	if(!_ps2ip.server) return -1;

	WaitSema(lock_sema);

	send_pkt->socket = s;
	send_pkt->length = len;
	send_pkt->flags = flags;
	send_pkt->ee_addr = mem;
	send_pkt->intr_data = _intr_data;

	if( !IS_UNCACHED_SEG(mem))
		sceSifWriteBackDCache(mem, len);

	if (sceSifCallRpc(&_ps2ip, PS2IPS_ID_RECVFROM, 0, (void*)send_pkt, sizeof(s_recv_pkt),
				(void*)recv_pkt, sizeof(r_recv_pkt), recv_intr, _intr_data) < 0)
	{
		SignalSema(lock_sema);
		return -1;
	}

	*from = recv_pkt->sockaddr;
	*fromlen = sizeof(struct sockaddr);

	result = recv_pkt->ret;

	SignalSema(lock_sema);

	return result;
}

int ps2ipc_send(int s, const void *dataptr, int size, unsigned int flags)
{
	int result;
	send_pkt *pkt = &_rpc_buffer.send_pkt;
	int miss;

	WaitSema(lock_sema);

	pkt->socket = s;
	pkt->length = size;
	pkt->flags = flags;
	pkt->ee_addr = (void *)dataptr;

	if((u32)dataptr & 0x3f)
	{
		miss = 64 - ((u32)dataptr & 0x3f);
		if(miss > size) miss = size;

	} else {

		miss = 0;
	}

	pkt->malign = miss;

	if( !IS_UNCACHED_SEG(dataptr))
		sceSifWriteBackDCache((void *)dataptr, size);

	memcpy((void *)pkt->malign_buff, UNCACHED_SEG(dataptr), miss);

	if (sceSifCallRpc(&_ps2ip, PS2IPS_ID_SEND, 0, (void*)pkt, sizeof(send_pkt),
				(void*)&_rpc_buffer.result, sizeof(s32), NULL, NULL) < 0)
	{
		SignalSema(lock_sema);
		return -1;
	}

	result = _rpc_buffer.result;

	SignalSema(lock_sema);

	return result;
}

int ps2ipc_sendto(int s, const void *dataptr, int size, unsigned int flags,
		const struct sockaddr *to, int tolen)
{
	int result;
	send_pkt *pkt = &_rpc_buffer.send_pkt;
	int miss;

	(void)tolen;

	WaitSema(lock_sema);

	pkt->socket = s;
	pkt->length = size;
	pkt->flags = flags;
	pkt->ee_addr = (void *)dataptr;
	pkt->sockaddr = *to;

	if((u32)dataptr & 0x3f)
	{
		miss = 64 - ((u32)dataptr & 0x3f);
		if(miss > size) miss = size;

	} else {

		miss = 0;
	}

	pkt->malign = miss;

	if( !IS_UNCACHED_SEG(dataptr))
		sceSifWriteBackDCache((void *)dataptr, size);

	memcpy((void *)pkt->malign_buff, UNCACHED_SEG(dataptr), miss);

	if (sceSifCallRpc(&_ps2ip, PS2IPS_ID_SENDTO, 0, (void*)pkt, sizeof(send_pkt),
				(void*)&_rpc_buffer.result, sizeof(s32), NULL, NULL) < 0)
	{
		SignalSema(lock_sema);
		return -1;
	}

	result = _rpc_buffer.result;

	SignalSema(lock_sema);

	return result;
}

int ps2ipc_socket(int domain, int type, int protocol)
{
	int result;
	socket_pkt *pkt = &_rpc_buffer.socket_pkt;

	if(!_ps2ip.server) return -1;

	WaitSema(lock_sema);

	pkt->domain = domain;
	pkt->type = type;
	pkt->protocol = protocol;

	if (sceSifCallRpc(&_ps2ip, PS2IPS_ID_SOCKET, 0, (void*)pkt, sizeof(socket_pkt), (void*)&_rpc_buffer.result, sizeof(s32), NULL, NULL) < 0)
	{
		SignalSema(lock_sema);
		return -1;
	}

	result = _rpc_buffer.result;

	SignalSema(lock_sema);

	return result;
}

int ps2ipc_ps2ip_setconfig(const t_ip_info *ip_info)
{
	int result;

	if(!_ps2ip.server) return -1;

	WaitSema(lock_sema);

	// return config
	_rpc_buffer.ip_info = *ip_info;

	if (sceSifCallRpc(&_ps2ip, PS2IPS_ID_SETCONFIG, 0, (void*)&_rpc_buffer.ip_info, sizeof(t_ip_info), (void*)&_rpc_buffer.result, sizeof(s32), NULL, NULL) < 0)
	{
		SignalSema(lock_sema);
		return -1;
	}

	result = _rpc_buffer.result;

	SignalSema(lock_sema);

	return result;
}

int ps2ipc_ps2ip_getconfig(char *netif_name, t_ip_info *ip_info)
{
	if(!_ps2ip.server) return -1;

	WaitSema(lock_sema);

	// call with netif name
	strncpy(_rpc_buffer.netif_name, netif_name, sizeof(_rpc_buffer.netif_name));
	_rpc_buffer.netif_name[sizeof(_rpc_buffer.netif_name) - 1] = '\0';

	if (sceSifCallRpc(&_ps2ip, PS2IPS_ID_GETCONFIG, 0, (void*)_rpc_buffer.netif_name, sizeof(_rpc_buffer.netif_name), (void*)&_rpc_buffer.ip_info, sizeof(t_ip_info), NULL, NULL) < 0)
	{
		SignalSema(lock_sema);
		return -1;
	}

	// return config
	*ip_info = _rpc_buffer.ip_info;

	SignalSema(lock_sema);

	return 1;
}

/* Marshal between the caller's struct fd_set (newlib, ~128 bytes on EE)
 * and the on-the-wire ps2ip_rpc_fd_set (MEMP_NUM_NETCONN/8 bytes, fixed
 * size on both sides). Bit-by-bit copy avoids any reliance on
 * sizeof(struct fd_set) matching across the RPC boundary. */
static void ps2ipc_pack_fdset(ps2ip_rpc_fd_set *dst, struct fd_set *src, int maxfdp1)
{
	int i;
	memset(dst, 0, sizeof(*dst));
	if (src == NULL) return;
	if (maxfdp1 > (int)(sizeof(dst->fd_bits) * 8))
		maxfdp1 = (int)(sizeof(dst->fd_bits) * 8);
	for (i = 0; i < maxfdp1; i++) {
		if (FD_ISSET(i, src)) {
			dst->fd_bits[i >> 3] |= (unsigned char)(1U << (i & 7));
		}
	}
}

static void ps2ipc_unpack_fdset(struct fd_set *dst, const ps2ip_rpc_fd_set *src, int maxfdp1)
{
	int i;
	if (dst == NULL) return;
	FD_ZERO(dst);
	if (maxfdp1 > (int)(sizeof(src->fd_bits) * 8))
		maxfdp1 = (int)(sizeof(src->fd_bits) * 8);
	for (i = 0; i < maxfdp1; i++) {
		if (src->fd_bits[i >> 3] & (1U << (i & 7))) {
			FD_SET(i, dst);
		}
	}
}

int ps2ipc_select(int maxfdp1, struct fd_set *readset, struct fd_set *writeset, struct fd_set *exceptset, struct timeval *timeout)
{
	int result;
	select_pkt *pkt = &_rpc_buffer.select_pkt;

	if(!_ps2ip.server) return -1;

	WaitSema(lock_sema);

	pkt->maxfdp1 = maxfdp1;
	pkt->readset_p = (void *)readset;
	pkt->writeset_p = (void *)writeset;
	pkt->exceptset_p = (void *)exceptset;
	pkt->timeout_p = timeout;
	if( timeout )
	{
		pkt->timeout_sec  = (s32)timeout->tv_sec;
		pkt->timeout_usec = (s32)timeout->tv_usec;
	}

	ps2ipc_pack_fdset(&pkt->readset,   readset,   maxfdp1);
	ps2ipc_pack_fdset(&pkt->writeset,  writeset,  maxfdp1);
	ps2ipc_pack_fdset(&pkt->exceptset, exceptset, maxfdp1);

	if (sceSifCallRpc(&_ps2ip, PS2IPS_ID_SELECT, 0, (void*)pkt, sizeof(select_pkt), (void*)pkt, sizeof(select_pkt), NULL, NULL) < 0)
	{
		SignalSema(lock_sema);
		return -1;
	}

	if( timeout )
	{
		timeout->tv_sec  = pkt->timeout_sec;
		timeout->tv_usec = pkt->timeout_usec;
	}

	ps2ipc_unpack_fdset(readset,   &pkt->readset,   maxfdp1);
	ps2ipc_unpack_fdset(writeset,  &pkt->writeset,  maxfdp1);
	ps2ipc_unpack_fdset(exceptset, &pkt->exceptset, maxfdp1);

	result = pkt->result;

	SignalSema(lock_sema);

	return result;
}

int ps2ipc_ioctl(int s, long cmd, void *argp)
{
	int result;
	ioctl_pkt *pkt = &_rpc_buffer.ioctl_pkt;

	if(!_ps2ip.server) return -1;

	WaitSema(lock_sema);

	pkt->s = s;
	pkt->cmd = (s32)cmd;
	pkt->argp = argp;
	if( argp )
		pkt->value = *(s32*)argp;

	if (sceSifCallRpc(&_ps2ip, PS2IPS_ID_IOCTL, 0, (void*)pkt, sizeof(ioctl_pkt), (void*)pkt, sizeof(ioctl_pkt), NULL, NULL) < 0)
	{
		SignalSema(lock_sema);
		return -1;
	}

	if( argp )
		*(s32*)argp = pkt->value;

	result = pkt->result;

	SignalSema(lock_sema);

	return result;
}

int ps2ipc_getsockname(int s, struct sockaddr *name, int *namelen)
{
	int result;
	cmd_pkt *pkt = &_rpc_buffer.cmd_pkt;

	if(!_ps2ip.server) return -1;

	WaitSema(lock_sema);

	pkt->socket = s;

	if (sceSifCallRpc(&_ps2ip, PS2IPS_ID_GETSOCKNAME, 0, (void*)pkt, sizeof(pkt->socket), (void*)pkt, sizeof(cmd_pkt), NULL, NULL) < 0)
	{
		SignalSema(lock_sema);
		return -1;
	}

	if(pkt->len < *namelen) *namelen = pkt->len;
	memcpy((void *)name, (void *)&pkt->sockaddr, *namelen);

	result = pkt->socket;

	SignalSema(lock_sema);

	return result;
}

int ps2ipc_getpeername(int s, struct sockaddr *name, int *namelen)
{
	int result;
	cmd_pkt *pkt = &_rpc_buffer.cmd_pkt;

	if(!_ps2ip.server) return -1;

	WaitSema(lock_sema);

	pkt->socket = s;

	if (sceSifCallRpc(&_ps2ip, PS2IPS_ID_GETPEERNAME, 0, (void*)pkt, sizeof(pkt->socket), (void*)pkt, sizeof(cmd_pkt), NULL, NULL) < 0)
	{
		SignalSema(lock_sema);
		return -1;
	}

	if(pkt->len < *namelen) *namelen = pkt->len;
	memcpy((void *)name, (void *)&pkt->sockaddr, *namelen);

	result = pkt->socket;

	SignalSema(lock_sema);

	return result;
}

int ps2ipc_getsockopt(int s, int level, int optname, void* optval, socklen_t* optlen)
{
	getsockopt_pkt *pkt = &_rpc_buffer.getsockopt_pkt;
	getsockopt_res_pkt *res_pkt = &_rpc_buffer.getsockopt_res_pkt;
	int result;

	if(!_ps2ip.server) return -1;

	WaitSema(lock_sema);

	pkt->s = s;
	pkt->level = level;
	pkt->optname = optname;

	if (sceSifCallRpc(&_ps2ip, PS2IPS_ID_GETSOCKOPT, 0, (void*)pkt, sizeof(getsockopt_pkt), (void*)res_pkt, sizeof(getsockopt_res_pkt), NULL, NULL) < 0)
	{
		SignalSema(lock_sema);
		return -1;
	}

	if(res_pkt->optlen < *optlen) *optlen = res_pkt->optlen;
	memcpy((void*)optval, res_pkt->buffer, *optlen);

	result = res_pkt->result;

	SignalSema(lock_sema);

	return result;
}

int ps2ipc_setsockopt(int s, int level, int optname, const void *optval, socklen_t optlen)
{
	setsockopt_pkt *pkt = &_rpc_buffer.setsockopt_pkt;
	int result;

	if(!_ps2ip.server) return -1;

	WaitSema(lock_sema);

	pkt->s = s;
	pkt->level = level;
	pkt->optname = optname;
	pkt->optlen = optlen;

	memcpy(pkt->buffer, optval, optlen);

	if (sceSifCallRpc(&_ps2ip, PS2IPS_ID_SETSOCKOPT, 0, (void*)pkt, sizeof(setsockopt_pkt), (void*)&_rpc_buffer.result, sizeof(s32), NULL, NULL) < 0)
	{
		SignalSema(lock_sema);
		return -1;
	}

	result = _rpc_buffer.result;

	SignalSema(lock_sema);

	return result;
}

#ifdef PS2IP_DNS
struct hostent *ps2ipc_gethostbyname(const char *name)
{
	gethostbyname_res_pkt *res_pkt = &_rpc_buffer.gethostbyname_res_pkt;
	struct hostent *result;
	static ip_addr_t addr;
	static ip_addr_t *addr_list[2];
	static struct hostent hostent;

	if(!_ps2ip.server) return NULL;

	WaitSema(lock_sema);

	result = NULL;
	strncpy(_rpc_buffer.hostname, name, sizeof(_rpc_buffer.hostname));
	_rpc_buffer.hostname[sizeof(_rpc_buffer.hostname) - 1] = '\0';
	if(sceSifCallRpc(&_ps2ip, PS2IPS_ID_GETHOSTBYNAME, 0, (void*)_rpc_buffer.hostname, sizeof(_rpc_buffer.hostname), (void*)res_pkt, sizeof(gethostbyname_res_pkt), NULL, NULL) >=0)
	{
		if(res_pkt->result == 0)
		{
			hostent.h_addrtype = res_pkt->hostent.h_addrtype;
			hostent.h_length = res_pkt->hostent.h_length;
			hostent.h_name = (char*)name;
			hostent.h_aliases = NULL;
			addr = res_pkt->hostent.h_addr;
			addr_list[0] = &addr;
			addr_list[1] = NULL;
			hostent.h_addr_list = (char**)&addr_list;
			result = &hostent;
		}
	}

	SignalSema(lock_sema);

	return result;
}

void ps2ipc_dns_setserver(u8 numdns, const ip_addr_t *dnsserver)
{
	dns_setserver_pkt *pkt = &_rpc_buffer.dns_setserver_pkt;

	if(!_ps2ip.server) return;

	WaitSema(lock_sema);

	pkt->numdns = numdns;
	pkt->dnsserver = (dnsserver != NULL) ? (*dnsserver) : *IP4_ADDR_ANY;

	sceSifCallRpc(&_ps2ip, PS2IPS_ID_DNS_SETSERVER, 0, (void*)pkt, sizeof(dns_setserver_pkt), NULL, 0, NULL, NULL);

	if (numdns < DNS_MAX_SERVERS)
		dns_servers[numdns] = (dnsserver != NULL) ? (*dnsserver) : *IP4_ADDR_ANY;

	SignalSema(lock_sema);
}

const ip_addr_t *ps2ipc_dns_getserver(u8 numdns)
{
	dns_getserver_res_pkt *res_pkt = &_rpc_buffer.dns_getserver_res_pkt;
	ip_addr_t *dns;

	if ((!_ps2ip.server) || (numdns >= DNS_MAX_SERVERS))
		return IP4_ADDR_ANY;

	WaitSema(lock_sema);

	_rpc_buffer.numdns = numdns;
	dns = &dns_servers[numdns];

	//If this fails, use the cached copy.
	if(sceSifCallRpc(&_ps2ip, PS2IPS_ID_DNS_GETSERVER, 0, (void*)&_rpc_buffer.numdns, sizeof(u8), (void*)res_pkt, sizeof(dns_getserver_res_pkt), NULL, NULL) >=0)
		ip_addr_copy(*dns, res_pkt->dnsserver);

	SignalSema(lock_sema);

	return dns;
}
#endif

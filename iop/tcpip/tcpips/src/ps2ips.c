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
# Remote Procedure Call server for ps2ip. Depends on ps2ip.irx and dns.irx.
*/

/*

NOTES:

- recv/recvfrom/send/sendto are designed so that the data buffer can be misaligned, and the recv/send size is
  not restricted to a multiple of 16 bytes. The implimentation method was borrowed from fileio
  read/write code :P

*/

#include <types.h>
#include <loadcore.h>
#include <stdio.h>
#include <sifman.h>
#include <sifcmd.h>
#include <sysclib.h>
#include <thbase.h>
#include <intrman.h>
#include <ps2ip.h>
#include <ps2ip_rpc.h>

#define MODNAME	"TCP/IP_Stack_RPC"
IRX_ID(MODNAME, 1, 1);

#define BUFF_SIZE	(1024)

#define MIN(a, b)	(((a)<(b))?(a):(b))
#define RDOWN_16(a)	(((a) >> 4) << 4)

static SifRpcDataQueue_t ps2ips_queue;
static SifRpcServerData_t ps2ips_server;
static int _rpc_buffer[512];

static char lwip_buffer[BUFF_SIZE + 32];
static rests_pkt rests;

static void do_accept( void * rpcBuffer, int size )
{
	int *ptr = rpcBuffer;
	cmd_pkt *pkt = (cmd_pkt *)ptr;
	struct sockaddr addr;
	int addrlen, ret;

	ret = accept(pkt->socket, &addr, &addrlen);

	pkt->socket = ret;
	memcpy(&pkt->sockaddr, &addr, sizeof(struct sockaddr));
	pkt->len = sizeof(struct sockaddr);
}

static void do_bind( void * rpcBuffer, int size )
{
	int *ptr = rpcBuffer;
	cmd_pkt *pkt = (cmd_pkt *)rpcBuffer;
	int ret;

	ret = bind(pkt->socket, &pkt->sockaddr, pkt->len);

	ptr[0] = ret;
}

static void do_disconnect( void * rpcBuffer, int size )
{
	int *ptr = rpcBuffer;
	int ret;

	ret = disconnect(ptr[0]);

	ptr[0] = ret;
}

static void do_connect( void * rpcBuffer, int size )
{
	int *ptr = rpcBuffer;
	cmd_pkt *pkt = (cmd_pkt *)_rpc_buffer;
	int ret;

	ret = connect(pkt->socket, &pkt->sockaddr, pkt->len);

	ptr[0] = ret;
}

static void do_listen( void * rpcBuffer, int size )
{
	int *ptr = rpcBuffer;
	int ret;

	ret = listen(ptr[0], ptr[1]);

	ptr[0] = ret;
}

static void do_recv( void * rpcBuffer, int size )
{
	int srest, erest, asize; // size of unaligned portion, remainder portion, aligned portion
	void *abuffer, *aebuffer; // aligned buffer start, aligned buffer end
	int s_offset; // offset into lwip_buffer of srest data
	int rlen, recvlen;
	int dma_id = 0;
	int intr_stat;
	s_recv_pkt *recv_pkt = (s_recv_pkt *)rpcBuffer;
	r_recv_pkt *ret_pkt = (r_recv_pkt *)rpcBuffer;
	struct t_SifDmaTransfer sifdma;

	if(recv_pkt->length <= 16)
	{
		srest = recv_pkt->length;

	} else {

		if( ((int)recv_pkt->ee_addr & 0xF) == 0 ) // ee address is aligned
			srest = 0;
		else
			srest = RDOWN_16((int)recv_pkt->ee_addr) - (int)recv_pkt->ee_addr + 16;
	}

	s_offset = 16 - srest;
	recvlen = MIN(BUFF_SIZE, recv_pkt->length);

	// Do actual TCP recv
	rlen = recv(recv_pkt->socket, lwip_buffer + s_offset, recvlen, recv_pkt->flags);

	if(rlen <= 0) goto recv_end;
	if(rlen <= 16) srest = rlen;

	// fill sbuffer, calculate align buffer & erest valules, fill ebuffer
	if(srest)
		memcpy((void *)rests.sbuffer, (void *)(lwip_buffer + s_offset), srest);

	if(rlen > 16)
	{
		abuffer = recv_pkt->ee_addr + srest; // aligned buffer (round up)
		aebuffer = (void *)RDOWN_16((int)recv_pkt->ee_addr + rlen); // aligned buffer end (round down)

		asize = (int)aebuffer - (int)abuffer;

		erest = recv_pkt->ee_addr + rlen - aebuffer;

		if(erest)
			memcpy((void *)rests.ebuffer, (void *)(lwip_buffer + 16 + asize), erest);

//		printf("srest = 0x%X\nabuffer = 0x%X\naebuffer = 0x%X\nasize = 0x%X\nerest = 0x%X\n", srest, abuffer, aebuffer, asize, erest);

	} else {

		erest = asize = (int)abuffer = (int)aebuffer = 0;

	}

	// DMA back the abuffer
	if(asize)
	{
		while(SifDmaStat(dma_id) >= 0);

		sifdma.src = lwip_buffer + 16;
		sifdma.dest = abuffer;
		sifdma.size = asize;
		sifdma.attr = 0;
		CpuSuspendIntr(&intr_stat);
		dma_id = SifSetDma(&sifdma, 1);
		CpuResumeIntr(intr_stat);
	}

	// Fill rest of rests structure, dma back
	rests.ssize = srest;
	rests.esize = erest;
	rests.sbuf = recv_pkt->ee_addr;
	rests.ebuf = aebuffer;

	while(SifDmaStat(dma_id) >= 0);

	sifdma.src = &rests;
	sifdma.dest = recv_pkt->intr_data;
	sifdma.size = sizeof(rests_pkt);
	sifdma.attr = 0;
	CpuSuspendIntr(&intr_stat);
	dma_id = SifSetDma(&sifdma, 1);
	CpuResumeIntr(intr_stat);

recv_end:
	ret_pkt->ret = rlen;
}

static void do_recvfrom( void * rpcBuffer, int size )
{
	int srest, erest, asize; // size of unaligned portion, remainder portion, aligned portion
	void *abuffer, *aebuffer; // aligned buffer start, aligned buffer end
	int s_offset; // offset into lwip_buffer of srest data
	int rlen, recvlen;
	int dma_id = 0;
	int intr_stat;
	s_recv_pkt *recv_pkt = (s_recv_pkt *)rpcBuffer;
	r_recv_pkt *ret_pkt = (r_recv_pkt *)rpcBuffer;
	static struct t_SifDmaTransfer sifdma;
	static struct sockaddr sockaddr;
	int fromlen;

	if(recv_pkt->length <= 16)
	{
		srest = recv_pkt->length;

	} else {

		if( ((int)recv_pkt->ee_addr & 0xF) == 0 ) // ee address is aligned
			srest = 0;
		else
			srest = RDOWN_16((int)recv_pkt->ee_addr) - (int)recv_pkt->ee_addr + 16;
	}

	s_offset = 16 - srest;
	recvlen = MIN(BUFF_SIZE, recv_pkt->length);

	// Do actual UDP recvfrom
	rlen = recvfrom(recv_pkt->socket, lwip_buffer + s_offset, recvlen, recv_pkt->flags, &sockaddr, &fromlen);

	if(rlen <= 0) goto recv_end;
	if(rlen <= 16) srest = rlen;

	// copy sockaddr struct to return packet
	memcpy((void *)&ret_pkt->sockaddr, (void *)&sockaddr, sizeof(struct sockaddr));

	// fill sbuffer, calculate align buffer & erest valules, fill ebuffer
	if(srest)
		memcpy((void *)rests.sbuffer, (void *)(lwip_buffer + s_offset), srest);

	if(rlen > 16)
	{
		abuffer = recv_pkt->ee_addr + srest; // aligned buffer (round up)
		aebuffer = (void *)RDOWN_16((int)recv_pkt->ee_addr + rlen); // aligned buffer end (round down)

		asize = (int)aebuffer - (int)abuffer;

		erest = recv_pkt->ee_addr + rlen - aebuffer;

		if(erest)
			memcpy((void *)rests.ebuffer, (void *)(lwip_buffer + 16 + asize), erest);

//		printf("srest = 0x%X\nabuffer = 0x%X\naebuffer = 0x%X\nasize = 0x%X\nerest = 0x%X\n", srest, abuffer, aebuffer, asize, erest);

	} else {

		erest = asize = (int)abuffer = (int)aebuffer = 0;

	}

	// DMA back the abuffer
	if(asize)
	{
		while(SifDmaStat(dma_id) >= 0);

		sifdma.src = lwip_buffer + 16;
		sifdma.dest = abuffer;
		sifdma.size = asize;
		sifdma.attr = 0;
		CpuSuspendIntr(&intr_stat);
		dma_id = SifSetDma(&sifdma, 1);
		CpuResumeIntr(intr_stat);
	}

	// Fill rest of rests structure, dma back
	rests.ssize = srest;
	rests.esize = erest;
	rests.sbuf = recv_pkt->ee_addr;
	rests.ebuf = aebuffer;

	while(SifDmaStat(dma_id) >= 0);

	sifdma.src = &rests;
	sifdma.dest = recv_pkt->intr_data;
	sifdma.size = sizeof(rests_pkt);
	sifdma.attr = 0;
	CpuSuspendIntr(&intr_stat);
	dma_id = SifSetDma(&sifdma, 1);
	CpuResumeIntr(intr_stat);

recv_end:
	ret_pkt->ret = rlen;
}

static void do_send( void * rpcBuffer, int size )
{
	int *ptr = rpcBuffer;
	send_pkt *pkt = (send_pkt *)rpcBuffer;
	int slen, sendlen;
	int s_offset; // offset into lwip_buffer for malign data
	void *ee_pos;
	SifRpcReceiveData_t rdata;

	if(pkt->malign)
	{
//		printf("send: misaligned = %d\n", pkt->malign);

		s_offset = 16 - pkt->malign;
		memcpy((void *)(lwip_buffer + s_offset), pkt->malign_buff, pkt->malign);

	} else s_offset = 16;

	ee_pos = pkt->ee_addr + pkt->malign;

	sendlen = MIN(BUFF_SIZE, pkt->length);

	SifRpcGetOtherData(&rdata, ee_pos, lwip_buffer + 16, sendlen - pkt->malign, 0);

	// So actual TCP send
	slen = send(pkt->socket, lwip_buffer + s_offset, sendlen, pkt->flags);

	ptr[0] = slen;
}


static void do_sendto( void * rpcBuffer, int size )
{
	int *ptr = rpcBuffer;
	send_pkt *pkt = (send_pkt *)rpcBuffer;
	int slen, sendlen;
	int s_offset; // offset into lwip_buffer for malign data
	void *ee_pos;
	SifRpcReceiveData_t rdata;

	if(pkt->malign)
	{
//		printf("send: misaligned = %d\n", pkt->malign);

		s_offset = 16 - pkt->malign;
		memcpy((void *)(lwip_buffer + s_offset), pkt->malign_buff, pkt->malign);

	} else s_offset = 16;

	ee_pos = pkt->ee_addr + pkt->malign;

	sendlen = MIN(BUFF_SIZE, pkt->length);

	SifRpcGetOtherData(&rdata, ee_pos, lwip_buffer + 16, sendlen - pkt->malign, 0);

	// So actual UDP sendto
	slen = sendto(pkt->socket, lwip_buffer + s_offset, sendlen, pkt->flags, &pkt->sockaddr, sizeof(struct sockaddr));

	ptr[0] = slen;
}

static void do_socket( void * rpcBuffer, int size )
{
	int *ptr = rpcBuffer;
	int ret;

	ret = socket(ptr[0], ptr[1], ptr[2]);

	ptr[0] = ret;
}

static void do_getconfig(void *rpcBuffer, int size)
{
	ps2ip_getconfig((char *)rpcBuffer, (t_ip_info *)rpcBuffer);
}

static void do_setconfig(void *rpcBuffer, int size)
{
	ps2ip_setconfig((t_ip_info *)rpcBuffer);
}

static void do_select( void * rpcBuffer, int size )
{
	struct timeval timeout;
	struct fd_set readset;
	struct fd_set writeset;
	struct fd_set exceptset;
	struct fd_set *readset_p = NULL;
	struct fd_set *writeset_p = NULL;
	struct fd_set *exceptset_p = NULL;
	struct timeval *timeout_p = NULL;
	int *ptr = rpcBuffer;
	int ret;
	int maxfdp1;

	maxfdp1 = ((int*)rpcBuffer)[0];
	readset_p = (struct fd_set *)((int*)rpcBuffer)[1];
	writeset_p = (struct fd_set *)((int*)rpcBuffer)[2];
	exceptset_p = (struct fd_set *)((int*)rpcBuffer)[3];
	timeout_p = (struct timeval *)((int*)rpcBuffer)[4];
	if( timeout_p )
	{
		timeout_p = &timeout;
		timeout_p->tv_sec = ((long long*)rpcBuffer)[3];
		timeout_p->tv_usec = ((long long*)rpcBuffer)[4];
	}
	if( readset_p )
	{
		readset_p = &readset;
		readset_p->fd_bits[0] = ((char*)rpcBuffer)[40];
		readset_p->fd_bits[1] = ((char*)rpcBuffer)[41];
	}
	if( writeset_p )
	{
		writeset_p = &writeset;
		writeset_p->fd_bits[0] = ((char*)rpcBuffer)[42];
		writeset_p->fd_bits[1] = ((char*)rpcBuffer)[43];
	}
	if( exceptset_p )
	{
		exceptset_p = &exceptset;
		exceptset_p->fd_bits[0] = ((char*)rpcBuffer)[44];
		exceptset_p->fd_bits[1] = ((char*)rpcBuffer)[45];
	}

	ret = select( maxfdp1, readset_p, writeset_p, exceptset_p, timeout_p );
	ptr[0] = ret;

	if( timeout_p )
	{
		((long long*)rpcBuffer)[3] = timeout_p->tv_sec;
		((long long*)rpcBuffer)[4] = timeout_p->tv_usec;
	}
	if( readset_p )
	{
		((char*)rpcBuffer)[40] = readset_p->fd_bits[0];
		((char*)rpcBuffer)[41] = readset_p->fd_bits[1];
	}
	if( writeset_p )
	{
		((char*)rpcBuffer)[42] = writeset_p->fd_bits[0];
		((char*)rpcBuffer)[43] = writeset_p->fd_bits[1];
	}
	if( exceptset_p )
	{
		((char*)rpcBuffer)[44] = exceptset_p->fd_bits[0];
		((char*)rpcBuffer)[45] = exceptset_p->fd_bits[1];
	}
}

// cmd should be 64 bits wide; I think.
static void do_ioctlsocket( void *rpcBuffer, int size )
{
	int *ptr = rpcBuffer, ret;
	int s;
	unsigned int cmd;
	unsigned int argpv;
	void *argp;

	s = ((int*)_rpc_buffer)[0];
	cmd = ((unsigned int*)_rpc_buffer)[1];
	argp = (void*)(((int*)_rpc_buffer)[2]);
	if( argp )
	{
		argpv = ((int*)_rpc_buffer)[3];
		argp = &argpv;
	}

	ret = ioctlsocket( s, cmd, argp );
	ptr[0] = ret;
}
static void do_getsockname( void *rpcBuffer, int size )
{
	int *ptr = rpcBuffer;
	cmd_pkt *pkt = (cmd_pkt *)ptr;
	struct sockaddr addr;
	int addrlen, ret;

	ret = getsockname(pkt->socket, &addr, &addrlen);

	pkt->socket = ret;
	memcpy(&pkt->sockaddr, &addr, sizeof(struct sockaddr));
	pkt->len = sizeof(struct sockaddr);
}

static void do_getpeername( void *rpcBuffer, int size )
{
	int *ptr = rpcBuffer;
	cmd_pkt *pkt = (cmd_pkt *)ptr;
	struct sockaddr addr;
	int addrlen, ret;

	ret = getpeername(pkt->socket, &addr, &addrlen);

	pkt->socket = ret;
	memcpy(&pkt->sockaddr, &addr, sizeof(struct sockaddr));
	pkt->len = sizeof(struct sockaddr);
}

static void do_getsockopt( void *rpcBuffer, int size )
{
	int ret, s, level, optname, optlen;
	unsigned char optval[128];

	s	= ((getsockopt_pkt*)rpcBuffer)->s;
	level	= ((getsockopt_pkt*)rpcBuffer)->level;
	optname	= ((getsockopt_pkt*)rpcBuffer)->optname;
	optlen	= sizeof(optval);

	ret = getsockopt(s, level, optname, optval, &optlen);

	((getsockopt_res_pkt*)rpcBuffer)->result = ret;
	((getsockopt_res_pkt*)rpcBuffer)->optlen = optlen;
	memcpy( ((getsockopt_res_pkt*)rpcBuffer)->buffer, optval, 128 );
}

static void do_setsockopt( void *rpcBuffer, int size )
{
	int *ptr = rpcBuffer, ret;
	int s;
	int level;
	int optname;
	int optlen;
	unsigned char optval[128];

	s	= ((setsockopt_pkt*)rpcBuffer)->s;
	level	= ((setsockopt_pkt*)rpcBuffer)->level;
	optname	= ((setsockopt_pkt*)rpcBuffer)->optname;
	optlen	= ((setsockopt_pkt*)rpcBuffer)->optlen;
	memcpy(optval, ((setsockopt_pkt*)rpcBuffer)->buffer, optlen);

	ret = setsockopt(s, level, optname, optval, optlen);
	ptr[0] = ret;
}

#ifdef PS2IP_DNS
static void do_gethostbyname( void *rpcBuffer, int size )
{
	struct hostent *ret;
	gethostbyname_res_pkt *resPtr = rpcBuffer;

	if((ret = gethostbyname((char*)_rpc_buffer)) != NULL)
	{
		resPtr->result = 0;
		resPtr->hostent.h_addrtype = ret->h_addrtype;
		resPtr->hostent.h_length = ret->h_length;
		memcpy(&resPtr->hostent.h_addr, &ret->h_addr_list[0], sizeof(resPtr->hostent.h_addr));
	}else{
		resPtr->result = -1;
	}
}

static void do_dns_setserver( void *rpcBuffer, int size )
{
	dns_setserver(((dns_setserver_pkt*)_rpc_buffer)->numdns, &((dns_setserver_pkt*)_rpc_buffer)->dnsserver);
}

static void do_dns_getserver( void *rpcBuffer, int size )
{
	((dns_getserver_res_pkt*)rpcBuffer)->dnsserver = dns_getserver(*(u8*)_rpc_buffer);
}
#endif

static void * rpcHandlerFunction(unsigned int command, void * rpcBuffer, int size)
{
	switch(command)
	{
	case PS2IPS_ID_ACCEPT:
		do_accept(rpcBuffer, size);
		break;
	case PS2IPS_ID_BIND:
		do_bind(rpcBuffer, size);
		break;
	case PS2IPS_ID_DISCONNECT:
		do_disconnect(rpcBuffer, size);
		break;
	case PS2IPS_ID_CONNECT:
		do_connect(rpcBuffer, size);
		break;
	case PS2IPS_ID_LISTEN:
		do_listen(rpcBuffer, size);
		break;
	case PS2IPS_ID_RECV:
		do_recv(rpcBuffer, size);
		break;
	case PS2IPS_ID_RECVFROM:
		do_recvfrom(rpcBuffer, size);
		break;
	case PS2IPS_ID_SEND:
		do_send(rpcBuffer, size);
		break;
	case PS2IPS_ID_SENDTO:
		do_sendto(rpcBuffer, size);
		break;
	case PS2IPS_ID_SOCKET:
		do_socket(rpcBuffer, size);
		break;
	case PS2IPS_ID_GETCONFIG:
		do_getconfig(rpcBuffer, size);
		break;
	case PS2IPS_ID_SETCONFIG:
		do_setconfig(rpcBuffer, size);
		break;
	case PS2IPS_ID_SELECT:
		do_select(rpcBuffer, size);
		break;
	case PS2IPS_ID_IOCTL:
		do_ioctlsocket(rpcBuffer, size);
		break;
	case PS2IPS_ID_GETSOCKNAME:
		do_getsockname(rpcBuffer, size);
		break;
	case PS2IPS_ID_GETPEERNAME:
		do_getpeername(rpcBuffer, size);
		break;
	case PS2IPS_ID_GETSOCKOPT:
		do_getsockopt(rpcBuffer, size);
		break;
	case PS2IPS_ID_SETSOCKOPT:
		do_setsockopt(rpcBuffer, size);
		break;
#ifdef PS2IP_DNS
	case PS2IPS_ID_GETHOSTBYNAME:
		do_gethostbyname(rpcBuffer, size);
		break;
	case PS2IPS_ID_DNS_SETSERVER:
		do_dns_setserver(rpcBuffer, size);
		break;
	case PS2IPS_ID_DNS_GETSERVER:
		do_dns_getserver(rpcBuffer, size);
		break;
#endif
	default:
		printf("PS2IPS: Unknown Function called!\n");

  }

  return rpcBuffer;
}

static void threadRpcFunction(void *arg)
{
	printf("PS2IPS: RPC Thread Started\n");

	SifSetRpcQueue( &ps2ips_queue , GetThreadId() );
	SifRegisterRpc( &ps2ips_server, PS2IP_IRX, (void *)rpcHandlerFunction,(u8 *)&_rpc_buffer,NULL,NULL, &ps2ips_queue );
	SifRpcLoop( &ps2ips_queue );
}

int _start( int argc, char *argv[])
{
	int		threadId;
	iop_thread_t	t;

	printf( "PS2IPS: Module Loaded.\n" );

	t.attr = TH_C;
	t.option = 0;
	t.thread = &threadRpcFunction;
	t.stacksize = 0x800;
	t.priority = 0x1e;

	threadId = CreateThread( &t );
	if ( threadId < 0 )
	{
		printf( "PS2IPS: CreateThread failed.  %i\n", threadId );
		return MODULE_NO_RESIDENT_END;
	}
	else
	{
		StartThread( threadId, NULL );
		return MODULE_RESIDENT_END;
	}
}

/*
  _____     ___ ____
   ____|   |    ____|      PSX2 OpenSource Project
  |     ___|   |____       (C)2003, Nick Van Veen (nickvv@xtra.co.nz)
  ------------------------------------------------------------------------
  ps2ips.c                Remote Procedure Call server for ps2ip
*/

/* 

NOTES:

- recv/recvfrom/send/sendto are designed so that the data buffer can be misaligned, and the recv/send size is
  not restricted to a multiple of 16 bytes. The implimentation method was borrowed from fileio
  read/write code :P

*/

#include <types.h>
#include <stdio.h>
#include <sifman.h>
#include <sifrpc.h>
#include <sysmem.h>
#include <sysclib.h>
#include <thbase.h>
#include <intrman.h>
#include <ps2ip.h>

#define PS2IP_IRX 0xB0125F2

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
#define ID_SELECT   14
#define ID_IOCTL    15

#define BUFF_SIZE	(1024)

#define MIN(a, b)	(((a)<(b))?(a):(b))
#define RDOWN_16(a)	(((a) >> 4) << 4)

static SifRpcDataQueue_t ps2ips_queue __attribute__((aligned(64)));
static SifRpcServerData_t ps2ips_server __attribute((aligned(64)));
static int _rpc_buffer[512] __attribute((aligned(64)));

static char *lwip_buffer;

typedef struct {
	int  ssize;
	int  esize;
	void *sbuf;
	void *ebuf;
	u8 sbuffer[16];
	u8 ebuffer[16];
} rests_pkt; // sizeof = 48

static rests_pkt rests __attribute((aligned(64)));

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

void do_accept( void * rpcBuffer, int size )
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

void do_bind( void * rpcBuffer, int size )
{
	int *ptr = rpcBuffer;
	cmd_pkt *pkt = (cmd_pkt *)rpcBuffer;
	int ret;

	ret = bind(pkt->socket, &pkt->sockaddr, pkt->len);

	ptr[0] = ret;
}

void do_disconnect( void * rpcBuffer, int size )
{
	int *ptr = rpcBuffer;
	int ret;

	ret = disconnect(ptr[0]);

	ptr[0] = ret;
}

void do_connect( void * rpcBuffer, int size )
{
	int *ptr = rpcBuffer;
	cmd_pkt *pkt = (cmd_pkt *)_rpc_buffer;
	int ret;

	ret = connect(pkt->socket, &pkt->sockaddr, pkt->len);

	ptr[0] = ret;
}

void do_listen( void * rpcBuffer, int size )
{
	int *ptr = rpcBuffer;
	int ret;

	ret = listen(ptr[0], ptr[1]);

	ptr[0] = ret;
}

/*
void do_recv( void * rpcBuffer, int size )
{
	int srest, erest, asize; // size of unaligned portion, remainder portion, aligned portion
	void *abuffer, *aebuffer; // aligned buffer start, aligned buffer end
	int total = 0, rlen, iter_len;
	int dma_id = 0;
	int intr_stat;
	s_recv_pkt *recv_pkt = (s_recv_pkt *)rpcBuffer;
	r_recv_pkt *ret_pkt = (r_recv_pkt *)rpcBuffer;
	struct t_SifDmaTransfer sifdma;

	if(recv_pkt->length < 16)
	{
		srest = recv_pkt->length;
		erest = asize = (int)abuffer = (int)aebuffer = 0;

	} else {

		if( ((int)recv_pkt->ee_addr & 0xF) == 0 ) // ee address is aligned
			srest = 0;
		else
			srest = RDOWN_16((int)recv_pkt->ee_addr) - (int)recv_pkt->ee_addr + 16;

		abuffer = recv_pkt->ee_addr + srest; // aligned buffer start (round up)
		aebuffer = (void *)RDOWN_16((int)recv_pkt->ee_addr + recv_pkt->length); // aligned buffer end (round down)

		asize = (int)aebuffer - (int)abuffer;

		erest = recv_pkt->ee_addr + recv_pkt->length - aebuffer;
	}

	if(srest > 0)
	{
//		printf("PS2IPS: recv: srest = %d\n", srest);

		rlen = recv(recv_pkt->socket, rests.sbuffer, srest, recv_pkt->flags);

		if(rlen != srest)
		{
			if(rlen > 0)
				total += rlen;
			else total = rlen;

			goto recv_end;
		}

		total += rlen;
	}

	while(asize > 0)
	{
		iter_len = MIN(BUFF_SIZE, asize);

		while(SifDmaStat(dma_id) >= 0); // Wait for previous DMA transfer to complete

		rlen = recv(recv_pkt->socket, lwip_buffer, iter_len, recv_pkt->flags);

		if(rlen != iter_len) // incomplete recv
		{
			int align_size;

			if(rlen <= 0)
			{
				total = rlen;
				erest = 0;
				goto recv_end;
			}

			// Now must dma back data what was received, then re-calculate erest info
//			align_size = (rlen >> 4) << 4;
			align_size = RDOWN_16(rlen);
			erest = rlen - align_size;
			aebuffer = abuffer + align_size;
			memcpy((void *)rests.ebuffer, (void *)&lwip_buffer[align_size], erest);

//			printf("PS2IPS: recv: NEW erest = %d\n", erest);

			total += rlen;

			if(align_size >= 16)
			{
				sifdma.src = lwip_buffer;
				sifdma.dest = abuffer;
				sifdma.size = rlen;
				sifdma.attr = 0;

				CpuSuspendIntr(&intr_stat);
				dma_id = SifSetDma(&sifdma, 1);
				CpuResumeIntr(intr_stat);
			}

			goto recv_end;	

		} else { // recv ok
			
			sifdma.src = lwip_buffer;
			sifdma.dest = abuffer;
			sifdma.size = rlen;
			sifdma.attr = 0;

			total += rlen;
			asize -= rlen;
			abuffer += rlen;
			
			CpuSuspendIntr(&intr_stat);
			dma_id = SifSetDma(&sifdma, 1);
			CpuResumeIntr(intr_stat);

		}
	}

	if(erest > 0)
	{
//		printf("PS2IPS: recv: erest = %d\n", erest);

		rlen = recv(recv_pkt->socket, rests.ebuffer, erest, recv_pkt->flags);

		if(rlen != erest)
		{
			if(rlen > 0)
				total += rlen;
			else total = rlen;

			goto recv_end;
		}

		total += rlen;
	}

recv_end:

	while(SifDmaStat(dma_id) >= 0);

	rests.ssize = srest;
	rests.esize = erest;
	rests.sbuf = recv_pkt->ee_addr;
	rests.ebuf = aebuffer;

	sifdma.src = &rests;
	sifdma.dest = recv_pkt->intr_data;
	sifdma.size = sizeof(rests_pkt);
	sifdma.attr = 0;
	CpuSuspendIntr(&intr_stat);
	SifSetDma(&sifdma, 1);
	CpuResumeIntr(intr_stat);

	ret_pkt->ret = total;
}
*/

void do_recv( void * rpcBuffer, int size )
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


void do_recvfrom( void * rpcBuffer, int size )
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

/*
void do_send( void * rpcBuffer, int size )
{
	int *ptr = rpcBuffer;
	send_pkt *pkt = (send_pkt *)rpcBuffer;
	int left, total = 0, slen, iter_len;
	void *ee_pos;
	struct t_rpc_receive_data rdata;

//	printf("PS2IPS: Send called. size = %d\n", pkt->length);
//	printf("PS2IPS: Send data:\n\n\n");

	left = pkt->length;

	if(pkt->malign) // send misaligned portion
	{
//		printf("PS2IPS: Send: Misaligned EE addr\n");

		slen = send(pkt->socket, pkt->malign_buff, pkt->malign, pkt->flags);

//		printf("send ret = %d\n", slen);
		pkt->malign_buff[slen] = '\0';
//		printf("%s", pkt->malign_buff);

		if(slen != pkt->malign)
		{
			if(slen > 0)
				total += slen;
			else total = slen;

			goto send_end;
		}

		total += slen;
		left -= slen;
	}

	ee_pos = pkt->ee_addr + pkt->malign;

	while(left)
	{
		iter_len = MIN(BUFF_SIZE, left);
		SifRpcGetOtherData(&rdata, ee_pos, lwip_buffer, iter_len, 0);
		slen = send(pkt->socket, lwip_buffer, iter_len, pkt->flags);
		
//		printf("send ret = %d\n", slen);
		lwip_buffer[slen] = '\0';
//		printf("%s", lwip_buffer);

		if(slen != iter_len)
		{
			if(slen > 0)
				total += slen;
			else total = slen;

			goto send_end;
		}

		left -= slen;
		ee_pos += slen;
		total += slen;

	}		

send_end:
	ptr[0] = total;
//	printf("\n\nPS2IPS: Send ret = %d\n", total);
}
*/


void do_send( void * rpcBuffer, int size )
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


void do_sendto( void * rpcBuffer, int size )
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

void do_socket( void * rpcBuffer, int size )
{
	int *ptr = rpcBuffer;
	int ret;

	ret = socket(ptr[0], ptr[1], ptr[2]);

	ptr[0] = ret;
}

void do_getconfig(void *rpcBuffer, int size)
{
    ps2ip_getconfig((char *)rpcBuffer, (t_ip_info *)rpcBuffer);
}

void do_setconfig(void *rpcBuffer, int size)
{
    ps2ip_setconfig((t_ip_info *)rpcBuffer);
}

void do_select( void * rpcBuffer, int size )
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
}

// cmd should be 64 bits wide; I think.
void do_ioctlsocket( void *rpcBuffer, int size )
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

void * rpcHandlerFunction(unsigned int command, void * rpcBuffer, int size)
{

  switch(command)
  {
  case ID_ACCEPT:
		do_accept(rpcBuffer, size);
        break;
  case ID_BIND:
		do_bind(rpcBuffer, size);
        break;
  case ID_DISCONNECT:
		do_disconnect(rpcBuffer, size);
        break;
  case ID_CONNECT:
		do_connect(rpcBuffer, size);
        break;
  case ID_LISTEN:
		do_listen(rpcBuffer, size);
        break;
  case ID_RECV:
		do_recv(rpcBuffer, size);
        break;
  case ID_RECVFROM:
		do_recvfrom(rpcBuffer, size);
        break;
  case ID_SEND:
		do_send(rpcBuffer, size);
        break;
  case ID_SENDTO:
		do_sendto(rpcBuffer, size);
        break;
  case ID_SOCKET:
		do_socket(rpcBuffer, size);
        break;
  case ID_GETCONFIG:
        do_getconfig(rpcBuffer, size);
        break;
  case ID_SETCONFIG:
        do_setconfig(rpcBuffer, size);
        break;
  case ID_SELECT:
		do_select(rpcBuffer, size);
        break;
  case ID_IOCTL:
		do_ioctlsocket(rpcBuffer, size);
		break;
  default:
        printf("PS2IPS: Unknown Function called!\n");

  }
        
  return rpcBuffer;
}


void threadRpcFunction()
{
   int threadId;
  
   printf("PS2IPS: RPC Thread Started\n");

   // Allocate buffer for send/revc
   lwip_buffer = AllocSysMemory(0, BUFF_SIZE + 32, NULL);
   if(!lwip_buffer)
   {
      printf( "PS2IPS: ERROR! Failed to allocate memory!\n");
	  return;
   }
   memset((void *)lwip_buffer,0, BUFF_SIZE + 32);

   SifInitRpc( 0 );
   threadId = GetThreadId();

   SifSetRpcQueue( &ps2ips_queue , threadId );
   SifRegisterRpc( &ps2ips_server, PS2IP_IRX, (void *)rpcHandlerFunction,(u8 *)&_rpc_buffer,0,0, &ps2ips_queue );
   SifRpcLoop( &ps2ips_queue );

}


int _start( int argc, char **argv)
{
   int				threadId;
   iop_thread_t	t;

   printf( "PS2IPS: Module Loaded.\n" );

   t.attr = TH_C;
   t.option = 0;
   t.thread = threadRpcFunction;
   t.stacksize = 0x800;
   t.priority = 0x1e;
 
   threadId = CreateThread( &t );
   if ( threadId < 0 ) 
   {
       printf( "PS2IPS: CreateThread failed.  %i\n", threadId );
   }
   else
   {
       StartThread( threadId, 0 );
   }
  
   return 0;
} 

/*
# _____     ___ ____     ___ ____
#  ____|   |    ____|   |        | |____|
# |     ___|   |____ ___|    ____| |    \    PS2DEV Open Source Project.
#-----------------------------------------------------------------------
# Copyright ps2dev - http://www.ps2dev.org
# Licenced under Academic Free License version 2.0
# Review ps2sdk README & LICENSE files for further details.
*/

#include "irx_imports.h"
#include <s147_mmio_hwport.h>

IRX_ID("S147LINK", 2, 7);
// Text section hash:
// f409cc0329bb98d776c9aacef6573aa9

typedef struct CL_COM_
{
	unsigned int mynode;
	unsigned int maxnode;
	unsigned int nodemask;
	u8 *R_top;
	unsigned int R_in;
	unsigned int R_out;
	unsigned int R_remain;
	u8 R_number[16];
	unsigned int R_lost[16];
	unsigned int R_pd[16];
	unsigned int R_count;
	u8 *T_top;
	unsigned int T_in;
	unsigned int T_out;
	unsigned int T_remain;
	unsigned int T_node;
	int T_error[16];
	unsigned int T_pd[16];
	unsigned int T_time[16];
	u8 T_number;
	unsigned int timeout;
	unsigned int online;
	unsigned int ontimer;
	unsigned int offtimer;
	unsigned int rbfix;
	iop_sys_clock_t sys_clock;
} CL_COM;

static int InitS147link(int maxnode, int mynode, int priority);
static unsigned int alarm_handler(void *userdata);
static void s147link_loop(void *userdata);
static void *dispatch(int fno, void *buf, int size);

static int gbBRE;
static u8 rpc_buf[32784];
static u8 rx_buff[512][64];
static u8 tx_buff[256][64];
static CL_COM cl_info;

int _start(int argc, char **argv)
{
	int maxnode;
	int mynode;
	int priority;

	maxnode = (argc >= 2) ? strtol(argv[1], 0, 10) : 0;
	if ( maxnode < 2 || maxnode >= 16 )
		maxnode = 2;
	mynode = (argc >= 3) ? strtol(argv[2], 0, 10) : 0;
	if ( mynode <= 0 || maxnode < mynode )
		mynode = 1;
	priority = (argc >= 4) ? strtol(argv[3], 0, 10) : 0;
	if ( priority < 9 || priority >= 124 )
		priority = 28;
	gbBRE = (argc >= 5 && toupper(*argv[4]) == 'N' && toupper(argv[4][1]) == 'B' && toupper(argv[4][2]) == 'R') ? 0 : 1;
	printf("== S147LINK (%d/%d)@%d ", mynode, maxnode, priority);
	if ( !gbBRE )
		printf("NBR ");
	printf("v2.07 ==\n");
	if ( InitS147link(maxnode, mynode, priority) )
	{
		printf("S147LINK: Can't Initialize driver thread.\n");
		return MODULE_NO_RESIDENT_END;
	}
	return MODULE_RESIDENT_END;
}

static void T_fix(CL_COM *io_pCommon)
{
	if ( io_pCommon->T_remain >= 0x101 )
	{
		io_pCommon->T_remain = 0x100;
		io_pCommon->T_out = 0;
		io_pCommon->T_in = io_pCommon->T_out;
		io_pCommon->rbfix += 1;
		if ( !io_pCommon->rbfix )
			io_pCommon->rbfix -= 1;
	}
	if ( io_pCommon->T_remain == 0x100 && io_pCommon->T_in != io_pCommon->T_out )
	{
		io_pCommon->T_out = 0;
		io_pCommon->T_in = io_pCommon->T_out;
		io_pCommon->rbfix += 1;
		if ( !io_pCommon->rbfix )
			io_pCommon->rbfix -= 1;
	}
}

static int clink_InterruptHandler(void *userdata)
{
	u8 *bufptr;
	unsigned int i;
	u8 stsH;
	u8 stsL;
	unsigned int rxfs;
	unsigned int rxfc;
	unsigned int tflag;
	int state;
	CL_COM *io_pCommon;
	USE_S147LINK_DEV9_MEM_MMIO();

	io_pCommon = (CL_COM *)userdata;
	stsH = s147link_dev9_mem_mmio->m_stsH_unk12;
	stsL = s147link_dev9_mem_mmio->m_stsL_unk13;
	if ( (stsL & 8) != 0 )
	{
		if ( (s147link_dev9_mem_mmio->m_unk03 & 8) != 0 )
		{
			s147link_dev9_mem_mmio->m_unk17 = 1;
			s147link_dev9_mem_mmio->m_unk17 = 0xE;
		}
		if ( (s147link_dev9_mem_mmio->m_unk01 & 4) != 0 )
		{
			s147link_dev9_mem_mmio->m_unk17 = 0x16;
			if ( !io_pCommon->ontimer )
				io_pCommon->offtimer = 1;
			io_pCommon->ontimer = 1;
		}
	}
	rxfc = 0;
	rxfs =
		((s147link_dev9_mem_mmio->m_rxfc_hi_unk1E << 8) | s147link_dev9_mem_mmio->m_rxfc_lo_unk1F) & io_pCommon->nodemask;
	if ( rxfs )
	{
		for ( i = 1; io_pCommon->maxnode >= i; i += 1 )
		{
			if ( i != io_pCommon->mynode )
			{
				u8 unk09_tmp;

				if ( (rxfs & (1 << i)) == 0 )
				{
					continue;
				}
				s147link_dev9_mem_mmio->m_node_unk05 = i | 0xC0;
				s147link_dev9_mem_mmio->m_unk07 = 0;
				unk09_tmp = s147link_dev9_mem_mmio->m_unk09;
				if ( unk09_tmp == io_pCommon->mynode )
				{
					// cppcheck-suppress incorrectLogicOperator
					if ( s147link_dev9_mem_mmio->m_unk09 == 4 && !s147link_dev9_mem_mmio->m_unk09 )
					{
						u8 rnum;

						rnum = s147link_dev9_mem_mmio->m_unk09;
						if ( io_pCommon->R_number[i] != rnum )
						{
							unk09_tmp = s147link_dev9_mem_mmio->m_unk09;
							if ( io_pCommon->R_remain )
							{
								unsigned int j;

								bufptr = &io_pCommon->R_top[0x40 * io_pCommon->R_in];
								bufptr[0] = i;
								bufptr[1] = io_pCommon->mynode;
								bufptr[2] = 4;
								bufptr[3] = 0;
								bufptr[4] = rnum;
								bufptr[5] = unk09_tmp;
								for ( j = 0; j < 0x3A; j += 1 )
									bufptr[j + 6] = s147link_dev9_mem_mmio->m_unk09;
								io_pCommon->R_remain -= 1;
								io_pCommon->R_in += 1;
								io_pCommon->R_in &= 0x1FF;
								io_pCommon->R_number[i] = rnum;
							}
							else if ( io_pCommon->R_pd[i] )
							{
								io_pCommon->R_lost[i] += 1;
								if ( !io_pCommon->R_lost[i] )
									io_pCommon->R_lost[i] += 1;
							}
							else
							{
								continue;
							}
						}
					}
				}
				else if ( !unk09_tmp )
				{
					if ( s147link_dev9_mem_mmio->m_unk09 == 0x38 )
					{
						s147link_dev9_mem_mmio->m_node_unk05 = i | 0xC0;
						s147link_dev9_mem_mmio->m_unk07 = 0x38;
						unk09_tmp = s147link_dev9_mem_mmio->m_unk09;
						if ( (unk09_tmp & 0xE0) == 0x20 || (unk09_tmp & 0xE0) == 0x60 )
						{
							if ( io_pCommon->R_remain )
							{
								bufptr = &io_pCommon->R_top[0x40 * io_pCommon->R_in];
								bufptr[0] = i;
								bufptr[1] = 0;
								bufptr[2] = 56;
								bufptr[56] = unk09_tmp;
								bufptr[57] = s147link_dev9_mem_mmio->m_unk09;
								bufptr[58] = s147link_dev9_mem_mmio->m_unk09;
								bufptr[59] = s147link_dev9_mem_mmio->m_unk09;
								bufptr[60] = s147link_dev9_mem_mmio->m_unk09;
								bufptr[61] = s147link_dev9_mem_mmio->m_unk09;
								bufptr[62] = s147link_dev9_mem_mmio->m_unk09;
								bufptr[63] = s147link_dev9_mem_mmio->m_unk09;
								io_pCommon->R_remain -= 1;
								io_pCommon->R_in += 1;
								io_pCommon->R_in &= 0x1FF;
							}
							else if ( io_pCommon->R_pd[i] )
							{
								io_pCommon->R_lost[i] += 1;
								if ( !io_pCommon->R_lost[i] )
									io_pCommon->R_lost[i] += 1;
							}
							else
							{
								continue;
							}
						}
					}
				}
			}
			rxfc |= 1 << i;
		}
	}
	tflag = 0;
	if ( (stsL & 0x10) != 0 )
	{
		s147link_dev9_mem_mmio->m_unk17 = 1;
		s147link_dev9_mem_mmio->m_unk17 = 0xE;
	}
	if ( (stsL & 2) != 0 && (stsL & 4) == 0 && io_pCommon->timeout )
	{
		tflag = 1;
	}
	else if ( (stsL & 1) != 0 )
	{
		io_pCommon->timeout = 0;
		while ( 1 )
		{
			if ( io_pCommon->T_remain == 0x100 )
			{
				s147link_dev9_mem_mmio->m_unk15 = 0x1A;
				break;
			}
			bufptr = &io_pCommon->T_top[0x40 * io_pCommon->T_out];
			if ( *bufptr == io_pCommon->mynode )
			{
				io_pCommon->T_node = bufptr[1];
				if ( !io_pCommon->T_error[io_pCommon->T_node] || !io_pCommon->T_pd[io_pCommon->T_node] )
				{
					s147link_dev9_mem_mmio->m_node_unk05 = (io_pCommon->mynode & 0xFF) | 0x40;
					s147link_dev9_mem_mmio->m_unk07 = 0;
					for ( i = 0; i < 0x40; i += 1 )
						s147link_dev9_mem_mmio->m_unk09 = bufptr[i];
					io_pCommon->T_remain += 1;
					io_pCommon->T_out += 1;
					io_pCommon->T_out &= 0xFF;
					T_fix(io_pCommon);
					s147link_dev9_mem_mmio->m_unk15 = 0x1B;
					tflag = 1;
					io_pCommon->timeout = 1;
					break;
				}
			}
			io_pCommon->T_remain += 1;
			io_pCommon->T_out += 1;
			io_pCommon->T_out &= 0xFF;
			T_fix(io_pCommon);
		}
	}
	CpuSuspendIntr(&state);
	if ( tflag )
		s147link_dev9_mem_mmio->m_unk17 = 3;
	if ( rxfc )
	{
		s147link_dev9_mem_mmio->m_rxfc_hi_unk1E = (rxfc >> 8) & 0xFF;
		s147link_dev9_mem_mmio->m_rxfc_lo_unk1F = rxfc;
	}
	s147link_dev9_mem_mmio->m_stsH_unk12 = stsH;
	s147link_dev9_mem_mmio->m_stsL_unk13 = stsL;
	CpuResumeIntr(state);
	return 1;
}

static int cl_mread(void *dstptr, int count)
{
	int state;
	int size;
	int packs;

	CpuSuspendIntr(&state);
	size = 0x200 - cl_info.R_remain;
	if ( cl_info.R_remain == 0x200 )
	{
		CpuResumeIntr(state);
		return 0;
	}
	if ( count >= size )
		count = size;
	else
		size = count;
	packs = cl_info.R_out + size - 0x200;
	if ( packs > 0 )
	{
		size -= packs;
		size <<= 6;
		memcpy(dstptr, rx_buff[cl_info.R_out], size);
		memcpy((char *)dstptr + size, rx_buff, packs << 6);
	}
	else
	{
		memcpy(dstptr, rx_buff[cl_info.R_out], size << 6);
	}
	cl_info.R_remain += count;
	cl_info.R_out += count;
	cl_info.R_out &= 0x1FF;
	if ( cl_info.R_remain >= 0x201 )
	{
		cl_info.R_remain = 0x200;
		cl_info.R_out = 0;
		cl_info.R_in = 0;
		cl_info.rbfix += 1;
		if ( !cl_info.rbfix )
			cl_info.rbfix -= 1;
		count = 0;
	}
	if ( cl_info.R_remain == 512 && cl_info.R_in != cl_info.R_out )
	{
		cl_info.R_out = 0;
		cl_info.R_in = 0;
		cl_info.rbfix += 1;
		if ( !cl_info.rbfix )
			cl_info.rbfix -= 1;
		count = 0;
	}
	CpuResumeIntr(state);
	return count;
}

static int cl_write(int node, u8 *srcptr, int size)
{
	int state;
	USE_S147LINK_DEV9_MEM_MMIO();

	CpuSuspendIntr(&state);
	if ( !cl_info.T_remain || size >= 0x41 )
	{
		CpuResumeIntr(state);
		return 0;
	}
	memcpy(tx_buff[cl_info.T_in], srcptr, size);
	tx_buff[cl_info.T_in][0] = cl_info.mynode;
	tx_buff[cl_info.T_in][1] = node & 0xFF;
	tx_buff[cl_info.T_in][2] = 4;
	tx_buff[cl_info.T_in][3] = 0;
	cl_info.T_number += 1;
	tx_buff[cl_info.T_in][4] = cl_info.T_number;
	cl_info.T_remain -= 1;
	cl_info.T_in += 1;
	cl_info.T_in &= 0xFF;
	s147link_dev9_mem_mmio->m_unk15 = 0x1B;
	clink_InterruptHandler(&cl_info);
	CpuResumeIntr(state);
	return size;
}

static int cl_write_custom(int node, u8 *srcptr, int cpVal)
{
	int state;
	USE_S147LINK_DEV9_MEM_MMIO();

	CpuSuspendIntr(&state);
	if ( !cl_info.T_remain )
	{
		CpuResumeIntr(state);
		return 0;
	}
	memcpy(tx_buff[cl_info.T_in], srcptr, sizeof(u8[64]));
	tx_buff[cl_info.T_in][0] = cl_info.mynode;
	tx_buff[cl_info.T_in][1] = node & 0xFF;
	tx_buff[cl_info.T_in][2] = cpVal & 0xFF;
	cl_info.T_remain -= 1;
	cl_info.T_in += 1;
	cl_info.T_in &= 0xFF;
	s147link_dev9_mem_mmio->m_unk15 = 0x1B;
	clink_InterruptHandler(&cl_info);
	CpuResumeIntr(state);
	return 64;
}

static int cl_mwrite(u8 *srcptr, int count)
{
	int state;
	int i;
	int packs;
	USE_S147LINK_DEV9_MEM_MMIO();

	if ( cl_info.T_remain < (unsigned int)count )
		return 0;
	if ( count >= 0x101 )
		return 0;
	for ( i = 0; i < count; i += 1 )
	{
		srcptr[(i * 0x40)] = cl_info.mynode;
		srcptr[(i * 0x40) + 2] = 4;
		srcptr[(i * 0x40) + 3] = 0;
		srcptr[(i * 0x40) + 4] = cl_info.T_number + i + 1;
	}
	cl_info.T_number += i;
	CpuSuspendIntr(&state);
	if ( cl_info.T_remain < (unsigned int)count )
	{
		CpuResumeIntr(state);
		return 0;
	}
	packs = cl_info.T_in + count - 0x100;
	if ( packs > 0 )
	{
		memcpy(tx_buff[cl_info.T_in], srcptr, (count - packs) << 6);
		memcpy(tx_buff, &srcptr[0x40 * (count - packs)], packs << 6);
	}
	else
	{
		memcpy(tx_buff[cl_info.T_in], srcptr, count << 6);
	}
	cl_info.T_remain -= count;
	cl_info.T_in += count;
	cl_info.T_in &= 0xFF;
	s147link_dev9_mem_mmio->m_unk15 = 0x1B;
	clink_InterruptHandler(&cl_info);
	CpuResumeIntr(state);
	return count;
}

static int InitS147link(int maxnode, int mynode, int priority)
{
	iop_thread_t param;
	int thid;
	int i;
	int j;
	int state;
	u8 stsH;
	u8 stsL;
	USE_S147LINK_DEV9_MEM_MMIO();

	s147link_dev9_mem_mmio->m_unk0D |= 0x80;
	s147link_dev9_mem_mmio->m_unk22 = 2;
	s147link_dev9_mem_mmio->m_unk23 = gbBRE ? 0x51 : 0x11;
	s147link_dev9_mem_mmio->m_maxnode_unk2B = maxnode;
	s147link_dev9_mem_mmio->m_mynode_unk2D = mynode;
	s147link_dev9_mem_mmio->m_unk31 = 0;
	s147link_dev9_mem_mmio->m_unk2F = 2;
	for ( i = 0; i < 4; i += 1 )
	{
		s147link_dev9_mem_mmio->m_node_unk05 = i | 0x40;
		s147link_dev9_mem_mmio->m_unk07 = 0;
		for ( j = 0; j < 256; j += 1 )
			s147link_dev9_mem_mmio->m_unk09 = 0;
	}
	s147link_dev9_mem_mmio->m_unk28 = 0;
	s147link_dev9_mem_mmio->m_unk29 = 0;
	s147link_dev9_mem_mmio->m_unk21 = 0;
	s147link_dev9_mem_mmio->m_unk24 = 0;
	s147link_dev9_mem_mmio->m_unk25 = 0xFF;
	s147link_dev9_mem_mmio->m_unk0D &= 0x7F;
	s147link_dev9_mem_mmio->m_unk22 |= 1;
	s147link_dev9_mem_mmio->m_node_unk05 = mynode | 0x40;
	s147link_dev9_mem_mmio->m_unk07 = 0;
	s147link_dev9_mem_mmio->m_unk09 = mynode;
	s147link_dev9_mem_mmio->m_unk09 = 2;
	s147link_dev9_mem_mmio->m_unk09 = 4;
	cl_info.mynode = mynode;
	cl_info.maxnode = maxnode;
	j = 1;
	for ( i = 0; i < maxnode; i += 1 )
	{
		j = (j << 1) | 1;
	}
	cl_info.nodemask = j;
	cl_info.R_top = rx_buff[0];
	cl_info.R_remain = 0x200;
	cl_info.R_out = 0;
	cl_info.R_in = 0;
	for ( i = 0; i < 16; i += 1 )
	{
		cl_info.R_number[i] = 0;
		cl_info.T_error[i] = 0;
		cl_info.R_lost[i] = 0;
		cl_info.T_pd[i] = 1;
		cl_info.R_pd[i] = 1;
		cl_info.T_time[i] = 2;
	}
	cl_info.T_top = tx_buff[0];
	cl_info.T_remain = 0x100;
	cl_info.offtimer = 0;
	cl_info.ontimer = 0;
	cl_info.online = 0;
	cl_info.timeout = 0;
	cl_info.T_out = 0;
	cl_info.T_in = 0;
	cl_info.T_number = 0;
	cl_info.T_node = 2;
	cl_info.rbfix = 0;
	CpuSuspendIntr(&state);
	ReleaseIntrHandler(13);
	RegisterIntrHandler(13, 1, clink_InterruptHandler, &cl_info);
	s147link_dev9_mem_mmio->m_unk01 = 0xC;
	s147link_dev9_mem_mmio->m_unk14 = 0x8E;
	s147link_dev9_mem_mmio->m_unk15 = 0x1A;
	s147link_dev9_mem_mmio->m_unk1C = 0xFF;
	s147link_dev9_mem_mmio->m_unk1D = 0xFF;
	s147link_dev9_mem_mmio->m_rxfc_hi_unk1E = 0xFF;
	s147link_dev9_mem_mmio->m_rxfc_lo_unk1F = 0xFF;
	stsH = s147link_dev9_mem_mmio->m_stsH_unk12;
	stsL = s147link_dev9_mem_mmio->m_stsL_unk13;
	s147link_dev9_mem_mmio->m_stsH_unk12 = stsH;
	s147link_dev9_mem_mmio->m_stsL_unk13 = stsL;
	CpuResumeIntr(state);
	EnableIntr(13);
	sceSifInitRpc(0);
	param.attr = TH_C;
	param.thread = s147link_loop;
	param.priority = priority;
	param.stacksize = 0x800;
	param.option = 0;
	thid = CreateThread(&param);
	if ( thid <= 0 )
	{
		printf("S147LINK: Cannot create RPC server thread ...\n");
		return -1;
	}
	if ( StartThread(thid, 0) )
	{
		printf("S147LINK: Cannot start RPC server thread ...\n");
		DeleteThread(thid);
		return -2;
	}
	USec2SysClock(0x7D0, &cl_info.sys_clock);
	if ( SetAlarm(&cl_info.sys_clock, alarm_handler, &cl_info) )
	{
		printf("S147LINK: Cannot set alarm handler ...\n");
		DeleteThread(thid);
		return -3;
	}
	return 0;
}

#ifdef UNUSED_FUNC
static void reset_circlink(void)
{
	u8 stsH;
	u8 stsL;
	int i;
	int j;
	USE_S147LINK_DEV9_MEM_MMIO();

	s147link_dev9_mem_mmio->m_unk0D |= 0x80;
	s147link_dev9_mem_mmio->m_unk22 = 2;
	s147link_dev9_mem_mmio->m_unk23 = gbBRE ? 0x51 : 0x11;
	s147link_dev9_mem_mmio->m_maxnode_unk2B = cl_info.maxnode;
	s147link_dev9_mem_mmio->m_mynode_unk2D = cl_info.mynode;
	s147link_dev9_mem_mmio->m_unk31 = 0;
	s147link_dev9_mem_mmio->m_unk2F = 2;
	for ( i = 0; i < 4; i += 1 )
	{
		s147link_dev9_mem_mmio->m_node_unk05 = i | 0x40;
		s147link_dev9_mem_mmio->m_unk07 = 0;
		for ( j = 0; j < 256; j += 1 )
			s147link_dev9_mem_mmio->m_unk09 = 0;
	}
	s147link_dev9_mem_mmio->m_unk28 = 0;
	s147link_dev9_mem_mmio->m_unk29 = 0;
	s147link_dev9_mem_mmio->m_unk21 = 0;
	s147link_dev9_mem_mmio->m_unk24 = 0;
	s147link_dev9_mem_mmio->m_unk25 = 0xFF;
	s147link_dev9_mem_mmio->m_unk0D &= 0x7F;
	s147link_dev9_mem_mmio->m_unk22 |= 1;
	s147link_dev9_mem_mmio->m_node_unk05 = (cl_info.mynode & 0xFF) | 0x40;
	s147link_dev9_mem_mmio->m_unk07 = 0;
	s147link_dev9_mem_mmio->m_unk09 = cl_info.mynode;
	s147link_dev9_mem_mmio->m_unk09 = 2;
	s147link_dev9_mem_mmio->m_unk09 = 4;
	s147link_dev9_mem_mmio->m_unk01 = 0xC;
	s147link_dev9_mem_mmio->m_unk14 = 0x8E;
	s147link_dev9_mem_mmio->m_unk15 = 0x1A;
	s147link_dev9_mem_mmio->m_unk1C = 0xFF;
	s147link_dev9_mem_mmio->m_unk1D = 0xFF;
	s147link_dev9_mem_mmio->m_rxfc_hi_unk1E = 0xFF;
	s147link_dev9_mem_mmio->m_rxfc_lo_unk1F = 0xFF;
	stsH = s147link_dev9_mem_mmio->m_stsH_unk12;
	stsL = s147link_dev9_mem_mmio->m_stsL_unk13;
	s147link_dev9_mem_mmio->m_stsH_unk12 = stsH;
	s147link_dev9_mem_mmio->m_stsL_unk13 = stsL;
}
#endif

static unsigned int alarm_handler(void *userdata)
{
	int state;
	CL_COM *io_pCommon;
	USE_S147LINK_DEV9_MEM_MMIO();

	io_pCommon = (CL_COM *)userdata;
	if ( io_pCommon->timeout )
	{
		if ( io_pCommon->T_time[io_pCommon->T_node] )
		{
			if ( io_pCommon->T_time[io_pCommon->T_node] >= io_pCommon->timeout )
			{
				io_pCommon->timeout += 1;
			}
			else
			{
				CpuSuspendIntr(&state);
				(void)s147link_dev9_mem_mmio->m_stsL_unk13;
				io_pCommon->timeout = 0;
				s147link_dev9_mem_mmio->m_unk17 = 1;
				s147link_dev9_mem_mmio->m_unk17 = 0xE;
				cl_info.T_error[cl_info.T_node] = 0xFFFFFFFE;
				s147link_dev9_mem_mmio->m_unk15 = 0x1B;
				clink_InterruptHandler(&cl_info);
				CpuResumeIntr(state);
			}
		}
		else
		{
			io_pCommon->timeout = 0;
		}
	}
	if ( io_pCommon->ontimer )
	{
		if ( io_pCommon->ontimer < 0xFA )
		{
			io_pCommon->ontimer += 1;
		}
		else
		{
			io_pCommon->online = 1;
			io_pCommon->offtimer = 0;
			io_pCommon->ontimer = io_pCommon->offtimer;
		}
	}
	if ( io_pCommon->offtimer )
	{
		if ( io_pCommon->offtimer < 0xFA )
			io_pCommon->offtimer += 1;
		else
			io_pCommon->online = 0;
	}
	return io_pCommon->sys_clock.lo;
}

static void s147link_loop(void *userdata)
{
	SifRpcDataQueue_t qd;
	SifRpcServerData_t sd;

	(void)userdata;
	sceSifSetRpcQueue(&qd, GetThreadId());
	sceSifRegisterRpc(&sd, 0x14799, dispatch, rpc_buf, 0, 0, &qd);
	sceSifRpcLoop(&qd);
}

static void *dispatch(int fno, void *buf, int size)
{
	int state;
	int node;
	int sizeb;
	unsigned int i;
	USE_S147LINK_DEV9_MEM_MMIO();

	(void)size;
	FlushDcache();
	node = fno & 0xFF;
	sizeb = (fno & 0xFFFF00) >> 8;
	switch ( fno >> 24 )
	{
		case 0x00:
			*(u32 *)buf = 1;
			break;
		case 0x01:
			cl_info.R_pd[node] = *(u32 *)buf;
			cl_info.T_pd[node] = *((u32 *)buf + 1);
			cl_info.T_time[node] = *((u32 *)buf + 2);
			break;
		case 0x10:
			CpuSuspendIntr(&state);
			cl_info.T_out = 0;
			cl_info.T_in = 0;
			cl_info.T_remain = 0x100;
			s147link_dev9_mem_mmio->m_unk17 = 1;
			s147link_dev9_mem_mmio->m_unk17 = 0xE;
			cl_info.T_error[cl_info.T_node] = 0xFFFFFFFD;
			*(u32 *)buf = cl_info.T_node;
			CpuResumeIntr(state);
			break;
		case 0x11:
			*(u32 *)buf = cl_info.rbfix;
			break;
		case 0x20:
			*(u32 *)buf = cl_mread((char *)buf + 4, *(u32 *)buf);
			CpuSuspendIntr(&state);
			clink_InterruptHandler(&cl_info);
			CpuResumeIntr(state);
			break;
		case 0x30:
			*(u32 *)buf = cl_write(node, (u8 *)buf, sizeb);
			break;
		case 0x40:
			*(u32 *)buf = cl_mwrite((u8 *)buf, sizeb);
			break;
		case 0x50:
			CpuSuspendIntr(&state);
			cl_info.R_out = 0;
			cl_info.R_in = 0;
			cl_info.R_remain = 0x200;
			CpuResumeIntr(state);
			break;
		case 0x60:
			*(u32 *)buf = cl_info.R_remain;
			break;
		case 0x70:
			*(u32 *)buf = cl_info.T_remain;
			break;
		case 0x80:
			*(u32 *)buf = cl_info.online;
			break;
		case 0x90:
			*(u32 *)buf = cl_info.T_error[node];
			break;
		case 0xA0:
			CpuSuspendIntr(&state);
			cl_info.T_error[node] = 0;
			s147link_dev9_mem_mmio->m_unk15 = 0x1B;
			clink_InterruptHandler(&cl_info);
			CpuResumeIntr(state);
			break;
		case 0xB0:
			CpuSuspendIntr(&state);
			s147link_dev9_mem_mmio->m_unk17 = 1;
			s147link_dev9_mem_mmio->m_unk17 = 0xE;
			cl_info.T_error[cl_info.T_node] = 0xFFFFFFFD;
			*(u32 *)buf = cl_info.T_node;
			s147link_dev9_mem_mmio->m_unk15 = 0x1B;
			clink_InterruptHandler(&cl_info);
			CpuResumeIntr(state);
			break;
		case 0xC0:
			*(u32 *)buf = cl_info.R_lost[node];
			cl_info.R_lost[node] = 0;
			break;
		case 0xD0:
			*(u32 *)buf = cl_write_custom(node, (u8 *)buf, sizeb);
			break;
		case 0xF0:
			CpuSuspendIntr(&state);
			clink_InterruptHandler(&cl_info);
			CpuResumeIntr(state);
			break;
		default:
			printf("S147LINK: Unknown RPC command (%X)\n", fno);
			break;
	}
	*(u32 *)buf |= (cl_info.online ? 0x10000 : 0);
	for ( i = 1; i < cl_info.maxnode; i += 1 )
	{
		if ( cl_info.T_error[i] )
			*(u32 *)buf |= 0x10000 << i;
		if ( cl_info.R_lost[i] )
			*(u32 *)buf |= 0x10000 << i;
	}
	FlushDcache();
	return buf;
}

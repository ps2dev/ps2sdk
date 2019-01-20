/*
 * Copyright (c) 2007 Lukasz Bruun <mail@lukasz.dk>
 *
 * See the file LICENSE included with this distribution for licensing terms.
 */

/**
 * @file
 * IOP multitap driver
 */

#include "types.h"
#include "irx.h"
#include "stdio.h"
#include "loadcore.h"
#include "thevent.h"
#include "thbase.h"
#include "xsio2man.h"
#include "freemtap.h"

extern struct irx_export_table _exp_mtapman;

#define BANNER "FREEMTAP %s\n"
#define VERSION "v1.0"

#define EF_UPDATE_SLOTS		0x1
#define EF_EXIT_THREAD		0x2

IRX_ID("multitap_manager", 2, 2);

static s32 event_flag;
static s32 threadid_main;

static u32 state_open[4];
static u32 state_getcon[4];
static u32 state_slots[4];
static u8 in_buffer[256];
static u8 out_buffer[256];
static sio2_transfer_data_t td;

s32 read_stat6c_bit(u32 bit, sio2_transfer_data_t *tdata)
{
	switch(bit)
	{
		case 0:		return (tdata->stat6c >> 16) & 1;
		case 1:		return (tdata->stat6c >> 17) & 1;
		case 2:		return (tdata->stat6c >> 18) & 1;
		case 3:		return (tdata->stat6c >> 19) & 1;
		case 4:		return (tdata->stat6c >> 20) & 1;
		case 5:		return (tdata->stat6c >> 21) & 1;
		case 6:		return (tdata->stat6c >> 22) & 1;
		case 7:		return (tdata->stat6c >> 23) & 1;
		case 8:		return (tdata->stat6c >> 24) & 1;
		case 9:		return (tdata->stat6c >> 25) & 1;
		case 10:	return (tdata->stat6c >> 26) & 1;
		case 11:	return (tdata->stat6c >> 27) & 1;
		case 12:	return (tdata->stat6c >> 28) & 1;
		case 13:	return (tdata->stat6c >> 29) & 1;
		case 14:	return (tdata->stat6c >> 30) & 1;
		case 15:	return (tdata->stat6c >> 31) & 1;
		default: 	return 0;
	}
}

void get_slot_number_setup_td(u32 port, u32 reg)
{
	u32 p = port | 0x2;
	u32 i;

	td.port_ctrl1[p] = 0xFF020505;
	td.port_ctrl2[p] = 0x00030064;

	td.regdata[reg] = (p & 0x3) | 0x180640;

	for(i=0; i < 6 ; i++) td.in[td.in_size + i] = 0;

	td.in[td.in_size] = 0x21;

	if(port < 2)
		td.in[td.in_size + 1] = 0x12;
	else
		td.in[td.in_size + 1] = 0x13;

	td.in_size += 6;
	td.out_size += 6;
	td.in_dma.addr = 0;
	td.out_dma.addr = 0;
}

s32 get_slot_number_check_td(u32 bit)
{
	s32 res;
	u32 i;

	if(read_stat6c_bit(bit, &td) != 1)
	{
		if(td.out[5] == 0x66)
			res = -2;
		else
			res = td.out[3];
	}
	else
	{
		res = -1;
	}

	for(i=0; i < 250; i++) td.out[i] = td.out[i+6];

	td.out_size -= 6;

	return res;
}
s32 get_slot_number(u32 port, u32 retries)
{
	u32 i,j;
	s32 slots = -1;

	if(port >= 4) return -3;

	if(state_open[port] == 0) return -4;

	i = 0;

	while((slots < 0) && (i <= retries))
	{
		sio2_mtap_transfer_init();

		td.in_size = 0;
		td.out_size = 0;

		for(j=0; j < 16; j++) td.regdata[j] = 0;

		get_slot_number_setup_td(port, 0);
		sio2_transfer2(&td);
		slots = get_slot_number_check_td(0);

		sio2_transfer_reset2();

		if((slots == -2) || (slots >= 0)) return slots;

		i++;
		//M_PRINTF("get_slot_number retry %i.\n", (int)i);
	}

	return -4;
}


void update_slot_numbers_thread()
{
	while(1)
	{
		u32 resbits, port;
		s32 slots;

		WaitEventFlag(event_flag, EF_UPDATE_SLOTS | EF_EXIT_THREAD, 0x11, &resbits);

		if(resbits & EF_EXIT_THREAD)
		{
			SetEventFlag(event_flag, 0x4);
			ExitThread();
		}

		for(port=0; port < 4; port++)
		{
			if(state_open[port] == 1)
			{
				if(state_getcon[port] == 0)
					slots = get_slot_number(port, 10);
				else
					slots = get_slot_number(port, 0);

				if(slots < 0)
				{
					state_slots[port] = 1;
					state_getcon[port] = 0;
				}
				else
				{
					state_slots[port] = slots;
					state_getcon[port] = 1;
				}
			}
		}


	}
}

s32 change_slot_setup_td(u32 port, s32 slot, u32 reg)
{
	u32 p = port | 0x2;
	u32 i;

	td.port_ctrl1[p] = 0xFF020505;
	td.port_ctrl2[p] = 0x00030064;

	td.regdata[reg] = (p & 0x3) | 0x1c0740;

	for(i=0; i < 7; i++) td.in[td.in_size + i] = 0;

	td.in[td.in_size] = 0x21;

	if(port < 2)
	{
		td.in[td.in_size + 1] = 0x21;
	}
	else
	{
		td.in[td.in_size + 1] = 0x22;
	}

	td.in[td.in_size + 2] = (u8)slot;

	td.in_size += 7;
	td.out_size += 7;
	td.in_dma.addr = 0;
	td.out_dma.addr = 0;


	return 0;
}

s32 change_slot_check_td(u32 a)
{
	u32 i;
	s32 res;

	if(td.out_size >= 7)
	{
		if(read_stat6c_bit(a, &td) != 1)
		{
			if(td.out[5] == 0x66)
				res = -2;
			else
				res = td.out[5];

		}
		else
		{
			res = -1;
		}
	}
	else
	{
		return -99;
	}


	for(i=0; i < 249; i++)
	{
		td.out[i] = td.out[i+7];
	}

	td.out_size -= 7;


	return res;
}

int change_slot(s32 *arg)
{
	u32 loop = 0;
	u32 count = 4;
	u32 reg = 0;
	s32 data[4];
	u32 i, port;

	while((loop < 10) && (count != 0))
	{
		count = 4;
		reg = 0;

		td.in_size = 0;
		td.out_size = 0;

		for(i=0; i < 16; i++) td.regdata[i] = 0;

		// Setup transfer data
		for(port=0; port < 4; port++)
		{
			data[port] = -1;

			if(arg[port] >= 0)
			{
				if((state_open[port] == 1) && (state_getcon[port] == 1))
				{
					if(state_slots[port] > arg[port])
					{
						change_slot_setup_td(port, arg[port], reg);

						data[port] = port;
						reg++;
					}
					else
					{
						arg[port+4] = -1;
					}

				}
				else
				{
					if(arg[port] == 0)
						arg[port+4] = 1;
					else
						arg[port+4] = -1;
				}
			}
			else
			{
				arg[port+4] = 0;
			}

		}

		// Send transfer data and check result
		if(reg > 0)
		{
			sio2_transfer2(&td);

			for(port=0; port < 4; port++)
			{
				if(data[port] >= 0)
				{
					s32 res = change_slot_check_td(data[port]);

					if(res == -2)
					{
						arg[port+4] = -1;
						count--;
					}
					else
					{
						if(res >= 0)
						{
							if(res == arg[port])
								arg[port+4] = 1;
							else
								arg[port+4] = -1;

							count--;
						}
					}
				}
				else
				{
					count--;
				}
			}
		}

		if(count != 0) loop++;
	}

	for(i=0; i < 4; i++)
	{
		if(arg[i+4] < 0)
			return 0;
	}

	return 1;

}

int get_slots1(int port)
{
	return state_slots[port];
}

int get_slots2(int port)
{
	return state_slots[port];
}

void update_slot_numbers()
{
	SetEventFlag(event_flag, EF_UPDATE_SLOTS);
}

s32 _start(char **argv, int argc)
{
	iop_event_t event;
	iop_thread_t thread;
	u32 i;

	printf(BANNER,VERSION);

	if(RegisterLibraryEntries(&_exp_mtapman) != 0)
	{
		M_PRINTF("RegisterLibraryEntries failed.\n");
		return 1;
	}

	if(InitRpcServers() == 0)
	{
		M_PRINTF("Failed to setup RPC Servers.\n");
		return 1;
	}

	event.attr = 2;
	event.bits = 0;

	event_flag = CreateEventFlag(&event);

	if(event_flag < 0)
	{
		M_PRINTF("Could not create event flag (%i)\n.", (int)event_flag);
		return 1;
	}

	thread.attr = TH_C;
	thread.thread = update_slot_numbers_thread;
	thread.stacksize = 0x800;
	thread.priority = 32;

	threadid_main = CreateThread(&thread);

	if(threadid_main < 0)
	{
		M_PRINTF("Could not create thread (%i)\n.", (int)threadid_main);
		return 1;
	}

	StartThread(threadid_main, 0);

	for(i=0; i < 4; i++)
	{
		state_open[i] = 0;
		state_getcon[i] = 0;
		state_slots[i] = 0;
	}

	sio2_mtap_change_slot_set(change_slot);
	sio2_mtap_get_slot_max_set(get_slots1);
	sio2_mtap_get_slot_max2_set(get_slots2);
	sio2_mtap_update_slots_set(update_slot_numbers);

	td.in = in_buffer;
	td.out = out_buffer;

	return 0;
}


void shutdown()
{
	u32 i;

	sio2_mtap_change_slot_set(NULL);
	sio2_mtap_get_slot_max_set(NULL);
	sio2_mtap_get_slot_max2_set(NULL);
	sio2_mtap_update_slots_set(NULL);

	for(i=0; i < 4; i++)
	{
		state_open[i] = 0;
		state_getcon[i] = 0;
		state_slots[i] = 0;
	}
}

s32 mtapPortOpen(u32 port)
{
	if(port < 4)
	{
		state_open[port] = 1;

		s32 res = get_slot_number(port, 10);

		if(res < 0)
		{
			state_getcon[port] = 0;
			state_slots[port] = 1;
			return res;
		}
		else
		{
			state_getcon[port] = 1;
			state_slots[port] = res;
			return 1;
		}
	}

	return 0;
}

s32 mtapPortClose(u32 port)
{
	state_open[port] = 0;
	state_getcon[port] = 0;

	return 1;
}

s32 mtapGetConnection(u32 port)
{
	return state_getcon[port];
}

s32 mtapGetSlotNumber(u32 port)
{
	s32 res;

	if(port > 4) return -1;

	if(state_open[port] == 0) return 1;

	res = get_slot_number(port, 10);

	if(res >= 0)
		return res;
	else
		return 1;
}

s32 mtapChangeSlot(u32 port, u32 slot)
{
	s32 data[4];
	u32 i;

	if(port > 4) return 0;

	if(state_open[port] == 0)
	{
		M_PRINTF("mtap manager doesn't work\n");
		return -1;
	}

	for(i=0; i < 4; i++)
		data[i] = -1;

	data[port] = slot;

	sio2_mtap_transfer_init();

	change_slot(data);

	sio2_transfer_reset2();

	if(data[port] < 0)
	{
		M_PRINTF("Failed to change slot.\n");
		return data[port];
	}
	else
	{
		M_PRINTF("Change slot complete.\n");
		return 1;
	}
}




/*
 * Copyright (c) 2007 Lukasz Bruun <mail@lukasz.dk>
 *
 * See the file LICENSE included with this distribution for licensing terms.
 */

/**
 * @file
 * IOP pad driver
 */

#include "irx.h"
#include "types.h"
#include "sio2man.h"
#include "xsio2man.h"
#include "sifman.h"
#include "sio2Cmds.h"
#include "padData.h"
#include "stdio.h"
#include "freepad.h"

typedef struct
{
	u32 active;
	u32 unused_4;
	u32 unused_8;
	u32 stat70bit;
	u32 in_size;
	u32 out_size;
	u8 in_buffer[32];
	u8 out_buffer[32];
	u32 port_ctrl1;
	u32 port_ctrl2;
	u32 reg_data;
	u32 error;
} padData_t;

static padData_t padData[2][4];

static sio2_transfer_data_t sio2_td;
static u8 sio2_in_buffer[256];
static u8 sio2_out_buffer[256];
static s32 change_slot_buffer[8];

static int transferCount;

u32 pdGetInSize(u8 id)
{
	if(id == 0) id = PAD_ID_DIGITAL;

	return (PAD_ID_LO(id)*2)+3;
}

u32 pdGetOutSize(u8 id)
{
	if(id == 0) id = PAD_ID_DIGITAL;

	return (PAD_ID_LO(id)*2)+3;
}

u32 pdGetRegData(u32 id)
{
	u32 ret1, ret2;

	ret1 = pdGetInSize((u8)id);

	ret2 = pdGetOutSize((u8)id);

	return ((ret2 & 0x1FF) << 18) | ((ret1 & 0x1FF) << 8) | 0x40;
}

u32 pdSetRegData(u32 port, u32 slot, u32 reg_data)
{
	if(port < 2)
	{
		padData[port][slot].reg_data = reg_data;
		return 1;
	}

	return 0;

}

u32 setupReadData(u32 port, u32 slot, u32 val)
{
	u32 res;
	u8 buf[32];

	res = sio2CmdGetPortCtrl1(0, val, 0);

	pdSetCtrl1(port, slot, res);

	res = sio2CmdGetPortCtrl2(0, val);

	pdSetCtrl2(port, slot, res);


	res = pdGetRegData(0);

	pdSetRegData(port, slot, res);


	res = pdGetInSize(0);

	pdSetInSize(port, slot, res);

	res = pdGetOutSize(0);

	pdSetOutSize(port, slot, res);

	sio2CmdSetReadData(0, buf);

	pdSetInBuffer(port, slot, 0, buf);

	return 1;
}

u32 pdSetActive(u32 port, u32 slot, u32 active)
{
	u32 res = padData[port][slot].active;

	padData[port][slot].active = active;

	return res;
}

u32 pdIsActive(u32 port, u32 slot)
{
	return padData[port][slot].active;
}

static u32 mtapChangeSlot(u32 slot)
{
	change_slot_buffer[0] = -1;
	change_slot_buffer[1] = -1;
	change_slot_buffer[2] = -1;
	change_slot_buffer[3] = -1;

	if( (padData[0][slot].active == 0) && (padData[1][slot].active == 0) )
		return 0;

	if(padData[0][slot].active == 1)
		change_slot_buffer[0] = slot;

	if(padData[1][slot].active == 1)
		change_slot_buffer[1] = slot;

	sio2_mtap_change_slot(change_slot_buffer);

	return 1;
}

u32 pdSetStat70bit(u32 port, u32 slot, u32 val)
{
	u32 ret = padData[port][slot].stat70bit;

	padData[port][slot].stat70bit = val;

	return ret;
}

static u32 setupTransferData(u32 index, u32 port, u32 slot)
{

	if(padData[port][slot].in_size > 0)
	{
		int i;

		for(i=0; i < padData[port][slot].in_size; i++)
		{
			sio2_td.in[sio2_td.in_size] = padData[port][slot].in_buffer[i];
			sio2_td.in_size++;
		}
	}

	sio2_td.out_size += padData[port][slot].out_size;

	sio2_td.port_ctrl1[port] = padData[port][slot].port_ctrl1;
	sio2_td.port_ctrl2[port] = padData[port][slot].port_ctrl2;
	sio2_td.regdata[index] = (padData[port][slot].reg_data & 0xFFFFFFFC) | (port & 0x3);

	sio2_td.regdata[index+1] = 0;

	return (index+1);
}

u32 readStat6cBit(u32 bit, sio2_transfer_data_t *td)
{
	return( (td->stat6c & (0x00010000 << bit)) != 0);
}

u32 readSio2OutBuffer(u32 bit, u32 port, u32 slot)
{
	if(((sio2_td.stat6c >> 13) & 0x1) == 1)
	{
		padData[port][slot].error = 1;
		return 0;
	}
	else
	{
		if(readStat6cBit(bit, &sio2_td) == 1)
		{
			padData[port][slot].error = 1;
			return 0;
		}
		else
		{
			padData[port][slot].error = 0;

			if( padData[port][slot].out_size > 0)
			{
				int i;

				for(i=0; i < padData[port][slot].out_size; i++)
				{
					padData[port][slot].out_buffer[i] = sio2_td.out[sio2_td.out_size];
					sio2_td.out_size++;
				}
			}

			return 1;
		}
	}
}

u32 padTransfer(u32 slot)
{
	u32 stat70;
	u32 trans_count = 0, port0_start = 0, port1_start = 0;
	u32 val_port0 = 0;
	u32 val_port1 = 0;

	sio2_td.in_size = 0;
	sio2_td.out_size = 0;
	sio2_td.in_dma.addr = 0;
	sio2_td.out_dma.addr = 0;

	stat70 = sio2_stat70_get();

	pdSetStat70bit(0, slot, (stat70 >> 4) & 1);
	pdSetStat70bit(1, slot, (stat70 >> 5) & 1);

	if( padData[0][slot].active == 1 )
	{
		if(change_slot_buffer[4] == 1)
		{
			val_port0 = 1;
			port0_start = 0;
			trans_count = setupTransferData(0, 0, slot);
		}
	}

	if( padData[1][slot].active == 1 )
	{
		if(change_slot_buffer[5] == 1)
		{
			val_port1 = 1;
			port1_start = trans_count;
			trans_count = setupTransferData(trans_count, 1, slot);
		}
	}

	/*	These two blocks of code change position before v3.6.
		At v3.3 and earlier, they were within the transfer block below. At v3.0, there were no checks on change_slot_buffer.	*/
	if(padData[0][slot].active == 1)
	{
		if(change_slot_buffer[4] != 1)
			padData[0][slot].error = 0xA;
	}

	if(padData[1][slot].active == 1)
	{
		if(change_slot_buffer[5] != 1)
			padData[1][slot].error = 0xA;
	}

	if(trans_count != 0)
	{
		sio2_transfer2( &sio2_td );

		sio2_td.out_size = 0;

		if(val_port0 == 1)
		{
			readSio2OutBuffer(port0_start, 0, slot);
			sio2_td.out_size = padData[0][slot].out_size;
		}

		if(val_port1 == 1)
		{
			readSio2OutBuffer(port1_start, 1, slot);
			sio2_td.out_size = padData[1][slot].out_size;
		}

		return 1;
	}

	return 0;
}

void pdTransfer(void)
{
	u32 slot;

	sio2_pad_transfer_init();

	transferCount++;

	for(slot=0; slot < 4; slot++)
	{
		if( (padData[0][slot].active == 1) || (padData[1][slot].active == 1) )
		{
			if(slot > 0) mtapChangeSlot(slot);

			padTransfer(slot);
		}
	}

	mtapChangeSlot(0);

	sio2_transfer_reset2();
}

u32 pdGetStat70bit(u32 port, u32 slot)
{
	return padData[port][slot].stat70bit;
}

void pdReset(void)
{
	int i,j;

	for(i=0; i < 2; i++)
	{
		for(j=0; j < 4; j++)
		{
			padData[i][j].active = 0;
			padData[i][j].stat70bit = 0;	//This is probably more correct, compared to the original line (below)
		}

		//padData[i+1][0].stat70bit = 0;	//BUG: can cause buffer overrun.
	}

	sio2_td.in = sio2_in_buffer;
	sio2_td.out = sio2_out_buffer;
}

//In versions before v3.6, only the slot argument was present.
static u32 SlotCheckConnection(u32 port, u32 slot)
{
	u32 stat70;

	sio2_td.in_size = 0;
	sio2_td.out_size = 0;
	sio2_td.out_dma.addr = 0;
	sio2_td.in_dma.addr = 0;

	if(change_slot_buffer[4+port] == 1)
	{	//In versions before v3.6, this outer check did not exist.
		stat70 = sio2_stat70_get();

		//In versions before v3.6, stat70 is always set regardless of the slot selected.
		switch(port)
		{
			case 0:
				pdSetStat70bit(0, slot, (stat70 >> 4) & 0x1);
				break;
			case 1:
				pdSetStat70bit(1, slot, (stat70 >> 5) & 0x1);
				break;
		}

		//In versions before v3.6, there used to be checks on change_slot_buffer[] and independent calls to setupTransferData, for each port (very much like padTransfer).
		setupTransferData(0, port, slot);

		//As a result, there is neither a check on whether there is data to send.
		sio2_transfer2( &sio2_td );

		sio2_td.out_size = 0;

		//There used to be independent calls to readSio2OutBuffer for each port too.
		readSio2OutBuffer(0, port, slot);
		sio2_td.out_size = padData[port][slot].out_size;

		//This used to be per port too.
		if(change_slot_buffer[4+port] != 1)
			padData[port][slot].error = 0xA;

		return 1;
	}

	return 0;
}

u32 pdCheckConnection(u32 port, u32 slot)
{
	u32 res;
	u32 ret = 0;

	sio2_pad_transfer_init();

	change_slot_buffer[0] = slot;
	change_slot_buffer[1] = slot;
	change_slot_buffer[2] = -1;
	change_slot_buffer[3] = -1;

	sio2_mtap_change_slot(change_slot_buffer);

	res = pdGetStat70bit(0, slot);
	setupReadData(0, slot, res);

	res = pdGetStat70bit(1, slot);
	setupReadData(1, slot, res);


	if(SlotCheckConnection(port, slot) == 1)
	{
		if( padData[port][slot].error == 0 )
			ret = 1;
	}

	change_slot_buffer[0] = 0;
	change_slot_buffer[1] = 0;
	change_slot_buffer[2] = -1;
	change_slot_buffer[3] = -1;

	sio2_mtap_change_slot(change_slot_buffer);

	sio2_transfer_reset2();

	return ret;
}

s32 pdGetError(u32 port, u32 slot)
{
	if(port < 2)
		return padData[port][slot].error;
	else
		return -1;
}

u32 pdSetCtrl1(u32 port, u32 slot, u32 ctrl)
{
	if(port < 2)
	{
		padData[port][slot].port_ctrl1 = ctrl;
		return 1;
	}

	return 0;
}

u32 pdSetCtrl2(u32 port, u32 slot, u32 ctrl)
{
	if(port < 2)
	{
		padData[port][slot].port_ctrl2 = ctrl;
		return 1;
	}

	return 0;

}

u32 pdSetInSize(u32 port, u32 slot, u32 size)
{
	if(port < 2)
	{
		padData[port][slot].in_size = size;
		return size;
	}

	return 0;
}

u32 pdSetOutSize(u32 port, u32 slot, u32 size)
{
	if(port < 2)
	{
		padData[port][slot].out_size = size;
		return size;
	}

	return 0;

}

u32 pdSetInBuffer(u32 port, u32 slot, u32 size, const u8 *buf)
{
	if(port < 2)
	{
		if(size == 0) size = padData[port][slot].in_size;

		if(size > 0)
		{
			int i;

			for(i=0; i < size; i++)
				padData[port][slot].in_buffer[i] = buf[i];
		}

		return size;
	}

	return 0;
}

u32 pdGetOutBuffer(u32 port, u32 slot, u32 size, u8 *buf)
{
	if(port < 2)
	{
		if(size == 0) size = padData[port][slot].out_size;

		if(size > 0)
		{
			int i;

			for(i=0; i < size; i++)
				buf[i] = padData[port][slot].out_buffer[i];
		}

		return size;
	}

	return 0;
}

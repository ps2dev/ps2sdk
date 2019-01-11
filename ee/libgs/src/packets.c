/*
# _____     ___ ____     ___ ____
#  ____|   |    ____|   |        | |____|
# |     ___|   |____ ___|    ____| |    \    PS2DEV Open Source Project.
#-----------------------------------------------------------------------
# (c) 2009 Lion
# Licenced under Academic Free License version 2.0
# Review ps2sdk README & LICENSE files for further details.
*/

#include <errno.h>
#include <stdio.h>
#include <kernel.h>
#include <libgs.h>

#include "internal.h"

QWORD GsPrimWorkArea[64] __attribute__((aligned(64))); // Align to a 64-byte boundary.

QWORD *GsGifPacketsAlloc(GS_PACKET_TABLE *table, u32 num_qwords)
{
	void *pointer;
	GS_GIF_PACKET *packet;

	if(num_qwords <= (GS_PACKET_DATA_QWORD_MAX-table->qword_offset))			//check if we can alloc in current packet
	{
		pointer=&table->packets[table->packet_offset].data[table->qword_offset];
		table->qword_offset += num_qwords;
	}
	else	//couldnt fit so we going to have to try to jump to next packet
	{
		if(table->packet_offset+1 == table->packet_count) //no more packet available so we return error;
		{
			pointer=NULL;
		}
		else //there is another pocket available so we can jump to it
		{
			//before we jup to this packet then we got to end current packet and point it to the new one
			packet=(GS_GIF_PACKET*)UNCACHED_SEG(&table->packets[table->packet_offset]);
			packet->tag.qwc=table->qword_offset;
			packet->tag.pad1=0;
			packet->tag.pce=0;
			packet->tag.id=0x02; //next = tag.addr
			packet->tag.irq	=0;
			packet->tag.addr=(unsigned int)&((GS_GIF_PACKET*)UNCACHED_SEG(&table->packets[table->packet_offset + 1]))->tag;
			packet->tag.spr		=0;
			packet->tag.pad2	=0;

			//now reset qwords offset in this packet & update packet offset
			table->qword_offset= 0;
			table->packet_offset++;

			//now we use the new packet
			// but we still got to check if enough mem is available
			if( num_qwords <= (GS_PACKET_DATA_QWORD_MAX-table->qword_offset) )
			{
				pointer=&table->packets[table->packet_offset].data[table->qword_offset];
				table->qword_offset += num_qwords;
			}
			else //not enough
			{
				pointer=NULL;
			}
		}
	}

	return pointer;
}

void GsGifPacketsClear(GS_PACKET_TABLE *table)
{
	table->packet_offset=0;
	table->qword_offset=0;
}

int GsGifPacketsExecute(GS_PACKET_TABLE *table, u16 wait)
{
	GS_GIF_PACKET *packet;

	if(table->packet_offset==0    &&   table->qword_offset==0)
		return 0;

	if(table->packets == NULL)
		return -1;

	//close the current pocket
	packet=(GS_GIF_PACKET*)UNCACHED_SEG(&table->packets[table->packet_offset]);
	packet->tag.qwc		=table->qword_offset;
	packet->tag.pad1	=0;
	packet->tag.pce		=0;
	packet->tag.id		=0x07; //end
	packet->tag.irq		=0;
	packet->tag.addr	=(u32)0;
	packet->tag.spr		=0;
	packet->tag.pad2	=0;

	GsDmaSend_tag(0, 0, &table->packets[0].tag);
	if(wait)
		GsDmaWait();

	return 0;
}

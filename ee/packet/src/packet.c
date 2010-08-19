#include <string.h>
#include <stdlib.h>
#include <packet.h>

#define SPR_BEGIN 0x70000000

packet_t *packet_init(int qwords, int type)
{

	int byte_size = 0;
	packet_t *packet = (packet_t*)calloc(1,sizeof(packet_t));

	if (packet == NULL)
	{

		return NULL;

	}

	if (type == PACKET_SPR)
	{

		(u32*)packet->data = (u32*)SPR_BEGIN;
		packet->qwc = 0x1000;

	}
	else
	{
		// Size of qwords in bytes.
		byte_size = qwords << 4;

		// Allocate the data area in bytes aligned to cache line.
		if ((packet->data = memalign(64, byte_size)) == NULL)
		{
			free(packet);
			return NULL;

		}
	}

	// Set the pointer attribute to ucab space.
	if (type == PACKET_UCAB)
	{

		(u32)packet->data |= (u32)0x30000000;

	}

	// Clear the packet area.
	memset(packet->data, 0, byte_size);

	// Set the packet counts
	packet->qwc = 0;
	packet->qwords = qwords;
	packet->type = type;

	// End function.
	return packet;

}

void packet_free(packet_t *packet)
{

	// Free the allocated data buffer.
	if (packet->type == PACKET_SPR)
	{

		packet->data = NULL; 

	}
	else
	{
		if (packet->type == PACKET_UCAB) 
		{

			(u32)packet->data ^= 0x30000000;

		}

		free(packet->data);
	}

	free(packet);

}

void packet_reset(packet_t *packet)
{

	// Reset the quadword counter.
	packet->qwc = 0;

	if (packet->type == PACKET_SPR) 
	{

		(u8*)packet->data = (u8*)SPR_BEGIN;
		return;

	}

	// Zero out the data
	memset(packet->data, 0, packet->qwords << 4);

}

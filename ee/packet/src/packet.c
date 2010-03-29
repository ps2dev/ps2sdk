#include <string.h>
#include <stdlib.h>
#include <packet.h>

#define SPR_BEGIN 0x70000000

int packet_allocate(PACKET *packet, int num, int ucab, int spr)
{

	if (packet == NULL)
	{

		return 0;

	}

	if (spr && (num >= 0x1000))
	{

		(u32*)packet->data = (u32*)SPR_BEGIN; 
		packet->total = 0x1000;

  return 0;

 }

	// Size of qwords in bytes.
	int byte_size = num << 4;

	// Allocate the data area in bytes aligned to cache line.
	if ((packet->data = memalign(64, byte_size)) == NULL)
	{

		return -1;

 }

	// Set the pointer attribute to ucab space.
	if (ucab)
	{

		(u32)packet->data |= (u32)0x30000000;

	}

	// Clear the packet area.
	memset(packet->data, 0, byte_size);

	// Set the packet counts
	packet->qwc = 0;
	packet->total = num;
	packet->spr = spr;
	packet->ucab = ucab;

  // End function.
  return 0;

}

void packet_free(PACKET *packet)
{

	// Free the allocated data buffer.
	if (packet->spr)
	{

		packet->data = NULL; 
		return;

	}

	if (packet->ucab) 
	{

		(u32)packet->data ^= 0x30000000;
		return;

 }

	free(packet->data);
	free(packet);

}

void packet_reset(PACKET *packet)
{

	// Reset the quadword counter.
	packet->qwc = 0;

	if (packet->spr) 
	{

		(u8*)packet->data = (u8*)SPR_BEGIN;
		return;

 }

	if (packet->ucab)
	{

		(u32)packet->data ^= 0x30000000;

	}

	// Zero out the data
	memset(packet->data, 0, packet->total << 4);

	if (packet->ucab)
	{

		(u32)packet->data ^= 0x30000000;

 }

}

// For those that like getters and setters
QWORD *packet_get_qword(PACKET *packet)
{

	return packet->data;

}

QWORD *packet_increment_qwc(PACKET *packet, int num)
{

	// Check if we have enough qwords left
	if ((packet->qwc += num) > packet->total) 
	{

		// Return the old qword count
		packet->qwc -= num;

	}

	// Return the current qword count
	return packet->data + packet->qwc+1;

}


 #include <tamtypes.h>

 #include <dma.h>
 #include <stdlib.h>
 #include <packet.h>

 //////////////////////
 // PACKET FUNCTIONS //
 //////////////////////

 int packet_allocate(PACKET *packet, int size) {

  // Check the packet argument.
  if (packet == NULL) { return -1; }

  // Allocate the data area.
  if ((packet->data = memalign(128, size)) == NULL) { return -1; }

  // Save the data size.
  packet->size = size;

  // Reset the packet.
  if (packet_reset(packet) < 0) { return -1; }

  // End function.
  return 0;

 }

 int packet_reset(PACKET *packet) {

  // Check the packet argument.
  if (packet == NULL) { return -1; }

  // Reset the packet counter.
  packet->count = 0;

  // End function.
  return 0;

 }

 int packet_append_8(PACKET *packet, u8 data) {

  // Check the packet argument.
  if (packet == NULL) { return -1; }

  // Check the packet data area.
  if (packet->data == NULL) { return -1; }

  // Append the data to the packet.
  *(u8 *)(&packet->data[packet->count]) = data;

  // Increment the counter.
  if ((packet->count += sizeof(u8)) > packet->size) { return -1; }

  // End function.
  return 0;

 }

 int packet_append_16(PACKET *packet, u16 data) {

  // Check the packet argument.
  if (packet == NULL) { return -1; }

  // Check the packet data area.
  if (packet->data == NULL) { return -1; }

  // Append the data to the packet.
  *(u16 *)(&packet->data[packet->count]) = data;

  // Increment the counter.
  if ((packet->count += sizeof(u16)) > packet->size) { return -1; }

  // End function.
  return 0;

 }

 int packet_append_32(PACKET *packet, u32 data) {

  // Check the packet argument.
  if (packet == NULL) { return -1; }

  // Check the packet data area.
  if (packet->data == NULL) { return -1; }

  // Append the data to the packet.
  *(u32 *)(&packet->data[packet->count]) = data;

  // Increment the counter.
  if ((packet->count += sizeof(u32)) > packet->size) { return -1; }

  // End function.
  return 0;

 }

 int packet_append_64(PACKET *packet, u64 data) {

  // Check the packet argument.
  if (packet == NULL) { return -1; }

  // Check the packet data area.
  if (packet->data == NULL) { return -1; }

  // Append the data to the packet.
  *(u64 *)(&packet->data[packet->count]) = data;

  // Increment the counter.
  if ((packet->count += sizeof(u64)) > packet->size) { return -1; }

  // End function.
  return 0;

 }

 int packet_send(PACKET *packet, int dma_channel, int dma_flags) {

  // Check the packet argument.
  if (packet == NULL) { return -1; }

  // Check the packet data area.
  if (packet->data == NULL) { return -1; }

  // Fill out the packet out for quadword alignment.
  while (packet->count & 0x000F) { packet_append_8(packet, 0); }

  // Send the packet data.
  if (dma_channel_send(dma_channel, packet->data, packet->count, dma_flags) < 0) { return -1; }

  // End function.
  return 0;

 }

 int packet_free(PACKET *packet) {

  // Check the packet argument.
  if (packet == NULL) { return -1; }

  // Check the packet data area.
  if (packet->data == NULL) { return -1; }

  // Free the allocated data buffer.
  free(packet->data);

  // End function.
  return 0;

 }

/*
# _____     ___ ____     ___ ____
#  ____|   |    ____|   |        | |____|
# |     ___|   |____ ___|    ____| |    \    PS2DEV Open Source Project.
#-----------------------------------------------------------------------
# (c) 2005 Dan Peori <peori@oopo.net>
# Licenced under Academic Free License version 2.0
# Review ps2sdk README & LICENSE files for further details.
#
*/

 #include <graph.h>
 #include <graph_registers.h>

 #include <tamtypes.h>
 #include <kernel.h>
 #include <malloc.h>
 #include <string.h>
 #include <../../dma/include/dma.h>

 GRAPH_MODE graph_mode[4] = {
  { 640, 448, 0x02, 1, 660, 48, 3, 0, 2559, 447 }, // NTSC (640x448i)
  { 640, 512, 0x03, 1, 652, 30, 3, 0, 2559, 511 }, // PAL  (640x512i)
  { 720, 480, 0x50, 0, 232, 35, 2, 0, 2159, 479 }, // HDTV (720x480p)
  { 640, 480, 0x1A, 0, 280, 18, 3, 0, 2559, 479 }  // VGA  (640x480p)
 };

 GRAPH_PACKET graph_packet;

 int current_mode = 0, current_bpp = 0;

 /////////////////////
 // GRAPH FUNCTIONS //
 /////////////////////

 int graph_initialize(void) {

  // Reset the gif.
  ResetEE(0x08);

  // Reset and flush the gs.
  GS_REG_CSR = GS_SET_CSR(0, 0, 0, 0, 0, 1, 1, 0, 0, 0, 0, 0);

  // Initialize the dma controller.
  dma_initialize();

  // Initialize the gif dma channel.
  dma_channel_initialize(DMA_CHANNEL_GIF, NULL);

  // Allocate an internal use packet.
  graph_packet_allocate(&graph_packet, 512);

  // End function.
  return 0;

 }

 int graph_shutdown(void) {

  // Shut down the dma gif channel.
  dma_channel_shutdown(DMA_CHANNEL_GIF);

  // Free the internal use packet.
  graph_packet_free(&graph_packet);

  // End function.
  return 0;

 }

 ////////////////////////////
 // GRAPH PACKET FUNCTIONS //
 ////////////////////////////

 int graph_packet_allocate(GRAPH_PACKET *packet, int size) {

  // Allocate space for the packet data.
  packet->data = memalign(size, 128);

  // Save the packet size.
  packet->size = size;

  // Clear the packet before use.
  graph_packet_clear(packet);

  // End function.
  return 0;

 }

 int graph_packet_clear(GRAPH_PACKET *packet) {

  // Clear the packet data.
  memset(packet->data, 0x00, packet->size);

  // Reset the packet counter.
  packet->count = 0;

  // End function.
  return 0;

 }

 int graph_packet_append(GRAPH_PACKET *packet, u64 data) {

  // Append the data to the packet.
  packet->data[packet->count++] = data;

  // End function.
  return 0;

 }

 int graph_packet_send(GRAPH_PACKET *packet) {

  // Pad the packet to the nearest quadword.
  if (packet->count & 0x01) { graph_packet_append(packet, 0); }

  // Send the packet to the gif, in number of bytes.
  dma_channel_send(DMA_CHANNEL_GIF, packet->data, packet->count << 3);

  // End function.
  return 0;

 }

 int graph_packet_free(GRAPH_PACKET *packet) {

  // Free the packet data buffer.
  free(packet->data);

  // End function.
  return 0;

 }

 /////////////////////////
 // GRAPH SET FUNCTIONS //
 /////////////////////////

 int graph_set_displaymode(int mode, int bpp) {

  // Request the mode change.
  SetGsCrt(graph_mode[mode].interlace, graph_mode[mode].mode, 0);

  // Set up the mode.
  GS_REG_PMODE = GS_SET_PMODE(1, 1, 0, 0, 0, 256);
  GS_REG_DISPLAY1 = GS_SET_DISPLAY(graph_mode[mode].dx, graph_mode[mode].dy, graph_mode[mode].magh, graph_mode[mode].magv, graph_mode[mode].dw, graph_mode[mode].dh);
  GS_REG_DISPLAY2 = GS_SET_DISPLAY(graph_mode[mode].dx, graph_mode[mode].dy, graph_mode[mode].magh, graph_mode[mode].magv, graph_mode[mode].dw, graph_mode[mode].dh);

  // Save the mode and bpp.
  current_mode = mode; current_bpp = bpp;

  // End function.
  return 0;

 }

 int graph_set_displaybuffer(int address) {

  // Set up the display buffer.
  GS_REG_DISPFB1 = GS_SET_DISPFB(address >> 13, graph_mode[current_mode].width >> 6, current_bpp, 0, 0);
  GS_REG_DISPFB2 = GS_SET_DISPFB(address >> 13, graph_mode[current_mode].width >> 6, current_bpp, 0, 0);

  // End function.
  return 0;

 }

 int graph_set_drawbuffer(int address, int width, int height, int bpp) {

  // Autofill width, height and bpp if none are given.
  if (width  < 0) { width  = graph_mode[current_mode].width;  }
  if (height < 0) { height = graph_mode[current_mode].height; }
  if (bpp    < 0) { bpp = current_bpp; }

  // Set up the draw buffer.
  graph_packet_clear(&graph_packet);
  graph_packet_append(&graph_packet, GIF_SET_TAG(4, 1, 0, 0, 0, 1));
  graph_packet_append(&graph_packet, 0x0E);
  graph_packet_append(&graph_packet, GIF_SET_FRAME(address >> 13, width >> 6, bpp, 0));
  graph_packet_append(&graph_packet, GIF_REG_FRAME_1);
  graph_packet_append(&graph_packet, GIF_SET_SCISSOR(0, width - 1, 0, height - 1));
  graph_packet_append(&graph_packet, GIF_REG_SCISSOR_1);
  graph_packet_append(&graph_packet, GIF_SET_TEST(0, 0, 0, 0, 0, 0, 1, 2));
  graph_packet_append(&graph_packet, GIF_REG_TEST_1);
  graph_packet_append(&graph_packet, GIF_SET_XYOFFSET(0, 0));
  graph_packet_append(&graph_packet, GIF_REG_XYOFFSET_1);
  graph_packet_send(&graph_packet);

  // End function.
  return 0;

 }

 int graph_set_zbuffer(int address, int bpp) {

  // Set up the zbuffer.
  graph_packet_clear(&graph_packet);
  graph_packet_append(&graph_packet, GIF_SET_TAG(1, 1, 0, 0, 0, 1));
  graph_packet_append(&graph_packet, 0x0E);
  graph_packet_append(&graph_packet, GIF_SET_ZBUF(address >> 13, bpp, 0));
  graph_packet_append(&graph_packet, GIF_REG_ZBUF_1);
  graph_packet_send(&graph_packet);

  // End function.
  return 0;

 }

 int graph_set_clearbuffer(int red, int green, int blue, int alpha) {

  // Clear the screen.
  graph_packet_clear(&graph_packet);
  graph_packet_append(&graph_packet, GIF_SET_TAG(6, 1, 0, 0, 0, 1));
  graph_packet_append(&graph_packet, 0x000000000000000E);
  graph_packet_append(&graph_packet, GIF_SET_TEST(0, 0, 0, 0, 0, 0, 0, 1));
  graph_packet_append(&graph_packet, GIF_REG_TEST_1);
  graph_packet_append(&graph_packet, GIF_SET_PRIM(6, 0, 0, 0, 0, 0, 0, 0, 0));
  graph_packet_append(&graph_packet, GIF_REG_PRIM);
  graph_packet_append(&graph_packet, GIF_SET_RGBAQ(red, green, blue, alpha, 0x3F800000));
  graph_packet_append(&graph_packet, GIF_REG_RGBAQ);
  graph_packet_append(&graph_packet, GIF_SET_XYZ(0, 0, 0));
  graph_packet_append(&graph_packet, GIF_REG_XYZ2);
  graph_packet_append(&graph_packet, GIF_SET_XYZ(65535, 65535, 0));
  graph_packet_append(&graph_packet, GIF_REG_XYZ2);
  graph_packet_append(&graph_packet, GIF_SET_TEST(0, 0, 0, 0, 0, 0, 1, 2));
  graph_packet_append(&graph_packet, GIF_REG_TEST_1);
  graph_packet_send(&graph_packet);

  // End function.
  return 0;

 }

 //////////////////////////
 // GRAPH VRAM FUNCTIONS //
 //////////////////////////

 int graph_vram_read(int address, int width, int height, int bpp, void *buffer) {

  // NOT READY YET

  // End function.
  return 0;

 }

 int graph_vram_write(int address, int width, int height, int bpp, void *buffer) {

  // NOT READY YET

  // End function.
  return 0;

 }

 ///////////////////////////
 // GRAPH VSYNC FUNCTIONS //
 ///////////////////////////

 int graph_vsync_wait(void) {

  // Enable the Vsync interrupt and flush the GS.
  GS_REG_CSR &= GS_SET_CSR(0, 0, 0, 1, 0, 1, 0, 0, 0, 0, 0, 0);

  // Wait for the vsync interrupt.
  while(!(GS_REG_CSR & (GS_SET_CSR(0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0)))) { }

  // End function.
  return 0;

 }

 int graph_vsync_handler_add(void *handler) {

  // NOT READY YET

  // End function.
  return 0;

 }

 int graph_vsync_handler_remove(void) {

  // NOT READY YET

  // End function.
  return 0;

 }

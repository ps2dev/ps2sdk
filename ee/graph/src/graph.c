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

 #include <tamtypes.h>

 #include <dma.h>
 #include <graph.h>
 #include <string.h>
 #include <kernel.h>
 #include <packet.h>

 GRAPH_MODE graph_mode[9] = {
  {  640,  448, 0x02, 1,  286720, GS_SET_DISPLAY(632, 50, 3, 0, 2559,  447) }, // GRAPH_MODE_NTSC
  {  640,  512, 0x03, 1,  286720, GS_SET_DISPLAY(652, 72, 3, 0, 2559,  511) }, // GRAPH_MODE_PAL
  {  720,  480, 0x50, 0,  393216, GS_SET_DISPLAY(232, 35, 1, 0, 1439,  479) }, // GRAPH_MODE_HDTV_480P
  { 1280,  720, 0x51, 0,  983040, GS_SET_DISPLAY(302, 24, 0, 0, 1279,  719) }, // GRAPH_MODE_HDTV_720P
  { 1920, 1080, 0x52, 1, 2088960, GS_SET_DISPLAY(238, 40, 0, 0, 1919, 1079) }, // GRAPH_MODE_HDTV_1080I
  {  640,  480, 0x1A, 0,  327680, GS_SET_DISPLAY(276, 34, 1, 0, 1279,  479) }, // GRAPH_MODE_VGA_640
  {  800,  600, 0x2B, 0,  532480, GS_SET_DISPLAY(420, 26, 1, 0, 1599,  599) }, // GRAPH_MODE_VGA_800
  { 1024,  768, 0x3B, 0,  786432, GS_SET_DISPLAY(580, 34, 1, 0, 2047,  767) }, // GRAPH_MODE_VGA_1024
  { 1280, 1024, 0x4A, 0, 1310720, GS_SET_DISPLAY(348, 40, 0, 0, 1279, 1023) }, // GRAPH_MODE_VGA_1280
 };

 int current_mode = -1, current_psm = -1, current_zpsm = -1;

 int graph_initialized = -1;

 PACKET graph_packet;

 /////////////////////
 // GRAPH FUNCTIONS //
 /////////////////////

 int graph_initialize(void) {

  // Reset the gif.
  ResetEE(0x08);

  // Reset and flush the gs.
  GS_REG_CSR = GS_SET_CSR(0, 0, 0, 0, 0, 1, 1, 0, 0, 0, 0, 0);

  // Allocate space for the packet.
  if (packet_allocate(&graph_packet, 1024) < 0) { return -1; }

  // Reset the current mode, psm and zpsm.
  current_mode = -1; current_psm = -1; current_zpsm = -1;

  // Tell everyone we are initialized.
  graph_initialized = 1;

  // End function.
  return 0;

 }

 int graph_shutdown(void) {

  // If we are not initialized, no need to shutdown.
  if (graph_initialized < 0) { return 0; }

  // Free the space for the packet.
  if (packet_free(&graph_packet) < 0) { return -1; }

  // Reset the current mode, psm and zpsm.
  current_mode = -1; current_psm = -1; current_zpsm = -1;

  // Tell everyone we are not initialized.
  graph_initialized = -1;

  // End function.
  return 0;

 }

 /////////////////////////
 // GRAPH GET FUNCTIONS //
 /////////////////////////

 int graph_get_bpp(void) {

  // If we are not initialized, initialize the library.
  if (graph_initialized < 0) { if (graph_initialize() < 0) { return -1; } }

  // If no mode is set, return an error.
  if (current_mode < 0) { return -1; }

  // If no psm is set, return an error.
  if (current_psm < 0) { return -1; }

  // Return the bpp of the current mode.
  if (current_psm == GRAPH_PSM_32)  { return 32; } else
  if (current_psm == GRAPH_PSM_24)  { return 24; } else
  if (current_psm == GRAPH_PSM_16)  { return 16; } else
  if (current_psm == GRAPH_PSM_16S) { return 16; } else { return -1; }

  // End function.
  return 0;

 }

 int graph_get_height(void) {

  // If we are not initialized, initialize the library.
  if (graph_initialized < 0) { if (graph_initialize() < 0) { return -1; } }

  // If no mode is set, return an error.
  if (current_mode < 0) { return -1; }

  // Return the height of the current mode.
  return graph_mode[current_mode].height;

 }

 int graph_get_interlace(void) {

  // If we are not initialized, initialize the library.
  if (graph_initialized < 0) { if (graph_initialize() < 0) { return -1; } }

  // If no mode is set, return an error.
  if (current_mode < 0) { return -1; }

  // Return the interlace of the current mode.
  return graph_mode[current_mode].interlace;

 }

 int graph_get_mode(GRAPH_MODE *mode) {

  // If we are not initialized, initialize the library.
  if (graph_initialized < 0) { if (graph_initialize() < 0) { return -1; } }

  // If no mode is set, return an error.
  if (current_mode < 0) { return -1; }

  // If no mode is given, just return the current mode.
  if (mode == NULL) { return current_mode; }

  // Copy the mode structure.
  memcpy(mode, &graph_mode[current_mode], sizeof(GRAPH_MODE));

  // End function.
  return current_mode;

 }

 int graph_get_psm(void) {

  // If we are not initialized, initialize the library.
  if (graph_initialized < 0) { if (graph_initialize() < 0) { return -1; } }

  // If no psm is set, return an error.
  if (current_psm < 0) { return -1; }

  // Return the the current psm.
  return current_psm;

 }

 int graph_get_size(void) {

  // If we are not initialized, initialize the library.
  if (graph_initialized < 0) { if (graph_initialize() < 0) { return -1; } }

  // If no mode is set, return an error.
  if (current_mode < 0) { return -1; }

  // Return the size of the current mode.
  return graph_mode[current_mode].size;

 }

 int graph_get_width(void) {

  // If we are not initialized, initialize the library.
  if (graph_initialized < 0) { if (graph_initialize() < 0) { return -1; } }

  // If no mode is set, return an error.
  if (current_mode < 0) { return -1; }

  // Return the width of the current mode.
  return graph_mode[current_mode].width;

 }

 int graph_get_zbpp(void) {

  // If we are not initialized, initialize the library.
  if (graph_initialized < 0) { if (graph_initialize() < 0) { return -1; } }

  // If no mode is set, return an error.
  if (current_mode < 0) { return -1; }

  // If no zpsm is set, return an error.
  if (current_zpsm < 0) { return -1; }

  // Return the zbpp of the current mode.
  if (current_zpsm == GRAPH_PSM_32)  { return 32; } else
  if (current_zpsm == GRAPH_PSM_24)  { return 24; } else
  if (current_zpsm == GRAPH_PSM_16)  { return 16; } else
  if (current_zpsm == GRAPH_PSM_16S) { return 16; } else { return -1; }

  // End function.
  return 0;

 }

 int graph_get_zpsm(void) {

  // If we are not initialized, initialize the library.
  if (graph_initialized < 0) { if (graph_initialize() < 0) { return -1; } }

  // If no zpsm is set, return an error.
  if (current_zpsm < 0) { return -1; }

  // Return the the current zpsm.
  return current_zpsm;

 }

 /////////////////////////
 // GRAPH SET FUNCTIONS //
 /////////////////////////

 int graph_set_clearbuffer(int red, int green, int blue) {

  // If we are not initialized, initialize the library.
  if (graph_initialized < 0) { if (graph_initialize() < 0) { return -1; } }

  // Reset the packet.
  if (packet_reset(&graph_packet) < 0) { return -1; }

  // Clear the screen.
  packet_append_64(&graph_packet, GIF_SET_TAG(6, 1, 0, 0, GIF_TAG_PACKED, 1));
  packet_append_64(&graph_packet, 0x0E);
  packet_append_64(&graph_packet, GIF_SET_TEST(0, 0, 0, 0, 0, 0, 1, 1));
  packet_append_64(&graph_packet, GIF_REG_TEST_1);
  packet_append_64(&graph_packet, GIF_SET_PRIM(6, 0, 0, 0, 0, 0, 0, 0, 0));
  packet_append_64(&graph_packet, GIF_REG_PRIM);
  packet_append_64(&graph_packet, GIF_SET_RGBAQ(red, green, blue, 0x80, 0x3F800000));
  packet_append_64(&graph_packet, GIF_REG_RGBAQ);
  packet_append_64(&graph_packet, GIF_SET_XYZ(0x0000, 0x0000, 0x0000));
  packet_append_64(&graph_packet, GIF_REG_XYZ2);
  packet_append_64(&graph_packet, GIF_SET_XYZ(0xFFFF, 0xFFFF, 0x0000));
  packet_append_64(&graph_packet, GIF_REG_XYZ2);
  packet_append_64(&graph_packet, GIF_SET_TEST(0, 0, 0, 0, 0, 0, 1, 2));
  packet_append_64(&graph_packet, GIF_REG_TEST_1);

  // Send the packet.
  if (packet_send(&graph_packet, DMA_CHANNEL_GIF, DMA_FLAG_NORMAL) < 0) { return -1; }

  // End function.
  return 0;

 }

 int graph_set_displaybuffer(int address) {

  // If we are not initialized, initialize the library.
  if (graph_initialized < 0) { if (graph_initialize() < 0) { return -1; } }

  // Set up the display buffer.
  GS_REG_DISPFB1 = GS_SET_DISPFB(address >> 13, graph_get_width() >> 6, current_psm, 0, 0);
  GS_REG_DISPFB2 = GS_SET_DISPFB(address >> 13, graph_get_width() >> 6, current_psm, 0, 0);

  // End function.
  return 0;

 }

 int graph_set_drawbuffer(int address) {

  // If we are not initialized, initialize the library.
  if (graph_initialized < 0) { if (graph_initialize() < 0) { return -1; } }

  // Reset the packet.
  if (packet_reset(&graph_packet) < 0) { return -1; }

  // Set up the draw buffer.
  packet_append_64(&graph_packet, GIF_SET_TAG(5, 1, 0, 0, GIF_TAG_PACKED, 1));
  packet_append_64(&graph_packet, 0x0E);
  packet_append_64(&graph_packet, GIF_SET_FRAME(address >> 13, graph_get_width() >> 6, current_psm, 0));
  packet_append_64(&graph_packet, GIF_REG_FRAME_1);
  packet_append_64(&graph_packet, GIF_SET_SCISSOR(0, graph_get_width() - 1, 0, graph_get_height() - 1));
  packet_append_64(&graph_packet, GIF_REG_SCISSOR_1);
  packet_append_64(&graph_packet, GIF_SET_TEST(0, 0, 0, 0, 0, 0, 1, 2));
  packet_append_64(&graph_packet, GIF_REG_TEST_1);
  packet_append_64(&graph_packet, GIF_SET_XYOFFSET((2048 - (graph_get_width() >> 1)) << 4, (2048 - (graph_get_height() >> 1)) << 4));
  packet_append_64(&graph_packet, GIF_REG_XYOFFSET_1);
  packet_append_64(&graph_packet, GIF_SET_PRMODECONT(1));
  packet_append_64(&graph_packet, GIF_REG_PRMODECONT);

  // Send the packet.
  if (packet_send(&graph_packet, DMA_CHANNEL_GIF, DMA_FLAG_NORMAL) < 0) { return -1; }

  // End function.
  return 0;

 }

 int graph_set_mode(int mode, int psm, int zpsm) {

  // If we are not initialized, initialize the library.
  if (graph_initialized < 0) { if (graph_initialize() < 0) { return -1; } }

  // Request the mode change.
  SetGsCrt(graph_mode[mode].interlace, graph_mode[mode].mode, 0);

  // Set up the mode.
  GS_REG_PMODE = GS_SET_PMODE(1, 1, 0, 0, 0, 128);
  GS_REG_DISPLAY1 = graph_mode[mode].display;
  GS_REG_DISPLAY2 = graph_mode[mode].display;

  // Save the mode, psm and zpsm.
  current_mode = mode; current_psm = psm; current_zpsm = zpsm;

  // End function.
  return 0;

 }

 int graph_set_zbuffer(int address) {

  // If we are not initialized, initialize the library.
  if (graph_initialized < 0) { if (graph_initialize() < 0) { return -1; } }

  // Reset the packet.
  if (packet_reset(&graph_packet) < 0) { return -1; }

  // Set up the zbuffer.
  packet_append_64(&graph_packet, GIF_SET_TAG(1, 1, 0, 0, GIF_TAG_PACKED, 1));
  packet_append_64(&graph_packet, 0x0E);
  packet_append_64(&graph_packet, GIF_SET_ZBUF(address >> 13, current_zpsm, 0));
  packet_append_64(&graph_packet, GIF_REG_ZBUF_1);

  // Send the packet.
  if (packet_send(&graph_packet, DMA_CHANNEL_GIF, DMA_FLAG_NORMAL) < 0) { return -1; }

  // End function.
  return 0;

 }

 //////////////////////////
 // GRAPH VRAM FUNCTIONS //
 //////////////////////////

 #define VIF1_REG_STAT *((vu32 *)(0x10003C00))

 int graph_vram_read(int address, int width, int height, int psm, void *data, int data_size) {

  // If we are not initialized, initialize the library.
  if (graph_initialized < 0) { if (graph_initialize() < 0) { return -1; } }

  // Reset the packet.
  if (packet_reset(&graph_packet) < 0) { return -1; }

  // Build the packet.
  packet_append_64(&graph_packet, GIF_SET_TAG(4, 1, 0, 0, GIF_TAG_PACKED, 1));
  packet_append_64(&graph_packet, 0x0E);
  packet_append_64(&graph_packet, GIF_SET_BITBLTBUF(address >> 8, width >> 6, psm, 0, 0, 0));
  packet_append_64(&graph_packet, GIF_REG_BITBLTBUF);
  packet_append_64(&graph_packet, GIF_SET_TRXPOS(0, 0, 0, 0, 0));
  packet_append_64(&graph_packet, GIF_REG_TRXPOS);
  packet_append_64(&graph_packet, GIF_SET_TRXREG(width, height));
  packet_append_64(&graph_packet, GIF_REG_TRXREG);
  packet_append_64(&graph_packet, GIF_SET_TRXDIR(1));
  packet_append_64(&graph_packet, GIF_REG_TRXDIR);

  // Send the packet.
  if (packet_send(&graph_packet, DMA_CHANNEL_GIF, DMA_FLAG_NORMAL) < 0) { return -1; }

  // Wait for the packet transfer to complete.
  if (dma_channel_wait(DMA_CHANNEL_GIF, -1, DMA_FLAG_NORMAL) < 0) { return -1; }

  // Reverse the bus direction.
  GS_REG_BUSDIR = GS_SET_BUSDIR(1); VIF1_REG_STAT = (1 << 23);

  // Receive the data.
  if (dma_channel_receive(DMA_CHANNEL_VIF1, data, data_size, DMA_FLAG_NORMAL) < 0) { return -1; }

  // Wait for the data transfer to complete.
  if (dma_channel_wait(DMA_CHANNEL_VIF1, -1, DMA_FLAG_NORMAL) < 0) { return -1; }

  // Restore the bus direction.
  GS_REG_BUSDIR = GS_SET_BUSDIR(0); VIF1_REG_STAT = (0 << 23);

  // Flush the cache, just in case.
  FlushCache(0);

  // End function.
  return 0;

 }

 int graph_vram_write(int address, int width, int height, int psm, void *data, int data_size) {

  // If we are not initialized, initialize the library.
  if (graph_initialized < 0) { if (graph_initialize() < 0) { return -1; } }

  // Reset the packet.
  if (packet_reset(&graph_packet) < 0) { return -1; }

  // Build the packet.
  packet_append_64(&graph_packet, DMA_SET_TAG(6, 0, DMA_TAG_CNT, 0, 0, 0));
  packet_append_64(&graph_packet, 0x00);
  packet_append_64(&graph_packet, GIF_SET_TAG(4, 1, 0, 0, GIF_TAG_PACKED, 1));
  packet_append_64(&graph_packet, 0x0E);
  packet_append_64(&graph_packet, GIF_SET_BITBLTBUF(0, 0, 0, address >> 8, width >> 6, psm));
  packet_append_64(&graph_packet, GIF_REG_BITBLTBUF);
  packet_append_64(&graph_packet, GIF_SET_TRXPOS(0, 0, 0, 0, 0));
  packet_append_64(&graph_packet, GIF_REG_TRXPOS);
  packet_append_64(&graph_packet, GIF_SET_TRXREG(width, height));
  packet_append_64(&graph_packet, GIF_REG_TRXREG);
  packet_append_64(&graph_packet, GIF_SET_TRXDIR(0));
  packet_append_64(&graph_packet, GIF_REG_TRXDIR);
  packet_append_64(&graph_packet, GIF_SET_TAG(data_size >> 4, 1, 0, 0, GIF_TAG_IMAGE, 1));
  packet_append_64(&graph_packet, 0x00);
  packet_append_64(&graph_packet, DMA_SET_TAG(data_size >> 4, 0, DMA_TAG_REF, 0, (u32)data, 0));
  packet_append_64(&graph_packet, 0x00);
  packet_append_64(&graph_packet, DMA_SET_TAG(2, 0, DMA_TAG_END, 0, 0, 0));
  packet_append_64(&graph_packet, 0x00);
  packet_append_64(&graph_packet, GIF_SET_TAG(1, 1, 0, 0, GIF_TAG_PACKED, 1));
  packet_append_64(&graph_packet, 0x0E);
  packet_append_64(&graph_packet, 0x00);
  packet_append_64(&graph_packet, GIF_REG_TEXFLUSH);

  // Send the packet.
  if (packet_send(&graph_packet, DMA_CHANNEL_GIF, DMA_FLAG_NORMAL) < 0) { return -1; }

  // End function.
  return 0;

 }

 //////////////////////////
 // GRAPH WAIT FUNCTIONS //
 //////////////////////////

 int graph_wait_hsync(void) {

  // Enable the hsync interrupt.
  GS_REG_CSR |= GS_SET_CSR(0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0);

  // Wait for the hsync interrupt.
  while (!(GS_REG_CSR & (GS_SET_CSR(0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0)))) { }

  // Disable the hsync interrupt.
  GS_REG_CSR &= ~GS_SET_CSR(0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0);

  // End function.
  return 0;

 }

 int graph_wait_vsync(void) {

  // Enable the vsync interrupt.
  GS_REG_CSR |= GS_SET_CSR(0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0);

  // Wait for the vsync interrupt.
  while (!(GS_REG_CSR & (GS_SET_CSR(0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0)))) { }

  // Disable the vsync interrupt.
  GS_REG_CSR &= ~GS_SET_CSR(0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0);

  // End function.
  return 0;

 }

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

 #include <string.h>

 #include <graph.h>
 #include <graph_registers.h>

 #include <../../dma/include/dma.h>
 #include <../../dma/include/dma_registers.h>

 #include <tamtypes.h>
 #include <kernel.h>

 GRAPH_MODE graph_mode[4] = {
  { 640, 448, 0x02, 1, 660, 48, 3, 0, 2559, 447 }, // NTSC (640x448i)
  { 640, 512, 0x03, 1, 652, 30, 3, 0, 2559, 511 }, // PAL  (640x512i)
  { 720, 480, 0x50, 0, 232, 35, 1, 0, 2559, 479 }, // HDTV (720x480p)
  { 640, 480, 0x1A, 0, 280, 18, 1, 0, 2559, 479 }  // VGA  (640x480p)
 };

 int current_mode = 0, current_psm = 0, current_zpsm = 0;

 /////////////////////
 // GRAPH FUNCTIONS //
 /////////////////////

 int graph_initialize(void) {

  // Reset the gif.
  ResetEE(0x08);

  // Reset and flush the gs.
  GS_REG_CSR = GS_SET_CSR(0, 0, 0, 0, 0, 1, 1, 0, 0, 0, 0, 0);

  // Initialize the gif channel.
  dma_channel_initialize(DMA_CHANNEL_GIF, NULL, DMA_FLAG_NORMAL);

  // Initialize the vif1 channel.
  dma_channel_initialize(DMA_CHANNEL_VIF1, NULL, DMA_FLAG_NORMAL);

  // End function.
  return 0;

 }

 int graph_shutdown(void) {

  // Shut down the gif channel.
  dma_channel_shutdown(DMA_CHANNEL_GIF, DMA_FLAG_NORMAL);

  // End function.
  return 0;

 }

 //////////////////////////
 // GRAPH MODE FUNCTIONS //
 //////////////////////////

 int graph_mode_set(int mode, int psm, int zpsm) {

  // Request the mode change.
  SetGsCrt(graph_mode[mode].interlace, graph_mode[mode].mode, 0);

  // Set up the mode.
  GS_REG_PMODE = GS_SET_PMODE(1, 1, 0, 0, 0, 128);
  GS_REG_DISPLAY1 = GS_SET_DISPLAY(graph_mode[mode].dx, graph_mode[mode].dy, graph_mode[mode].magh, graph_mode[mode].magv, graph_mode[mode].dw, graph_mode[mode].dh);
  GS_REG_DISPLAY2 = GS_SET_DISPLAY(graph_mode[mode].dx, graph_mode[mode].dy, graph_mode[mode].magh, graph_mode[mode].magv, graph_mode[mode].dw, graph_mode[mode].dh);

  // Save the mode, psm and zpsm.
  current_mode = mode; current_psm = psm; current_zpsm = zpsm;

  // End function.
  return 0;

 }

 int graph_mode_get(GRAPH_MODE *mode) {

  // If no mode is given, return the current mode.
  if (mode == NULL) { return current_mode; }

  // Copy the mode structure.
  memcpy(mode, &graph_mode[current_mode], sizeof(GRAPH_MODE));

  // End function.
  return current_mode;

 }

 /////////////////////////
 // GRAPH SET FUNCTIONS //
 /////////////////////////

 int graph_set_displaybuffer(int address) {

  // Set up the display buffer.
  GS_REG_DISPFB1 = GS_SET_DISPFB(address >> 13, graph_mode[current_mode].width >> 6, current_psm, 0, 0);
  GS_REG_DISPFB2 = GS_SET_DISPFB(address >> 13, graph_mode[current_mode].width >> 6, current_psm, 0, 0);

  // End function.
  return 0;

 }

 int graph_set_drawbuffer(int address) {
  u64 packet[0x0C];

  // Set up the draw buffer.
  packet[0x00] = GIF_SET_TAG(5, 1, 0, 0, GIF_TAG_PACKED, 1);
  packet[0x01] = 0x0E;
  packet[0x02] = GIF_SET_FRAME(address >> 13, graph_mode[current_mode].width >> 6, current_psm, 0);
  packet[0x03] = GIF_REG_FRAME_1;
  packet[0x04] = GIF_SET_SCISSOR(0, graph_mode[current_mode].width - 1, 0, graph_mode[current_mode].height - 1);
  packet[0x05] = GIF_REG_SCISSOR_1;
  packet[0x06] = GIF_SET_TEST(0, 0, 0, 0, 0, 0, 1, 2);
  packet[0x07] = GIF_REG_TEST_1;
  packet[0x08] = GIF_SET_XYOFFSET((2048 - (graph_mode[current_mode].width >> 1)) << 4, (2048 - (graph_mode[current_mode].height >> 1)) << 4);
  packet[0x09] = GIF_REG_XYOFFSET_1;
  packet[0x0A] = GIF_SET_PRMODECONT(1);
  packet[0x0B] = GIF_REG_PRMODECONT;

  // Send the packet.
  dma_channel_send(DMA_CHANNEL_GIF, packet, sizeof(packet), DMA_FLAG_NORMAL);

  // End function.
  return 0;

 }

 int graph_set_zbuffer(int address) {
  u64 packet[0x04];

  // Set up the zbuffer.
  packet[0x00] = GIF_SET_TAG(1, 1, 0, 0, GIF_TAG_PACKED, 1);
  packet[0x01] = 0x0E;
  packet[0x02] = GIF_SET_ZBUF(address >> 13, current_zpsm, 0);
  packet[0x03] = GIF_REG_ZBUF_1;

  // Send the packet.
  dma_channel_send(DMA_CHANNEL_GIF, packet, sizeof(packet), DMA_FLAG_NORMAL);

  // End function.
  return 0;

 }

 int graph_set_clearbuffer(int red, int green, int blue) {
  u64 packet[0x0E];

  // Clear the screen.
  packet[0x00] = GIF_SET_TAG(6, 1, 0, 0, GIF_TAG_PACKED, 1);
  packet[0x01] = 0x0E;
  packet[0x02] = GIF_SET_TEST(0, 0, 0, 0, 0, 0, 1, 1);
  packet[0x03] = GIF_REG_TEST_1;
  packet[0x04] = GIF_SET_PRIM(6, 0, 0, 0, 0, 0, 0, 0, 0);
  packet[0x05] = GIF_REG_PRIM;
  packet[0x06] = GIF_SET_RGBAQ(red, green, blue, 0x80, 0x3F800000);
  packet[0x07] = GIF_REG_RGBAQ;
  packet[0x08] = GIF_SET_XYZ(0x0000, 0x0000, 0x0000);
  packet[0x09] = GIF_REG_XYZ2;
  packet[0x0A] = GIF_SET_XYZ(0xFFFF, 0xFFFF, 0x0000);
  packet[0x0B] = GIF_REG_XYZ2;
  packet[0x0C] = GIF_SET_TEST(0, 0, 0, 0, 0, 0, 1, 2);
  packet[0x0D] = GIF_REG_TEST_1;

  // Send the packet.
  dma_channel_send(DMA_CHANNEL_GIF, packet, sizeof(packet), DMA_FLAG_NORMAL);

  // End function.
  return 0;

 }

 //////////////////////////
 // GRAPH VRAM FUNCTIONS //
 //////////////////////////

 #define VIF1_REG_STAT *((vu32 *)(0x10003C00))

 int graph_vram_read(int address, int width, int height, int psm, void *data, int data_size) {
  u64 packet[0x0A];

  // Build the packet.
  packet[0x00] = GIF_SET_TAG(4, 1, 0, 0, GIF_TAG_PACKED, 1);
  packet[0x01] = 0x0E;
  packet[0x02] = GIF_SET_BITBLTBUF(address >> 8, width >> 6, psm, 0, 0, 0);
  packet[0x03] = GIF_REG_BITBLTBUF;
  packet[0x04] = GIF_SET_TRXPOS(0, 0, 0, 0, 0);
  packet[0x05] = GIF_REG_TRXPOS;
  packet[0x06] = GIF_SET_TRXREG(width, height);
  packet[0x07] = GIF_REG_TRXREG;
  packet[0x08] = GIF_SET_TRXDIR(1);
  packet[0x09] = GIF_REG_TRXDIR;

  // Send the packet.
  dma_channel_send(DMA_CHANNEL_GIF, packet, sizeof(packet), DMA_FLAG_NORMAL);

  // Wait for the packet transfer to complete.
  dma_channel_wait(DMA_CHANNEL_GIF, -1, DMA_FLAG_NORMAL);

  // Reverse the bus direction.
  GS_REG_BUSDIR = GS_SET_BUSDIR(1); VIF1_REG_STAT = (1 << 23);

  // Receive the data.
  dma_channel_receive(DMA_CHANNEL_VIF1, data, data_size, DMA_FLAG_NORMAL);

  // Wait for the data transfer to complete.
  dma_channel_wait(DMA_CHANNEL_VIF1, -1, DMA_FLAG_NORMAL);

  // Restore the bus direction.
  GS_REG_BUSDIR = GS_SET_BUSDIR(0); VIF1_REG_STAT = (0 << 23);

  // Flush the cache, just in case.
  FlushCache(0);

  // End function.
  return 0;

 }

 int graph_vram_write(int address, int width, int height, int psm, void *data, int data_size) {
  u64 packet[0x16];

  // Build the packet.
  packet[0x00] = DMA_SET_TAG(6, 0, DMA_TAG_CNT, 0, 0, 0);
  packet[0x01] = 0x00;
  packet[0x02] = GIF_SET_TAG(4, 1, 0, 0, GIF_TAG_PACKED, 1);
  packet[0x03] = 0x0E;
  packet[0x04] = GIF_SET_BITBLTBUF(0, 0, 0, address >> 8, width >> 6, psm);
  packet[0x05] = GIF_REG_BITBLTBUF;
  packet[0x06] = GIF_SET_TRXPOS(0, 0, 0, 0, 0);
  packet[0x07] = GIF_REG_TRXPOS;
  packet[0x08] = GIF_SET_TRXREG(width, height);
  packet[0x09] = GIF_REG_TRXREG;
  packet[0x0A] = GIF_SET_TRXDIR(0);
  packet[0x0B] = GIF_REG_TRXDIR;
  packet[0x0C] = GIF_SET_TAG(data_size >> 4, 1, 0, 0, GIF_TAG_IMAGE, 1);
  packet[0x0D] = 0x00;
  packet[0x0E] = DMA_SET_TAG(data_size >> 4, 0, DMA_TAG_REF, 0, (u32)data, 0);
  packet[0x0F] = 0x00;
  packet[0x10] = DMA_SET_TAG(2, 0, DMA_TAG_END, 0, 0, 0);
  packet[0x11] = 0x00;
  packet[0x12] = GIF_SET_TAG(1, 1, 0, 0, GIF_TAG_PACKED, 1);
  packet[0x13] = 0x0E;
  packet[0x14] = 0x00;
  packet[0x15] = GIF_REG_TEXFLUSH;

  // Send the packet.
  dma_channel_send(DMA_CHANNEL_GIF, packet, sizeof(packet), DMA_FLAG_CHAIN);

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

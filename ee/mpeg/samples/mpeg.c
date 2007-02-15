/*
# _____     ___ ____     ___ ____
#  ____|   |    ____|   |        | |____|
# |     ___|   |____ ___|    ____| |    \    PS2DEV Open Source Project.
#-----------------------------------------------------------------------
# Copyright (c) 2006-2007 Eugene Plotnikov <e-plotnikov@operamail.com>
# Licenced under Academic Free License version 2.0
# Review ps2sdk README & LICENSE files for further details.
#
# Simplest example of MPEG bitstream deconing/display
# Note: the program expects raw mpeg video data as it does not perform
#       any demuxing. Only 4:2:0 colorspace is supported by the MPEG
#       decoder. Scalable extensions are not supported either.
#       Test bitstreams can be obtained for free at ftp://ftp.tek.com/tv/test/streams/Element/MPEG-Video-Conformance/main-profile/
#       This program was only tested on SCPH-3004R PAL console.
#       For real life usage of 'libmpeg' refer SMS project.
*/
#include "libmpeg.h"

#include <kernel.h>
#include <dma.h>
#include <fileio.h>
#include <malloc.h>
#include <graph.h>
#include <packet.h>
#include <stdio.h>

#define MPEG_BITSTREAM_FILE "host:test.bin"
/* get the whole file (or first 24MB) into memory for simplicity */
#define MAX_SIZE            ( 1024 * 1024 * 24 )

typedef struct InitCBParam {

 MPEGSequenceInfo* m_pInfo;
 void*             m_pData;
 PACKET            m_XFerPck;
 PACKET            m_DrawPck;
 int               m_TexAddr;

} InitCBParam;

static unsigned char* s_pMPEGData;
static unsigned char* s_pTransferPtr;
static unsigned int   s_MPEGDataSize;

static int   SetDMA ( void* );
static void* InitCB ( void*, MPEGSequenceInfo* );

int main ( void ) {
/* read file (or part of it ) into memory */
 PACKET      lPck;
 InitCBParam lInfo;
 int         lFD = fioOpen ( MPEG_BITSTREAM_FILE, O_RDONLY );
 long        lSize;
 long        lPTS, lCurPTS;

 if ( lFD < 0 ) {
  printf ( "test_mpeg: could not open '%s'\n", MPEG_BITSTREAM_FILE );
  goto end;
 }  /* end if */

 lSize = fioLseek ( lFD, 0, SEEK_END );
 fioLseek ( lFD, 0, SEEK_SET );

 if ( lSize <= 0 ) {
  printf ( "test_mpeg: could not obtain file size (%ld)\n", lSize );
  goto end;
 }  /* end if */

 s_pMPEGData = ( unsigned char* )malloc ( lSize = lSize > MAX_SIZE ? MAX_SIZE : lSize );

 if ( !s_pMPEGData ) {
  printf ( "test_mpeg: could not allocate enough memory (%ld)\n", lSize );
  goto end;
 }  /* end if */

 if (  fioRead (
        lFD, s_pTransferPtr = s_pMPEGData, s_MPEGDataSize = lSize
       ) != lSize
 ) {
  printf ( "test_mpeg: could not read file\n" );
  goto end;
 }  /* end if */

 fioClose ( lFD );
/* initialize DMAC (I have no idea what this code does as */
/* I'm not quite familiar with ps2sdk)                    */
 dma_initialize ();
 dma_channel_initialize ( DMA_CHANNEL_toIPU, NULL, DMA_FLAG_NORMAL );
 dma_channel_initialize ( DMA_CHANNEL_GIF,   NULL, DMA_FLAG_CHAIN  );
/* initialize graphics synthesizer */
 graph_initialize ();
 graph_set_mode ( GRAPH_MODE_AUTO, GRAPH_PSM_32, GRAPH_PSM_16S );
 graph_set_displaybuffer ( 0 );
 graph_set_drawbuffer ( 0 );
/* setup texture buffer address just after the framebuffer */
 lInfo.m_TexAddr = graph_get_width  () *
                   graph_get_height () *
                   (  graph_get_bpp () >> 3  );
/* clear screen */
 graph_set_clearbuffer ( 0, 0, 0 );
/* ps2sdk's code sets XYOFFSET and TEST registers to some */
/* "strange" values :). Let's set them to "ordinary" ones */
/* and disable all pixel tests                            */
 packet_allocate ( &lPck, 48 );
  packet_append_64 (  &lPck, GIF_SET_TAG( 2, 1, 0, 0, 0, 1 )  );
  packet_append_64 (  &lPck, 0x0E  );
  packet_append_64 (  &lPck, 0 );
  packet_append_64 (  &lPck, GIF_REG_TEST_1 );
  packet_append_64 (  &lPck, GIF_SET_XYOFFSET( 0, 0 )  );
  packet_append_64 (  &lPck, GIF_REG_XYOFFSET_1 );
 packet_send ( &lPck, DMA_CHANNEL_GIF, DMA_FLAG_NORMAL );
/* now it's time to initialize MPEG decoder (though it can be   */
/* initialized any time). Just make sure that DMA transfers     */
/* to and from IPU (and DRAM -> SPR) are not active, otherwise  */
/* unpredicted things will happen. Initialization code is also  */
/* allocating some memory using 'memalign' function and no      */
/* check is performed whether the allocation was successful or  */
/* not, so, before calling this make sure that at least WxHx4x3 */
/* bytes are avaliable for dynamic allocation (possibly using   */
/* ps2_sbrk ( 0 ) call) where W and H are picture dimensions in */
/* units of pixels.                                             */
 MPEG_Initialize ( SetDMA, NULL, InitCB, &lInfo, &lCurPTS );
/* during decoding scratchpad RAM from address 0x0000 to 0x3C00 */
/* is used by the decoder.                                      */
/* let's go                                                     */
 while ( 1 ) {
/* try decode picture into "lInfo.m_pData" area. It's allowed     */
/* to supply different area each time, just make sure that        */
/* there're no conflicts with data cache, as decoder doesn't do   */
/* anything to synchronize/flush/invalidate data cache.           */
/* RGB -> YUV colorspace conversion is pefromed automatically     */
/* using interrupt hahdler/semaphore, so, multithreaded           */
/* application can benefit from it. Usage of IPU and DMA channels */
/* to/from IPU and DRAM -> SPR is strictly forbidden during       */
/* decoding :).                                                   */
  if (  !MPEG_Picture ( lInfo.m_pData, &lPTS )  ) {
/* MPEG_Picture returns nonzero if the picture was successfully */
/* decoded. Zero return means one of the following:             */
/* - end of stream was detected (SetDMA function returned zero) */
/* - MPEG sequence end code (0x000001B7) was detected           */
/* this test just finishes in both cases                        */
   if ( lInfo.m_pInfo -> m_fEOF  ) break;
/* ...instead of 'break' we can continue to the next sequence...*/
/* ...but I'm too lazy to handle second call of 'InitCB' :D     */
   else break;
  }  /* end if */
/* now transfer decoded picture data into texture area of GS RAM */
  packet_send ( &lInfo.m_XFerPck, DMA_CHANNEL_GIF, DMA_FLAG_CHAIN );
/* wait for vsync 2 times (we have interlaced frame mode)  */
  graph_wait_vsync ();
  graph_wait_vsync ();
/* no need to wait for DMA transfer completion since vsyncs above */
/* have enough lattency...                                        */
/* ...and finally draw decoded picture...                         */
  packet_send ( &lInfo.m_DrawPck, DMA_CHANNEL_GIF, DMA_FLAG_NORMAL );
/* ...and go back for the next one */
 }  /* end while */
/* free memory and other resources */
 MPEG_Destroy ();
end:
 printf ( "test_mpeg: test finished\n" );
 return SleepThread (), 0;

}  /* end main */
/* This gets called by MPEG decoder to get data to decode.  */
/* It performs normal DMA data transfer to IPU and returns  */
/* nozero to indicate that data have been sent. Zero return */
/* indicates end-of-data condition, Amount of data per      */
/* transfer doesn't really matter, but it must be less than */
/* 1MB minus 16. Sample function uses 2048 bytes blocks.    */
/* Don't use source chain transfer as it will lead to       */
/* unpredictable results.                                   */
static int SetDMA ( void* apUserData ) {

 if ( s_pTransferPtr - s_pMPEGData >= s_MPEGDataSize ) return 0;

 dma_channel_send (
  DMA_CHANNEL_toIPU, s_pTransferPtr, 2048, DMA_FLAG_NORMAL
 );
 s_pTransferPtr += 2048;

 return 1;

}  /* end SetDMA */

static int inline GS_PowerOf2 ( int aVal ) {
 int i;
 for ( i = 0; ( 1 << i ) < aVal; ++i );
 return i;
}  /* end GS_PowerOf2 */
/* This gets called when sequence start header is detected in the     */
/* input bitstream. <apInfo> is filled by the decoder and callback    */
/* function initializes display process and other required stuff      */
/* based upon values provided in <apInfo> stucture. It should return  */
/* pointer to the data area where decoded picture (RGBA32, 16x16      */
/* blocks) will be stored. Pointer is supposed to be 16 byte aligned. */
/* It can be called several times (depending on number of video       */
/* sequences inside a bitstream). It is allowed to return the same    */
/* pointer each time, but the data area should be large enough to     */
/* accomodate the whole picture. <apParam> is just a user supplied    */
/* data (anything).  */
static void* InitCB ( void* apParam, MPEGSequenceInfo* apInfo ) {

 int          lDataSize = apInfo -> m_Width * apInfo -> m_Height * 4;
 char*        retVal    = ( char* )malloc ( lDataSize );
 InitCBParam* lpParam   = ( InitCBParam* )apParam;
 int          lMBW      = ( apInfo -> m_Width  ) >> 4;
 int          lMBH      = ( apInfo -> m_Height ) >> 4;
 int          lTBW      = ( apInfo -> m_Width + 63 ) >> 6;
 int          lTW       = GS_PowerOf2 ( apInfo -> m_Width  );
 int          lTH       = GS_PowerOf2 ( apInfo -> m_Height );
 int          lX, lY;
 char*        lpImg;

 lpParam -> m_TexAddr >>= 8;

 lpParam -> m_pData = lpImg = retVal;
 lpParam -> m_pInfo = apInfo;
 SyncDCache ( retVal, retVal + lDataSize );
/* This initializes picture transfer packet.      */
/* Decoded picture is a sequence of 16x16 pixels  */
/* 'subpictures' (macroblocks) and DMA controller */
/* will transfer them all at once using source    */
/* chain transfer mode.                           */
 packet_allocate (  &lpParam -> m_XFerPck, ( 10 + 12 * lMBW * lMBH ) << 3  );
 packet_append_64 (  &lpParam -> m_XFerPck, DMA_SET_TAG( 3, 0, DMA_TAG_CNT, 0, 0, 0 )  );
 packet_append_64 (  &lpParam -> m_XFerPck, 0  );
 packet_append_64 (  &lpParam -> m_XFerPck, GIF_SET_TAG( 2, 0, 0, 0, 0, 1 )  );
 packet_append_64 (  &lpParam -> m_XFerPck, 0x0E  );
 packet_append_64 (  &lpParam -> m_XFerPck, GIF_SET_TRXREG( 16, 16 )  );
 packet_append_64 (  &lpParam -> m_XFerPck, GIF_REG_TRXREG  );
 packet_append_64 (  &lpParam -> m_XFerPck, GIF_SET_BITBLTBUF( 0, 0, GRAPH_PSM_32, lpParam -> m_TexAddr, lTBW, GRAPH_PSM_32 )  );
 packet_append_64 (  &lpParam -> m_XFerPck, GIF_REG_BITBLTBUF  );

 for ( lY = 0; lY < apInfo -> m_Height; lY += 16 ) {
  for ( lX = 0; lX < apInfo -> m_Width; lX += 16, lpImg  += 1024 ) {
   packet_append_64 (  &lpParam -> m_XFerPck, DMA_SET_TAG( 4, 0, DMA_TAG_CNT, 0, 0, 0 )  );
   packet_append_64 (  &lpParam -> m_XFerPck, 0 );
   packet_append_64 (  &lpParam -> m_XFerPck, GIF_SET_TAG( 2, 0, 0, 0, 0, 1 )  );
   packet_append_64 (  &lpParam -> m_XFerPck, 0x0E  );
   packet_append_64 (  &lpParam -> m_XFerPck, GIF_SET_TRXPOS( 0, 0, lX, lY, 0 )  );
   packet_append_64 (  &lpParam -> m_XFerPck, GIF_REG_TRXPOS  );
   packet_append_64 (  &lpParam -> m_XFerPck, GIF_SET_TRXDIR( 0 )  );
   packet_append_64 (  &lpParam -> m_XFerPck, GIF_REG_TRXDIR  );
   packet_append_64 (  &lpParam -> m_XFerPck, GIF_SET_TAG( 64, 1, 0, 0, 2, 1 )  );
   packet_append_64 (  &lpParam -> m_XFerPck, 0  );
   packet_append_64 (  &lpParam -> m_XFerPck, DMA_SET_TAG( 64, 1, DMA_TAG_REF, 0, ( unsigned )lpImg, 0 )  );
   packet_append_64 (  &lpParam -> m_XFerPck, 0  );
  }  /* end for */
 }  /* end for */

 packet_append_64 (  &lpParam -> m_XFerPck, DMA_SET_TAG( 0, 0, DMA_TAG_END, 0, 0, 0 )  );
 packet_append_64 (  &lpParam -> m_XFerPck, 0  );
/* This initializes picture drawing packet. Just textrured sprite */
/* that occupies the whole screen (no aspect ratio is taken into  */
/* account for simplicity.                                        */
 packet_allocate (  &lpParam -> m_DrawPck, 112 );
 packet_append_64 (  &lpParam -> m_DrawPck, GIF_SET_TAG( 6, 1, 0, 0, 0, 1 )  );
 packet_append_64 (  &lpParam -> m_DrawPck, 0x0E  );
 packet_append_64 (  &lpParam -> m_DrawPck, GIF_SET_TEX0( lpParam -> m_TexAddr, lTBW, GRAPH_PSM_32, lTW, lTH, 1, 1, 0, 0, 0, 0, 0 )  );
 packet_append_64 (  &lpParam -> m_DrawPck, GIF_REG_TEX0_1 );
 packet_append_64 (  &lpParam -> m_DrawPck, GIF_SET_PRIM( 6, 0, 1, 0, 0, 0, 1, 0, 0 )  );
 packet_append_64 (  &lpParam -> m_DrawPck, GIF_REG_PRIM  );
 packet_append_64 (  &lpParam -> m_DrawPck, GIF_SET_UV( 0, 0 )  );
 packet_append_64 (  &lpParam -> m_DrawPck, GIF_REG_UV  );
 packet_append_64 (  &lpParam -> m_DrawPck, GIF_SET_XYZ( 0, 0, 0 )  );
 packet_append_64 (  &lpParam -> m_DrawPck, GIF_REG_XYZ2  );
 packet_append_64 (  &lpParam -> m_DrawPck, GIF_SET_UV( apInfo -> m_Width << 4, apInfo -> m_Height << 4 )  );
 packet_append_64 (  &lpParam -> m_DrawPck, GIF_REG_UV  );
 packet_append_64 (  &lpParam -> m_DrawPck, GIF_SET_XYZ(  graph_get_width () << 4, graph_get_height () << 4, 0 )  );
 packet_append_64 (  &lpParam -> m_DrawPck, GIF_REG_XYZ2  );

 return retVal;

}  /* end InitCB */

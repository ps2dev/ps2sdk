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

#ifndef __DMA_REGISTERS_H__
#define __DMA_REGISTERS_H__

 #include <tamtypes.h>

#ifdef __cplusplus
extern "C" {
#endif

 ///////////////////////////
 // DMA CONTROL REGISTERS //
 ///////////////////////////

 #define DMA_REG_CTRL		*(volatile u32 *)0x1000E000	// DMA Control Register
 #define DMA_REG_STAT		*(volatile u32 *)0x1000E010	// Interrupt Status Register
 #define DMA_REG_PCR		*(volatile u32 *)0x1000E020	// Priority Control Register
 #define DMA_REG_SQWC		*(volatile u32 *)0x1000E030	// Interleave Size Register
 #define DMA_REG_RBSR		*(volatile u32 *)0x1000E040	// Ring Buffer Size Register
 #define DMA_REG_RBOR		*(volatile u32 *)0x1000E050	// Ring Buffer Address Register
 #define DMA_REG_STADR		*(volatile u32 *)0x1000E060	// Stall Address Register
 #define DMA_REG_ENABLER	*(volatile u32 *)0x1000F520	// DMA Hold State Register
 #define DMA_REG_ENABLEW	*(volatile u32 *)0x1000F590	// DMA Hold Control Register

 #define DMA_SET_CTRL(A,B,C,D,E,F) \
  (u32)((A) & 0x00000001) <<  0 | (u32)((B) & 0x00000001) <<  1 | \
  (u32)((C) & 0x00000003) <<  2 | (u32)((D) & 0x00000003) <<  4 | \
  (u32)((E) & 0x00000003) <<  6 | (u32)((F) & 0x00000007) <<  8

 #define DMA_SET_STAT(A,B,C,D,E,F,G,H,I,J,K,L,M,N,O,P,Q,R,S,T,U,V,W,X,Y) \
  (u32)((A) & 0x00000001) <<  0 | (u32)((B) & 0x00000001) <<  1 | \
  (u32)((C) & 0x00000001) <<  2 | (u32)((D) & 0x00000001) <<  3 | \
  (u32)((E) & 0x00000001) <<  4 | (u32)((F) & 0x00000001) <<  5 | \
  (u32)((G) & 0x00000001) <<  6 | (u32)((H) & 0x00000001) <<  7 | \
  (u32)((I) & 0x00000001) <<  8 | (u32)((J) & 0x00000001) <<  9 | \
  (u32)((K) & 0x00000001) << 13 | (u32)((L) & 0x00000001) << 14 | \
  (u32)((M) & 0x00000001) << 15 | (u32)((N) & 0x00000001) << 16 | \
  (u32)((O) & 0x00000001) << 17 | (u32)((P) & 0x00000001) << 18 | \
  (u32)((Q) & 0x00000001) << 19 | (u32)((R) & 0x00000001) << 20 | \
  (u32)((S) & 0x00000001) << 21 | (u32)((T) & 0x00000001) << 22 | \
  (u32)((U) & 0x00000001) << 23 | (u32)((V) & 0x00000001) << 24 | \
  (u32)((W) & 0x00000001) << 25 | (u32)((X) & 0x00000001) << 29 | \
  (u32)((Y) & 0x00000001) << 30

 #define DMA_SET_PCR(A,B,C,D,E,F,G,H,I,J,K,L,M,N,O,P,Q,R,S,T,U) \
  (u32)((A) & 0x00000001) <<  0 | (u32)((B) & 0x00000001) <<  1 | \
  (u32)((C) & 0x00000001) <<  2 | (u32)((D) & 0x00000001) <<  3 | \
  (u32)((E) & 0x00000001) <<  4 | (u32)((F) & 0x00000001) <<  5 | \
  (u32)((G) & 0x00000001) <<  6 | (u32)((H) & 0x00000001) <<  7 | \
  (u32)((I) & 0x00000001) <<  8 | (u32)((J) & 0x00000001) <<  9 | \
  (u32)((K) & 0x00000001) << 16 | (u32)((L) & 0x00000001) << 17 | \
  (u32)((M) & 0x00000001) << 18 | (u32)((N) & 0x00000001) << 19 | \
  (u32)((O) & 0x00000001) << 20 | (u32)((P) & 0x00000001) << 21 | \
  (u32)((Q) & 0x00000001) << 22 | (u32)((R) & 0x00000001) << 23 | \
  (u32)((S) & 0x00000001) << 24 | (u32)((T) & 0x00000001) << 25 | \
  (u32)((U) & 0x00000001) << 31

 #define DMA_SET_SQWC(A,B) \
  (u32)((A) & 0x000000FF) <<  0 | (u32)((B) & 0x000000FF) << 16

 #define DMA_SET_RBOR(A) (u32)((A) & 0x00007FFF)

 #define DMA_SET_RBSR(A) (u32)((A) & 0x00007FFF)

 #define DMA_SET_STADR(A) (u32)((A) & 0x00007FFF)

 #define DMA_SET_ENABLEW(A) (u32)((A) & 0x00000001) << 16

 #define DMA_SET_ENABLER(A) (u32)((A) & 0x00000001) << 16

 ///////////////////////////
 // DMA CHANNEL REGISTERS //
 ///////////////////////////

 #define DMA_SET_CHCR(A,B,C,D,E,F,G) \
  (u32)((A) & 0x00000001) <<  0 | (u32)((B) & 0x00000003) <<  2 | \
  (u32)((C) & 0x00000003) <<  4 | (u32)((D) & 0x00000001) <<  6 | \
  (u32)((E) & 0x00000001) <<  7 | (u32)((F) & 0x00000001) <<  8 | \
  (u32)((G) & 0x0000FFFF) << 16

 #define DMA_SET_MADR(A,B) \
  (u32)((A) & 0x7FFFFFFF) <<  0 | (u32)((B) & 0x00000001) << 31

 #define DMA_SET_TADR(A,B) \
  (u32)((A) & 0x7FFFFFFF) <<  0 | (u32)((B) & 0x00000001) << 31

 #define DMA_SET_ASR0(A,B) \
  (u32)((A) & 0x7FFFFFFF) <<  0 | (u32)((B) & 0x00000001) << 31

 #define DMA_SET_ASR1(A,B) \
  (u32)((A) & 0x7FFFFFFF) <<  0 | (u32)((B) & 0x00000001) << 31

 #define DMA_SET_SADR(A) (u32)((A) & 0x00003FFF)

 #define DMA_SET_QWC(A) (u32)((A) & 0x0000FFFF)

 ///////////////////////
 // DMA TAG REGISTERS //
 ///////////////////////

 #define DMA_TAG_REFE	0x00
 #define DMA_TAG_CNT	0x01
 #define DMA_TAG_NEXT	0x02
 #define DMA_TAG_REF	0x03
 #define DMA_TAG_REFS	0x04
 #define DMA_TAG_CALL	0x05
 #define DMA_TAG_RET	0x06
 #define DMA_TAG_END	0x07

 #define DMA_SET_TAG(A,B,C,D,E,F) \
  (u64)((A) & 0x0000FFFF) <<  0 | (u64)((B) & 0x00000003) << 26 | \
  (u64)((C) & 0x00000007) << 28 | (u64)((D) & 0x00000001) << 31 | \
  (u64)((E) & 0x7FFFFFFF) << 32 | (u64)((F) & 0x00000001) << 63

#ifdef __cplusplus
}
#endif

#endif

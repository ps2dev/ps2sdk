#ifndef __DMA_TAGS_H__
#define __DMA_TAGS_H__

#include <tamtypes.h>

// T   = location of qwords
// D   = next dmatag to be read
// QWC = quadword count for dmatag

#define DMA_TAG_CNTS	0x00	//T=QWC D=QWC+1 MADR => STADR
#define DMA_TAG_REFE	0x00	//T=ADDR then END
#define DMA_TAG_CNT	0x01	//T=QWC D=QWC+1
#define DMA_TAG_NEXT	0x02	//T=QWC D=ADDR
#define DMA_TAG_REF	0x03	//D=D+1 T=ADDR
#define DMA_TAG_REFS	0x04	//.. + stall ctrl
#define DMA_TAG_CALL	0x05	//T=QWC D=ADDR QWC+1 => ASR0
#define DMA_TAG_RET	0x06	//T=QWC (ASR0 => D) if !ASR0 then END
#define DMA_TAG_END	0x07	//T=QWC then END

#define DMATAG(QWC,PCE,ID,IRQ,ADDR,SPR) \
	(u64)((QWC)  & 0x0000FFFF) <<  0 | (u64)((PCE) & 0x00000003) << 26 | \
	(u64)((ID)   & 0x00000007) << 28 | (u64)((IRQ) & 0x00000001) << 31 | \
	(u64)((ADDR) & 0x7FFFFFFF) << 32 | (u64)((SPR) & 0x00000001) << 63

#define PACK_DMATAG(Q,D0,W2,W3) \
	Q->dw[0] = (u64)(D0), \
	Q->sw[2] = (u32)(W2), \
	Q->sw[3] = (u32)(W3)

// Insert before qword block, and add dmatag after qword block to continue
#define DMATAG_CNT(Q,QWC,SPR,W2,W3) \
	Q->dw[0] = DMATAG(QWC,0,DMA_TAG_CNT,0,0,SPR), \
	Q->sw[2] = (u32)(W2), \
	Q->sw[3] = (u32)(W3)

// Insert before qword block, and add dmatag after qword block to continue
#define DMATAG_CNTS(Q,QWC,SPR,W2,W3) \
	Q->dw[0] = DMATAG(QWC,0,DMA_TAG_CNTS,0,0,SPR), \
	Q->sw[2] = (u32)(W2), \
	Q->sw[3] = (u32)(W3)

// Insert before qword block, and add dmatag at addr to continue
#define DMATAG_NEXT(Q,QWC,ADDR,SPR,W2,W3), \
	Q->dw[0] = DMATAG(QWC,0,DMA_TAG_NEXT,0,ADDR,SPR), \
	Q->sw[2] = (u32)(W2), \
	Q->sw[3] = (u32)(W3)

// Insert before final qword block, or by itself to end transfer
#define DMATAG_END(Q,QWC,SPR,W2,W3) \
	Q->dw[0] = DMATAG(QWC,0,DMA_TAG_END,0,0,SPR), \
	Q->sw[2] = (u32)(W2), \
	Q->sw[3] = (u32)(W3)

// Insert before qword block, and add dmatag at qword after qwc to be saved (up to 2)
#define DMATAG_CALL(Q,QWC,ADDR,SPR,W2,W3) \
	Q->dw[0] = DMATAG(QWC,0,DMA_TAG_CALL,0,ADDR,SPR), \
	Q->sw[2] = (u32)(W2), \
	Q->sw[3] = (u32)(W3)

// Insert before qword block, and continues with saved dmatag
#define DMATAG_RET(Q,QWC,SPR,W2,W3) \
	Q->dw[0] = DMATAG(QWC,0,DMA_TAG_RET,0,0,SPR), \
	Q->sw[2] = (u32)(W2), \
	Q->sw[3] = (u32)(W3)

// Insert anywhere, qwc at addr is sent, reads next qword to continue
#define DMATAG_REF(Q,QWC,ADDR,SPR,W2,W3) \
	Q->dw[0] = DMATAG(QWC,0,DMA_TAG_REF,0,ADDR,SPR), \
	Q->sw[2] = (u32)(W2), \
	Q->sw[3] = (u32)(W3)

// Insert anywhere, qwc at addr is sent, reads next qword to continue
#define DMATAG_REFS(Q,QWC,ADDR,SPR,W2,W3) \
	Q->dw[0] = DMATAG(QWC,0,DMA_TAG_REFS,0,ADDR,SPR), \
	Q->sw[2] = (u32)(W2), \
	Q->sw[3] = (u32)(W3)

// Insert anywhere, qwc at addr is sent, then ends transfer
#define DMATAG_REFE(Q,QWC,ADDR,SPR,W2,W3) \
	Q->dw[0] = DMATAG(qwc, 0, DMA_TAG_REFE, 0, addr, spr), \
	Q->sw[2] = (u32)(W2), \
	Q->sw[3] = (u32)(W3)

#endif /*__DMA_TAGS_H__*/

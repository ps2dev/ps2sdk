/**
 * @file
 * GIF Tags
 */

#ifndef __GIF_TAGS_H__
#define __GIF_TAGS_H__

#include <tamtypes.h>

/** Not sure if this is correct... */
#define GIF_BLOCK_SIZE			0x7FFF

/** Enable PRIM field output */
#define GIF_PRE_DISABLE			0x00
/** Disable PRIM field output */
#define GIF_PRE_ENABLE			0x01

/** Point primitive */
#define GIF_PRIM_POINT			0x00
/** Line primitive */
#define GIF_PRIM_LINE			0x01
/** Line strip primitive */
#define GIF_PRIM_LINE_STRIP		0x02
/** Triangle primitive */
#define GIF_PRIM_TRIANGLE		0x03
/** Triangle strip primitive */
#define GIF_PRIM_TRIANGLE_STRIP	0x04
/** Triangle fan primitive */
#define GIF_PRIM_TRIANGLE_FAN	0x05
/** Sprite primitive */
#define GIF_PRIM_SPRITE			0x06

/** Packed GIF packet */
#define GIF_FLG_PACKED			0x00
/** Reglist GIF packet */
#define GIF_FLG_REGLIST			0x01
/** Image GIF packet */
#define GIF_FLG_IMAGE			0x02

/** Drawing primitive setting. */
#define GIF_REG_PRIM			0x00
/** Vertex color setting. */
#define GIF_REG_RGBAQ			0x01
/** Specification of vertex texture coordinates. */
#define GIF_REG_ST				0x02
/** Specification of vertex texture coordinates. */
#define GIF_REG_UV				0x03
/** Setting for vertex coordinate values. */
#define GIF_REG_XYZF2			0x04
/** Setting for vertex coordinate values. */
#define GIF_REG_XYZ2			0x05
/** Texture information setting. */
#define GIF_REG_TEX0			0x06
/** Texture information setting. (Context 1) */
#define GIF_REG_TEX0_1			0x06
/** Texture information setting. (Context 2) */
#define GIF_REG_TEX0_2			0x07
/** Texture wrap mode. */
#define GIF_REG_CLAMP			0x08
/** Texture wrap mode. (Context 1) */
#define GIF_REG_CLAMP_1			0x08
/** Texture wrap mode. (Context 2) */
#define GIF_REG_CLAMP_2			0x09
/** Vertex fog value setting. */
#define GIF_REG_FOG				0x0A
/** Setting for vertex coordinate values. (Without Drawing Kick) */
#define GIF_REG_XYZF3			0x0C
/** Setting for vertex coordinate values. (Without Drawing Kick) */
#define GIF_REG_XYZ3			0x0D
/** GIFtag Address+Data */
#define GIF_REG_AD				0x0E
/** GIFtag No Operation */
#define GIF_REG_NOP				0x0F

#define PACK_GIFTAG(Q,D0,D1) \
	Q->dw[0] = (u64)(D0), \
	Q->dw[1] = (u64)(D1)

#define GIF_SET_TAG(NLOOP,EOP,PRE,PRIM,FLG,NREG) \
	(u64)((NLOOP) & 0x00007FFF) <<  0 | (u64)((EOP)  & 0x00000001) << 15 | \
	(u64)((PRE)   & 0x00000001) << 46 | (u64)((PRIM) & 0x000007FF) << 47 | \
	(u64)((FLG)   & 0x00000003) << 58 | (u64)((NREG) & 0x0000000F) << 60

#define GIF_SET_PRIM(PRIM,IIP,TME,FGE,ABE,AA1,FST,CTXT,FIX) \
	(u64)((PRIM) & 0x00000007) <<  0 | (u64)((IIP) & 0x00000001) <<  3 | \
	(u64)((TME)  & 0x00000001) <<  4 | (u64)((FGE) & 0x00000001) <<  5 | \
	(u64)((ABE)  & 0x00000001) <<  6 | (u64)((AA1) & 0x00000001) <<  7 | \
	(u64)((FST)  & 0x00000001) <<  8 | (u64)((CTXT) & 0x00000001) <<  9 | \
	(u64)((FIX)  & 0x00000001) << 10

#define GIF_SET_RGBAQ(R,G,B,A,Q) \
	(u64)((R) & 0x000000FF) <<  0 | (u64)((G) & 0x000000FF) <<  8 | \
	(u64)((B) & 0x000000FF) << 16 | (u64)((A) & 0x000000FF) << 24 | \
	(u64)((Q) & 0xFFFFFFFF) << 32

#define GIF_SET_ST(S,T) \
	(u64)((S) & 0xFFFFFFFF) <<  0 | (u64)((T) & 0xFFFFFFFF) << 32

#define GIF_SET_UV(U,V) \
	(u64)((U) & 0x00003FFF) <<  0 | (u64)((V) & 0x00003FFF) << 16

#define GIF_SET_XYZ(X,Y,Z) \
	(u64)((X) & 0x0000FFFF) <<  0 | (u64)((Y) & 0x0000FFFF) << 16 | \
	(u64)((Z) & 0xFFFFFFFF) << 32

#define GIF_SET_XYZF(X,Y,Z,F) \
	(u64)((X) & 0x0000FFFF) <<  0 | (u64)((Y) & 0x0000FFFF) << 16 | \
	(u64)((Z) & 0x00FFFFFF) << 32 | (u64)((F) & 0x000000FF) << 56

#define GIF_SET_TEX0(TBA,TBW,PSM,TW,TH,TCC,TFNCT,CBA,CPSM,CSM,CSA,CLD) \
	(u64)((TBA)   & 0x00003FFF) <<  0 | (u64)((TBW) & 0x0000003F) << 14 | \
	(u64)((PSM)   & 0x0000003F) << 20 | (u64)((TW)  & 0x0000000F) << 26 | \
	(u64)((TH)    & 0x0000000F) << 30 | (u64)((TCC) & 0x00000001) << 34 | \
	(u64)((TFNCT) & 0x00000003) << 35 | (u64)((CBA) & 0x00003FFF) << 37 | \
	(u64)((CPSM)  & 0x0000000F) << 51 | (u64)((CSM) & 0x00000001) << 55 | \
	(u64)((CSA)   & 0x0000001F) << 56 | (u64)((CLD) & 0x00000007) << 61

#define GIF_SET_CLAMP(WMS,WMT,MINU,MAXU,MINV,MAXV) \
	(u64)((WMS)  & 0x00000003) <<  0 | (u64)((WMT)  & 0x00000003) <<  2 | \
	(u64)((MINU) & 0x000003FF) <<  4 | (u64)((MAXU) & 0x000003FF) << 14 | \
	(u64)((MINV) & 0x000003FF) << 24 | (u64)((MAXV) & 0x000003FF) << 34

#define GIF_SET_FOG(FOG) (u64)((FOG) & 0x000000FF) << 56

#endif /* __GIFTAGS_H__ */

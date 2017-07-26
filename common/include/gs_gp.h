/**
 * @file
 * GS General Purpose Registers
 */

#ifndef __GS_GP_H__
#define __GS_GP_H__

#include <tamtypes.h>

// General Purpose Registers
/** Drawing primitive setting. */
#define GS_REG_PRIM			0x00
/** Vertex color setting. */
#define GS_REG_RGBAQ		0x01
/** Specification of vertex texture coordinates. */
#define GS_REG_ST			0x02
/** Specification of vertex texture coordinates. */
#define GS_REG_UV			0x03
/** Setting for vertex coordinate values. */
#define GS_REG_XYZF2		0x04
/** Setting for vertex coordinate values. */
#define GS_REG_XYZ2			0x05
/** Texture information setting. */
#define GS_REG_TEX0			0x06
/** Texture information setting. (Context 1) */
#define GS_REG_TEX0_1		0x06
/** Texture information setting. (Context 2) */
#define GS_REG_TEX0_2		0x07
/** Texture wrap mode. */
#define GS_REG_CLAMP		0x08
/** Texture wrap mode. (Context 1) */
#define GS_REG_CLAMP_1		0x08
/** Texture wrap mode. (Context 2) */
#define GS_REG_CLAMP_2		0x09
/** Vertex fog value setting. */
#define GS_REG_FOG			0x0A
/** Setting for vertex coordinate values. (Without Drawing Kick) */
#define GS_REG_XYZF3		0x0C
/** Setting for vertex coordinate values. (Without Drawing Kick) */
#define GS_REG_XYZ3			0x0D
/** Texture information setting. */
#define GS_REG_TEX1			0x14
/** Texture information setting. (Context 1) */
#define GS_REG_TEX1_1		0x14
/** Texture information setting. (Context 2) */
#define GS_REG_TEX1_2		0x15
/** Texture information setting. */
#define GS_REG_TEX2			0x16
/** Texture information setting. (Context 1) */
#define GS_REG_TEX2_1		0x16
/** Texture information setting. (Context 2) */
#define GS_REG_TEX2_2		0x17
/** Offset value setting. */
#define GS_REG_XYOFFSET		0x18
/** Offset value setting. (Context 1) */
#define GS_REG_XYOFFSET_1	0x18
/** Offset value setting. (Context 2) */
#define GS_REG_XYOFFSET_2	0x19
/** Specification of primitive attribute setting method. */
#define GS_REG_PRMODECONT	0x1A
/** Setting for attributes of drawing primitives. */
#define GS_REG_PRMODE		0x1B
/** Clut position specification. */
#define GS_REG_TEXCLUT		0x1C
/** Raster address mask setting. */
#define GS_REG_SCANMSK		0x22
/** Mipmap information setting for levels 1 - 3. */
#define GS_REG_MIPTBP1		0x34
/** Mipmap information setting for levels 1 - 3. (Context 1) */
#define GS_REG_MIPTBP1_1	0x34
/** Mipmap information setting for levels 1 - 3. (Context 2) */
#define GS_REG_MIPTBP1_2	0x35
/** Mipmap information setting for levels 4 - 6. */
#define GS_REG_MIPTBP2		0x36
/** Mipmap information setting for levels 4 - 6. (Context 1) */
#define GS_REG_MIPTBP2_1	0x36
/** Mipmap information setting for levels 4 - 6. (Context 2) */
#define GS_REG_MIPTBP2_2	0x37
/** Texture alpha value setting. */
#define GS_REG_TEXA			0x3B
/** Distant fog color setting. */
#define GS_REG_FOGCOL		0x3D
/** Texture page buffer disabling. */
#define GS_REG_TEXFLUSH		0x3F
/** Setting for scissoring area. */
#define GS_REG_SCISSOR		0x40
/** Setting for scissoring area. (Context 1) */
#define GS_REG_SCISSOR_1	0x40
/** Setting for scissoring area. (Context 2) */
#define GS_REG_SCISSOR_2	0x41
/** Alpha blending setting. */
#define GS_REG_ALPHA		0x42
/** Alpha blending setting. (Context 1) */
#define GS_REG_ALPHA_1		0x42
/** Alpha blending setting. (Context 2) */
#define GS_REG_ALPHA_2		0x43
/** Dither matrix setting. */
#define GS_REG_DIMX			0x44
/** Dither control. */
#define GS_REG_DTHE			0x45
/** Color clamp control. */
#define GS_REG_COLCLAMP		0x46
/** Pixel test control. */
#define GS_REG_TEST			0x47
/** Pixel test control. (Context 1) */
#define GS_REG_TEST_1		0x47
/** Pixel test control. (Context 2) */
#define GS_REG_TEST_2		0x48
/** Alpha blending control in units of pixels. */
#define GS_REG_PABE			0x49
/** Alpha correction value. */
#define GS_REG_FBA			0x4A
/** Alpha correction value. (Context 1) */
#define GS_REG_FBA_1		0x4A
/** Alpha correction value. (Context 2) */
#define GS_REG_FBA_2		0x4B
/** Frame buffer setting. */
#define GS_REG_FRAME		0x4C
/** Frame buffer setting. (Context 1) */
#define GS_REG_FRAME_1		0x4C
/** Frame buffer setting. (Context 2) */
#define GS_REG_FRAME_2		0x4D
/** Z-Buffer setting. */
#define GS_REG_ZBUF			0x4E
/** Z-Buffer setting. (Context 1) */
#define GS_REG_ZBUF_1		0x4E
/** Z-Buffer setting. (Context 2) */
#define GS_REG_ZBUF_2		0x4F
/** Setting for transmissions between buffers. */
#define GS_REG_BITBLTBUF	0x50
/** Specification of transmission area in buffers. */
#define GS_REG_TRXPOS		0x51
/** Specification of transmission area in buffers. */
#define GS_REG_TRXREG		0x52
/** Activation of transmission area in buffers. */
#define GS_REG_TRXDIR		0x53
/** Data port for transmission between buffers. */
#define GS_REG_HWREG		0x54
/** Signal event occurence request. */
#define GS_REG_SIGNAL		0x60
/** Finish event occurence request. */
#define GS_REG_FINISH		0x61
/** Label event occurence request. */
#define GS_REG_LABEL		0x62
/** GS No Operation */
#define GS_REG_NOP			0x7F

// GS Primitive types
/** Point primitive */
#define GS_PRIM_POINT			0x00
/** Line primitive */
#define GS_PRIM_LINE			0x01
/** Line strip primitive */
#define GS_PRIM_LINE_STRIP		0x02
/** Triangle primitive */
#define GS_PRIM_TRIANGLE		0x03
/** Triangle strip primitive */
#define GS_PRIM_TRIANGLE_STRIP	0x04
/** Triangle fan primitive */
#define GS_PRIM_TRIANGLE_FAN	0x05
/** Sprite primitive */
#define GS_PRIM_SPRITE			0x06


// For generic enabling or disabling
#define GS_DISABLE				0x00
#define GS_ENABLE				0x01

#define GS_SET_ALPHA(A,B,C,D,ALPHA) \
	(u64)((A)     & 0x00000003) <<  0 | (u64)((B) & 0x00000003) <<  2 | \
	(u64)((C)     & 0x00000003) <<  4 | (u64)((D) & 0x00000003) <<  6 | \
	(u64)((ALPHA) & 0x000000FF) << 32

#define GS_SET_BITBLTBUF(SBA,SBW,SPSM,DBA,DBW,DPSM) \
	(u64)((SBA)  & 0x00003FFF) <<  0 | (u64)((SBW)  & 0x0000003F) << 16 | \
	(u64)((SPSM) & 0x0000003F) << 24 | (u64)((DBA)  & 0x00003FFF) << 32 | \
	(u64)((DBW)  & 0x0000003F) << 48 | (u64)((DPSM) & 0x0000003F) << 56

#define GS_SET_CLAMP(WMS,WMT,MINU,MAXU,MINV,MAXV) \
	(u64)((WMS)  & 0x00000003) <<  0 | (u64)((WMT)  & 0x00000003) <<  2 | \
	(u64)((MINU) & 0x000003FF) <<  4 | (u64)((MAXU) & 0x000003FF) << 14 | \
	(u64)((MINV) & 0x000003FF) << 24 | (u64)((MAXV) & 0x000003FF) << 34

#define GS_SET_COLCLAMP(CLAMP) (u64)((CLAMP) & 0x00000001)

#define GS_SET_DIMX(D00,D01,D02,D03,D10,D11,D12,D13,D20,D21,D22,D23,D30,D31,D32,D33) \
	(u64)((D00) & 0x00000003) <<  0 | (u64)((D01) & 0x00000003) <<  4 | \
	(u64)((D02) & 0x00000003) <<  8 | (u64)((D03) & 0x00000003) << 12 | \
	(u64)((D10) & 0x00000003) << 16 | (u64)((D11) & 0x00000003) << 20 | \
	(u64)((D12) & 0x00000003) << 24 | (u64)((D13) & 0x00000003) << 28 | \
	(u64)((D20) & 0x00000003) << 32 | (u64)((D21) & 0x00000003) << 36 | \
	(u64)((D22) & 0x00000003) << 40 | (u64)((D23) & 0x00000003) << 44 | \
	(u64)((D30) & 0x00000003) << 48 | (u64)((D31) & 0x00000003) << 52 | \
	(u64)((D32) & 0x00000003) << 56 | (u64)((D33) & 0x00000003) << 60

#define GS_SET_DTHE(ENABLE) (u64)((ENABLE) & 0x00000001)

#define GS_SET_FBA(ALPHA) (u64)((ALPHA) & 0x00000001)

#define GS_SET_FINISH(A) (u64)((A) & 0xFFFFFFFF)

#define GS_SET_FOG(FOG) (u64)((FOG) & 0x000000FF) << 56

#define GS_SET_FOGCOL(R,G,B) \
	(u64)((R) & 0x000000FF) <<  0 | (u64)((G) & 0x000000FF) <<  8 | \
	(u64)((B) & 0x000000FF) << 16

#define GS_SET_FRAME(FBA,FBW,PSM,FMSK) \
	(u64)((FBA) & 0x000001FF) <<  0 | (u64)((FBW)  & 0x0000003F) << 16 | \
	(u64)((PSM) & 0x0000003F) << 24 | (u64)((FMSK) & 0xFFFFFFFF) << 32

// PSMCT16 FMSK for GS_SET_FRAME
#define GS_SET_FMSK16(R,G,B,A) \
	(u32)((R) & 0x0000001F) <<  3 | (u32)((G) & 0x0000001F) << 11 | \
	(u32)((G) & 0x0000001F) << 19 | (u32)((A) & 0x00000001) << 31

#define GS_SET_HWREG(A) (u64)((A) & 0xFFFFFFFFFFFFFFFF)

#define GS_SET_LABEL(ID,MSK) \
	(u64)((ID) & 0xFFFFFFFF) <<  0 | (u64)((MSK) & 0xFFFFFFFF) << 32

#define GS_SET_MIPTBP1(TBA1,TBW1,TBA2,TBW2,TBA3,TBW3) \
	(u64)((TBA1) & 0x000003FF) <<  0 | (u64)((TBW1) & 0x0000003F) << 14 | \
	(u64)((TBA2) & 0x000003FF) << 20 | (u64)((TBW2) & 0x0000003F) << 34 | \
	(u64)((TBA3) & 0x000003FF) << 40 | (u64)((TBW3) & 0x0000003F) << 54

#define GS_SET_MIPTBP2(TBA4,TBW4,TBA5,TBW5,TBA6,TBW6) \
	(u64)((TBA4) & 0x000003FF) <<  0 | (u64)((TBW4) & 0x0000003F) << 14 | \
	(u64)((TBA5) & 0x000003FF) << 20 | (u64)((TBW5) & 0x0000003F) << 34 | \
	(u64)((TBA6) & 0x000003FF) << 40 | (u64)((TBW6) & 0x0000003F) << 54

#define GS_SET_NOP(A) (u64)((A) & 0xFFFFFFFF)

#define GS_SET_PABE(ENABLE) (u64)((ENABLE) & 0x00000001)

#define GS_SET_PRIM(PRIM,IIP,TME,FGE,ABE,AA1,FST,CTXT,FIX) \
	(u64)((PRIM) & 0x00000007) <<  0 | (u64)((IIP)  & 0x00000001) <<  3 | \
	(u64)((TME)  & 0x00000001) <<  4 | (u64)((FGE)  & 0x00000001) <<  5 | \
	(u64)((ABE)  & 0x00000001) <<  6 | (u64)((AA1)  & 0x00000001) <<  7 | \
	(u64)((FST)  & 0x00000001) <<  8 | (u64)((CTXT) & 0x00000001) <<  9 | \
	(u64)((FIX)  & 0x00000001) << 10

#define GS_SET_PRMODE(IIP,TME,FGE,ABE,AA1,FST,CTXT,FIX) \
	(u64)((IIP)  & 0x00000001) <<  3 | (u64)((TME) & 0x00000001) <<  4 | \
	(u64)((FGE)  & 0x00000001) <<  5 | (u64)((ABE) & 0x00000001) <<  6 | \
	(u64)((AA1)  & 0x00000001) <<  7 | (u64)((FST) & 0x00000001) <<  8 | \
	(u64)((CTXT) & 0x00000001) <<  9 | (u64)((FIX) & 0x00000001) << 10

#define GS_SET_PRMODECONT(CTRL) (u64)((CTRL) & 0x00000001)

#define GS_SET_RGBAQ(R,G,B,A,Q) \
	(u64)((R) & 0x000000FF) <<  0 | (u64)((G) & 0x000000FF) <<  8 | \
	(u64)((B) & 0x000000FF) << 16 | (u64)((A) & 0x000000FF) << 24 | \
	(u64)((Q) & 0xFFFFFFFF) << 32

#define GS_SET_SCANMSK(MSK) (u64)((MSK) & 0x00000003)

#define GS_SET_SCISSOR(X0,X1,Y0,Y1) \
	(u64)((X0) & 0x000007FF) <<  0 | (u64)((X1) & 0x000007FF) << 16 | \
	(u64)((Y0) & 0x000007FF) << 32 | (u64)((Y1) & 0x000007FF) << 48

#define GS_SET_SIGNAL(ID,MSK) \
	(u64)((ID) & 0xFFFFFFFF) <<  0 | (u64)((MSK) & 0xFFFFFFFF) << 32

#define GS_SET_ST(S,T) \
	(u64)((S) & 0xFFFFFFFF) <<  0 | (u64)((T) & 0xFFFFFFFF) << 32

#define GS_SET_TEST(ATEN,ATMETH,ATREF,ATFAIL,DATEN,DATMD,ZTEN,ZTMETH) \
	(u64)((ATEN)  & 0x00000001) <<  0 | (u64)((ATMETH) & 0x00000007) <<  1 | \
	(u64)((ATREF) & 0x000000FF) <<  4 | (u64)((ATFAIL) & 0x00000003) << 12 | \
	(u64)((DATEN) & 0x00000001) << 14 | (u64)((DATMD)  & 0x00000001) << 15 | \
	(u64)((ZTEN)  & 0x00000001) << 16 | (u64)((ZTMETH) & 0x00000003) << 17

#define GS_SET_TEX0_SMALL(TBA,TBW,PSM,TW,TH,TCC,TFNCT) \
	(u64)((TBA)   & 0x00003FFF) <<  0 | (u64)((TBW) & 0x0000003F) << 14 | \
	(u64)((PSM)   & 0x0000003F) << 20 | (u64)((TW)  & 0x0000000F) << 26 | \
	(u64)((TH)    & 0x0000000F) << 30 | (u64)((TCC) & 0x00000001) << 34 | \
	(u64)((TFNCT) & 0x00000003) << 35

#define GS_SET_TEX0(TBA,TBW,PSM,TW,TH,TCC,TFNCT,CBA,CPSM,CSM,CSA,CLD) \
	(u64)((TBA)   & 0x00003FFF) <<  0 | (u64)((TBW) & 0x0000003F) << 14 | \
	(u64)((PSM)   & 0x0000003F) << 20 | (u64)((TW)  & 0x0000000F) << 26 | \
	(u64)((TH)    & 0x0000000F) << 30 | (u64)((TCC) & 0x00000001) << 34 | \
	(u64)((TFNCT) & 0x00000003) << 35 | (u64)((CBA) & 0x00003FFF) << 37 | \
	(u64)((CPSM)  & 0x0000000F) << 51 | (u64)((CSM) & 0x00000001) << 55 | \
	(u64)((CSA)   & 0x0000001F) << 56 | (u64)((CLD) & 0x00000007) << 61

#define GS_SET_TEX1(LCM,MXL,MMAG,MMIN,MTBA,L,K) \
	(u64)((LCM)  & 0x00000001) <<  0 | (u64)((MXL)  & 0x00000007) <<  2 | \
	(u64)((MMAG) & 0x00000001) <<  5 | (u64)((MMIN) & 0x00000007) <<  6 | \
	(u64)((MTBA) & 0x00000001) <<  9 | (u64)((L)    & 0x00000003) << 19 | \
	(u64)((K)    & 0x00000FFF) << 32

#define GS_SET_TEX2(PSM,CBA,CPSM,CSM,CSA,CLD) \
	(u64)((PSM)  & 0x0000003F) << 20 | (u64)((CBA) & 0x00003FFF) << 37 | \
	(u64)((CPSM) & 0x0000000F) << 51 | (u64)((CSM) & 0x00000001) << 55 | \
	(u64)((CSA)  & 0x0000001F) << 56 | (u64)((CLD) & 0x00000007) << 61

#define GS_SET_TEXA(A0,AM,A1) \
	(u64)((A0) & 0x000000FF) <<  0 | (u64)((AM) & 0x00000001) << 15 | \
	(u64)((A1) & 0x000000FF) << 32

#define GS_SET_TEXCLUT(CBW,CU,CV) \
	(u64)((CBW) & 0x0000003F) <<  0 | (u64)((CU) & 0x0000003F) <<  6 | \
	(u64)((CV)  & 0x00000FFF) << 12

#define GS_SET_TRXDIR(DIR) (u64)((DIR) & 0x00000003)

#define GS_SET_TEXFLUSH(A) (u64)((A) & 0xFFFFFFFF)

#define GS_SET_TRXPOS(SX,SY,DX,DY,DIR) \
	(u64)((SX)  & 0x000007FF) <<  0 | (u64)((SY) & 0x000007FF) << 16 | \
	(u64)((DX)  & 0x000007FF) << 32 | (u64)((DY) & 0x000007FF) << 48 | \
	(u64)((DIR) & 0x00000003) << 59

#define GS_SET_TRXREG(W,H) \
	(u64)((W) & 0x00000FFF) <<  0 | (u64)((H) & 0x00000FFF) << 32

#define GS_SET_UV(U,V) \
	(u64)((U) & 0x00003FFF) <<  0 | (u64)((V) & 0x00003FFF) << 16

#define GS_SET_XYOFFSET(X,Y) \
	(u64)((X) & 0x0000FFFF) <<  0 | (u64)((Y) & 0x0000FFFF) << 32

#define GS_SET_XYZ(X,Y,Z) \
	(u64)((X) & 0x0000FFFF) <<  0 | (u64)((Y) & 0x0000FFFF) << 16 | \
	(u64)((Z) & 0xFFFFFFFF) << 32

#define GS_SET_XYZF(X,Y,Z,F) \
	(u64)((X) & 0x0000FFFF) <<  0 | (u64)((Y) & 0x0000FFFF) << 16 | \
	(u64)((Z) & 0x00FFFFFF) << 32 | (u64)((F) & 0x000000FF) << 56

#define GS_SET_ZBUF(ZBA,ZSM,ZMSK) \
	(u64)((ZBA) & 0x000001FF) <<  0 | (u64)((ZSM) & 0x0000000F) << 24 | \
	(u64)((ZMSK) & 0x00000001) << 32

#endif /* __GS_GP_H__ */

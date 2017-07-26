/**
 * @file
 * GS Privileged Registers
 */

#ifndef __GS_PRIVILEGED_H__
#define __GS_PRIVILEGED_H__

#include <tamtypes.h>

// These are the privileged GS registers mapped to main ram
// These are modified directly without the use of dma

/** PCRTC Mode Setting */
#define GS_REG_PMODE		(volatile u64 *)0x12000000
/** VHP,VCKSEL,SLCK2,NVCK,CLKSEL,PEVS,PEHS,PVS,PHS,GCONT,SPML,PCK2,XPCK,SINT,PRST,EX,CMOD,SLCK,T1248,LC,RC */
#define GS_REG_SMODE1		(volatile u64 *)0x12000010
/** Setting For Modes Related to Video Synchronization */
#define GS_REG_SMODE2		(volatile u64 *)0x12000020
/** DRAM Refresh Settings */
#define GS_REG_SRFSH		(volatile u64 *)0x12000030
/** HS,HSVS,HSEQ,HBP,HFP */
#define GS_REG_SYNCH1		(volatile u64 *)0x12000040
/** HB,HF */
#define GS_REG_SYNCH2		(volatile u64 *)0x12000050
/** VS,VDP,VBPE,VBP,VFPE,VFP */
#define GS_REG_SYNCHV		(volatile u64 *)0x12000060
/** Setting For Rectangular Area Read Output Circuit 1 */
#define GS_REG_DISPFB1		(volatile u64 *)0x12000070
/** Setting For Rectangular Area Read Output Circuit 1 */
#define GS_REG_DISPLAY1	(volatile u64 *)0x12000080
/** Setting For Rectangular Area Read Output Circuit 2 */
#define GS_REG_DISPFB2		(volatile u64 *)0x12000090
/** Setting For Rectangular Area Read Output Circuit 2 */
#define GS_REG_DISPLAY2	(volatile u64 *)0x120000A0
/** Setting For Feedback Buffer Write Buffer */
#define GS_REG_EXTBUF		(volatile u64 *)0x120000B0
/** Feedback Write Setting */
#define GS_REG_EXTDATA		(volatile u64 *)0x120000C0
/** Feedback Write Function Control */
#define GS_REG_EXTWRITE	(volatile u64 *)0x120000D0
/** Background Color Setting */
#define GS_REG_BGCOLOR		(volatile u64 *)0x120000E0
/** System Status */
#define GS_REG_CSR			(volatile u64 *)0x12001000
/** Interrupt Mask Control */
#define GS_REG_IMR			(volatile u64 *)0x12001010
/** Host I/F Bus Switching */
#define GS_REG_BUSDIR		(volatile u64 *)0x12001040
/** Signal ID Value Read */
#define GS_REG_SIGLBLID	(volatile u64 *)0x12001080

#define GS_SET_BGCOLOR(R,G,B) \
 ((u64)((R) & 0x000000FF) <<  0 | (u64)((G) & 0x000000FF) <<  8 | \
 (u64)((B) & 0x000000FF) << 16)

#define GS_SET_BUSDIR(DIR) ((u64)((DIR) & 0x00000001))

// pcsx2's source defines two more regs as ZERO1 and ZERO2
// probably need to be set 0
#define GS_SET_CSR(SIGNAL,FINISH,HSINT,VSINT,EDWINT,FLUSH,RESET,NFIELD,FIELD,FIFO,REV,ID) \
 ((u64)((SIGNAL)  & 0x00000001) <<  0 | (u64)((FINISH) & 0x00000001) <<  1 | \
 (u64)((HSINT)   & 0x00000001) <<  2 | (u64)((VSINT)  & 0x00000001) <<  3 | \
 (u64)((EDWINT)  & 0x00000001) <<  4 | (u64)((0)      & 0x00000001) <<  5 | \
 (u64)((0)       & 0x00000001) <<  6 | (u64)((FLUSH)  & 0x00000001) <<  8 | \
 (u64)((RESET)   & 0x00000001) <<  9 | (u64)((NFIELD) & 0x00000001) << 12 | \
 (u64)((FIELD)   & 0x00000001) << 13 | (u64)((FIFO)   & 0x00000003) << 14 | \
 (u64)((REV)     & 0x000000FF) << 16 | (u64)((ID)     & 0x000000FF) << 24)

#define GS_SET_DISPFB(FBP,FBW,PSM,DBX,DBY) \
 ((u64)((FBP) & 0x000001FF) <<  0 | (u64)((FBW) & 0x0000003F) <<  9 | \
 (u64)((PSM) & 0x0000001F) << 15 | (u64)((DBX) & 0x000007FF) << 32 | \
 (u64)((DBY) & 0x000007FF) << 43)

#define GS_SET_DISPLAY(DX,DY,MAGH,MAGV,DW,DH) \
 ((u64)((DX)   & 0x00000FFF) <<  0 | (u64)((DY)   & 0x000007FF) << 12 | \
 (u64)((MAGH) & 0x0000000F) << 23 | (u64)((MAGV) & 0x00000003) << 27 | \
 (u64)((DW)   & 0x00000FFF) << 32 | (u64)((DH)    & 0x000007FF) << 44)

#define GS_SET_EXTBUF(EXBP,EXBW,FBIN,WFFMD,EMODA,EMODC,WDX,WDY) \
 ((u64)((EXBP)  & 0x00003FFF) <<  0 | (u64)((EXBW)  & 0x0000003F) << 14 | \
 (u64)((FBIN)  & 0x00000003) << 20 | (u64)((WFFMD) & 0x00000001) << 22 | \
 (u64)((EMODA) & 0x00000003) << 23 | (u64)((EMODC) & 0x00000003) << 25 | \
 (u64)((WDX)   & 0x000007FF) << 32 | (u64)((WDY)   & 0x000007FF) << 43)

#define GS_SET_EXTDATA(SX,SY,SMPH,SMPV,WW,WH) \
 ((u64)((SX)   & 0x00000FFF) <<  0 | (u64)((SY)   & 0x000007FF) << 12 | \
 (u64)((SMPH) & 0x0000000F) << 23 | (u64)((SMPV) & 0x00000003) << 27 | \
 (u64)((WW)   & 0x00000FFF) << 32 | (u64)((WH)   & 0x000007FF) << 44)

#define GS_SET_EXTWRITE(WRITE) ((u64)((WRITE) & 0x00000001))

#define GS_SET_IMR(SIGMSK,FINMSK,HSMSK,VSMSK,EDWMSK) \
 ((u64)((SIGMSK) & 0x00000001) <<  8 | (u64)((FINMSK) & 0x00000001) <<  9 | \
 (u64)((HSMSK)  & 0x00000001) << 10 | (u64)((VSMSK)  & 0x00000001) << 11 | \
 (u64)((EDWMSK) & 0x00000001) << 12 | (u64)((1)      & 0x00000001) << 13 | \
 (u64)((1)      & 0x00000001) << 14)

// I guess CRTMD is always set 1
#define GS_SET_PMODE(EN1,EN2,MMOD,AMOD,SLBG,ALP) \
 ((u64)((EN1)  & 0x00000001) <<  0 | (u64)((EN2)  & 0x00000001) <<  1 | \
 (u64)((1)    & 0x00000007) <<  2 | (u64)((MMOD) & 0x00000001) <<  5 | \
 (u64)((AMOD) & 0x00000001) <<  6 | (u64)((SLBG) & 0x00000001) <<  7 | \
 (u64)((ALP)  & 0x000000FF) <<  8 | (u64)((0)    & 0x00000001) << 16)

#define GS_SET_PMODE_EXT(EN1,EN2,MMOD,AMOD,SLBG,ALP,NFLD,EXVWINS,EXVWINE,EXSYNCMD) \
 ((u64)((EN1)      & 0x00000001) <<  0 | (u64)((EN2)     & 0x00000001) <<  1 | \
 (u64)((1)        & 0x00000007) <<  2 | (u64)((MMOD)    & 0x00000001) <<  5 | \
 (u64)((AMOD)     & 0x00000001) <<  6 | (u64)((SLBG)    & 0x00000001) <<  7 | \
 (u64)((ALP)      & 0x000000FF) <<  8 | (u64)((NFLD)    & 0x00000001) << 16 | \
 (u64)((EXVWINS)  & 0x000003FF) << 32 | (u64)((EXVWINE) & 0x000003FF) << 42 | \
 (u64)((EVSYNCMD) & 0x00001FFF) << 52)

#define GS_SET_SIGLBLID(SIGID,LBLID) \
 ((u64)((SIGID) & 0xFFFFFFFF) <<  0 | (u64)((LBLID) & 0xFFFFFFFF) << 32)

#define GS_SET_SMODE1(RC,LC,T1248,SLCK,CMOD,EX,PRST,SINT,XPCK, \
                      PCK2,SPML,GCONT,PHS,PVS,PEHS,PEVS,CLKSEL, \
                      NVCK,SLCK2,VCKSEL,VHP) \
 ((u64)((RC)     & 0x00000007) <<  0 | (u64)((LC)     & 0x0000007F) <<  3 | \
 (u64)((T1248)  & 0x00000003) << 10 | (u64)((SLCK)   & 0x00000001) << 12 | \
 (u64)((CMOD)   & 0x00000003) << 13 | (u64)((EX)     & 0x00000001) << 15 | \
 (u64)((PRST)   & 0x00000001) << 16 | (u64)((SINT)   & 0x00000001) << 17 | \
 (u64)((XPCK)   & 0x00000001) << 18 | (u64)((PCK2)   & 0x00000003) << 19 | \
 (u64)((SPML)   & 0x0000000F) << 21 | (u64)((GCONT)  & 0x00000001) << 25 | \
 (u64)((PHS)    & 0x00000001) << 26 | (u64)((PVS)    & 0x00000001) << 27 | \
 (u64)((PEHS)   & 0x00000001) << 28 | (u64)((PEVS)   & 0x00000001) << 29 | \
 (u64)((CLKSEL) & 0x00000003) << 30 | (u64)((NVCK)   & 0x00000001) << 32 | \
 (u64)((SLCK2)  & 0x00000001) << 33 | (u64)((VCKSEL) & 0x00000003) << 34 | \
 (u64)((VHP)    & 0x00000003) << 36)

#define GS_SET_SMODE2(INT,FFMD,DPMS) \
 ((u64)((INT)  & 0x00000001) <<  0 | (u64)((FFMD) & 0x00000001) <<  1 | \
 (u64)((DPMS) & 0x00000003) <<  2)

#define GS_SET_SRFSH(A) ((u64)((A) & 0x00000000))

#define GS_SET_SYNCH1(HFP,HBP,HSEQ,HSVS,HS) \
 ((u64)((HFP)  & 0x000007FF) <<  0 | (u64)((HBP)  & 0x000007FF) << 11 | \
 (u64)((HSEQ) & 0x000003FF) << 22 | (u64)((HSVS) & 0x000007FF) << 32 | \
 (u64)((HS)   & 0x0000FFFF) << 43)

#define GS_SET_SYNCH2(HF,HB) \
 ((u64)((HF) & 0x000007FF) <<  0 | (u64)((HB) & 0x0000FFFF) << 11)

#define GS_SET_SYNCHV(VFP,VFPE,VBP,VBPE,VDP,VS) \
 ((u64)((VFP) & 0x000003FF) <<  0 | (u64)((VFPE) & 0x000003FF) << 10 | \
 (u64)((VBP) & 0x00000FFF) << 20 | (u64)((VBPE) & 0x00000FFF) << 32 | \
 (u64)((VDP) & 0x000007FF) << 42 | (u64)((VS)   & 0x00000FFF) << 53)

#endif /* __GS_PRIVILEGED_H__ */

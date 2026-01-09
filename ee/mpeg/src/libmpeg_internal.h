/*
# _____     ___ ____     ___ ____
#  ____|   |    ____|   |        | |____|
# |     ___|   |____ ___|    ____| |    \    PS2DEV Open Source Project.
#-----------------------------------------------------------------------
# Copyright (c) 2006 Eugene Plotnikov <e-plotnikov@operamail.com>
# Licenced under Academic Free License version 2.0
# Review ps2sdk README & LICENSE files for further details.
*/

/**
 * @file
 * MPEG decoding internal functions.
 */

#ifndef __libmpeg_internal_H
#define __libmpeg_internal_H

#include <libmpeg.h>

#define TRACE0()                                          \
    do {                                                  \
        printf("%s:%d\n", __PRETTY_FUNCTION__, __LINE__); \
    } while (0)
#define TRACE1(val)                                                             \
    do {                                                                        \
        printf("%s:%d %s == %08x\n", __PRETTY_FUNCTION__, __LINE__, #val, val); \
    } while (0)

#define GET_MACRO(_0, _1, _2, NAME, ...) NAME
#define TRACE(...)                       GET_MACRO(_0, ##__VA_ARGS__, TRACE2, TRACE1, TRACE0)(__VA_ARGS__)

#define _MPEG_PT_I 1
#define _MPEG_PT_P 2
#define _MPEG_PT_B 3
#define _MPEG_PT_D 4

#define _MPEG_PS_TOP_FIELD    1
#define _MPEG_PS_BOTTOM_FIELD 2
#define _MPEG_PS_FRAME        3

#define _MPEG_MBT_INTRA           1
#define _MPEG_MBT_PATTERN         2
#define _MPEG_MBT_MOTION_BACKWARD 4
#define _MPEG_MBT_MOTION_FORWARD  8
#define _MPEG_MBT_QUANT           16

#define _MPEG_MC_FIELD 1
#define _MPEG_MC_FRAME 2
#define _MPEG_MC_16X8  2
#define _MPEG_MC_DMV   3

#define _MPEG_MV_FIELD 0
#define _MPEG_MV_FRAME 1

#define _MPEG_CODE_PIC_START 0x00000100
#define _MPEG_CODE_SLICE_MIN 0x00000101
#define _MPEG_CODE_SLICE_MAX 0x000001AF
#define _MPEG_CODE_USER_DATA 0x000001B2
#define _MPEG_CODE_SEQ_HDR   0x000001B3
#define _MPEG_CODE_EXTENSION 0x000001B5
#define _MPEG_CODE_SEQ_END   0x000001B7
#define _MPEG_CODE_GRP_START 0x000001B8

#define _MPEG_XID_0         0
#define _MPEG_XID_SEQUENCE  1
#define _MPEG_XID_DISPLAY   2
#define _MPEG_XID_QMATRIX   3
#define _MPEG_XID_COPYRIGHT 4
#define _MPEG_XID_SCALABLE  5
#define _MPEG_XID_6         6
#define _MPEG_XID_PIC_DSP   7
#define _MPEG_XID_PIC_COD   8
#define _MPEG_XID_PIC_SSC   9
#define _MPEG_XID_PIC_TSC   10

typedef struct _MPEGMBXY
{
    unsigned char m_X;
    unsigned char m_Y;
} _MPEGMBXY;

typedef struct _MPEGMacroBlock8
{
    unsigned char m_Y[16][16];
    unsigned char m_Cb[8][8];
    unsigned char m_Cr[8][8];
} _MPEGMacroBlock8;

typedef struct _MPEGMotion
{
    unsigned char *m_pSrc;
    short *m_pDstY;
    short *m_pDstCbCr;
    int m_X;
    int m_Y;
    int m_H;
    int m_fInt;
    int m_Field;

    // These are used from assembly, supress linter warnings.
    // cppcheck-suppress unusedStructMember
    void (*MC_Luma)(struct _MPEGMotion *m, u8 *a2, short *a3, int a4, int var1, int tmpa, int a7, int a8);
    // cppcheck-suppress unusedStructMember
    void (*MC_Chroma)(struct _MPEGMotion *m, u8 *a2, short *a3, int a4, int var1, int tmpa, int a7, int a8);
} _MPEGMotion;

typedef struct _MPEGMotions
{
    unsigned char *m_pMBDstY;
    unsigned char *m_pMBDstCbCr;
    unsigned char *m_pSrc;
    unsigned char *m_pSPRBlk;
    unsigned char *m_pSPRRes;
    unsigned char *m_pSPRMC;
    int m_Stride;
    int m_nMotions;
    void (*BlockOp)(struct _MPEGMotions *arg0);
    _MPEGMotion m_Motion[7];
} _MPEGMotions;

typedef struct _MPEGContext
{
    MPEGSequenceInfo m_SI;
    s64 m_FwdPTS;
    s64 m_BckPTS;
    s64 m_AuxPTS;
    int m_MBWidth;
    int m_MBHeight;
    void *m_pFrameArena;
    _MPEGMacroBlock8 *m_pFwdFrame;
    _MPEGMacroBlock8 *m_pBckFrame;
    _MPEGMacroBlock8 *m_pAuxFrame;
    _MPEGMacroBlock8 *m_pCurFrame;
    _MPEGMBXY *m_pMBXY;
    unsigned char *m_pCurFrameY;
    unsigned char *m_pCurFrameCbCr;
    int m_FRCode;
    int m_MBStride;
    int m_MBCount;
    int m_fProgSeq;
    int m_fMPEG2;
    int m_fRepFF;
    int m_fTopFF;
    int m_fFPFrmDCT;
    int m_fConsMV;
    int m_fSecField;
    int m_fError;
    int m_fDCRst;
    int m_QScale;
    int m_PictStruct;
    int m_PictCodingType;
    int m_FPFVector;
    int m_FwdFCode;
    int m_FPBVector;
    int m_BckFCode;
    int m_FCode[2][2];
    int m_CurMC;
    _MPEGMotions m_MC[2];
    _MPEGMotions *m_pCurMotions;
} _MPEGContext;

extern void _MPEG_Initialize(_MPEGContext *mc, int (*data_cb)(void *userdata), void *cb_user, int *eof_flag);
extern void _MPEG_Destroy(void);
extern int _MPEG_CSCImage(void *source, void *dest, int mbcount);
extern void _MPEG_SetDefQM(int arg0);
extern void _MPEG_SetQM(int iqm);
extern int _MPEG_GetMBAI(void);
extern int _MPEG_GetMBType(void);
extern int _MPEG_GetMotionCode(void);
extern int _MPEG_GetDMVector(void);
extern unsigned int _MPEG_NextStartCode(void);
extern void _MPEG_AlignBits(void);
extern unsigned int _MPEG_GetBits(unsigned int bits);
extern unsigned int _MPEG_ShowBits(unsigned int bits);
extern void _MPEG_SetIDCP(void);
extern void _MPEG_SetQSTIVFAS(void);
extern void _MPEG_SetPCT(unsigned int arg0);
extern void _MPEG_BDEC(int mbi, int dcr, int dt, int qsc, void *spaddr);
extern int _MPEG_WaitBDEC(void);
extern void _MPEG_dma_ref_image(_MPEGMacroBlock8 *mb, _MPEGMotion *motions, s64 n_motions, int width);
extern void _MPEG_do_mc(_MPEGMotion *arg0);
extern void _MPEG_put_luma(_MPEGMotion *m, u8 *a2, short *a3, int a4, int var1, int ta, int a7, int a8);
extern void _MPEG_put_luma_X(_MPEGMotion *m, u8 *a2, short *a3, int a4, int var1, int ta, int a7, int a8);
extern void _MPEG_put_luma_Y(_MPEGMotion *m, u8 *a2, short *a3, int a4, int var1, int ta, int a7, int a8);
extern void _MPEG_put_luma_XY(_MPEGMotion *m, u8 *a2, short *a3, int a4, int var1, int ta, int a7, int a8);
extern void _MPEG_put_chroma(_MPEGMotion *m, u8 *a2, short *a3, int a4, int var1, int ta, int a7, int a8);
extern void _MPEG_put_chroma_X(_MPEGMotion *m, u8 *a2, short *a3, int a4, int var1, int ta, int a7, int a8);
extern void _MPEG_put_chroma_Y(_MPEGMotion *m, u8 *a2, short *a3, int a4, int var1, int ta, int a7, int a8);
extern void _MPEG_put_chroma_XY(_MPEGMotion *m, u8 *a2, short *a3, int a4, int var1, int ta, int a7, int a8);
extern void _MPEG_avg_luma(_MPEGMotion *m, u8 *a2, short *a3, int a4, int var1, int ta, int a7, int a8);
extern void _MPEG_avg_luma_X(_MPEGMotion *m, u8 *a2, short *a3, int a4, int var1, int ta, int a7, int a8);
extern void _MPEG_avg_luma_Y(_MPEGMotion *m, u8 *a2, short *a3, int a4, int var1, int ta, int a7, int a8);
extern void _MPEG_avg_luma_XY(_MPEGMotion *m, u8 *a2, short *a3, int a4, int var1, int ta, int a7, int a8);
extern void _MPEG_avg_chroma(_MPEGMotion *m, u8 *a2, short *a3, int a4, int var1, int ta, int a7, int a8);
extern void _MPEG_avg_chroma_X(_MPEGMotion *m, u8 *a2, short *a3, int a4, int var1, int ta, int a7, int a8);
extern void _MPEG_avg_chroma_Y(_MPEGMotion *m, u8 *a2, short *a3, int a4, int var1, int ta, int a7, int a8);
extern void _MPEG_avg_chroma_XY(_MPEGMotion *m, u8 *a2, short *a3, int a4, int var1, int ta, int a7, int a8);
extern void _MPEG_put_block_fr(_MPEGMotions *arg0);
extern void _MPEG_put_block_fl(_MPEGMotions *arg0);
extern void _MPEG_put_block_il(_MPEGMotions *arg0);
extern void _MPEG_add_block_ilfl(_MPEGMotions *arg0);
extern void _MPEG_add_block_frfr(_MPEGMotions *arg0);
extern void _MPEG_add_block_frfl(_MPEGMotions *arg0);
extern void _MPEG_Suspend(void);
extern void _MPEG_Resume(void);

#endif /* __libmpeg_internal_H */

/*
# _____     ___ ____     ___ ____
#  ____|   |    ____|   |        | |____|
# |     ___|   |____ ___|    ____| |    \    PS2DEV Open Source Project.
#-----------------------------------------------------------------------
# Copyright (c) 2006-2007 Eugene Plotnikov <e-plotnikov@operamail.com>
# Licenced under Academic Free License version 2.0
# Review ps2sdk README & LICENSE files for further details.
# Based on refernce software of MSSG
*/
#include "libmpeg.h"
#include "libmpeg_internal.h"

#include <malloc.h>
#include <stdio.h>
#include <string.h>
#include <kernel.h>

static _MPEGContext s_MPEG12Ctx;
static long*        s_pCurPTS;

static void ( *LumaOp[ 8 ] ) ( _MPEGMotion* ) = {
 _MPEG_put_luma, _MPEG_put_luma_X, _MPEG_put_luma_Y, _MPEG_put_luma_XY,
 _MPEG_avg_luma, _MPEG_avg_luma_X, _MPEG_avg_luma_Y, _MPEG_avg_luma_XY
};

static void ( *ChromaOp[ 8 ] ) ( void ) = {
 _MPEG_put_chroma, _MPEG_put_chroma_X, _MPEG_put_chroma_Y, _MPEG_put_chroma_XY,
 _MPEG_avg_chroma, _MPEG_avg_chroma_X, _MPEG_avg_chroma_Y, _MPEG_avg_chroma_XY
};

static void ( *PutBlockOp[ 3 ] ) ( _MPEGMotions* ) = {
 _MPEG_put_block_il, _MPEG_put_block_fr, _MPEG_put_block_fl
};

static void ( *AddBlockOp[ 2 ][ 2 ] ) ( _MPEGMotions* ) = {
 { _MPEG_add_block_frfr, _MPEG_add_block_frfl },
 { _MPEG_add_block_ilfl, _MPEG_add_block_ilfl }
};

static float s_FrameRate[ 16 ] = {
  0.0F, (  ( 23.0F * 1000.0F ) / 1001.0F  ),
 24.0F, 25.0F, (  ( 30.0F * 1000.0F ) / 1001.0F  ),
 30.0F, 50.0F, (  ( 60.0F * 1000.0F ) / 1001.0F  ),
 60.0F, 0.0F, 0.0F, 0.0F, 0.0F, 0.0F, 0.0F, 0.0F
};

static void* ( *_init_cb ) ( void*, MPEGSequenceInfo* );
static void* s_pInitCBParam;

static void* _init_seq    ( void );
static void  _destroy_seq ( void );

static int _get_hdr ( void );

static void _seq_header  ( void );
static void _gop_header  ( void );
static void _pic_header  ( void );
static void _ext_and_ud  ( void );
static void _ext_unknown ( void );
static void _ext_seq     ( void );
static void _ext_seq_dsp ( void );
static void _ext_qnt_mtx ( void );
static void _ext_cpy_rgt ( void );
static void _ext_seq_scl ( void );
static void _ext_pic_dsp ( void );
static void _ext_pic_cod ( void );
static void _ext_pic_ssc ( void );
static void _ext_pic_tsc ( void );
static void _xtra_bitinf ( void );

static int _get_next_picture  ( void*, long* );
static int _get_first_picture ( void*, long* );

int ( *MPEG_Picture ) ( void*, long* );

static void ( *DoMC ) ( void );

static void _mpeg12_picture_data ( void );
static int  _mpeg12_slice        ( int  );
static int  _mpeg12_dec_mb       ( int*, int*, int[ 2 ][ 2 ][ 2 ], int [ 2 ][ 2 ], int[ 2 ] );

void MPEG_Initialize (
      int   ( *apDataCB ) ( void*                    ), void* apDataCBParam,
      void* ( *apInitCB ) ( void*, MPEGSequenceInfo* ), void* apInitCBParam,
      long* apCurPTS
     ) {

 memset (  &s_MPEG12Ctx, 0, sizeof ( s_MPEG12Ctx )  );

 _MPEG_Initialize ( &s_MPEG12Ctx, apDataCB, apDataCBParam, &s_MPEG12Ctx.m_SI.m_fEOF );

 _init_cb       = apInitCB;
 s_pInitCBParam = apInitCBParam;
 s_pCurPTS      = apCurPTS;

 s_MPEG12Ctx.m_SI.m_FrameCnt  =  0;
 s_MPEG12Ctx.m_SI.m_fEOF      =  0;
 s_MPEG12Ctx.m_SI.m_Profile   = -1;
 s_MPEG12Ctx.m_SI.m_Level     = -1;
 s_MPEG12Ctx.m_SI.m_ChromaFmt = MPEG_CHROMA_FORMAT_420;
 s_MPEG12Ctx.m_SI.m_VideoFmt  = MPEG_VIDEO_FORMAT_UNSPEC;
 s_MPEG12Ctx.m_fMPEG2         =  0;

 s_MPEG12Ctx.m_MC[ 0 ].m_pSPRBlk = ( void* )0x70000000;
 s_MPEG12Ctx.m_MC[ 0 ].m_pSPRRes = ( void* )0x70000300;
 s_MPEG12Ctx.m_MC[ 0 ].m_pSPRMC  = ( void* )0x70000600;
 s_MPEG12Ctx.m_MC[ 1 ].m_pSPRBlk = ( void* )0x70001E00;
 s_MPEG12Ctx.m_MC[ 1 ].m_pSPRRes = ( void* )0x70002100;
 s_MPEG12Ctx.m_MC[ 1 ].m_pSPRMC  = ( void* )0x70002400;

 MPEG_Picture = _get_first_picture;

}  /* end MPEG_Initialize */

void MPEG_Destroy ( void ) {

 _destroy_seq  ();
 _MPEG_Destroy ();

}  /* end MPEG_Destroy */

static void* _init_seq ( void ) {

 int i, lSize, lMBWidth, lMBHeight;

 if ( !s_MPEG12Ctx.m_fMPEG2 ) {
  s_MPEG12Ctx.m_fProgSeq   = 1;
  s_MPEG12Ctx.m_PictStruct = _MPEG_PS_FRAME;
  s_MPEG12Ctx.m_fFPFrmDCT  = 1;
 }  /* end if */

 lMBWidth  = ( s_MPEG12Ctx.m_SI.m_Width + 15 ) / 16;
 lMBHeight = s_MPEG12Ctx.m_fMPEG2 && !s_MPEG12Ctx.m_fProgSeq ? 2 * (  ( s_MPEG12Ctx.m_SI.m_Height + 31 ) / 32  )
                                                             : ( s_MPEG12Ctx.m_SI.m_Height + 15 ) / 16;

 if ( lMBWidth  != s_MPEG12Ctx.m_MBWidth ||
      lMBHeight != s_MPEG12Ctx.m_MBHeight
 ) {

  if ( s_MPEG12Ctx.m_pFwdFrame ) _destroy_seq ();

  s_MPEG12Ctx.m_MBWidth     = lMBWidth;
  s_MPEG12Ctx.m_MBHeight    = lMBHeight;
  s_MPEG12Ctx.m_SI.m_Width  = lMBWidth  << 4;
  s_MPEG12Ctx.m_SI.m_Height = lMBHeight << 4;

  s_MPEG12Ctx.m_MBStride = lMBWidth * sizeof ( _MPEGMacroBlock8 );

  lSize = lMBWidth * ( lMBHeight + 1 ) * sizeof ( _MPEGMacroBlock8 ) + sizeof ( _MPEGMacroBlock8 );

  s_MPEG12Ctx.m_pFwdFrame = ( _MPEGMacroBlock8* )memalign ( 64, lSize );
  s_MPEG12Ctx.m_pBckFrame = ( _MPEGMacroBlock8* )memalign ( 64, lSize );
  s_MPEG12Ctx.m_pAuxFrame = ( _MPEGMacroBlock8* )memalign ( 64, lSize );

  s_MPEG12Ctx.m_pMBXY = ( _MPEGMBXY* )malloc (
   sizeof ( _MPEGMBXY ) * ( s_MPEG12Ctx.m_MBCount = s_MPEG12Ctx.m_MBWidth * s_MPEG12Ctx.m_MBHeight )
  );

  for ( i = 0; i < s_MPEG12Ctx.m_MBCount; ++i ) {
   s_MPEG12Ctx.m_pMBXY[ i ].m_X = i % lMBWidth;
   s_MPEG12Ctx.m_pMBXY[ i ].m_Y = i / lMBWidth;
  }  /* end for */

 }  /* end if */

 return _init_cb ( s_pInitCBParam, &s_MPEG12Ctx.m_SI );

}  /* end _init_seq */

static void _destroy_seq ( void ) {

 MPEG_Picture = _get_first_picture;

 if ( s_MPEG12Ctx.m_pAuxFrame ) {
  free ( s_MPEG12Ctx.m_pAuxFrame );
  s_MPEG12Ctx.m_pAuxFrame = NULL;
 }  /* end if */

 if (  s_MPEG12Ctx.m_pBckFrame ) {
  free ( s_MPEG12Ctx.m_pBckFrame );
  s_MPEG12Ctx.m_pBckFrame = NULL;
 }  /* end if */

 if ( s_MPEG12Ctx.m_pFwdFrame ) {
  free ( s_MPEG12Ctx.m_pFwdFrame );
  s_MPEG12Ctx.m_pFwdFrame = NULL;
 }  /* end if */

 if ( s_MPEG12Ctx.m_pMBXY ) {
  free ( s_MPEG12Ctx.m_pMBXY );
  s_MPEG12Ctx.m_pMBXY = NULL;
 }  /* end if */

 s_MPEG12Ctx.m_MBWidth  =
 s_MPEG12Ctx.m_MBHeight = 0;

}  /* end _destroy_seq */

static int _get_hdr ( void ) {

 unsigned int lCode;

 while ( 1 ) {

  lCode = _MPEG_NextStartCode (); _MPEG_GetBits ( 32 );

  switch ( lCode ) {

   case _MPEG_CODE_SEQ_HDR:
    _seq_header ();
   break;

   case _MPEG_CODE_GRP_START:
    _gop_header ();
   break;

   case _MPEG_CODE_PIC_START:
    _pic_header ();
   return 1;

   case _MPEG_CODE_SEQ_END:
    MPEG_Picture = _get_first_picture;
    return 0;
   break;

  }  /* end switch */

 }  /* end while */

}  /* end _get_hdr */

static void _seq_header ( void ) {

 s_MPEG12Ctx.m_SI.m_Width  = _MPEG_GetBits ( 12 );
 s_MPEG12Ctx.m_SI.m_Height = _MPEG_GetBits ( 12 );

 _MPEG_GetBits (  4 );  /* aspect_ratio_information    */
 s_MPEG12Ctx.m_SI.m_MSPerFrame = ( int )(
  (  1000.0F / s_FrameRate[ s_MPEG12Ctx.m_FRCode = _MPEG_GetBits ( 4 ) ]  ) + 0.5F
 );
 _MPEG_GetBits ( 18 );  /* bit_rate_value              */
 _MPEG_GetBits (  1 );  /* marker_bit                  */
 _MPEG_GetBits ( 10 );  /* vbv_buffer_size             */
 _MPEG_GetBits (  1 );  /* constrained_parameters_flag */

 if (  _MPEG_GetBits ( 1 )  )
  _MPEG_SetQM ( 0 );
 else _MPEG_SetDefQM ( 0 );

 if (  _MPEG_GetBits ( 1 )  )
  _MPEG_SetQM ( 1 );
 else _MPEG_SetDefQM ( 1 );

 _ext_and_ud ();

}  /* end _seq_header */

static void _gop_header ( void ) {
#ifdef _DEBUG
 _MPEG_GetBits ( 1 );  /* drop_flag   */
 _MPEG_GetBits ( 5 );  /* hour        */
 _MPEG_GetBits ( 6 );  /* minute      */
 _MPEG_GetBits ( 1 );  /* marker_bit  */
 _MPEG_GetBits ( 6 );  /* sec         */
 _MPEG_GetBits ( 6 );  /* frame       */
 _MPEG_GetBits ( 1 );  /* closed_gop  */
 _MPEG_GetBits ( 1 );  /* broken_link */
#else
 _MPEG_GetBits ( 27 );
#endif  /* _DEBUG */
 _ext_and_ud ();

}  /* end _gop_header */

static void _pic_header ( void ) {

 unsigned int lPicCT;

 _MPEG_GetBits ( 10 );  /* temporal_reference */
 lPicCT = _MPEG_GetBits ( 3 );
 _MPEG_GetBits ( 16 );  /* vbv_delay          */

 if ( lPicCT == _MPEG_PT_P || lPicCT == _MPEG_PT_B ) {
  s_MPEG12Ctx.m_FPFVector = _MPEG_GetBits ( 1 );
  s_MPEG12Ctx.m_FwdFCode  = _MPEG_GetBits ( 3 );
 }  /* end if */

 if ( lPicCT == _MPEG_PT_B ) {
  s_MPEG12Ctx.m_FPBVector = _MPEG_GetBits ( 1 );
  s_MPEG12Ctx.m_BckFCode  = _MPEG_GetBits ( 3 );
 }  /* end if */

 _xtra_bitinf ();
 _ext_and_ud  ();

 _MPEG_SetPCT ( s_MPEG12Ctx.m_PictCodingType = lPicCT );

}  /* end _pic_header */

static void _ext_and_ud ( void ) {

 int lCode, lXID;

 lCode = _MPEG_NextStartCode ();

 while ( lCode == _MPEG_CODE_EXTENSION || lCode == _MPEG_CODE_USER_DATA ) {

  if (  lCode == _MPEG_CODE_EXTENSION ) {

   _MPEG_GetBits ( 32 );
   lXID = _MPEG_GetBits ( 4 );

   switch ( lXID ) {

    case _MPEG_XID_0:
     _ext_unknown ();
    break;

    case _MPEG_XID_SEQUENCE:
     _ext_seq ();
    break;

    case _MPEG_XID_DISPLAY:
     _ext_seq_dsp ();
    break;

    case _MPEG_XID_QMATRIX:
     _ext_qnt_mtx ();
    break;

    case _MPEG_XID_COPYRIGHT:
     _ext_cpy_rgt ();
    break;

    case _MPEG_XID_SCALABLE:
     _ext_seq_scl ();
    break;

    case _MPEG_XID_6:
     _ext_unknown ();
    break;

    case _MPEG_XID_PIC_DSP:
     _ext_pic_dsp ();
    break;

    case _MPEG_XID_PIC_COD:
     _ext_pic_cod ();
    break;

    case _MPEG_XID_PIC_SSC:
     _ext_pic_ssc ();
    break;

    case _MPEG_XID_PIC_TSC:
     _ext_pic_tsc ();
    break;

   }  /* end switch */

   lCode = _MPEG_NextStartCode ();

  } else {  /* user data */

   _MPEG_GetBits ( 32 );
   lCode = _MPEG_NextStartCode ();

  }  /* end else */

 }  /* end while */

}  /* end _ext_and_ud */

static void _ext_unknown ( void ) {


}  /* end _ext_unknown */

static void _ext_seq ( void ) {

 int lHSzX;
 int lVSzX;
 int lProfLevel;
 int lFRXn, lFRXd;

 s_MPEG12Ctx.m_fMPEG2 = 1;

 *( volatile unsigned int* )0x10002010 &= 0xFF7FFFFF;

 lProfLevel                   = _MPEG_GetBits ( 8 );
 s_MPEG12Ctx.m_fProgSeq       = _MPEG_GetBits ( 1 );
 s_MPEG12Ctx.m_SI.m_ChromaFmt = _MPEG_GetBits ( 2 );
 lHSzX                        = _MPEG_GetBits ( 2 );
 lVSzX                        = _MPEG_GetBits ( 2 );
#ifdef _DEBUG
 _MPEG_GetBits ( 12 );  /* bit_rate_extension        */
 _MPEG_GetBits (  1 );  /* marker_bit                */
 _MPEG_GetBits (  8 );  /* vbv_buffer_size_extension */
 _MPEG_GetBits (  1 );  /* low_delay                 */
 lFRXn = _MPEG_GetBits ( 2 );
 lFRXd = _MPEG_GetBits ( 5 );
#else
 _MPEG_GetBits ( 22 );
 lFRXn = _MPEG_GetBits ( 2 );
 lFRXd = _MPEG_GetBits ( 5 );
#endif  /* _DEBUG */
 s_MPEG12Ctx.m_SI.m_MSPerFrame = ( int )(
  (  1000.0F / (
      s_FrameRate[ s_MPEG12Ctx.m_FRCode ] * (  ( lFRXn + 1.0F ) / ( lFRXd + 1.0F )  )
     )
  ) + 0.5F
 );

 if(  ( lProfLevel >> 7 ) & 1  ) {
  
  if (  ( lProfLevel & 15 ) == 5  ) {

   s_MPEG12Ctx.m_SI.m_Profile = MPEG_PROFILE_422;
   s_MPEG12Ctx.m_SI.m_Level   = MPEG_LEVEL_MAIN;  

  } else s_MPEG12Ctx.m_SI.m_Profile = s_MPEG12Ctx.m_SI.m_Level = -1;

 } else {

  s_MPEG12Ctx.m_SI.m_Profile = lProfLevel >> 4;
  s_MPEG12Ctx.m_SI.m_Level   = lProfLevel & 0xF;

 }  /* end else */

 s_MPEG12Ctx.m_SI.m_Width  = ( lHSzX << 12 ) | ( s_MPEG12Ctx.m_SI.m_Width  & 0x0FFF );
 s_MPEG12Ctx.m_SI.m_Height = ( lVSzX << 12 ) | ( s_MPEG12Ctx.m_SI.m_Height & 0x0FFF );

}  /* end _ext_seq */

static void _ext_seq_dsp ( void ) {

 s_MPEG12Ctx.m_SI.m_VideoFmt = _MPEG_GetBits ( 3 );

 if (  _MPEG_GetBits ( 1 )  ) {  /* color_description */
#ifdef _DEBUG
  _MPEG_GetBits ( 8 );  /* color_primaries          */
  _MPEG_GetBits ( 8 );  /* transfer_characteristics */
  _MPEG_GetBits ( 8 );  /* matrix_coefficients      */
#else
  _MPEG_GetBits ( 24 );
#endif  /* _DEBUG */
 }  /* end if */
#ifdef _DEBUG
 _MPEG_GetBits ( 14 );  /* display_horizontal_size */
 _MPEG_GetBits (  1 );  /* marker_bit              */
 _MPEG_GetBits ( 14 );  /* display_vertical_size   */
#else
 _MPEG_GetBits ( 29 );
#endif  /* _DEBUG */
}  /* end _ext_seq_dsp */

static void _ext_qnt_mtx ( void ) {

 int i;

 if (  _MPEG_GetBits ( 1 )  ) _MPEG_SetQM ( 0 );
 if (  _MPEG_GetBits ( 1 )  ) _MPEG_SetQM ( 1 );

 if (  _MPEG_GetBits ( 1 )  ) for ( i = 0; i < 16; ++i ) _MPEG_GetBits ( 32 );
 if (  _MPEG_GetBits ( 1 )  ) for ( i = 0; i < 16; ++i ) _MPEG_GetBits ( 32 );

}  /* end _ext_qnt_mtx */

static void _ext_cpy_rgt ( void ) {
#ifdef _DEBUG
 _MPEG_GetBits (  1 );  /* copyright_flag       */
 _MPEG_GetBits (  8 );  /* copyright_identifier */
 _MPEG_GetBits (  1 );  /* original_or_copy     */
 _MPEG_GetBits (  7 );  /* reserved_data        */
 _MPEG_GetBits (  1 );  /* marker_bit           */
 _MPEG_GetBits ( 20 );  /* copyright_number_1   */
 _MPEG_GetBits (  1 );  /* marker_bit           */
 _MPEG_GetBits ( 22 );  /* copyright_number_2   */
 _MPEG_GetBits (  1 );  /* marker_bit           */
 _MPEG_GetBits ( 22 );  /* copyright_number_3   */
#else
 _MPEG_GetBits ( 32 );
 _MPEG_GetBits ( 32 );
 _MPEG_GetBits ( 20 );
#endif  /* _DEBUG */
}  /* end _ext_cop_rgt */

static void _ext_seq_scl ( void ) {

}  /* end _ext_seq_scl */

static void _ext_pic_dsp ( void ) {

 int i;
 int lnFCO;

 if ( s_MPEG12Ctx.m_fProgSeq ) {

  if ( s_MPEG12Ctx.m_fRepFF )
   lnFCO = s_MPEG12Ctx.m_fTopFF ? 3 : 2;
  else lnFCO = 1;

 } else {

  if ( s_MPEG12Ctx.m_PictStruct != _MPEG_PS_FRAME )
   lnFCO = 1;
  else lnFCO = s_MPEG12Ctx.m_fRepFF ? 3 : 2;

 }  /* end else */

 for ( i = 0; i < lnFCO; ++i ) {
  _MPEG_GetBits ( 16 );  /* frame_center_horizontal_offset[ i ] */
  _MPEG_GetBits (  1 );  /* marker_bit                          */
  _MPEG_GetBits ( 16 );  /* frame_center_vertical_offset [ i ]  */
  _MPEG_GetBits (  1 );  /* marker_bit                          */
 }  /* end for */

}  /* end _ext_pic_dsp */

static void _ext_pic_cod ( void ) {

 s_MPEG12Ctx.m_FCode[ 0 ][ 0 ] = _MPEG_GetBits ( 4 );
 s_MPEG12Ctx.m_FCode[ 0 ][ 1 ] = _MPEG_GetBits ( 4 );
 s_MPEG12Ctx.m_FCode[ 1 ][ 0 ] = _MPEG_GetBits ( 4 );
 s_MPEG12Ctx.m_FCode[ 1 ][ 1 ] = _MPEG_GetBits ( 4 );
 _MPEG_SetIDCP ();
 s_MPEG12Ctx.m_PictStruct = _MPEG_GetBits ( 2 );
 s_MPEG12Ctx.m_fTopFF     = _MPEG_GetBits ( 1 );
 s_MPEG12Ctx.m_fFPFrmDCT  = _MPEG_GetBits ( 1 );
 s_MPEG12Ctx.m_fConsMV    = _MPEG_GetBits ( 1 );
 _MPEG_SetQSTIVFAS ();
 s_MPEG12Ctx.m_fRepFF     = _MPEG_GetBits ( 1 );
#ifdef _DEBUG
 _MPEG_GetBits ( 1 );  /* chroma_420_type   */
 _MPEG_GetBits ( 1 );  /* progressive_frame */
#else
 _MPEG_GetBits ( 2 );
#endif  /* _DEBUG */
 if (  _MPEG_GetBits ( 1 )  ) {  /* composite_display_flag */
#ifdef _DEBUG
  _MPEG_GetBits ( 1 );  /* v_axis            */
  _MPEG_GetBits ( 3 );  /* field_sequence    */
  _MPEG_GetBits ( 1 );  /* sub_carrier       */
  _MPEG_GetBits ( 7 );  /* burst_amplitude   */
  _MPEG_GetBits ( 8 );  /* sub_carrier_phase */
#else
  _MPEG_GetBits ( 20 );
#endif  /* _DEBUG */
 }  /* end if */

}  /* end _ext_pic_cod */

static void _ext_pic_ssc ( void ) {

}  /* end _ext_pic_ssc */

static void _ext_pic_tsc ( void ) {

}  /* end _ext_pic_tsc */

static void _xtra_bitinf ( void ) {

 while (  _MPEG_GetBits ( 1 )  ) _MPEG_GetBits ( 8 );

}  /* end _xtra_bitinf */

static int _get_first_picture ( void* apData, long* apPTS ) {

 int retVal = _get_hdr ();

 if ( retVal ) {

  s_MPEG12Ctx.m_SI.m_FrameCnt = 0;

  apData = _init_seq ();
  _mpeg12_picture_data ();

  if ( s_MPEG12Ctx.m_PictStruct != _MPEG_PS_FRAME ) s_MPEG12Ctx.m_fSecField ^= 1;

  MPEG_Picture = _get_next_picture;

  if ( !s_MPEG12Ctx.m_fSecField ) ++s_MPEG12Ctx.m_SI.m_FrameCnt;

  retVal = _get_next_picture ( apData, apPTS );

 }  /* end if */

 return retVal;

}  /* end _get_first_picture */

static int _get_next_picture ( void* apData, long* apPTS ) {

 int retVal;

 while ( 1 ) {

  if (   (  retVal = _get_hdr ()  )   ) {

   int lfPic = 0;

   _mpeg12_picture_data ();

   if (  ( s_MPEG12Ctx.m_PictStruct == _MPEG_PS_FRAME || s_MPEG12Ctx.m_fSecField ) && s_MPEG12Ctx.m_SI.m_FrameCnt  ) {

    void* lpData;

    if ( s_MPEG12Ctx.m_PictCodingType == _MPEG_PT_B ) {
     lpData = s_MPEG12Ctx.m_pAuxFrame;
     *apPTS = s_MPEG12Ctx.m_AuxPTS;
    } else {
     lpData = s_MPEG12Ctx.m_pFwdFrame;
     *apPTS = s_MPEG12Ctx.m_FwdPTS;
    }  /* end else */

    lfPic = _MPEG_CSCImage ( lpData, apData, s_MPEG12Ctx.m_MBCount );

   }  /* end if */

   if ( s_MPEG12Ctx.m_PictStruct != _MPEG_PS_FRAME ) s_MPEG12Ctx.m_fSecField ^= 1;

   if ( !s_MPEG12Ctx.m_fSecField ) ++s_MPEG12Ctx.m_SI.m_FrameCnt;

   if ( lfPic ) break;

  } else break;

 }  /* end while */

 return retVal;

}  /* end _get_next_picture */

static void _mpeg12_do_next_mc ( void ) {

 _MPEGMotions* lpMotions = &s_MPEG12Ctx.m_MC[ !s_MPEG12Ctx.m_CurMC ];
 _MPEGMotion*  lpMotion  = &lpMotions -> m_Motion[ 0 ];

 while ( lpMotion -> MC_Luma ) {

  _MPEG_do_mc ( lpMotion );
  ++lpMotion;

 }  /* end while */

 lpMotions -> BlockOp ( lpMotions );

}  /* end _mpeg12_do_next_mc */

static void _mpeg12_do_first_mc ( void ) {

 DoMC = _mpeg12_do_next_mc;

}  /* end _mpeg12_do_first_mc */

static void _mpeg12_picture_data ( void ) {

 int               lMBAMax = s_MPEG12Ctx.m_MBWidth * s_MPEG12Ctx.m_MBHeight;
 _MPEGMacroBlock8* lpMB;
 long              lPTS;

 if ( s_MPEG12Ctx.m_PictStruct == _MPEG_PS_FRAME && s_MPEG12Ctx.m_fSecField ) s_MPEG12Ctx.m_fSecField = 0;

 if ( s_MPEG12Ctx.m_PictCodingType == _MPEG_PT_B ) {
  s_MPEG12Ctx.m_pCurFrame = s_MPEG12Ctx.m_pAuxFrame;
  s_MPEG12Ctx.m_AuxPTS    = *s_pCurPTS;
 } else {
  if ( !s_MPEG12Ctx.m_fSecField ) {
   lpMB = s_MPEG12Ctx.m_pFwdFrame;
   lPTS = s_MPEG12Ctx.m_FwdPTS;
   s_MPEG12Ctx.m_pFwdFrame = s_MPEG12Ctx.m_pBckFrame;
   s_MPEG12Ctx.m_FwdPTS    = s_MPEG12Ctx.m_BckPTS;
   s_MPEG12Ctx.m_pBckFrame = lpMB;
   s_MPEG12Ctx.m_BckPTS    = lPTS;
  }  /* end if */
  s_MPEG12Ctx.m_pCurFrame = s_MPEG12Ctx.m_pBckFrame;
  s_MPEG12Ctx.m_BckPTS    = *s_pCurPTS;
 }  /* end else */
 s_MPEG12Ctx.m_pCurFrameY    = ( unsigned char* )s_MPEG12Ctx.m_pCurFrame;
 s_MPEG12Ctx.m_pCurFrameCbCr = ( unsigned char* )s_MPEG12Ctx.m_pCurFrame + 256;
 if ( s_MPEG12Ctx.m_PictStruct == _MPEG_PS_BOTTOM_FIELD ) {
  s_MPEG12Ctx.m_pCurFrameY    += 16;
  s_MPEG12Ctx.m_pCurFrameCbCr +=  8;
 }  /* end if */

 if ( s_MPEG12Ctx.m_PictStruct != _MPEG_PS_FRAME ) lMBAMax >>= 1;

 s_MPEG12Ctx.m_CurMC = 0;
 DoMC = _mpeg12_do_first_mc;

 while (  _mpeg12_slice ( lMBAMax ) >= 0  );

 _MPEG_WaitBDEC ();
 DoMC ();

}  /* end _mpeg12_picture_data */

static void _mpeg12_decode_motion_vector (
             int* apPred, int aRSize, int aMotionCode, int aMotionResidual, int aFullPelVector
            ) {

 int lLim = 16 << aRSize;
 int lVec = aFullPelVector ? *apPred >> 1 : *apPred;

 if ( aMotionCode > 0 ) {

  lVec += (  ( aMotionCode - 1 ) << aRSize ) + aMotionResidual + 1;

  if ( lVec >= lLim ) lVec -= lLim + lLim;

 } else if ( aMotionCode < 0 ) {

  lVec -= (  ( -aMotionCode - 1 ) << aRSize ) + aMotionResidual + 1;

  if ( lVec < -lLim ) lVec += lLim + lLim;

 }  /* end if */

 *apPred = aFullPelVector ? lVec << 1 : lVec;

}  /* end _mpeg12_decode_motion_vector */

static void _mpeg12_dual_prime_vector ( int aDMV[][ 2 ], int* apDMVector, int aMVX, int aMVY ) {

 if ( s_MPEG12Ctx.m_PictStruct == _MPEG_PS_FRAME ) {

  if ( s_MPEG12Ctx.m_fTopFF ) {

   aDMV[ 0 ][ 0 ] = (  ( aMVX + ( aMVX > 0 )  ) >> 1  ) + apDMVector[ 0 ];
   aDMV[ 0 ][ 1 ] = (  ( aMVY + ( aMVY > 0 )  ) >> 1  ) + apDMVector[ 1 ] - 1;

   aDMV[ 1 ][ 0 ] = (   (  3 * aMVX + (  aMVX > 0 )  ) >> 1   ) + apDMVector[ 0 ];
   aDMV[ 1 ][ 1 ] = (   (  3 * aMVY + (  aMVY > 0 )  ) >> 1   ) + apDMVector[ 1 ] + 1;

  } else {

   aDMV[ 0 ][ 0 ] = (   ( 3 * aMVX + (  aMVX > 0 )  ) >> 1   ) + apDMVector[ 0 ];
   aDMV[ 0 ][ 1 ] = (   ( 3 * aMVY + (  aMVY > 0 )  ) >> 1   ) + apDMVector[ 1 ] - 1;

   aDMV[ 1 ][ 0 ] = (   (  aMVX + ( aMVX > 0 )  ) >> 1  ) + apDMVector[ 0 ];
   aDMV[ 1 ][ 1 ] = (   (  aMVY + ( aMVY > 0 )  ) >> 1  ) + apDMVector[ 1 ] + 1;

  }  /* end else */

 } else {

  aDMV[ 0 ][ 0 ] = (   (  aMVX + ( aMVX > 0 )  ) >> 1   ) + apDMVector[ 0 ];
  aDMV[ 0 ][ 1 ] = (   (  aMVY + ( aMVY > 0 )  ) >> 1   ) + apDMVector[ 1 ];

  if ( s_MPEG12Ctx.m_PictStruct == _MPEG_PS_TOP_FIELD )
   --aDMV[ 0 ][ 1 ];
  else ++aDMV[ 0 ][ 1 ];

 }  /* end else */

}  /* end _mpeg12_dual_prime_vector */

static void _mpeg12_motion_vector (
             int* apPMV, int* apDMVector, int aHRSize, int aVRSize, int aDMV, int aMVScale, int aFullPelVector
            ) {

 int lMotionCode     = _MPEG_GetMotionCode ();
 int lMotionResidual = aHRSize && lMotionCode ? _MPEG_GetBits ( aHRSize ) : 0;

 _mpeg12_decode_motion_vector (
  &apPMV[ 0 ], aHRSize, lMotionCode, lMotionResidual, aFullPelVector
 );

 if ( aDMV ) apDMVector[ 0 ] = _MPEG_GetDMVector ();

 s_MPEG12Ctx.m_fError = lMotionCode == -32768;

 lMotionCode     = _MPEG_GetMotionCode ();
 lMotionResidual = aVRSize && lMotionCode ? _MPEG_GetBits ( aVRSize ) : 0;

 if ( aMVScale ) apPMV[ 1 ] >>= 1;

 _mpeg12_decode_motion_vector (
  &apPMV[ 1 ], aVRSize, lMotionCode, lMotionResidual, aFullPelVector
 );

 if ( aMVScale ) apPMV[ 1 ] <<= 1;

 if ( aDMV ) apDMVector[ 1 ] = _MPEG_GetDMVector ();

 s_MPEG12Ctx.m_fError = lMotionCode == -32768;

}  /* end _mpeg12_motion_vector */

static void _mpeg12_motion_vectors (
             int aPMV[ 2 ][ 2 ][ 2 ], int aDMVector[ 2 ], int aMVFS[ 2 ][ 2 ], int aS, int anMV,
             int aMVFmt, int aHRSize, int aVRSize, int aDMV, int aMVScale
            ) {

 if ( anMV == 1 ) {

  if ( aMVFmt == _MPEG_MV_FIELD && !aDMV ) aMVFS[ 1 ][ aS ] = aMVFS[ 0 ][ aS ] = _MPEG_GetBits ( 1 );

  _mpeg12_motion_vector ( aPMV[ 0 ][ aS ], aDMVector, aHRSize, aVRSize, aDMV, aMVScale, 0 );

  aPMV[ 1 ][ aS ][ 0 ] = aPMV[ 0 ][ aS ][ 0 ];
  aPMV[ 1 ][ aS ][ 1 ] = aPMV[ 0 ][ aS ][ 1 ];

 } else {

  aMVFS[ 0 ][ aS ] = _MPEG_GetBits ( 1 );
  _mpeg12_motion_vector ( aPMV[ 0 ][ aS ], aDMVector, aHRSize, aVRSize, aDMV, aMVScale, 0 );

  aMVFS[ 1 ][ aS ] = _MPEG_GetBits ( 1 );
  _mpeg12_motion_vector ( aPMV[ 1 ][ aS ], aDMVector, aHRSize, aVRSize, aDMV, aMVScale, 0 );

 }  /* end else */

}  /* end _mpeg12_motion_vectors */

static int _mpeg12_dec_mb (
            int* apMBType, int* apMotionType,
            int aPMV[ 2 ][ 2 ][ 2 ], int aMVFS[ 2 ][ 2 ], int aDMVector[ 2 ]
           ) {

 int lMBType;
 int lDMV;
 int lMVScale;
 int lnMV;
 int lMVFmt;
 int lDCType;
 int lMotionType;
 int lfIntra;

 lMotionType = 0;
 lMBType     = _MPEG_GetMBType ();

 if ( !lMBType ) return 0;

 lfIntra = lMBType & _MPEG_MBT_INTRA;

 if (  lMBType & ( _MPEG_MBT_MOTION_FORWARD | _MPEG_MBT_MOTION_BACKWARD )  ) {

  if ( s_MPEG12Ctx.m_PictStruct == _MPEG_PS_FRAME )
   lMotionType = s_MPEG12Ctx.m_fFPFrmDCT ? _MPEG_MC_FRAME : _MPEG_GetBits ( 2 );
  else lMotionType = _MPEG_GetBits ( 2 );

 } else if ( lfIntra && s_MPEG12Ctx.m_fConsMV )
  lMotionType = ( s_MPEG12Ctx.m_PictStruct == _MPEG_PS_FRAME ) ? _MPEG_MC_FRAME : _MPEG_MC_FIELD;

 if ( s_MPEG12Ctx.m_PictStruct == _MPEG_PS_FRAME ) {
  lnMV   = lMotionType == _MPEG_MC_FIELD ? 2 : 1;
  lMVFmt = lMotionType == _MPEG_MC_FRAME ? _MPEG_MV_FRAME : _MPEG_MV_FIELD;
 } else {
  lnMV   = ( lMotionType == _MPEG_MC_16X8 ) ? 2 : 1;
  lMVFmt = _MPEG_MV_FIELD;
 }  /* end else */

 lDMV     = lMotionType == _MPEG_MC_DMV;
 lMVScale = lMVFmt == _MPEG_MV_FIELD && s_MPEG12Ctx.m_PictStruct == _MPEG_PS_FRAME;
 lDCType  = s_MPEG12Ctx.m_PictStruct == _MPEG_PS_FRAME &&
           !s_MPEG12Ctx.m_fFPFrmDCT                    &&
            lMBType & ( _MPEG_MBT_PATTERN | _MPEG_MBT_INTRA ) ? _MPEG_GetBits ( 1 ) : 0;

 if ( lMBType & _MPEG_MBT_QUANT ) s_MPEG12Ctx.m_QScale = _MPEG_GetBits ( 5 );

 if (  ( lMBType & _MPEG_MBT_MOTION_FORWARD ) ||
       ( lfIntra && s_MPEG12Ctx.m_fConsMV )
 ) {

  if ( s_MPEG12Ctx.m_fMPEG2 )
   _mpeg12_motion_vectors (
    aPMV, aDMVector, aMVFS, 0, lnMV, lMVFmt,
    s_MPEG12Ctx.m_FCode[ 0 ][ 0 ] - 1, s_MPEG12Ctx.m_FCode[ 0 ][ 1 ] - 1, lDMV, lMVScale
   );
  else _mpeg12_motion_vector (
        aPMV[ 0 ][ 0 ], aDMVector, s_MPEG12Ctx.m_FwdFCode - 1, s_MPEG12Ctx.m_FwdFCode - 1,
        0, 0, s_MPEG12Ctx.m_FPFVector
       );
 }  /* end if */

 if ( s_MPEG12Ctx.m_fError ) return 0;

 if ( lMBType & _MPEG_MBT_MOTION_BACKWARD ) {

  if ( s_MPEG12Ctx.m_fMPEG2 )
   _mpeg12_motion_vectors (
    aPMV, aDMVector, aMVFS, 1, lnMV, lMVFmt,
    s_MPEG12Ctx.m_FCode[ 1 ][ 0 ] - 1, s_MPEG12Ctx.m_FCode[ 1 ][ 1 ] - 1, 0, lMVScale
   );
  else _mpeg12_motion_vector (
        aPMV[ 0 ][ 1 ], aDMVector, s_MPEG12Ctx.m_BckFCode - 1, s_MPEG12Ctx.m_BckFCode - 1,
        0, 0, s_MPEG12Ctx.m_FPBVector
       );
 }  /* end if */

 if ( s_MPEG12Ctx.m_fError ) return 0;

 if ( lfIntra && s_MPEG12Ctx.m_fConsMV ) _MPEG_GetBits ( 1 );

 if (  lMBType & ( _MPEG_MBT_INTRA | _MPEG_MBT_PATTERN )  )
  _MPEG_BDEC ( lfIntra, s_MPEG12Ctx.m_fDCRst, lDCType, s_MPEG12Ctx.m_QScale, s_MPEG12Ctx.m_pCurMotions -> m_pSPRBlk );

 s_MPEG12Ctx.m_fDCRst = !lfIntra;

 if ( lfIntra && !s_MPEG12Ctx.m_fConsMV )
  aPMV[ 0 ][ 0 ][ 0 ] = aPMV[ 0 ][ 0 ][ 1 ] =
  aPMV[ 1 ][ 0 ][ 0 ] = aPMV[ 1 ][ 0 ][ 1 ] =
  aPMV[ 0 ][ 1 ][ 0 ] = aPMV[ 0 ][ 1 ][ 1 ] =
  aPMV[ 1 ][ 1 ][ 0 ] = aPMV[ 1 ][ 1 ][ 1 ] = 0;

 if (  ( s_MPEG12Ctx.m_PictCodingType == _MPEG_PT_P ) &&
      !( lMBType & ( _MPEG_MBT_MOTION_FORWARD | _MPEG_MBT_INTRA )  )
 ) {

  aPMV[ 0 ][ 0 ][ 0 ] = aPMV[ 0 ][ 0 ][ 1 ] =
  aPMV[ 1 ][ 0 ][ 0 ] = aPMV[ 1 ][ 0 ][ 1 ] = 0;

  if ( s_MPEG12Ctx.m_PictStruct == _MPEG_PS_FRAME )
   lMotionType = _MPEG_MC_FRAME;
  else {
   lMotionType = _MPEG_MC_FIELD;
   aMVFS[ 0 ][ 0 ] = s_MPEG12Ctx.m_PictStruct == _MPEG_PS_BOTTOM_FIELD;
  }  /* end else */

 }  /* end if */

 *apMBType     = lMBType;
 *apMotionType = lMotionType;

 return 1;

}  /* end _mpeg12_dec_mb */

static void _mpeg12_get_ref (
             _MPEGMacroBlock8* apMBSrc, int aX, int anY,
             int aDX, int aDY, int aH, int aFSrc, int aFDst, int afAvg
            ) {

 int          lfInt    = ( aH & 8 ) >> 3;
 int          lDXY     = (  ( aDY & 1 ) << 1  ) | ( aDX & 1 );
 int          lUVXY    = (  ( aDX & 2 ) >> 1  ) | ( aDY & 2 );
 int          lSrcX    = aX  + ( aDX >> 1 );
 int          lSrcY    = (   (  anY + ( aDY >> 1 )  ) << lfInt   ) + aFSrc;
 int          lMBX     = lSrcX >> 4;
 int          lMBY     = lSrcY >> 4;
 _MPEGMotion* lpMotion = &s_MPEG12Ctx.m_pCurMotions -> m_Motion[ s_MPEG12Ctx.m_pCurMotions -> m_nMotions++ ];

 afAvg <<= 2;

 __asm__ __volatile__(
  ".set noat\n\t"
  "pnor     $v0, $zero, $zero\n\t"
  "ld       $at, %4\n\t"
  "pextlw   %0, %3, %0\n\t"
  "paddw    $at, $at, $v0\n\t"
  "pmaxw    %0, %0, $zero\n\t"
  "pminw    %0, %0, $at\n\t"
  "dsrl32   %1, %0, 0\n\t"
  "sll      %0, %0, 0\n\t"
  ".set at\n\t"
  : "=r"( lMBX ), "=r"( lMBY ) : "r"( lMBX ), "r"( lMBY ), "m"( s_MPEG12Ctx.m_MBWidth ) : "at", "v0"
 );
 
 lpMotion -> m_pSrc     = ( unsigned char* )(  apMBSrc + lMBX + lMBY * s_MPEG12Ctx.m_MBWidth  );
 lpMotion -> m_pDstY    = ( short* )(  s_MPEG12Ctx.m_pCurMotions -> m_pSPRRes       + ( aFDst << 5 )  );
 lpMotion -> m_pDstCbCr = ( short* )(  s_MPEG12Ctx.m_pCurMotions -> m_pSPRRes + 512 + ( aFDst << 3 )  );
 lpMotion -> m_X        = lSrcX & 0xF;
 lpMotion -> m_Y        = lSrcY & 0xF;
 lpMotion -> m_H        = aH;
 lpMotion -> m_fInt     = lfInt;
 lpMotion -> m_Field    = aFSrc;
 lpMotion -> MC_Luma    = LumaOp  [ lDXY  + afAvg ];
 lpMotion -> MC_Chroma  = ChromaOp[ lUVXY + afAvg ];

}  /* end _mpeg12_get_ref */

static void _mpeg12_get_refs (
             int aBX, int aBY, int aMBType, int aMotionType, int aPMV[ 2 ][ 2 ][ 2 ],
             int aMVFS[ 2 ][ 2 ], int aDMVector[ 2 ]
            ) {

 int lCurField;
 int lDMV[ 2 ][ 2 ];
 int lfAdd = 0;

 s_MPEG12Ctx.m_pCurMotions -> m_nMotions = 0;

 if (  ( aMBType & _MPEG_MBT_MOTION_FORWARD    ) ||
       ( s_MPEG12Ctx.m_PictCodingType == _MPEG_PT_P )
 ) {

  if ( s_MPEG12Ctx.m_PictStruct == _MPEG_PS_FRAME ) {

   if (  ( aMotionType == _MPEG_MC_FRAME      ) ||
        !( aMBType & _MPEG_MBT_MOTION_FORWARD )
   ) {

    _mpeg12_get_ref (
     s_MPEG12Ctx.m_pFwdFrame, aBX, aBY, aPMV[ 0 ][ 0 ][ 0 ], aPMV[ 0 ][ 0 ][ 1 ],
     16, 0, 0, 0
    );

   } else if ( aMotionType == _MPEG_MC_FIELD ) {

    _mpeg12_get_ref (
     s_MPEG12Ctx.m_pFwdFrame, aBX, aBY >> 1, aPMV[ 0 ][ 0 ][ 0 ], aPMV[ 0 ][ 0 ][ 1 ] >> 1,
     8, aMVFS[ 0 ][ 0 ], 0, 0
    );
    _mpeg12_get_ref (
     s_MPEG12Ctx.m_pFwdFrame, aBX, aBY >> 1, aPMV[ 1 ][ 0 ][ 0 ], aPMV[ 1 ][ 0 ][ 1 ] >> 1,
     8, aMVFS[ 1 ][ 0 ], 8, 0
    );

   } else if ( aMotionType == _MPEG_MC_DMV ) {

    _mpeg12_dual_prime_vector ( lDMV, aDMVector, aPMV[ 0 ][ 0 ][ 0 ], aPMV[ 0 ][ 0 ][ 1 ] >> 1 );

    _mpeg12_get_ref (
     s_MPEG12Ctx.m_pFwdFrame, aBX, aBY >> 1, aPMV[ 0 ][ 0 ][ 0 ], aPMV[ 0 ][ 0 ][ 1 ] >> 1,
     8, 0, 0, 0
    );
    _mpeg12_get_ref (
     s_MPEG12Ctx.m_pFwdFrame, aBX, aBY >> 1, lDMV[ 0 ][ 0 ], lDMV[ 0 ][ 1 ],
     8, 1, 0, 1
    );
    _mpeg12_get_ref (
     s_MPEG12Ctx.m_pFwdFrame, aBX, aBY >> 1, aPMV[ 0 ][ 0 ][ 0 ], aPMV[ 0 ][ 0 ][ 1 ] >> 1,
     8, 1, 8, 0
    );
    _mpeg12_get_ref (
     s_MPEG12Ctx.m_pFwdFrame, aBX, aBY >> 1, lDMV[ 1 ][ 0 ], lDMV[ 1 ][ 1 ],
     8, 0, 8, 1
    );

   }  /* end if */

  } else {

   _MPEGMacroBlock8* lpMBSrc;

   lCurField = ( s_MPEG12Ctx.m_PictStruct == _MPEG_PS_BOTTOM_FIELD );
   lpMBSrc   = ( s_MPEG12Ctx.m_PictCodingType == _MPEG_PT_P  ) &&
               s_MPEG12Ctx.m_fSecField                         &&
               ( lCurField != aMVFS[ 0 ][ 0 ]   ) ? s_MPEG12Ctx.m_pBckFrame
                                                  : s_MPEG12Ctx.m_pFwdFrame;

   if (  ( aMotionType == _MPEG_MC_FIELD ) || !( aMBType & _MPEG_MBT_MOTION_FORWARD )  ) {

    _mpeg12_get_ref (
     lpMBSrc, aBX, aBY, aPMV[ 0 ][ 0 ][ 0 ], aPMV[ 0 ][ 0 ][ 1 ], 8,
     aMVFS[ 0 ][ 0 ], 0, 0
    );
    _mpeg12_get_ref (
     lpMBSrc, aBX, aBY + 8, aPMV[ 0 ][ 0 ][ 0 ], aPMV[ 0 ][ 0 ][ 1 ], 8,
     aMVFS[ 0 ][ 0 ], 8, 0
    );

   } else if ( aMotionType == _MPEG_MC_16X8 ) {

    _mpeg12_get_ref (
     lpMBSrc, aBX, aBY, aPMV[ 0 ][ 0 ][ 0 ], aPMV[ 0 ][ 0 ][ 1 ], 8,
     aMVFS[ 0 ][ 0 ], 0, 0
    );

    lpMBSrc = ( s_MPEG12Ctx.m_PictCodingType == _MPEG_PT_P ) &&
              s_MPEG12Ctx.m_fSecField                        &&
              ( lCurField != aMVFS[ 1 ][ 0 ]    ) ? s_MPEG12Ctx.m_pBckFrame
                                                  : s_MPEG12Ctx.m_pFwdFrame;
    _mpeg12_get_ref (
     lpMBSrc, aBX, aBY + 8, aPMV[ 1 ][ 0 ][ 0 ], aPMV[ 1 ][ 0 ][ 1 ], 8,
     aMVFS[ 1 ][ 0 ], 8, 0
    );

   } else if ( aMotionType == _MPEG_MC_DMV ) {

    lpMBSrc = s_MPEG12Ctx.m_fSecField ? s_MPEG12Ctx.m_pBckFrame : s_MPEG12Ctx.m_pFwdFrame;

    _mpeg12_dual_prime_vector ( lDMV, aDMVector, aPMV[ 0 ][ 0 ][ 0 ], aPMV[ 0 ][ 0 ][ 1 ] );

    _mpeg12_get_ref (
     s_MPEG12Ctx.m_pFwdFrame, aBX, aBY, aPMV[ 0 ][ 0 ][ 0 ], aPMV[ 0 ][ 0 ][ 1 ], 16,
     lCurField, 0, 0
    );
    _mpeg12_get_ref (
     lpMBSrc, aBX, aBY, lDMV[ 0 ][ 0 ], lDMV[ 0 ][ 1 ], 16, !lCurField, 1, 1
    );

   }  /* end if */

  }  /* end else */

  lfAdd = 1;

 }  /* end if */

 if ( aMBType & _MPEG_MBT_MOTION_BACKWARD ) {

  if ( s_MPEG12Ctx.m_PictStruct == _MPEG_PS_FRAME ) {

   if ( aMotionType == _MPEG_MC_FRAME ) {

    _mpeg12_get_ref (
     s_MPEG12Ctx.m_pBckFrame, aBX, aBY, aPMV[ 0 ][ 1 ][ 0 ], aPMV[ 0 ][ 1 ][ 1 ],
     16, 0, 0, lfAdd
    );

   } else {

    _mpeg12_get_ref (
     s_MPEG12Ctx.m_pBckFrame, aBX, aBY >> 1, aPMV[ 0 ][ 1 ][ 0 ], aPMV[ 0 ][ 1 ][ 1 ] >> 1,
     8, aMVFS[ 0 ][ 1 ], 0, lfAdd
    );
    _mpeg12_get_ref (
     s_MPEG12Ctx.m_pBckFrame, aBX, aBY >> 1, aPMV[ 1 ][ 1 ][ 0 ], aPMV[ 1 ][ 1 ][ 1 ] >> 1,
     8, aMVFS[ 1 ][ 1 ], 8, lfAdd
    );

   }  /* end else */

  } else {

   if ( aMotionType == _MPEG_MC_FIELD ) {

    _mpeg12_get_ref (
     s_MPEG12Ctx.m_pBckFrame, aBX, aBY, aPMV[ 0 ][ 1 ][ 0 ], aPMV[ 0 ][ 1 ][ 1 ], 8,
     aMVFS[ 0 ][ 1 ], 0, lfAdd
    );
    _mpeg12_get_ref (
     s_MPEG12Ctx.m_pBckFrame, aBX, aBY + 8, aPMV[ 0 ][ 1 ][ 0 ], aPMV[ 0 ][ 1 ][ 1 ], 8,
     aMVFS[ 0 ][ 1 ], 8, lfAdd
    );

   } else if ( aMotionType == _MPEG_MC_16X8 ) {

    _mpeg12_get_ref (
     s_MPEG12Ctx.m_pBckFrame, aBX, aBY, aPMV[ 0 ][ 1 ][ 0 ], aPMV[ 0 ][ 1 ][ 1 ],
     8, aMVFS[ 0 ][ 1 ], 0, lfAdd
    );
    _mpeg12_get_ref (
     s_MPEG12Ctx.m_pBckFrame, aBX, aBY + 8, aPMV[ 1 ][ 1 ][ 0 ], aPMV[ 1 ][ 1 ][ 1 ],
     8, aMVFS[ 1 ][ 1 ], 8, lfAdd
    );

   }  /* end if */

  }  /* end else */

 }  /* end if */

 _MPEG_dma_ref_image (
  ( _MPEGMacroBlock8* )s_MPEG12Ctx.m_pCurMotions -> m_pSPRMC,
  &s_MPEG12Ctx.m_pCurMotions -> m_Motion[ 0 ],
  s_MPEG12Ctx.m_pCurMotions -> m_nMotions, s_MPEG12Ctx.m_MBWidth
 );

}  /* end _mpeg12_get_refs */

static void _mpeg2_mc (
             int aMBA, int aMBType, int aMotionType, int aPMV[ 2 ][ 2 ][ 2 ],
             int aMVFS[ 2 ][ 2 ], int aDMVector[ 2 ], int aMBAI
            ){

 int lBX, lBY;
 int lfField;
 int lfIntra;
 int lOffset;
 int lfNoSkip;
 int lfFiledMV;

 lBX       = s_MPEG12Ctx.m_pMBXY[ aMBA ].m_X;
 lBY       = s_MPEG12Ctx.m_pMBXY[ aMBA ].m_Y;
 lfField   = s_MPEG12Ctx.m_PictStruct != _MPEG_PS_FRAME;
 lOffset   = (  lBY * ( s_MPEG12Ctx.m_MBWidth << lfField ) + lBX  ) * sizeof ( _MPEGMacroBlock8 );
 lfIntra   = aMBType & _MPEG_MBT_INTRA;
 lfNoSkip  = aMBAI == 1;
 lfFiledMV = aMotionType & _MPEG_MC_FIELD;

 s_MPEG12Ctx.m_pCurMotions -> m_Stride     = s_MPEG12Ctx.m_MBStride;
 s_MPEG12Ctx.m_pCurMotions -> m_pMBDstY    = UNCACHED_SEG( s_MPEG12Ctx.m_pCurFrameY    + lOffset );
 s_MPEG12Ctx.m_pCurMotions -> m_pMBDstCbCr = UNCACHED_SEG( s_MPEG12Ctx.m_pCurFrameCbCr + lOffset );

 lBX <<= 4;
 lBY <<= 4;

 if ( !lfIntra ) {
  _mpeg12_get_refs ( lBX, lBY, aMBType, aMotionType, aPMV, aMVFS, aDMVector );
  if (  lfNoSkip && ( aMBType & _MPEG_MBT_PATTERN )  )
   s_MPEG12Ctx.m_pCurMotions -> BlockOp = AddBlockOp[ lfField ][ lfFiledMV ];
  else {
   s_MPEG12Ctx.m_pCurMotions -> m_pSrc  = s_MPEG12Ctx.m_pCurMotions -> m_pSPRRes;
   s_MPEG12Ctx.m_pCurMotions -> BlockOp = PutBlockOp[ !lfField + ( lfNoSkip && lfFiledMV && !lfField ) ];
  }  /* end else */
 } else {
  s_MPEG12Ctx.m_pCurMotions -> m_Motion[ 0 ].MC_Luma = NULL;
  s_MPEG12Ctx.m_pCurMotions -> m_pSrc  = s_MPEG12Ctx.m_pCurMotions -> m_pSPRBlk;
  s_MPEG12Ctx.m_pCurMotions -> BlockOp = PutBlockOp[ !lfField ];
 }  /* end else */

}  /* end _mpeg2_mc */

static int _mpeg12_slice ( int aMBAMax ) {

 int lPMV     [ 2 ][ 2 ][ 2 ];
 int lMVFS    [ 2 ][ 2 ];
 int lDMVector[ 2 ];
 int lMBA, lMBAI, lMBType, lMotionType;
 int retVal;

 s_MPEG12Ctx.m_fError = 0;
 lMBType              = _MPEG_NextStartCode ();

 if ( lMBType < _MPEG_CODE_SLICE_MIN || lMBType > _MPEG_CODE_SLICE_MAX ) return -1;

 _MPEG_GetBits ( 32 );

 s_MPEG12Ctx.m_QScale = _MPEG_GetBits ( 5 );

 if (  _MPEG_GetBits ( 1 )  ) {
  _MPEG_GetBits ( 8 );
  _xtra_bitinf ();
 }  /* end if */

 lMBAI = _MPEG_GetMBAI ();

 if ( lMBAI ) {

  lMBA = (  ( lMBType & 255 ) - 1  ) * s_MPEG12Ctx.m_MBWidth + lMBAI - 1;

  lMBAI                =
  s_MPEG12Ctx.m_fDCRst = 1;
  
  lPMV[ 0 ][ 0 ][ 0 ] = lPMV[ 0 ][ 0 ][ 1 ] = lPMV[ 1 ][ 0 ][ 0 ] = lPMV[ 1 ][ 0 ][ 1 ] = 0;
  lPMV[ 0 ][ 1 ][ 0 ] = lPMV[ 0 ][ 1 ][ 1 ] = lPMV[ 1 ][ 1 ][ 0 ] = lPMV[ 1 ][ 1 ][ 1 ] = 0;

 } else return 0;

 while ( 1 ) {

  s_MPEG12Ctx.m_pCurMotions = &s_MPEG12Ctx.m_MC[ s_MPEG12Ctx.m_CurMC ];

  if (  lMBA >= aMBAMax || !_MPEG_WaitBDEC ()  ) return -1;

  if ( !lMBAI ) {

   if (  !_MPEG_ShowBits ( 23 ) || s_MPEG12Ctx.m_fError  ) {
resync:
    s_MPEG12Ctx.m_fError = 0;
    return 0;

   } else {

    lMBAI = _MPEG_GetMBAI ();

    if ( !lMBAI ) goto resync;

   }  /* end else */

  }  /* end if */

  if ( lMBA >= aMBAMax ) return -1;

  if ( lMBAI == 1 ) {

   retVal = _mpeg12_dec_mb ( &lMBType, &lMotionType, lPMV, lMVFS, lDMVector );

   if ( retVal < 0 ) return retVal;
   if ( !retVal    ) goto resync;

  } else {  /* skipped macroblock */

   s_MPEG12Ctx.m_fDCRst = 1;

   if ( s_MPEG12Ctx.m_PictCodingType == _MPEG_PT_P )
    lPMV[ 0 ][ 0 ][ 0 ] = lPMV[ 0 ][ 0 ][ 1 ] =
    lPMV[ 1 ][ 0 ][ 0 ] = lPMV[ 1 ][ 0 ][ 1 ] = 0;
   if ( s_MPEG12Ctx.m_PictStruct == _MPEG_PS_FRAME )
    lMotionType = _MPEG_MC_FRAME;
   else {
    lMotionType = _MPEG_MC_FIELD;
    lMVFS[ 0 ][ 0 ] =
    lMVFS[ 0 ][ 1 ] = s_MPEG12Ctx.m_PictStruct == _MPEG_PS_BOTTOM_FIELD;
   }  /* end else */

   lMBType &= ~_MPEG_MBT_INTRA;

  }  /* end else */

  _mpeg2_mc ( lMBA, lMBType, lMotionType, lPMV, lMVFS, lDMVector, lMBAI );

  DoMC ();

  ++lMBA;
  --lMBAI;

  s_MPEG12Ctx.m_CurMC ^= 1;
 
  if ( lMBA >= aMBAMax ) return -1;

 }  /* end while */

}  /* end _mpeg12_slice */

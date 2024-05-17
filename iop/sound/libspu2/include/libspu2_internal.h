/*
# _____     ___ ____     ___ ____
#  ____|   |    ____|   |        | |____|
# |     ___|   |____ ___|    ____| |    \    PS2DEV Open Source Project.
#-----------------------------------------------------------------------
# Copyright ps2dev - http://www.ps2dev.org
# Licenced under Academic Free License version 2.0
# Review ps2sdk README & LICENSE files for further details.
*/

#ifndef _LIBSPU2_INTERNAL_H
#define _LIBSPU2_INTERNAL_H

#include <intrman.h>
#include <loadcore.h>
#include <stdio.h>
#include <string.h>
#include <sysmem.h>
#include <tamtypes.h>

#include <libspu2.h>

typedef struct libspu2_reverb_param_entry_
{
	u32 flags;
	u16 dAPF1;
	u16 dAPF2;
	u16 vIIR;
	u16 vCOMB1;
	u16 vCOMB2;
	u16 vCOMB3;
	u16 vCOMB4;
	u16 vWALL;
	u16 vAPF1;
	u16 vAPF2;
	u16 mLSAME;
	u16 mRSAME;
	u16 mLCOMB1;
	u16 mRCOMB1;
	u16 mLCOMB2;
	u16 mRCOMB2;
	u16 dLSAME;
	u16 dRSAME;
	u16 mLDIFF;
	u16 mRDIFF;
	u16 mLCOMB3;
	u16 mRCOMB3;
	u16 mLCOMB4;
	u16 mRCOMB4;
	u16 dLDIFF;
	u16 dRDIFF;
	u16 mLAPF1;
	u16 mRAPF1;
	u16 mLAPF2;
	u16 mRAPF2;
	u16 vLIN;
	u16 vRIN;
} libspu2_reverb_param_entry_t;

typedef struct libspu2_malloc_
{
	u32 addr_area;
	u32 size_area;
} libspu2_malloc_t;

extern vu16 *_spu_RXX;
extern u32 _spu_tsa[2];
extern u32 _spu_transMode;
extern u32 _spu_inTransfer;
extern SpuTransferCallbackProc _spu_transferCallback;
extern SpuTransferCallbackProc _spu_AutoDMACallback;
extern SpuIRQCallbackProc _spu_IRQCallback;

extern u32 _spu_keystat[2];
extern u32 _spu_trans_mode;
extern u32 _spu_rev_flag;
extern u32 _spu_rev_reserve_wa;
extern u32 _spu_rev_offsetaddr;
extern SpuReverbAttr _spu_rev_attr;
extern u32 _spu_RQvoice;
extern u32 _spu_RQmask;
extern s16 _spu_voice_centerNote[2][24];
extern u32 _spu_env;
extern u32 _spu_isCalled;
extern SpuIRQCallbackProc _spu_irq_callback;

// extern int _spu_eea[4];

extern int _spu_AllocBlockNum;
extern int _spu_AllocLastNum;
extern libspu2_malloc_t *_spu_memList;

extern u32 _spu_zerobuf[256];

extern s32 _spu_rev_workareasize[12];

extern libspu2_reverb_param_entry_t _spu_rev_param[10];

extern u16 _spu_RQ[16];

extern int _spu_core;

extern u16 gDMADeliverEvent;

extern void _spu2_config_iop(void);
extern void _spu2_config_initialize(void);
extern void _spu2_config_initialize_typically(void);
extern void _spu2_config_initialize_hot(void);
extern void _spu2_config_before_compatible(void);
extern int _spu_init(int flag);
extern int spu_do_set_DmaCoreIndex(int dma_core_index);
extern int spu_do_get_DmaCoreIndex(void);
extern int _spu_FiDMA(void *userdata);
extern int _spu_FiAutoDMA(void *userdata);
extern void _spu_Fr_(void *data, int addr, u32 size);
extern int _spu_t(int count, ...);
extern int _spu_Fw(void *addr, u32 size);
extern int _spu_StopAutoDMA(void);
extern int _spu_AutoDMAGetStatus(void);
extern unsigned int _spu_FwAutoDMA(u8 *addr, unsigned int size, int mode);
extern unsigned int _spu_FwAutoDMAfrom(u8 *addr, unsigned int size, int mode, u8 *unk_a4);
extern void _spu_Fr(void *addr, u32 size);
extern void _spu_MGFsetRXX2(int offset, int value);
extern void _spu_FsetRXX(int l, u32 addr, int flag);
extern int _spu_FsetRXXa(int l, u32 flag);
extern int _spu_MGFgetRXX2(int offset);
extern void _spu_FsetPCR(int flag);
extern void _spu_Fw1ts(void);
extern void _SpuCallback(SpuIRQCallbackProc cb);
extern void _SpuDataCallback(int (*callback)(void *userdata));
extern void _SpuAutoDMACallback(int (*callback)(void *userdata));
extern void _SpuInit(int mode);
extern int _SpuDefaultCallback(void *userdata);
extern void SpuStopFreeRun(void);
extern void _spu_gcSPU(void);
extern int _SpuIsInAllocateArea(u32 addr);
extern int _SpuIsInAllocateArea_(u32 addr);
extern void _spu_print(void);
extern unsigned int _SpuSetAnyVoice(int on_off_flags, unsigned int voice_bits, int word_idx1, int word_idx2);
extern unsigned int _SpuGetAnyVoice(int word_idx1, int word_idx2);
extern void _spu_setReverbAttr(const libspu2_reverb_param_entry_t *p_rev_param_entry);
extern void _spu_setInTransfer(int mode);
extern int _spu_getInTransfer(void);
extern u16 _spu_note2pitch(u16 cen_note_high, u16 cen_note_low, u16 note_high, u16 note_low);
extern int _spu_pitch2note(s16 note_high, s16 note_low, u16 pitch);
extern void _SpuStCBPrepare(void);
extern int IntFunc(void);
extern void _SpuStCB_IRQfinal(void);

#endif

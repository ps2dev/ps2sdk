/**
 * @file
 * Common definitions for libsd on the EE and IOP
 */

#ifndef __LIBSD_COMMON_H__
#define __LIBSD_COMMON_H__

#include <tamtypes.h>

#define SD_VOICE(_core,_v)   ((_core)|((_v)<<1))

/* Macros to set ADSR: i=increment, d=decrement */
#define SD_ADSR_AR_LINEARi      0
#define SD_ADSR_AR_EXPi         1
#define SD_ADSR_SR_LINEARi      0
#define SD_ADSR_SR_LINEARd      2
#define SD_ADSR_SR_EXPi         4
#define SD_ADSR_SR_EXPd         6
#define SD_ADSR_RR_LINEARd      0
#define SD_ADSR_RR_EXPd         1
#define SD_SET_ADSR1(_arm, _ar, _dr, _sl)  ((((_arm)&1)<<15) | (((_ar)&0x7f)<<8) | (((_dr)&0xf)<<4) | ((_sl)&0xf))
#define SD_SET_ADSR2(_srm, _sr, _rrm, _rr) ((((_srm)&7)<<13) | (((_sr)&0x7f)<<6) | (((_rrm)&1)<<5) | ((_rr)&0x1f))

/* Parameters */
#define SD_VPARAM_VOLL				(0x00<<8)
#define SD_VPARAM_VOLR				(0x01<<8)
#define SD_VPARAM_PITCH				(0x02<<8)
#define SD_VPARAM_ADSR1				(0x03<<8)
#define SD_VPARAM_ADSR2				(0x04<<8)
#define SD_VPARAM_ENVX				(0x05<<8)
#define SD_VPARAM_VOLXL				(0x06<<8)
#define SD_VPARAM_VOLXR				(0x07<<8)
#define SD_PARAM_MMIX				(0x08<<8)
#define SD_PARAM_MVOLL				((0x09<<8)|0x80)
#define SD_PARAM_MVOLR				((0x0A<<8)|0x80)
#define SD_PARAM_EVOLL				((0x0B<<8)|0x80)
#define SD_PARAM_EVOLR				((0x0C<<8)|0x80)
#define SD_PARAM_AVOLL				((0x0D<<8)|0x80)
#define SD_PARAM_AVOLR				((0x0E<<8)|0x80)
#define SD_PARAM_BVOLL				((0x0F<<8)|0x80)
#define SD_PARAM_BVOLR				((0x10<<8)|0x80)
#define SD_PARAM_MVOLXL				((0x11<<8)|0x80)
#define SD_PARAM_MVOLXR				((0x12<<8)|0x80)

/* Transfer modes */
#define SD_TRANS_WRITE          0
#define SD_TRANS_READ           1
#define SD_TRANS_STOP           2
#define SD_TRANS_WRITE_FROM     3          /* only for block */
#define SD_TRANS_LOOP           0x10 /* only for block*/
#define SD_TRANS_MODE_DMA       0
#define SD_TRANS_MODE_IO        8

// Reverb
#define SD_EFFECT_MODE_OFF  		0x0
#define SD_EFFECT_MODE_ROOM			0x1
#define SD_EFFECT_MODE_STUDIO_1		0x2
#define SD_EFFECT_MODE_STUDIO_2		0x3
#define SD_EFFECT_MODE_STUDIO_3		0x4
#define SD_EFFECT_MODE_HALL			0x5
#define SD_EFFECT_MODE_SPACE		0x6
#define SD_EFFECT_MODE_ECHO			0x7
#define SD_EFFECT_MODE_DELAY		0x8
#define SD_EFFECT_MODE_PIPE			0x9
#define SD_EFFECT_MODE_CLEAR		0x100

// CoreAttr
#define SD_CORE_EFFECT_ENABLE		0x2
#define SD_CORE_IRQ_ENABLE			0x4
#define SD_CORE_MUTE_ENABLE			0x6
#define SD_CORE_NOISE_CLK			0x8
#define SD_CORE_SPDIF_MODE			0xA

/* Switches */
#define SD_SWITCH_PMON              (0x13<<8)
#define SD_SWITCH_NON               (0x14<<8)
#define SD_SWITCH_KON               (0x15<<8)
#define SD_SWITCH_KEYDOWN SD_SWITCH_KON //For backward-compatibility
#define SD_SWITCH_KOFF              (0x16<<8)
#define SD_SWITCH_KEYUP SD_SWITCH_KOFF //For backward-compatibility
#define SD_SWITCH_ENDX              (0x17<<8)
#define SD_SWITCH_VMIXL             (0x18<<8)
#define SD_SWITCH_VMIXEL            (0x19<<8)
#define SD_SWITCH_VMIXR             (0x1A<<8)
#define SD_SWITCH_VMIXER            (0x1B<<8)

/* Addresses */
#define SD_ADDR_ESA					(0x1C<<8)
#define SD_ADDR_EEA					(0x1D<<8)
#define SD_ADDR_TSA					(0x1E<<8)
#define SD_ADDR_IRQA				(0x1F<<8)
#define SD_VADDR_SSA                (0x20<<8)
#define SD_VADDR_LSAX               (0x21<<8)
#define SD_VADDR_NAX                (0x22<<8)

// SD_CORE_ATTR Macros
#define SD_SPU2_ON					(1 << 15)
#define SD_MUTE						(1 << 14)
#define SD_NOISE_CLOCK(c)			((c & 0x1F) << 8) // Bits 8..13 is noise clock
#define SD_ENABLE_EFFECTS			(1 << 7)
#define SD_ENABLE_IRQ				(1 << 6)
#define SD_DMA_IO					(1 << 4)
#define SD_DMA_WRITE				(2 << 4)
#define SD_DMA_READ					(3 << 4)
#define SD_DMA_IN_PROCESS			(3 << 4) // If either of the DMA bits are set, the DMA channel is occupied.
#define SD_CORE_DMA					(3 << 4)
#define SD_ENABLE_EX_INPUT			(1 << 0) // Enable external input, Not sure.

// SD_C_STATX
#define SD_IO_IN_PROCESS			(1 << 10)

// Batch
#define SD_BATCH_SETPARAM       0x1
#define SD_BATCH_SETSWITCH      0x2
#define SD_BATCH_SETADDR        0x3
#define SD_BATCH_SETCORE        0x4
#define SD_BATCH_WRITEIOP       0x5
#define SD_BATCH_WRITEEE        0x6
#define SD_BATCH_EERETURN       0x7
#define SD_BATCH_GETPARAM       0x10
#define SD_BATCH_GETSWITCH      0x12
#define SD_BATCH_GETADDR        0x13
#define SD_BATCH_GETCORE        0x14

#define ADPCM_LOOP_START    4  /* Set on first block of looped data */
#define ADPCM_LOOP          2  /* Set on all blocks (?that are inside the loop?) */
#define ADPCM_LOOP_END      1  /* Set on last block to loop */

typedef struct
{
	u16 func;
	u16 entry;
	u32 value;
} sceSdBatch;

typedef struct
{
	int     core;
	int     mode;
	short   depth_L;
	short   depth_R;
	int     delay;
	int     feedback;
} sceSdEffectAttr;

typedef int (*sceSdSpu2IntrHandler)(int, void *);
typedef int (*sceSdTransIntrHandler)(int, void *);
typedef int (*SdIntrCallback)(void *data);

#ifdef __cplusplus
extern "C" {
#endif

int sceSdQuit();
int sceSdInit(int flag);
SdIntrCallback sceSdSetIRQCallback(SdIntrCallback cb);
SdIntrCallback sceSdSetTransCallback(s32 core, SdIntrCallback cb);

void sceSdSetParam(u16 entry, u16 value);
u16 sceSdGetParam(u16 entry);

void sceSdSetCoreAttr(u16 entry, u16 value );
u16 sceSdGetCoreAttr(u16 entry );
int sceSdClearEffectWorkArea (int core, int channel, int effect_mode);

void sceSdSetAddr(u16 entry, u32 value);
u32 sceSdGetAddr(u16 entry );

void sceSdSetSwitch(u16 entry, u32 value);
u32 sceSdGetSwitch(u16 entry );

u16 sceSdNote2Pitch (u16 center_note, u16 center_fine, u16 note, s16 fine);
u16 sceSdPitch2Note (u16 center_note, u16 center_fine, u16 pitch);

int sceSdSetEffectAttr (int core, sceSdEffectAttr *attr );
void sceSdGetEffectAttr (int core, sceSdEffectAttr *attr );

int sceSdProcBatch(sceSdBatch *batch, u32 *rets, u32 num);
int sceSdProcBatchEx(sceSdBatch *batch, u32 *rets, u32 num, u32 voice);

int sceSdVoiceTrans(s16 chan, u16 mode, u8 *iopaddr, u32 *spuaddr, u32 size);
int sceSdBlockTrans(s16 chan, u16 mode, u8 *iopaddr, u32 size, ...);
u32 sceSdVoiceTransStatus (s16 channel, s16 flag);
u32 sceSdBlockTransStatus (s16 channel, s16 flag);

sceSdTransIntrHandler sceSdSetTransIntrHandler(int channel, sceSdTransIntrHandler func, void *arg);
sceSdSpu2IntrHandler sceSdSetSpu2IntrHandler(sceSdSpu2IntrHandler func, void *arg);

void *sceSdGetTransIntrHandlerArgument(int arg);
void *sceSdGetSpu2IntrHandlerArgument();
int sceSdStopTrans(int channel);
int sceSdCleanEffectWorkArea(int core, int channel, int effect_mode);
int sceSdSetEffectMode(int core, sceSdEffectAttr *param);
int sceSdSetEffectModeParams(int core, sceSdEffectAttr *attr);

#ifdef __cplusplus
}
#endif

//Backwards compatibility definitions
#define BATCH_SETPARAM SD_BATCH_SETPARAM
#define BATCH_SETSWITCH SD_BATCH_SETSWITCH
#define BATCH_SETADDR SD_BATCH_SETADDR
#define BATCH_SETCORE SD_BATCH_SETCORE
#define BATCH_WRITEIOP SD_BATCH_WRITEIOP
#define BATCH_WRITEEE SD_BATCH_WRITEEE
#define BATCH_EERETURN SD_BATCH_EERETURN
#define BATCH_GETPARAM SD_BATCH_GETPARAM
#define BATCH_GETSWITCH SD_BATCH_GETSWITCH
#define BATCH_GETADDR SD_BATCH_GETADDR
#define BATCH_GETCORE SD_BATCH_GETCORE

#define SD_BLOCK_TRANS_WRITE SD_TRANS_WRITE
#define SD_BLOCK_TRANS_READ SD_TRANS_READ
#define SD_BLOCK_TRANS_STOP SD_TRANS_STOP
#define SD_BLOCK_TRANS_WRITE_FROM SD_TRANS_WRITE_FROM
#define SD_BLOCK_TRANS_LOOP SD_TRANS_LOOP

#define SD_VOICE_TRANS_WRITE SD_TRANS_WRITE
#define SD_VOICE_TRANS_READ SD_TRANS_READ
#define SD_VOICE_TRANS_STOP SD_TRANS_STOP
#define	SD_VOICE_TRANS_MODE_DMA SD_TRANS_MODE_DMA
#define SD_VOICE_TRANS_MODE_IO SD_TRANS_MODE_IO

#define SdBatch sceSdBatch
#define SdEffectAttr sceSdEffectAttr
#define SdSpu2IntrHandler sceSdSpu2IntrHandler
#define SdTransIntrHandler sceSdTransIntrHandler
#define IntrCallback SdIntrCallback

#define SdQuit sceSdQuit
#define SdInit sceSdInit
#define SdSetIRQCallback sceSdSetIRQCallback
#define SdSetTransCallback sceSdSetTransCallback
#define SdSetParam sceSdSetParam
#define SdGetParam sceSdGetParam
#define SdSetCoreAttr sceSdSetCoreAttr
#define SdGetCoreAttr sceSdGetCoreAttr
#define SdClearEffectWorkArea sceSdClearEffectWorkArea
#define SdSetAddr sceSdSetAddr
#define SdGetAddr sceSdGetAddr
#define SdSetSwitch sceSdSetSwitch
#define SdGetSwitch sceSdGetSwitch
#define SdNote2Pitch sceSdNote2Pitch
#define SdPitch2Note sceSdPitch2Note
#define SdSetEffectAttr sceSdSetEffectAttr
#define SdGetEffectAttr sceSdGetEffectAttr
#define SdProcBatch sceSdProcBatch
#define SdProcBatchEx sceSdProcBatchEx
#define SdVoiceTrans sceSdVoiceTrans
#define SdBlockTrans sceSdBlockTrans
#define SdVoiceTransStatus sceSdVoiceTransStatus
#define SdBlockTransStatus sceSdBlockTransStatus
#define SdSetTransIntrHandler sceSdSetTransIntrHandler
#define SdSetSpu2IntrHandler sceSdSetSpu2IntrHandler
#define SdGetTransIntrHandlerArgument sceSdGetTransIntrHandlerArgument
#define SdGetSpu2IntrHandlerArgument sceSdGetSpu2IntrHandlerArgument
#define SdStopTrans sceSdStopTrans
#define SdCleanEffectWorkArea sceSdCleanEffectWorkArea
#define SdSetEffectMode sceSdSetEffectMode
#define SdSetEffectModeParams sceSdSetEffectModeParams

#endif /* __LIBSD_COMMON_H__ */

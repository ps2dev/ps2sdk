#ifndef __FREESD_H__
#define __FREESD_H__

// Param
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


// BlockTrans
#define SD_BLOCK_TRANS_WRITE		0
#define SD_BLOCK_TRANS_READ			1
#define SD_BLOCK_TRANS_STOP			2
#define SD_BLOCK_TRANS_WRITE_FROM	3
#define SD_BLOCK_TRANS_LOOP			0x10

// VoiceTrans
#define SD_VOICE_TRANS_WRITE		0
#define SD_VOICE_TRANS_READ			1
#define SD_VOICE_TRANS_STOP			2
#define	SD_VOICE_TRANS_MODE_DMA		0
#define SD_VOICE_TRANS_MODE_IO		8
 
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

// Addr
#define SD_ADDR_ESA					(0x1C<<8)
#define SD_ADDR_EEA					(0x1D<<8)
#define SD_ADDR_TSA					(0x1E<<8)
#define SD_ADDR_IRQA				(0x1F<<8)

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
#define BATCH_SETPARAM				0x1
#define BATCH_SETSWITCH				0x2
#define BATCH_SETADDR				0x3
#define BATCH_SETCORE				0x4
#define BATCH_WRITEIOP				0x5
#define BATCH_WRITEEE				0x6
#define BATCH_EERETURN				0x7
#define BATCH_GETPARAM				0x10
#define BATCH_GETSWITCH				0x12
#define BATCH_GETADDR				0x13
#define BATCH_GETCORE				0x14

typedef struct
{	
	u32 mode;
	void *data;
} IntrData;


typedef struct _SdEffectAttr 
{
  s32 core; // not used.
  s32 mode;
  s16 depth_l;
  s16 depth_r;
  s32 delay;
  s32 feedback;
} SdEffectAttr __attribute__((packed));

typedef struct _SdBatch

{
  u16 func;
  u16 entry;
  u32 val;
} SdBatch __attribute__((packed));

typedef int (*SdSpu2IntrHandler)(int core, void *data);
typedef int (*SdTransIntrHandler)(int channel, void *data);
typedef int (*IntrCallback)(void *data);

s32 SdQuit();

s32 SdInit(s32 flag);

void SdSetParam(u16 entry, u16 value);
u16 SdGetParam(u16 entry);

void SdSetSwitch(u16 entry, u32 value );
u32	SdGetSwitch(u16 entry );

void SdSetAddr(u16 reg, u32 val);
u32	SdGetAddr(u16 entry );

void SdSetCoreAttr(u16 entry, u16 value );
u16	SdGetCoreAttr(u16 entry );

u16	SdNote2Pitch(u16 center_note, u16 center_fine, u16 note, s16 fine);
u16	SdPitch2Note(u16 center_note, u16 center_fine, u16 pitch);

s32	SdProcBatch(SdBatch* batch, u32 returns[], u32 num  );
s32	SdProcBatchEx(SdBatch* batch, u32 returns[], u32 num, u32 voice  );


s32 SdVoiceTrans(s16 chan, u16 mode, u8 *iopaddr, u32 *spuaddr, u32 size);
s32 SdBlockTrans(s16 chan, u16 mode, u8 *iopaddr, u32 size, u8 *startaddr);

u32	SdVoiceTransStatus (s16 channel, s16 flag);
u32	SdBlockTransStatus (s16 channel, s16 flag);

IntrCallback SdSetTransCallback(s32 core, IntrCallback cb);
IntrCallback SdSetIRQCallback(IntrCallback cb);

s32	SdSetEffectAttr (s32 core, SdEffectAttr *attr );
void SdGetEffectAttr (s32 core, SdEffectAttr *attr );
s32	SdClearEffectWorkArea (s32 core, s32 channel, s32 effect_mode );

SdTransIntrHandler SdSetTransIntrHandler(s32 channel, SdTransIntrHandler func, void *arg);
SdSpu2IntrHandler SdSetSpu2IntrHandler(SdSpu2IntrHandler func, void *arg);

#endif

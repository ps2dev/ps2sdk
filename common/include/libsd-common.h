#ifndef __LIBSD_COMMON_H
#define __LIBSD_COMMON_H 1


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
#define SD_SWITCH_KEYDOWN           (0x15<<8)
#define SD_SWITCH_KEYUP             (0x16<<8)
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
} SdBatch;

typedef struct
{
     int     core;
     int     mode;
     short   depth_L;
     short   depth_R;
     int     delay;
     int     feedback;
} SdEffectAttr;

typedef int (*SdIntrHandler)(int, void *);

#endif /* __LIBSD_H */


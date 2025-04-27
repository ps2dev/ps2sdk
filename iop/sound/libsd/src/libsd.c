/*
# _____     ___ ____     ___ ____
#  ____|   |    ____|   |        | |____|
# |     ___|   |____ ___|    ____| |    \    PS2DEV Open Source Project.
#-----------------------------------------------------------------------
# Copyright ps2dev - http://www.ps2dev.org
# Licenced under Academic Free License version 2.0
# Review ps2sdk README & LICENSE files for further details.
*/

/**
 * @file
 * Part of the IOP Sound Driver
 */

#include <irx_imports.h>

#include <iop_mmio_hwport.h>
#include <libsd.h>
#include <spu2_mmio_hwport.h>
#include <stdarg.h>

#include <spu2regs.h>

IRX_ID("Sound_Device_Library", 3, 3);
// Based on the module from SDK 3.1.0.

#define SD_DMA_CS (1 << 9)  // Continuous stream
#define SD_DMA_START (1 << 24)
#define SD_DMA_DIR_SPU2IOP 0
#define SD_DMA_DIR_IOP2SPU 1

#define SD_CORE_0 0
#define SD_CORE_1 1

#define SD_INTERNAL_MMIO_ATTR 0x2300
#define SD_INTERNAL_MMIO_STD 0x2500
#define SD_INTERNAL_MMIO_UNK1AE 0x2600
#define SD_INTERNAL_MMIO_ADMAS 0x2700
#define SD_INTERNAL_MMIO_STATX 0x2800

extern struct irx_export_table _exp_libsd;

struct mode_data_struct
{
	u32 m_mode_flags;
	u16 m_d_apf1_size;
	u16 m_d_apf2_size;
	u16 m_d_iir_vol;
	u16 m_d_comb1_vol;
	u16 m_d_comb2_vol;
	u16 m_d_comb3_vol;
	u16 m_d_comb4_vol;
	u16 m_d_wall_vol;
	u16 m_d_apf1_vol;
	u16 m_d_apf2_vol;
	u16 m_d_same_l_dst;
	u16 m_d_same_r_dst;
	u16 m_d_comb1_l_src;
	u16 m_d_comb1_r_src;
	u16 m_d_comb2_l_src;
	u16 m_d_comb2_r_src;
	u16 m_d_same_l_src;
	u16 m_d_same_r_src;
	u16 m_d_diff_l_dst;
	u16 m_d_diff_r_dst;
	u16 m_d_comb3_l_src;
	u16 m_d_comb3_r_src;
	u16 m_d_comb4_l_src;
	u16 m_d_comb4_r_src;
	u16 m_d_diff_l_src;
	u16 m_d_diff_r_src;
	u16 m_d_apf1_l_dst;
	u16 m_d_apf1_r_dst;
	u16 m_d_apf2_l_dst;
	u16 m_d_apf2_r_dst;
	u16 m_d_in_coef_l;
	u16 m_d_in_coef_r;
};

typedef struct
{
	u32 m_mode;
	void *m_data;
} IntrData;

typedef struct BlockHandlerIntrData_
{
	sceSdBlockTransHandler m_cb;
	void *m_userdata;
} BlockHandlerIntrData_t;

typedef struct CleanRegionBufferElement_
{
	u32 *m_spuaddr;
	u32 m_size;
} CleanRegionBufferElement_t;

typedef struct CleanRegionBuffer_
{
	CleanRegionBufferElement_t m_elements[97];
} CleanRegionBuffer_t;

typedef int (*SdCleanHandler)(int core);

static int GetEEA(int core);
static void InitSpu2_Inner(void);
static void libsd_do_busyloop(int count);
static u32 DmaStartStop(int mainarg, void *vararg2, u32 vararg3);
static u32 VoiceTrans_Write_IOMode(const u16 *iopaddr, u32 size, int core);
static u32 BlockTransWriteFrom(u8 *iopaddr, u32 size, int core, int mode, u8 *startaddr);
static u32 BlockTransRead(u8 *iopaddr, u32 size, int core, u16 mode);
static void reset_vars(void);

// clang-format off
static vu16 *const g_ParamRegList[] =
{
	SD_VP_VOLL(0, 0),
	SD_VP_VOLR(0, 0),
	SD_VP_PITCH(0, 0),
	SD_VP_ADSR1(0, 0),
	SD_VP_ADSR2(0, 0),
	SD_VP_ENVX(0, 0),
	SD_VP_VOLXL(0, 0),
	SD_VP_VOLXR(0, 0),
	SD_P_MMIX(0),
	SD_P_MVOLL(0),
	SD_P_MVOLR(0),
	SD_P_EVOLL(0),
	SD_P_EVOLR(0),
	SD_P_AVOLL(0),
	SD_P_AVOLR(0),
	SD_P_BVOLL(0),
	SD_P_BVOLR(0),
	SD_P_MVOLXL(0),
	SD_P_MVOLXR(0),
	SD_S_PMON_HI(0),
	SD_S_NON_HI(0),
	SD_A_KON_HI(0),
	SD_A_KOFF_HI(0),
	SD_S_ENDX_HI(0),
	SD_S_VMIXL_HI(0),
	SD_S_VMIXEL_HI(0),
	SD_S_VMIXR_HI(0),
	SD_S_VMIXER_HI(0),
	SD_A_ESA_HI(0),
	SD_A_EEA_HI(0),
	SD_A_TSA_HI(0),
	SD_CORE_IRQA(0),
	SD_VA_SSA_HI(0, 0),
	SD_VA_LSAX(0, 0),
	SD_VA_NAX(0, 0),
	SD_CORE_ATTR(0),
	SD_A_TSA_HI(0),
	SD_A_STD(0),
	// 1AE & 1B0 are both related to core attr & dma somehow
	(vu16 *)0xBF9001AE,
	(vu16 *)0xBF9001B0,
	(vu16 *)0xBF900344,
	NULL,
	NULL,
	NULL,
};
// The values are more or less the same as on PSX (SPU)
static const u32 g_EffectSizes[] =
{
	0x2,
	0x4d8,
	0x3e8,
	0x908,
	0xdfc,
	0x15bc,
	0x1ed8,
	0x3008,
	0x3008,
	0x780,
};
static const struct mode_data_struct g_EffectParams[] =
{
	// SD_EFFECT_MODE_OFF
	{
		0x0,
		0x0,
		0x0,
		0x0,
		0x0,
		0x0,
		0x0,
		0x0,
		0x0,
		0x0,
		0x0,
		0x1,
		0x1,
		0x0,
		0x0,
		0x0,
		0x0,
		0x0,
		0x0,
		0x1,
		0x1,
		0x0,
		0x0,
		0x0,
		0x0,
		0x0,
		0x0,
		0x0,
		0x0,
		0x0,
		0x0,
		0x0,
		0x0,
	},
	// SD_EFFECT_MODE_ROOM
	{
		0x0,
		0x7d,
		0x5b,
		0x6d80,
		0x54b8,
		0xbed0,
		0x0,
		0x0,
		0xba80,
		0x5800,
		0x5300,
		0x4d6,
		0x333,
		0x3f0,
		0x227,
		0x374,
		0x1ef,
		0x336,
		0x1b7,
		0x335,
		0x1b6,
		0x334,
		0x1b5,
		0x334,
		0x1b5,
		0x334,
		0x1b5,
		0x1b4,
		0x136,
		0xb8,
		0x5c,
		0x8000,
		0x8000,
	},
	// SD_EFFECT_MODE_STUDIO_1
	{
		0x0,
		0x33,
		0x25,
		0x70f0,
		0x4fa8,
		0xbce0,
		0x4410,
		0xc0f0,
		0x9c00,
		0x5280,
		0x4ec0,
		0x3e4,
		0x31b,
		0x3a4,
		0x2af,
		0x372,
		0x266,
		0x31c,
		0x25d,
		0x25c,
		0x18e,
		0x22f,
		0x135,
		0x1d2,
		0xb7,
		0x18f,
		0xb5,
		0xb4,
		0x80,
		0x4c,
		0x26,
		0x8000,
		0x8000,
	},
	// SD_EFFECT_MODE_STUDIO_2
	{
		0x0,
		0xb1,
		0x7f,
		0x70f0,
		0x4fa8,
		0xbce0,
		0x4510,
		0xbef0,
		0xb4c0,
		0x5280,
		0x4ec0,
		0x904,
		0x76b,
		0x824,
		0x65f,
		0x7a2,
		0x616,
		0x76c,
		0x5ed,
		0x5ec,
		0x42e,
		0x50f,
		0x305,
		0x462,
		0x2b7,
		0x42f,
		0x265,
		0x264,
		0x1b2,
		0x100,
		0x80,
		0x8000,
		0x8000,
	},
	// SD_EFFECT_MODE_STUDIO_3
	{
		0x0,
		0xe3,
		0xa9,
		0x6f60,
		0x4fa8,
		0xbce0,
		0x4510,
		0xbef0,
		0xa680,
		0x5680,
		0x52c0,
		0xdfb,
		0xb58,
		0xd09,
		0xa3c,
		0xbd9,
		0x973,
		0xb59,
		0x8da,
		0x8d9,
		0x5e9,
		0x7ec,
		0x4b0,
		0x6ef,
		0x3d2,
		0x5ea,
		0x31d,
		0x31c,
		0x238,
		0x154,
		0xaa,
		0x8000,
		0x8000,
	},
	// SD_EFFECT_MODE_HALL
	{
		0x0,
		0x1a5,
		0x139,
		0x6000,
		0x5000,
		0x4c00,
		0xb800,
		0xbc00,
		0xc000,
		0x6000,
		0x5c00,
		0x15ba,
		0x11bb,
		0x14c2,
		0x10bd,
		0x11bc,
		0xdc1,
		0x11c0,
		0xdc3,
		0xdc0,
		0x9c1,
		0xbc4,
		0x7c1,
		0xa00,
		0x6cd,
		0x9c2,
		0x5c1,
		0x5c0,
		0x41a,
		0x274,
		0x13a,
		0x8000,
		0x8000,
	},
	// SD_EFFECT_MODE_SPACE
	{
		0x0,
		0x33d,
		0x231,
		0x7e00,
		0x5000,
		0xb400,
		0xb000,
		0x4c00,
		0xb000,
		0x6000,
		0x5400,
		0x1ed6,
		0x1a31,
		0x1d14,
		0x183b,
		0x1bc2,
		0x16b2,
		0x1a32,
		0x15ef,
		0x15ee,
		0x1055,
		0x1334,
		0xf2d,
		0x11f6,
		0xc5d,
		0x1056,
		0xae1,
		0xae0,
		0x7a2,
		0x464,
		0x232,
		0x8000,
		0x8000,
	},
	// SD_EFFECT_MODE_ECHO
	{
		0x0,
		0x3,
		0x3,
		0x7fff,
		0x7fff,
		0x0,
		0x0,
		0x0,
		0x8100,
		0x0,
		0x0,
		0x1ffd,
		0xffd,
		0x1009,
		0x9,
		0x0,
		0x0,
		0x1009,
		0x9,
		0x1fff,
		0x1fff,
		0x1ffe,
		0x1ffe,
		0x1ffe,
		0x1ffe,
		0x1ffe,
		0x1ffe,
		0x1008,
		0x1004,
		0x8,
		0x4,
		0x8000,
		0x8000,
	},
	// SD_EFFECT_MODE_DELAY
	{
		0x0,
		0x3,
		0x3,
		0x7fff,
		0x7fff,
		0x0,
		0x0,
		0x0,
		0x0,
		0x0,
		0x0,
		0x1ffd,
		0xffd,
		0x1009,
		0x9,
		0x0,
		0x0,
		0x1009,
		0x9,
		0x1fff,
		0x1fff,
		0x1ffe,
		0x1ffe,
		0x1ffe,
		0x1ffe,
		0x1ffe,
		0x1ffe,
		0x1008,
		0x1004,
		0x8,
		0x4,
		0x8000,
		0x8000,
	},
	// SD_EFFECT_MODE_CLEAR
	{
		0x0,
		0x17,
		0x13,
		0x70f0,
		0x4fa8,
		0xbce0,
		0x4510,
		0xbef0,
		0x8500,
		0x5f80,
		0x54c0,
		0x371,
		0x2af,
		0x2e5,
		0x1df,
		0x2b0,
		0x1d7,
		0x358,
		0x26a,
		0x1d6,
		0x11e,
		0x12d,
		0xb1,
		0x11f,
		0x59,
		0x1a0,
		0xe3,
		0x58,
		0x40,
		0x28,
		0x14,
		0x8000,
		0x8000,
	}
};
static const u16 g_NotePitchTable[] =
{
	0x8000,
	0x879c,
	0x8fac,
	0x9837,
	0xa145,
	0xaadc,
	0xb504,
	0xbfc8,
	0xcb2f,
	0xd744,
	0xe411,
	0xf1a1,
	0x8000,
	0x800e,
	0x801d,
	0x802c,
	0x803b,
	0x804a,
	0x8058,
	0x8067,
	0x8076,
	0x8085,
	0x8094,
	0x80a3,
	0x80b1,
	0x80c0,
	0x80cf,
	0x80de,
	0x80ed,
	0x80fc,
	0x810b,
	0x811a,
	0x8129,
	0x8138,
	0x8146,
	0x8155,
	0x8164,
	0x8173,
	0x8182,
	0x8191,
	0x81a0,
	0x81af,
	0x81be,
	0x81cd,
	0x81dc,
	0x81eb,
	0x81fa,
	0x8209,
	0x8218,
	0x8227,
	0x8236,
	0x8245,
	0x8254,
	0x8263,
	0x8272,
	0x8282,
	0x8291,
	0x82a0,
	0x82af,
	0x82be,
	0x82cd,
	0x82dc,
	0x82eb,
	0x82fa,
	0x830a,
	0x8319,
	0x8328,
	0x8337,
	0x8346,
	0x8355,
	0x8364,
	0x8374,
	0x8383,
	0x8392,
	0x83a1,
	0x83b0,
	0x83c0,
	0x83cf,
	0x83de,
	0x83ed,
	0x83fd,
	0x840c,
	0x841b,
	0x842a,
	0x843a,
	0x8449,
	0x8458,
	0x8468,
	0x8477,
	0x8486,
	0x8495,
	0x84a5,
	0x84b4,
	0x84c3,
	0x84d3,
	0x84e2,
	0x84f1,
	0x8501,
	0x8510,
	0x8520,
	0x852f,
	0x853e,
	0x854e,
	0x855d,
	0x856d,
	0x857c,
	0x858b,
	0x859b,
	0x85aa,
	0x85ba,
	0x85c9,
	0x85d9,
	0x85e8,
	0x85f8,
	0x8607,
	0x8617,
	0x8626,
	0x8636,
	0x8645,
	0x8655,
	0x8664,
	0x8674,
	0x8683,
	0x8693,
	0x86a2,
	0x86b2,
	0x86c1,
	0x86d1,
	0x86e0,
	0x86f0,
	0x8700,
	0x870f,
	0x871f,
	0x872e,
	0x873e,
	0x874e,
	0x875d,
	0x876d,
	0x877d,
	0x878c,
};
// Enable/disable bits in SD_CORE_ATTR
static const u16 g_CoreAttrShifts[] =
{
	0x7,
	0x6,
	0xe,
	0x8,
};
static const u16 g_VoiceDataInit[] =
{
	0x700,
	0x0,
	0x0,
	0x0,
	0x0,
	0x0,
	0x0,
	0x0,
};
// clang-format on

// Unofficial: move to bss
static const u32 g_ClearEffectData[256] __attribute__((__aligned__(16)));
// Unofficial: move to bss
static int g_VoiceTransStatus[2];
// Unofficial: move to bss
static int g_VoiceTransIoMode[2];
// Unofficial: move to bss
static int g_SpdifSettings;
// Unofficial: move to bss
static sceSdEffectAttr g_EffectAttr[2];
// Unofficial: move to bss
static int g_VoiceTransCompleteBool[2];
// Unofficial: move to bss
static int g_VoiceTransCompleteEf[2];
// Unofficial: move to bss
static int g_vars_inited;
// Unofficial: move to bss
static SdIntrCallback g_Spu2IrqCallback;
// Unofficial: move to bss
static sceSdSpu2IntrHandler g_Spu2IntrHandler;
// Unofficial: move to bss
static void *g_Spu2IntrHandlerData;
// Unofficial: move to bss
static sceSdTransIntrHandler g_TransIntrHandlers[2];
// Unofficial: move to bss
static BlockHandlerIntrData_t g_BlockHandlerIntrData[2];
// Unofficial: move to bss
static SdCleanHandler g_CleanHandlers[2];
// Unofficial: move to bss
static IntrData g_TransIntrData[2];
static u32 g_CleanRegionMax[2];
static u32 g_CleanRegionCur[2];
static CleanRegionBuffer_t g_CleanRegionBuffer[2];
static u32 g_BlockTransBuff[2];
static u8 *g_BlockTransAddr[2];
static u32 g_BlockTransSize[2];
static u32 g_BatchData __attribute__((__aligned__(16)));
static SdIntrCallback g_TransIntrCallbacks[2];
static u32 g_EffectAddr[2];

int _start(int ac, char **av)
{
	int regres;
	int state;

	(void)ac;
	(void)av;
	CpuSuspendIntr(&state);
	regres = RegisterLibraryEntries(&_exp_libsd);
	CpuResumeIntr(state);
	if ( regres )
		return MODULE_NO_RESIDENT_END;
	InitSpu2_Inner();
	reset_vars();
	return MODULE_RESIDENT_END;
}

static void SetEffectRegisterPair(spu2_u16pair_t *pair, u32 val)
{
	val <<= 2;
	// Unofficial: receive register pair instead of base+offset
	pair->m_pair[0] = (val >> 16) & 0xFFFF;
	pair->m_pair[1] = val;
}

static void SetEffectData(int core, const struct mode_data_struct *mode_data)
{
	int mode_flags;
	// Unofficial: use local instead of global variable for SPU2 MMIO
	USE_SPU2_MMIO_HWPORT();

	mode_flags = mode_data->m_mode_flags;
	if ( !mode_flags )
		mode_flags = 0xFFFFFFFF;
	if ( (mode_flags & 1) )
		SetEffectRegisterPair(&spu2_mmio_hwport->m_u.m_m.m_core_regs[core].m_cregs.m_apf1_size, mode_data->m_d_apf1_size);
	if ( (mode_flags & 2) )
		SetEffectRegisterPair(&spu2_mmio_hwport->m_u.m_m.m_core_regs[core].m_cregs.m_apf2_size, mode_data->m_d_apf2_size);
	if ( (mode_flags & 4) )
		spu2_mmio_hwport->m_u.m_e.m_different_regs[core].m_iir_vol = mode_data->m_d_iir_vol;
	if ( (mode_flags & 8) )
		spu2_mmio_hwport->m_u.m_e.m_different_regs[core].m_comb1_vol = mode_data->m_d_comb1_vol;
	if ( (mode_flags & 0x10) )
		spu2_mmio_hwport->m_u.m_e.m_different_regs[core].m_comb2_vol = mode_data->m_d_comb2_vol;
	if ( (mode_flags & 0x20) )
		spu2_mmio_hwport->m_u.m_e.m_different_regs[core].m_comb3_vol = mode_data->m_d_comb3_vol;
	if ( (mode_flags & 0x40) )
		spu2_mmio_hwport->m_u.m_e.m_different_regs[core].m_comb4_vol = mode_data->m_d_comb4_vol;
	if ( (mode_flags & 0x80) )
		spu2_mmio_hwport->m_u.m_e.m_different_regs[core].m_wall_vol = mode_data->m_d_wall_vol;
	if ( (mode_flags & 0x100) )
		spu2_mmio_hwport->m_u.m_e.m_different_regs[core].m_apf1_vol = mode_data->m_d_apf1_vol;
	if ( (mode_flags & 0x200) )
		spu2_mmio_hwport->m_u.m_e.m_different_regs[core].m_apf2_vol = mode_data->m_d_apf2_vol;
	if ( (mode_flags & 0x400) )
		SetEffectRegisterPair(&spu2_mmio_hwport->m_u.m_m.m_core_regs[core].m_cregs.m_same_l_dst, mode_data->m_d_same_l_dst);
	if ( (mode_flags & 0x800) )
		SetEffectRegisterPair(&spu2_mmio_hwport->m_u.m_m.m_core_regs[core].m_cregs.m_same_r_dst, mode_data->m_d_same_r_dst);
	if ( (mode_flags & 0x1000) )
		SetEffectRegisterPair(
			&spu2_mmio_hwport->m_u.m_m.m_core_regs[core].m_cregs.m_comb1_l_src, mode_data->m_d_comb1_l_src);
	if ( (mode_flags & 0x2000) )
		SetEffectRegisterPair(
			&spu2_mmio_hwport->m_u.m_m.m_core_regs[core].m_cregs.m_comb1_r_src, mode_data->m_d_comb1_r_src);
	if ( (mode_flags & 0x4000) )
		SetEffectRegisterPair(
			&spu2_mmio_hwport->m_u.m_m.m_core_regs[core].m_cregs.m_comb2_l_src, mode_data->m_d_comb2_l_src);
	if ( (mode_flags & 0x8000) )
		SetEffectRegisterPair(
			&spu2_mmio_hwport->m_u.m_m.m_core_regs[core].m_cregs.m_comb2_r_src, mode_data->m_d_comb2_r_src);
	if ( (mode_flags & 0x10000) )
		SetEffectRegisterPair(&spu2_mmio_hwport->m_u.m_m.m_core_regs[core].m_cregs.m_same_l_src, mode_data->m_d_same_l_src);
	if ( (mode_flags & 0x20000) )
		SetEffectRegisterPair(&spu2_mmio_hwport->m_u.m_m.m_core_regs[core].m_cregs.m_same_r_src, mode_data->m_d_same_r_src);
	if ( (mode_flags & 0x40000) )
		SetEffectRegisterPair(&spu2_mmio_hwport->m_u.m_m.m_core_regs[core].m_cregs.m_diff_l_dst, mode_data->m_d_diff_l_dst);
	if ( (mode_flags & 0x80000) )
		SetEffectRegisterPair(&spu2_mmio_hwport->m_u.m_m.m_core_regs[core].m_cregs.m_diff_r_dst, mode_data->m_d_diff_r_dst);
	if ( (mode_flags & 0x100000) )
		SetEffectRegisterPair(
			&spu2_mmio_hwport->m_u.m_m.m_core_regs[core].m_cregs.m_comb3_l_src, mode_data->m_d_comb3_l_src);
	if ( (mode_flags & 0x200000) )
		SetEffectRegisterPair(
			&spu2_mmio_hwport->m_u.m_m.m_core_regs[core].m_cregs.m_comb3_r_src, mode_data->m_d_comb3_r_src);
	if ( (mode_flags & 0x400000) )
		SetEffectRegisterPair(
			&spu2_mmio_hwport->m_u.m_m.m_core_regs[core].m_cregs.m_comb4_l_src, mode_data->m_d_comb4_l_src);
	if ( (mode_flags & 0x800000) )
		SetEffectRegisterPair(
			&spu2_mmio_hwport->m_u.m_m.m_core_regs[core].m_cregs.m_comb4_r_src, mode_data->m_d_comb4_r_src);
	if ( (mode_flags & 0x1000000) )
		SetEffectRegisterPair(&spu2_mmio_hwport->m_u.m_m.m_core_regs[core].m_cregs.m_diff_l_src, mode_data->m_d_diff_l_src);
	if ( (mode_flags & 0x2000000) )
		SetEffectRegisterPair(&spu2_mmio_hwport->m_u.m_m.m_core_regs[core].m_cregs.m_diff_r_src, mode_data->m_d_diff_r_src);
	if ( (mode_flags & 0x4000000) )
		SetEffectRegisterPair(&spu2_mmio_hwport->m_u.m_m.m_core_regs[core].m_cregs.m_apf1_l_dst, mode_data->m_d_apf1_l_dst);
	if ( (mode_flags & 0x8000000) )
		SetEffectRegisterPair(&spu2_mmio_hwport->m_u.m_m.m_core_regs[core].m_cregs.m_apf1_r_dst, mode_data->m_d_apf1_r_dst);
	if ( (mode_flags & 0x10000000) )
		SetEffectRegisterPair(&spu2_mmio_hwport->m_u.m_m.m_core_regs[core].m_cregs.m_apf2_l_dst, mode_data->m_d_apf2_l_dst);
	if ( (mode_flags & 0x20000000) )
		SetEffectRegisterPair(&spu2_mmio_hwport->m_u.m_m.m_core_regs[core].m_cregs.m_apf2_r_dst, mode_data->m_d_apf2_r_dst);
	if ( (mode_flags & 0x40000000) )
		spu2_mmio_hwport->m_u.m_e.m_different_regs[core].m_in_coef_l = mode_data->m_d_in_coef_l;
	if ( (mode_flags & 0x80000000) )
		spu2_mmio_hwport->m_u.m_e.m_different_regs[core].m_in_coef_r = mode_data->m_d_in_coef_r;
}

int sceSdClearEffectWorkArea(int core, int channel, int effect_mode)
{
	u32 aligned_addr;
	u32 effect_size;
	u32 effect_addr;
	int xferres;
	SdIntrCallback callback_tmp;
	sceSdTransIntrHandler handler_tmp;

	effect_mode &= 0xFF;
	if ( effect_mode > SD_EFFECT_MODE_PIPE )
		return -100;
	if ( !effect_mode )
		return 0;
	// Unofficial: restrict channel
	channel &= 1;
	if ( DmaStartStop((channel << 4) | 4, 0, 0) )
		return -210;
	if ( g_VoiceTransIoMode[channel] != 1 )
		return -201;
	if ( QueryIntrContext() )
		return -202;
	aligned_addr = 0;
	effect_size = g_EffectSizes[effect_mode] << 3;
	effect_addr = (GetEEA(core) - (effect_size - 1)) >> 1;
	if ( (effect_size & 0x3F) )
	{
		effect_size &= 0x3FFFFFF;
		aligned_addr = (GetEEA(core) - (effect_size - 1)) >> 1;
	}
	// Disable intr_handlers by removing them
	handler_tmp = g_TransIntrHandlers[channel];
	callback_tmp = g_TransIntrCallbacks[channel];
	g_TransIntrHandlers[channel] = 0;
	g_TransIntrCallbacks[channel] = 0;
	xferres = 0;
	if ( aligned_addr )
	{
		xferres = sceSdVoiceTrans(channel, 0, (u8 *)g_ClearEffectData, (u32 *)(effect_addr << 1), 0x40);
		if ( xferres >= 0 )
			xferres = sceSdVoiceTransStatus(channel, 1);
		effect_addr = aligned_addr;
	}
	if ( xferres >= 0 )
	{
		int i;

		for ( i = 0;; i += 1 )
		{
			u32 size;

			size = (effect_size <= 0x400) ? effect_size : 0x400;
			xferres = sceSdVoiceTrans(channel, 0, (u8 *)g_ClearEffectData, (u32 *)((effect_addr + (i << 9)) << 1), size);
			if ( xferres < 0 )
				break;
			// Wait for completion
			xferres = sceSdVoiceTransStatus(channel, 1);
			if ( xferres < 0 )
				break;
			if ( effect_size <= 0x400 )
			{
				xferres = 0;
				break;
			}
			effect_size -= 0x400;
		}
	}
	// Enable intr_handlers by adding them again
	g_TransIntrHandlers[channel] = handler_tmp;
	g_TransIntrCallbacks[channel] = callback_tmp;
	return xferres;
}

static int CleanHandler(int core)
{
	// Unofficial: restrict core
	core &= 1;
	g_CleanRegionCur[core] += 1;
	if ( (int)g_CleanRegionCur[core] >= (int)(g_CleanRegionMax[core] - 1) )
		g_CleanHandlers[core] = 0;
	DmaStartStop((core << 4) | 2, g_CleanRegionBuffer[core].m_elements[g_CleanRegionCur[core]].m_spuaddr, 0);
	DmaStartStop(
		(core << 4) | 6, (u8 *)g_ClearEffectData, g_CleanRegionBuffer[core].m_elements[g_CleanRegionCur[core]].m_size);
	return 0;
}

int sceSdCleanEffectWorkArea(int core, int channel, int effect_mode)
{
	u32 effect_size;
	u32 effect_addr;
	int xferres;
	int i;

	effect_mode &= 0xFF;
	if ( effect_mode > SD_EFFECT_MODE_PIPE )
		return -100;
	if ( !effect_mode )
		return 0;
	// Unofficial: restrict channel
	channel &= 1;
	if ( DmaStartStop((channel << 4) | 4, 0, 0) )
		return -210;
	if ( g_VoiceTransIoMode[channel] != 1 )
		return -201;
	effect_size = g_EffectSizes[effect_mode] << 3;
	effect_addr = GetEEA(core) - (effect_size - 1);
	if ( (effect_size & 0x3F) )
	{
		effect_size &= 0x3FFFFFF;
		xferres = sceSdVoiceTrans(channel, 8, (u8 *)g_ClearEffectData, (u32 *)effect_addr, 0x40);
		if ( xferres < 0 )
			return xferres;
		effect_addr = GetEEA(core) - (effect_size - 1);
	}
	effect_addr += 0x100;
	effect_size -= 0x400;
	for ( i = 0;; i += 1 )
	{
		g_CleanRegionBuffer[channel].m_elements[i].m_spuaddr = (u32 *)effect_addr;
		g_CleanRegionBuffer[channel].m_elements[i].m_size = (effect_size <= 0x400) ? effect_size : 0x400;
		if ( effect_size <= 0x400 )
			break;
		effect_addr += 0x100;
		effect_size -= 0x400;
	}
	g_CleanRegionMax[channel] = i + 1;
	g_CleanHandlers[channel] = CleanHandler;
	g_CleanRegionCur[channel] = 0;
	xferres = sceSdVoiceTrans(channel, 0, (u8 *)g_ClearEffectData, (u32 *)effect_addr, 0x400);
	if ( xferres >= 0 )
		xferres = 0;
	return xferres;
}

void sceSdGetEffectAttr(int core, sceSdEffectAttr *attr)
{
	attr->core = core;
	attr->mode = g_EffectAttr[core].mode;
	attr->delay = g_EffectAttr[core].delay;
	attr->feedback = g_EffectAttr[core].feedback;
	// Unofficial: use getters/setters instead of MMIO access
	attr->depth_L = sceSdGetParam(core | SD_PARAM_EVOLL);
	attr->depth_R = sceSdGetParam(core | SD_PARAM_EVOLR);
}

int sceSdSetEffectAttr(int core, const sceSdEffectAttr *attr)
{
	int clearram;
	int channel;
	int mode;
	int effects_enabled;
	int retval;
	struct mode_data_struct mode_data;
	int state;
	int effect_mode;

	mode_data.m_mode_flags = 0;
	mode = attr->mode;
	clearram = !!(mode & SD_EFFECT_MODE_CLEAR);
	effect_mode = clearram ? g_EffectAttr[core].mode : 0;
	channel = clearram && !!(mode & 0x200);
	mode &= 0xFF;
	// Check if valid mode
	if ( mode > SD_EFFECT_MODE_PIPE )
		return -100;
	g_EffectAttr[core].mode = mode;
	g_EffectAddr[core] = GetEEA(core) - ((g_EffectSizes[mode] << 3) - 1);
	// Unofficial: use memcpy from sysclib
	memcpy(&mode_data, &g_EffectParams[mode], sizeof(mode_data));
	switch ( mode )
	{
		case SD_EFFECT_MODE_ECHO:
			g_EffectAttr[core].feedback = 0x80;
			g_EffectAttr[core].delay = 0x80;
			break;
		case SD_EFFECT_MODE_DELAY:
			g_EffectAttr[core].feedback = 0;
			g_EffectAttr[core].delay = 0x80;
			break;
		default:
			g_EffectAttr[core].feedback = 0;
			g_EffectAttr[core].delay = 0;
			break;
	}
	if ( mode >= SD_EFFECT_MODE_ECHO && mode <= SD_EFFECT_MODE_DELAY )
	{
		int delay;

		delay = attr->delay;
		g_EffectAttr[core].delay = delay;
		g_EffectAttr[core].feedback = attr->feedback;
		delay += 1;
		delay &= 0xFFFF;
		mode_data.m_d_same_l_dst = (s16)((u16)delay << 6) - (s16)mode_data.m_d_apf1_size;
		delay <<= 5;
		delay &= 0xFFFF;
		mode_data.m_d_same_l_src = delay + mode_data.m_d_same_r_src;
		mode_data.m_d_same_r_dst = delay - mode_data.m_d_apf2_size;
		mode_data.m_d_comb1_l_src = delay + mode_data.m_d_comb1_r_src;
		mode_data.m_d_apf1_l_dst = delay + mode_data.m_d_apf2_l_dst;
		mode_data.m_d_apf1_r_dst = delay + mode_data.m_d_apf2_r_dst;
		mode_data.m_d_wall_vol = 0x102 * g_EffectAttr[core].feedback;
	}
	// Disable effects
	// Unofficial: use getters/setters instead of MMIO access
	effects_enabled = (sceSdGetParam(core | SD_INTERNAL_MMIO_ATTR) >> 7) & 1;
	if ( effects_enabled )
	{
		CpuSuspendIntr(&state);
		// Unofficial: use getters/setters instead of MMIO access
		sceSdSetParam(core | SD_INTERNAL_MMIO_ATTR, sceSdGetParam(core | SD_INTERNAL_MMIO_ATTR) & (~SD_ENABLE_EFFECTS));
		CpuResumeIntr(state);
	}
	// Clean up after last mode
	retval = (effects_enabled && clearram) ? sceSdClearEffectWorkArea(core, channel, effect_mode) : 0;
	if ( retval >= 0 )
	{
		// Depth / Volume
		// Unofficial: use getters/setters instead of MMIO access
		sceSdSetParam(core | SD_PARAM_EVOLL, attr->depth_L);
		sceSdSetParam(core | SD_PARAM_EVOLR, attr->depth_R);
		SetEffectData(core, &mode_data);
		// Set effect start addr (ESA)
		// Unofficial: use getters/setters instead of MMIO access
		sceSdSetAddr(core | SD_ADDR_ESA, g_EffectAddr[core]);
		retval = clearram ? sceSdClearEffectWorkArea(core, channel, mode) : 0;
	}
	// Enable effects
	if ( effects_enabled )
	{
		CpuSuspendIntr(&state);
		// Unofficial: use getters/setters instead of MMIO access
		sceSdSetParam(core | SD_INTERNAL_MMIO_ATTR, sceSdGetParam(core | SD_INTERNAL_MMIO_ATTR) | SD_ENABLE_EFFECTS);
		CpuResumeIntr(state);
	}
	return retval;
}

static int GetEEA(int core)
{
	// Unofficial: use getters/setters instead of MMIO access
	return sceSdGetAddr(core | SD_ADDR_EEA);
}

int sceSdSetEffectMode(int core, const sceSdEffectAttr *param)
{
	int clearram;
	int channel;
	u32 mode;
	int effects_enabled;
	struct mode_data_struct mode_data;
	int state;

	mode_data.m_mode_flags = 0;
	mode = param->mode;
	clearram = !!(mode & 0x100);
	channel = clearram && !!(mode & 0x200);
	mode &= 0xFF;
	if ( mode > SD_EFFECT_MODE_PIPE )
		return -100;
	g_EffectAttr[core].mode = mode;
	g_EffectAttr[core].delay = 0;
	g_EffectAttr[core].feedback = 0;
	g_EffectAddr[core] = GetEEA(core) - ((g_EffectSizes[mode] << 3) - 1);
	// Unofficial: don't use inlined memcpy
	memcpy(&mode_data, &g_EffectParams[mode], sizeof(mode_data));
	// Unofficial: use getters/setters instead of MMIO access
	effects_enabled = (sceSdGetParam(core | SD_INTERNAL_MMIO_ATTR) >> 7) & 1;
	if ( effects_enabled )
	{
		CpuSuspendIntr(&state);
		// Unofficial: use getters/setters instead of MMIO access
		sceSdSetParam(core | SD_INTERNAL_MMIO_ATTR, sceSdGetParam(core | SD_INTERNAL_MMIO_ATTR) & ~SD_ENABLE_EFFECTS);
		CpuResumeIntr(state);
	}
	// Unofficial: use getters/setters instead of MMIO access
	sceSdSetParam(core | SD_PARAM_EVOLL, 0);
	sceSdSetParam(core | SD_PARAM_EVOLR, 0);
	SetEffectData(core, &mode_data);
	// Unofficial: use getters/setters instead of MMIO access
	sceSdSetAddr(core | SD_ADDR_ESA, g_EffectAddr[core]);
	if ( effects_enabled )
	{
		CpuSuspendIntr(&state);
		// Unofficial: use getters/setters instead of MMIO access
		sceSdSetParam(core | SD_INTERNAL_MMIO_ATTR, sceSdGetParam(core | SD_INTERNAL_MMIO_ATTR) | SD_ENABLE_EFFECTS);
		CpuResumeIntr(state);
	}
	return clearram ? sceSdCleanEffectWorkArea(core, channel, mode) : 0;
}

int sceSdSetEffectModeParams(int core, const sceSdEffectAttr *attr)
{
	int mode;
	struct mode_data_struct mode_data;

	mode = attr->mode;
	mode &= 0xFF;
	if ( mode > SD_EFFECT_MODE_PIPE )
		return -100;
	if ( g_EffectAttr[core].mode != mode )
		return -100;
	if ( mode >= SD_EFFECT_MODE_ECHO && mode <= SD_EFFECT_MODE_DELAY )
	{
		int delay;

		// Unofficial: don't use inlined memcpy
		memcpy(&mode_data, &g_EffectParams[mode], sizeof(mode_data));
		mode_data.m_mode_flags = 0xC011C80;
		delay = attr->delay;
		g_EffectAttr[core].delay = delay;
		g_EffectAttr[core].feedback = attr->feedback;
		delay += 1;
		delay &= 0xFFFF;
		mode_data.m_d_same_l_dst = (s16)(((u16)delay << 6) - (s16)mode_data.m_d_apf1_size);
		delay <<= 5;
		delay &= 0xFFFF;
		mode_data.m_d_same_l_src = delay + mode_data.m_d_same_r_src;
		mode_data.m_d_same_r_dst = delay - mode_data.m_d_apf2_size;
		mode_data.m_d_comb1_l_src = delay + mode_data.m_d_comb1_r_src;
		mode_data.m_d_apf1_l_dst = delay + mode_data.m_d_apf2_l_dst;
		mode_data.m_d_apf1_r_dst = delay + mode_data.m_d_apf2_r_dst;
		mode_data.m_d_wall_vol = 0x102 * g_EffectAttr[core].feedback;
		SetEffectData(core, &mode_data);
	}
	// Unofficial: use getters/setters instead of MMIO access
	sceSdSetParam(core | SD_PARAM_EVOLL, attr->depth_L);
	sceSdSetParam(core | SD_PARAM_EVOLR, attr->depth_R);
	return 0;
}

static void InitSpu2_Inner(void)
{
	int state;
	USE_IOP_MMIO_HWPORT();

	iop_mmio_hwport->ssbus2.ind_4_address = 0xBF900000;
	iop_mmio_hwport->ssbus2.ind_9_address = 0xBF900800;
	CpuSuspendIntr(&state);
	iop_mmio_hwport->dmac1.dpcr1 |= 0x80000;
	iop_mmio_hwport->dmac2.dpcr2 |= 8;
	CpuResumeIntr(state);
	iop_mmio_hwport->ssbus1.ind_4_delay = 0x200B31E1;
	iop_mmio_hwport->ssbus2.ind_9_delay = 0x200B31E1;
}

static void InitSpu2(void)
{
	USE_SPU2_MMIO_HWPORT();

	InitSpu2_Inner();
	spu2_mmio_hwport->m_u.m_e.m_spdif_mode = 0x0900;
	spu2_mmio_hwport->m_u.m_e.m_spdif_media = 0x200;
	spu2_mmio_hwport->m_u.m_e.m_unknown7ca = 8;
}

// Core / Volume Registers
static void InitCoreVolume(int flag)
{
	int i;
	USE_SPU2_MMIO_HWPORT();

	spu2_mmio_hwport->m_u.m_e.m_spdif_out = 0xC032;
	// Unofficial: rerolled
	// Unofficial: use getters/setters instead of MMIO access
	for ( i = 0; i < 2; i += 1 )
		sceSdSetParam(
			SD_CORE_0 | SD_INTERNAL_MMIO_ATTR,
			(flag ? SD_ENABLE_EFFECTS : 0) | (i ? SD_ENABLE_EX_INPUT : 0) | SD_MUTE | SD_SPU2_ON);
	// Unofficial: rerolled
	// HIgh is voices 0-15, Low is 16-23, representing voices 0..23 (24)
	for ( i = 0; i < 2; i += 1 )
	{
		sceSdSetSwitch(i | SD_SWITCH_VMIXL, 0xFFFFFF);
		sceSdSetSwitch(i | SD_SWITCH_VMIXEL, 0xFFFFFF);
		sceSdSetSwitch(i | SD_SWITCH_VMIXR, 0xFFFFFF);
		sceSdSetSwitch(i | SD_SWITCH_VMIXER, 0xFFFFFF);
		sceSdSetParam(i | SD_PARAM_MMIX, 0xFF0 + (i * 0xC));
	}
	if ( !flag )
	{
		// Unofficial: rerolled
		// Unofficial: use getters/setters instead of MMIO access
		for ( i = 0; i < 2; i += 1 )
		{
			sceSdSetParam(i | SD_PARAM_MVOLL, 0);
			sceSdSetParam(i | SD_PARAM_MVOLR, 0);
			sceSdSetParam(i | SD_PARAM_EVOLL, 0);
			sceSdSetParam(i | SD_PARAM_EVOLR, 0);
		}
		// Unofficial: rerolled
		// Unofficial: use getters/setters instead of MMIO access
		// Effect End Address, Upper part
		for ( i = 0; i < 2; i += 1 )
			sceSdSetAddr(i | SD_ADDR_EEA, (0x000E + i) << 17);
	}
	// Unofficial: rerolled
	// Unofficial: use getters/setters instead of MMIO access
	for ( i = 0; i < 2; i += 1 )
	{
		// Core 1 External Input Volume.
		// The external Input is Core 0's output.
		sceSdSetParam(i | SD_PARAM_AVOLL, i ? 0x7FFF : 0);
		sceSdSetParam(i | SD_PARAM_AVOLR, i ? 0x7FFF : 0);
		sceSdSetParam(i | SD_PARAM_BVOLL, 0);
		sceSdSetParam(i | SD_PARAM_BVOLR, 0);
	}
}

int sceSdVoiceTrans(s16 chan, u16 mode, u8 *iopaddr, u32 *spuaddr, u32 size)
{
	int core;

	core = chan & 1;
	if ( !size )
		return -100;
	if ( DmaStartStop((core << 4) | 4, 0, 0) )
		return -210;
	if ( g_VoiceTransIoMode[core] != 1 )
		return -201;
	switch ( mode & 3 )
	{
		case SD_TRANS_READ:
			g_TransIntrData[core].m_mode = core | 0x900;
			g_BlockHandlerIntrData[core].m_cb = 0;
			g_BlockHandlerIntrData[core].m_userdata = 0;
			g_VoiceTransStatus[core] = 0;
			DmaStartStop((core << 4) | 2, spuaddr, 0);
			if ( QueryIntrContext() )
				iClearEventFlag(g_VoiceTransCompleteEf[core], ~1);
			else
				ClearEventFlag(g_VoiceTransCompleteEf[core], ~1);
			g_VoiceTransIoMode[core] = 0;
			return DmaStartStop((core << 4) | 5, iopaddr, (size & 0x3FFFFFF) + ((size & 0x3F) ? 0x40 : 0));
		case SD_TRANS_WRITE:
			g_TransIntrData[core].m_mode = core | 0x500;
			g_BlockHandlerIntrData[core].m_cb = 0;
			g_BlockHandlerIntrData[core].m_userdata = 0;
			DmaStartStop((core << 4) | 2, spuaddr, 0);
			if ( QueryIntrContext() )
				iClearEventFlag(g_VoiceTransCompleteEf[core], ~1);
			else
				ClearEventFlag(g_VoiceTransCompleteEf[core], ~1);
			g_VoiceTransIoMode[core] = 0;
			if ( !(mode & SD_TRANS_MODE_IO) )
			{
				g_VoiceTransStatus[core] = 0;
				return DmaStartStop((core << 4) | 6, iopaddr, (size & 0x3FFFFFF) + ((size & 0x3F) ? 0x40 : 0));
			}
			g_VoiceTransStatus[core] = 1;
			return VoiceTrans_Write_IOMode((u16 *)iopaddr, (size & 0x3FFFFFF) + ((size & 0x3F) ? 0x40 : 0), core);
		default:
			return -100;
	}
}

u32 sceSdVoiceTransStatus(s16 channel, s16 flag)
{
	u32 efres;
	int core;

	core = channel & 1;
	if ( g_VoiceTransStatus[core] == 1 || g_VoiceTransIoMode[core] == 1 )
		return 1;
	switch ( flag )
	{
		case 0:
			if ( g_VoiceTransCompleteBool[core] )
			{
				g_VoiceTransCompleteBool[core] = 0;
				g_VoiceTransIoMode[core] = 1;
			}
			break;
		case 1:
			if ( QueryIntrContext() )
				return -202;
			WaitEventFlag(g_VoiceTransCompleteEf[core], 1, 0, &efres);
			g_VoiceTransCompleteBool[core] = 0;
			g_VoiceTransIoMode[core] = 1;
			break;
		default:
			break;
	}
	return g_VoiceTransIoMode[core];
}

int sceSdStopTrans(int channel)
{
	int core;

	core = channel & 1;
	g_TransIntrData[core].m_mode = core;
	g_BlockHandlerIntrData[core].m_cb = 0;
	g_BlockHandlerIntrData[core].m_userdata = 0;
	return DmaStartStop((core << 4) | 0xA, 0, 0);
}

int sceSdBlockTrans(s16 chan, u16 mode, u8 *iopaddr, u32 size, ...)
{
	int core;
	u32 started;
	int transfer_dir;
	int retres_1;
	uiptr vararg_elm1;
	uiptr vararg_elm2;
	uiptr vararg_elm3;
	va_list va2;

	va_start(va2, size);
	vararg_elm1 = va_arg(va2, uiptr);
	vararg_elm2 = va_arg(va2, uiptr);
	vararg_elm3 = va_arg(va2, uiptr);
	va_end(va2);
	core = chan & 1;
	started = DmaStartStop((core << 4) | 4, 0, 0);
	transfer_dir = mode & 3;
	switch ( transfer_dir )
	{
		case SD_TRANS_READ:
			if ( !size )
				return -100;
			if ( started )
				return -210;
			g_TransIntrData[core].m_mode = core | 0xA00;
			g_BlockHandlerIntrData[core].m_cb = 0;
			g_BlockHandlerIntrData[core].m_userdata = 0;
			if ( (mode & 0x80) )
			{
				if ( !vararg_elm1 )
				{
					g_TransIntrData[core].m_mode = core;
					return -100;
				}
				g_BlockHandlerIntrData[core].m_cb = (void *)vararg_elm1;
				g_BlockHandlerIntrData[core].m_userdata = (void *)vararg_elm2;
				g_TransIntrData[core].m_mode |= 0x8000;
			}
			else if ( (mode & SD_TRANS_LOOP) )
			{
				size >>= 1;
				g_TransIntrData[core].m_mode |= SD_TRANS_LOOP << 8;
			}
			retres_1 = BlockTransRead(iopaddr, size, core, mode);
			break;
		case SD_TRANS_STOP:
			g_BlockHandlerIntrData[core].m_cb = 0;
			g_BlockHandlerIntrData[core].m_userdata = 0;
			g_TransIntrData[core].m_mode = core;
			return DmaStartStop((core << 4) | 0xA, 0, 0);
		case SD_TRANS_WRITE:
		case SD_TRANS_WRITE_FROM:
			if ( !size )
				return -100;
			if ( started )
				return -210;
			g_TransIntrData[core].m_mode = core | 0x600;
			g_BlockHandlerIntrData[core].m_cb = 0;
			g_BlockHandlerIntrData[core].m_userdata = 0;
			if ( (mode & 0x80) )
			{
				if ( !vararg_elm2 )
				{
					g_TransIntrData[core].m_mode = core;
					return -100;
				}
				g_BlockHandlerIntrData[core].m_cb = (void *)vararg_elm2;
				g_BlockHandlerIntrData[core].m_userdata = (void *)vararg_elm3;
				g_TransIntrData[core].m_mode |= 0x8000;
			}
			else if ( (mode & SD_TRANS_LOOP) )
			{
				size >>= 1;
				g_TransIntrData[core].m_mode |= SD_TRANS_LOOP << 8;
			}
			retres_1 =
				BlockTransWriteFrom(iopaddr, size, chan, mode, (transfer_dir == SD_TRANS_WRITE_FROM) ? (void *)vararg_elm1 : 0);
			break;
		default:
			return -100;
	}
	if ( retres_1 < 0 )
	{
		g_BlockHandlerIntrData[core].m_cb = 0;
		g_BlockHandlerIntrData[core].m_userdata = 0;
		g_TransIntrData[core].m_mode = core;
	}
	return retres_1;
}

u32 sceSdBlockTransStatus(s16 channel, s16 flag)
{
	int core;
	// Unofficial: inline thunk
	USE_IOP_MMIO_HWPORT();

	(void)flag;
	core = channel & 1;
	// Unofficial: use getters/setters instead of MMIO access
	return (g_BlockTransBuff[core] << 24)
			 | (((sceSdGetParam(core | SD_INTERNAL_MMIO_ADMAS) & 7) ?
						 (core ? &iop_mmio_hwport->dmac2.newch[0] : &iop_mmio_hwport->dmac1.oldch[4])->madr :
						 0)
					& ~0xFF000000);
}

static int InitSpdif()
{
	int i;
	USE_SPU2_MMIO_HWPORT();

	spu2_mmio_hwport->m_u.m_e.m_spdif_out = 0;
	libsd_do_busyloop(2);
	spu2_mmio_hwport->m_u.m_e.m_spdif_out = 0x8000;
	libsd_do_busyloop(1);
	// Unofficial: rerolled
	// Unofficial: use getters/setters instead of MMIO access
	for ( i = 0; i < 2; i += 1 )
	{
		sceSdSetParam(i | SD_PARAM_MVOLL, 0);
		sceSdSetParam(i | SD_PARAM_MVOLR, 0);
	}
	// Unofficial: rerolled
	// Unofficial: use getters/setters instead of MMIO access
	for ( i = 0; i < 2; i += 1 )
		sceSdSetParam(i | SD_INTERNAL_MMIO_ADMAS, 0);
	// Unofficial: rerolled
	// Unofficial: use getters/setters instead of MMIO access
	for ( i = 0; i < 2; i += 1 )
		sceSdSetParam(i | SD_INTERNAL_MMIO_ATTR, 0);
	libsd_do_busyloop(1);
	// Unofficial: rerolled
	// Unofficial: use getters/setters instead of MMIO access
	for ( i = 0; i < 2; i += 1 )
		sceSdSetParam(i | SD_INTERNAL_MMIO_ATTR, SD_SPU2_ON);
	// Unofficial: rerolled
	// Unofficial: use getters/setters instead of MMIO access
	for ( i = 0; i < 2; i += 1 )
	{
		sceSdSetParam(i | SD_PARAM_MVOLL, 0);
		sceSdSetParam(i | SD_PARAM_MVOLR, 0);
	}
	// Unofficial: use getters/setters instead of MMIO access
	for ( i = 0; (sceSdGetParam(SD_CORE_0 | SD_INTERNAL_MMIO_STATX) & 0x7FF)
							 && (sceSdGetParam(SD_CORE_1 | SD_INTERNAL_MMIO_STATX) & 0x7FF) && i < 0xF00;
				i += 1 )
		libsd_do_busyloop(1);
	// Unofficial: rerolled
	// Unofficial: use getters/setters instead of MMIO access
	for ( i = 0; i < 2; i += 1 )
		sceSdSetSwitch(i | SD_SWITCH_KOFF, 0xFFFFFF);
	// Unofficial: rerolled
	// Unofficial: use getters/setters instead of MMIO access
	for ( i = 0; i < 2; i += 1 )
	{
		sceSdSetSwitch(i | SD_SWITCH_PMON, 0);
		sceSdSetSwitch(i | SD_SWITCH_NON, 0);
	}
	return 0;
}

static void SetDmaWrite(int chan)
{
	vu32 *dmachanptr;
	USE_IOP_MMIO_HWPORT();

	dmachanptr = chan ? &iop_mmio_hwport->ssbus2.ind_9_delay : &iop_mmio_hwport->ssbus1.ind_4_delay;
	*dmachanptr &= ~0xF000000;
}

static void SetDmaRead(int chan)
{
	vu32 *dmachanptr;
	USE_IOP_MMIO_HWPORT();

	dmachanptr = chan ? &iop_mmio_hwport->ssbus2.ind_9_delay : &iop_mmio_hwport->ssbus1.ind_4_delay;
	*dmachanptr = (*dmachanptr & ~0xF000000) | 0x2000000;
}

static void __attribute__((optimize("no-unroll-loops"))) libsd_do_busyloop_inner(void)
{
	int i;
	int loopmul;

	loopmul = 13;
	for ( i = 0; i < 120; i += 1 )
	{
		loopmul *= 13;
		__asm__ __volatile__("" : "+g"(loopmul) : :);
	}
}

static void libsd_do_busyloop(int count)
{
	int i;

	for ( i = 0; i < count; i += 1 )
		libsd_do_busyloop_inner();
}

static u32 DmaStartStop(int mainarg, void *vararg2, u32 vararg3)
{
	int core;
	u32 vararg3_cal;
	u32 blocktransbufitem;
	int dma_addr;
	int i;
	int hichk;
	int state;
	USE_IOP_MMIO_HWPORT();

	// Unofficial: restrict core
	core = (mainarg >> 4) & 1;
	switch ( mainarg & 0xF )
	{
		case 2:
			// Unofficial: use getters/setters instead of MMIO access
			sceSdSetAddr(core | SD_ADDR_TSA, (uiptr)vararg2);
			return 0;
		case 4:
			// Unofficial: use getters/setters instead of MMIO access
			if ( (sceSdGetParam(core | SD_INTERNAL_MMIO_ATTR) & SD_DMA_IN_PROCESS) )
				return -1;
			if ( ((core ? &iop_mmio_hwport->dmac2.newch[0] : &iop_mmio_hwport->dmac1.oldch[4])->chcr & SD_DMA_START) )
				return -1;
			// Unofficial: use getters/setters instead of MMIO access
			if ( sceSdGetParam(core | SD_INTERNAL_MMIO_ADMAS) )
				return -1;
			return 0;
		case 5:
			CpuSuspendIntr(&state);
			// Unofficial: use getters/setters instead of MMIO access
			sceSdSetParam(core | SD_INTERNAL_MMIO_ATTR, sceSdGetParam(core | SD_INTERNAL_MMIO_ATTR) | SD_DMA_READ);
			CpuResumeIntr(state);
			SetDmaRead(core);
			vararg3_cal = (vararg3 >> 6) + (!!(vararg3 & 0x3F));
			(core ? &iop_mmio_hwport->dmac2.newch[0] : &iop_mmio_hwport->dmac1.oldch[4])->madr = (uiptr)vararg2;
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wstrict-aliasing"
			((vu16 *)&((core ? &iop_mmio_hwport->dmac2.newch[0] : &iop_mmio_hwport->dmac1.oldch[4])->bcr))[0] = 16;
#pragma GCC diagnostic pop
			((vu16 *)&((core ? &iop_mmio_hwport->dmac2.newch[0] : &iop_mmio_hwport->dmac1.oldch[4])->bcr))[1] = vararg3_cal;
			(core ? &iop_mmio_hwport->dmac2.newch[0] : &iop_mmio_hwport->dmac1.oldch[4])->chcr =
				SD_DMA_START | SD_DMA_CS | SD_DMA_DIR_SPU2IOP;
			return vararg3_cal << 6;
		case 6:
			CpuSuspendIntr(&state);
			// Unofficial: use getters/setters instead of MMIO access
			sceSdSetParam(
				core | SD_INTERNAL_MMIO_ATTR, (sceSdGetParam(core | SD_INTERNAL_MMIO_ATTR) & ~SD_CORE_DMA) | SD_DMA_WRITE);
			CpuResumeIntr(state);
			SetDmaWrite(core);
			vararg3_cal = (vararg3 >> 6) + (!!(vararg3 & 0x3F));
			(core ? &iop_mmio_hwport->dmac2.newch[0] : &iop_mmio_hwport->dmac1.oldch[4])->madr = (uiptr)vararg2;
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wstrict-aliasing"
			((vu16 *)&((core ? &iop_mmio_hwport->dmac2.newch[0] : &iop_mmio_hwport->dmac1.oldch[4])->bcr))[0] = 16;
#pragma GCC diagnostic pop
			((vu16 *)&((core ? &iop_mmio_hwport->dmac2.newch[0] : &iop_mmio_hwport->dmac1.oldch[4])->bcr))[1] = vararg3_cal;
			(core ? &iop_mmio_hwport->dmac2.newch[0] : &iop_mmio_hwport->dmac1.oldch[4])->chcr =
				SD_DMA_START | SD_DMA_CS | SD_DMA_DIR_IOP2SPU;
			return vararg3_cal << 6;
		case 0xA:
			blocktransbufitem = 0;
			dma_addr = 0;
			if ( ((core ? &iop_mmio_hwport->dmac2.newch[0] : &iop_mmio_hwport->dmac1.oldch[4])->chcr & SD_DMA_START) )
			{
				blocktransbufitem = g_BlockTransBuff[core];
				dma_addr = (core ? &iop_mmio_hwport->dmac2.newch[0] : &iop_mmio_hwport->dmac1.oldch[4])->madr;
				(core ? &iop_mmio_hwport->dmac2.newch[0] : &iop_mmio_hwport->dmac1.oldch[4])->chcr &= ~SD_DMA_START;
				// Unofficial: use getters/setters instead of MMIO access
				if ( (sceSdGetParam(core | SD_INTERNAL_MMIO_ATTR) & SD_CORE_DMA) )
				{
					// Unofficial: use getters/setters instead of MMIO access
					for ( i = 0; !(sceSdGetParam(core | SD_INTERNAL_MMIO_STATX) & 0x80) && i < 0x1000000; i += 1 )
					{
					}
				}
			}
			// Unofficial: use getters/setters instead of MMIO access
			if ( (sceSdGetParam(core | SD_INTERNAL_MMIO_ATTR) & SD_CORE_DMA) )
			{
				CpuSuspendIntr(&state);
				// Unofficial: use getters/setters instead of MMIO access
				sceSdSetParam(core | SD_INTERNAL_MMIO_ATTR, sceSdGetParam(core | SD_INTERNAL_MMIO_ATTR) & ~SD_CORE_DMA);
				CpuResumeIntr(state);
				// Unofficial: use getters/setters instead of MMIO access
				for ( i = 0; (sceSdGetParam(core | SD_INTERNAL_MMIO_ATTR) & SD_CORE_DMA) && i < 0xF00; i += 1 )
				{
				}
			}
			// Unofficial: use getters/setters instead of MMIO access
			hichk = !!(sceSdGetParam(core | SD_INTERNAL_MMIO_ADMAS) & 7);
			if ( hichk )
				sceSdSetParam(core | SD_INTERNAL_MMIO_ADMAS, 0);
			if ( QueryIntrContext() )
				iSetEventFlag(g_VoiceTransCompleteEf[core], 1);
			else
				SetEventFlag(g_VoiceTransCompleteEf[core], 1);
			g_VoiceTransCompleteBool[core] = 0;
			g_VoiceTransIoMode[core] = 1;
			g_CleanHandlers[core] = 0;
			return (dma_addr && hichk) ? ((dma_addr & ~0xFF000000) | (blocktransbufitem << 24)) : 0;
		default:
			return 0;
	}
}

static u32 VoiceTrans_Write_IOMode(const u16 *iopaddr, u32 size, int core)
{
	u32 size_tmp;
	int count;
	int i;
	int state;

	// Unofficial: restrict core
	core &= 1;
	for ( size_tmp = size; size_tmp; size_tmp -= count )
	{
		count = (size_tmp <= 0x40) ? size_tmp : 0x40;
		// Unofficial: use getters/setters instead of MMIO access
		for ( i = 0; i < (count / 2); i += 1 )
			sceSdSetParam(core | SD_INTERNAL_MMIO_STD, iopaddr[i]);
		CpuSuspendIntr(&state);
		// Set Transfer mode to IO
		// Unofficial: use getters/setters instead of MMIO access
		sceSdSetParam(
			core | SD_INTERNAL_MMIO_ATTR, (sceSdGetParam(core | SD_INTERNAL_MMIO_ATTR) & ~SD_CORE_DMA) | SD_DMA_IO);
		CpuResumeIntr(state);
		// Wait for transfer to complete;
		// Unofficial: use getters/setters instead of MMIO access
		for ( i = 0; (sceSdGetParam(core | SD_INTERNAL_MMIO_STATX) & SD_IO_IN_PROCESS) && i < 0xF00; i += 1 )
			libsd_do_busyloop(1);
	}
	CpuSuspendIntr(&state);
	// Reset DMA settings
	// Unofficial: use getters/setters instead of MMIO access
	sceSdSetParam(core | SD_INTERNAL_MMIO_ATTR, sceSdGetParam(core | SD_INTERNAL_MMIO_ATTR) & ~SD_CORE_DMA);
	CpuResumeIntr(state);
	g_VoiceTransIoMode[core] = 1;
	// Unofficial: return size
	return size;
}

static void do_finish_block_clean_xfer(int core)
{
	// Unofficial: use getters/setters instead of MMIO access
	sceSdSetParam(core | SD_INTERNAL_MMIO_ATTR, sceSdGetParam(core | SD_INTERNAL_MMIO_ATTR) & ~SD_CORE_DMA);
	sceSdSetParam(core | SD_INTERNAL_MMIO_ADMAS, 0);
}

static int TransInterrupt(IntrData *intr)
{
	int dma_dir;
	u32 mode;
	int core;
	int i;
	void *dma_addr;
	int dma_size;
	USE_IOP_MMIO_HWPORT();

	mode = intr->m_mode;
	switch ( mode & 0xC00 )
	{
		case 0x400:
			dma_dir = SD_DMA_DIR_IOP2SPU;
			break;
		case 0x800:
			dma_dir = SD_DMA_DIR_SPU2IOP;
			break;
		default:
			return 1;
	}
	core = mode & 1;
	switch ( mode & 0x300 )
	{
		// Voice Transfer
		case 0x100:
			// SD_C_STATX(core)
			// If done elsewise, it doesn't work, havn't figured out why yet.
			// Unofficial: use getters/setters instead of MMIO access
			for ( i = 0; !(sceSdGetParam(core | SD_INTERNAL_MMIO_STATX) & 0x80) && i < 0x1000000; i += 1 )
			{
			}
			// Unofficial: use getters/setters instead of MMIO access
			sceSdSetParam(core | SD_INTERNAL_MMIO_ATTR, sceSdGetParam(core | SD_INTERNAL_MMIO_ATTR) & ~SD_CORE_DMA);
			for ( i = 0; (sceSdGetParam(core | SD_INTERNAL_MMIO_ATTR) & SD_CORE_DMA) && i < 0xF00; i += 1 )
			{
			}
			if ( dma_dir == SD_DMA_DIR_SPU2IOP )
				FlushDcache();
			if ( g_CleanHandlers[core] )
				g_CleanHandlers[core](core);
			else
			{
				iSetEventFlag(g_VoiceTransCompleteEf[core], 1);
				if ( g_TransIntrHandlers[core] )
				{
					g_VoiceTransIoMode[core] = 1;
					g_TransIntrHandlers[core](core, intr->m_data);
				}
				else if ( !g_TransIntrCallbacks[core] )
					g_VoiceTransCompleteBool[core] = 1;
				else
				{
					g_VoiceTransIoMode[core] = 1;
					g_TransIntrCallbacks[core](0);
				}
			}
			break;
		// Block Transfer
		case 0x200:
			if ( (mode & 0x8000) )
			{
				if ( g_BlockHandlerIntrData[core].m_cb )
				{
					g_BlockHandlerIntrData[core].m_cb(core, g_BlockHandlerIntrData[core].m_userdata, &dma_addr, &dma_size);
					if ( dma_size > 0 )
					{
						((vu16 *)&((core ? &iop_mmio_hwport->dmac2.newch[0] : &iop_mmio_hwport->dmac1.oldch[4])->bcr))[1] =
							(dma_size >> 6) + (dma_size - (dma_size & 0x3FFFFFF) > 0);
						(core ? &iop_mmio_hwport->dmac2.newch[0] : &iop_mmio_hwport->dmac1.oldch[4])->madr = (uiptr)dma_addr;
						(core ? &iop_mmio_hwport->dmac2.newch[0] : &iop_mmio_hwport->dmac1.oldch[4])->chcr =
							dma_dir | SD_DMA_START | SD_DMA_CS;
					}
					else
					{
						do_finish_block_clean_xfer(core);
						g_BlockHandlerIntrData[core].m_cb = 0;
						g_BlockHandlerIntrData[core].m_userdata = 0;
					}
				}
				else
					do_finish_block_clean_xfer(core);
				if ( dma_dir == SD_DMA_DIR_SPU2IOP )
					FlushDcache();
			}
			else
			{
				if ( (mode & (SD_TRANS_LOOP << 8)) )
				{
					// Switch buffers
					g_BlockTransBuff[core] = 1 - g_BlockTransBuff[core];
					// Setup DMA & send
					((vu16 *)&((core ? &iop_mmio_hwport->dmac2.newch[0] : &iop_mmio_hwport->dmac1.oldch[4])->bcr))[1] =
						(int)g_BlockTransSize[core] / 0x40 + ((int)g_BlockTransSize[core] % 0x40 > 0);
					(core ? &iop_mmio_hwport->dmac2.newch[0] : &iop_mmio_hwport->dmac1.oldch[4])->madr =
						(uiptr)(g_BlockTransAddr[core] + g_BlockTransBuff[core] * g_BlockTransSize[core]);
					(core ? &iop_mmio_hwport->dmac2.newch[0] : &iop_mmio_hwport->dmac1.oldch[4])->chcr =
						dma_dir | SD_DMA_START | SD_DMA_CS;
				}
				else
					do_finish_block_clean_xfer(core);
				if ( dma_dir == SD_DMA_DIR_SPU2IOP )
					FlushDcache();
				if ( g_TransIntrHandlers[core] )
					g_TransIntrHandlers[core](core, intr->m_data);
				else if ( g_TransIntrCallbacks[core] )
					g_TransIntrCallbacks[core](0);
			}
			break;
		default:
			break;
	}
	return 1;
}

static u32 BlockTransWriteFrom(u8 *iopaddr, u32 size, int core, int mode, u8 *startaddr)
{
	u8 *startaddr_tmp;
	int size_align;
	int size_align_r6;
	int state;
	USE_IOP_MMIO_HWPORT();

	core &= 1;
	startaddr_tmp = startaddr;
	g_BlockTransAddr[core] = iopaddr;
	g_BlockTransBuff[core] = 0;
	g_BlockTransSize[core] = size;
	if ( startaddr )
	{
		size_align = size - (startaddr - iopaddr);
		if ( (u32)(startaddr - iopaddr) >= size )
		{
			u32 other_align;

			other_align = startaddr - iopaddr - size;
			if ( !(mode & SD_TRANS_LOOP) || other_align >= size )
				return -100;
			g_BlockTransBuff[core] += 1;
			size_align = size - other_align;
		}
		if ( size_align % 0x400 > 0 )
		{
			size_align = (size_align / 0x400 + 1) << 10;
			startaddr_tmp = iopaddr + g_BlockTransBuff[core] * size + size - size_align;
		}
	}
	else
	{
		startaddr_tmp = iopaddr;
		size_align = size;
	}
	CpuSuspendIntr(&state);
	// Unofficial: use getters/setters instead of MMIO access
	sceSdSetParam(core | SD_INTERNAL_MMIO_ATTR, sceSdGetParam(core | SD_INTERNAL_MMIO_ATTR) & ~SD_CORE_DMA);
	CpuResumeIntr(state);
	// Unofficial: use getters/setters instead of MMIO access
	sceSdSetAddr(core | SD_ADDR_TSA, 0);
	sceSdSetParam(core | SD_INTERNAL_MMIO_ADMAS, 1 << core);
	SetDmaWrite(core);
	(core ? &iop_mmio_hwport->dmac2.newch[0] : &iop_mmio_hwport->dmac1.oldch[4])->madr = (uiptr)startaddr_tmp;
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wstrict-aliasing"
	((vu16 *)&((core ? &iop_mmio_hwport->dmac2.newch[0] : &iop_mmio_hwport->dmac1.oldch[4])->bcr))[0] = 16;
#pragma GCC diagnostic pop
	size_align_r6 = ((size_align < 0) ? (size_align + 63) : size_align) >> 6;
	((vu16 *)&((core ? &iop_mmio_hwport->dmac2.newch[0] : &iop_mmio_hwport->dmac1.oldch[4])->bcr))[1] =
		size_align_r6 + (size_align - (size_align_r6 << 6) > 0);
	(core ? &iop_mmio_hwport->dmac2.newch[0] : &iop_mmio_hwport->dmac1.oldch[4])->chcr =
		SD_DMA_START | SD_DMA_CS | SD_DMA_DIR_IOP2SPU;
	return size;
}

static u32 BlockTransRead(u8 *iopaddr, u32 size, int core, u16 mode)
{
	int state;
	USE_IOP_MMIO_HWPORT();

	core &= 1;
	g_BlockTransAddr[core] = iopaddr;
	g_BlockTransBuff[core] = 0;
	g_BlockTransSize[core] = size;
	CpuSuspendIntr(&state);
	// Unofficial: use getters/setters instead of MMIO access
	sceSdSetParam(core | SD_INTERNAL_MMIO_ATTR, sceSdGetParam(core | SD_INTERNAL_MMIO_ATTR) & ~SD_CORE_DMA);
	CpuResumeIntr(state);
	// Unofficial: use getters/setters instead of MMIO access
	sceSdSetAddr(core | SD_ADDR_TSA, (((mode & ~0xF0FF) << 1) + 0x400) << 1);
	sceSdSetParam(core | SD_INTERNAL_MMIO_UNK1AE, (mode & ~0xFFF) >> 11);
	libsd_do_busyloop(3);
	// Unofficial: use getters/setters instead of MMIO access
	sceSdSetParam(core | SD_INTERNAL_MMIO_ADMAS, 4);
	SetDmaRead(core);
	(core ? &iop_mmio_hwport->dmac2.newch[0] : &iop_mmio_hwport->dmac1.oldch[4])->madr = (uiptr)iopaddr;
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wstrict-aliasing"
	((vu16 *)&((core ? &iop_mmio_hwport->dmac2.newch[0] : &iop_mmio_hwport->dmac1.oldch[4])->bcr))[0] = 16;
#pragma GCC diagnostic pop
	((vu16 *)&((core ? &iop_mmio_hwport->dmac2.newch[0] : &iop_mmio_hwport->dmac1.oldch[4])->bcr))[1] =
		(int)g_BlockTransSize[core] / 0x40 + ((int)g_BlockTransSize[core] % 0x40 > 0);
	(core ? &iop_mmio_hwport->dmac2.newch[0] : &iop_mmio_hwport->dmac1.oldch[4])->chcr = SD_DMA_START | SD_DMA_CS;
	return size;
}

static int SifDmaBatch(void *ee_addr, void *iop_addr, int size)
{
	int dmat;
	int i;
	int dma_status;
	SifDmaTransfer_t xferparam;
	int state;

	xferparam.dest = ee_addr;
	xferparam.src = iop_addr;
	xferparam.size = size;
	xferparam.attr = 0;
	CpuSuspendIntr(&state);
	dmat = sceSifSetDma(&xferparam, SIF_DMA_TO_EE);
	CpuResumeIntr(state);
	if ( !dmat )
		return -1;
	for ( i = 0, dma_status = 0; i >= 0 && dma_status >= 0; i += 1 )
	{
		CpuSuspendIntr(&state);
		dma_status = sceSifDmaStat(dmat);
		CpuResumeIntr(state);
	}
	return (i < 0) ? -1 : 0;
}

int sceSdProcBatch(const sceSdBatch *batch, u32 *rets, u32 num)
{
	u32 cnt;

	for ( cnt = 0; cnt < num; cnt += 1 )
	{
		u32 Param;

		Param = 0;
		switch ( batch[cnt].func )
		{
			case SD_BATCH_SETPARAM:
				sceSdSetParam(batch[cnt].entry, batch[cnt].value);
				break;
			case SD_BATCH_SETSWITCH:
				sceSdSetSwitch(batch[cnt].entry, batch[cnt].value);
				break;
			case SD_BATCH_SETADDR:
				sceSdSetAddr(batch[cnt].entry, batch[cnt].value);
				break;
			case SD_BATCH_SETCORE:
				sceSdSetCoreAttr(batch[cnt].entry, batch[cnt].value);
				break;
			case SD_BATCH_WRITEIOP:
				*(u32 *)batch[cnt].value = batch[cnt].entry;
				break;
			case SD_BATCH_WRITEEE:
				g_BatchData = batch[cnt].entry;
				Param = SifDmaBatch((void *)batch[cnt].value, &g_BatchData, sizeof(g_BatchData));
				break;
			case SD_BATCH_EERETURN:
				Param = SifDmaBatch((void *)batch[cnt].value, rets, batch[cnt].entry);
				break;
			case SD_BATCH_GETPARAM:
				Param = sceSdGetParam(batch[cnt].entry);
				break;
			case SD_BATCH_GETSWITCH:
				Param = sceSdGetSwitch(batch[cnt].entry);
				break;
			case SD_BATCH_GETADDR:
				Param = sceSdGetAddr(batch[cnt].entry);
				break;
			case SD_BATCH_GETCORE:
				Param = sceSdGetCoreAttr(batch[cnt].entry);
				break;
			default:
				return ~cnt;
		}
		if ( rets )
			rets[cnt] = Param;
	}
	return cnt;
}

int sceSdProcBatchEx(const sceSdBatch *batch, u32 *rets, u32 num, u32 voice)
{
	u32 cnt;
	int loop;
	int i;

	loop = 0;
	for ( cnt = 0; cnt < num; cnt += 1 )
	{
		u32 Param;

		Param = 0;
		switch ( batch[cnt].func )
		{
			case SD_BATCH_SETPARAM:
				if ( (batch[cnt].entry & 0x3E) == 0x3E )
				{
					for ( i = 0; i < 24; i += 1 )
					{
						if ( ((1 << i) & voice) )
						{
							loop += 1;
							sceSdSetParam((batch[cnt].entry & ~0x3E) | (i << 1), batch[cnt].value);
						}
					}
					loop -= 1;
				}
				else
					sceSdSetParam(batch[cnt].entry, batch[cnt].value);
				break;
			case SD_BATCH_SETSWITCH:
				sceSdSetSwitch(batch[cnt].entry, batch[cnt].value);
				break;
			case SD_BATCH_SETADDR:
				if ( (batch[cnt].entry & 0x7E) == 0x7E )
				{
					for ( i = 0; i < 24; i += 1 )
					{
						if ( ((1 << i) & voice) )
						{
							loop += 1;
							sceSdSetAddr((batch[cnt].entry & ~0x3E) | (i << 1), batch[cnt].value);
						}
					}
					loop -= 1;
				}
				else
					sceSdSetAddr(batch[cnt].entry, batch[cnt].value);
				break;
			case SD_BATCH_SETCORE:
				sceSdSetCoreAttr(batch[cnt].entry, batch[cnt].value);
				break;
			case SD_BATCH_WRITEIOP:
				*(u32 *)batch[cnt].value = batch[cnt].entry;
				break;
			case SD_BATCH_WRITEEE:
				g_BatchData = batch[cnt].entry;
				Param = SifDmaBatch((void *)batch[cnt].value, &g_BatchData, sizeof(g_BatchData));
				break;
			case SD_BATCH_EERETURN:
				Param = SifDmaBatch((void *)batch[cnt].value, rets, batch[cnt].entry);
				break;
			case SD_BATCH_GETPARAM:
				if ( (batch[cnt].entry & 0x3E) == 0x3E )
				{
					for ( i = 0; i < 24; i += 1 )
					{
						if ( ((1 << i) & voice) )
							Param = sceSdGetParam((batch[cnt].entry & ~0x3E) | (i << 1));
						if ( rets )
							rets[loop] = Param;
						loop += 1;
					}
					loop -= 1;
				}
				else
					Param = sceSdGetParam(batch[cnt].entry);
				break;
			case SD_BATCH_GETSWITCH:
				Param = sceSdGetSwitch(batch[cnt].entry);
				break;
			case SD_BATCH_GETADDR:
				if ( (batch[cnt].entry & 0x7E) == 0x7E )
				{
					for ( i = 0; i < 24; i += 1 )
					{
						if ( ((1 << i) & voice) )
						{
							Param = sceSdGetAddr((batch[cnt].entry & ~0x3E) | (i << 1));
							if ( rets )
								rets[loop] = Param;
							loop += 1;
						}
					}
					loop -= 1;
				}
				else
					Param = sceSdGetAddr(batch[cnt].entry);
				break;
			case SD_BATCH_GETCORE:
				Param = sceSdGetCoreAttr(batch[cnt].entry);
				break;
			default:
				return ~cnt;
		}
		if ( rets )
			rets[loop] = Param;
		loop += 1;
	}
	return loop;
}

void sceSdSetParam(u16 entry, u16 value)
{
	// Determine the channel offset (entry & 0x80)
	g_ParamRegList[((entry >> 8) & 0xFF)]
								[((entry & 0x3E) << 2) + (((entry & 1) * (0x400 - 984 * (!!(entry & 0x80)))) >> 1)] = value;
}

u16 sceSdGetParam(u16 entry)
{
	// Determine the channel offset (entry & 0x80)
	return g_ParamRegList[((entry >> 8) & 0xFF)]
											 [((entry & 0x3E) << 2) + (((entry & 1) * (0x400 - 984 * (!!(entry & 0x80)))) >> 1)];
}

void sceSdSetSwitch(u16 entry, u32 value)
{
	vu16 *regptr;

	regptr = &g_ParamRegList[((entry >> 8) & 0xFF)][(entry & 1) << 9];
	regptr[0] = value;
	regptr[1] = (value >> 16) & 0xFF;
}

u32 sceSdGetSwitch(u16 entry)
{
	const vu16 *regptr;

	regptr = &g_ParamRegList[((entry >> 8) & 0xFF)][(entry & 1) << 9];
	return regptr[0] | (regptr[1] << 16);
}

void sceSdSetAddr(u16 entry, u32 value)
{
	vu16 *reg1;

	reg1 = &g_ParamRegList[((entry >> 8) & 0xFF)][((entry & 1) << 9) + 3 * (entry & 0x3E)];
	reg1[0] = value >> 17;
	if ( (entry & 0xFF00) != SD_ADDR_EEA )
		reg1[1] = (value >> 1) & ~7;
}

u32 sceSdGetAddr(u16 entry)
{
	int retlo;
	const vu16 *reg1;
	int regmask;
	int rethi;

	retlo = 0x1FFFF;
	reg1 = &g_ParamRegList[((entry >> 8) & 0xFF)][((entry & 1) << 9) + 3 * (entry & 0x3E)];
	regmask = entry & 0xFF00;
	rethi = reg1[0] << 17;
	if ( regmask != SD_ADDR_EEA )
	{
		retlo = reg1[1] << 1;
		if ( regmask == SD_VADDR_LSAX || regmask == SD_VADDR_NAX )
		{
			rethi = reg1[0] << 17;
			retlo = reg1[1] << 1;
		}
	}
	return rethi | retlo;
}

u16 sceSdNote2Pitch(u16 center_note, u16 center_fine, u16 note, s16 fine)
{
	int _fine;
	s16 _note;
	int _fine2;
	int offset2;
	int val2;
	s16 val;
	s16 offset1;
	int retval;

	_fine = fine + center_fine;
	_fine2 = _fine / 0x80;
	_note = note + _fine2 - center_note;
	offset2 = _fine % 0x80;
	val2 = ((_note / 6) >> 1) - (_note < 0);
	offset1 = _note - 12 * val2;
	val = val2 - 2;
	if ( (offset1 < 0) || (!offset1 && offset2 < 0) )
	{
		offset1 += 12;
		val -= 1;
	}
	if ( offset2 < 0 )
	{
		offset2 += (_fine2 + 1) << 7;
		offset1 -= 1;
		offset1 += _fine2;
	}
	retval = (g_NotePitchTable[offset1] * g_NotePitchTable[offset2 + 12]) >> 16;
	return (val < 0) ? (u32)(retval + (1 << (-val - 1))) >> -val : (u32)retval;
}

u16 sceSdPitch2Note(u16 center_note, u16 center_fine, u16 pitch)
{
	int bit;
	int i1;
	s16 val;
	int i2;
	int i5;

	bit = 0;
	pitch = (pitch > 0x3FFF) ? 0x3FFF : pitch;
	for ( i1 = 0; i1 < 14; i1 += 1 )
	{
		if ( ((pitch >> i1) & 1) )
			bit = i1;
	}
	val = pitch << (15 - bit);
	for ( i2 = 11; val < g_NotePitchTable[i2] && i2 > 0; i2 -= 1 )
	{
	}
	if ( !g_NotePitchTable[i2] )
		__builtin_trap();
	val <<= 15;
	val /= g_NotePitchTable[i2];
	for ( i5 = 127; val < g_NotePitchTable[i5 + 12] && i5 > 0; i5 -= 1 )
	{
	}
	return (((center_fine + i5 + 1) & 0x7E)
					+ ((i2 + center_note + 12 * (bit - 12) + ((u16)(center_fine + i5 + 1) >> 7)) << 8))
			 & ~1;
}

static int SetSpdifMode(int val)
{
	u16 spdif_out_new;
	u16 spdif_mode_new;
	USE_SPU2_MMIO_HWPORT();

	spdif_out_new = spu2_mmio_hwport->m_u.m_e.m_spdif_out & ~0x1A8;
	spdif_mode_new = spu2_mmio_hwport->m_u.m_e.m_spdif_mode & ~0xBF06;
	switch ( val & 0xF )
	{
		case 0:
			spdif_out_new |= 0x20;
			break;
		case 1:
			spdif_out_new |= 0x100;
			spdif_mode_new |= 2;
			break;
		case 2:
			break;
		case 3:
			spdif_out_new |= 0x100;
			break;
		default:
			return -100;
	}
	spdif_mode_new |= (val & 0x80) ? 0x8000 : 0;
	switch ( val & 0xF00 )
	{
		case 0x400:
			spu2_mmio_hwport->m_u.m_e.m_spdif_media = 0;
			spdif_mode_new |= 0x100;
			break;
		case 0x800:
			spu2_mmio_hwport->m_u.m_e.m_spdif_media = 0x200;
			spdif_mode_new |= 0x1900;
			break;
		default:
			spu2_mmio_hwport->m_u.m_e.m_spdif_media = 0x200;
			spdif_mode_new |= 0x900;
			break;
	}
	spu2_mmio_hwport->m_u.m_e.m_spdif_out = spdif_out_new;
	spu2_mmio_hwport->m_u.m_e.m_spdif_mode = spdif_mode_new;
	g_SpdifSettings = val;
	return 0;
}

void sceSdSetCoreAttr(u16 entry, u16 value)
{
	u16 setting_tmp;
	int core;
	int state;

	core = entry & 1;
	switch ( entry & ~0xFFFF0001 )
	{
		case SD_CORE_SPDIF_MODE:
			SetSpdifMode(value);
			break;
		case SD_CORE_NOISE_CLK:
			CpuSuspendIntr(&state);
			sceSdSetParam(
				core | SD_INTERNAL_MMIO_ATTR, (sceSdGetParam(core | SD_INTERNAL_MMIO_ATTR) & ~0x3F00) | ((value & 0x3F) << 8));
			CpuResumeIntr(state);
			break;
		default:
			// Unofficial: inline the following
			setting_tmp = g_CoreAttrShifts[((entry & 0xE) >> 1) - 1];
			CpuSuspendIntr(&state);
			sceSdSetParam(
				core | SD_INTERNAL_MMIO_ATTR,
				(sceSdGetParam(core | SD_INTERNAL_MMIO_ATTR) & ~(1 << setting_tmp)) | ((value & 1) << setting_tmp));
			CpuResumeIntr(state);
			break;
	}
}

u16 sceSdGetCoreAttr(u16 entry)
{
	int core;

	core = entry & 1;
	switch ( entry & 0xE )
	{
		case SD_CORE_EFFECT_ENABLE:
			// Unofficial: use getters/setters instead of MMIO access
			return (sceSdGetParam(core | SD_INTERNAL_MMIO_ATTR) >> 7) & 1;
		case SD_CORE_IRQ_ENABLE:
			// Unofficial: use getters/setters instead of MMIO access
			return (sceSdGetParam(core | SD_INTERNAL_MMIO_ATTR) >> 6) & 1;
		case SD_CORE_MUTE_ENABLE:
			// Unofficial: use getters/setters instead of MMIO access
			return (sceSdGetParam(core | SD_INTERNAL_MMIO_ATTR) >> 14) & 1;
		case SD_CORE_NOISE_CLK:
			// Unofficial: use getters/setters instead of MMIO access
			return (sceSdGetParam(core | SD_INTERNAL_MMIO_ATTR) >> 8) & 0x3F;
		case SD_CORE_SPDIF_MODE:
			return g_SpdifSettings & 0xFFFF;
		default:
			return 0;
	}
}

SdIntrCallback sceSdSetTransCallback(s32 core, SdIntrCallback cb)
{
	SdIntrCallback oldtmp;

	// Unofficial: restrict core
	core &= 1;
	oldtmp = g_TransIntrCallbacks[core];
	g_TransIntrCallbacks[core] = cb;
	return oldtmp;
}

sceSdTransIntrHandler sceSdSetTransIntrHandler(int channel, sceSdTransIntrHandler func, void *arg)
{
	sceSdTransIntrHandler oldtmp;
	int core;

	// Unofficial: restrict core
	core = channel & 1;
	oldtmp = g_TransIntrHandlers[core];
	g_TransIntrHandlers[core] = func;
	g_TransIntrData[core].m_data = arg;
	return oldtmp;
}

void *sceSdGetTransIntrHandlerArgument(int arg)
{
	return g_TransIntrData[arg].m_data;
}

SdIntrCallback sceSdSetIRQCallback(SdIntrCallback cb)
{
	SdIntrCallback oldtmp;

	oldtmp = g_Spu2IrqCallback;
	g_Spu2IrqCallback = cb;
	return oldtmp;
}

sceSdSpu2IntrHandler sceSdSetSpu2IntrHandler(sceSdSpu2IntrHandler func, void *arg)
{
	sceSdSpu2IntrHandler oldtmp;

	oldtmp = g_Spu2IntrHandler;
	g_Spu2IntrHandler = func;
	g_Spu2IntrHandlerData = arg;
	return oldtmp;
}

void *sceSdGetSpu2IntrHandlerArgument()
{
	return g_Spu2IntrHandlerData;
}

static int Spu2Interrupt(void *data)
{
	int val;
	USE_SPU2_MMIO_HWPORT();

	(void)data;
	if ( !g_Spu2IntrHandler && !g_Spu2IrqCallback )
		return 1;
	while ( (val = (spu2_mmio_hwport->m_u.m_e.m_spdif_irqinfo & 0xC) >> 2) )
	{
		int i;

		for ( i = 0; i < 2; i += 1 )
			if ( val & (1 << i) )
				// Unofficial: use getters/setters instead of MMIO access
				sceSdSetParam(i | SD_INTERNAL_MMIO_ATTR, sceSdGetParam(i | SD_INTERNAL_MMIO_ATTR) & ~0x40);
		if ( g_Spu2IntrHandler )
			g_Spu2IntrHandler(val, g_Spu2IntrHandlerData);
		else if ( g_Spu2IrqCallback )
			g_Spu2IrqCallback(0);
	}
	return 1;
}

static int InitVoices(void)
{
	int i;
	int j;

	// Unofficial: use getters/setters instead of MMIO access
	sceSdSetParam(SD_CORE_0 | SD_INTERNAL_MMIO_ATTR, sceSdGetParam(SD_CORE_0 | SD_INTERNAL_MMIO_ATTR) & ~SD_CORE_DMA);
	// Set Start Address of data to transfer.
	sceSdSetAddr(SD_CORE_0 | SD_ADDR_TSA, 0x5000);
	// Fill with data.
	// First 16 bytes are reserved.
	for ( i = 0; i < (int)(sizeof(g_VoiceDataInit) / sizeof(g_VoiceDataInit[0])); i += 1 )
		sceSdSetParam(SD_CORE_0 | SD_INTERNAL_MMIO_STD, g_VoiceDataInit[i]);

	// Set Transfer mode to IO
	sceSdSetParam(
		SD_CORE_0 | SD_INTERNAL_MMIO_ATTR, (sceSdGetParam(SD_CORE_0 | SD_INTERNAL_MMIO_ATTR) & ~SD_CORE_DMA) | SD_DMA_IO);
	// Wait for transfer to complete;
	for ( i = 0; (sceSdGetParam(SD_CORE_0 | SD_INTERNAL_MMIO_STATX) & SD_IO_IN_PROCESS) && i <= 0x1000000; i += 1 )
		libsd_do_busyloop(1);
	// Reset DMA settings
	// Unofficial: use getters/setters instead of MMIO access
	sceSdSetParam(SD_CORE_0 | SD_INTERNAL_MMIO_ATTR, sceSdGetParam(SD_CORE_0 | SD_INTERNAL_MMIO_ATTR) & ~SD_CORE_DMA);
	// Unofficial: rerolled
	// Unofficial: use getters/setters instead of MMIO access
	// Init voices
	for ( i = 0; i < 24; i += 1 )
	{
		for ( j = 0; j < 2; j += 1 )
			sceSdSetParam(SD_VOICE(j ^ 1, i) | SD_VPARAM_VOLL, 0);
		for ( j = 0; j < 2; j += 1 )
			sceSdSetParam(SD_VOICE(j ^ 1, i) | SD_VPARAM_VOLR, 0);
		for ( j = 0; j < 2; j += 1 )
			sceSdSetParam(SD_VOICE(j ^ 1, i) | SD_VPARAM_PITCH, 0x3FFF);
		for ( j = 0; j < 2; j += 1 )
			sceSdSetParam(SD_VOICE(j ^ 1, i) | SD_VPARAM_ADSR1, 0);
		for ( j = 0; j < 2; j += 1 )
			sceSdSetParam(SD_VOICE(j ^ 1, i) | SD_VPARAM_ADSR2, 0);
		// Top address of waveform data
		for ( j = 0; j < 2; j += 1 )
			sceSdSetAddr(SD_VOICE(j ^ 1, i) | SD_VADDR_SSA, 0x5000);
	}
	// Unofficial: rerolled
	// Unofficial: use getters/setters instead of MMIO access
	// Set all voices to ON
	for ( i = 0; i < 2; i += 1 )
		sceSdSetSwitch((i ^ 1) | SD_SWITCH_KON, 0xFFFFFF);
	// There is no guarantee that voices will be turn on at once.
	// So we wait to make sure.
	libsd_do_busyloop(3);
	// Unofficial: rerolled
	// Unofficial: use getters/setters instead of MMIO access
	// Set all voices to OFF
	for ( i = 0; i < 2; i += 1 )
		sceSdSetSwitch((i ^ 1) | SD_SWITCH_KOFF, 0xFFFFFF);
	// There is no guarantee that voices will be turn off at once.
	// So we wait to make sure.
	libsd_do_busyloop(3);
	// Unofficial: rerolled
	// Unofficial: use getters/setters instead of MMIO access
	for ( i = 0; i < 2; i += 1 )
		sceSdSetSwitch(i | SD_SWITCH_ENDX, 0);
	return 0;
}

static int Reset(int flag)
{
	iop_event_t efparam;
	int intrstate;
	int i;

	DisableIntr(IOP_IRQ_DMA_SPU, &intrstate);
	DisableIntr(IOP_IRQ_DMA_SPU2, &intrstate);
	DisableIntr(IOP_IRQ_SPU, &intrstate);
	ReleaseIntrHandler(IOP_IRQ_DMA_SPU);
	ReleaseIntrHandler(IOP_IRQ_DMA_SPU2);
	ReleaseIntrHandler(IOP_IRQ_SPU);
	// Unofficial: rerolled
	for ( i = 0; i < 2; i += 1 )
	{
		g_VoiceTransStatus[i] = 0;
		g_VoiceTransIoMode[i] = 1;
		g_VoiceTransCompleteBool[i] = 0;
		g_TransIntrHandlers[i] = 0;
		g_CleanHandlers[i] = 0;
		g_TransIntrData[i].m_mode = i;
		g_TransIntrData[i].m_data = 0;
		g_BlockHandlerIntrData[i].m_cb = 0;
		g_BlockHandlerIntrData[i].m_userdata = 0;
		g_TransIntrCallbacks[i] = 0;
	}
	g_Spu2IntrHandler = 0;
	g_Spu2IntrHandlerData = 0;
	g_Spu2IrqCallback = 0;
	if ( !(flag & 0xF) )
	{
		bzero(g_EffectAttr, sizeof(g_EffectAttr));
		// Unofficial: rerolled
		for ( i = 0; i < 2; i += 1 )
			g_EffectAddr[i] = 0x1DFFF0 + (0x20000 * i);
		// Unofficial: rerolled
		// Unofficial: use getters/setters instead of MMIO access
		for ( i = 0; i < 2; i += 1 )
			sceSdSetAddr(i | SD_ADDR_ESA, (((0x000E + i) << 16) | 0xFFF8) << 1);
	}
	efparam.attr = EA_MULTI;
	efparam.bits = 1;
	efparam.option = 0;
	// Unofficial: rerolled
	for ( i = 0; i < 2; i += 1 )
	{
		if ( g_VoiceTransCompleteEf[i] <= 0 )
			g_VoiceTransCompleteEf[i] = CreateEventFlag(&efparam);
		else
		{
			if ( QueryIntrContext() )
				iSetEventFlag(g_VoiceTransCompleteEf[i], 1);
			else
				SetEventFlag(g_VoiceTransCompleteEf[i], 1);
		}
	}
	return (g_VoiceTransCompleteEf[0] <= 0 || g_VoiceTransCompleteEf[1] <= 0) ? -301 : 0;
}

static void reset_vars(void)
{
	int i;

	g_vars_inited = 0;
	// Unofficial: rerolled
	for ( i = 0; i < 2; i += 1 )
		g_VoiceTransCompleteEf[i] = 0;
}

int sceSdInit(int flag)
{
	int resetres;

	InitSpu2();
	if ( !(flag & 0xF) )
		InitSpdif();
	resetres = Reset(flag);
	InitVoices();
	InitCoreVolume(flag & 0xF);
	EnableIntr(IOP_IRQ_DMA_SPU);
	EnableIntr(IOP_IRQ_DMA_SPU2);
	EnableIntr(IOP_IRQ_SPU);
	RegisterIntrHandler(IOP_IRQ_DMA_SPU, 1, (int (*)(void *))TransInterrupt, &g_TransIntrData[0]);
	RegisterIntrHandler(IOP_IRQ_DMA_SPU2, 1, (int (*)(void *))TransInterrupt, &g_TransIntrData[1]);
	RegisterIntrHandler(IOP_IRQ_SPU, 1, (int (*)(void *))Spu2Interrupt, g_Spu2IntrHandlerData);
	g_vars_inited = 1;
	return resetres;
}

int sceSdQuit()
{
	int intrstate;
	int i;

	// Unofficial: rerolled
	for ( i = 0; i < 2; i += 1 )
		DmaStartStop((i << 4) | 0xA, 0, 0);
	// Unofficial: rerolled
	for ( i = 0; i < 2; i += 1 )
		if ( g_VoiceTransCompleteEf[i] > 0 )
			DeleteEventFlag(g_VoiceTransCompleteEf[i]);
	DisableIntr(IOP_IRQ_DMA_SPU2, &intrstate);
	DisableIntr(IOP_IRQ_DMA_SPU, &intrstate);
	DisableIntr(IOP_IRQ_SPU, &intrstate);
	ReleaseIntrHandler(IOP_IRQ_DMA_SPU2);
	ReleaseIntrHandler(IOP_IRQ_DMA_SPU);
	ReleaseIntrHandler(IOP_IRQ_SPU);
	return 0;
}

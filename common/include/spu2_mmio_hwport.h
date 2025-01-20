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
 * Definitions for memory-mapped I/O for SPU2.
 */

#ifndef __SPU2_MMIO_HWPORT__
#define __SPU2_MMIO_HWPORT__

typedef struct spu2_u16pair_
{
	vu16 m_pair[2];
} spu2_u16pair_t;

typedef struct spu2_voice_params_
{
	vu16 m_voll;
	vu16 m_volr;
	vu16 m_pitch;
	vu16 m_adsr1;
	vu16 m_adsr2;
	vu16 m_envx;
	vu16 m_volxl;
	vu16 m_volxr;
} spu2_voice_params_t;

typedef struct spu2_voice_address_
{
	spu2_u16pair_t m_ssa;
	spu2_u16pair_t m_lsax;
	spu2_u16pair_t m_nax;
} spu2_voice_address_t;

typedef struct spu2_core_regs_
{
	spu2_voice_params_t m_voice_params[24];   /* 0x000 */
	spu2_u16pair_t m_pmon;                    /* 0x180 */
	spu2_u16pair_t m_non;                     /* 0x184 */
	spu2_u16pair_t m_vmixl;                   /* 0x188 */
	spu2_u16pair_t m_vmixel;                  /* 0x18c */
	spu2_u16pair_t m_vmixr;                   /* 0x190 */
	spu2_u16pair_t m_vmixer;                  /* 0x194 */
	vu16 m_mmix;                              /* 0x198 */
	vu16 m_attr;                              /* 0x19a */
	spu2_u16pair_t m_irqa;                    /* 0x19c */
	spu2_u16pair_t m_kon;                     /* 0x1a0 */
	spu2_u16pair_t m_koff;                    /* 0x1a4 */
	spu2_u16pair_t m_tsa;                     /* 0x1a8 */
	vu16 m_xferdata;                          /* 0x1ac */
	vu16 m_unk1ae;                            /* 0x1ae */
	vu16 m_admas;                             /* 0x1b0 */
	vu16 unk1b2[7];                           /* 0x1b2 */
	spu2_voice_address_t m_voice_address[24]; /* 0x1c0 */
	spu2_u16pair_t m_esa;                     /* 0x2e0 */
	spu2_u16pair_t m_apf1_size;               /* 0x2e4 */
	spu2_u16pair_t m_apf2_size;               /* 0x2e8 */
	spu2_u16pair_t m_same_l_dst;              /* 0x2EC */
	spu2_u16pair_t m_same_r_dst;              /* 0x2F0 */
	spu2_u16pair_t m_comb1_l_src;             /* 0x2F4 */
	spu2_u16pair_t m_comb1_r_src;             /* 0x2F8 */
	spu2_u16pair_t m_comb2_l_src;             /* 0x2FC */
	spu2_u16pair_t m_comb2_r_src;             /* 0x300 */
	spu2_u16pair_t m_same_l_src;              /* 0x304 */
	spu2_u16pair_t m_same_r_src;              /* 0x308 */
	spu2_u16pair_t m_diff_l_dst;              /* 0x30C */
	spu2_u16pair_t m_diff_r_dst;              /* 0x310 */
	spu2_u16pair_t m_comb3_l_src;             /* 0x314 */
	spu2_u16pair_t m_comb3_r_src;             /* 0x318 */
	spu2_u16pair_t m_comb4_l_src;             /* 0x31C */
	spu2_u16pair_t m_comb4_r_src;             /* 0x320 */
	spu2_u16pair_t m_diff_l_src;              /* 0x324 */
	spu2_u16pair_t m_diff_r_src;              /* 0x328 */
	spu2_u16pair_t m_apf1_l_dst;              /* 0x32C */
	spu2_u16pair_t m_apf1_r_dst;              /* 0x330 */
	spu2_u16pair_t m_apf2_l_dst;              /* 0x334 */
	spu2_u16pair_t m_apf2_r_dst;              /* 0x338 */
	vu16 m_eea;                               /* 0x33C */
	vu16 unk33e;                              /* 0x33E */
	spu2_u16pair_t m_endx;                    /* 0x340 */
	vu16 m_statx;                             /* 0x344 */
	vu16 unk346[13];                          /* 0x346 */
} spu2_core_regs_t;

typedef struct spu2_different_regs_
{
	vu16 m_mvoll;     /* 0x760 */
	vu16 m_mvolr;     /* 0x762 */
	vu16 m_evoll;     /* 0x764 */
	vu16 m_evolr;     /* 0x766 */
	vu16 m_avoll;     /* 0x768 */
	vu16 m_avolr;     /* 0x76A */
	vu16 m_bvoll;     /* 0x76C */
	vu16 m_bvolr;     /* 0x76E */
	vu16 m_mvolxl;    /* 0x770 */
	vu16 m_mvolxr;    /* 0x772 */
	vu16 m_iir_vol;   /* 0x774 */
	vu16 m_comb1_vol; /* 0x776 */
	vu16 m_comb2_vol; /* 0x778 */
	vu16 m_comb3_vol; /* 0x77A */
	vu16 m_comb4_vol; /* 0x77C */
	vu16 m_wall_vol;  /* 0x77E */
	vu16 m_apf1_vol;  /* 0x780 */
	vu16 m_apf2_vol;  /* 0x782 */
	vu16 m_in_coef_l; /* 0x784 */
	vu16 m_in_coef_r; /* 0x786 */
} spu2_different_regs_t;

typedef struct spu2_core_regs_padded_
{
	spu2_core_regs_t m_cregs;
	vu16 padding[80];
} spu2_core_regs_padded_t;

typedef struct spu2_regs_main_
{
	spu2_core_regs_padded_t m_core_regs[2];
} spu2_regs_main_t;

typedef struct spu2_regs_extra_
{
	spu2_core_regs_t core0_regs;
	vu16 padding346[80];
	spu2_core_regs_t core1_regs; /* 0x400 */
	spu2_different_regs_t m_different_regs[2];
	vu16 unk7b0[8];
	vu16 m_spdif_out;     /* 0x7c0 */
	vu16 m_spdif_irqinfo; /* 0x7c2 */
	vu16 unk7c4;          /* 0x7c4 */
	vu16 m_spdif_mode;    /* 0x7c6 */
	vu16 m_spdif_media;   /* 0x7c8 */
	vu16 m_unknown7ca;    /* 0x7ca */
	vu16 m_spdif_protect; /* 0x7cc */
	vu16 unk7ce[25];
} spu2_regs_extra_t;

typedef struct spu2_mmio_hwport_ /* base => 0xBF900000 */
{
	union spu2_regs_union_
	{
		spu2_regs_main_t m_m;
		spu2_regs_extra_t m_e;
	} m_u;
} spu2_mmio_hwport_t;

#if !defined(USE_SPU2_MMIO_HWPORT) && defined(_IOP)
// cppcheck-suppress-macro constVariablePointer
#define USE_SPU2_MMIO_HWPORT() spu2_mmio_hwport_t *const spu2_mmio_hwport = (spu2_mmio_hwport_t *)0xBF900000
#endif
#if !defined(USE_SPU2_MMIO_HWPORT)
#define USE_SPU2_MMIO_HWPORT()
#endif

#endif /* __SPU2_MMIO_HWPORT__ */

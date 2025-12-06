/*
# _____     ___ ____     ___ ____
#  ____|   |    ____|   |        | |____|
# |     ___|   |____ ___|    ____| |    \    PS2DEV Open Source Project.
#-----------------------------------------------------------------------
# Copyright ps2dev - http://www.ps2dev.org
# Licenced under Academic Free License version 2.0
# Review ps2sdk README & LICENSE files for further details.
*/

#ifdef _IOP
#include "irx_imports.h"
#else
#include <stdint.h>
typedef int8_t s8;
typedef int16_t s16;
typedef int32_t s32;
typedef uint8_t u8;
typedef uint16_t u16;
typedef uint32_t u32;
#endif
#include <sdhd.h>

#ifdef _IOP
IRX_ID("Sound_Data_HD", 2, 2);
#endif
// Based on the module from SCE SDK 3.1.0.

struct sceHardSynthVersionChunk_
{
	unsigned int Creator;
	unsigned int Type;
	unsigned int chunkSize;
	u16 reserved;
	u8 versionMajor;
	u8 versionMinor;
};

typedef struct sceHardSynthVersionChunk_ sceHardSynthVersionChunk;
typedef struct sceHardSynthVersionChunk_ sceSeqVersionChunk;

typedef struct sceHardSynthHeaderChunk_
{
	unsigned int Creator;
	unsigned int Type;
	unsigned int chunkSize;
	unsigned int headerSize;
	unsigned int bodySize;
	unsigned int programChunkAddr;
	unsigned int sampleSetChunkAddr;
	unsigned int sampleChunkAddr;
	unsigned int vagInfoChunkAddr;
	unsigned int seTimbreChunkAddr;
} sceHardSynthHeaderChunk;

typedef struct sceHardSynthProgramChunk_
{
	unsigned int Creator;
	unsigned int Type;
	unsigned int chunkSize;
	unsigned int maxProgramNumber;
	unsigned int programOffsetAddr[];
} sceHardSynthProgramChunk;

typedef struct sceHardSynthSampleSetChunk_
{
	unsigned int Creator;
	unsigned int Type;
	unsigned int chunkSize;
	unsigned int maxSampleSetNumber;
	unsigned int sampleSetOffsetAddr[];
} sceHardSynthSampleSetChunk;

typedef struct sceHardSynthSampleChunk_
{
	unsigned int Creator;
	unsigned int Type;
	unsigned int chunkSize;
	unsigned int maxSampleNumber;
	unsigned int sampleOffsetAddr[];
} sceHardSynthSampleChunk;

typedef struct sceHardSynthVagInfoChunk
{
	unsigned int Creator;
	unsigned int Type;
	unsigned int chunkSize;
	unsigned int maxVagInfoNumber;
	unsigned int vagInfoOffsetAddr[];
} sceHardSynthVagInfoChunk;

struct sdhd_info
{
	sceHardSynthVersionChunk *m_vers;
	sceHardSynthHeaderChunk *m_head;
	sceHardSynthProgramChunk *m_prog;
	sceHardSynthSampleSetChunk *m_sset;
	sceHardSynthSampleChunk *m_smpl;
	sceHardSynthVagInfoChunk *m_vagi;
};

#ifdef _IOP
extern struct irx_export_table _exp_sdhd;
#endif

static unsigned int do_get_vers_head_chunk(sceHardSynthVersionChunk *indata, struct sdhd_info *dinfo)
{
	dinfo->m_vers = 0;
	dinfo->m_head = 0;
	dinfo->m_prog = 0;
	dinfo->m_sset = 0;
	dinfo->m_smpl = 0;
	dinfo->m_vagi = 0;
	dinfo->m_vers = indata;
	if ( indata->Creator != 0x53434549 || indata->Type != 0x56657273 )
	{
		dinfo->m_vers = 0;
		return 0x8103000E;
	}
	if ( (indata->chunkSize & 0x80000000) )
	{
		dinfo->m_vers = 0;
		return 0x8103002F;
	}
	dinfo->m_head = (sceHardSynthHeaderChunk *)((char *)indata + indata->chunkSize);
	if ( dinfo->m_head->Creator != 0x53434549 || dinfo->m_head->Type != 0x48656164 )
	{
		dinfo->m_vers = 0;
		dinfo->m_head = 0;
		return 0x8103002F;
	}
	return 0;
}

static unsigned int do_get_prog_chunk(void *indata, struct sdhd_info *dinfo)
{
	if ( dinfo->m_head->programChunkAddr == 0xFFFFFFFF )
		return 0x81039005;
	if ( (dinfo->m_head->programChunkAddr & 0x80000000) )
		return 0x8103002F;
	dinfo->m_prog = (sceHardSynthProgramChunk *)((char *)indata + dinfo->m_head->programChunkAddr);
	if ( dinfo->m_prog->Creator != 0x53434549 || dinfo->m_prog->Type != 0x50726F67 )
	{
		dinfo->m_prog = 0;
		return 0x8103002F;
	}
	return 0;
}

static unsigned int do_get_sset_chunk(void *indata, struct sdhd_info *dinfo)
{
	if ( dinfo->m_head->sampleSetChunkAddr == 0xFFFFFFFF )
		return 0x81039006;
	if ( (dinfo->m_head->sampleSetChunkAddr & 0x80000000) )
		return 0x8103002F;
	dinfo->m_sset = (sceHardSynthSampleSetChunk *)((char *)indata + dinfo->m_head->sampleSetChunkAddr);
	if ( dinfo->m_sset->Creator != 0x53434549 || dinfo->m_sset->Type != 0x53736574 )
	{
		dinfo->m_sset = 0;
		return 0x8103002F;
	}
	return 0;
}

static unsigned int do_get_smpl_chunk(void *indata, struct sdhd_info *dinfo)
{
	if ( dinfo->m_head->sampleChunkAddr == 0xFFFFFFFF )
		return 0x81039007;
	if ( (dinfo->m_head->sampleChunkAddr & 0x80000000) )
		return 0x8103002F;
	dinfo->m_smpl = (sceHardSynthSampleChunk *)((char *)indata + dinfo->m_head->sampleChunkAddr);
	if ( dinfo->m_smpl->Creator != 0x53434549 || dinfo->m_smpl->Type != 0x536D706C )
	{
		dinfo->m_smpl = 0;
		return 0x8103002F;
	}
	return 0;
}

static unsigned int do_get_vagi_chunk(void *indata, struct sdhd_info *dinfo)
{
	if ( dinfo->m_head->vagInfoChunkAddr == 0xFFFFFFFF )
		return 0x81039008;
	if ( (dinfo->m_head->vagInfoChunkAddr & 0x80000000) )
		return 0x8103002F;
	dinfo->m_vagi = (sceHardSynthVagInfoChunk *)((char *)indata + dinfo->m_head->vagInfoChunkAddr);
	if ( dinfo->m_vagi->Creator != 0x53434549 || dinfo->m_vagi->Type != 0x56616769 )
	{
		dinfo->m_vagi = 0;
		return 0x8103002F;
	}
	return 0;
}

static void do_copy_to_sdhd_program_param(SceSdHdProgramParam *dst, const sceHardSynthProgramParam *src)
{
	dst->nSplit = src->nSplit;
	dst->progAttr = src->progAttr;
	dst->common.volume = src->progVolume;
	dst->common.panpot = src->progPanpot;
	dst->common.transpose = src->progTranspose;
	dst->common.detune = src->progDetune;
	dst->keyFollow.pan = src->keyFollowPan;
	dst->keyFollow.panCenter = src->keyFollowPanCenter;
	dst->LFO.wavePitch = src->progLfoWave;
	dst->LFO.waveAmp = src->progLfoWave2;
	dst->LFO.startPhasePitch = src->progLfoStartPhase;
	dst->LFO.startPhaseAmp = src->progLfoStartPhase2;
	dst->LFO.phaseRandomPitch = src->progLfoPhaseRandom;
	dst->LFO.phaseRandomAmp = src->progLfoPhaseRandom2;
	dst->LFO.cyclePitch = src->progLfoFreq;
	dst->LFO.cycleAmp = src->progLfoFreq2;
	dst->LFO.pitchDepthUp = src->progLfoPitchDepth;
	dst->LFO.pitchDepthDown = src->progLfoPitchDepth2;
	dst->LFO.midiPitchDepthUp = src->progLfoMidiPitchDepth;
	dst->LFO.midiPitchDepthDown = src->progLfoMidiPitchDepth2;
	dst->LFO.ampDepthUp = src->progLfoAmpDepth;
	dst->LFO.ampDepthDown = src->progLfoAmpDepth2;
	dst->LFO.midiAmpDepthUp = src->progLfoMidiAmpDepth;
	dst->LFO.midiAmpDepthDown = src->progLfoMidiAmpDepth2;
}

static void do_copy_to_sdhd_split_block(SceSdHdSplitBlock *dst, const sceHardSynthSplitBlock *src)
{
	dst->sampleSetIndex = src->sampleSetIndex;
	dst->splitNumber = src->splitNumber;
	dst->range.low = src->splitRangeLow;
	dst->range.crossFade = src->splitCrossFade;
	dst->range.high = src->splitRangeHigh;
	dst->bendRange.low = src->splitBendRangeLow;
	dst->bendRange.high = src->splitBendRangeHigh;
	dst->keyFollow.pitch = src->keyFollowPitch;
	dst->keyFollow.pitchCenter = src->keyFollowPitchCenter;
	dst->keyFollow.amp = src->keyFollowAmp;
	dst->keyFollow.ampCenter = src->keyFollowAmpCenter;
	dst->keyFollow.pan = src->keyFollowPan;
	dst->keyFollow.panCenter = src->keyFollowPanCenter;
	dst->common.volume = src->splitVolume;
	dst->common.panpot = src->splitPanpot;
	dst->common.transpose = src->splitTranspose;
	dst->common.detune = src->splitDetune;
}

static void do_copy_to_sdhd_set_param(SceSdHdSampleSetParam *dst, const sceHardSynthSampleSetParam *src)
{
	dst->velCurve = src->velCurve;
	dst->velLimitLow = src->velLimitLow;
	dst->velLimitHigh = src->velLimitHigh;
	dst->nSample = src->nSample;
}

static void do_copy_to_sdhd_sample_param(SceSdHdSampleParam *dst, const sceHardSynthSampleParam *src)
{
	dst->vagIndex = src->VagIndex;
	dst->spuAttr = src->sampleSpuAttr;
	dst->lfoAttr = src->sampleLfoAttr;
	dst->velRange.low = src->velRangeLow;
	dst->velRange.crossFade = src->velCrossFade;
	dst->velRange.high = src->velRangeHigh;
	dst->velFollow.pitch = src->velFollowPitch;
	dst->velFollow.pitchCenter = src->velFollowPitchCenter;
	dst->velFollow.pitchVelCurve = src->velFollowPitchVelCurve;
	dst->velFollow.amp = src->velFollowAmp;
	dst->velFollow.ampCenter = src->velFollowAmpCenter;
	dst->velFollow.ampVelCurve = src->velFollowAmpVelCurve;
	dst->common.baseNote = src->sampleBaseNote;
	dst->common.detune = src->sampleDetune;
	dst->common.panpot = src->samplePanpot;
	dst->common.group = src->sampleGroup;
	dst->common.priority = src->samplePriority;
	dst->common.volume = src->sampleVolume;
	dst->ADSR.ADSR1 = src->sampleAdsr1;
	dst->ADSR.ADSR2 = src->sampleAdsr2;
	dst->keyFollow.ar = src->keyFollowAr;
	dst->keyFollow.arCenter = src->keyFollowArCenter;
	dst->keyFollow.dr = src->keyFollowDr;
	dst->keyFollow.drCenter = src->keyFollowDrCenter;
	dst->keyFollow.sr = src->keyFollowSr;
	dst->keyFollow.srCenter = src->keyFollowSrCenter;
	dst->keyFollow.rr = src->keyFollowRr;
	dst->keyFollow.rrCenter = src->keyFollowRrCenter;
	dst->keyFollow.sl = src->keyFollowSl;
	dst->keyFollow.slCenter = src->keyFollowSlCenter;
	dst->LFO.pitchLFODelay = src->samplePitchLfoDelay;
	dst->LFO.pitchLFOFade = src->samplePitchLfoFade;
	dst->LFO.ampLFODelay = src->sampleAmpLfoDelay;
	dst->LFO.ampLFOFade = src->sampleAmpLfoFade;
}

static void do_copy_to_sdhd_vag_info_param(SceSdHdVAGInfoParam *dst, unsigned int sz, const sceHardSynthVagParam *src)
{
	dst->vagOffsetAddr = src->vagOffsetAddr;
	dst->vagSampleRate = src->vagSampleRate;
	dst->vagSize = sz;
	dst->vagAttribute = src->vagAttribute;
}

static unsigned int do_get_vag_size(sceHardSynthVersionChunk *indata, const unsigned int *vagoffsaddr)
{
	unsigned int i;
	unsigned int bodySize;
	const sceHardSynthVagParam *vagparam;
	struct sdhd_info dinfo;

	if ( do_get_vers_head_chunk(indata, &dinfo) || do_get_vagi_chunk(indata, &dinfo) )
		return 0;
	bodySize = dinfo.m_head->bodySize;
	for ( i = 0; dinfo.m_vagi->maxVagInfoNumber >= i; i += 1 )
	{
		if ( dinfo.m_vagi->vagInfoOffsetAddr[i] != 0xFFFFFFFF )
		{
			vagparam = (sceHardSynthVagParam *)((char *)(dinfo.m_vagi) + dinfo.m_vagi->vagInfoOffsetAddr[i]);
			if ( *vagoffsaddr < vagparam->vagOffsetAddr && vagparam->vagOffsetAddr < bodySize )
				bodySize = vagparam->vagOffsetAddr;
		}
	}
	return bodySize - *vagoffsaddr;
}

static unsigned int
do_check_chunk_in_bounds(void *indata, const struct sdhd_info *dinfo, unsigned int hdrmagic, unsigned int idx)
{
	(void)indata;

	switch ( hdrmagic )
	{
		case 0x536D706C:
		{
			if ( !dinfo->m_smpl )
				return 0x8103002F;
			if ( dinfo->m_smpl->maxSampleNumber < idx )
				return 0x81039014;
			if ( dinfo->m_smpl->sampleOffsetAddr[idx] == 0xFFFFFFFF )
				return 0x81039015;
			break;
		}
		case 0x53736574:
		{
			if ( !dinfo->m_sset )
				return 0x8103002F;
			if ( dinfo->m_sset->maxSampleSetNumber < idx )
				return 0x81039012;
			if ( dinfo->m_sset->sampleSetOffsetAddr[idx] == 0xFFFFFFFF )
				return 0x81039012;
			break;
		}
		case 0x56616769:
		{
			if ( !dinfo->m_vagi )
				return 0x8103002F;
			if ( dinfo->m_vagi->maxVagInfoNumber < idx )
				return 0x81039016;
			if ( dinfo->m_vagi->vagInfoOffsetAddr[idx] == 0xFFFFFFFF )
				return 0x81039017;
			break;
		}
		case 0x50726F67:
		{
			if ( !dinfo->m_prog )
				return 0x8103002F;
			if ( dinfo->m_prog->maxProgramNumber < idx )
				return 0x81039010;
			if ( dinfo->m_prog->programOffsetAddr[idx] == 0xFFFFFFFF )
				return 0x81039011;
			break;
		}
		default:
			break;
	}
	return 0;
}

static int do_get_common_block_ptr_note(
	void *buffer,
	unsigned int programNumber,
	unsigned int swcase,
	unsigned int noteNumber,
	unsigned int velocity,
	unsigned int mode,
	void **ptr)
{
	int idx1;
	int i;
	sceHardSynthSplitBlock *p_splitblock;
	int j;
	sceHardSynthProgramParam *p_programparam;
	sceHardSynthSampleSetParam *p_samplesetparam;
	sceHardSynthSampleParam *p_sampleparam;
	sceHardSynthVagParam *p_vagparam;

	idx1 = 0;
	if (
		sceSdHdGetProgramParamAddr(buffer, programNumber, &p_programparam) || p_programparam->splitBlockAddr == 0xFFFFFFFF )
	{
		return 0;
	}
	for ( i = 0; i < p_programparam->nSplit; i += 1 )
	{
		p_splitblock = (sceHardSynthSplitBlock *)((char *)p_programparam + p_programparam->splitBlockAddr
																							+ p_programparam->sizeSplitBlock * i);
		if ( noteNumber < (p_splitblock->splitRangeLow & 0x7Fu) || noteNumber > (p_splitblock->splitRangeHigh & 0x7Fu) )
		{
			continue;
		}
		switch ( swcase )
		{
			case 0:
				idx1 += 1;
				break;
			case 1:
				idx1 += 1;
				*ptr = p_splitblock;
				ptr = (void **)(((char *)ptr) + sizeof(void *));
				break;
			case 2:
				idx1 += 1;
				do_copy_to_sdhd_split_block((SceSdHdSplitBlock *)ptr, p_splitblock);
				ptr = (void **)(((char *)ptr) + sizeof(SceSdHdSplitBlock));
				break;
			default:
				if (
					p_splitblock->sampleSetIndex == 0xFFFF
					|| sceSdHdGetSampleSetParamAddr(buffer, p_splitblock->sampleSetIndex, &p_samplesetparam) )
				{
					break;
				}
				switch ( swcase )
				{
					case 3:
						idx1 += 1;
						break;
					case 4:
						idx1 += 1;
						*ptr = p_samplesetparam;
						ptr = (void **)(((char *)ptr) + sizeof(void *));
						break;
					case 5:
						idx1 += 1;
						do_copy_to_sdhd_set_param((SceSdHdSampleSetParam *)ptr, p_samplesetparam);
						ptr = (void **)(((char *)ptr) + sizeof(SceSdHdSampleSetParam));
						break;
					default:
						switch ( mode )
						{
							case 0:
								if ( velocity < p_samplesetparam->velLimitLow || velocity > p_samplesetparam->velLimitHigh )
								{
									break;
								}
								for ( j = 0; j < p_samplesetparam->nSample; j += 1 )
								{
									if (
										p_samplesetparam->sampleIndex[j] != 0xFFFF
										&& !sceSdHdGetSampleParamAddr(buffer, p_samplesetparam->sampleIndex[j], &p_sampleparam)
										&& velocity >= (p_sampleparam->velRangeLow & 0x7Fu)
										&& (p_sampleparam->velRangeHigh & 0x7Fu) >= velocity )
									{
										switch ( swcase )
										{
											case 6:
												idx1 += 1;
												break;
											case 7:
												idx1 += 1;
												*ptr = p_sampleparam;
												ptr = (void **)(((char *)ptr) + sizeof(void *));
												break;
											case 8:
												idx1 += 1;
												do_copy_to_sdhd_sample_param((SceSdHdSampleParam *)ptr, p_sampleparam);
												ptr = (void **)(((char *)ptr) + sizeof(SceSdHdSampleParam));
												break;
											default:
												if (
													p_sampleparam->VagIndex == 0xFFFF
													|| sceSdHdGetVAGInfoParamAddr(buffer, p_sampleparam->VagIndex, &p_vagparam) )
												{
													break;
												}
												switch ( swcase )
												{
													case 9:
														idx1 += 1;
														break;
													case 10:
														idx1 += 1;
														*ptr = p_vagparam;
														ptr = (void **)(((char *)ptr) + sizeof(void *));
														break;
													case 11:
														idx1 += 1;
														do_copy_to_sdhd_vag_info_param(
															(SceSdHdVAGInfoParam *)ptr,
															do_get_vag_size((sceHardSynthVersionChunk *)buffer, &p_vagparam->vagOffsetAddr),
															p_vagparam);
														ptr = (void **)(((char *)ptr) + sizeof(SceSdHdVAGInfoParam));
														break;
													default:
														break;
												}
												break;
										}
									}
								}
								break;
							case 1:
								for ( j = 0; j < p_samplesetparam->nSample; j += 1 )
								{
									if (
										p_samplesetparam->sampleIndex[j] != 0xFFFF
										&& !sceSdHdGetSampleParamAddr(buffer, p_samplesetparam->sampleIndex[j], &p_sampleparam)
										&& velocity >= (p_sampleparam->velRangeLow & 0x7Fu)
										&& (p_sampleparam->velRangeHigh & 0x7Fu) >= velocity )
									{
										switch ( swcase )
										{
											case 6:
												idx1 += 1;
												break;
											case 7:
												idx1 += 1;
												*ptr = p_sampleparam;
												ptr = (void **)(((char *)ptr) + sizeof(void *));
												break;
											case 8:
												idx1 += 1;
												do_copy_to_sdhd_sample_param((SceSdHdSampleParam *)ptr, p_sampleparam);
												ptr = (void **)(((char *)ptr) + sizeof(SceSdHdSampleParam));
												break;
											default:
												if (
													p_sampleparam->VagIndex == 0xFFFF
													|| sceSdHdGetVAGInfoParamAddr(buffer, p_sampleparam->VagIndex, &p_vagparam) )
												{
													break;
												}
												switch ( swcase )
												{
													case 9:
														idx1 += 1;
														break;
													case 10:
														idx1 += 1;
														*ptr = p_vagparam;
														ptr = (void **)(((char *)ptr) + sizeof(void *));
														break;
													case 11:
														idx1 += 1;
														do_copy_to_sdhd_vag_info_param(
															(SceSdHdVAGInfoParam *)ptr,
															do_get_vag_size((sceHardSynthVersionChunk *)buffer, &p_vagparam->vagOffsetAddr),
															p_vagparam);
														ptr = (void **)(((char *)ptr) + sizeof(SceSdHdVAGInfoParam));
														break;
													default:
														break;
												}
												break;
										}
									}
								}
								break;
							default:
								break;
						}
						break;
				}
				break;
		}
	}
	return idx1;
}

static int do_get_common_block_ptr(
	void *buffer,
	unsigned int sampleSetNumber,
	unsigned int swcase,
	unsigned int velocity,
	unsigned int mode,
	void **param)
{
	int idx1;
	int i;
	sceHardSynthSampleSetParam *p_samplesetparam;
	sceHardSynthSampleParam *p_sampleparam;
	sceHardSynthVagParam *p_vagparam;

	idx1 = 0;
	if ( sceSdHdGetSampleSetParamAddr(buffer, sampleSetNumber, &p_samplesetparam) )
		return 0;
	switch ( mode )
	{
		case 0:
			if ( velocity < p_samplesetparam->velLimitLow || velocity > p_samplesetparam->velLimitHigh )
				return 0;
			for ( i = 0; i < p_samplesetparam->nSample; i += 1 )
			{
				if (
					p_samplesetparam->sampleIndex[i] != 0xFFFF
					&& !sceSdHdGetSampleParamAddr(buffer, p_samplesetparam->sampleIndex[i], &p_sampleparam)
					&& velocity >= (p_sampleparam->velRangeLow & 0x7Fu) && (p_sampleparam->velRangeHigh & 0x7Fu) >= velocity )
				{
					switch ( swcase )
					{
						case 6:
							idx1 += 1;
							break;
						case 7:
							idx1 += 1;
							*param = p_sampleparam;
							param = (void **)(((char *)param) + sizeof(void *));
							break;
						case 8:
							idx1 += 1;
							do_copy_to_sdhd_sample_param((SceSdHdSampleParam *)param, p_sampleparam);
							param = (void **)(((char *)param) + sizeof(SceSdHdSampleParam));
							break;
						default:
							if (
								p_sampleparam->VagIndex == 0xFFFF
								|| sceSdHdGetVAGInfoParamAddr(buffer, p_sampleparam->VagIndex, &p_vagparam) )
							{
								break;
							}
							switch ( swcase )
							{
								case 9:
									idx1 += 1;
									break;
								case 10:
									idx1 += 1;
									*param = p_vagparam;
									param = (void **)(((char *)param) + sizeof(void *));
									break;
								case 11:
									idx1 += 1;
									do_copy_to_sdhd_vag_info_param(
										(SceSdHdVAGInfoParam *)param,
										do_get_vag_size((sceHardSynthVersionChunk *)buffer, &p_vagparam->vagOffsetAddr),
										p_vagparam);
									param = (void **)(((char *)param) + sizeof(SceSdHdVAGInfoParam));
									break;
								default:
									break;
							}
					}
				}
			}
			return idx1;
		case 1:
			for ( i = 0; i < p_samplesetparam->nSample; i += 1 )
			{
				if (
					p_samplesetparam->sampleIndex[i] != 0xFFFF
					&& !sceSdHdGetSampleParamAddr(buffer, p_samplesetparam->sampleIndex[i], &p_sampleparam)
					&& velocity >= (p_sampleparam->velRangeLow & 0x7Fu) && (p_sampleparam->velRangeHigh & 0x7Fu) >= velocity )
				{
					switch ( swcase )
					{
						case 6:
							idx1 += 1;
							break;
						case 7:
							idx1 += 1;
							*param = p_sampleparam;
							param = (void **)(((char *)param) + sizeof(void *));
							break;
						case 8:
							idx1 += 1;
							do_copy_to_sdhd_sample_param((SceSdHdSampleParam *)param, p_sampleparam);
							param = (void **)(((char *)param) + sizeof(SceSdHdSampleParam));
							break;
						default:
							if (
								p_sampleparam->VagIndex == 0xFFFF
								|| sceSdHdGetVAGInfoParamAddr(buffer, p_sampleparam->VagIndex, &p_vagparam) )
								break;
							switch ( swcase )
							{
								case 9:
									idx1 += 1;
									break;
								case 10:
									idx1 += 1;
									*param = p_vagparam;
									param = (void **)(((char *)param) + sizeof(void *));
									break;
								case 11:
									idx1 += 1;
									do_copy_to_sdhd_vag_info_param(
										(SceSdHdVAGInfoParam *)param,
										do_get_vag_size((sceHardSynthVersionChunk *)buffer, &p_vagparam->vagOffsetAddr),
										p_vagparam);
									param = (void **)(((char *)param) + sizeof(SceSdHdVAGInfoParam));
									break;
								default:
									break;
							}
							break;
					}
				}
			}
			return idx1;
		default:
			return 0;
	}
}

int sceSdHdGetMaxProgramNumber(void *buffer)
{
	int result;
	struct sdhd_info dinfo;

	result = (int)do_get_vers_head_chunk((sceHardSynthVersionChunk *)buffer, &dinfo);
	if ( result )
		return result;
	result = (int)do_get_prog_chunk(buffer, &dinfo);
	if ( result )
		return result;
	return (int)dinfo.m_prog->maxProgramNumber;
}

int sceSdHdGetMaxSampleSetNumber(void *buffer)
{
	int result;
	struct sdhd_info dinfo;

	result = (int)do_get_vers_head_chunk((sceHardSynthVersionChunk *)buffer, &dinfo);
	if ( result )
		return result;
	result = (int)do_get_sset_chunk(buffer, &dinfo);
	if ( result )
		return result;
	return (int)dinfo.m_sset->maxSampleSetNumber;
}

int sceSdHdGetMaxSampleNumber(void *buffer)
{
	int result;
	struct sdhd_info dinfo;

	result = (int)do_get_vers_head_chunk((sceHardSynthVersionChunk *)buffer, &dinfo);
	if ( result )
		return result;
	result = (int)do_get_smpl_chunk(buffer, &dinfo);
	if ( result )
		return result;
	return (int)dinfo.m_smpl->maxSampleNumber;
}

int sceSdHdGetMaxVAGInfoNumber(void *buffer)
{
	int result;
	struct sdhd_info dinfo;

	result = (int)do_get_vers_head_chunk((sceHardSynthVersionChunk *)buffer, &dinfo);
	if ( result )
		return result;
	result = (int)do_get_vagi_chunk(buffer, &dinfo);
	if ( result )
		return result;
	return (int)dinfo.m_vagi->maxVagInfoNumber;
}

int sceSdHdGetProgramParamAddr(void *buffer, unsigned int programNumber, sceHardSynthProgramParam **ptr)
{
	int result;
	struct sdhd_info dinfo;

	result = (int)do_get_vers_head_chunk((sceHardSynthVersionChunk *)buffer, &dinfo);
	if ( result )
		return result;
	result = (int)do_get_prog_chunk(buffer, &dinfo);
	if ( result )
		return result;
	result = (int)do_check_chunk_in_bounds(buffer, &dinfo, 0x50726F67u, programNumber);
	if ( result )
		return result;
	*ptr = (sceHardSynthProgramParam *)((char *)dinfo.m_prog + dinfo.m_prog->programOffsetAddr[programNumber]);
	return 0;
}

int sceSdHdGetProgramParam(void *buffer, unsigned int programNumber, SceSdHdProgramParam *param)
{
	int result;
	sceHardSynthProgramParam *p_programparam;

	result = sceSdHdGetProgramParamAddr(buffer, programNumber, &p_programparam);
	if ( result )
		return result;
	do_copy_to_sdhd_program_param(param, p_programparam);
	return 0;
}

int sceSdHdGetSplitBlockAddr(
	void *buffer, unsigned int programNumber, unsigned int splitBlockNumber, sceHardSynthSplitBlock **theParamPtr)
{
	int result;
	sceHardSynthProgramParam *p_programparam;

	result = sceSdHdGetProgramParamAddr(buffer, programNumber, &p_programparam);
	if ( result )
		return result;
	if ( p_programparam->splitBlockAddr == 0xFFFFFFFF )
		return (int)0x81039009;
	if ( (unsigned int)p_programparam->nSplit - 1 < splitBlockNumber )
		return (int)0x81039018;
	*theParamPtr = (sceHardSynthSplitBlock *)((char *)p_programparam + p_programparam->splitBlockAddr
																						+ p_programparam->sizeSplitBlock * splitBlockNumber);
	return 0;
}

int sceSdHdGetSplitBlock(
	void *buffer, unsigned int programNumber, unsigned int splitBlockNumber, SceSdHdSplitBlock *param)
{
	int result;
	sceHardSynthSplitBlock *p_splitblock;

	result = sceSdHdGetSplitBlockAddr(buffer, programNumber, splitBlockNumber, &p_splitblock);
	if ( result )
		return result;
	do_copy_to_sdhd_split_block(param, p_splitblock);
	return 0;
}

int sceSdHdGetSampleSetParamAddr(void *buffer, unsigned int sampleSetNumber, sceHardSynthSampleSetParam **ptr)
{
	int result;
	struct sdhd_info dinfo;

	result = (int)do_get_vers_head_chunk((sceHardSynthVersionChunk *)buffer, &dinfo);
	if ( result )
		return result;
	result = (int)do_get_sset_chunk(buffer, &dinfo);
	if ( result )
		return result;
	result = (int)do_check_chunk_in_bounds(buffer, &dinfo, 0x53736574, sampleSetNumber);
	if ( result )
		return result;
	*ptr = (sceHardSynthSampleSetParam *)((char *)dinfo.m_sset + dinfo.m_sset->sampleSetOffsetAddr[sampleSetNumber]);
	return 0;
}

int sceSdHdGetSampleSetParam(void *buffer, unsigned int sampleSetNumber, SceSdHdSampleSetParam *param)
{
	int result;
	sceHardSynthSampleSetParam *p_samplesetparam;

	result = sceSdHdGetSampleSetParamAddr(buffer, sampleSetNumber, &p_samplesetparam);
	if ( result )
		return result;
	do_copy_to_sdhd_set_param(param, p_samplesetparam);
	return 0;
}

int sceSdHdGetSampleParamAddr(void *buffer, unsigned int sampleNumber, sceHardSynthSampleParam **ptr)
{
	int result;
	struct sdhd_info dinfo;

	result = (int)do_get_vers_head_chunk((sceHardSynthVersionChunk *)buffer, &dinfo);
	if ( result )
		return result;
	result = (int)do_get_smpl_chunk(buffer, &dinfo);
	if ( result )
		return result;
	result = (int)do_check_chunk_in_bounds(buffer, &dinfo, 0x536D706C, sampleNumber);
	if ( result )
		return result;
	*ptr = (sceHardSynthSampleParam *)((char *)dinfo.m_smpl + dinfo.m_smpl->sampleOffsetAddr[sampleNumber]);
	return 0;
}

int sceSdHdGetSampleParam(void *buffer, unsigned int sampleNumber, SceSdHdSampleParam *param)
{
	int result;
	sceHardSynthSampleParam *p_sampleparam;

	result = sceSdHdGetSampleParamAddr(buffer, sampleNumber, &p_sampleparam);
	if ( result )
		return result;
	do_copy_to_sdhd_sample_param(param, p_sampleparam);
	return 0;
}

int sceSdHdGetVAGInfoParamAddr(void *buffer, unsigned int vagInfoNumber, sceHardSynthVagParam **ptr)
{
	int result;
	struct sdhd_info dinfo;

	result = (int)do_get_vers_head_chunk((sceHardSynthVersionChunk *)buffer, &dinfo);
	if ( result )
		return result;
	result = (int)do_get_vagi_chunk(buffer, &dinfo);
	if ( result )
		return result;
	result = (int)do_check_chunk_in_bounds(buffer, &dinfo, 0x56616769, vagInfoNumber);
	if ( result )
		return result;
	*ptr = (sceHardSynthVagParam *)((char *)dinfo.m_vagi + dinfo.m_vagi->vagInfoOffsetAddr[vagInfoNumber]);
	return 0;
}

int sceSdHdGetVAGInfoParam(void *buffer, unsigned int vagInfoNumber, SceSdHdVAGInfoParam *param)
{
	int result;
	sceHardSynthVagParam *p_vagparam;

	result = sceSdHdGetVAGInfoParamAddr(buffer, vagInfoNumber, &p_vagparam);
	if ( result )
		return result;
	do_copy_to_sdhd_vag_info_param(
		param, do_get_vag_size((sceHardSynthVersionChunk *)buffer, &p_vagparam->vagOffsetAddr), p_vagparam);
	return 0;
}

int sceSdHdCheckProgramNumber(void *buffer, unsigned int programNumber)
{
	int result;
	struct sdhd_info dinfo;

	result = (int)do_get_vers_head_chunk((sceHardSynthVersionChunk *)buffer, &dinfo);
	if ( result )
		return result;
	result = (int)do_get_prog_chunk(buffer, &dinfo);
	if ( result )
		return result;
	return (int)do_check_chunk_in_bounds(buffer, &dinfo, 0x50726F67, programNumber);
}

int sceSdHdGetSplitBlockCountByNote(void *buffer, unsigned int programNumber, unsigned int noteNumber)
{
	return do_get_common_block_ptr_note(buffer, programNumber, 0, noteNumber, 0, 0, 0);
}

int sceSdHdGetSplitBlockAddrByNote(
	void *buffer, unsigned int programNumber, unsigned int noteNumber, sceHardSynthSplitBlock **ptr)
{
	return do_get_common_block_ptr_note(buffer, programNumber, 1, noteNumber, 0, 0, (void **)ptr);
}

int sceSdHdGetSplitBlockByNote(
	void *buffer, unsigned int programNumber, unsigned int noteNumber, SceSdHdSplitBlock *param)
{
	return do_get_common_block_ptr_note(buffer, programNumber, 2, noteNumber, 0, 0, (void **)param);
}

int sceSdHdGetSampleSetParamCountByNote(void *buffer, unsigned int programNumber, unsigned int noteNumber)
{
	return do_get_common_block_ptr_note(buffer, programNumber, 3, noteNumber, 0, 0, 0);
}

int sceSdHdGetSampleSetParamAddrByNote(
	void *buffer, unsigned int programNumber, unsigned int noteNumber, sceHardSynthSampleSetParam **ptr)
{
	return do_get_common_block_ptr_note(buffer, programNumber, 4, noteNumber, 0, 0, (void **)ptr);
}

int sceSdHdGetSampleSetParamByNote(
	void *buffer, unsigned int programNumber, unsigned int noteNumber, SceSdHdSampleSetParam *param)
{
	return do_get_common_block_ptr_note(buffer, programNumber, 5, noteNumber, 0, 0, (void **)param);
}

int sceSdHdGetSampleParamCountByNoteVelocity(
	void *buffer, unsigned int programNumber, unsigned int noteNumber, unsigned int velocity, unsigned int mode)
{
	return do_get_common_block_ptr_note(buffer, programNumber, 6, noteNumber, velocity, mode, 0);
}

int sceSdHdGetSampleParamAddrByNoteVelocity(
	void *buffer,
	unsigned int programNumber,
	unsigned int noteNumber,
	unsigned int velocity,
	unsigned int mode,
	sceHardSynthSampleParam **ptr)
{
	return do_get_common_block_ptr_note(buffer, programNumber, 7, noteNumber, velocity, mode, (void **)ptr);
}

int sceSdHdGetSampleParamByNoteVelocity(
	void *buffer,
	unsigned int programNumber,
	unsigned int noteNumber,
	unsigned int velocity,
	unsigned int mode,
	SceSdHdSampleParam *param)
{
	return do_get_common_block_ptr_note(buffer, programNumber, 8, noteNumber, velocity, mode, (void **)param);
}

int sceSdHdGetVAGInfoParamCountByNoteVelocity(
	void *buffer, unsigned int programNumber, unsigned int noteNumber, unsigned int velocity, unsigned int mode)
{
	return do_get_common_block_ptr_note(buffer, programNumber, 9, noteNumber, velocity, mode, 0);
}

int sceSdHdGetVAGInfoParamAddrByNoteVelocity(
	void *buffer,
	unsigned int programNumber,
	unsigned int noteNumber,
	unsigned int velocity,
	unsigned int mode,
	sceHardSynthVagParam **ptr)
{
	return do_get_common_block_ptr_note(buffer, programNumber, 0xA, noteNumber, velocity, mode, (void **)ptr);
}

int sceSdHdGetVAGInfoParamByNoteVelocity(
	void *buffer,
	unsigned int programNumber,
	unsigned int noteNumber,
	unsigned int velocity,
	unsigned int mode,
	SceSdHdVAGInfoParam *param)
{
	return do_get_common_block_ptr_note(buffer, programNumber, 0xB, noteNumber, velocity, mode, (void **)param);
}

int sceSdHdGetSampleParamCountByVelocity(
	void *buffer, unsigned int sampleSetNumber, unsigned int velocity, unsigned int mode)
{
	return do_get_common_block_ptr(buffer, sampleSetNumber, 6, velocity, mode, 0);
}

int sceSdHdGetSampleParamAddrByVelocity(
	void *buffer, unsigned int sampleSetNumber, unsigned int velocity, unsigned int mode, sceHardSynthSampleParam **ptr)
{
	return do_get_common_block_ptr(buffer, sampleSetNumber, 7, velocity, mode, (void **)ptr);
}

int sceSdHdGetSampleParamByVelocity(
	void *buffer, unsigned int sampleSetNumber, unsigned int velocity, unsigned int mode, SceSdHdSampleParam *param)
{
	return do_get_common_block_ptr(buffer, sampleSetNumber, 8, velocity, mode, (void **)param);
}

int sceSdHdGetVAGInfoParamCountByVelocity(
	void *buffer, unsigned int sampleSetNumber, unsigned int velocity, unsigned int mode)
{
	return do_get_common_block_ptr(buffer, sampleSetNumber, 9, velocity, mode, 0);
}

int sceSdHdGetVAGInfoParamAddrByVelocity(
	void *buffer, unsigned int sampleSetNumber, unsigned int velocity, unsigned int mode, sceHardSynthVagParam **ptr)
{
	return do_get_common_block_ptr(buffer, sampleSetNumber, 0xA, velocity, mode, (void **)ptr);
}

int sceSdHdGetVAGInfoParamByVelocity(
	void *buffer, unsigned int sampleSetNumber, unsigned int velocity, unsigned int mode, SceSdHdVAGInfoParam *param)
{
	return do_get_common_block_ptr(buffer, sampleSetNumber, 0xB, velocity, mode, (void **)param);
}

int sceSdHdGetVAGInfoParamAddrBySampleNumber(void *buffer, unsigned int sampleNumber, sceHardSynthVagParam **ptr)
{
	int result;
	sceHardSynthSampleParam *p_sampleparam;

	result = sceSdHdGetSampleParamAddr(buffer, sampleNumber, &p_sampleparam);
	if ( result )
		return result;
	if ( p_sampleparam->VagIndex == 0xFFFF )
		return (int)0x81039019;
	return sceSdHdGetVAGInfoParamAddr(buffer, p_sampleparam->VagIndex, ptr);
}

int sceSdHdGetVAGInfoParamBySampleNumber(void *buffer, unsigned int sampleNumber, SceSdHdVAGInfoParam *param)
{
	int result;
	sceHardSynthVagParam *p_vagparam;

	result = sceSdHdGetVAGInfoParamAddrBySampleNumber(buffer, sampleNumber, &p_vagparam);
	if ( result )
		return result;
	do_copy_to_sdhd_vag_info_param(
		param, do_get_vag_size((sceHardSynthVersionChunk *)buffer, &p_vagparam->vagOffsetAddr), p_vagparam);
	return 0;
}

int sceSdHdGetSplitBlockNumberBySplitNumber(void *buffer, unsigned int programNumber, unsigned int splitNumber)
{
	int result;
	int i;
	sceHardSynthSplitBlock *splitblock;
	sceHardSynthProgramParam *p_programparam;

	result = sceSdHdGetProgramParamAddr(buffer, programNumber, &p_programparam);
	if ( result )
		return result;
	if ( p_programparam->splitBlockAddr == 0xFFFFFFFF )
		return (int)0x81039009;
	splitblock = (sceHardSynthSplitBlock *)((char *)p_programparam + p_programparam->splitBlockAddr);
	for ( i = 0; i < p_programparam->nSplit; i += 1 )
	{
		if ( splitNumber == splitblock->splitNumber )
			return i;
		splitblock = (sceHardSynthSplitBlock *)((char *)splitblock + p_programparam->sizeSplitBlock);
	}
	return (int)0x81039020;
}

int sceSdHdGetVAGSize(void *buffer, unsigned int vagInfoNumber)
{
	int result;
	sceHardSynthVagParam *p_vagparam;

	result = sceSdHdGetVAGInfoParamAddr(buffer, vagInfoNumber, &p_vagparam);
	if ( result )
		return result;
	return (int)do_get_vag_size((sceHardSynthVersionChunk *)buffer, &p_vagparam->vagOffsetAddr);
}

int sceSdHdGetSplitBlockCount(void *buffer, unsigned int programNumber)
{
	int result;
	sceHardSynthProgramParam *p_programparam;

	result = sceSdHdGetProgramParamAddr(buffer, programNumber, &p_programparam);
	if ( result )
		return result;
	return p_programparam->nSplit;
}

int sceSdHdGetMaxSplitBlockCount(void *buffer)
{
	int curminval;
	unsigned int retres;
	unsigned int i;
	unsigned int j;
	int curval1;
	int curval2;
	sceHardSynthProgramParam *p_programparam;
	sceHardSynthSplitBlock *p_splitblock;

	curminval = 0;
	retres = (unsigned int)sceSdHdGetMaxProgramNumber(buffer);
	if ( (retres & 0x80000000) != 0 )
		return 0;
	for ( i = 0; i < retres; i += 1 )
	{
		if ( !sceSdHdGetProgramParamAddr(buffer, i, &p_programparam) )
		{
			for ( j = 0; j < p_programparam->nSplit; j += 1 )
			{
				if ( !sceSdHdGetSplitBlockAddr(buffer, i, j, &p_splitblock) )
				{
					curval1 = sceSdHdGetSplitBlockCountByNote(buffer, i, p_splitblock->splitRangeLow & 0x7F);
					curminval = (curminval < curval1) ? curval1 : curminval;
					curval2 = sceSdHdGetSplitBlockCountByNote(buffer, i, p_splitblock->splitRangeHigh & 0x7F);
					curminval = (curminval < curval2) ? curval2 : curminval;
				}
			}
		}
	}
	return curminval;
}

int sceSdHdGetMaxSampleSetParamCount(void *buffer)
{
	int curminval;
	unsigned int retres;
	unsigned int i;
	unsigned int j;
	int curval1;
	int curval2;
	sceHardSynthProgramParam *p_programparam;
	sceHardSynthSplitBlock *p_splitblock;

	curminval = 0;
	retres = (unsigned int)sceSdHdGetMaxProgramNumber(buffer);
	if ( (retres & 0x80000000) != 0 )
		return 0;
	for ( i = 0; i < retres; i += 1 )
	{
		if ( !sceSdHdGetProgramParamAddr(buffer, i, &p_programparam) )
		{
			for ( j = 0; j < p_programparam->nSplit; j += 1 )
			{
				if ( !sceSdHdGetSplitBlockAddr(buffer, i, j, &p_splitblock) )
				{
					curval1 = sceSdHdGetSampleSetParamCountByNote(buffer, i, p_splitblock->splitRangeLow & 0x7F);
					curminval = (curminval < curval1) ? curval1 : curminval;
					curval2 = sceSdHdGetSampleSetParamCountByNote(buffer, i, p_splitblock->splitRangeHigh & 0x7F);
					curminval = (curminval < curval2) ? curval2 : curminval;
				}
			}
		}
	}
	return curminval;
}

int sceSdHdGetMaxSampleParamCount(void *buffer)
{
	int curminval;
	unsigned int retres;
	unsigned int i;
	unsigned int j;
	unsigned int k;
	int curval1;
	int curval2;
	int curval3;
	int curval4;
	sceHardSynthProgramParam *p_programparam;
	sceHardSynthSplitBlock *p_splitblock;
	sceHardSynthSampleSetParam *p_samplesetparam;
	sceHardSynthSampleParam *p_sampleparam;

	curminval = 0;
	retres = (unsigned int)sceSdHdGetMaxProgramNumber(buffer);
	if ( (retres & 0x80000000) != 0 )
		return 0;
	for ( i = 0; i < retres; i += 1 )
	{
		if ( !sceSdHdGetProgramParamAddr(buffer, i, &p_programparam) )
		{
			for ( j = 0; j < p_programparam->nSplit; j += 1 )
			{
				if (
					!sceSdHdGetSplitBlockAddr(buffer, i, j, &p_splitblock)
					&& !sceSdHdGetSampleSetParamAddr(buffer, p_splitblock->sampleSetIndex, &p_samplesetparam) )
				{
					for ( k = 0; k < p_samplesetparam->nSample; k += 1 )
					{
						if ( !sceSdHdGetSampleParamAddr(
									 buffer,
									 (unsigned int)sceSdHdGetSampleNumberBySampleIndex(buffer, p_splitblock->sampleSetIndex, k),
									 &p_sampleparam) )
						{
							curval1 = sceSdHdGetSampleParamCountByNoteVelocity(
								buffer, i, p_splitblock->splitRangeLow & 0x7F, p_sampleparam->velRangeLow & 0x7F, 1u);
							curminval = (curminval < curval1) ? curval1 : curminval;
							curval2 = sceSdHdGetSampleParamCountByNoteVelocity(
								buffer, i, p_splitblock->splitRangeLow & 0x7F, p_sampleparam->velRangeHigh & 0x7F, 1u);
							curminval = (curminval < curval2) ? curval2 : curminval;
							curval3 = sceSdHdGetSampleParamCountByNoteVelocity(
								buffer, i, p_splitblock->splitRangeHigh & 0x7F, p_sampleparam->velRangeLow & 0x7F, 1u);
							curminval = (curminval < curval3) ? curval3 : curminval;
							curval4 = sceSdHdGetSampleParamCountByNoteVelocity(
								buffer, i, p_splitblock->splitRangeHigh & 0x7F, p_sampleparam->velRangeHigh & 0x7F, 1u);
							curminval = (curminval < curval4) ? curval4 : curminval;
						}
					}
				}
			}
		}
	}
	return curminval;
}

int sceSdHdGetMaxVAGInfoParamCount(void *buffer)
{
	int curminval;
	unsigned int retres;
	unsigned int i;
	unsigned int j;
	unsigned int k;
	int curval1;
	int curval2;
	int curval3;
	int curval4;
	sceHardSynthProgramParam *p_programparam;
	sceHardSynthSplitBlock *p_splitblock;
	sceHardSynthSampleSetParam *p_samplesetparam;
	sceHardSynthSampleParam *p_sampleparam;

	curminval = 0;
	retres = (unsigned int)sceSdHdGetMaxProgramNumber(buffer);
	if ( (retres & 0x80000000) != 0 )
		return 0;
	for ( i = 0; i < retres; i += 1 )
	{
		if ( !sceSdHdGetProgramParamAddr(buffer, i, &p_programparam) )
		{
			for ( j = 0; j < p_programparam->nSplit; j += 1 )
			{
				if (
					!sceSdHdGetSplitBlockAddr(buffer, i, j, &p_splitblock)
					&& !sceSdHdGetSampleSetParamAddr(buffer, p_splitblock->sampleSetIndex, &p_samplesetparam) )
				{
					for ( k = 0; k < p_samplesetparam->nSample; k += 1 )
					{
						if ( !sceSdHdGetSampleParamAddr(
									 buffer,
									 (unsigned int)sceSdHdGetSampleNumberBySampleIndex(buffer, p_splitblock->sampleSetIndex, k),
									 &p_sampleparam) )
						{
							curval1 = sceSdHdGetVAGInfoParamCountByNoteVelocity(
								buffer, i, p_splitblock->splitRangeLow & 0x7F, p_sampleparam->velRangeLow & 0x7F, 1u);
							curminval = (curminval < curval1) ? curval1 : curminval;
							curval2 = sceSdHdGetVAGInfoParamCountByNoteVelocity(
								buffer, i, p_splitblock->splitRangeLow & 0x7F, p_sampleparam->velRangeHigh & 0x7F, 1u);
							curminval = (curminval < curval2) ? curval2 : curminval;
							curval3 = sceSdHdGetVAGInfoParamCountByNoteVelocity(
								buffer, i, p_splitblock->splitRangeHigh & 0x7F, p_sampleparam->velRangeLow & 0x7F, 1u);
							curminval = (curminval < curval3) ? curval3 : curminval;
							curval4 = sceSdHdGetVAGInfoParamCountByNoteVelocity(
								buffer, i, p_splitblock->splitRangeHigh & 0x7F, p_sampleparam->velRangeHigh & 0x7F, 1u);
							curminval = (curminval < curval4) ? curval4 : curminval;
						}
					}
				}
			}
		}
	}
	return curminval;
}

int sceSdHdModifyVelocity(unsigned int curveType, int velocity)
{
	switch ( curveType )
	{
		case 1u:
			return 128 - velocity;
		case 2u:
			velocity = velocity * velocity / 0x7F;
			return velocity ? velocity : 1;
		case 3u:
			velocity = velocity * velocity / 0x7F;
			velocity = velocity ? velocity : 1;
			return 128 - velocity;
		case 4u:
			velocity = (128 - velocity) * (128 - velocity);
			velocity = (velocity / 0x7F) & 0x3FFFFFF;
			velocity = velocity ? velocity : 1;
			return 128 - velocity;
		case 5u:
			velocity = 128 - velocity;
			velocity = velocity * velocity / 0x7F;
			return velocity ? velocity : 1;
		default:
			return velocity;
	}
}

int sceSdHdModifyVelocityLFO(unsigned int curveType, int velocity, int center)
{
	int calc5;
	int calcb;

	center = (center >= 0) ? ((center >= 128) ? 127 : center) : 0;
	velocity = (velocity >= 0) ? ((velocity >= 128) ? 127 : velocity) : 0;
	calc5 = 0;
	switch ( curveType )
	{
		case 0u:
		default:
			calc5 = (velocity - center) << 16;
			calc5 = (calc5 / 126) - (int)((unsigned int)calc5 >> 31);
			break;
		case 1u:
			calc5 = (center - velocity) << 16;
			calc5 = (calc5 / 126) - (int)((unsigned int)calc5 >> 31);
			break;
		case 2u:
			calc5 = ((velocity - 1) << 15) / 126 * (((velocity - 1) << 15) / 126)
						- ((center - 1) << 15) / 126 * (((center - 1) << 15) / 126);
			calc5 = (calc5 < 0) ? ((calc5 + 0x3FFF) >> 14) : (calc5 >> 14);
			break;
		case 3u:
			calc5 = ((velocity - 1) << 15) / 126 * (((velocity - 1) << 15) / 126) / -16384
						- ((center - 1) << 15) / 126 * (((center - 1) << 15) / 126) / -16384;
			break;
		case 4u:
			calc5 = (0x10000 - ((center - 1) << 15) / 126) * (0x10000 - ((center - 1) << 15) / 126) / 0x4000
						- (0x10000 - ((velocity - 1) << 15) / 126) * (0x10000 - ((velocity - 1) << 15) / 126) / 0x4000;
			break;
		case 5u:
			calc5 = (0x10000 - ((velocity - 1) << 15) / 126) * (0x10000 - ((velocity - 1) << 15) / 126) / 0x4000
						- (0x10000 - ((center - 1) << 15) / 126) * (0x10000 - ((center - 1) << 15) / 126) / 0x4000;
			break;
		case 6u:
			if ( velocity == center )
				break;
			calc5 = (center >= velocity) ? (center - 1) : (127 - center);
			calcb = (velocity - center) << 16;
			if ( !calc5 )
				__builtin_trap();
			if ( calc5 == -1 && (unsigned int)calcb == 0x80000000 )
				__builtin_trap();
			calc5 = calcb / calc5;
			break;
		case 7u:
			if ( velocity == center )
				break;
			calc5 = (center >= velocity) ? (center - 1) : (127 - center);
			calcb = (center - velocity) << 16;
			if ( !calc5 )
				__builtin_trap();
			if ( calc5 == -1 && (unsigned int)calcb == 0x80000000 )
				__builtin_trap();
			calc5 = calcb / calc5;
			break;
		case 8u:
			if ( velocity == center )
				break;
			calcb = (velocity - center) << 15;
			if ( center >= velocity )
			{
				if ( center == 1 )
					__builtin_trap();
				if ( !center && (unsigned int)calcb == 0x80000000 )
					__builtin_trap();
				calc5 = calcb / (center - 1) * (calcb / (center - 1)) / -16384;
			}
			else
			{
				calc5 = 127 - center;
				if ( 127 == center )
					__builtin_trap();
				if ( calc5 == -1 && (unsigned int)calcb == 0x80000000 )
					__builtin_trap();
				calc5 = calcb / calc5 * (calcb / calc5);
				calc5 = (calc5 < 0) ? ((calc5 + 0x3FFF) >> 14) : (calc5 >> 14);
			}
			break;
		case 9u:
			if ( velocity == center )
				break;
			calcb = (velocity - center) << 15;
			if ( center >= velocity )
			{
				if ( center == 1 )
					__builtin_trap();
				if ( !center && (unsigned int)calcb == 0x80000000 )
					__builtin_trap();
				calc5 = (calcb / (center - 1) + 0x8000) * (calcb / (center - 1) + 0x8000) / 0x4000 - 0x10000;
			}
			else
			{
				calc5 = 127 - center;
				if ( 127 == center )
					__builtin_trap();
				if ( calc5 == -1 && (unsigned int)calcb == 0x80000000 )
					__builtin_trap();
				calc5 = (0x8000 - calcb / calc5) * (0x8000 - calcb / calc5);
				calc5 = 0x10000 - ((calc5 < 0) ? ((calc5 + 0x3FFF) >> 14) : (calc5 >> 14));
			}
			break;
	}
	return (calc5 < -65536) ? -65536 : ((calc5 > 0xFFFF) ? 0xFFFF : calc5);
}

int sceSdHdGetValidProgramNumberCount(void *buffer)
{
	int validcnt;
	int result;
	unsigned int i;
	struct sdhd_info dinfo;

	validcnt = 0;
	result = (int)do_get_vers_head_chunk((sceHardSynthVersionChunk *)buffer, &dinfo);
	if ( result )
		return result;
	result = (int)do_get_prog_chunk(buffer, &dinfo);
	if ( result )
		return result;
	for ( i = 0; dinfo.m_prog->maxProgramNumber >= i; i += 1 )
	{
		if ( dinfo.m_prog->programOffsetAddr[i] != 0xFFFFFFFF )
			validcnt += 1;
	}
	return validcnt;
}

int sceSdHdGetValidProgramNumber(void *buffer, unsigned int *ptr)
{
	int validcnt;
	int result;
	unsigned int i;
	struct sdhd_info dinfo;

	validcnt = 0;
	result = (int)do_get_vers_head_chunk((sceHardSynthVersionChunk *)buffer, &dinfo);
	if ( result )
		return result;
	result = (int)do_get_prog_chunk(buffer, &dinfo);
	if ( result )
		return result;
	for ( i = 0; dinfo.m_prog->maxProgramNumber >= i; i += 1 )
	{
		if ( dinfo.m_prog->programOffsetAddr[i] != 0xFFFFFFFF )
		{
			ptr[validcnt] = i;
			validcnt += 1;
		}
	}
	return validcnt;
}

int sceSdHdGetSampleNumberBySampleIndex(void *buffer, unsigned int sampleSetNumber, unsigned int sampleIndexNumber)
{
	int result;
	sceHardSynthSampleSetParam *p_samplesetparam;

	result = sceSdHdGetSampleSetParamAddr(buffer, sampleSetNumber, &p_samplesetparam);
	if ( result )
		return result;
	return ((unsigned int)p_samplesetparam->nSample - 1 < sampleIndexNumber) ?
					 0x9006 :
					 p_samplesetparam->sampleIndex[sampleIndexNumber];
}

#ifdef _IOP
int _start(int ac, char *av[], void *startaddr, ModuleInfo_t *mi)
{
	int regres;
	int state;

	(void)av;
	(void)startaddr;
	if ( ac < 0 )
	{
		CpuSuspendIntr(&state);
		regres = ReleaseLibraryEntries(&_exp_sdhd);
		CpuResumeIntr(state);
		return (!regres) ? MODULE_NO_RESIDENT_END : MODULE_REMOVABLE_END;
	}
	CpuSuspendIntr(&state);
	regres = RegisterLibraryEntries(&_exp_sdhd);
	CpuResumeIntr(state);
	if ( regres )
		return MODULE_NO_RESIDENT_END;
#if 0
  return MODULE_REMOVABLE_END;
#else
	if ( mi && ((mi->newflags & 2) != 0) )
		mi->newflags |= 0x10;
	return MODULE_RESIDENT_END;
#endif
}
#endif

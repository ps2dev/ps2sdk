/*
# _____     ___ ____     ___ ____
#  ____|   |    ____|   |        | |____|
# |     ___|   |____ ___|    ____| |    \    PS2DEV Open Source Project.
#-----------------------------------------------------------------------
# Copyright ps2dev - http://www.ps2dev.org
# Licenced under Academic Free License version 2.0
# Review ps2sdk README & LICENSE files for further details.
*/

#ifndef _SDHD_H
#define _SDHD_H

#ifdef _IOP
#include <tamtypes.h>
#endif

typedef struct sceHardSynthSplitBlock_
{
	u16 sampleSetIndex;
	u8 splitRangeLow;
	u8 splitCrossFade;
	u8 splitRangeHigh;
	u8 splitNumber;
	u16 splitBendRangeLow;
	u16 splitBendRangeHigh;
	char keyFollowPitch;
	u8 keyFollowPitchCenter;
	char keyFollowAmp;
	u8 keyFollowAmpCenter;
	char keyFollowPan;
	u8 keyFollowPanCenter;
	u8 splitVolume;
	char splitPanpot;
	char splitTranspose;
	char splitDetune;
} sceHardSynthSplitBlock;

typedef struct sceHardSynthProgramParam_
{
	unsigned int splitBlockAddr;
	u8 nSplit;
	u8 sizeSplitBlock;
	u8 progVolume;
	char progPanpot;
	char progTranspose;
	char progDetune;
	char keyFollowPan;
	u8 keyFollowPanCenter;
	u8 progAttr;
	u8 dmy;
	u8 progLfoWave;
	u8 progLfoWave2;
	u8 progLfoStartPhase;
	u8 progLfoStartPhase2;
	u8 progLfoPhaseRandom;
	u8 progLfoPhaseRandom2;
	u16 progLfoFreq;
	u16 progLfoFreq2;
	s16 progLfoPitchDepth;
	s16 progLfoPitchDepth2;
	s16 progLfoMidiPitchDepth;
	s16 progLfoMidiPitchDepth2;
	char progLfoAmpDepth;
	char progLfoAmpDepth2;
	char progLfoMidiAmpDepth;
	char progLfoMidiAmpDepth2;
	sceHardSynthSplitBlock splitBlock[];
} sceHardSynthProgramParam;

struct SceSdHdProgramCommon_
{
	unsigned int volume;
	int panpot;
	int transpose;
	int detune;
};

typedef struct SceSdHdProgramCommon_ SceSdHdProgramCommon;
typedef struct SceSdHdProgramCommon_ SceSdHdSplitCommon;

typedef struct SceSdHdProgramKeyFollow_
{
	int pan;
	unsigned int panCenter;
} SceSdHdProgramKeyFollow;

typedef struct SceSdHdProgramLFO_
{
	unsigned int wavePitch;
	unsigned int waveAmp;
	unsigned int startPhasePitch;
	unsigned int startPhaseAmp;
	unsigned int phaseRandomPitch;
	unsigned int phaseRandomAmp;
	unsigned int cyclePitch;
	unsigned int cycleAmp;
	int pitchDepthUp;
	int pitchDepthDown;
	int midiPitchDepthUp;
	int midiPitchDepthDown;
	int ampDepthUp;
	int ampDepthDown;
	int midiAmpDepthUp;
	int midiAmpDepthDown;
} SceSdHdProgramLFO;

typedef struct SceSdHdProgramParam_
{
	unsigned int nSplit;
	unsigned int progAttr;
	SceSdHdProgramCommon common;
	SceSdHdProgramKeyFollow keyFollow;
	SceSdHdProgramLFO LFO;
} SceSdHdProgramParam;

struct SceSdHdSplitRange_
{
	unsigned int low;
	unsigned int crossFade;
	unsigned int high;
};

typedef struct SceSdHdSplitRange_ SceSdHdSplitRange;
typedef struct SceSdHdSplitRange_ SceSdHdSampleVelRange;

typedef struct SceSdHdSplitBendRange_
{
	unsigned int low;
	unsigned int high;
} SceSdHdSplitBendRange;

typedef struct SceSdHdSplitKeyFollow_
{
	int pitch;
	unsigned int pitchCenter;
	int amp;
	unsigned int ampCenter;
	int pan;
	unsigned int panCenter;
} SceSdHdSplitKeyFollow;

typedef struct SceSdHdSplitBlock_
{
	unsigned int sampleSetIndex;
	unsigned int splitNumber;
	SceSdHdSplitRange range;
	SceSdHdSplitBendRange bendRange;
	SceSdHdSplitKeyFollow keyFollow;
	SceSdHdSplitCommon common;
} SceSdHdSplitBlock;

typedef struct sceHardSynthSampleSetParam_
{
	u8 velCurve;
	u8 velLimitLow;
	u8 velLimitHigh;
	u8 nSample;
	u16 sampleIndex[];
} sceHardSynthSampleSetParam;

typedef struct SceSdHdSampleSetParam_
{
	unsigned int velCurve;
	unsigned int velLimitLow;
	unsigned int velLimitHigh;
	unsigned int nSample;
} SceSdHdSampleSetParam;

typedef struct sceHardSynthSampleParam_
{
	u16 VagIndex;
	u8 velRangeLow;
	u8 velCrossFade;
	u8 velRangeHigh;
	char velFollowPitch;
	u8 velFollowPitchCenter;
	u8 velFollowPitchVelCurve;
	char velFollowAmp;
	u8 velFollowAmpCenter;
	u8 velFollowAmpVelCurve;
	u8 sampleBaseNote;
	char sampleDetune;
	char samplePanpot;
	u8 sampleGroup;
	u8 samplePriority;
	u8 sampleVolume;
	u8 dmy;
	u16 sampleAdsr1;
	u16 sampleAdsr2;
	char keyFollowAr;
	u8 keyFollowArCenter;
	char keyFollowDr;
	u8 keyFollowDrCenter;
	char keyFollowSr;
	u8 keyFollowSrCenter;
	char keyFollowRr;
	u8 keyFollowRrCenter;
	char keyFollowSl;
	u8 keyFollowSlCenter;
	u16 samplePitchLfoDelay;
	u16 samplePitchLfoFade;
	u16 sampleAmpLfoDelay;
	u16 sampleAmpLfoFade;
	u8 sampleLfoAttr;
	u8 sampleSpuAttr;
} sceHardSynthSampleParam;

typedef struct SceSdHdSampleVelFollow_
{
	int pitch;
	unsigned int pitchCenter;
	unsigned int pitchVelCurve;
	int amp;
	unsigned int ampCenter;
	unsigned int ampVelCurve;
} SceSdHdSampleVelFollow;

typedef struct SceSdHdSampleCommon_
{
	unsigned int baseNote;
	int detune;
	int panpot;
	unsigned int group;
	unsigned int priority;
	unsigned int volume;
} SceSdHdSampleCommon;

typedef struct SceSdHdSampleADSR_
{
	unsigned int ADSR1;
	unsigned int ADSR2;
} SceSdHdSampleADSR;

typedef struct SceSdHdSampleKeyFollow_
{
	int ar;
	unsigned int arCenter;
	int dr;
	unsigned int drCenter;
	int sr;
	unsigned int srCenter;
	int rr;
	unsigned int rrCenter;
	int sl;
	unsigned int slCenter;
} SceSdHdSampleKeyFollow;

typedef struct SceSdHdSampleLFO_
{
	unsigned int pitchLFODelay;
	unsigned int pitchLFOFade;
	unsigned int ampLFODelay;
	unsigned int ampLFOFade;
} SceSdHdSampleLFO;

typedef struct SceSdHdSampleParam_
{
	int vagIndex;
	unsigned int spuAttr;
	unsigned int lfoAttr;
	SceSdHdSampleVelRange velRange;
	SceSdHdSampleVelFollow velFollow;
	SceSdHdSampleCommon common;
	SceSdHdSampleADSR ADSR;
	SceSdHdSampleKeyFollow keyFollow;
	SceSdHdSampleLFO LFO;
} SceSdHdSampleParam;

typedef struct sceHardSynthVagParam_
{
	unsigned int vagOffsetAddr;
	u16 vagSampleRate;
	u8 vagAttribute;
	u8 dmy;
} sceHardSynthVagParam;

typedef struct SceSdHdVAGInfoParam_
{
	unsigned int vagOffsetAddr;
	unsigned int vagSize;
	unsigned int vagSampleRate;
	unsigned int vagAttribute;
} SceSdHdVAGInfoParam;

extern int sceSdHdGetMaxProgramNumber(void *buffer);
extern int sceSdHdGetMaxSampleSetNumber(void *buffer);
extern int sceSdHdGetMaxSampleNumber(void *buffer);
extern int sceSdHdGetMaxVAGInfoNumber(void *buffer);
extern int sceSdHdGetProgramParamAddr(void *buffer, unsigned int programNumber, sceHardSynthProgramParam **ptr);
extern int sceSdHdGetProgramParam(void *buffer, unsigned int programNumber, SceSdHdProgramParam *param);
extern int sceSdHdGetSplitBlockAddr(
	void *buffer, unsigned int programNumber, unsigned int splitBlockNumber, sceHardSynthSplitBlock **theParamPtr);
extern int
sceSdHdGetSplitBlock(void *buffer, unsigned int programNumber, unsigned int splitBlockNumber, SceSdHdSplitBlock *param);
extern int sceSdHdGetSampleSetParamAddr(void *buffer, unsigned int sampleSetNumber, sceHardSynthSampleSetParam **ptr);
extern int sceSdHdGetSampleSetParam(void *buffer, unsigned int sampleSetNumber, SceSdHdSampleSetParam *param);
extern int sceSdHdGetSampleParamAddr(void *buffer, unsigned int sampleNumber, sceHardSynthSampleParam **ptr);
extern int sceSdHdGetSampleParam(void *buffer, unsigned int sampleNumber, SceSdHdSampleParam *param);
extern int sceSdHdGetVAGInfoParamAddr(void *buffer, unsigned int vagInfoNumber, sceHardSynthVagParam **ptr);
extern int sceSdHdGetVAGInfoParam(void *buffer, unsigned int vagInfoNumber, SceSdHdVAGInfoParam *param);
extern int sceSdHdCheckProgramNumber(void *buffer, unsigned int programNumber);
extern int sceSdHdGetSplitBlockCountByNote(void *buffer, unsigned int programNumber, unsigned int noteNumber);
extern int sceSdHdGetSplitBlockAddrByNote(
	void *buffer, unsigned int programNumber, unsigned int noteNumber, sceHardSynthSplitBlock **ptr);
extern int
sceSdHdGetSplitBlockByNote(void *buffer, unsigned int programNumber, unsigned int noteNumber, SceSdHdSplitBlock *param);
extern int sceSdHdGetSampleSetParamCountByNote(void *buffer, unsigned int programNumber, unsigned int noteNumber);
extern int sceSdHdGetSampleSetParamAddrByNote(
	void *buffer, unsigned int programNumber, unsigned int noteNumber, sceHardSynthSampleSetParam **ptr);
extern int sceSdHdGetSampleSetParamByNote(
	void *buffer, unsigned int programNumber, unsigned int noteNumber, SceSdHdSampleSetParam *param);
extern int sceSdHdGetSampleParamCountByNoteVelocity(
	void *buffer, unsigned int programNumber, unsigned int noteNumber, unsigned int velocity, unsigned int mode);
extern int sceSdHdGetSampleParamAddrByNoteVelocity(
	void *buffer,
	unsigned int programNumber,
	unsigned int noteNumber,
	unsigned int velocity,
	unsigned int mode,
	sceHardSynthSampleParam **ptr);
extern int sceSdHdGetSampleParamByNoteVelocity(
	void *buffer,
	unsigned int programNumber,
	unsigned int noteNumber,
	unsigned int velocity,
	unsigned int mode,
	SceSdHdSampleParam *param);
extern int sceSdHdGetVAGInfoParamCountByNoteVelocity(
	void *buffer, unsigned int programNumber, unsigned int noteNumber, unsigned int velocity, unsigned int mode);
extern int sceSdHdGetVAGInfoParamAddrByNoteVelocity(
	void *buffer,
	unsigned int programNumber,
	unsigned int noteNumber,
	unsigned int velocity,
	unsigned int mode,
	sceHardSynthVagParam **ptr);
extern int sceSdHdGetVAGInfoParamByNoteVelocity(
	void *buffer,
	unsigned int programNumber,
	unsigned int noteNumber,
	unsigned int velocity,
	unsigned int mode,
	SceSdHdVAGInfoParam *param);
extern int sceSdHdGetSampleParamCountByVelocity(
	void *buffer, unsigned int sampleSetNumber, unsigned int velocity, unsigned int mode);
extern int sceSdHdGetSampleParamAddrByVelocity(
	void *buffer, unsigned int sampleSetNumber, unsigned int velocity, unsigned int mode, sceHardSynthSampleParam **ptr);
extern int sceSdHdGetSampleParamByVelocity(
	void *buffer, unsigned int sampleSetNumber, unsigned int velocity, unsigned int mode, SceSdHdSampleParam *param);
extern int sceSdHdGetVAGInfoParamCountByVelocity(
	void *buffer, unsigned int sampleSetNumber, unsigned int velocity, unsigned int mode);
extern int sceSdHdGetVAGInfoParamAddrByVelocity(
	void *buffer, unsigned int sampleSetNumber, unsigned int velocity, unsigned int mode, sceHardSynthVagParam **ptr);
extern int sceSdHdGetVAGInfoParamByVelocity(
	void *buffer, unsigned int sampleSetNumber, unsigned int velocity, unsigned int mode, SceSdHdVAGInfoParam *param);
extern int
sceSdHdGetVAGInfoParamAddrBySampleNumber(void *buffer, unsigned int sampleNumber, sceHardSynthVagParam **ptr);
extern int sceSdHdGetVAGInfoParamBySampleNumber(void *buffer, unsigned int sampleNumber, SceSdHdVAGInfoParam *param);
extern int sceSdHdGetSplitBlockNumberBySplitNumber(void *buffer, unsigned int programNumber, unsigned int splitNumber);
extern int sceSdHdGetVAGSize(void *buffer, unsigned int vagInfoNumber);
extern int sceSdHdGetSplitBlockCount(void *buffer, unsigned int programNumber);
extern int sceSdHdGetMaxSplitBlockCount(void *buffer);
extern int sceSdHdGetMaxSampleSetParamCount(void *buffer);
extern int sceSdHdGetMaxSampleParamCount(void *buffer);
extern int sceSdHdGetMaxVAGInfoParamCount(void *buffer);
extern int sceSdHdModifyVelocity(unsigned int curveType, int velocity);
extern int sceSdHdModifyVelocityLFO(unsigned int curveType, int velocity, int center);
extern int sceSdHdGetValidProgramNumberCount(void *buffer);
extern int sceSdHdGetValidProgramNumber(void *buffer, unsigned int *ptr);
extern int
sceSdHdGetSampleNumberBySampleIndex(void *buffer, unsigned int sampleSetNumber, unsigned int sampleIndexNumber);

#define sdhd_IMPORTS_start DECLARE_IMPORT_TABLE(sdhd, 1, 2)
#define sdhd_IMPORTS_end END_IMPORT_TABLE

#define I_sceSdHdGetMaxProgramNumber DECLARE_IMPORT(4, sceSdHdGetMaxProgramNumber)
#define I_sceSdHdGetMaxSampleSetNumber DECLARE_IMPORT(5, sceSdHdGetMaxSampleSetNumber)
#define I_sceSdHdGetMaxSampleNumber DECLARE_IMPORT(6, sceSdHdGetMaxSampleNumber)
#define I_sceSdHdGetMaxVAGInfoNumber DECLARE_IMPORT(7, sceSdHdGetMaxVAGInfoNumber)
#define I_sceSdHdGetProgramParamAddr DECLARE_IMPORT(8, sceSdHdGetProgramParamAddr)
#define I_sceSdHdGetProgramParam DECLARE_IMPORT(9, sceSdHdGetProgramParam)
#define I_sceSdHdGetSplitBlockAddr DECLARE_IMPORT(10, sceSdHdGetSplitBlockAddr)
#define I_sceSdHdGetSplitBlock DECLARE_IMPORT(11, sceSdHdGetSplitBlock)
#define I_sceSdHdGetSampleSetParamAddr DECLARE_IMPORT(12, sceSdHdGetSampleSetParamAddr)
#define I_sceSdHdGetSampleSetParam DECLARE_IMPORT(13, sceSdHdGetSampleSetParam)
#define I_sceSdHdGetSampleParamAddr DECLARE_IMPORT(14, sceSdHdGetSampleParamAddr)
#define I_sceSdHdGetSampleParam DECLARE_IMPORT(15, sceSdHdGetSampleParam)
#define I_sceSdHdGetVAGInfoParamAddr DECLARE_IMPORT(16, sceSdHdGetVAGInfoParamAddr)
#define I_sceSdHdGetVAGInfoParam DECLARE_IMPORT(17, sceSdHdGetVAGInfoParam)
#define I_sceSdHdCheckProgramNumber DECLARE_IMPORT(18, sceSdHdCheckProgramNumber)
#define I_sceSdHdGetSplitBlockCountByNote DECLARE_IMPORT(19, sceSdHdGetSplitBlockCountByNote)
#define I_sceSdHdGetSplitBlockAddrByNote DECLARE_IMPORT(20, sceSdHdGetSplitBlockAddrByNote)
#define I_sceSdHdGetSplitBlockByNote DECLARE_IMPORT(21, sceSdHdGetSplitBlockByNote)
#define I_sceSdHdGetSampleSetParamCountByNote DECLARE_IMPORT(22, sceSdHdGetSampleSetParamCountByNote)
#define I_sceSdHdGetSampleSetParamAddrByNote DECLARE_IMPORT(23, sceSdHdGetSampleSetParamAddrByNote)
#define I_sceSdHdGetSampleSetParamByNote DECLARE_IMPORT(24, sceSdHdGetSampleSetParamByNote)
#define I_sceSdHdGetSampleParamCountByNoteVelocity DECLARE_IMPORT(25, sceSdHdGetSampleParamCountByNoteVelocity)
#define I_sceSdHdGetSampleParamAddrByNoteVelocity DECLARE_IMPORT(26, sceSdHdGetSampleParamAddrByNoteVelocity)
#define I_sceSdHdGetSampleParamByNoteVelocity DECLARE_IMPORT(27, sceSdHdGetSampleParamByNoteVelocity)
#define I_sceSdHdGetVAGInfoParamCountByNoteVelocity DECLARE_IMPORT(28, sceSdHdGetVAGInfoParamCountByNoteVelocity)
#define I_sceSdHdGetVAGInfoParamAddrByNoteVelocity DECLARE_IMPORT(29, sceSdHdGetVAGInfoParamAddrByNoteVelocity)
#define I_sceSdHdGetVAGInfoParamByNoteVelocity DECLARE_IMPORT(30, sceSdHdGetVAGInfoParamByNoteVelocity)
#define I_sceSdHdGetSampleParamCountByVelocity DECLARE_IMPORT(31, sceSdHdGetSampleParamCountByVelocity)
#define I_sceSdHdGetSampleParamAddrByVelocity DECLARE_IMPORT(32, sceSdHdGetSampleParamAddrByVelocity)
#define I_sceSdHdGetSampleParamByVelocity DECLARE_IMPORT(33, sceSdHdGetSampleParamByVelocity)
#define I_sceSdHdGetVAGInfoParamCountByVelocity DECLARE_IMPORT(34, sceSdHdGetVAGInfoParamCountByVelocity)
#define I_sceSdHdGetVAGInfoParamAddrByVelocity DECLARE_IMPORT(35, sceSdHdGetVAGInfoParamAddrByVelocity)
#define I_sceSdHdGetVAGInfoParamByVelocity DECLARE_IMPORT(36, sceSdHdGetVAGInfoParamByVelocity)
#define I_sceSdHdGetVAGInfoParamAddrBySampleNumber DECLARE_IMPORT(37, sceSdHdGetVAGInfoParamAddrBySampleNumber)
#define I_sceSdHdGetVAGInfoParamBySampleNumber DECLARE_IMPORT(38, sceSdHdGetVAGInfoParamBySampleNumber)
#define I_sceSdHdGetSplitBlockNumberBySplitNumber DECLARE_IMPORT(39, sceSdHdGetSplitBlockNumberBySplitNumber)
#define I_sceSdHdGetVAGSize DECLARE_IMPORT(40, sceSdHdGetVAGSize)
#define I_sceSdHdGetSplitBlockCount DECLARE_IMPORT(41, sceSdHdGetSplitBlockCount)
#define I_sceSdHdGetMaxSplitBlockCount DECLARE_IMPORT(42, sceSdHdGetMaxSplitBlockCount)
#define I_sceSdHdGetMaxSampleSetParamCount DECLARE_IMPORT(43, sceSdHdGetMaxSampleSetParamCount)
#define I_sceSdHdGetMaxSampleParamCount DECLARE_IMPORT(44, sceSdHdGetMaxSampleParamCount)
#define I_sceSdHdGetMaxVAGInfoParamCount DECLARE_IMPORT(45, sceSdHdGetMaxVAGInfoParamCount)
#define I_sceSdHdModifyVelocity DECLARE_IMPORT(46, sceSdHdModifyVelocity)
#define I_sceSdHdModifyVelocityLFO DECLARE_IMPORT(47, sceSdHdModifyVelocityLFO)
#define I_sceSdHdGetValidProgramNumberCount DECLARE_IMPORT(48, sceSdHdGetValidProgramNumberCount)
#define I_sceSdHdGetValidProgramNumber DECLARE_IMPORT(49, sceSdHdGetValidProgramNumber)
#define I_sceSdHdGetSampleNumberBySampleIndex DECLARE_IMPORT(50, sceSdHdGetSampleNumberBySampleIndex)

#endif

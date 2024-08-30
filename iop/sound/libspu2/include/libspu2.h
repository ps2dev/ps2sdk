/*
# _____     ___ ____     ___ ____
#  ____|   |    ____|   |        | |____|
# |     ___|   |____ ___|    ____| |    \    PS2DEV Open Source Project.
#-----------------------------------------------------------------------
# Copyright ps2dev - http://www.ps2dev.org
# Licenced under Academic Free License version 2.0
# Review ps2sdk README & LICENSE files for further details.
*/

#ifndef _LIBSPU2_H
#define _LIBSPU2_H

#define SPU_SUCCESS 0
#define SPU_INVALID_ARGS (-3)
#define SPU_DIAG (-2)
#define SPU_CHECK (-1)
#define SPU_OFF 0
#define SPU_ON 1
#define SPU_CLEAR 2
#define SPU_RESET 3
#define SPU_DONT_CARE 4
#define SPU_ALL 0
#define SPU_CDONLY 5
#define SPU_VOICEONLY 6
#define SPU_CONT 7
#define SPU_BIT 8
#define SPU_NULL 0

#define SpuDiag SPU_DIAG
#define SpuCheck SPU_CHECK
#define SpuOff SPU_OFF
#define SpuOn SPU_ON
#define SpuClear SPU_CLEAR
#define SpuReset SPU_RESET
#define SpuDontCare SPU_DONT_CARE
#define SpuALL SPU_ALL
#define SpuCDOnly SPU_CDONLY
#define SpuVoiceOnly SPU_VOICEONLY
#define SpuCont SPU_CONT
#define SpuNull SPU_NULL

#define SPU_OFF_ENV_ON 2
#define SPU_ON_ENV_OFF 3

#define SpuOffEnvOn SPU_OFF_ENV_ON
#define SpuOnEnvOff SPU_ON_ENV_OFF

#define SPU_ERROR (-1)
#define SpuError SPU_ERROR

#define SPU_TRANSFER_BY_DMA 0
#define SPU_TRANSFER_BY_IO 1

#define SpuTransferByDMA SPU_TRANSFER_BY_DMA
#define SpuTransferByIO SPU_TRANSFER_BY_IO
#define SpuTransByDMA SPU_TRANSFER_BY_DMA
#define SpuTransByIO SPU_TRANSFER_BY_IO

#define SPU_TRANSFER_WAIT 1
#define SPU_TRANSFER_PEEK 0
#define SPU_TRANSFER_GLANCE SPU_TRANSFER_PEEK

#define SPU_AUTODMA_ONESHOT 0
#define SPU_AUTODMA_LOOP 1
#define SPU_AUTODMA_START_ADDR (1 << 1)
#define SPU_AUTODMA_BIT4 (1 << 4)
#define SPU_AUTODMA_BIT6 (1 << 6)
#define SPU_AUTODMA_BIT7 (1 << 6)

#define SPU_00CH ((u32)1 << 0)
#define SPU_01CH ((u32)1 << 1)
#define SPU_02CH ((u32)1 << 2)
#define SPU_03CH ((u32)1 << 3)
#define SPU_04CH ((u32)1 << 4)
#define SPU_05CH ((u32)1 << 5)
#define SPU_06CH ((u32)1 << 6)
#define SPU_07CH ((u32)1 << 7)
#define SPU_08CH ((u32)1 << 8)
#define SPU_09CH ((u32)1 << 9)
#define SPU_10CH ((u32)1 << 10)
#define SPU_11CH ((u32)1 << 11)
#define SPU_12CH ((u32)1 << 12)
#define SPU_13CH ((u32)1 << 13)
#define SPU_14CH ((u32)1 << 14)
#define SPU_15CH ((u32)1 << 15)
#define SPU_16CH ((u32)1 << 16)
#define SPU_17CH ((u32)1 << 17)
#define SPU_18CH ((u32)1 << 18)
#define SPU_19CH ((u32)1 << 19)
#define SPU_20CH ((u32)1 << 20)
#define SPU_21CH ((u32)1 << 21)
#define SPU_22CH ((u32)1 << 22)
#define SPU_23CH ((u32)1 << 23)

#define SPU_0CH SPU_00CH
#define SPU_1CH SPU_01CH
#define SPU_2CH SPU_02CH
#define SPU_3CH SPU_03CH
#define SPU_4CH SPU_04CH
#define SPU_5CH SPU_05CH
#define SPU_6CH SPU_06CH
#define SPU_7CH SPU_07CH
#define SPU_8CH SPU_08CH
#define SPU_9CH SPU_09CH

#define SPU_ALLCH                                                                                                      \
	(SPU_00CH | SPU_01CH | SPU_02CH | SPU_03CH | SPU_04CH | SPU_05CH | SPU_06CH | SPU_07CH | SPU_08CH | SPU_09CH         \
	 | SPU_10CH | SPU_11CH | SPU_12CH | SPU_13CH | SPU_14CH | SPU_15CH | SPU_16CH | SPU_17CH | SPU_18CH | SPU_19CH       \
	 | SPU_20CH | SPU_21CH | SPU_22CH | SPU_23CH | 0)

#define SPU_KEYCH(x) ((u32)1 << (x))
#define SPU_VOICECH(...) SPU_VOICECH(__VA_ARGS__)

#define SPU_VOICE_VOLL ((u32)1 << 0)
#define SPU_VOICE_VOLR ((u32)1 << 1)
#define SPU_VOICE_VOLMODEL ((u32)1 << 2)
#define SPU_VOICE_VOLMODER ((u32)1 << 3)
#define SPU_VOICE_PITCH ((u32)1 << 4)
#define SPU_VOICE_NOTE ((u32)1 << 5)
#define SPU_VOICE_SAMPLE_NOTE ((u32)1 << 6)
#define SPU_VOICE_WDSA ((u32)1 << 7)
#define SPU_VOICE_ADSR_AMODE ((u32)1 << 8)
#define SPU_VOICE_ADSR_SMODE ((u32)1 << 9)
#define SPU_VOICE_ADSR_RMODE ((u32)1 << 10)
#define SPU_VOICE_ADSR_AR ((u32)1 << 11)
#define SPU_VOICE_ADSR_DR ((u32)1 << 12)
#define SPU_VOICE_ADSR_SR ((u32)1 << 13)
#define SPU_VOICE_ADSR_RR ((u32)1 << 14)
#define SPU_VOICE_ADSR_SL ((u32)1 << 15)
#define SPU_VOICE_LSAX ((u32)1 << 16)
#define SPU_VOICE_ADSR_ADSR1 ((u32)1 << 17)
#define SPU_VOICE_ADSR_ADSR2 ((u32)1 << 18)

enum libspu2_spu_voice_enum
{
	SPU_VOICE_DIRECT = 0,
	SPU_VOICE_LINEARIncN,
	SPU_VOICE_LINEARIncR,
	SPU_VOICE_LINEARDecN,
	SPU_VOICE_LINEARDecR,
	SPU_VOICE_EXPIncN,
	SPU_VOICE_EXPIncR,
	SPU_VOICE_EXPDec,
	SPU_VOICE_EXPDecN = 7,
	SPU_VOICE_EXPDecR = 7,
};

#define SPU_DECODED_FIRSTHALF 0
#define SPU_DECODED_SECONDHALF 1
#define SPU_DECODE_FIRSTHALF SPU_DECODED_FIRSTHALF
#define SPU_DECODE_SECONDHALF SPU_DECODED_SECONDHALF

#define SPU_COMMON_MVOLL (1 << 0)
#define SPU_COMMON_MVOLR (1 << 1)
#define SPU_COMMON_MVOLMODEL (1 << 2)
#define SPU_COMMON_MVOLMODER (1 << 3)
#define SPU_COMMON_RVOLL (1 << 4)
#define SPU_COMMON_RVOLR (1 << 5)
#define SPU_COMMON_CDVOLL (1 << 6)
#define SPU_COMMON_CDVOLR (1 << 7)
#define SPU_COMMON_CDREV (1 << 8)
#define SPU_COMMON_CDMIX (1 << 9)
#define SPU_COMMON_EXTVOLL (1 << 10)
#define SPU_COMMON_EXTVOLR (1 << 11)
#define SPU_COMMON_EXTREV (1 << 12)
#define SPU_COMMON_EXTMIX (1 << 13)

#define SPU_REV_MODE (1 << 0)
#define SPU_REV_DEPTHL (1 << 1)
#define SPU_REV_DEPTHR (1 << 2)
#define SPU_REV_DELAYTIME (1 << 3)
#define SPU_REV_FEEDBACK (1 << 4)

enum libspu2_spu_rev_type_enum
{
	SPU_REV_MODE_CHECK = -1,
	SPU_REV_MODE_OFF = 0,
	SPU_REV_MODE_ROOM,
	SPU_REV_MODE_STUDIO_A,
	SPU_REV_MODE_STUDIO_B,
	SPU_REV_MODE_STUDIO_C,
	SPU_REV_MODE_HALL,
	SPU_REV_MODE_SPACE,
	SPU_REV_MODE_ECHO,
	SPU_REV_MODE_DELAY,
	SPU_REV_MODE_PIPE,
	SPU_REV_MODE_MAX,
};

#define SPU_REV_MODE_CLEAR_WA 0x100

#define SPU_EVENT_KEY (1 << 0)
#define SPU_EVENT_PITCHLFO (1 << 1)
#define SPU_EVENT_NOISE (1 << 2)
#define SPU_EVENT_REVERB (1 << 3)

enum libspu2_spu_spdif_out_enum
{
	SPU_SPDIF_OUT_OFF = 0,
	SPU_SPDIF_OUT_PCM,
	SPU_SPDIF_OUT_BITSTREAM,
	SPU_SPDIF_OUT_BYPASS,
};

#define SPU_SPDIF_COPY_NORMAL 0x00
#define SPU_SPDIF_COPY_PROHIBIT 0x80

#define SPU_SPDIF_MEDIA_CD 0x000
#define SPU_SPDIF_MEDIA_DVD 0x800

#define SPU_DECODEDDATA_SIZE 0x200
#define SPU_DECODEDATA_SIZE SPU_DECODEDDATA_SIZE

// sizeof(libspu2_malloc_t)
#define SPU_MALLOC_RECSIZ 8

#define SPU_ENV_EVENT_QUEUEING (1 << 0)

#define SPU_ST_NOT_AVAILABLE 0
#define SPU_ST_ACCEPT 1
#define SPU_ST_ERROR (-1)
#define SPU_ST_INVALID_ARGUMENT (-2)
#define SPU_ST_WRONG_STATUS (-3)
#define SPU_ST_STOP 2
#define SPU_ST_IDLE 3
#define SPU_ST_PREPARE 4
#define SPU_ST_START 5
#define SPU_ST_PLAY 6
#define SPU_ST_TRANSFER 7
#define SPU_ST_FINAL 8

#define SPU_ST_VAG_HEADER_SIZE 0x30

typedef struct SpuVolume_
{
	s16 left;
	s16 right;
} SpuVolume;

typedef struct SpuVoiceAttr_
{
	unsigned int voice;
	unsigned int mask;
	SpuVolume volume;
	SpuVolume volmode;
	SpuVolume volumex;
	u16 pitch;
	u16 note;
	u16 sample_note;
	s16 envx;
	unsigned int addr;
	unsigned int loop_addr;
	int a_mode;
	int s_mode;
	int r_mode;
	u16 ar;
	u16 dr;
	u16 sr;
	u16 rr;
	u16 sl;
	u16 adsr1;
	u16 adsr2;
} SpuVoiceAttr;

typedef struct SpuLVoiceAttr_
{
	s16 voiceNum;
	s16 pad;
	SpuVoiceAttr attr;
} SpuLVoiceAttr;

typedef struct SpuReverbAttr_
{
	unsigned int mask;
	int mode;
	SpuVolume depth;
	int delay;
	int feedback;
} SpuReverbAttr;

typedef struct SpuDecodedData_
{
	s16 cd_left[SPU_DECODEDDATA_SIZE];
	s16 cd_right[SPU_DECODEDDATA_SIZE];
	s16 voice1[SPU_DECODEDDATA_SIZE];
	s16 voice3[SPU_DECODEDDATA_SIZE];
} SpuDecodedData;

typedef struct SpuExtAttr_
{
	SpuVolume volume;
	int reverb;
	int mix;
} SpuExtAttr;

typedef struct SpuCommonAttr_
{
	unsigned int mask;
	SpuVolume mvol;
	SpuVolume mvolmode;
	SpuVolume mvolx;
	SpuExtAttr cd;
	SpuExtAttr ext;
} SpuCommonAttr;

typedef void (*SpuIRQCallbackProc)(void);
typedef void (*SpuTransferCallbackProc)(void);

typedef struct SpuEnv_
{
	unsigned int mask;
	unsigned int queueing;
} SpuEnv;

typedef struct SpuStVoiceAttr_
{
	char status;
	char pad1;
	char pad2;
	char pad3;
	int last_size;
	unsigned int buf_addr;
	unsigned int data_addr;
} SpuStVoiceAttr;

typedef struct SpuStEnv_
{
	int size;
	int low_priority;
	SpuStVoiceAttr voice[24];
} SpuStEnv;

typedef void (*SpuStCallbackProc)(unsigned int voice_bit, int status);

extern void SpuSetDigitalOut(int mode);
#define SpuSetDegitalOut(...) SpuSetDigitalOut(__VA_ARGS__)
extern void SpuStart(void);
extern void SpuInit(void);
extern unsigned int SpuSetCore(unsigned int which_core);
extern unsigned int SpuGetCore(void);
extern void SpuSetReverbEndAddr(unsigned int eea);
extern unsigned int SpuGetReverbEndAddr(void);
extern void SpuInitHot(void);
extern void SpuQuit(void);
extern int SpuSetMute(int on_off);
extern int SpuGetMute(void);
extern int SpuInitMalloc(int num, char *top);
extern int SpuMalloc(int size);
extern int SpuMallocWithStartAddr(unsigned int addr, int size);
extern void SpuFree(unsigned int addr);
extern void SpuSetEnv(const SpuEnv *env);
extern unsigned int SpuFlush(unsigned int ev);
extern unsigned int SpuSetNoiseVoice(int on_off, unsigned int voice_bit);
extern unsigned int SpuGetNoiseVoice(void);
extern int SpuSetNoiseClock(int n_clock);
extern int SpuGetNoiseClock(void);
extern int SpuSetReverb(int on_off);
extern int SpuGetReverb(void);
extern int SpuSetReverbModeParam(SpuReverbAttr *attr);
extern void SpuGetReverbModeParam(SpuReverbAttr *attr);
extern int SpuReserveReverbWorkArea(int on_off);
extern int SpuIsReverbWorkAreaReserved(int on_off);
extern int SpuSetReverbDepth(SpuReverbAttr *attr);
extern unsigned int SpuSetReverbVoice(int on_off, unsigned int voice_bit);
extern unsigned int SpuGetReverbVoice(void);
extern int SpuClearReverbWorkArea(int mode);
extern int SpuReadDecodedData(SpuDecodedData *d_data, int flag);
extern int SpuSetIRQ(int on_off);
extern int SpuGetIRQ(void);
extern unsigned int SpuSetIRQAddr(unsigned int addr);
extern unsigned int SpuGetIRQAddr(void);
extern unsigned int SpuGetNextAddr(int v_num);
extern SpuIRQCallbackProc SpuSetIRQCallback(SpuIRQCallbackProc func);
extern void SpuSetKey(int on_off, unsigned int voice_bit);
extern int SpuGetKeyStatus(unsigned int voice_bit);
extern void SpuSetKeyOnWithAttr(SpuVoiceAttr *attr);
extern void SpuGetVoiceEnvelopeAttr(int v_num, int *key_stat, s16 *envx);
extern unsigned int SpuRead(u8 *addr, unsigned int size);
extern unsigned int SpuWrite(u8 *addr, unsigned int size);
extern unsigned int SpuAutoDMAWrite(u8 *addr, unsigned int size, unsigned int mode, ...);
extern int SpuAutoDMAStop(void);
extern int SpuAutoDMAGetStatus(void);
extern unsigned int SpuWrite0(unsigned int size);
extern unsigned int SpuSetTransferStartAddr(unsigned int addr);
extern unsigned int SpuGetTransferStartAddr(void);
extern int SpuSetTransferMode(int mode);
extern int SpuGetTransferMode(void);
extern unsigned int SpuWritePartly(u8 *addr, unsigned int size);
extern int SpuIsTransferCompleted(int flag);
extern SpuTransferCallbackProc SpuSetTransferCallback(SpuTransferCallbackProc func);
extern SpuTransferCallbackProc SpuAutoDMASetCallback(SpuTransferCallbackProc func);
extern unsigned int SpuSetPitchLFOVoice(int on_off, unsigned int voice_bit);
extern unsigned int SpuGetPitchLFOVoice(void);
extern void SpuSetCommonAttr(SpuCommonAttr *attr);
extern void SpuGetCommonAttr(SpuCommonAttr *attr);
extern int SpuRGetAllKeysStatus(int min_, int max_, char *status);
extern void SpuGetAllKeysStatus(char *status);
extern int SpuStTransfer(int flag, unsigned int voice_bit);
extern SpuStEnv *SpuStInit(int mode);
extern int SpuStQuit(void);
extern int SpuStGetStatus(void);
extern unsigned int SpuStGetVoiceStatus(void);
extern SpuStCallbackProc SpuStSetPreparationFinishedCallback(SpuStCallbackProc func);
extern SpuStCallbackProc SpuStSetTransferFinishedCallback(SpuStCallbackProc func);
extern SpuStCallbackProc SpuStSetStreamFinishedCallback(SpuStCallbackProc func);
extern unsigned int SpuStSetCore(unsigned int core);
extern void SpuSetVoiceVolume(int v_num, s16 voll, s16 volr);
extern void SpuSetVoiceVolumeAttr(int v_num, s16 voll, s16 volr, s16 voll_mode, s16 volr_mode);
extern void SpuSetVoicePitch(int v_num, u16 pitch);
extern void SpuSetVoiceNote(int v_num, u16 note);
extern void SpuSetVoiceSampleNote(int v_num, u16 sample_note);
extern void SpuSetVoiceStartAddr(int v_num, unsigned int start_addr);
extern void SpuSetVoiceLoopStartAddr(int v_num, unsigned int lsa);
extern void SpuSetVoiceAR(int v_num, u16 ar);
extern void SpuSetVoiceDR(int v_num, u16 dr);
extern void SpuSetVoiceSR(int v_num, u16 sr);
extern void SpuSetVoiceRR(int v_num, u16 rr);
extern void SpuSetVoiceSL(int v_num, u16 sl);
extern void SpuSetVoiceARAttr(int v_num, u16 ar, int ar_mode);
extern void SpuSetVoiceSRAttr(int v_num, u16 sr, int sr_mode);
extern void SpuSetVoiceRRAttr(int v_num, u16 rr, int rr_mode);
extern void SpuSetVoiceADSR(int v_num, u16 ar, u16 dr, u16 sr, u16 rr, u16 sl);
extern void
SpuSetVoiceADSRAttr(int v_num, u16 ar, u16 dr, u16 sr, u16 rr, u16 sl, int ar_mode, int sr_mode, int rr_mode);
extern void SpuSetVoiceAttr(SpuVoiceAttr *arg);
extern int SpuRSetVoiceAttr(int min_, int max_, SpuVoiceAttr *arg);
extern void SpuNSetVoiceAttr(int v_num, SpuVoiceAttr *arg);
extern void SpuLSetVoiceAttr(int num, SpuLVoiceAttr *arg_list);
extern void SpuGetVoiceVolume(int v_num, s16 *voll, s16 *volr);
extern void SpuGetVoiceVolumeAttr(int v_num, s16 *voll, s16 *volr, s16 *voll_mode, s16 *volr_mode);
extern void SpuGetVoiceVolumeX(int v_num, s16 *voll_x, s16 *volr_x);
extern void SpuGetVoicePitch(int v_num, u16 *pitch);
extern void SpuGetVoiceNote(int v_num, u16 *note);
extern void SpuGetVoiceSampleNote(int v_num, u16 *sample_note);
extern void SpuGetVoiceEnvelope(int v_num, s16 *envx);
extern void SpuGetVoiceStartAddr(int v_num, unsigned int *start_addr);
extern void SpuGetVoiceLoopStartAddr(int v_num, unsigned int *loop_start_addr);
extern void SpuGetVoiceAR(int v_num, u16 *ar);
extern void SpuGetVoiceDR(int v_num, u16 *dr);
extern void SpuGetVoiceSR(int v_num, u16 *sr);
extern void SpuGetVoiceRR(int v_num, u16 *rr);
extern void SpuGetVoiceSL(int v_num, u16 *sl);
extern void SpuGetVoiceARAttr(int v_num, u16 *ar, int *ar_mode);
extern void SpuGetVoiceSRAttr(int v_num, u16 *sr, int *sr_mode);
extern void SpuGetVoiceRRAttr(int v_num, u16 *rr, int *rr_mode);
extern void SpuGetVoiceADSR(int v_num, u16 *ar, u16 *dr, u16 *sr, u16 *rr, u16 *sl);
extern void
SpuGetVoiceADSRAttr(int v_num, u16 *ar, u16 *dr, u16 *sr, u16 *rr, u16 *sl, int *ar_mode, int *sr_mode, int *rr_mode);
extern void SpuSetCommonMasterVolume(s16 mvol_left, s16 mvol_right);
extern void SpuSetCommonMasterVolumeAttr(s16 mvol_left, s16 mvol_right, s16 mvolmode_left, s16 mvolmode_right);
extern void SpuSetCommonCDVolume(s16 cd_left, s16 cd_right);
extern void SpuSetCommonCDReverb(int cd_reverb);
extern void SpuSetCommonCDMix(int cd_mix);
extern void SpuGetCommonMasterVolume(s16 *mvol_left, s16 *mvol_right);
extern void SpuGetCommonMasterVolumeX(s16 *mvolx_left, s16 *mvolx_right);
extern void SpuGetCommonMasterVolumeAttr(s16 *mvol_left, s16 *mvol_right, s16 *mvolmode_left, s16 *mvolmode_right);
extern void SpuGetCommonCDVolume(s16 *cd_left, s16 *cd_right);
extern void SpuGetCommonCDReverb(int *cd_reverb);
extern void SpuGetCommonCDMix(int *cd_mix);
extern int SpuSetReverbModeType(int mode);
extern void SpuSetReverbModeDepth(s16 depth_left, s16 depth_right);
extern void SpuSetReverbModeDelayTime(int delay);
extern void SpuSetReverbModeFeedback(int feedback);
extern void SpuGetReverbModeType(int *mode);
extern void SpuGetReverbModeDepth(s16 *depth_left, s16 *depth_right);
extern void SpuGetReverbModeDelayTime(int *delay);
extern void SpuGetReverbModeFeedback(int *feedback);
extern void SpuGetVoiceAttr(SpuVoiceAttr *arg);
extern void SpuNGetVoiceAttr(int v_num, SpuVoiceAttr *arg);
extern void SpuSetESA(int rev_addr);
extern void SpuSetAutoDMAAttr(s16 vol_l, s16 vol_r, s16 dry_on, s16 effect_on);
extern void SpuSetSerialInAttr(s16 dry_on, s16 effect_on);

#endif

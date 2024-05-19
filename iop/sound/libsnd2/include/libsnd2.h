/*
# _____     ___ ____     ___ ____
#  ____|   |    ____|   |        | |____|
# |     ___|   |____ ___|    ____| |    \    PS2DEV Open Source Project.
#-----------------------------------------------------------------------
# Copyright ps2dev - http://www.ps2dev.org
# Licenced under Academic Free License version 2.0
# Review ps2sdk README & LICENSE files for further details.
*/

#ifndef _LIBSND2_H
#define _LIBSND2_H

enum libsnd2_ss_tick_enum
{
	SS_NOTICK = 0x1000,
	SS_NOTICK0 = 0,
	SS_TICK60,
	SS_TICK240,
	SS_TICK120,
	SS_TICK50,
	SS_TICKVSYNC,
	SS_TICKMODE_MAX,
};

enum libsnd2_ss_play_enum
{
	SSPLAY_INFINITY = 0,
	SSPLAY_PAUSE = 0,
	SSPLAY_PLAY,
};

enum libsnd2_ss_s_enum
{
	SS_SOFF = 0,
	SS_SON,
};

enum libsnd2_ss_mix_enum
{
	SS_MIX = 0,
	SS_REV,
};

enum libsnd2_ss_serial_enum
{
	SS_SERIAL_A = 0,
	SS_SERIAL_B,
};

enum libsnd2_ss_mute_enum
{
	SS_MUTE_OFF = 0,
	SS_MUTE_ON,
};

enum libsnd2_wait_enum
{
	SS_IMEDIATE = 0,
	SS_IMMEDIATE = 0,
	SS_WAIT_COMPLETED,
};

enum libsnd2_ss_rev_type_enum
{
	SS_REV_TYPE_CHECK = -1,
	SS_REV_TYPE_OFF = 0,
	SS_REV_TYPE_ROOM,
	SS_REV_TYPE_STUDIO_A,
	SS_REV_TYPE_STUDIO_B,
	SS_REV_TYPE_STUDIO_C,
	SS_REV_TYPE_HALL,
	SS_REV_TYPE_SPACE,
	SS_REV_TYPE_ECHO,
	SS_REV_TYPE_DELAY,
	SS_REV_TYPE_PIPE,
	SS_REV_TYPE_MAX,
};

enum libsnd2_ss_skip_enum
{
	SSSKIP_TICK = 0,
	SSSKIP_NOTE4,
	SSSKIP_NOTE8,
	SSSKIP_BAR,
};

// sizeof(libsnd2_sequence_struct_t)
#define SS_SEQ_TABSIZ 0xb0

#define SND_VOLL (1 << 0)
#define SND_VOLR (1 << 1)
#define SND_ADSR1 (1 << 2)
#define SND_ADSR2 (1 << 3)
#define SND_ADDR (1 << 4)
#define SND_PITCH (1 << 5)

enum libsnd2_control_enum
{
	CC_NUMBER = 0,
	CC_BANKCHANGE,
	CC_DATAENTRY,
	CC_MAINVOL,
	CC_PANPOT,
	CC_EXPRESSION,
	CC_DAMPER,
	CC_NRPN1,
	CC_NRPN2,
	CC_RPN1,
	CC_RPN2,
	CC_EXTERNAL,
	CC_RESETALL,
	CC_MAX,
};

enum libsnd2_ccentry_enum
{
	DE_PRIORITY = 0,
	DE_MODE,
	DE_LIMITL,
	DE_LIMITH,
	DE_ADSR_AR_L,
	DE_ADSR_AR_E,
	DE_ADSR_DR,
	DE_ADSR_SL,
	DE_ADSR_SR_L,
	DE_ADSR_SR_E,
	DE_ADSR_RR_L,
	DE_ADSR_RR_E,
	DE_ADSR_SR,
	DE_VIB_TIME,
	DE_PORTA_DEPTH,
	DE_REV_TYPE,
	DE_REV_DEPTH,
	DE_ECHO_FB,
	DE_ECHO_DELAY,
	DE_DELAY,
	DE_MAX,
};

typedef struct VabHdr
{
	int form;
	int ver;
	int id;
	unsigned int fsize;
	u16 reserved0;
	u16 ps;
	u16 ts;
	u16 vs;
	u8 mvol;
	u8 pan;
	u8 attr1;
	u8 attr2;
	unsigned int reserved1;
} VabHdr;

typedef struct ProgAtr
{
	u8 tones;
	u8 mvol;
	u8 prior;
	u8 mode;
	u8 mpan;
	char reserved0;
	s16 attr;
	unsigned int m_fake_prog_idx;
	u16 m_vag_spu_addr_hi;
	u16 m_vag_spu_addr_lo;
} ProgAtr;

typedef struct VagAtr
{
	u8 prior;
	u8 mode;
	u8 vol;
	u8 pan;
	u8 center;
	u8 shift;
	u8 min;
	u8 max;
	u8 vibW;
	u8 vibT;
	u8 porW;
	u8 porT;
	u8 pbmin;
	u8 pbmax;
	u8 reserved1;
	u8 reserved2;
	u16 adsr1;
	u16 adsr2;
	s16 prog;
	s16 vag;
	s16 reserved[4];
} VagAtr;

typedef struct SndVolume_
{
	u16 left;
	u16 right;
} SndVolume;

typedef struct SndVolume2
{
	s16 left;
	s16 right;
} SndVolume2;

typedef struct SndRegisterAttr
{
	SndVolume2 volume;
	s16 pitch;
	s16 mask;
	s16 addr;
	s16 adsr1;
	s16 adsr2;
} SndRegisterAttr;

typedef struct SndVoiceStats
{
	s16 vagId;
	s16 vabId;
	u16 pitch;
	s16 note;
	s16 tone;
	s16 prog_num;
	s16 prog_actual;
	s16 vol;
	s16 pan;
} SndVoiceStats;

typedef void (*SsMarkCallbackProc)(s16, s16, s16);

typedef struct _SsFCALL_
{
	void (*noteon)(s16, s16, u8, u8);
	void (*programchange)(s16, s16, u8);
	void (*pitchbend)(s16, s16);
	void (*metaevent)(s16, s16, u8);
	void (*control[CC_MAX])(s16, s16, u8);
	void (*ccentry[DE_MAX])(s16, s16, s16, VagAtr, s16, u8);
} _SsFCALL;

extern void _SsContBankChange(s16 sep_no, s16 seq_no, u8 control_value);
extern void _SsContDataEntry(s16 sep_no, s16 seq_no, u8 control_value);
extern void _SsContMainVol(s16 sep_no, s16 seq_no, u8 control_value);
extern void _SsContPanpot(s16 sep_no, s16 seq_no, u8 control_value);
extern void _SsContExpression(s16 sep_no, s16 seq_no, u8 control_value);
extern void _SsContDamper(s16 sep_no, s16 seq_no, u8 control_value);
extern void _SsContExternal(s16 sep_no, s16 seq_no, u8 control_value);
extern void _SsContNrpn1(s16 sep_no, s16 seq_no, u8 control_value);
extern void _SsContNrpn2(s16 sep_no, s16 seq_no, u8 control_value);
extern void _SsContRpn1(s16 sep_no, s16 seq_no, u8 control_value);
extern void _SsContRpn2(s16 sep_no, s16 seq_no, u8 control_value);
extern void _SsContResetAll(s16 sep_no, s16 seq_no, u8 control_value);
extern void _SsSetNrpnVabAttr0(s16 vab_id, s16 prog, s16 tone, VagAtr vag_attr, s16 fn_idx, u8 attribute_value);
extern void _SsSetNrpnVabAttr1(s16 vab_id, s16 prog, s16 tone, VagAtr vag_attr, s16 fn_idx, u8 attribute_value);
extern void _SsSetNrpnVabAttr2(s16 vab_id, s16 prog, s16 tone, VagAtr vag_attr, s16 fn_idx, u8 attribute_value);
extern void _SsSetNrpnVabAttr3(s16 vab_id, s16 prog, s16 tone, VagAtr vag_attr, s16 fn_idx, u8 attribute_value);
extern void _SsSetNrpnVabAttr4(s16 vab_id, s16 prog, s16 tone, VagAtr vag_attr, s16 fn_idx, u8 attribute_value);
extern void _SsSetNrpnVabAttr5(s16 vab_id, s16 prog, s16 tone, VagAtr vag_attr, s16 fn_idx, u8 attribute_value);
extern void _SsSetNrpnVabAttr6(s16 vab_id, s16 prog, s16 tone, VagAtr vag_attr, s16 fn_idx, u8 attribute_value);
extern void _SsSetNrpnVabAttr7(s16 vab_id, s16 prog, s16 tone, VagAtr vag_attr, s16 fn_idx, u8 attribute_value);
extern void _SsSetNrpnVabAttr8(s16 vab_id, s16 prog, s16 tone, VagAtr vag_attr, s16 fn_idx, u8 attribute_value);
extern void _SsSetNrpnVabAttr9(s16 vab_id, s16 prog, s16 tone, VagAtr vag_attr, s16 fn_idx, u8 attribute_value);
extern void _SsSetNrpnVabAttr10(s16 vab_id, s16 prog, s16 tone, VagAtr vag_attr, s16 fn_idx, u8 attribute_value);
extern void _SsSetNrpnVabAttr11(s16 vab_id, s16 prog, s16 tone, VagAtr vag_attr, s16 fn_idx, u8 attribute_value);
extern void _SsSetNrpnVabAttr12(s16 vab_id, s16 prog, s16 tone, VagAtr vag_attr, s16 fn_idx, u8 attribute_value);
extern void _SsSetNrpnVabAttr13(s16 vab_id, s16 prog, s16 tone, VagAtr vag_attr, s16 fn_idx, u8 attribute_value);
extern void _SsSetNrpnVabAttr14(s16 vab_id, s16 prog, s16 tone, VagAtr vag_attr, s16 fn_idx, u8 attribute_value);
extern void _SsSetNrpnVabAttr15(s16 vab_id, s16 prog, s16 tone, VagAtr vag_attr, s16 fn_idx, u8 attribute_value);
extern void _SsSetNrpnVabAttr16(s16 vab_id, s16 prog, s16 tone, VagAtr vag_attr, s16 fn_idx, u8 attribute_value);
extern void _SsSetNrpnVabAttr17(s16 vab_id, s16 prog, s16 tone, VagAtr vag_attr, s16 fn_idx, u8 attribute_value);
extern void _SsSetNrpnVabAttr18(s16 vab_id, s16 prog, s16 tone, VagAtr vag_attr, s16 fn_idx, u8 attribute_value);
extern void _SsSetNrpnVabAttr19(s16 vab_id, s16 prog, s16 tone, VagAtr vag_attr, s16 fn_idx, u8 attribute_value);
extern void _SsSetPitchBend(s16 sep_no, s16 seq_no);
extern void _SsSetControlChange(s16 sep_no, s16 seq_no, u8 control_value);
extern void _SsGetMetaEvent(s16 sep_no, s16 seq_no, u8 meta_event);
extern void _SsNoteOn(s16 sep_no, s16 seq_no, u8 note, u8 vollr);
extern void _SsSetProgramChange(s16 sep_no, s16 seq_no, u8 prog);
extern void SsSeqSetAccelerando(s16 seq_no, int tempo, int v_time);
extern void SsSepSetAccelerando(s16 seq_no, s16 sep_no, int tempo, int v_time);
extern void SsSeqCalledTbyT(void);
extern void SsSeqClose(s16 seq_sep_no);
extern void SsSepClose(s16 seq_sep_no);
extern void SsChannelMute(s16 sep_no, s16 seq_no, int channels);
extern int SsGetChannelMute(s16 sep_no, s16 seq_no);
extern void SsSeqSetCrescendo(s16 sep_no, s16 vol, int v_time);
extern void SsSepSetCrescendo(s16 sep_no, s16 seq_no, s16 vol, int v_time);
extern void SsSeqSetDecrescendo(s16 sep_no, s16 vol, int v_time);
extern void SsSepSetDecrescendo(s16 sep_no, s16 seq_no, s16 vol, int v_time);
extern void SsEnd(void);
extern char SsGetMute(void);
extern void SsGetMVol(SndVolume *m_vol);
extern s16 SsGetNck(void);
extern void SsGetRVol(SndVolume *r_vol);
extern char SsGetSerialAttr(char s_num, char attr);
extern void SsGetSerialVol(char s_num, SndVolume *s_vol);
extern void SsInit(void);
extern void SsPitchCorrect(s16 pitch_correct);
extern void SsInitHot(void);
extern void SsSetLoop(s16 sep_no, s16 seq_no, s16 l_count);
extern s16 SsIsEos(s16 sep_no, s16 seq_no);
extern void SsSetMarkCallback(s16 sep_no, s16 seq_no, SsMarkCallbackProc proc);
extern void SsSetNext(s16 sep_no1, s16 seq_no1, s16 sep_no2, s16 seq_no2);
extern void SsSeqSetNext(s16 sep_no1, s16 sep_no2);
extern void SsSetNoiseOff(void);
extern void SsSetNoiseOn(s16 voll, s16 volr);
extern s16 SsSepOpen(unsigned int *addr, s16 vab_id, s16 seq_cnt);
extern s16 SsSeqOpen(unsigned int *addr, s16 vab_id);
extern s16 SsSepOpenJ(unsigned int *addr, s16 vab_id, s16 seq_cnt);
extern s16 SsSeqOpenJ(unsigned int *addr, s16 vab_id);
extern void SsSeqPause(s16 sep_no);
extern void SsSepPause(s16 sep_no, s16 seq_no);
extern void SsSeqPlay(s16 sep_no, char play_mode, s16 l_count);
extern void SsSepPlay(s16 sep_no, s16 seq_no, char play_mode, s16 l_count);
extern void SsPlayBack(s16 sep_no, s16 seq_no, s16 l_count);
extern void SsQuit(void);
extern void SsSeqReplay(s16 sep_no);
extern void SsSepReplay(s16 sep_no, s16 seq_no);
extern void SsSeqSetRitardando(s16 sep_no, int tempo, int v_time);
extern void SsSepSetRitardando(s16 sep_no, s16 seq_no, int tempo, int v_time);
extern int SsSeqSkip(s16 sep_no, s16 seq_no, char unit, s16 count);
extern int SsSetCurrentPoint(s16 sep_no, s16 seq_no, u8 *point);
extern void SsSeqPlayPtoP(s16 sep_no, s16 seq_no, u8 *start_point, u8 *end_point, char play_mode, s16 l_count);
extern void SsSetSerialAttr(char s_num, char attr, char mode);
extern void SsSetMute(char mode);
extern void SsSetMVol(s16 voll, s16 volr);
extern void SsSetNck(s16 n_clock);
extern void SsSetRVol(s16 voll, s16 volr);
extern void SsStart(void);
extern void SsStart2(void);
extern void SsSeqStop(s16 sep_no);
extern void SsSepStop(s16 sep_no, s16 seq_no);
extern void SsSetSerialVol(char s_num, s16 voll, s16 volr);
extern void SsSetTableSize(char *table, s16 s_max, s16 t_max);
extern void SsSetTempo(s16 sep_no, s16 seq_no, s16 tempo);
extern void SsSetTickMode(int tick_mode);
extern void *SsSetTickCallback(void (*cb)(void));
extern int SsVoKeyOff(int vab_pro, int pitch);
extern int SsVoKeyOn(int vab_pro, int pitch, u16 voll, u16 volr);
extern void SsSeqSetVol(s16 sep_no, s16 voll, s16 volr);
extern void SsSepSetVol(s16 sep_no, s16 seq_no, s16 voll, s16 volr);
extern void SsSeqGetVol(s16 sep_no, s16 seq_no, s16 *voll, s16 *volr);
extern void dmy_nothing1(s16 seq_no, s16 sep_no, u8 note, u8 vollr);
extern void dmy_SsNoteOn(s16 sep_no, s16 seq_no, u8 note, u8 vollr);
extern void dmy_SsSetProgramChange(s16 sep_no, s16 seq_no, u8 prog);
extern void dmy_SsGetMetaEvent(s16 sep_no, s16 seq_no, u8 meta_event);
extern void dmy_SsSetPitchBend(s16 sep_no, s16 seq_no);
extern void dmy_SsSetControlChange(s16 sep_no, s16 seq_no, u8 control_value);
extern void dmy_SsContBankChange(s16 sep_no, s16 seq_no, u8 control_value);
extern void dmy_SsContDataEntry(s16 sep_no, s16 seq_no, u8 control_value);
extern void dmy_SsContMainVol(s16 sep_no, s16 seq_no, u8 control_value);
extern void dmy_SsContPanpot(s16 sep_no, s16 seq_no, u8 control_value);
extern void dmy_SsContExpression(s16 sep_no, s16 seq_no, u8 control_value);
extern void dmy_SsContDamper(s16 sep_no, s16 seq_no, u8 control_value);
extern void dmy_SsContExternal(s16 sep_no, s16 seq_no, u8 control_value);
extern void dmy_SsContNrpn1(s16 sep_no, s16 seq_no, u8 control_value);
extern void dmy_SsContNrpn2(s16 sep_no, s16 seq_no, u8 control_value);
extern void dmy_SsContRpn1(s16 sep_no, s16 seq_no, u8 control_value);
extern void dmy_SsContRpn2(s16 sep_no, s16 seq_no, u8 control_value);
extern void dmy_SsContResetAll(s16 sep_no, s16 seq_no, u8 control_value);
extern void dmy_SsSetNrpnVabAttr0(s16 vab_id, s16 prog, s16 tone, VagAtr vag_attr, s16 fn_idx, u8 attribute_value);
extern void dmy_SsSetNrpnVabAttr1(s16 vab_id, s16 prog, s16 tone, VagAtr vag_attr, s16 fn_idx, u8 attribute_value);
extern void dmy_SsSetNrpnVabAttr2(s16 vab_id, s16 prog, s16 tone, VagAtr vag_attr, s16 fn_idx, u8 attribute_value);
extern void dmy_SsSetNrpnVabAttr3(s16 vab_id, s16 prog, s16 tone, VagAtr vag_attr, s16 fn_idx, u8 attribute_value);
extern void dmy_SsSetNrpnVabAttr4(s16 vab_id, s16 prog, s16 tone, VagAtr vag_attr, s16 fn_idx, u8 attribute_value);
extern void dmy_SsSetNrpnVabAttr5(s16 vab_id, s16 prog, s16 tone, VagAtr vag_attr, s16 fn_idx, u8 attribute_value);
extern void dmy_SsSetNrpnVabAttr6(s16 vab_id, s16 prog, s16 tone, VagAtr vag_attr, s16 fn_idx, u8 attribute_value);
extern void dmy_SsSetNrpnVabAttr7(s16 vab_id, s16 prog, s16 tone, VagAtr vag_attr, s16 fn_idx, u8 attribute_value);
extern void dmy_SsSetNrpnVabAttr8(s16 vab_id, s16 prog, s16 tone, VagAtr vag_attr, s16 fn_idx, u8 attribute_value);
extern void dmy_SsSetNrpnVabAttr9(s16 vab_id, s16 prog, s16 tone, VagAtr vag_attr, s16 fn_idx, u8 attribute_value);
extern void dmy_SsSetNrpnVabAttr10(s16 vab_id, s16 prog, s16 tone, VagAtr vag_attr, s16 fn_idx, u8 attribute_value);
extern void dmy_SsSetNrpnVabAttr11(s16 vab_id, s16 prog, s16 tone, VagAtr vag_attr, s16 fn_idx, u8 attribute_value);
extern void dmy_SsSetNrpnVabAttr12(s16 vab_id, s16 prog, s16 tone, VagAtr vag_attr, s16 fn_idx, u8 attribute_value);
extern void dmy_SsSetNrpnVabAttr13(s16 vab_id, s16 prog, s16 tone, VagAtr vag_attr, s16 fn_idx, u8 attribute_value);
extern void dmy_SsSetNrpnVabAttr14(s16 vab_id, s16 prog, s16 tone, VagAtr vag_attr, s16 fn_idx, u8 attribute_value);
extern void dmy_SsSetNrpnVabAttr15(s16 vab_id, s16 prog, s16 tone, VagAtr vag_attr, s16 fn_idx, u8 attribute_value);
extern void dmy_SsSetNrpnVabAttr16(s16 vab_id, s16 prog, s16 tone, VagAtr vag_attr, s16 fn_idx, u8 attribute_value);
extern void dmy_SsSetNrpnVabAttr17(s16 vab_id, s16 prog, s16 tone, VagAtr vag_attr, s16 fn_idx, u8 attribute_value);
extern void dmy_SsSetNrpnVabAttr18(s16 vab_id, s16 prog, s16 tone, VagAtr vag_attr, s16 fn_idx, u8 attribute_value);
extern void dmy_SsSetNrpnVabAttr19(s16 vab_id, s16 prog, s16 tone, VagAtr vag_attr, s16 fn_idx, u8 attribute_value);
extern u8 *SsGetCurrentPoint(s16 sep_no, s16 seq_no);
extern void SsSetVoiceMask(unsigned int s_voice);
extern unsigned int SsGetVoiceMask(void);
extern void SsUtAllKeyOff(s16 mode);
extern s16 SsUtAutoPan(s16 vc, s16 start_pan, s16 end_pan, s16 delta_time);
extern s16 SsUtAutoVol(s16 vc, s16 start_vol, s16 end_vol, s16 delta_time);
extern s16 SsUtChangeADSR(s16 vc, s16 vab_id, s16 prog, s16 old_note, u16 adsr1, u16 adsr2);
extern s16 SsUtChangePitch(s16 vc, s16 vab_id, s16 prog, s16 old_note, s16 old_fine, s16 new_note, s16 new_fine);
extern void SsUtFlush(void);
extern s16 SsUtGetProgAtr(s16 vab_id, s16 prog, ProgAtr *prog_attr_ptr);
extern s16 SsUtGetVagAtr(s16 vab_id, s16 prog, s16 tone, VagAtr *vag_attr_ptr);
extern int SsUtGetVagAddr(s16 vab_id, s16 vag_id);
extern unsigned int SsUtGetVagAddrFromTone(s16 vab_id, s16 prog, s16 tone);
extern unsigned int SsUtGetVBaddrInSB(s16 vab_id);
extern s16 SsUtGetVabHdr(s16 vab_id, VabHdr *vab_hdr_ptr);
extern s16 SsUtKeyOn(s16 vab_id, s16 prog, s16 tone, s16 note, s16 fine, s16 voll, s16 volr);
extern s16 SsUtKeyOff(s16 vc, s16 vab_id, s16 prog, s16 tone, s16 note);
extern s16 SsUtKeyOnV(s16 vc, s16 vab_id, s16 prog, s16 tone, s16 note, s16 fine, s16 voll, s16 volr);
extern s16 SsUtKeyOffV(s16 vc);
extern s16 SsUtPitchBend(s16 vc, s16 vab_id, s16 prog, s16 note, s16 pbend);
extern void SsUtSetReverbDelay(s16 delay);
extern void SsUtSetReverbDepth(s16 ldepth, s16 rdepth);
extern s16 SsUtSetReverbType(s16 type);
extern s16 SsUtGetReverbType(void);
extern void SsUtSetReverbFeedback(s16 feedback);
extern void SsUtReverbOff(void);
extern void SsUtReverbOn(void);
extern s16 SsUtSetProgAtr(s16 vab_id, s16 prog, const ProgAtr *prog_attr_ptr);
extern s16 SsUtSetVagAtr(s16 vab_id, s16 prog, s16 tone, const VagAtr *vag_attr_ptr);
extern s16 SsUtSetVabHdr(s16 vab_id, const VabHdr *vab_hdr_ptr);
extern s16 SsUtGetDetVVol(s16 vc, s16 *detvoll, s16 *detvolr);
extern s16 SsUtSetDetVVol(s16 vc, s16 detvoll, s16 detvolr);
extern s16 SsUtGetVVol(s16 vc, s16 *voll, s16 *volr);
extern s16 SsUtSetVVol(s16 vc, s16 voll, s16 volr);
extern u16 SsPitchFromNote(s16 note, s16 fine, u8 center, u8 shift);
extern void SsSetAutoKeyOffMode(s16 mode);
extern void SsSetMono(void);
extern void SsSetStereo(void);
extern char SsSetReservedVoice(char voices);
extern void SsVabClose(s16 vab_id);
extern s16 SsVabFakeBody(s16 vab_id);
extern s16 SsVabOpenHeadSticky(u8 *addr, s16 vab_id, unsigned int sbaddr);
extern s16 SsVabFakeHead(u8 *addr, s16 vab_id, unsigned int sbaddr);
extern s16 SsVabOpenHead(u8 *addr, s16 vab_id);
extern s16 SsVabTransfer(u8 *vh_addr, u8 *vb_addr, s16 vab_id, s16 i_flag);
extern s16 SsVabTransBody(u8 *addr, s16 vab_id);
extern s16 SsVabTransBodyPartly(u8 *addr, unsigned int bufsize, s16 vab_id);
extern s16 SsVabTransCompleted(s16 immediate_flag);
extern char SsBlockVoiceAllocation(void);
extern char SsUnBlockVoiceAllocation(void);
extern int SsAllocateVoices(u8 voices, u8 priority);
extern void SsQueueKeyOn(int voices);
extern void SsQueueReverb(int voices, int reverb);
extern void SsQueueRegisters(int vc, SndRegisterAttr *sra);
extern s16 SsGetActualProgFromProg(s16 vab_id, s16 prog);
extern void SsSetVoiceSettings(int vc, const SndVoiceStats *snd_v_attr);
extern s16 SsVoiceCheck(int vc, int vab_id, s16 note);

#endif

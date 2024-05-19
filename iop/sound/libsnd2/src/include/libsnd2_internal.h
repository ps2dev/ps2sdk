/*
# _____     ___ ____     ___ ____
#  ____|   |    ____|   |        | |____|
# |     ___|   |____ ___|    ____| |    \    PS2DEV Open Source Project.
#-----------------------------------------------------------------------
# Copyright ps2dev - http://www.ps2dev.org
# Licenced under Academic Free License version 2.0
# Review ps2sdk README & LICENSE files for further details.
*/

#ifndef _LIBSND2_INTERNAL_H
#define _LIBSND2_INTERNAL_H

#include <intrman.h>
#include <stdio.h>
#include <sysclib.h>
#include <tamtypes.h>
#include <thbase.h>
#include <timrman.h>
#include <vblank.h>

#include "libspu2_internal.h"

#include <libsnd2.h>

typedef struct libsnd2_seq_tick_env_
{
	s32 m_tick_mode;
	s32 m_manual_tick;
	void (*m_tick_callback)();
	void (*m_vsync_callback)();
	u8 m_vsync_tick;
	u8 m_unk11;
	u8 m_alarm_tick;
	u8 m_pad13;
} libsnd2_seq_tick_env_t;

typedef int (*libsnd2_vab_allocate_callback)(unsigned int size_in_bytes, int mode, s16 vab_id);

typedef void (*libsnd2_ss_mark_callback_proc)(s16 seq_no, s16 sep_no, s16 data);

typedef struct libsnd2_sequence_marker_
{
	libsnd2_ss_mark_callback_proc m_entries[16];
} libsnd2_sequence_marker_t;

typedef struct libsnd2_sequence_struct_
{
	u8 *m_seq_ptr;
	u8 *m_unk04;
	u8 *m_unk08;
	u8 *m_unk0C;
	u8 *m_unk10;
	char m_play_mode;
	char m_unk15;
	u8 m_running_status;
	u8 m_channel_idx;
	char m_unk18;
	char m_unk19;
	char m_fn_idx;
	u8 m_unk1B;
	char m_unk1C;
	u8 m_unk1D;
	char m_unk1E;
	char m_unk1F;
	char m_l_count;
	char m_unk21;
	s8 m_next_sep;
	s8 m_next_seq;
	char m_rhythm_n;
	char m_rhythm_d;
	u8 m_vab_id;
	char m_panpot[16];
	char m_programs[16];
	char m_unk47;
	s16 m_unk48;
	s16 m_unk4A;
	s16 m_unk4C;
	s16 m_unk4E;
	s16 m_resolution_of_quarter_note;
	s16 m_unk52;
	s16 m_unk54;
	s16 m_unk56;
	s16 m_voll;
	s16 m_volr;
	s16 m_unk5C;
	s16 m_unk5E;
	s16 m_vol[16];
	s16 m_channel_mute;
	char m_unk82;
	char m_unk83;
	int m_unk84;
	int m_unk88;
	int m_tempo;
	int m_delta_value;
	int m_unk94;
	unsigned int m_flags;
	int m_unk9C;
	int m_unkA0;
	int m_unkA4;
	int m_unkA8;
	int m_unkAC;
} libsnd2_sequence_struct_t;

typedef struct libsnd2_reg_buffer_struct_
{
	s16 m_vol_left;
	s16 m_vol_right;
	s16 m_pitch;
	u16 m_adsr1;
	u16 m_adsr2;
	s16 m_unka;
	s16 m_unkc;
	u16 m_unke;
} libsnd2_reg_buffer_struct_t;

typedef struct libsnd2_spu_voice_
{
	s16 m_vag_idx;
	s16 m_unk02;
	s16 m_pitch;
	s16 m_key_stat;
	s16 m_voll1;
	char m_pan;
	char m_pad05;
	s16 m_channel_idx;
	s16 m_note;
	s16 m_seq_sep_no;
	s16 m_fake_program;
	s16 m_prog;
	s16 m_tone;
	s16 m_vab_id;
	s16 m_priority;
	char m_pad08;
	char m_unk1d;
	s16 m_b_auto_vol;
	s16 m_auto_vol_amount;
	s16 m_auto_vol_dt1;
	s16 m_auto_vol_dt2;
	s16 m_auto_vol_start;
	s16 m_auto_vol_end;
	s16 m_b_auto_pan;
	s16 m_auto_pan_amount;
	s16 m_auto_pan_dt1;
	s16 m_auto_pan_dt2;
	s16 m_auto_pan_start;
	s16 m_auto_pan_end;
	s16 m_voll2;
} libsnd2_spu_voice_t;

typedef struct libsnd2_svm_
{
	char m_sep_sep_no_tonecount;
	u8 m_vab_id;
	char m_note;
	char m_fine;
	char m_voll;
	char m_unk05;
	char m_program;
	char m_fake_program;
	char m_unknown;
	char m_pad09;
	char m_mvol;
	char m_mpan;
	char m_tone;
	char m_vol;
	char m_pan;
	char m_prior;
	char m_centre;
	u8 m_shift;
	char m_mode;
	char m_pad13;
	s16 m_seq_sep_no;
	s16 m_vag_idx2;
	s16 m_voice_idx;
	int m_unk1c;
} libsnd2_svm_t;

typedef void (*libsnd2_auto_vol_pan_callback)(unsigned int voice);

typedef struct libsnd2_reg_buffer_struct_2_
{
	u16 m_unk0;
	u16 m_unk2;
	u16 m_unk4;
	u16 m_unk6;
	u16 m_vag_spu_addr;
	u16 m_vab_spu_offset;
	u16 m_unkc;
	u16 m_unke;
} libsnd2_reg_buffer_struct_2_t;

extern libsnd2_sequence_marker_t _SsMarkCallback[32];
extern int VBLANK_MINUS;
extern unsigned int _snd_openflag;
extern int _snd_ev_flag;
extern _SsFCALL SsFCALL;
extern libsnd2_sequence_struct_t *_ss_score[32];
extern s16 _snd_seq_s_max;
extern s16 _snd_seq_t_max;

extern s16 _svm_stereo_mono;
extern int _svm_vab_not_send_size;
extern SpuReverbAttr _svm_rattr;
extern char _svm_vab_used[16];
extern char _SsVmMaxVoice;
extern s16 _svm_vab_count;
extern s16 kMaxPrograms;
extern libsnd2_svm_t _svm_cur;
extern s16 _svm_damper;
extern u8 _svm_auto_kof_mode;
extern VabHdr *_svm_vab_vh[16];
extern ProgAtr *_svm_vab_pg[16];
extern VagAtr *_svm_vab_tn[16];
extern int _svm_vab_start[16];
extern int _svm_vab_total[16];
extern VabHdr *_svm_vh;
extern ProgAtr *_svm_pg;
extern VagAtr *_svm_tn;

extern u16 _svm_okon1;
// extern int _svm_envx_ptr;
extern unsigned int _svm_envx_hist[16];
extern libsnd2_spu_voice_t _svm_voice[24];
extern u16 _svm_okof1;
extern u16 _svm_okof2;
extern u16 _svm_okon2;
extern libsnd2_auto_vol_pan_callback _autovol;
extern libsnd2_auto_vol_pan_callback _autopan;
extern char _svm_sreg_dirty[24];
extern libsnd2_reg_buffer_struct_t _svm_sreg_buf[24];
extern libsnd2_reg_buffer_struct_2_t _svm_sreg_buf2[24];
extern u16 _svm_orev2;
extern u16 _svm_orev1;
extern u16 _svm_onos2;
extern u16 _svm_onos1;

extern s16 gVabOffet[16];

extern unsigned int _snd_vmask;

// extern s16 gPitchCorrect;

extern libsnd2_seq_tick_env_t _snd_seq_tick_env;
extern int _snd_seq_interval;

extern void _SsUtResolveADSR(u16 adsr1, u16 adsr2, u16 *adsr_buf);
extern void _SsUtBuildADSR(const u16 *adsr_buf, u16 *adsr1, u16 *adsr2);
extern void _SsSndCrescendo(s16 sep_no, s16 seq_no);
extern void _SsSeqPlay(s16 sep_no, s16 seq_no);
extern void _SsSeqGetEof(s16 sep_no, s16 seq_no);
extern int _SsGetSeqData(s16 sep_no, s16 seq_no);
extern s32 _SsReadDeltaValue(s16 sep_no, s16 seq_no);
extern void _SsSndNextSep(s16 sep_no, s16 seq_no);
extern void _SsSndNextPause(s16 sep_no, s16 seq_no);
extern void _SsSndPause(s16 sep_no, s16 seq_no);
extern void _SsSndPlay(s16 sep_no, s16 seq_no);
extern void _SsSndReplay(s16 sep_no, s16 seq_no);
extern int _SsInitSoundSep(s16 sep_no, int seq_no, u8 vab_id, u8 *addr);
extern s16 _SsInitSoundSeq(s16 seq_no, s16 vab_id, u8 *addr);
extern void _SsSndSetAccele(s16 sep_no, s16 seq_no, int tempo, int v_time);
extern void _SsSndSetCres(s16 sep_no, s16 seq_no, s16 vol, int v_time);
extern void _SsSndSetDecres(s16 sep_no, s16 seq_no, s16 vol, int v_time);
extern void _SsInit(void);
extern void _SsSndSetPauseMode(s16 sep_no, s16 seq_no);
extern void _SsSndSetReplayMode(s16 sep_no, s16 seq_no);
extern void Snd_SetPlayMode(s16 sep_no, s16 seq_no, char play_mode, char l_count);
extern void _SsSndSetRit(s16 sep_no, s16 seq_no, int tempo, int v_time);
#ifdef LIB_1300
extern void _SsTrapIntrVSync(void);
#endif
#ifdef LIB_1300
extern void _SsSeqCalledTbyT_1per2(void);
#endif
extern int _SsTrapIntrProcIOP(void *userdata);
extern void _SsSndStop(s16 sep_no, s16 seq_no);
extern void _SsSndSetVol(s16 sep_no, s16 seq_no, u16 voll, u16 volr);
extern void _SsSndTempo(s16 sep_no, s16 seq_no);
extern void _SsSndSetVolData(s16 sep_no, s16 seq_no, s16 vol, int v_time);
extern s16 _SsVmAlloc(void);
extern void _SsVmDoAllocate(void);
extern void SeAutoPan(s16 vc, s16 start_pan, s16 end_pan, s16 delta_time);
extern void SetAutoPan(int vc);
extern void SeAutoVol(s16 vc, s16 start_vol, s16 end_vol, s16 delta_time);
extern void SetAutoVol(int vc);
extern void _SsVmDamperOff(void);
extern void _SsVmDamperOn(void);
extern void wait1fsa(void);
extern void DumpSpu(void);
extern void DumpVoice(void);
extern void DumpVoice2(void);
extern void _SsVmFlush(void);
extern void _SsVmInit(int voice_count);
extern int _SsVmKeyOn(int seq_sep_no, s16 vab_id, s16 prog, s16 note, s16 voll, s16 unknown27);
extern int _SsVmKeyOff(int seq_sep_no, s16 vab_id, s16 prog, s16 note);
extern int _SsVmSeKeyOn(s16 vab_id, s16 prog, u16 note, int pitch, u16 voll, u16 volr);
extern int _SsVmSeKeyOff(s16 vab_id, s16 prog, s16 note);
extern void KeyOnCheck(void);
extern s16 note2pitch(void);
extern s16 note2pitch2(s16 note, s16 fine);
extern void vmNoiseOn(u8 vc);
extern void vmNoiseOn2(u8 vc, u16 voll, u16 volr, u16 arg3, u16 arg4);
extern void vmNoiseOff(u8 vc);
extern void _SsVmNoiseOnWithAdsr(s32 voll, s32 volr, s32 arg2, s32 arg3);
extern void _SsVmNoiseOff(void);
extern void _SsVmNoiseOn(u16 voll, u16 volr);
extern void _SsVmKeyOffNow(void);
extern void _SsVmKeyOnNow(s16 vag_count, s16 pitch);
extern int _SsVmPBVoice(s16 vc, s16 seq_sep_num, s16 vab_id, s16 prog, s16 pitch);
extern int _SsVmPitchBend(s16 seq_sep_no, int vab_id, int prog, s16 pitch);
extern void _SsVmSetProgVol(s16 vab_id, s16 prog, u8 vol);
extern int _SsVmGetProgVol(s16 vab_id, s16 prog);
extern int _SsVmSetProgPan(s16 vab_id, s16 prog, char mpan);
extern int _SsVmGetProgPan(s16 vab_id, s16 prog);
extern void _SsVmSetSeqVol(s16 seq_sep_num, s16 voll, s16 volr);
extern void _SsVmGetSeqVol(s16 seq_sep_no, s16 *voll_ptr, s16 *volr_ptr);
extern int _SsVmGetSeqLVol(s16 seq_sep_no);
extern int _SsVmGetSeqRVol(s16 seq_sep_no);
extern void _SsVmSeqKeyOff(s16 seq_sep_no);
extern void SePitchBend(u8 vc, s16 arg1);
extern s16 _SsVmSelectToneAndVag(u8 *vag_attr_idx_ptr, u8 *vag_nums_ptr);
extern void SeVibOn(void);
extern void SetVib(void);
extern void SsUtVibrateOn(void);
extern void SsUtVibrateOff(void);
extern int _SsVmSetVol(s16 seq_sep_no, s16 vab_id, s16 prog, s16 voll, s16 volr);
extern int _SsVmVSetUp(s16 vab_id, s16 prog);
extern int _SsVabOpenHeadWithMode(u8 *addr, int vab_id, libsnd2_vab_allocate_callback alloc_fn, int mode);

#endif

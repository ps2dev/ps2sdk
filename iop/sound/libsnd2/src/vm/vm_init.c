/*
# _____     ___ ____     ___ ____
#  ____|   |    ____|   |        | |____|
# |     ___|   |____ ___|    ____| |    \    PS2DEV Open Source Project.
#-----------------------------------------------------------------------
# Copyright ps2dev - http://www.ps2dev.org
# Licenced under Academic Free License version 2.0
# Review ps2sdk README & LICENSE files for further details.
*/

#include "libsnd2_internal.h"

static libspu2_malloc_t _ss_spu_vm_rec[32];

void _SsVmInit(int voice_count)
{
	u16 v8;
	SpuVoiceAttr voice_attr;

	_spu_setInTransfer(0);
	_svm_damper = 0;
	SpuInitMalloc(32, (char *)_ss_spu_vm_rec);
	memset(&_svm_sreg_buf, 0, sizeof(_svm_sreg_buf));
	memset(&_svm_sreg_buf2, 0, sizeof(_svm_sreg_buf2));
	memset(&_svm_sreg_dirty, 0, sizeof(_svm_sreg_dirty));
	_svm_vab_count = 0;
	memset(&_svm_vab_used, 0, sizeof(_svm_vab_used));
	if ( ((char)voice_count & 0xFFFFu) < 0x18 )
		_SsVmMaxVoice = (char)voice_count;
	else
		_SsVmMaxVoice = 24;
	voice_attr.mask =
		SPU_VOICE_VOLL | SPU_VOICE_VOLR | SPU_VOICE_PITCH | SPU_VOICE_WDSA | SPU_VOICE_ADSR_ADSR1 | SPU_VOICE_ADSR_ADSR2;
	voice_attr.pitch = 0x1000;
	voice_attr.addr = 0x5000;
	voice_attr.adsr1 = 0x80FF;
	voice_attr.volume.left = 0;
	voice_attr.volume.right = 0;
	voice_attr.adsr2 = 0x4000;
	for ( v8 = 0; v8 < _SsVmMaxVoice; v8 += 1 )
	{
		libsnd2_spu_voice_t *voice_struct;

		voice_struct = &_svm_voice[v8];
		voice_struct->m_unk02 = 24;
		voice_struct->m_vag_idx = 255;
		voice_struct->m_unk1d = 0;
		voice_struct->m_pitch = 0;
		voice_struct->m_key_stat = 0;
		voice_struct->m_seq_sep_no = -1;
		voice_struct->m_fake_program = 0;
		voice_struct->m_prog = 0;
		voice_struct->m_tone = 255;
		voice_struct->m_voll1 = 0;
		voice_struct->m_channel_idx = 0;
		voice_struct->m_pan = 64;
		voice_struct->m_voll2 = 0;
		voice_struct->m_b_auto_vol = 0;
		voice_struct->m_auto_vol_amount = 0;
		voice_struct->m_auto_vol_dt1 = 0;
		voice_struct->m_auto_vol_dt2 = 0;
		voice_struct->m_b_auto_pan = 0;
		voice_struct->m_auto_pan_amount = 0;
		voice_struct->m_auto_pan_dt1 = 0;
		voice_struct->m_auto_pan_dt2 = 0;
		voice_struct->m_auto_pan_start = 0;
		voice_struct->m_auto_vol_start = 0;
		voice_attr.voice = 1 << v8;
		SpuSetVoiceAttr(&voice_attr);
		_svm_cur.m_voice_idx = v8;
		_SsVmKeyOffNow();
	}
	_svm_rattr.mask = 0;
	_svm_rattr.depth.left = 0x3FFF;
	_svm_rattr.depth.right = 0x3FFF;
	_svm_rattr.mode = 0;
	_svm_okon1 = 0;
	_svm_okon2 = 0;
	_svm_okof1 = 0;
	_svm_orev1 = 0;
	_svm_orev2 = 0;
	_svm_onos1 = 0;
	_svm_onos2 = 0;
	_svm_auto_kof_mode = 0;
	_svm_stereo_mono = 0;
	_svm_vab_not_send_size = 0;
	kMaxPrograms = 128;
	_SsVmFlush();
}

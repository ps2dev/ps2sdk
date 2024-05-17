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

static u16 _svm_ntable[12] = {
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
};
static u16 _svm_ftable[128] = {
	0x8000, 0x800e, 0x801d, 0x802c, 0x803b, 0x804a, 0x8058, 0x8067, 0x8076, 0x8085, 0x8094, 0x80a3, 0x80b1,
	0x80c0, 0x80cf, 0x80de, 0x80ed, 0x80fc, 0x810b, 0x811a, 0x8129, 0x8138, 0x8146, 0x8155, 0x8164, 0x8173,
	0x8182, 0x8191, 0x81a0, 0x81af, 0x81be, 0x81cd, 0x81dc, 0x81eb, 0x81fa, 0x8209, 0x8218, 0x8227, 0x8236,
	0x8245, 0x8254, 0x8263, 0x8272, 0x8282, 0x8291, 0x82a0, 0x82af, 0x82be, 0x82cd, 0x82dc, 0x82eb, 0x82fa,
	0x830a, 0x8319, 0x8328, 0x8337, 0x8346, 0x8355, 0x8364, 0x8374, 0x8383, 0x8392, 0x83a1, 0x83b0, 0x83c0,
	0x83cf, 0x83de, 0x83ed, 0x83fd, 0x840c, 0x841b, 0x842a, 0x843a, 0x8449, 0x8458, 0x8468, 0x8477, 0x8486,
	0x8495, 0x84a5, 0x84b4, 0x84c3, 0x84d3, 0x84e2, 0x84f1, 0x8501, 0x8510, 0x8520, 0x852f, 0x853e, 0x854e,
	0x855d, 0x856d, 0x857c, 0x858b, 0x859b, 0x85aa, 0x85ba, 0x85c9, 0x85d9, 0x85e8, 0x85f8, 0x8607, 0x8617,
	0x8626, 0x8636, 0x8645, 0x8655, 0x8664, 0x8674, 0x8683, 0x8693, 0x86a2, 0x86b2, 0x86c1, 0x86d1, 0x86e0,
	0x86f0, 0x8700, 0x870f, 0x871f, 0x872e, 0x873e, 0x874e, 0x875d, 0x876d, 0x877d, 0x878c,
};

s16 note2pitch(void)
{
	u8 m_shift;

	m_shift = _svm_cur.m_shift;
	if ( (m_shift & 0x80u) != 0 )
		m_shift = 127;
	return SsPitchFromNote(_svm_cur.m_note, 0, _svm_cur.m_centre, m_shift);
}

s16 note2pitch2(s16 note, s16 fine)
{
	VagAtr *pVag;

	pVag = &_svm_tn[_svm_cur.m_tone + (_svm_cur.m_fake_program * 16)];
	return SsPitchFromNote(note, fine, pVag->center, pVag->shift);
}

u16 SsPitchFromNote(s16 note, s16 fine, u8 center, u8 shift)
{
	int shift_plus_fine;
	int shift_plus_fine_div_minus;
	unsigned int shift_plus_fine_div_minus_tmp;
	int shift_plus_fine_mod;
	int shift_plus_fine_mod_tmp;
	int shift_plus_fine_mul_minus;
	s16 v10;
	int v11;

	shift_plus_fine = (s16)(shift + fine);
	shift_plus_fine_div_minus = note + shift_plus_fine / 128 - center;
	shift_plus_fine_div_minus_tmp = shift_plus_fine_div_minus;
	shift_plus_fine_mod = shift_plus_fine % 128;
	shift_plus_fine_mod_tmp = shift_plus_fine_mod;
	if ( (shift_plus_fine_mod & 0x8000) != 0 )
	{
		shift_plus_fine_mod_tmp = shift_plus_fine_mod + 128;
		shift_plus_fine_div_minus_tmp = shift_plus_fine_div_minus - 1 + (s16)(shift_plus_fine_mod + 128) / 128;
	}
	shift_plus_fine_mul_minus = ((int)((u64)(0x2AAAAAABLL * (s16)shift_plus_fine_div_minus_tmp) >> 32) >> 1)
														- ((int)(shift_plus_fine_div_minus_tmp << 16 >> 31));
	v10 = (s16)shift_plus_fine_div_minus_tmp / 12 - 2;
	v11 = (s16)shift_plus_fine_div_minus_tmp - 12 * shift_plus_fine_mul_minus;
	if ( (v11 & 0x8000) != 0 )
	{
		v11 += 12;
		v10 = shift_plus_fine_mul_minus - 3;
	}
	if ( v10 >= 0 )
		return 0x3FFF;
	return (unsigned int)(((_svm_ntable[v11] * _svm_ftable[shift_plus_fine_mod_tmp]) >> 16) + (1 << (-v10 - 1))) >> -v10;
}

/*
# _____     ___ ____     ___ ____
#  ____|   |    ____|   |        | |____|
# |     ___|   |____ ___|    ____| |    \    PS2DEV Open Source Project.
#-----------------------------------------------------------------------
# Copyright ps2dev - http://www.ps2dev.org
# Licenced under Academic Free License version 2.0
# Review ps2sdk README & LICENSE files for further details.
*/

#include "libspu2_internal.h"

static s16 _spu_NTable[12] = {
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
static s16 _spu_FTable[128] = {
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

u16 _spu_note2pitch(u16 cen_note_high, u16 cen_note_low, u16 note_high, u16 note_low)
{
	u16 max_lo;
	int calc_note;
	u16 max_lo_idx;
	int calc_note_div12;
	int calc_note_div12_min12;
	int calc_note_mod12;
	int calc_note_mod12_idx;

	max_lo = note_low + cen_note_low;
	calc_note = (s16)(note_high + (max_lo >> 7) - cen_note_high);
	max_lo_idx = max_lo & 0x7F;
	calc_note_div12 = calc_note / 12;
	calc_note_div12_min12 = calc_note / 12 - 2;
	calc_note_mod12 = calc_note % 12;
	calc_note_mod12_idx = calc_note_mod12;
	if ( (calc_note_mod12 & 0x8000) != 0 )
	{
		calc_note_mod12_idx = calc_note_mod12 + 12;
		calc_note_div12_min12 = calc_note_div12 - 3;
	}
	if ( (calc_note_div12_min12 & 0x8000u) == 0 )
		return 0x3FFF;
	return (unsigned int)(((_spu_NTable[calc_note_mod12_idx] * (u16)_spu_FTable[max_lo_idx]) >> 16)
												+ (1 << (-(s16)calc_note_div12_min12 - 1)))
			>> -(s16)calc_note_div12_min12;
}

int _spu_pitch2note(s16 note_high, s16 note_low, u16 pitch)
{
	s16 v3;
	u16 v4;
	s16 v5;
	int v6;
	int v8;
	int v9;
	int v11;

	v3 = 0;
	v4 = 0;
	v5 = 0;
	if ( pitch >= 0x4000u )
		pitch = 0x3FFF;
	for ( v6 = 0; v6 < 14; v6 += 1 )
	{
		if ( (((int)pitch >> v6) & 1) != 0 )
			v5 = v6;
	}
	v8 = pitch << (15 - v5);
	for ( v9 = 11; v9 >= 0; v9 -= 1 )
	{
		if ( (u16)v8 >= (unsigned int)(u16)_spu_NTable[v9] )
		{
			v4 = v9;
			break;
		}
	}
	if ( !_spu_NTable[v4] )
		__builtin_trap();
	for ( v11 = 127; v11 >= 0; v11 -= 1 )
	{
		if ( (u16)(((u16)v8 << 15) / (unsigned int)(u16)_spu_NTable[v4]) >= (unsigned int)(u16)_spu_FTable[v11] )
		{
			v3 = v11;
			break;
		}
	}
	return ((u16)(v4 + note_high + 12 * (v5 - 12) + ((u16)(note_low + v3 + 1) >> 7)) << 8) | ((note_low + v3 + 1) & 0x7E);
}

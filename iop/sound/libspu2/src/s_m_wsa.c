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

static int getNeedBlock(int unk_a1, int unk_a2, int addr_area, int unk_a4);

int SpuMallocWithStartAddr(unsigned int addr, int size)
{
	int v2;
	int NeedBlock;
	int size_area;
	unsigned int v5;
	u32 v7;
	int v8;
	int v9;
	int v11;
	int v12;
	int v13;
	int v15;
	const libspu2_malloc_t *v17;
	int v20;
	libspu2_malloc_t *v22;
	int v29;

	if ( _spu_rev_reserve_wa )
		v7 = 0x200000 - _spu_rev_offsetaddr;
	else
		v7 = 0;
	v8 = 2 * ((int)addr >> 1);
	v9 = 2 * (size >> 1);
	if ( v8 < 0x5010 || (int)(0x200000 - v7) < v8 + v9 )
		return -1;
	_spu_gcSPU();
	v11 = _spu_AllocBlockNum;
	v12 = -1;
	for ( v13 = 0; v13 < _spu_AllocBlockNum; v13 += 1 )
	{
		if ( (_spu_memList[v13].addr_area & 0x40000000) != 0 )
		{
			v11 = v13;
			break;
		}
	}
	v29 = _spu_AllocBlockNum - v11;
	for ( v15 = 0; v15 < _spu_AllocBlockNum; v15 += 1 )
	{
		int addr_area;
		int v19;

		v17 = &_spu_memList[v15];
		size_area = v17->size_area;
		addr_area = v17->addr_area;
		v2 = v17->addr_area & 0xFFFFFFF;
		v19 = size_area;
		if ( v2 < v8 )
			v19 = size_area - (v8 - v2);
		if ( v19 >= v9 )
		{
			v5 = addr_area & 0xF0000000;
			if ( (addr_area & 0xF0000000) != 0 )
			{
				NeedBlock = getNeedBlock(v8, v9, addr_area, v19);
				if ( v29 >= NeedBlock )
				{
					v12 = v15;
					break;
				}
			}
		}
	}
	if ( v12 < 0 )
	{
		return -1;
	}
	v20 = -1;
	if ( NeedBlock )
	{
		int v21;

		for ( v21 = v11; v21 > v12; v21 -= 1 )
		{
			const libspu2_malloc_t *v25;

			v25 = &_spu_memList[v11];
			if ( _spu_AllocBlockNum >= v21 + NeedBlock )
			{
				libspu2_malloc_t *v26;

				v26 = &_spu_memList[v21 + NeedBlock];
				v26->addr_area = v25->addr_area;
				v26->size_area = v25->size_area;
			}
		}
		if ( v2 >= v8 )
		{
			libspu2_malloc_t *v28;

			v20 = v2;
			v28 = &_spu_memList[v12];
			v28[1].addr_area = (v2 + v9) | v5;
			v28->addr_area = v2;
			v28->size_area = v9;
			v28[1].size_area = size_area - v9;
		}
		else
		{
			libspu2_malloc_t *v27;

			v27 = &_spu_memList[v12];
			v27->addr_area = v2 | 0x80000000;
			v27->size_area = v8 - v2;
			v27[1].addr_area = v8;
			v27[1].size_area = v9;
			if ( NeedBlock == 2 )
			{
				v27[2].addr_area = (v8 + v9) | v5;
				v27[2].size_area = size_area - (v8 - v2) - v9;
			}
			return v8;
		}
	}
	else
	{
		v22 = &_spu_memList[v12];
		v20 = v22->addr_area & 0xFFFFFFF;
		v22->addr_area = v20;
	}
	return v20;
}

static int getNeedBlock(int unk_a1, int unk_a2, int addr_area, int unk_a4)
{
	if ( (addr_area & 0xFFFFFFF) >= unk_a1 )
	{
		if ( unk_a2 != unk_a4 )
			return 1;
		return (addr_area & 0x40000000) != 0;
	}
	else
	{
		if ( unk_a2 == unk_a4 )
		{
			if ( (addr_area & 0x40000000) != 0 )
				return 2;
			return 1;
		}
	}
	return 2;
}

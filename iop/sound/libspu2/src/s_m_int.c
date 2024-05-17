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

void _spu_gcSPU(void)
{
	int counter_1;
	libspu2_malloc_t *p_cur_block_2;
	int counter_3;
	libspu2_malloc_t *p_next_block;
	u32 addr_area;
	u32 size_area_1;
	int idx;
	const libspu2_malloc_t *p_cur_block_3;
	u32 size_area_2;
	int alloc_last_num_1;
	u32 size_area_3;

	for ( counter_1 = 0; counter_1 <= _spu_AllocLastNum; counter_1 += 1 )
	{
		while ( (_spu_memList[counter_1].addr_area & 0x80000000) != 0 )
		{
			int list_idx;

			for ( list_idx = counter_1 + 1; _spu_memList[list_idx].addr_area == 0x2fffffff; list_idx += 1 )
			{
			}
			p_cur_block_2 = &_spu_memList[list_idx];
			if (
				(p_cur_block_2->addr_area & 0x80000000) != 0
				&& (p_cur_block_2->addr_area & 0xFFFFFFF)
						 == (_spu_memList[counter_1].addr_area & 0xFFFFFFF) + _spu_memList[counter_1].size_area )
			{
				p_cur_block_2->addr_area = 0x2fffffff;
				_spu_memList[counter_1].size_area += p_cur_block_2->size_area;
				continue;
			}
			break;
		}
	}
	if ( _spu_AllocLastNum >= 0 )
	{
		if ( !_spu_memList[0].size_area )
			_spu_memList[0].addr_area = 0x2fffffff;
	}
	for ( counter_3 = 0; counter_3 <= _spu_AllocLastNum; counter_3 += 1 )
	{
		int counter_next;

		if ( (_spu_memList[counter_3].addr_area & 0x40000000) != 0 )
			break;
		for ( counter_next = counter_3 + 1; _spu_AllocLastNum >= counter_next; counter_next += 1 )
		{
			p_next_block = &_spu_memList[counter_next];
			if ( (p_next_block->addr_area & 0x40000000) != 0 )
				break;
			addr_area = _spu_memList[counter_3].addr_area;
			if ( (p_next_block->addr_area & 0xFFFFFFF) < (_spu_memList[counter_3].addr_area & 0xFFFFFFF) )
			{
				_spu_memList[counter_3].addr_area = p_next_block->addr_area;
				size_area_1 = _spu_memList[counter_3].size_area;
				_spu_memList[counter_3].size_area = p_next_block->size_area;
				p_next_block->addr_area = addr_area;
				p_next_block->size_area = size_area_1;
			}
		}
	}
	for ( idx = 0; idx <= _spu_AllocLastNum; idx += 1 )
	{
		if ( (_spu_memList[idx].addr_area & 0x40000000) != 0 )
		{
			break;
		}
		if ( _spu_memList[idx].addr_area == 0x2fffffff )
		{
			p_cur_block_3 = &_spu_memList[_spu_AllocLastNum];
			_spu_memList[idx].addr_area = p_cur_block_3->addr_area;
			size_area_2 = p_cur_block_3->size_area;
			_spu_AllocLastNum = idx;
			_spu_memList[idx].size_area = size_area_2;
			break;
		}
	}
	for ( alloc_last_num_1 = _spu_AllocLastNum - 1; alloc_last_num_1 >= 0; alloc_last_num_1 -= 1 )
	{
		int alloc_last_num_2;
		libspu2_malloc_t *p_prev_block;

		p_prev_block = &_spu_memList[alloc_last_num_1];
		if ( (p_prev_block->addr_area & 0x80000000) == 0 )
			break;
		alloc_last_num_2 = _spu_AllocLastNum;
		p_prev_block->addr_area = (p_prev_block->addr_area & 0xFFFFFFF) | 0x40000000;
		size_area_3 = p_prev_block->size_area;
		_spu_AllocLastNum = alloc_last_num_1;
		p_prev_block->size_area = size_area_3 + _spu_memList[alloc_last_num_2].size_area;
	}
}

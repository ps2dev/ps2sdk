/*
# _____     ___ ____     ___ ____
#  ____|   |    ____|   |        | |____|
# |     ___|   |____ ___|    ____| |    \    PS2DEV Open Source Project.
#-----------------------------------------------------------------------
# Copyright ps2dev - http://www.ps2dev.org
# Licenced under Academic Free License version 2.0
# Review ps2sdk README & LICENSE files for further details.
*/

#include "sysmem.h"
#include "xsysmem.h"

extern struct irx_export_table _exp_sysmem;

#ifdef _IOP
IRX_ID("System_Memory_Manager", 2, 3);
#endif
// Based on the module from SCE SDK 3.1.0.

static sysmem_internals_t sysmem_internals;
static KprintfHandler_t *kprintf_cb;
static void *kprintf_cb_userdata;

extern int sysmem_reinit(void);
static int cCpuSuspendIntr(int *state);
static int cCpuResumeIntr(int state);
static int allocSysMemory_internal(int flags, int size, void *mem);
static int freeSysMemory_internal(void *ptr);
static void updateSmemCtlBlk(void);
static sysmem_alloc_element_t *search_block(void *a1);

typedef struct intrman_callbacks_
{
	int (*cbCpuSuspendIntr)(int *state);
	int (*cbCpuResumeIntr)(int state);
	// cppcheck-suppress unusedStructMember
	int (*cbQueryIntrContext)(void);
} intrman_callbacks_t;

int _start(unsigned int memsize)
{
	if ( memsize > 0x7FFF00 )
		memsize = 0x7FFF00;
	sysmem_internals.alloclist = (sysmem_alloc_table_t *)0x401100;
	sysmem_internals.memsize = memsize & 0xFFFFFF00;
	sysmem_internals.intr_suspend_tbl = 0;
	sysmem_internals.allocation_count = 0;
	sysmem_internals.smemupdate_cur = 0;
	if ( (memsize & 0xFFFFFF00) >= 0x401200 )
		return sysmem_reinit();
	sysmem_internals.alloclist = 0;
	return 0;
}

int sysmem_reinit(void)
{
	sysmem_alloc_table_t *alloclist;
	sysmem_alloc_table_t *v1;
	int v2;
	int v3;
	sysmem_alloc_table_t *v4;
	sysmem_alloc_element_t *v5;
	int memsize;
	unsigned int info;
	const sysmem_alloc_table_t *v8;
	const sysmem_alloc_element_t *list;
	int result;

	alloclist = sysmem_internals.alloclist;
	if ( !sysmem_internals.alloclist )
		return 0;
	v1 = sysmem_internals.alloclist;
	sysmem_internals.alloclist->next = 0;
	v2 = 0;
	v3 = 12;
	v4 = alloclist;
	do
	{
		v5 = (sysmem_alloc_element_t *)((char *)v1 + v3);
		v3 += 8;
		v4->list[0].next = v5;
		v4->list[0].info = 0;
		++v2;
		v4 = (sysmem_alloc_table_t *)((char *)v4 + 8);
	} while ( v2 < 31 );
	memsize = sysmem_internals.memsize;
	info = v1->list[0].info;
	v1->list[30].next = 0;
	v1->list[0].info = (info & 0x1FFFF) | ((memsize / 256) << 17);
	sysmem_internals.allocation_count = 1;
	if (
		AllocSysMemory(0, (int)sysmem_internals.alloclist, 0)
		|| (v8 = (sysmem_alloc_table_t *)AllocSysMemory(0, 0xFC, 0), list = v8->list, v8 != sysmem_internals.alloclist) )
	{
		sysmem_internals.alloclist = 0;
		return 0;
	}
	result = 0;
	if ( list )
	{
		unsigned int v11;

		while ( 1 )
		{
			v11 = list->info;
			if ( (v11 & 1) == 0 )
				break;
			list = list->next;
			if ( !list )
				return 0;
		}
		return (u16)v11 >> 1 << 8;
	}
	return result;
}

u32 QueryMemSize()
{
	if ( sysmem_internals.alloclist )
		return sysmem_internals.memsize;
	else
		return 0;
}

u32 QueryMaxFreeMemSize()
{
	unsigned int v0;
	const sysmem_alloc_element_t *list;
	int state;

	v0 = 0;
	if ( !sysmem_internals.alloclist )
		return 0;
	cCpuSuspendIntr(&state);
	list = sysmem_internals.alloclist->list;
	if ( sysmem_internals.alloclist != (sysmem_alloc_table_t *)-4 )
	{
		do
		{
			if ( (list->info & 1) == 0 && v0 < list->info >> 17 )
				v0 = list->info >> 17;
			list = list->next;
		} while ( list );
	}
	cCpuResumeIntr(state);
	return v0 << 8;
}

u32 QueryTotalFreeMemSize()
{
	int v0;
	const sysmem_alloc_element_t *list;
	int state;

	v0 = 0;
	if ( !sysmem_internals.alloclist )
		return 0;
	cCpuSuspendIntr(&state);
	list = sysmem_internals.alloclist->list;
	if ( sysmem_internals.alloclist != (sysmem_alloc_table_t *)-4 )
	{
		do
		{
			unsigned int info;

			info = list->info;
			if ( (info & 1) == 0 )
				v0 += info >> 17;
			list = list->next;
		} while ( list );
	}
	cCpuResumeIntr(state);
	return v0 << 8;
}

void *AllocSysMemory(int mode, int size, void *ptr)
{
	void *v6;
	int state;

	if ( !sysmem_internals.alloclist || (unsigned int)mode >= 3 )
		return 0;
	cCpuSuspendIntr(&state);
	v6 = (void *)allocSysMemory_internal(mode, size, ptr);
	updateSmemCtlBlk();
	cCpuResumeIntr(state);
	return v6;
}

int FreeSysMemory(void *ptr)
{
	const sysmem_alloc_table_t *alloclist;
	int v4;
	int state;

	alloclist = sysmem_internals.alloclist;
	while ( alloclist && ptr != alloclist )
	{
		alloclist = alloclist->next;
	}
	if ( alloclist )
	{
		return -1;
	}
	cCpuSuspendIntr(&state);
	v4 = freeSysMemory_internal(ptr);
	if ( !v4 )
		updateSmemCtlBlk();
	cCpuResumeIntr(state);
	return v4;
}

void *QueryBlockTopAddress(void *address)
{
	int v2;
	const sysmem_alloc_element_t *v3;
	int state;

	v2 = -1;
	cCpuSuspendIntr(&state);
	v3 = search_block(address);
	if ( v3 )
	{
		unsigned int info;

		info = v3->info;
		if ( (info & 1) != 0 )
			v2 = (u16)info >> 1 << 8;
		else
			v2 = ((u16)info >> 1 << 8) + 0x80000000;
	}
	cCpuResumeIntr(state);
	return (void *)v2;
}

int QueryBlockSize(void *address)
{
	int v2;
	const sysmem_alloc_element_t *v3;
	int state;

	v2 = -1;
	cCpuSuspendIntr(&state);
	v3 = search_block(address);
	if ( v3 )
	{
		unsigned int info;

		info = v3->info;
		v2 = info >> 17 << 8;
		if ( (info & 1) == 0 )
			v2 |= 0x80000000;
	}
	cCpuResumeIntr(state);
	return v2;
}

void GetSysMemoryInfo(int flag, sysmem_info_t *info)
{
	sysmem_alloc_table_t *table_info;
	void *memsize;
	sysmem_alloc_table_t *v6;
	u32 v7;
	const sysmem_alloc_table_t *alloclist;
	sysmem_alloc_table_t *next;
	int state;

	cCpuSuspendIntr(&state);
	if ( flag )
	{
		info->meminfo.allocation_count = sysmem_internals.allocation_count;
		info->meminfo.memsize = sysmem_internals.memsize;
		info->meminfo.memlist_last = sysmem_internals.smemupdate_cur;
		info->meminfo.memlist_first = (sysmem_alloc_table_t *)sysmem_internals.alloclist->list;
		cCpuResumeIntr(state);
		return;
	}
	if ( info->meminfo.memlist_last != sysmem_internals.smemupdate_cur )
	{
		info->blockinfo.block_address = (void *)-1;
		info->blockinfo.flags_memsize = -1;
		info->blockinfo.table_info = 0;
		cCpuResumeIntr(state);
		return;
	}
	table_info = info->blockinfo.table_info;
	if ( !table_info )
	{
		memsize = (void *)sysmem_internals.memsize;
		info->blockinfo.flags_memsize = 1;
		info->blockinfo.block_address = memsize;
		cCpuResumeIntr(state);
		return;
	}
	v6 = info->blockinfo.table_info;
	info->blockinfo.block_address = (void *)((((unsigned int)table_info->list[0].next >> 1) & 0x7FFF) << 8);
	v7 = (unsigned int)v6->list[0].next >> 17 << 8;
	info->blockinfo.flags_memsize = v7;
	if ( ((int)v6->list[0].next & 1) != 0 )
	{
		alloclist = sysmem_internals.alloclist;
		while ( alloclist && alloclist != info->blockinfo.block_address )
		{
			alloclist = alloclist->next;
		}
		if ( alloclist )
		{
			info->blockinfo.flags_memsize |= 2u;
		}
	}
	else
	{
		info->blockinfo.flags_memsize |= 1;
	}
	next = info->blockinfo.table_info->next;
	info->blockinfo.table_info = next;
	if ( !((unsigned int)next->list[0].next >> 17) )
		info->blockinfo.table_info = 0;
	cCpuResumeIntr(state);
}

sysmem_internals_t *GetSysmemInternalData(void)
{
	return &sysmem_internals;
}

static int allocSysMemory_internal(int flags, int size, void *mem)
{
	unsigned int v3;
	int result;
	sysmem_alloc_element_t *i;
	unsigned int v6;
	unsigned int v7;
	sysmem_alloc_element_t *list;
	sysmem_alloc_element_t *v9;
	s16 v12;
	sysmem_alloc_element_t *next;
	int v16;
	unsigned int v17;
	unsigned int v18;
	unsigned int v19;
	unsigned int v20;
	int v21;
	int v22;
	unsigned int v23;
	unsigned int v24;
	sysmem_alloc_element_t *v25;
	sysmem_alloc_element_t *v26;
	unsigned int v27;
	int v28;
	unsigned int v31;
	unsigned int v32;
	unsigned int v35;
	unsigned int v36;

	v31 = 0;
	v3 = (unsigned int)(size + 255) >> 8;
	result = 0;
	if ( !v3 )
		return result;
	if ( flags == 1 )
	{
		list = sysmem_internals.alloclist->list;
		v9 = 0;
		if ( sysmem_internals.alloclist != (sysmem_alloc_table_t *)-4 )
		{
			do
			{
				unsigned int info;

				info = list->info;
				if ( (info & 1) == 0 && info >> 17 >= v3 )
					v9 = list;
				list = list->next;
			} while ( list );
		}
		i = v9;
		result = 0;
		if ( v9 )
		{
			v7 = v9->info;
			if ( v7 >> 17 != v3 )
			{
				unsigned int v11;
				int v13;
				int v14;

				++sysmem_internals.allocation_count;
				v11 = (v9->info & 0x1FFFF) | (((v9->info >> 17) - v3) << 17);
				v12 = (v11 >> 1) & 0x7FFF;
				i->info = v11;
				v11 >>= 17;
				v13 = (v31 & 0x10001) | (u16)(2 * ((v12 + v11) & 0x7FFF)) | (v3 << 17) | 1;
				v14 = (u16)((((u8 *)&v31)[0] & 1) | (2 * ((v12 + v11) & 0x7FFF)) | 1) >> 1;
				v31 = v13;
				next = i->next;
				v16 = v14 << 8;
				result = v16;
				if ( v31 >> 17 )
				{
					do
					{
						v36 = next->info;
						next->info = v31;
						v31 = v36;
						next = next->next;
						result = v16;
					} while ( v36 >> 17 );
				}
				return result;
			}
			i->info = v7 | 1;
			return (((v7 | 1) >> 1) & 0x7FFF) << 8;
		}
	}
	else if ( flags >= 2 )
	{
		result = 0;
		if ( flags == 2 )
		{
			result = 0;
			if ( (((uiptr)mem) & 0xFF) == 0 )
			{
				v17 = (unsigned int)mem >> 8;
				for ( i = sysmem_internals.alloclist->list;; i = i->next )
				{
					result = 0;
					if ( !i )
						break;
					v18 = i->info;
					v19 = (u16)v18 >> 1;
					result = 0;
					if ( v17 < v19 )
						break;
					if ( (v18 & 1) == 0 && v19 + (v18 >> 17) >= ((unsigned int)mem >> 8) + v3 )
					{
						if ( ((i->info >> 1) & 0x7FFF) < v17 )
						{
							++sysmem_internals.allocation_count;
							v20 = i->info;
							v21 = v20 & 0x1FFFF;
							v22 = (u16)v20 >> 1;
							v20 >>= 17;
							v23 = v22 + v20 - v17;
							v24 = v21 | ((v20 - v23) << 17);
							i->info = v24;
							v32 = (u16)(2 * ((((v24 >> 1) & 0x7FFF) + (v24 >> 17)) & 0x7FFF)) | (v23 << 17);
							v25 = i->next;
							v26 = v25;
							if ( v32 >> 17 )
							{
								do
								{
									v35 = v25->info;
									v25->info = v32;
									v32 = v35;
									v25 = v25->next;
								} while ( v35 >> 17 );
							}
							i = v26;
						}
						v7 = i->info;
						if ( v7 >> 17 != v3 )
						{
							++sysmem_internals.allocation_count;
							v27 = i->info;
							i->info = (v27 & 0x1FFFE) | 1 | (v3 << 17);
							v28 = 2 * ((((v27 >> 1) & 0x7FFF) + (u16)v3) & 0x7FFF);
							v31 = (v27 & 0x10001) | (v28 & 0x1FFFF) | (((((v27 & 0xFFFF0001) | v28) >> 17) - v3) << 17);
							next = i->next;
							v16 = (u16)v27 >> 1 << 8;
							result = v16;
							if ( v31 >> 17 )
							{
								do
								{
									v36 = next->info;
									next->info = v31;
									v31 = v36;
									next = next->next;
									result = v16;
								} while ( v36 >> 17 );
							}
							return result;
						}
						i->info = v7 | 1;
						return (((v7 | 1) >> 1) & 0x7FFF) << 8;
					}
				}
			}
		}
	}
	else
	{
		result = 0;
		if ( !flags )
		{
			i = sysmem_internals.alloclist->list;
			result = 0;
			if ( sysmem_internals.alloclist != (sysmem_alloc_table_t *)-4 )
			{
				do
				{
					v6 = i->info;
					if ( (v6 & 1) == 0 && v6 >> 17 >= v3 )
						break;
					i = i->next;
				} while ( i );
				result = 0;
				if ( i )
				{
					v7 = i->info;
					if ( v7 >> 17 != v3 )
					{
						++sysmem_internals.allocation_count;
						v27 = i->info;
						i->info = (v27 & 0x1FFFE) | 1 | (v3 << 17);
						v28 = 2 * ((((v27 >> 1) & 0x7FFF) + (u16)v3) & 0x7FFF);
						v31 = (v27 & 0x10001) | (v28 & 0x1FFFF) | (((((v27 & 0xFFFF0001) | v28) >> 17) - v3) << 17);
						next = i->next;
						v16 = (u16)v27 >> 1 << 8;
						result = v16;
						if ( v31 >> 17 )
						{
							do
							{
								v36 = next->info;
								next->info = v31;
								v31 = v36;
								next = next->next;
								result = v16;
							} while ( v36 >> 17 );
						}
						return result;
					}
					i->info = v7 | 1;
					return (((v7 | 1) >> 1) & 0x7FFF) << 8;
				}
			}
		}
	}
	return result;
}

static int freeSysMemory_internal(void *ptr)
{
	unsigned int v2;
	sysmem_alloc_element_t *v4;
	sysmem_alloc_element_t *v5;
	unsigned int v7;
	int v8;
	sysmem_alloc_element_t *v9;
	const sysmem_alloc_element_t *next;
	int v12;
	const sysmem_alloc_element_t *i;

	v2 = (unsigned int)ptr >> 8;
	if ( (((uiptr)ptr) & 0xFF) != 0 )
		return -1;
	v4 = &sysmem_internals.alloclist->list[2];
	v5 = 0;
	if ( sysmem_internals.alloclist == (sysmem_alloc_table_t *)-20 )
		return -1;
	do
	{
		unsigned int info;

		info = v4->info;
		if ( info >> 17 && (u16)info >> 1 == v2 )
			break;
		v5 = v4;
		v4 = v4->next;
	} while ( v4 );
	if ( !v4 )
		return -1;
	v7 = v4->info;
	v8 = 0;
	if ( (v7 & 1) == 0 )
		return -1;
	v9 = 0;
	next = v4->next;
	v4->info = v7 & 0xFFFFFFFE;
	if ( v4->next != 0 )
	{
		unsigned int v11;

		v11 = next->info;
		if ( v11 >> 17 )
		{
			if ( (v11 & 1) == 0 )
			{
				v8 = 1;
				--sysmem_internals.allocation_count;
				v9 = v4->next;
				v4->info = (v4->info & 0x1FFFF) | (((v4->info >> 17) + (v4->next->info >> 17)) << 17);
			}
		}
	}
	if ( v5 && (v5->info & 1) == 0 )
	{
		v9 = v4;
		++v8;
		--sysmem_internals.allocation_count;
		v5->info = (v5->info & 0x1FFFF) | (((v5->info >> 17) + (v4->info >> 17)) << 17);
	}
	v12 = v8 - 1;
	if ( v8 == 0 )
		return 0;
	for ( i = v9; v12 != -1; --v12 )
		i = i->next;
	while ( i )
	{
		v9->info = i->info;
		i = i->next;
		v9 = v9->next;
	}
	return 0;
}

static void updateSmemCtlBlk(void)
{
	sysmem_alloc_table_t *alloclist;
	sysmem_alloc_table_t *next;
	sysmem_alloc_element_t *v6;
	sysmem_alloc_table_t *v7;

	alloclist = sysmem_internals.alloclist;
	++sysmem_internals.smemupdate_cur;
	if ( sysmem_internals.alloclist->next )
	{
		do
			alloclist = alloclist->next;
		while ( alloclist->next );
	}
	if ( alloclist->list[27].info >> 17 )
	{
		int v1;

		v1 = allocSysMemory_internal(0, 256, 0);
		alloclist->next = (sysmem_alloc_table_t *)v1;
		if ( v1 )
		{
			unsigned int v2;
			int v4;
			sysmem_alloc_table_t *v5;

			v2 = 0;
			alloclist->list[30].next = (sysmem_alloc_element_t *)(v1 + 4);
			next = alloclist->next;
			v4 = 12;
			v5 = next;
			next->next = 0;
			do
			{
				v6 = (sysmem_alloc_element_t *)((char *)next + v4);
				v4 += 8;
				v5->list[0].next = v6;
				v5->list[0].info = 0;
				++v2;
				v5 = (sysmem_alloc_table_t *)((char *)v5 + 8);
			} while ( v2 < 0x1F );
			next->list[30].next = 0;
		}
	}
	v7 = sysmem_internals.alloclist;
	if ( sysmem_internals.alloclist->next )
	{
		sysmem_alloc_table_t *v8;

		while ( v7->next->next )
			v7 = v7->next;
		v8 = v7->next;
		if ( v7->next )
		{
			if ( !(v7->list[27].info >> 17) )
			{
				v7->list[30].next = 0;
				v7->next = 0;
				freeSysMemory_internal(v8);
			}
		}
	}
}

static sysmem_alloc_element_t *search_block(void *a1)
{
	sysmem_alloc_element_t *list;

	list = sysmem_internals.alloclist->list;
	if ( sysmem_internals.alloclist != (sysmem_alloc_table_t *)-4 )
	{
		do
		{
			unsigned int info;
			int v3;

			info = list->info;
			v3 = (u16)info >> 1;
			if ( (unsigned int)a1 >= (unsigned int)(v3 << 8) && (unsigned int)a1 < (v3 + (info >> 17)) << 8 )
				break;
			list = list->next;
		} while ( list );
	}
	return list;
}

static int cCpuSuspendIntr(int *state)
{
	intrman_callbacks_t *intrman_callbacks = (intrman_callbacks_t *)(sysmem_internals.intr_suspend_tbl);
	if ( intrman_callbacks && intrman_callbacks->cbCpuSuspendIntr )
		return intrman_callbacks->cbCpuSuspendIntr(state);
	else
		return 0;
}

static int cCpuResumeIntr(int state)
{
	intrman_callbacks_t *intrman_callbacks = (intrman_callbacks_t *)(sysmem_internals.intr_suspend_tbl);
	if ( intrman_callbacks && intrman_callbacks->cbCpuResumeIntr )
		return intrman_callbacks->cbCpuResumeIntr(state);
	else
		return 0;
}

#if 0
static int cQueryIntrContext(void)
{
	intrman_callbacks_t *intrman_callbacks = (intrman_callbacks_t *)(sysmem_internals.intr_suspend_tbl);
	if ( intrman_callbacks && intrman_callbacks->cbQueryIntrContext )
		return intrman_callbacks->cbQueryIntrContext();
	else
		return 0;
}
#endif

int Kprintf(const char *format, ...)
{
	if ( kprintf_cb )
	{
		int ret;
		va_list va;

		va_start(va, format);
		ret = kprintf_cb(kprintf_cb_userdata, format, va);
		va_end(va);
		return ret;
	}
	return 0;
}

void KprintfSet(KprintfHandler_t *new_cb, void *context)
{
	if ( kprintf_cb && new_cb )
	{
		new_cb(context, (const char *)Kprintf(NULL), NULL);
	}
	kprintf_cb = new_cb;
	kprintf_cb_userdata = context;
}

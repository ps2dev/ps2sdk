/*
# _____     ___ ____     ___ ____
#  ____|   |    ____|   |        | |____|
# |     ___|   |____ ___|    ____| |    \    PS2DEV Open Source Project.
#-----------------------------------------------------------------------
# Copyright ps2dev - http://www.ps2dev.org
# Licenced under Academic Free License version 2.0
# Review ps2sdk README & LICENSE files for further details.
*/

#include "vblank.h"
#include "irx_imports.h"
#include "kerr.h"

extern struct irx_export_table _exp_vblank;

#ifdef _IOP
IRX_ID("Vblank_service", 1, 1);
#endif
// Based on the module from SCE SDK 1.3.4.

static vblank_internals_t vblank_internals;

static int irq_vblank_interrupt_handler(void *userdata);
static int irq_evblank_interrupt_handler(void *userdata);
static int vblank_handler_base_beginning(void *userdata);
static int vblank_handler_base_end(void *userdata);
static void linked_list_set_self(vblank_ll_t *ll);
static int linked_list_next_is_self(const vblank_ll_t *ll);
static void linked_list_remove(vblank_ll_t *ll);
static void linked_list_add_after(vblank_ll_t *ll1, vblank_ll_t *ll2);

int _start(int ac, char **av)
{
	unsigned int i;
	iop_event_t evfp;
	int state;

	(void)ac;
	(void)av;

	CpuSuspendIntr(&state);
	if ( RegisterLibraryEntries(&_exp_vblank) )
	{
		CpuResumeIntr(state);
		return 1;
	}

	memset(&vblank_internals, 0, sizeof(vblank_internals));
	linked_list_set_self(&vblank_internals.list_00);
	linked_list_set_self(&vblank_internals.list_11);
	linked_list_set_self(&vblank_internals.list_free);
	for ( i = 0; i < (sizeof(vblank_internals.list_items) / sizeof(vblank_internals.list_items[0])); i += 1 )
	{
		linked_list_add_after(&vblank_internals.list_free, &(vblank_internals.list_items[i].ll));
	}
	evfp.bits = 0;
	evfp.attr = EA_MULTI;
	evfp.option = 0;
	vblank_internals.ef = CreateEventFlag(&evfp);
	RegisterVblankHandler(0, 128, vblank_handler_base_beginning, &vblank_internals);
	RegisterVblankHandler(1, 128, vblank_handler_base_end, &vblank_internals);
	RegisterIntrHandler(IOP_IRQ_VBLANK, 1, irq_vblank_interrupt_handler, &vblank_internals);
	RegisterIntrHandler(IOP_IRQ_EVBLANK, 1, irq_evblank_interrupt_handler, &vblank_internals);
	EnableIntr(IOP_IRQ_VBLANK);
	EnableIntr(IOP_IRQ_EVBLANK);
	CpuResumeIntr(state);
	return 0;
}

vblank_internals_t *GetVblankInternalData(void)
{
	return &vblank_internals;
}

int RegisterVblankHandler(int startend, int priority, int (*handler)(void *userdata), void *arg)
{
	const vblank_ll_t *list;
	vblank_ll_t *item;
	vblank_item_t *item_new;
	int state;

	if ( QueryIntrContext() != 0 )
	{
		return KE_ILLEGAL_CONTEXT;
	}
	CpuSuspendIntr(&state);
	if ( linked_list_next_is_self(&vblank_internals.list_free) )
	{
		CpuResumeIntr(state);
		return KE_NO_MEMORY;
	}
	list = &vblank_internals.list_11;
	if ( !startend )
		list = &vblank_internals.list_00;
	item = list->next;
	if ( list->next != list )
	{
		do
		{
			const vblank_item_t *item_vblank;

			item_vblank = (vblank_item_t *)item;
			if ( item_vblank->callback == handler )
			{
				CpuResumeIntr(state);
				return KE_FOUND_HANDLER;
			}
			item = item->next;
		} while ( item != list );
		item = list->next;
	}
	for ( ; item != list; item = item->next )
	{
		const vblank_item_t *item_vblank;

		item_vblank = (vblank_item_t *)item;
		if ( priority < item_vblank->priority )
			break;
	}
	item_new = (vblank_item_t *)vblank_internals.list_free.next;
	linked_list_remove(vblank_internals.list_free.next);
	item_new->callback = handler;
	item_new->userdata = arg;
	item_new->priority = priority;
	linked_list_add_after(item, &item_new->ll);
	CpuResumeIntr(state);
	return 0;
}

// cppcheck-suppress constParameterPointer
int ReleaseVblankHandler(int startend, int (*handler)(void *userdata))
{
	const vblank_ll_t *list;
	vblank_ll_t *item;
	int state;

	if ( QueryIntrContext() != 0 )
	{
		return KE_ILLEGAL_CONTEXT;
	}
	CpuSuspendIntr(&state);
	list = &vblank_internals.list_11;
	if ( !startend )
		list = &vblank_internals.list_00;
	item = list->next;
	if ( list->next == list )
	{
		CpuResumeIntr(state);
		return KE_NOTFOUND_HANDLER;
	}
	{
		const vblank_item_t *item_vblank;

		item_vblank = (vblank_item_t *)item;
		while ( item_vblank->callback != handler )
		{
			item = item->next;
			item_vblank = (vblank_item_t *)item;
			if ( item == list )
			{
				CpuResumeIntr(state);
				return KE_NOTFOUND_HANDLER;
			}
		}
	}
	linked_list_remove(item);
	linked_list_add_after(&vblank_internals.list_free, item);
	CpuResumeIntr(state);
	return 0;
}

static int irq_vblank_interrupt_handler(void *userdata)
{
	int item_count;
	vblank_ll_t *item;
	vblank_ll_t *item_tmp;
	vblank_internals_t *vbi;

	vbi = (vblank_internals_t *)userdata;
	item_count = vbi->item_count;
	if ( !item_count )
	{
		iSetEventFlag(GetSystemStatusFlag(), 0x200);
		item_count = vbi->item_count;
	}
	item = vbi->list_00.next;
	vbi->item_count = item_count + 1;
	if ( item != &vbi->list_00 )
	{
		do
		{
			vblank_item_t *item_vblank;

			item_vblank = (vblank_item_t *)item;
			item_tmp = item->next;
			if ( item_vblank->callback(item_vblank->userdata) == 0 )
			{
				linked_list_remove(item);
				linked_list_add_after(&vbi->list_free, item);
			}
			item = item_tmp;
		} while ( item_tmp != &vbi->list_00 );
	}
	return 1;
}

static int irq_evblank_interrupt_handler(void *userdata)
{
	vblank_ll_t *item;
	const vblank_ll_t *list_tail;
	vblank_ll_t *item_tmp;
	vblank_internals_t *vbi;

	vbi = (vblank_internals_t *)userdata;
	item = vbi->list_11.next;
	list_tail = &vbi->list_11;
	if ( item != list_tail )
	{
		do
		{
			vblank_item_t *item_vblank;

			item_vblank = (vblank_item_t *)item;
			item_tmp = item->next;
			if ( item_vblank->callback(item_vblank->userdata) == 0 )
			{
				linked_list_remove(item);
				linked_list_add_after(&vbi->list_free, item);
			}
			item = item_tmp;
		} while ( item_tmp != list_tail );
	}
	return 1;
}

#define EF_VBLANK_START 1
#define EF_VBLANK_END 4
#define EF_VBLANK 2
#define EF_NON_VBLANK 8

static int vblank_handler_base_beginning(void *userdata)
{
	vblank_internals_t *vbi;

	vbi = (vblank_internals_t *)userdata;
	iSetEventFlag(vbi->ef, EF_VBLANK_START);
	iSetEventFlag(vbi->ef, EF_VBLANK);
	iClearEventFlag(vbi->ef, ~(9));
	return 1;
}

static int vblank_handler_base_end(void *userdata)
{
	vblank_internals_t *vbi;

	vbi = (vblank_internals_t *)userdata;
	iSetEventFlag(vbi->ef, EF_VBLANK_END);
	iSetEventFlag(vbi->ef, EF_NON_VBLANK);
	iClearEventFlag(vbi->ef, ~(6));
	return 1;
}

void WaitVblankStart()
{
	WaitEventFlag(vblank_internals.ef, EF_VBLANK_START, WEF_OR, NULL);
}

void WaitVblankEnd()
{
	WaitEventFlag(vblank_internals.ef, EF_VBLANK_END, WEF_OR, NULL);
}

void WaitVblank()
{
	WaitEventFlag(vblank_internals.ef, EF_VBLANK, WEF_OR, NULL);
}

void WaitNonVblank()
{
	WaitEventFlag(vblank_internals.ef, EF_NON_VBLANK, WEF_OR, NULL);
}

static void linked_list_set_self(vblank_ll_t *ll)
{
	ll->next = ll;
	ll->prev = ll;
}

static int linked_list_next_is_self(const vblank_ll_t *ll)
{
	return ll->next == ll;
}

static void linked_list_remove(vblank_ll_t *ll)
{
	ll->next->prev = ll->prev;
	ll->prev->next = ll->next;
}

#if 0
// This function is unused
static int linked_list_is_circular(vblank_ll_t *ll)
{
	return ll->prev == ll->next;
}
#endif

static void linked_list_add_after(vblank_ll_t *ll1, vblank_ll_t *ll2)
{
	ll2->next = ll1;
	ll2->prev = ll1->prev;
	ll1->prev = ll2;
	ll2->prev->next = ll2;
}

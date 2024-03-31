/*
# _____     ___ ____     ___ ____
#  ____|   |    ____|   |        | |____|
# |     ___|   |____ ___|    ____| |    \    PS2DEV Open Source Project.
#-----------------------------------------------------------------------
# Copyright ps2dev - http://www.ps2dev.org
# Licenced under Academic Free License version 2.0
# Review ps2sdk README & LICENSE files for further details.
*/

#include "excepman.h"
#include "irx_imports.h"
#include "kerr.h"

extern struct irx_export_table _exp_excepman;

#ifdef _IOP
IRX_ID("Exception_Manager", 1, 1);
#endif
// Based on the module from SCE SDK 1.3.4.

static uiptr *exception_table;
static exception_handler_t exception_handlers[16];
static u32 *default_handler_funccode;
static exception_handler_t exception_list;

extern u32 exception_handler_shellcode_start[];
extern u32 exception_handler_shellcode_end[];
extern exception_handler_struct_t default_exception_handler;

static void update_exception_handler_table(void);
static exception_handler_t unlink_head_of_list(void);
static void link_to_head_of_list(exception_handler_t handler);
static void allocate_list(void);

int _start(int ac, char **av)
{
	unsigned int i;
	u32 *dst_ptr;

	(void)ac;
	(void)av;

	for ( i = 0; i < (sizeof(exception_handlers) / sizeof(exception_handlers[0])); i += 1 )
	{
		exception_handlers[i] = NULL;
	}
	default_handler_funccode = NULL;
	allocate_list();
	dst_ptr = 0;
	exception_table = (void *)0x440;
	// cppcheck-suppress comparePointers
	for ( i = 0; i < (unsigned int)(exception_handler_shellcode_end - exception_handler_shellcode_start); i += 1 )
	{
		u32 cur_instruction;

		cur_instruction = exception_handler_shellcode_start[i];
		if ( cur_instruction == 0x8F5A0000 || cur_instruction == 0x8F7B0000 )
		{
			cur_instruction |= (u16)(uiptr)exception_table;
		}
		// cppcheck-suppress nullPointer
		dst_ptr[i] = cur_instruction;
	}
	RegisterDefaultExceptionHandler(&default_exception_handler);
	RegisterLibraryEntries(&_exp_excepman);
	return 0;
}

int RegisterExceptionHandler(int exception, exception_handler_t handler)
{
	return RegisterPriorityExceptionHandler(exception, 2, handler);
}

int RegisterPriorityExceptionHandler(int exception, int priority, exception_handler_t handler)
{
	exception_handler_t exception_handler_new;
	int priority_masked;
	exception_handler_t *eh_ptr;
	exception_handler_t eh_cur1;
	exception_handler_t eh_cur2;

	if ( handler->next )
		return KE_EXPHANDLER_USED;
	if ( (unsigned int)exception >= (sizeof(exception_handlers) / sizeof(exception_handlers[0])) )
		return KE_ILLEGAL_EXPCODE;
	priority_masked = priority & 3;
	exception_handler_new = unlink_head_of_list();
	eh_ptr = &exception_handlers[exception];
	exception_handler_new->info = ((uiptr)handler & 0xFFFFFFFC) | priority_masked;
	eh_cur1 = *eh_ptr;
	if ( *eh_ptr )
	{
		do
		{
			eh_cur2 = *eh_ptr;
			if ( ((*eh_ptr)->info & 3) >= priority_masked )
				break;
			eh_ptr = (exception_handler_t *)*eh_ptr;
		} while ( eh_cur2->next );
		eh_cur1 = *eh_ptr;
	}
	exception_handler_new->next = eh_cur1;
	*eh_ptr = exception_handler_new;
	update_exception_handler_table();
	return 0;
}

int RegisterDefaultExceptionHandler(exception_handler_t handler)
{
	if ( handler->next )
		return KE_EXPHANDLER_USED;
	handler->next = (exception_handler_t)default_handler_funccode;
	default_handler_funccode = handler->funccode;
	update_exception_handler_table();
	return 0;
}

int ReleaseExceptionHandler(int exception, exception_handler_t handler)
{
	exception_handler_t exception_handler;
	exception_handler_t next;

	if ( (unsigned int)exception >= (sizeof(exception_handlers) / sizeof(exception_handlers[0])) )
		return KE_ILLEGAL_EXPCODE;
	exception_handler = (exception_handler_t)&exception_handlers[exception];
	if ( !exception_handler->next )
		return KE_EXPHANDLER_NOUSE;
	while ( 1 )
	{
		next = exception_handler->next;
		if ( (exception_handler_t)(exception_handler->next->info & 0xFFFFFFFC) == handler )
			break;
		exception_handler = exception_handler->next;
		if ( !next->next )
			return KE_EXPHANDLER_NOUSE;
	}
	exception_handler->next = next->next;
	*(u32 *)(next->info & 0xFFFFFFFC) = 0;
	link_to_head_of_list(next);
	update_exception_handler_table();
	return 0;
}

int ReleaseDefaultExceptionHandler(exception_handler_t handler)
{
	exception_handler_t cur_handler;

	if ( !default_handler_funccode )
		return KE_EXPHANDLER_NOUSE;
	// Unofficial: The original was referencing the funccode member, but we need the beginning of the struct
	cur_handler = (exception_handler_t)(((char *)default_handler_funccode) - 8);
	while ( 1 )
	{
		if ( cur_handler->funccode == handler->funccode )
			break;
		cur_handler = cur_handler->next;
		if ( !cur_handler )
			return KE_EXPHANDLER_NOUSE;
	}
	default_handler_funccode = cur_handler->next->funccode;
	cur_handler->next = 0;
	update_exception_handler_table();
	return 0;
}

void *GetExHandlersTable(void)
{
	return &exception_table;
}

static void update_exception_handler_table(void)
{
	exception_handler_t exception_handler;
	unsigned int i;

	for ( i = 0; i < (sizeof(exception_handlers) / sizeof(exception_handlers[0])); i += 1 )
	{
		exception_handler = exception_handlers[i];
		if ( exception_handler )
		{
			while ( exception_handler->next )
			{
				*(u32 *)(exception_handler->info & 0xFFFFFFFC) = (exception_handler->next->info & 0xFFFFFFFC) + 8;
				exception_handler = exception_handler->next;
			}
			*(u32 *)(exception_handler->info & 0xFFFFFFFC) = (uiptr)default_handler_funccode;
		}
	}
	for ( i = 0; i < (sizeof(exception_handlers) / sizeof(exception_handlers[0])); i += 1 )
	{
		exception_handler = exception_handlers[i];
		if ( exception_handler )
			exception_table[i] = (exception_handler->info & 0xFFFFFFFC) + 8;
		else
			exception_table[i] = (uiptr)default_handler_funccode;
	}
}

static exception_handler_t unlink_head_of_list(void)
{
	exception_handler_struct_t *exception_list_save;

	if ( !exception_list )
		allocate_list();
	exception_list_save = exception_list;
	if ( exception_list )
		exception_list = exception_list->next;
	return exception_list_save;
}

static void link_to_head_of_list(exception_handler_t handler)
{
	handler->next = exception_list;
	exception_list = handler;
}

static void allocate_list(void)
{
	unsigned int i;

	exception_list = (exception_handler_struct_t *)AllocSysMemory(0, 256, 0);
	for ( i = 0; i < 0x1F; i += 1 )
	{
		exception_list[i].next = &exception_list[i + 1];
	}
	exception_list[i].next = NULL;
}

// clang-format off
__asm__(
	"\t" ".set push" "\n"
	"\t" ".set noat" "\n"
	"\t" ".set noreorder" "\n"
	"\t" "exception_handler_shellcode_start:" "\n"
	"\t" "nop" "\n"
	"\t" "nop" "\n"
	"\t" "nop" "\n"
	"\t" "nop" "\n"
	"\t" "break       1" "\n"
	"\t" "break       1" "\n"
	"\t" "break       1" "\n"
	"\t" "break       1" "\n"
	"\t" "break       1" "\n"
	"\t" "break       1" "\n"
	"\t" "break       1" "\n"
	"\t" "break       1" "\n"
	"\t" "break       1" "\n"
	"\t" "break       1" "\n"
	"\t" "break       1" "\n"
	"\t" "break       1" "\n"
	"\t" "sw          $k0, 0x420($zero)" "\n" // memory[0x420] = k0
	"\t" "mfc0        $k1, $14" "\n" // k1 = EPC (k1 never saved!)
	"\t" "mfc0        $k0, $13" "\n" // k0 = Cause
	"\t" "sw          $k1, 0x424($zero)" "\n" // memory[0x424] = k1 = EPC
	"\t" "sw          $k0, 0x428($zero)" "\n" // memory[0x428] = k0 = Cause
	"\t" "mfc0        $k1, $12" "\n" // k1 = Status
	"\t" "mfc0        $k0, $7" "\n" // k0 = Breakpoint Control
	"\t" "sw          $k1, 0x42C($zero)" "\n" // memory[0x42C] = k1 = Status
	"\t" "sw          $k0, 0x430($zero)" "\n" // memory[0x430] = k0 = Breakpoint Control
	"\t" "addiu       $k1, $zero, 0x3C" "\n" // k1 = 0x3C
	"\t" "lw          $k1, 0x0($k1)" "\n" // k1 = exception_table[15] (this instruction gets modified to have exception_table address as immediate)
	"\t" "mtc0        $zero, $7" "\n" // Breakpoint Control = 0
	"\t" "jr          $k1" "\n" // (exception_table[15])()
	"\t" " nop" "\n"
	"\t" "nop" "\n"
	"\t" "nop" "\n"
	"\t" "sw          $at, 0x400($zero)" "\n" // memory[0x400] = at
	"\t" "sw          $k0, 0x410($zero)" "\n" // memory[0x410] = k0
	"\t" "mfc0        $k0, $14" "\n" // k0 = EPC
	"\t" "mfc0        $at, $12" "\n" // at = Status
	"\t" "sw          $k0, 0x404($zero)" "\n" // memory[0x404] = k0 = EPC
	"\t" "sw          $at, 0x408($zero)" "\n" // memory[0x408] = at = Status
	"\t" "mfc0        $k0, $13" "\n" // k0 = Cause
	"\t" "nop" "\n"
	"\t" "sw          $k0, 0x40C($zero)" "\n" // memory[0x40C] = k0 = Cause
	"\t" "andi        $k0, $k0, 0x3C" "\n" // k0 &= 0x3C
	"\t" "lw          $k0, 0x0($k0)" "\n" // k0 = exception_table[k0 / sizeof(exception_table[0])] (this instruction gets modified to have exception_table address as immediate)
	"\t" "nop" "\n"
	"\t" "jr          $k0" "\n" // (exception_table[(Cause & 0x3C) / sizeof(exception_table[0])])()
	"\t" " nop" "\n"
	"\t" "nop" "\n"
	"\t" "nop" "\n"
	"\t" "nop" "\n"
	"\t" "nop" "\n"
	"\t" "nop" "\n"
	"\t" "nop" "\n"
	"\t" "exception_handler_shellcode_end:" "\n"
	"\t" ".set pop" "\n"
);

__asm__(
	"\t" ".set push" "\n"
	"\t" ".set noat" "\n"
	"\t" ".set noreorder" "\n"
	"\t" "default_exception_handler:" "\n"
	"\t" ".word 0" "\n"
	"\t" ".word 0" "\n"
	"\t" "exception_handler_infloop:" "\n"
	"\t" "b           exception_handler_infloop" "\n"
	"\t" " nop" "\n"
	"\t" ".set pop" "\n"
);
// clang-format on

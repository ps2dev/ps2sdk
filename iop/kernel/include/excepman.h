/*
# _____     ___ ____     ___ ____
#  ____|   |    ____|   |        | |____|
# |     ___|   |____ ___|    ____| |    \    PS2DEV Open Source Project.
#-----------------------------------------------------------------------
# Copyright 2001-2004, ps2dev - http://www.ps2dev.org
# Licenced under Academic Free License version 2.0
# Review ps2sdk README & LICENSE files for further details.
*/

/**
 * @file
 * Definitions and imports for excepman
 */

#ifndef __EXCEPMAN_H__
#define __EXCEPMAN_H__

#include <types.h>
#include <irx.h>

#ifdef __cplusplus
extern "C" {
#endif

/* From any r3000's cop0 documentation */
/** External Interrupt */
#define IOP_EXCEPTION_INT	0
/** TLB Modification Exception */
#define IOP_EXCEPTION_MOD	1
/** TLB miss Exception (Load or instruction fetch) */
#define IOP_EXCEPTION_TLBL 2
/** TLB miss exception (Store) */
#define IOP_EXCEPTION_TLBS 3
/** Address Error Exception (Load or instruction fetch) */
#define IOP_EXCEPTION_ADEL 4
/** Address Error Exception (Store) */
#define IOP_EXCEPTION_ADES 5
/** Bus Error Exception (for Instruction Fetch) */
#define IOP_EXCEPTION_IBE  6
/** Bus Error Exception (for data Load or Store) */
#define IOP_EXCEPTION_DBE  7
/** SYSCALL Exception */
#define IOP_EXCEPTION_SYS  8
/** Breakpoint Exception */
#define IOP_EXCEPTION_BP   9
/** Reserved Instruction Exception */
#define IOP_EXCEPTION_RI  10
/** Co-Processor Unusable Exception */
#define IOP_EXCEPTION_CPU 11
/** Arithmetic Overflow Exception */
#define IOP_EXCEPTION_OVF 12
/** Reserved 13 */
#define IOP_EXCEPTION_R13 13
/** Reserved 14. This is FPE though, but won't be useful on IOP. */
#define IOP_EXCEPTION_R14 14
/** Hardware DeBug(aka "Hardware Breakpoint") */
#define IOP_EXCEPTION_HDB 15

typedef struct _exception_handler_struct_t
{
	struct _exception_handler_struct_t* next;
	int info;
	u32 funccode[];
} exception_handler_struct_t;

typedef exception_handler_struct_t* exception_handler_t;

void* GetExHandlersTable();

/** will call RegisterPriorityExceptionHandler with prio = 2 */
int RegisterExceptionHandler(int exception, exception_handler_t handler);
int RegisterPriorityExceptionHandler(int exception, int priority, exception_handler_t handler);
int RegisterDefaultExceptionHandler(exception_handler_t handler);
int ReleaseExceptionHandler(int exception, exception_handler_t handler);
int ReleaseDefaultExceptionHandler(exception_handler_t handler);

#define excepman_IMPORTS_start DECLARE_IMPORT_TABLE(excepman, 1, 2)
#define excepman_IMPORTS_end END_IMPORT_TABLE

#define I_GetExHandlersTable DECLARE_IMPORT(3, GetExHandlersTable)
#define I_RegisterExceptionHandler DECLARE_IMPORT(4, RegisterExceptionHandler)
#define I_RegisterPriorityExceptionHandler DECLARE_IMPORT(5, RegisterPriorityExceptionHandler)
#define I_RegisterDefaultExceptionHandler DECLARE_IMPORT(6, RegisterDefaultExceptionHandler)
#define I_ReleaseExceptionHandler DECLARE_IMPORT(7, ReleaseExceptionHandler)
#define I_ReleaseDefaultExceptionHandler DECLARE_IMPORT(8, ReleaseDefaultExceptionHandler)

#ifdef __cplusplus
}
#endif

#endif /* __EXCEPMAN_H__ */

/*
# _____     ___ ____     ___ ____
#  ____|   |    ____|   |        | |____|
# |     ___|   |____ ___|    ____| |    \    PS2DEV Open Source Project.
#-----------------------------------------------------------------------
# Copyright 2001-2004, ps2dev - http://www.ps2dev.org
# Licenced under Academic Free License version 2.0
# Review ps2sdk README & LICENSE files for further details.
#
# $Id$
# Definitions and imports for excepman
*/

#ifndef IOP_EXCEPMAN_H
#define IOP_EXCEPMAN_H

#include "types.h"
#include "irx.h"

#define excepman_IMPORTS_start DECLARE_IMPORT_TABLE(excepman, 1, 2)
#define excepman_IMPORTS_end END_IMPORT_TABLE

/* From any r3000's cop0 documentation */
// External Interrupt
#define EX_INT	0
// TLB Modification Exception
#define EX_MOD	1
// TLB miss Exception (Load or instruction fetch)
#define EX_TLBL 2
// TLB miss exception (Store)
#define EX_TLBS 3
// Address Error Exception (Load or instruction fetch)
#define EX_ADEL 4
// Address Error Exception (Store)
#define EX_ADES 5
// Bus Error Exception (for Instruction Fetch)
#define EX_IBE  6
// Bus Error Exception (for data Load or Store)
#define EX_DBE  7
// SYSCALL Exception
#define EX_SYS  8
// Breakpoint Exception
#define EX_BP   9
// Reserved Instruction Exception
#define EX_RI  10
// Co-Processor Unusable Exception
#define EX_CPU 11
// Arithmetic Overflow Exception
#define EX_OVF 12
// Reserved 13
#define EX_R13 13
// Reserved 14. This is FPE though, but won't be useful on IOP.
#define EX_R14 14
// What's this ... ?
#define EX_HDB 15

// does it really return void ?
typedef void (*exception_handler_t)(void);

// will call RegisterPriorityExceptionHandler with prio = 2
int RegisterExceptionHandler(int exception, exception_handler_t);
#define I_RegisterExceptionHandler DECLARE_IMPORT(4, RegisterExceptionHandler)
int RegisterPriorityExceptionHandler(int exception, int priority, exception_handler_t);
#define I_RegisterPriorityExceptionHandler DECLARE_IMPORT(5, RegisterPriorityExceptionHandler)
int RegisterDefaultExceptionHandler(exception_handler_t);
#define I_RegisterDefaultExceptionHandler DECLARE_IMPORT(6, RegisterDefaultExceptionHandler)
int ReleaseExceptionHandler(int exception, exception_handler_t);
#define I_ReleaseExceptionHandler DECLARE_IMPORT(7, ReleaseExceptionHandler)
int ReleaseDefaultExceptionHandler(exception_handler_t);
#define I_ReleaseDefaultExceptionHandler DECLARE_IMPORT(4, ReleaseDefaultExceptionHandler)


#endif /* IOP_EXCEPMAN_H */

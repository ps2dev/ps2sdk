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
 * Header used to force register names to a specific ABI
 */

#ifndef __AS_REG_COMPAT_H__
#define __AS_REG_COMPAT_H__

#ifdef _IOP
#define ABI_EABI64
#endif

#if !defined(ABI_EABI64) && !defined(ABI_N32)
// No ABI defined
#error "Must define ABI_EABI64 or ABI_N32"
#endif

#if defined(ABI_EABI64) && defined(ABI_N32)
// Both ABI's defined
#error "Must define ABI_EABI64 or ABI_N32"
#endif

#define zero 0 // Hardware zero
#define at   1 // Assembler temporary   Caller-saved
#define v0   2 // Function results      Caller-saved
#define v1   3 // Function results      Caller-saved
#define a0   4 // Subprogram arguments  Caller-saved
#define a1   5 // Subprogram arguments  Caller-saved
#define a2   6 // Subprogram arguments  Caller-saved
#define a3   7 // Subprogram arguments  Caller-saved

#ifdef ABI_EABI64
#define a4 INVALID_REG
#define a5 INVALID_REG
#define a6 INVALID_REG
#define a7 INVALID_REG
#define t0 8  // Temporaries           Caller-saved <- watch out!
#define t1 9  // Temporaries           Caller-saved <- watch out!
#define t2 10 // Temporaries           Caller-saved <- watch out!
#define t3 11 // Temporaries           Caller-saved <- watch out!
#define t4 12 // Temporaries           Caller-saved
#define t5 13 // Temporaries           Caller-saved
#define t6 14 // Temporaries           Caller-saved
#define t7 15 // Temporaries           Caller-saved
#endif

#ifdef ABI_N32
#define a4 8  // Subprogram arguments  Caller-saved
#define a5 9  // Subprogram arguments  Caller-saved
#define a6 10 // Subprogram arguments  Caller-saved
#define a7 11 // Subprogram arguments  Caller-saved
#define t0 12 // Temporaries           Caller-saved <- watch out!
#define t1 13 // Temporaries           Caller-saved <- watch out!
#define t2 14 // Temporaries           Caller-saved <- watch out!
#define t3 15 // Temporaries           Caller-saved <- watch out!
#define t4 INVALID_REG
#define t5 INVALID_REG
#define t6 INVALID_REG
#define t7 INVALID_REG
#endif

#define s0 16 // Saved                 Callee-saved
#define s1 17 // Saved                 Callee-saved
#define s2 18 // Saved                 Callee-saved
#define s3 19 // Saved                 Callee-saved
#define s4 20 // Saved                 Callee-saved
#define s5 21 // Saved                 Callee-saved
#define s6 22 // Saved                 Callee-saved
#define s7 23 // Saved                 Callee-saved
#define t8 24 // Temporary             Caller-saved
#define t9 25 // Temporary             Caller-saved
#define k0 26 // Reserved for kernel
#define k1 27 // Reserved for kernel
#define gp 28 // Global pointer        Callee-saved
#define sp 29 // Stack pointer         Callee-saved
#define fp 30 // Frame pointer         Callee-saved
#define ra 31 // Return address        Caller-saved

#endif /* __AS_REG_COMPAT_H__ */

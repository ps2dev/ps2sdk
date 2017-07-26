/*
# _____     ___ ____     ___ ____
#  ____|   |    ____|   |        | |____|
# |     ___|   |____ ___|    ____| |    \    PS2DEV Open Source Project.
#-----------------------------------------------------------------------
# Copyright (c) 2009 PS2DEV.org
# Licenced under Academic Free License version 2.0
# Review ps2sdk README & LICENSE files for further details.
*/

/**
 * @file
 * EE Co-processor 0(COP0) register definitions.
 */

#ifndef __EE_COP0_DEFS_H__
#define __EE_COP0_DEFS_H__

#define M_EE_GET_CAUSE_EXCODE(__cause) (((__cause) >> 2) & 0x1F)
#define M_EE_GET_CAUSE_EXC2(__cause) (((__cause) >> 16) & 0x7)

// EE COP0 Register name defs
#define EE_COP0_Index       $0
#define EE_COP0_Random      $1
#define EE_COP0_EntryLo0    $2
#define EE_COP0_EntryLo1    $3
#define EE_COP0_Context     $4
#define EE_COP0_Wired       $5
#define EE_COP0_BadVAddr    $8
#define EE_COP0_Count       $9
#define EE_COP0_EntryHi     $10
#define EE_COP0_Compare     $11
#define EE_COP0_Status      $12
#define EE_COP0_Cause       $13
#define EE_COP0_EPC         $14
#define EE_COP0_PRId        $15
#define EE_COP0_Config      $16
#define EE_COP0_BadPAddr    $23
#define EE_COP0_TagLo       $28
#define EE_COP0_TagHi       $29
#define EE_COP0_ErrorEPC    $30

// Bits for COP0 "Status" register.
#define EE_STATUS_IE    (1 << 0)
#define EE_STATUS_EXL   (1 << 1)
#define EE_STATUS_ERL   (1 << 2)

#define EE_EXC2_RST     (0)
#define EE_EXC2_NMI     (1)
#define EE_EXC2_PERF    (2)
#define EE_EXC2_DBG     (3)

// Bits for COP0 "Cause" register.
#define EE_CAUSE_BD        (1 << 31)
#define EE_CAUSE_BD2       (1 << 30)

/** Serial I/O pending flag */
#define EE_CAUSE_SIO       (1 << 12)

// Bits in EE Cop0 Breakpoint Control(BPC)

/* Instruction Address breakpoint Enable */
#define EE_BPC_IAE          (1 << 31)

/* Data Read breakpoint Enable */
#define EE_BPC_DRE         (1 << 30)

/* Data Write breakpoint Enable */
#define EE_BPC_DWE         (1 << 29)

/* Data Value breakpoint Enable */
#define EE_BPC_DVE         (1 << 28)

/* Instruction address breakpoint - User mode Enable */
#define EE_BPC_IUE         (1 << 26)

/* Instruction address breakpoint - Supervisor mode Enable */
#define EE_BPC_ISE         (1 << 25)

/* Instruction address breakpoint - Kernel mode Enable */
#define EE_BPC_IKE         (1 << 24)

/* Instruction address breakpoint - EXL mode Enable */
#define EE_BPC_IXE         (1 << 23)

/* Data breakpoint - User mode Enable */
#define EE_BPC_DUE         (1 << 21)

/* Data breakpoint - Supervisor mode Enable */
#define EE_BPC_DSE         (1 << 20)

/* Data breakpoint - Kernel mode Enable */
#define EE_BPC_DKE         (1 << 19)

/* Data breakpoint - EXL mode Enable */
#define EE_BPC_DXE         (1 << 18)

/* Instruction address breakpoint - Trigger generation Enable */
#define EE_BPC_ITE         (1 << 17)

/* Data breakpoint - Trigger generation Enable */
#define EE_BPC_DTE         (1 << 16)

/* Breakpoint Exception Disable */
#define EE_BPC_BED         (1 << 15)

/* Data Write Breakpoint establishment flag */
#define EE_BPC_DWB         (1 << 2)

/* Data Read Breakpoint establishment flag */
#define EE_BPC_DRB         (1 << 1)

/* Instruction Address Breakpoint */
#define EE_BPC_IAB         (1 << 0)

#endif /* __EE_COP0_DEFS_H__ */

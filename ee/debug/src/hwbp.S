#      
# _____     ___ ____     ___ ____
#  ____|   |    ____|   |        | |____|
# |     ___|   |____ ___|    ____| |    \    PS2DEV Open Source Project.
#-----------------------------------------------------------------------
# Copyright 2001-2004, ps2dev - http://www.ps2dev.org
# Licenced under Academic Free License version 2.0
# Review ps2sdk README & LICENSE files for further details.
#
# $Id$
# Hardware breakpoint functions
#

#include "as_reg_compat.h"

	.text
	.p2align 3

	.set push
	.set noreorder
	.set noat


	.global InitBPC
	.ent	InitBPC

InitBPC:
	sync.p

# Disable the exception handling
	li	$t0, (1 << 15)
	mtbpc	$t0

	// Clear out the registers
	mtiab	$0
	li	$t0, 0xffffffff
	mtiabm	$t0
	mtdab	$0
	mtdabm	$t0
	mtdvb	$0
	mtdvbm	$t0

	jr	$ra
	nop

	.end 	InitBPC

    .global SetInstructionBP
    .ent	SetInstructionBP

# Set the instruction breakpoint registers
# a0 = PC location
# a1 = PC Mask
# a2 = options
SetInstructionBP:

	sync.p	

	mfbpc	$t0
# Jump if top bit not set
	bgez	$t0, 1f
	nop

# Clear top bit
	li	$t1, (1 << 31)
	xor	$t0, $t0, $t1
	mtbpc	$t0

	sync.p
1:
	mtiab	$a0
	mtiabm	$a1
	mfbpc	$t0
# Clear flags relating to IAB
	li	$t1, ~((1 << 26) | (1 << 25) | (1 << 24) | (1 << 23) | (1 << 17) | (1 << 15) | 1)
   	and	$t0, $t0, $t1

# Set flags relating to IAB, only enable for user and supervisor modes
#	li  t1, (BPC_IAE | BPC_IUE | BPC_ISE)

# Clean up options to ensure we are only setting instruction stuff
	li  $t1, ((1 << 26) | (1 << 25) | (1 << 24) | (1 << 23))
	and $a2, $a2, $t1

# Or in options
	li	$t1, (1 << 31)
	or	$t1, $a2, $t1

# Or in original bpc
   	or	$t0, $t0, $t1
	mtbpc	$t0
	sync.p

	jr	$ra
	nop

	.end	SetInstructionBP

# Set a data address breakpoint
# a0 - Data address
# a1 - Data address mask
# a2 - options (e.g. BPC_DUE)
	.global SetDataAddrBP
	.ent	SetDataAddrBP
SetDataAddrBP:
	sync.l	

# Mask out any data address related bits
	mfbpc	$t0
	li	$t1, ~((1 << 30) | (1 << 29) | (1 << 28) | (1 << 21) |   \
	      	 (1 << 20) | (1 << 19) | (1 << 18) | (1 << 16) | \
			 (1 << 15) | (1 << 2) | (1 << 1))

   	and	$t0, $t0, $t1
	mtbpc	$t0
	sync.p

# Move in address and mask
	mtdab	$a0
	mtdabm	$a1

# Mask out all non data address bits
	li		$t1, ((1 << 30) | (1 << 29) | (1 << 21) | (1 << 20) | (1 << 19) | (1 << 18))
	and 	$t1, $a2, $t1

   	mfbpc	$t0
	or		$t0, $t0, $t1

	mtbpc	$t0
	sync.p

	jr	$ra
	nop

	.end	SetDataAddrBP

# Set a data value breakpoint
# a0 - Data address
# a1 - Data address mask
# a2 - Data value
# a3 - Data value mask
# t0 - options (e.g. BPC_DUE)
	.global SetDataValueBP
	.ent	SetDataValueBP
SetDataValueBP:
	sync.l	

# Mask out any data address related bits
	mfbpc	$t1
	li	$t2, ~((1 << 30) | (1 << 29) | (1 << 28) | (1 << 21) |   \
	      	 (1 << 20) | (1 << 19) | (1 << 18) | (1 << 16) | \
			 (1 << 15) | (1 << 2) | (1 << 1))

   	and	$t1, $t1, $t2
	mtbpc	$t1
	sync.p

# Move in address and mask
	mtdab	$a0
	mtdabm	$a1
	mtdvb   $a2
	mtdvbm  $a3

# Mask out all non data address bits
	li		$t2, ((1 << 30) | (1 << 29) | (1 << 21) | (1 << 20) | (1 << 19) | (1 << 18))
	and 	$t2, $t0, $t2
	li		$t0, (1 << 28) # Data value enable
	or		$t2, $t2, $t0

   	mfbpc	$t1
	or		$t1, $t1, $t2

	mtbpc	$t1
	sync.p

	jr	$ra
	nop

	.end	SetDataValueBP


# Get the current value of the BPC
	.global GetBPC
	.ent	GetBPC
GetBPC:
	sync.l
	mfbpc	$v0
	jr	$ra
	nop

	.end 	GetBPC

# Set the current value of the BPC
	.global SetBPC
	.ent	SetBPC
SetBPC:
	mtbpc	$a0
	sync.p
	jr		$ra
	nop

	.end SetBPC

# Get the current value of the Instruction address register
	.global GetIAB
	.ent GetIAB
GetIAB:
	mfiab	$v0
	jr		$ra
	nop

	.end GetIAB

# Get the current value of the Instruction address mask register
	.global GetIABM
	.ent GetIABM
GetIABM:
	mfiabm	$v0
	jr		$ra
	nop

	.end GetIABM

# Get the current value of the data address register
	.global GetDAB
	.ent GetDAB
GetDAB:
	mfdab	$v0
	jr		$ra
	nop

	.end GetDAB

# Get the current value of the data address mask register
	.global GetDABM
	.ent GetDABM
GetDABM:
	mfdabm	$v0
	jr		$ra
	nop

	.end GetDABM

# Get the current value of the data value register
	.global GetDVB
	.ent GetDVB
GetDVB:
	mfdvb	$v0
	jr		$ra
	nop

	.end GetDVB

# Get the current value of the data value mask register
	.global GetDVBM
	.ent GetDVBM
GetDVBM:
	mfdvbm	$v0
	jr		$ra
	nop

	.end GetDVB

# Set the current value of the Instruction address register
	.global SetIAB
	.ent SetIAB
SetIAB:
	mtiab	$a0
	jr		$ra
	nop

	.end SetIAB

# Set the current value of the Instruction address mask register
	.global SetIABM
	.ent SetIABM
SetIABM:
	mtiabm	$a0
	jr		$ra
	nop

	.end SetIABM

# Set the current value of the data address register
	.global SetDAB
	.ent SetDAB
SetDAB:
	mtdab	$a0
	jr		$ra
	nop

	.end GetDAB

# Set the current value of the data address mask register
	.global SetDABM
	.ent SetDABM
SetDABM:
	mtdabm	$a0
	jr		$ra
	nop

	.end SetDABM

# Set the current value of the data value register
	.global SetDVB
	.ent SetDVB
SetDVB:
	mtdvb	$a0
	jr		$ra
	nop

	.end SetDVB

# Set the current value of the data value mask register
	.global SetDVBM
	.ent SetDVBM
SetDVBM:
	mtdvbm	$a0
	jr		$ra
	nop

	.end SetDVB

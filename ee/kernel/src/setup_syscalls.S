# _____     ___ ____     ___ ____
#  ____|   |    ____|   |        | |____|
# |     ___|   |____ ___|    ____| |    \    PS2DEV Open Source Project.
#-----------------------------------------------------------------------
# Licenced under Academic Free License version 2.0
# Review ps2sdk README & LICENSE files for further details.

.text

.globl setup
.ent setup
setup:
	li $v1, 0x74
	syscall
	jr $ra
	nop
.end setup

.globl Copy
.ent Copy
Copy:
	li $v1, 0x5A
	syscall
	jr $ra
	nop
.end Copy

.globl GetEntryAddress
.ent GetEntryAddress
GetEntryAddress:
	li $v1, 0x5B
	syscall
	jr $ra
	nop
.end GetEntryAddress

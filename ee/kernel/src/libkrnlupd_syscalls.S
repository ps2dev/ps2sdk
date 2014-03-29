.text

#What did I learn? Never trust GCC... never EVER trust GCC.
#Never trust it to integrate your inline assembly code properly into the rest of the program on its own!

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


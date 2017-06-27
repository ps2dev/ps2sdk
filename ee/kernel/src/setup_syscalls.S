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

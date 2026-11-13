.pushsection .rodata

.align 4
init_vif_regs_12:	.word 0x01000404, 0x20000000, 0x00000000, 0x05000000
			.word 0x06000000, 0x03000000, 0x02000000, 0x04000000
.popsection

.pushsection .text

.globl GsResetPath
.ent GsResetPath
GsResetPath:
	li	$a3, 1
	la	$v0, 0x10003C10
	sw	$a3, ($v0)
	la	$v1, 0x10003C20
	li	$v0, 2
	move	$a0, $zero
	sw	$v0, 0($v1)
	sync
	cfc2	$a0, $28
	ori	$a0, 0x200
	ctc2	$a0, $28
	sync.p
	la	$a1, init_vif_regs_12
	la	$a2, 0x10005000
	lq	$a0, ($a1)
	la	$v1, 0x10003000
	sq	$a0, ($a2)
	lq	$v0, 16($a1)
	sq	$v0, ($a2)
.set push
.set noreorder
	jr	$ra
	sw	$a3, 0($v1)
.set pop
.end GsResetPath

.popsection

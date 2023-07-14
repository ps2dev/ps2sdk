
.set push
.set noat
.set noreorder

.text

.globl InvokeUserModeCallback
.ent InvokeUserModeCallback
InvokeUserModeCallback:
	#Also set status EXL=1 and KSU=01b (User Mode)
	#COP0 registers: EPC ($14), status ($12)
	lui $k0, %hi(dispatch_ra)
	sw $ra, %lo(dispatch_ra)($k0)
	lui $k0, %hi(dispatch_sp)
	sw $sp, %lo(dispatch_sp)($k0)
	mtc0 $a0, $14
	sync.p
	daddu $v1, $zero, $a1
	daddu $a0, $zero, $a2
	daddu $a1, $zero, $a3
	daddu $a2, $zero, $a4
	mfc0 $k0, $12
	ori $k0, $k0, 0x12
	mtc0 $k0, $12
	sync.p
	eret
.end InvokeUserModeCallback

.globl ResumeIntrDispatch
.ent ResumeIntrDispatch
ResumeIntrDispatch:
	#Clear IE and EXL, and set KSU=00b (Kernel Mode)
	#COP0 registers: EPC ($14), status ($12)
	mfc0 $at, $12
	addiu $k0, $zero, 0xFFE4
	and $at, $at, $k0
	mtc0 $at, $12
	sync.p
	lui $k0, %hi(dispatch_ra)
	lw $ra, %lo(dispatch_ra)($k0)
	lui $k0, %hi(dispatch_sp)
	jr $ra
	lw $sp, %lo(dispatch_sp)($k0)
.end ResumeIntrDispatch

.globl ChangeGP
.ent ChangeGP
ChangeGP:
	move	$v0, $gp
	jr	$ra
	move	$gp, $a0
.end ChangeGP

.globl SetGP
.ent SetGP
SetGP:
	jr	$ra
	move	$gp, $a0
.end SetGP

.globl GetGP
.ent GetGP
GetGP:
	jr	$ra
	move	$v0, $gp
.end GetGP

.set pop

.data
dispatch_ra: .quad 0
dispatch_sp: .quad 0

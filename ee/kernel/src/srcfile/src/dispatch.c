
#include <tamtypes.h>

__asm__
(
	"\t" ".set push" "\n"
	"\t" ".set noat" "\n"
	"\t" ".set noreorder" "\n"

	"\t" ".globl InvokeUserModeCallback" "\n"
	"\t" ".ent InvokeUserModeCallback" "\n"
	"\t" "InvokeUserModeCallback:" "\n"
	// Also set status EXL=1 and KSU=01b (User Mode)
	// COP0 registers: EPC ($14), status ($12)
	"\t" "\t" "lui $k0, %hi(dispatch_ra)" "\n"
	"\t" "\t" "sw $ra, %lo(dispatch_ra)($k0)" "\n"
	"\t" "\t" "lui $k0, %hi(dispatch_sp)" "\n"
	"\t" "\t" "sw $sp, %lo(dispatch_sp)($k0)" "\n"
	"\t" "\t" "mtc0 $a0, $14" "\n"
	"\t" "\t" "sync.p" "\n"
	"\t" "\t" "daddu $v1, $zero, $a1" "\n"
	"\t" "\t" "daddu $a0, $zero, $a2" "\n"
	"\t" "\t" "daddu $a1, $zero, $a3" "\n"
	"\t" "\t" "daddu $a2, $zero, $a4" "\n"
	"\t" "\t" "mfc0 $k0, $12" "\n"
	"\t" "\t" "ori $k0, $k0, 0x12" "\n"
	"\t" "\t" "mtc0 $k0, $12" "\n"
	"\t" "\t" "sync.p" "\n"
	"\t" "\t" "eret" "\n"
	"\t" ".end InvokeUserModeCallback" "\n"

	"\t" ".set pop" "\n"
);

__asm__
(
	"\t" ".set push" "\n"
	"\t" ".set noat" "\n"
	"\t" ".set noreorder" "\n"

	"\t" ".globl ResumeIntrDispatch" "\n"
	"\t" ".ent ResumeIntrDispatch" "\n"
	"\t" "ResumeIntrDispatch:" "\n"
	// Clear IE and EXL, and set KSU=00b (Kernel Mode)
	// COP0 registers: EPC ($14), status ($12)
	"\t" "\t" "mfc0 $at, $12" "\n"
	"\t" "\t" "addiu $k0, $zero, 0xFFE4" "\n"
	"\t" "\t" "and $at, $at, $k0" "\n"
	"\t" "\t" "mtc0 $at, $12" "\n"
	"\t" "\t" "sync.p" "\n"
	"\t" "\t" "lui $k0, %hi(dispatch_ra)" "\n"
	"\t" "\t" "lw $ra, %lo(dispatch_ra)($k0)" "\n"
	"\t" "\t" "lui $k0, %hi(dispatch_sp)" "\n"
	"\t" "\t" "jr $ra" "\n"
	"\t" "\t" "lw $sp, %lo(dispatch_sp)($k0)" "\n"
	"\t" ".end ResumeIntrDispatch" "\n"

	"\t" ".set pop" "\n"
);

__asm__
(
	"\t" ".set push" "\n"
	"\t" ".set noat" "\n"
	"\t" ".set noreorder" "\n"

	"\t" ".globl ChangeGP" "\n"
	"\t" ".ent ChangeGP" "\n"
	"\t" "ChangeGP:" "\n"
	"\t" "\t" "move    $v0, $gp" "\n"
	"\t" "\t" "jr  $ra" "\n"
	"\t" "\t" "move    $gp, $a0" "\n"
	"\t" ".end ChangeGP" "\n"

	"\t" ".set pop" "\n"
);

__asm__
(
	"\t" ".set push" "\n"
	"\t" ".set noat" "\n"
	"\t" ".set noreorder" "\n"

	"\t" ".globl SetGP" "\n"
	"\t" ".ent SetGP" "\n"
	"\t" "SetGP:" "\n"
	"\t" "\t" "jr  $ra" "\n"
	"\t" "\t" "move    $gp, $a0" "\n"
	"\t" ".end SetGP" "\n"

	"\t" ".set pop" "\n"
);

__asm__
(
	"\t" ".set push" "\n"
	"\t" ".set noat" "\n"
	"\t" ".set noreorder" "\n"

	"\t" ".globl GetGP" "\n"
	"\t" ".ent GetGP" "\n"
	"\t" "GetGP:" "\n"
	"\t" "\t" "jr  $ra" "\n"
	"\t" "\t" "move    $v0, $gp" "\n"
	"\t" ".end GetGP" "\n"

	"\t" ".set pop" "\n"
);

__asm__
(
	"\t" ".pushsection .data" "\n"
	"\t" "dispatch_ra: .quad 0" "\n"
	"\t" "dispatch_sp: .quad 0" "\n"
	"\t" ".popsection" "\n"
);

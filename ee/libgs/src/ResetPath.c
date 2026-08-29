
unsigned int init_vif_regs_12[] = {
	0x10000404, 0x20000000, 0x00000000, 0x50000000,
	0x60000000, 0x30000000, 0x20000000, 0x40000000,
};

__asm__
(
	"\t" ".globl GsResetPath" "\n"
	"\t" ".ent GsResetPath" "\n"
	"\t" "GsResetPath:" "\n"
	"\t" "\t" "li	$a3, 1" "\n"
	"\t" "\t" "la	$v0, 0x10003C10" "\n"
	"\t" "\t" "sw	$a3, ($v0)" "\n"
	"\t" "\t" "la	$v1, 0x10003C20" "\n"
	"\t" "\t" "li	$v0, 2" "\n"
	"\t" "\t" "move	$a0, $zero" "\n"
	"\t" "\t" "sw	$v0, 0($v1)" "\n"
	"\t" "\t" "sync" "\n"
	"\t" "\t" "cfc2	$a0, $28" "\n"
	"\t" "\t" "ori	$a0, 0x200" "\n"
	"\t" "\t" "ctc2	$a0, $28" "\n"
	"\t" "\t" "sync.p" "\n"
	"\t" "\t" "la	$a1, init_vif_regs_12" "\n"
	"\t" "\t" "la	$a2, 0x10005000" "\n"
	"\t" "\t" "lq	$a0, ($a1)" "\n"
	"\t" "\t" "la	$v1, 0x10003000" "\n"
	"\t" "\t" "sq	$a0, ($a2)" "\n"
	"\t" "\t" "lq	$v0, 8($a1)" "\n"
	"\t" "\t" "sq	$v0, ($a2)" "\n"
	"\t" "\t" "jr	$ra" "\n"
	"\t" "\t" "sw	$a3, 0($v1)" "\n"
	"\t" ".end GsResetPath" "\n"
);

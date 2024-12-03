/*
# _____     ___ ____     ___ ____
#  ____|   |    ____|   |        | |____|
# |     ___|   |____ ___|    ____| |    \    PS2DEV Open Source Project.
#-----------------------------------------------------------------------
# Copyright ps2dev - http://www.ps2dev.org
# Licenced under Academic Free License version 2.0
# Review ps2sdk README & LICENSE files for further details.
*/

#include "srxfixup_internal.h"
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static void getrs(unsigned int data, Operand *opr);
static void getrt(unsigned int data, Operand *opr);
static void getrd(unsigned int data, Operand *opr);
static void getc0rd_iop(unsigned int data, Operand *opr);
static void getc0rd_ee(unsigned int data, Operand *opr);
static void getczrd(unsigned int data, Operand *opr);
static void getshamt(unsigned int data, Operand *opr);
static void getfs(unsigned int data, Operand *opr);
static void getft(unsigned int data, Operand *opr);
static void getfd(unsigned int data, Operand *opr);
static void getczfs(unsigned int data, Operand *opr);
static void getbroff(unsigned int addr, unsigned int data, Operand *opr);
static void Rs(Disasm_result *result);
static void Rd(Disasm_result *result);
static void Rdrs(Disasm_result *result);
static void Rsrt(Disasm_result *result);
static void Rtc0rd_iop(Disasm_result *result);
static void Rtc0rd_ee(Disasm_result *result);
static void Rtczrd(Disasm_result *result);
static void Rdrsrt(Disasm_result *result);
static void Rdrtrs(Disasm_result *result);
static void Rdrtshamt(Disasm_result *result);
static void Rsseimm(Disasm_result *result);
static void Rtrsseimm(Disasm_result *result);
static void Rtrsimm(Disasm_result *result);
#if 0
static void  Rdimm(Disasm_result *result);
static void  Rsimm(Disasm_result *result);
#endif
static void Rtimm(Disasm_result *result);
static void Rsrtbroff(Disasm_result *result);
static void Rsbroff(Disasm_result *result);
static void Rtoffbase(Disasm_result *result);
static void Fdfs(Disasm_result *result);
static void Fsft(Disasm_result *result);
static void Fdfsft(Disasm_result *result);
static void Rtczfs(Disasm_result *result);
static void Code20(Disasm_result *result);
static void Jtarget(Disasm_result *result);
static void Cofun(Disasm_result *result);
static void Bcft(Disasm_result *result);

// clang-format off
static int regnmsw[5] = { 1, 1, 0, 1, 0 };
static const char * const REGNAME[2][32] =
{
	{
		"$0",
		"$at",
		"$2",
		"$3",
		"$4",
		"$5",
		"$6",
		"$7",
		"$8",
		"$9",
		"$10",
		"$11",
		"$12",
		"$13",
		"$14",
		"$15",
		"$16",
		"$17",
		"$18",
		"$19",
		"$20",
		"$21",
		"$22",
		"$23",
		"$24",
		"$25",
		"$26",
		"$27",
		"$gp",
		"$sp",
		"$30",
		"$31"
	},
	{
		"zero",
		"at",
		"v0",
		"v1",
		"a0",
		"a1",
		"a2",
		"a3",
		"t0",
		"t1",
		"t2",
		"t3",
		"t4",
		"t5",
		"t6",
		"t7",
		"s0",
		"s1",
		"s2",
		"s3",
		"s4",
		"s5",
		"s6",
		"s7",
		"t8",
		"t9",
		"k0",
		"k1",
		"gp",
		"sp",
		"fp",
		"ra"
	}
};
static const char * const REGC0_iop[2][32] =
{
	{
		"$0",
		"$1",
		"$2",
		"$3",
		"$4",
		"$5",
		"$6",
		"$7",
		"$8",
		"$9",
		"$10",
		"$11",
		"$12",
		"$13",
		"$14",
		"$15",
		"$16",
		"$17",
		"$18",
		"$19",
		"$20",
		"$21",
		"$22",
		"$23",
		"$24",
		"$25",
		"$26",
		"$27",
		"$28",
		"$29",
		"$30",
		"$31"
	},
	{
		"$0",
		"$1",
		"$2",
		"$bpc",
		"$4",
		"$bda",
		"$tar",
		"$dcic",
		"$bada",
		"$bdam",
		"$10",
		"$bpcm",
		"$sr",
		"$cause",
		"$epc",
		"$prid",
		"$16",
		"$17",
		"$18",
		"$19",
		"$20",
		"$21",
		"$22",
		"$23",
		"$24",
		"$25",
		"$26",
		"$27",
		"$28",
		"$29",
		"$30",
		"$31"
	}
};
static const char * const REGC0_ee[2][32] =
{
	{
		"$0",
		"$1",
		"$2",
		"$3",
		"$4",
		"$5",
		"$6",
		"$7",
		"$8",
		"$9",
		"$10",
		"$11",
		"$12",
		"$13",
		"$14",
		"$15",
		"$16",
		"$17",
		"$18",
		"$19",
		"$20",
		"$21",
		"$22",
		"$23",
		"$24",
		"$25",
		"$26",
		"$27",
		"$28",
		"$29",
		"$30",
		"$31"
	},
	{
		"$index",
		"$random",
		"$entrylo0",
		"$entrylo1",
		"$context",
		"$pagemask",
		"$wired",
		"$7",
		"$badvaddr",
		"$count",
		"$entryhi",
		"$compare",
		"$status",
		"$cause",
		"$epc",
		"$prid",
		"$config",
		"$17",
		"$18",
		"$19",
		"$20",
		"$21",
		"$22",
		"$badpaddr",
		"$hwbk",
		"$pccr",
		"$26",
		"$27",
		"$taglo",
		"$taghi",
		"$errorepc",
		"$31"
	}
};
static const char * const REGC1[2][32] =
{
	{
		"$f0",
		"$f1",
		"$f2",
		"$f3",
		"$f4",
		"$f5",
		"$f6",
		"$f7",
		"$f8",
		"$f9",
		"$f10",
		"$f11",
		"$f12",
		"$f13",
		"$f14",
		"$f15",
		"$f16",
		"$f17",
		"$f18",
		"$f19",
		"$f20",
		"$f21",
		"$f22",
		"$f23",
		"$f24",
		"$f25",
		"$f26",
		"$f27",
		"$f28",
		"$f29",
		"$f30",
		"$f31"
	},
	{
		"$fv0",
		"$fv1",
		"$ft0",
		"$ft1",
		"$ft2",
		"$ft3",
		"$ft4",
		"$ft5",
		"$ft6",
		"$ft7",
		"$ft8",
		"$ft9",
		"$fa0",
		"$fa1",
		"$fa2",
		"$fa3",
		"$fa4",
		"$fa5",
		"$fa6",
		"$fa7",
		"$fs0",
		"$fs1",
		"$fs2",
		"$fs3",
		"$fs4",
		"$fs5",
		"$fs6",
		"$fs7",
		"$fs8",
		"$fs9",
		"$fs10",
		"$fs11"
	}
};
// clang-format on

typedef void (*Operand_func)(Disasm_result *result);
typedef struct opcode_table_entry_
{
	const char *mnemonic;
	const struct opcode_table_ *subtable;
	Operand_func opfunc;
} Opcode_table_entry;
typedef struct opcode_table_
{
	int bit_pos;
	unsigned int bit_mask;
	const Opcode_table_entry *entries;
} Opcode_table;

static void getrs(unsigned int data, Operand *opr)
{
	opr->tag = OprTag_reg;
	opr->reg = (data >> 21) & 0x1F;
}

static void getrt(unsigned int data, Operand *opr)
{
	opr->tag = OprTag_reg;
	opr->reg = (data >> 16) & 0x1F;
}

static void getrd(unsigned int data, Operand *opr)
{
	opr->tag = OprTag_reg;
	opr->reg = (data >> 11) & 0x1F;
}

static void getc0rd_iop(unsigned int data, Operand *opr)
{
	opr->tag = OprTag_c0reg_iop;
	opr->reg = (data >> 11) & 0x1F;
}

static void getc0rd_ee(unsigned int data, Operand *opr)
{
	opr->tag = OprTag_c0reg_ee;
	opr->reg = (data >> 11) & 0x1F;
}

static void getczrd(unsigned int data, Operand *opr)
{
	opr->tag = OprTag_czreg;
	opr->reg = (data >> 11) & 0x1F;
}

static void getshamt(unsigned int data, Operand *opr)
{
	opr->tag = OprTag_shamt;
	opr->data = (data >> 6) & 0x1F;
}

static void getfs(unsigned int data, Operand *opr)
{
	opr->tag = OprTag_c1reg;
	opr->reg = (data >> 11) & 0x1F;
}

static void getft(unsigned int data, Operand *opr)
{
	opr->tag = OprTag_c1reg;
	opr->reg = (data >> 16) & 0x1F;
}

static void getfd(unsigned int data, Operand *opr)
{
	opr->tag = OprTag_c1reg;
	opr->reg = (data >> 6) & 0x1F;
}

static void getczfs(unsigned int data, Operand *opr)
{
	opr->tag = OprTag_c1reg;
	opr->reg = (data >> 11) & 0x1F;
}

static void getbroff(unsigned int addr, unsigned int data, Operand *opr)
{
	opr->tag = OprTag_jtarget;
	if ( (data & 0x8000) != 0 )
	{
		opr->data = 4 * (uint16_t)data - 0x40000;
	}
	else
	{
		opr->data = 4 * (uint16_t)data;
	}
	opr->data += 4 + addr;
}

static void Rs(Disasm_result *result)
{
	getrs(result->data, result->operands);
}

static void Rd(Disasm_result *result)
{
	getrd(result->data, result->operands);
}

static void Rdrs(Disasm_result *result)
{
	getrd(result->data, result->operands);
	getrs(result->data, &result->operands[1]);
}

static void Rsrt(Disasm_result *result)
{
	getrs(result->data, result->operands);
	getrt(result->data, &result->operands[1]);
}

static void Rtc0rd_iop(Disasm_result *result)
{
	getrt(result->data, result->operands);
	getc0rd_iop(result->data, &result->operands[1]);
}

static void Rtc0rd_ee(Disasm_result *result)
{
	getrt(result->data, result->operands);
	getc0rd_ee(result->data, &result->operands[1]);
}

static void Rtczrd(Disasm_result *result)
{
	getrt(result->data, result->operands);
	getczrd(result->data, &result->operands[1]);
}

static void Rdrsrt(Disasm_result *result)
{
	getrd(result->data, result->operands);
	getrs(result->data, &result->operands[1]);
	getrt(result->data, &result->operands[2]);
}

static void Rdrtrs(Disasm_result *result)
{
	getrd(result->data, result->operands);
	getrt(result->data, &result->operands[1]);
	getrs(result->data, &result->operands[2]);
}

static void Rdrtshamt(Disasm_result *result)
{
	getrd(result->data, result->operands);
	getrt(result->data, &result->operands[1]);
	getshamt(result->data, &result->operands[2]);
}

static void Rsseimm(Disasm_result *result)
{
	unsigned int imm;

	getrs(result->data, result->operands);
	if ( (int16_t)(result->data & 0xFFFF) < 0 )
	{
		imm = (result->data & 0xFFFF) - 0x10000;
	}
	else
	{
		imm = (result->data & 0xFFFF);
	}
	result->operands[1].tag = OprTag_imm;
	result->operands[1].data = imm;
}

static void Rtrsseimm(Disasm_result *result)
{
	unsigned int imm;

	getrt(result->data, result->operands);
	getrs(result->data, &result->operands[1]);
	if ( (int16_t)(result->data & 0xFFFF) < 0 )
	{
		imm = (result->data & 0xFFFF) - 0x10000;
	}
	else
	{
		imm = (result->data & 0xFFFF);
	}
	result->operands[2].tag = OprTag_imm;
	result->operands[2].data = imm;
}

static void Rtrsimm(Disasm_result *result)
{
	getrt(result->data, result->operands);
	getrs(result->data, &result->operands[1]);
	result->operands[2].tag = OprTag_imm;
	result->operands[2].data = (result->data & 0xFFFF);
}

#if 0
static void  Rdimm(Disasm_result *result)
{
	getrd(result->data, result->operands);
	result->operands[1].tag = OprTag_imm;
	result->operands[1].data = (result->data & 0xFFFF);
}

static void  Rsimm(Disasm_result *result)
{
	getrs(result->data, result->operands);
	result->operands[1].tag = OprTag_imm;
	result->operands[1].data = (result->data & 0xFFFF);
}
#endif

static void Rtimm(Disasm_result *result)
{
	getrt(result->data, result->operands);
	result->operands[1].tag = OprTag_imm;
	result->operands[1].data = (result->data & 0xFFFF);
}

static void Rsrtbroff(Disasm_result *result)
{
	getrs(result->data, result->operands);
	getrt(result->data, &result->operands[1]);
	getbroff(result->addr, result->data, &result->operands[2]);
}

static void Rsbroff(Disasm_result *result)
{
	getrs(result->data, result->operands);
	getbroff(result->addr, result->data, &result->operands[1]);
}

static void Rtoffbase(Disasm_result *result)
{
	unsigned int off;

	if ( (int16_t)(result->data & 0xFFFF) < 0 )
	{
		off = (result->data & 0xFFFF) - 0x10000;
	}
	else
	{
		off = (result->data & 0xFFFF);
	}
	getrt(result->data, result->operands);
	result->operands[1].tag = OprTag_regoffset;
	result->operands[1].data = off;
	result->operands[1].reg = (result->data >> 21) & 0x1F;
}

static void Fdfs(Disasm_result *result)
{
	getfd(result->data, result->operands);
	getfs(result->data, &result->operands[1]);
}

static void Fsft(Disasm_result *result)
{
	getfs(result->data, result->operands);
	getft(result->data, &result->operands[1]);
}

static void Fdfsft(Disasm_result *result)
{
	getfd(result->data, result->operands);
	getfs(result->data, &result->operands[1]);
	getft(result->data, &result->operands[2]);
}

static void Rtczfs(Disasm_result *result)
{
	getrt(result->data, result->operands);
	getczfs(result->data, &result->operands[1]);
}

static void Code20(Disasm_result *result)
{
	result->operands[0].tag = OprTag_code20;
	result->operands[0].data = (result->data >> 6) & 0xFFFFF;
}

static void Jtarget(Disasm_result *result)
{
	result->operands[0].tag = OprTag_jtarget;
	result->operands[0].data = (4 * (result->data & 0x3FFFFFF)) | (result->addr & 0xF0000000);
}

static void Cofun(Disasm_result *result)
{
	result->operands[0].tag = OprTag_code25;
	result->operands[0].data = result->data & 0x1FFFFFF;
}

static void Bcft(Disasm_result *result)
{
	result->mnemonic[0] = 'b';
	result->mnemonic[1] = 'c';
	result->mnemonic[2] = result->mnemonic[3];
	if ( (result->data & 0x10000) != 0 )
	{
		result->mnemonic[3] = 't';
	}
	else
	{
		result->mnemonic[3] = 'f';
	}
	if ( (result->data & 0x20000) != 0 )
	{
		result->mnemonic[4] = 'l';
	}
	else
	{
		result->mnemonic[4] = '\x00';
	}
	result->mnemonic[5] = '\x00';
	getbroff(result->addr, result->data, result->operands);
}

// clang-format off
static const Opcode_table_entry iop_SPECIAL_entries[] =
{
	{ "sll", NULL, &Rdrtshamt },
	{ NULL, NULL, NULL },
	{ "srl", NULL, &Rdrtshamt },
	{ "sra", NULL, &Rdrtshamt },
	{ "sllv", NULL, &Rdrtrs },
	{ NULL, NULL, NULL },
	{ "srlv", NULL, &Rdrtrs },
	{ "srav", NULL, &Rdrtrs },
	{ "jr", NULL, &Rs },
	{ "jalr", NULL, &Rdrs },
	{ NULL, NULL, NULL },
	{ NULL, NULL, NULL },
	{ "syscall", NULL, &Code20 },
	{ "break", NULL, &Code20 },
	{ NULL, NULL, NULL },
	{ NULL, NULL, NULL },
	{ "mfhi", NULL, &Rd },
	{ "mthi", NULL, &Rs },
	{ "mflo", NULL, &Rd },
	{ "mtlo", NULL, &Rs },
	{ NULL, NULL, NULL },
	{ NULL, NULL, NULL },
	{ NULL, NULL, NULL },
	{ NULL, NULL, NULL },
	{ "mult", NULL, &Rsrt },
	{ "multu", NULL, &Rsrt },
	{ "div", NULL, &Rsrt },
	{ "divu", NULL, &Rsrt },
	{ NULL, NULL, NULL },
	{ NULL, NULL, NULL },
	{ NULL, NULL, NULL },
	{ NULL, NULL, NULL },
	{ "add", NULL, &Rdrsrt },
	{ "addu", NULL, &Rdrsrt },
	{ "sub", NULL, &Rdrsrt },
	{ "subu", NULL, &Rdrsrt },
	{ "and", NULL, &Rdrsrt },
	{ "or", NULL, &Rdrsrt },
	{ "xor", NULL, &Rdrsrt },
	{ "nor", NULL, &Rdrsrt },
	{ NULL, NULL, NULL },
	{ NULL, NULL, NULL },
	{ "slt", NULL, &Rdrsrt },
	{ "sltu", NULL, &Rdrsrt },
	{ NULL, NULL, NULL },
	{ NULL, NULL, NULL },
	{ NULL, NULL, NULL },
	{ NULL, NULL, NULL },
	{ NULL, NULL, NULL },
	{ NULL, NULL, NULL },
	{ NULL, NULL, NULL },
	{ NULL, NULL, NULL },
	{ NULL, NULL, NULL },
	{ NULL, NULL, NULL },
	{ NULL, NULL, NULL },
	{ NULL, NULL, NULL },
	{ NULL, NULL, NULL },
	{ NULL, NULL, NULL },
	{ NULL, NULL, NULL },
	{ NULL, NULL, NULL },
	{ NULL, NULL, NULL },
	{ NULL, NULL, NULL },
	{ NULL, NULL, NULL },
	{ NULL, NULL, NULL }
};
static const Opcode_table iop_SPECIAL = { 0, 63, iop_SPECIAL_entries };
static const Opcode_table_entry ee_SPECIAL_entries[] =
{
	{ "sll", NULL, &Rdrtshamt },
	{ NULL, NULL, NULL },
	{ "srl", NULL, &Rdrtshamt },
	{ "sra", NULL, &Rdrtshamt },
	{ "sllv", NULL, &Rdrtrs },
	{ NULL, NULL, NULL },
	{ "srlv", NULL, &Rdrtrs },
	{ "srav", NULL, &Rdrtrs },
	{ "jr", NULL, &Rs },
	{ "jalr", NULL, &Rdrs },
	{ "movz", NULL, &Rdrtrs },
	{ "movn", NULL, &Rdrtrs },
	{ "syscall", NULL, &Code20 },
	{ "break", NULL, &Code20 },
	{ NULL, NULL, NULL },
	{ "sync", NULL, NULL },
	{ "mfhi", NULL, &Rd },
	{ "mthi", NULL, &Rs },
	{ "mflo", NULL, &Rd },
	{ "mtlo", NULL, &Rs },
	{ "dsllv", NULL, &Rdrtrs },
	{ NULL, NULL, NULL },
	{ "dsrlv", NULL, &Rdrtrs },
	{ "dsrav", NULL, &Rdrtrs },
	{ "mult", NULL, &Rsrt },
	{ "multu", NULL, &Rsrt },
	{ "div", NULL, &Rsrt },
	{ "divu", NULL, &Rsrt },
	{ NULL, NULL, NULL },
	{ NULL, NULL, NULL },
	{ NULL, NULL, NULL },
	{ NULL, NULL, NULL },
	{ "add", NULL, &Rdrsrt },
	{ "addu", NULL, &Rdrsrt },
	{ "sub", NULL, &Rdrsrt },
	{ "subu", NULL, &Rdrsrt },
	{ "and", NULL, &Rdrsrt },
	{ "or", NULL, &Rdrsrt },
	{ "xor", NULL, &Rdrsrt },
	{ "nor", NULL, &Rdrsrt },
	{ "mfsa", NULL, &Rd },
	{ "mtsa", NULL, &Rs },
	{ "slt", NULL, &Rdrsrt },
	{ "sltu", NULL, &Rdrsrt },
	{ "dadd", NULL, &Rdrsrt },
	{ "daddu", NULL, &Rdrsrt },
	{ "dsub", NULL, &Rdrsrt },
	{ "dsubu", NULL, &Rdrsrt },
	{ "tge", NULL, &Rsrt },
	{ "tgeu", NULL, &Rsrt },
	{ "tlt", NULL, &Rsrt },
	{ "tltu", NULL, &Rsrt },
	{ "teq", NULL, &Rsrt },
	{ NULL, NULL, NULL },
	{ "tne", NULL, &Rsrt },
	{ NULL, NULL, NULL },
	{ "dsll", NULL, &Rdrtshamt },
	{ NULL, NULL, NULL },
	{ "dsrl", NULL, &Rdrtshamt },
	{ "dsra", NULL, &Rdrtshamt },
	{ "dsll32", NULL, &Rdrtshamt },
	{ NULL, NULL, NULL },
	{ "dsrl32", NULL, &Rdrtshamt },
	{ "dsra32", NULL, &Rdrtshamt }
};
static const Opcode_table ee_SPECIAL = { 0, 63, ee_SPECIAL_entries };
static const Opcode_table_entry BCOND_entries[] =
{
	{ "bltz", NULL, &Rsbroff },
	{ "bgez", NULL, &Rsbroff },
	{ NULL, NULL, NULL },
	{ NULL, NULL, NULL },
	{ NULL, NULL, NULL },
	{ NULL, NULL, NULL },
	{ NULL, NULL, NULL },
	{ NULL, NULL, NULL },
	{ NULL, NULL, NULL },
	{ NULL, NULL, NULL },
	{ NULL, NULL, NULL },
	{ NULL, NULL, NULL },
	{ NULL, NULL, NULL },
	{ NULL, NULL, NULL },
	{ NULL, NULL, NULL },
	{ NULL, NULL, NULL },
	{ "bltzal", NULL, &Rsbroff },
	{ "bgezal", NULL, &Rsbroff },
	{ NULL, NULL, NULL },
	{ NULL, NULL, NULL },
	{ NULL, NULL, NULL },
	{ NULL, NULL, NULL },
	{ NULL, NULL, NULL },
	{ NULL, NULL, NULL },
	{ NULL, NULL, NULL },
	{ NULL, NULL, NULL },
	{ NULL, NULL, NULL },
	{ NULL, NULL, NULL },
	{ NULL, NULL, NULL },
	{ NULL, NULL, NULL },
	{ NULL, NULL, NULL },
	{ NULL, NULL, NULL }
};
static const Opcode_table BCOND = { 16, 31, BCOND_entries };
static const Opcode_table_entry ee_REGIMM_entries[] =
{
	{ "bltz", NULL, &Rsbroff },
	{ "bgez", NULL, &Rsbroff },
	{ "bltzl", NULL, &Rsbroff },
	{ "bgezl", NULL, &Rsbroff },
	{ NULL, NULL, NULL },
	{ NULL, NULL, NULL },
	{ NULL, NULL, NULL },
	{ NULL, NULL, NULL },
	{ "tgei", NULL, &Rsseimm },
	{ "tgeiu", NULL, &Rsseimm },
	{ "tlti", NULL, &Rsseimm },
	{ "tltiu", NULL, &Rsseimm },
	{ "teqi", NULL, &Rsseimm },
	{ NULL, NULL, NULL },
	{ "tnei", NULL, &Rsseimm },
	{ NULL, NULL, NULL },
	{ "bltzal", NULL, &Rsbroff },
	{ "bgezal", NULL, &Rsbroff },
	{ "bltzall", NULL, &Rsbroff },
	{ "bgezall", NULL, &Rsbroff },
	{ NULL, NULL, NULL },
	{ NULL, NULL, NULL },
	{ NULL, NULL, NULL },
	{ NULL, NULL, NULL },
	{ "mtsab", NULL, &Rsseimm },
	{ "mtsah", NULL, &Rsseimm },
	{ NULL, NULL, NULL },
	{ NULL, NULL, NULL },
	{ NULL, NULL, NULL },
	{ NULL, NULL, NULL },
	{ NULL, NULL, NULL },
	{ NULL, NULL, NULL }
};
static const Opcode_table ee_REGIMM = { 16, 31, ee_REGIMM_entries };
static const Opcode_table_entry iop_COP0CO_entries[] =
{
	{ NULL, NULL, NULL },
	{ "tlbr", NULL, NULL },
	{ "tlbwi", NULL, NULL },
	{ NULL, NULL, NULL },
	{ NULL, NULL, NULL },
	{ NULL, NULL, NULL },
	{ "tlbwr", NULL, NULL },
	{ NULL, NULL, NULL },
	{ "tlbp", NULL, NULL },
	{ NULL, NULL, NULL },
	{ NULL, NULL, NULL },
	{ NULL, NULL, NULL },
	{ NULL, NULL, NULL },
	{ NULL, NULL, NULL },
	{ NULL, NULL, NULL },
	{ NULL, NULL, NULL },
	{ "rfe", NULL, NULL },
	{ NULL, NULL, NULL },
	{ NULL, NULL, NULL },
	{ NULL, NULL, NULL },
	{ NULL, NULL, NULL },
	{ NULL, NULL, NULL },
	{ NULL, NULL, NULL },
	{ NULL, NULL, NULL },
	{ NULL, NULL, NULL },
	{ NULL, NULL, NULL },
	{ NULL, NULL, NULL },
	{ NULL, NULL, NULL },
	{ NULL, NULL, NULL },
	{ NULL, NULL, NULL },
	{ NULL, NULL, NULL },
	{ NULL, NULL, NULL },
	{ NULL, NULL, NULL },
	{ NULL, NULL, NULL },
	{ NULL, NULL, NULL },
	{ NULL, NULL, NULL },
	{ NULL, NULL, NULL },
	{ NULL, NULL, NULL },
	{ NULL, NULL, NULL },
	{ NULL, NULL, NULL },
	{ NULL, NULL, NULL },
	{ NULL, NULL, NULL },
	{ NULL, NULL, NULL },
	{ NULL, NULL, NULL },
	{ NULL, NULL, NULL },
	{ NULL, NULL, NULL },
	{ NULL, NULL, NULL },
	{ NULL, NULL, NULL },
	{ NULL, NULL, NULL },
	{ NULL, NULL, NULL },
	{ NULL, NULL, NULL },
	{ NULL, NULL, NULL },
	{ NULL, NULL, NULL },
	{ NULL, NULL, NULL },
	{ NULL, NULL, NULL },
	{ NULL, NULL, NULL },
	{ NULL, NULL, NULL },
	{ NULL, NULL, NULL },
	{ NULL, NULL, NULL },
	{ NULL, NULL, NULL },
	{ NULL, NULL, NULL },
	{ NULL, NULL, NULL },
	{ NULL, NULL, NULL },
	{ NULL, NULL, NULL }
};
static const Opcode_table iop_COP0CO = { 0, 63, iop_COP0CO_entries };
static const Opcode_table_entry ee_COP0CO_entries[] =
{
	{ NULL, NULL, NULL },
	{ "tlbr", NULL, NULL },
	{ "tlbwi", NULL, NULL },
	{ NULL, NULL, NULL },
	{ NULL, NULL, NULL },
	{ NULL, NULL, NULL },
	{ "tlbwr", NULL, NULL },
	{ NULL, NULL, NULL },
	{ "tlbp", NULL, NULL },
	{ NULL, NULL, NULL },
	{ NULL, NULL, NULL },
	{ NULL, NULL, NULL },
	{ NULL, NULL, NULL },
	{ NULL, NULL, NULL },
	{ NULL, NULL, NULL },
	{ NULL, NULL, NULL },
	{ NULL, NULL, NULL },
	{ NULL, NULL, NULL },
	{ NULL, NULL, NULL },
	{ NULL, NULL, NULL },
	{ NULL, NULL, NULL },
	{ NULL, NULL, NULL },
	{ NULL, NULL, NULL },
	{ NULL, NULL, NULL },
	{ "eret", NULL, NULL },
	{ NULL, NULL, NULL },
	{ NULL, NULL, NULL },
	{ NULL, NULL, NULL },
	{ NULL, NULL, NULL },
	{ NULL, NULL, NULL },
	{ NULL, NULL, NULL },
	{ NULL, NULL, NULL },
	{ NULL, NULL, NULL },
	{ NULL, NULL, NULL },
	{ NULL, NULL, NULL },
	{ NULL, NULL, NULL },
	{ NULL, NULL, NULL },
	{ NULL, NULL, NULL },
	{ NULL, NULL, NULL },
	{ NULL, NULL, NULL },
	{ NULL, NULL, NULL },
	{ NULL, NULL, NULL },
	{ NULL, NULL, NULL },
	{ NULL, NULL, NULL },
	{ NULL, NULL, NULL },
	{ NULL, NULL, NULL },
	{ NULL, NULL, NULL },
	{ NULL, NULL, NULL },
	{ NULL, NULL, NULL },
	{ NULL, NULL, NULL },
	{ NULL, NULL, NULL },
	{ NULL, NULL, NULL },
	{ NULL, NULL, NULL },
	{ NULL, NULL, NULL },
	{ NULL, NULL, NULL },
	{ NULL, NULL, NULL },
	{ "ei", NULL, NULL },
	{ "di", NULL, NULL },
	{ NULL, NULL, NULL },
	{ NULL, NULL, NULL },
	{ NULL, NULL, NULL },
	{ NULL, NULL, NULL },
	{ NULL, NULL, NULL },
	{ NULL, NULL, NULL }
};
static const Opcode_table ee_COP0CO = { 0, 63, ee_COP0CO_entries };
static const Opcode_table_entry iop_COP0_entries[] =
{
	{ "mfc0", NULL, &Rtc0rd_iop },
	{ NULL, NULL, NULL },
	{ "cfc0", NULL, &Rtc0rd_iop },
	{ NULL, NULL, NULL },
	{ "mtc0", NULL, &Rtc0rd_iop },
	{ NULL, NULL, NULL },
	{ "ctc0", NULL, &Rtc0rd_iop },
	{ NULL, NULL, NULL },
	{ "cop0", NULL, &Bcft },
	{ NULL, NULL, NULL },
	{ NULL, NULL, NULL },
	{ NULL, NULL, NULL },
	{ "cop0", NULL, &Bcft },
	{ NULL, NULL, NULL },
	{ NULL, NULL, NULL },
	{ NULL, NULL, NULL },
	{ NULL, &iop_COP0CO, NULL },
	{ NULL, &iop_COP0CO, NULL },
	{ NULL, &iop_COP0CO, NULL },
	{ NULL, &iop_COP0CO, NULL },
	{ NULL, &iop_COP0CO, NULL },
	{ NULL, &iop_COP0CO, NULL },
	{ NULL, &iop_COP0CO, NULL },
	{ NULL, &iop_COP0CO, NULL },
	{ NULL, &iop_COP0CO, NULL },
	{ NULL, &iop_COP0CO, NULL },
	{ NULL, &iop_COP0CO, NULL },
	{ NULL, &iop_COP0CO, NULL },
	{ NULL, &iop_COP0CO, NULL },
	{ NULL, &iop_COP0CO, NULL },
	{ NULL, &iop_COP0CO, NULL },
	{ NULL, &iop_COP0CO, NULL }
};
static const Opcode_table iop_COP0 = { 21, 31, iop_COP0_entries };
static const Opcode_table_entry ee_COP0_entries[] =
{
	{ "mfc0", NULL, &Rtc0rd_ee },
	{ NULL, NULL, NULL },
	{ "cfc0", NULL, &Rtc0rd_ee },
	{ NULL, NULL, NULL },
	{ "mtc0", NULL, &Rtc0rd_ee },
	{ NULL, NULL, NULL },
	{ "ctc0", NULL, &Rtc0rd_ee },
	{ NULL, NULL, NULL },
	{ "cop0", NULL, &Bcft },
	{ NULL, NULL, NULL },
	{ NULL, NULL, NULL },
	{ NULL, NULL, NULL },
	{ "cop0", NULL, &Bcft },
	{ NULL, NULL, NULL },
	{ NULL, NULL, NULL },
	{ NULL, NULL, NULL },
	{ NULL, &ee_COP0CO, NULL },
	{ NULL, &ee_COP0CO, NULL },
	{ NULL, &ee_COP0CO, NULL },
	{ NULL, &ee_COP0CO, NULL },
	{ NULL, &ee_COP0CO, NULL },
	{ NULL, &ee_COP0CO, NULL },
	{ NULL, &ee_COP0CO, NULL },
	{ NULL, &ee_COP0CO, NULL },
	{ NULL, &ee_COP0CO, NULL },
	{ NULL, &ee_COP0CO, NULL },
	{ NULL, &ee_COP0CO, NULL },
	{ NULL, &ee_COP0CO, NULL },
	{ NULL, &ee_COP0CO, NULL },
	{ NULL, &ee_COP0CO, NULL },
	{ NULL, &ee_COP0CO, NULL },
	{ NULL, &ee_COP0CO, NULL }
};
static const Opcode_table ee_COP0 = { 21, 31, ee_COP0_entries };
static const Opcode_table_entry ee_COP1CO_entries[] =
{
	{ "add.s", NULL, &Fdfsft },
	{ "sub.s", NULL, &Fdfsft },
	{ "mul.s", NULL, &Fdfsft },
	{ "div.s", NULL, &Fdfsft },
	{ "sqrt.s", NULL, &Fdfs },
	{ "abs.s", NULL, &Fdfs },
	{ "mov.s", NULL, &Fdfs },
	{ "neg.s", NULL, &Fdfs },
	{ NULL, NULL, NULL },
	{ NULL, NULL, NULL },
	{ NULL, NULL, NULL },
	{ NULL, NULL, NULL },
	{ NULL, NULL, NULL },
	{ NULL, NULL, NULL },
	{ NULL, NULL, NULL },
	{ NULL, NULL, NULL },
	{ NULL, NULL, NULL },
	{ NULL, NULL, NULL },
	{ NULL, NULL, NULL },
	{ NULL, NULL, NULL },
	{ NULL, NULL, NULL },
	{ NULL, NULL, NULL },
	{ "rsqrt.s", NULL, &Fdfs },
	{ NULL, NULL, NULL },
	{ "adda.s", NULL, &Fsft },
	{ "suba.s", NULL, &Fsft },
	{ "mula.s", NULL, &Fsft },
	{ NULL, NULL, NULL },
	{ "madd.s", NULL, &Fdfsft },
	{ "msub.s", NULL, &Fdfsft },
	{ "madda.s", NULL, &Fsft },
	{ "msuba.s", NULL, &Fsft },
	{ "cvt", NULL, &Fdfs },
	{ NULL, NULL, NULL },
	{ NULL, NULL, NULL },
	{ NULL, NULL, NULL },
	{ "cvt", NULL, &Fdfs },
	{ NULL, NULL, NULL },
	{ NULL, NULL, NULL },
	{ NULL, NULL, NULL },
	{ "max.s", NULL, &Fdfs },
	{ "min.s", NULL, &Fdfs },
	{ NULL, NULL, NULL },
	{ NULL, NULL, NULL },
	{ NULL, NULL, NULL },
	{ NULL, NULL, NULL },
	{ NULL, NULL, NULL },
	{ NULL, NULL, NULL },
	{ "c.f.s", NULL, &Fsft },
	{ NULL, NULL, NULL },
	{ "c.eq.s", NULL, &Fsft },
	{ NULL, NULL, NULL },
	{ "c.lt.s", NULL, &Fsft },
	{ NULL, NULL, NULL },
	{ "c.le.s", NULL, &Fsft },
	{ NULL, NULL, NULL },
	{ NULL, NULL, NULL },
	{ NULL, NULL, NULL },
	{ NULL, NULL, NULL },
	{ NULL, NULL, NULL },
	{ NULL, NULL, NULL },
	{ NULL, NULL, NULL },
	{ NULL, NULL, NULL },
	{ NULL, NULL, NULL }
};
static const Opcode_table ee_COP1CO = { 0, 63, ee_COP1CO_entries };
static const Opcode_table_entry iop_COP1_entries[] =
{
	{ "mfc1", NULL, &Rtczrd },
	{ NULL, NULL, NULL },
	{ "cfc1", NULL, &Rtczrd },
	{ NULL, NULL, NULL },
	{ "mtc1", NULL, &Rtczrd },
	{ NULL, NULL, NULL },
	{ "ctc1", NULL, &Rtczrd },
	{ NULL, NULL, NULL },
	{ "cop1", NULL, &Bcft },
	{ NULL, NULL, NULL },
	{ NULL, NULL, NULL },
	{ NULL, NULL, NULL },
	{ "cop1", NULL, &Bcft },
	{ NULL, NULL, NULL },
	{ NULL, NULL, NULL },
	{ NULL, NULL, NULL },
	{ "cop1", NULL, &Cofun },
	{ "cop1", NULL, &Cofun },
	{ "cop1", NULL, &Cofun },
	{ "cop1", NULL, &Cofun },
	{ "cop1", NULL, &Cofun },
	{ "cop1", NULL, &Cofun },
	{ "cop1", NULL, &Cofun },
	{ "cop1", NULL, &Cofun },
	{ "cop1", NULL, &Cofun },
	{ "cop1", NULL, &Cofun },
	{ "cop1", NULL, &Cofun },
	{ "cop1", NULL, &Cofun },
	{ "cop1", NULL, &Cofun },
	{ "cop1", NULL, &Cofun },
	{ "cop1", NULL, &Cofun },
	{ "cop1", NULL, &Cofun }
};
static const Opcode_table iop_COP1 = { 21, 31, iop_COP1_entries };
static const Opcode_table_entry ee_COP1_entries[] =
{
	{ "mfc1", NULL, &Rtczfs },
	{ NULL, NULL, NULL },
	{ "cfc1", NULL, &Rtczfs },
	{ NULL, NULL, NULL },
	{ "mtc1", NULL, &Rtczfs },
	{ NULL, NULL, NULL },
	{ "ctc1", NULL, &Rtczfs },
	{ NULL, NULL, NULL },
	{ "cop1", NULL, &Bcft },
	{ NULL, NULL, NULL },
	{ NULL, NULL, NULL },
	{ NULL, NULL, NULL },
	{ "cop1", NULL, &Bcft },
	{ NULL, NULL, NULL },
	{ NULL, NULL, NULL },
	{ NULL, NULL, NULL },
	{ NULL, &ee_COP1CO, NULL },
	{ NULL, &ee_COP1CO, NULL },
	{ NULL, &ee_COP1CO, NULL },
	{ NULL, &ee_COP1CO, NULL },
	{ NULL, &ee_COP1CO, NULL },
	{ NULL, &ee_COP1CO, NULL },
	{ NULL, &ee_COP1CO, NULL },
	{ NULL, &ee_COP1CO, NULL },
	{ NULL, &ee_COP1CO, NULL },
	{ NULL, &ee_COP1CO, NULL },
	{ NULL, &ee_COP1CO, NULL },
	{ NULL, &ee_COP1CO, NULL },
	{ NULL, &ee_COP1CO, NULL },
	{ NULL, &ee_COP1CO, NULL },
	{ NULL, &ee_COP1CO, NULL },
	{ NULL, &ee_COP1CO, NULL }
};
static const Opcode_table ee_COP1 = { 21, 31, ee_COP1_entries };
static const Opcode_table_entry iop_COP2_entries[] =
{
	{ "mfc2", NULL, &Rtczrd },
	{ NULL, NULL, NULL },
	{ "cfc2", NULL, &Rtczrd },
	{ NULL, NULL, NULL },
	{ "mtc2", NULL, &Rtczrd },
	{ NULL, NULL, NULL },
	{ "ctc2", NULL, &Rtczrd },
	{ NULL, NULL, NULL },
	{ "cop2", NULL, &Bcft },
	{ NULL, NULL, NULL },
	{ NULL, NULL, NULL },
	{ NULL, NULL, NULL },
	{ "cop2", NULL, &Bcft },
	{ NULL, NULL, NULL },
	{ NULL, NULL, NULL },
	{ NULL, NULL, NULL },
	{ "cop2", NULL, &Cofun },
	{ "cop2", NULL, &Cofun },
	{ "cop2", NULL, &Cofun },
	{ "cop2", NULL, &Cofun },
	{ "cop2", NULL, &Cofun },
	{ "cop2", NULL, &Cofun },
	{ "cop2", NULL, &Cofun },
	{ "cop2", NULL, &Cofun },
	{ "cop2", NULL, &Cofun },
	{ "cop2", NULL, &Cofun },
	{ "cop2", NULL, &Cofun },
	{ "cop2", NULL, &Cofun },
	{ "cop2", NULL, &Cofun },
	{ "cop2", NULL, &Cofun },
	{ "cop2", NULL, &Cofun },
	{ "cop2", NULL, &Cofun }
};
static const Opcode_table iop_COP2 = { 21, 31, iop_COP2_entries };
static const Opcode_table_entry ee_COP2_entries[] =
{
	{ "mfc2", NULL, &Rtczrd },
	{ "qmfc2", NULL, &Rtczrd },
	{ "cfc2", NULL, &Rtczrd },
	{ NULL, NULL, NULL },
	{ "mtc2", NULL, &Rtczrd },
	{ "qmtc2", NULL, &Rtczrd },
	{ "ctc2", NULL, &Rtczrd },
	{ NULL, NULL, NULL },
	{ "cop2", NULL, &Bcft },
	{ NULL, NULL, NULL },
	{ NULL, NULL, NULL },
	{ NULL, NULL, NULL },
	{ "cop2", NULL, &Bcft },
	{ NULL, NULL, NULL },
	{ NULL, NULL, NULL },
	{ NULL, NULL, NULL },
	{ "cop2", NULL, &Cofun },
	{ "cop2", NULL, &Cofun },
	{ "cop2", NULL, &Cofun },
	{ "cop2", NULL, &Cofun },
	{ "cop2", NULL, &Cofun },
	{ "cop2", NULL, &Cofun },
	{ "cop2", NULL, &Cofun },
	{ "cop2", NULL, &Cofun },
	{ "cop2", NULL, &Cofun },
	{ "cop2", NULL, &Cofun },
	{ "cop2", NULL, &Cofun },
	{ "cop2", NULL, &Cofun },
	{ "cop2", NULL, &Cofun },
	{ "cop2", NULL, &Cofun },
	{ "cop2", NULL, &Cofun },
	{ "cop2", NULL, &Cofun }
};
static const Opcode_table ee_COP2 = { 21, 31, ee_COP2_entries };
static const Opcode_table_entry COP3_entries[] =
{
	{ "mfc3", NULL, &Rtczrd },
	{ NULL, NULL, NULL },
	{ "cfc3", NULL, &Rtczrd },
	{ NULL, NULL, NULL },
	{ "mtc3", NULL, &Rtczrd },
	{ NULL, NULL, NULL },
	{ "ctc3", NULL, &Rtczrd },
	{ NULL, NULL, NULL },
	{ "cop3", NULL, &Bcft },
	{ NULL, NULL, NULL },
	{ NULL, NULL, NULL },
	{ NULL, NULL, NULL },
	{ "cop3", NULL, &Bcft },
	{ NULL, NULL, NULL },
	{ NULL, NULL, NULL },
	{ NULL, NULL, NULL },
	{ "cop3", NULL, &Cofun },
	{ "cop3", NULL, &Cofun },
	{ "cop3", NULL, &Cofun },
	{ "cop3", NULL, &Cofun },
	{ "cop3", NULL, &Cofun },
	{ "cop3", NULL, &Cofun },
	{ "cop3", NULL, &Cofun },
	{ "cop3", NULL, &Cofun },
	{ "cop3", NULL, &Cofun },
	{ "cop3", NULL, &Cofun },
	{ "cop3", NULL, &Cofun },
	{ "cop3", NULL, &Cofun },
	{ "cop3", NULL, &Cofun },
	{ "cop3", NULL, &Cofun },
	{ "cop3", NULL, &Cofun },
	{ "cop3", NULL, &Cofun }
};
static const Opcode_table COP3 = { 21, 31, COP3_entries };
static const Opcode_table_entry iop_opcode_root_table_entries[] =
{
	{ NULL, &iop_SPECIAL, NULL },
	{ NULL, &BCOND, NULL },
	{ "j", NULL, &Jtarget },
	{ "jal", NULL, &Jtarget },
	{ "beq", NULL, &Rsrtbroff },
	{ "bne", NULL, &Rsrtbroff },
	{ "blez", NULL, &Rsbroff },
	{ "bgtz", NULL, &Rsbroff },
	{ "addi", NULL, &Rtrsseimm },
	{ "addiu", NULL, &Rtrsseimm },
	{ "slti", NULL, &Rtrsseimm },
	{ "sltiu", NULL, &Rtrsseimm },
	{ "andi", NULL, &Rtrsimm },
	{ "ori", NULL, &Rtrsimm },
	{ "xori", NULL, &Rtrsimm },
	{ "lui", NULL, &Rtimm },
	{ NULL, &iop_COP0, NULL },
	{ NULL, &iop_COP1, NULL },
	{ NULL, &iop_COP2, NULL },
	{ NULL, &COP3, NULL },
	{ NULL, NULL, NULL },
	{ NULL, NULL, NULL },
	{ NULL, NULL, NULL },
	{ NULL, NULL, NULL },
	{ NULL, NULL, NULL },
	{ NULL, NULL, NULL },
	{ NULL, NULL, NULL },
	{ NULL, NULL, NULL },
	{ NULL, NULL, NULL },
	{ NULL, NULL, NULL },
	{ NULL, NULL, NULL },
	{ NULL, NULL, NULL },
	{ "lb", NULL, &Rtoffbase },
	{ "lh", NULL, &Rtoffbase },
	{ "lwl", NULL, &Rtoffbase },
	{ "lw", NULL, &Rtoffbase },
	{ "lbu", NULL, &Rtoffbase },
	{ "lhu", NULL, &Rtoffbase },
	{ "lwr", NULL, &Rtoffbase },
	{ NULL, NULL, NULL },
	{ "sb", NULL, &Rtoffbase },
	{ "sh", NULL, &Rtoffbase },
	{ "swl", NULL, &Rtoffbase },
	{ "sw", NULL, &Rtoffbase },
	{ NULL, NULL, NULL },
	{ NULL, NULL, NULL },
	{ "swr", NULL, &Rtoffbase },
	{ NULL, NULL, NULL },
	{ "lwc0", NULL, &Rtoffbase },
	{ "lwc1", NULL, &Rtoffbase },
	{ "lwc2", NULL, &Rtoffbase },
	{ "lwc3", NULL, &Rtoffbase },
	{ NULL, NULL, NULL },
	{ NULL, NULL, NULL },
	{ NULL, NULL, NULL },
	{ NULL, NULL, NULL },
	{ "swc0", NULL, &Rtoffbase },
	{ "swc1", NULL, &Rtoffbase },
	{ "swc2", NULL, &Rtoffbase },
	{ "swc3", NULL, &Rtoffbase },
	{ NULL, NULL, NULL },
	{ NULL, NULL, NULL },
	{ NULL, NULL, NULL },
	{ NULL, NULL, NULL }
};
static const Opcode_table_entry ee_opcode_root_table_entries[] =
{
	{ NULL, &ee_SPECIAL, NULL },
	{ NULL, &ee_REGIMM, NULL },
	{ "j", NULL, &Jtarget },
	{ "jal", NULL, &Jtarget },
	{ "beq", NULL, &Rsrtbroff },
	{ "bne", NULL, &Rsrtbroff },
	{ "blez", NULL, &Rsbroff },
	{ "bgtz", NULL, &Rsbroff },
	{ "addi", NULL, &Rtrsseimm },
	{ "addiu", NULL, &Rtrsseimm },
	{ "slti", NULL, &Rtrsseimm },
	{ "sltiu", NULL, &Rtrsseimm },
	{ "andi", NULL, &Rtrsimm },
	{ "ori", NULL, &Rtrsimm },
	{ "xori", NULL, &Rtrsimm },
	{ "lui", NULL, &Rtimm },
	{ NULL, &ee_COP0, NULL },
	{ NULL, &ee_COP1, NULL },
	{ NULL, &ee_COP2, NULL },
	{ NULL, &COP3, NULL },
	{ "beql", NULL, &Rsrtbroff },
	{ "bnel", NULL, &Rsrtbroff },
	{ "blezl", NULL, &Rsbroff },
	{ "bgtzl", NULL, &Rsbroff },
	{ "daddi", NULL, &Rtrsseimm },
	{ "daddiu", NULL, &Rtrsseimm },
	{ "ldl", NULL, &Rtoffbase },
	{ "ldr", NULL, &Rtoffbase },
	{ "mmi", NULL, NULL },
	{ NULL, NULL, NULL },
	{ "lq", NULL, &Rtoffbase },
	{ "sq", NULL, &Rtoffbase },
	{ "lb", NULL, &Rtoffbase },
	{ "lh", NULL, &Rtoffbase },
	{ "lwl", NULL, &Rtoffbase },
	{ "lw", NULL, &Rtoffbase },
	{ "lbu", NULL, &Rtoffbase },
	{ "lhu", NULL, &Rtoffbase },
	{ "lwr", NULL, &Rtoffbase },
	{ NULL, NULL, NULL },
	{ "sb", NULL, &Rtoffbase },
	{ "sh", NULL, &Rtoffbase },
	{ "swl", NULL, &Rtoffbase },
	{ "sw", NULL, &Rtoffbase },
	{ "sdl", NULL, &Rtoffbase },
	{ "sdr", NULL, &Rtoffbase },
	{ "swr", NULL, &Rtoffbase },
	{ "cache", NULL, NULL },
	{ "lwc0", NULL, &Rtoffbase },
	{ "lwc1", NULL, &Rtoffbase },
	{ "lwc2", NULL, &Rtoffbase },
	{ "pref", NULL, &Rtoffbase },
	{ NULL, NULL, NULL },
	{ NULL, NULL, NULL },
	{ "lqc2", NULL, &Rtoffbase },
	{ "ld", NULL, &Rtoffbase },
	{ "swc0", NULL, &Rtoffbase },
	{ "swc1", NULL, &Rtoffbase },
	{ "swc2", NULL, &Rtoffbase },
	{ "swc3", NULL, &Rtoffbase },
	{ NULL, NULL, NULL },
	{ NULL, NULL, NULL },
	{ "sqc2", NULL, &Rtoffbase },
	{ "sd", NULL, &Rtoffbase }
};
static const Opcode_table iop_opcode_root_table = { 26, 63, iop_opcode_root_table_entries };
static const Opcode_table ee_opcode_root_table = { 26, 63, ee_opcode_root_table_entries };
static const Opcode_table * opcode_root_table = &iop_opcode_root_table;
// clang-format on

void gen_asmmacro(Disasm_result *result)
{
	if ( result->data )
	{
		if (
			!strcmp("beq", result->mnemonic) && result->operands[0].tag == OprTag_reg && result->operands[1].tag == OprTag_reg
			&& result->operands[1].reg == result->operands[0].reg )
		{
			strcpy(result->mnemonic, "b");
			result->operands[0] = result->operands[2];
			result->operands[1].tag = OprTag_none;
		}
		else if ( !strcmp("bgezal", result->mnemonic) && result->operands[0].tag == OprTag_reg && !result->operands[0].reg )
		{
			strcpy(result->mnemonic, "bal");
			result->operands[0] = result->operands[1];
			result->operands[1].tag = OprTag_none;
		}
		else if ( !strcmp("cvt", result->mnemonic) )
		{
			if ( ((result->data >> 21) & 0x1F) == 20 )
			{
				strcpy(result->mnemonic, "cvt.s.w");
			}
			else if ( ((result->data >> 21) & 0x1F) == 16 )
			{
				strcpy(result->mnemonic, "cvt.w.s");
			}
		}
	}
	else
	{
		strcpy(result->mnemonic, "nop");
		result->operands[0].tag = OprTag_none;
	}
}

void initdisasm(int arch, int regform0, int regform1, int regform2, int regform3)
{
	switch ( arch )
	{
		case 1:
			opcode_root_table = &iop_opcode_root_table;
			break;
		case 2:
			opcode_root_table = &ee_opcode_root_table;
			break;
		default:
			break;
	}
	if ( regform0 != -1 )
	{
		if ( !regform0 || regform0 == 1 )
		{
			regnmsw[0] = regform0;
		}
		if ( !regform1 || regform1 == 1 )
		{
			regnmsw[1] = regform0;
		}
		if ( !regform2 || regform2 == 1 )
		{
			regnmsw[2] = regform0;
		}
		if ( !regform3 || regform3 == 1 )
		{
			regnmsw[3] = regform3;
		}
	}
}

Disasm_result *disassemble(unsigned int addr, unsigned int data)
{
	Disasm_result *v3;
	const Opcode_table_entry *op;
	const Opcode_table *optable;

	optable = opcode_root_table;
	v3 = (Disasm_result *)calloc(1, sizeof(Disasm_result));
	for ( ;; )
	{
		op = &optable->entries[(data >> optable->bit_pos) & optable->bit_mask];
		if ( !op->subtable )
		{
			break;
		}
		optable = op->subtable;
	}
	v3->addr = addr;
	v3->data = data;
	if ( op->mnemonic )
	{
		strcpy(v3->mnemonic, op->mnemonic);
		if ( op->opfunc )
		{
			op->opfunc(v3);
		}
	}
	return v3;
}

void shex(char *buf, unsigned int data)
{
	if ( (data & 0x80000000) != 0 )
	{
		sprintf(buf, "-0x%x", -data);
	}
	else
	{
		sprintf(buf, "0x%x", data);
	}
}

void format_operand(const Operand *opr, char *buf)
{
	switch ( opr->tag )
	{
		case OprTag_reg:
			strcpy(buf, REGNAME[regnmsw[0]][opr->reg]);
			break;
		case OprTag_c0reg_iop:
			strcpy(buf, REGC0_iop[regnmsw[0]][opr->reg]);
			break;
		case OprTag_c0reg_ee:
			strcpy(buf, REGC0_ee[regnmsw[0]][opr->reg]);
			break;
		case OprTag_czreg:
			sprintf(buf, "$%d", opr->reg);
			break;
		case OprTag_c1reg:
			strcpy(buf, REGC1[regnmsw[1]][opr->reg]);
			break;
		case OprTag_imm:
		case OprTag_jtarget:
			shex(buf, opr->data);
			break;
		case OprTag_shamt:
			sprintf(buf, "%d", (int)(opr->data));
			break;
		case OprTag_regoffset:
			shex(buf, opr->data);
			sprintf(&buf[strlen(buf)], "(%s)", REGNAME[regnmsw[0]][opr->reg]);
			break;
		case OprTag_code20:
		case OprTag_code25:
			sprintf(buf, "0x%x", opr->data);
			break;
		default:
			sprintf(buf, "?type?(0x%x)", opr->tag);
			fprintf(stderr, "Panic unknown oprand type 0x%x\n", opr->tag);
			break;
	}
}

void format_disasm(Disasm_result *dis, char *buf)
{
	char *s;

	sprintf(buf, "%08x: %08x", dis->addr, dis->data);
	s = &buf[strlen(buf)];
	if ( dis->mnemonic[0] )
	{
		int i;
		char *sa;

		sprintf(s, "  %-8s", dis->mnemonic);
		sa = &s[strlen(s)];
		for ( i = 0; dis->operands[i].tag; i += 1 )
		{
			format_operand(&dis->operands[i], sa);
			if ( dis->operands[i + 1].tag )
			{
				strcat(sa, ", ");
			}
			sa += strlen(sa);
		}
	}
}

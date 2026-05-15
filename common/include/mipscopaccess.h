/**
 * @file
 * MIPS COP coprocessor access
 */

#ifndef __MIPSCOPACCESS_H__
#define __MIPSCOPACCESS_H__

#include <tamtypes.h>

enum mips_cop0_reg
{
	/** Programmable register to select TLB entry for reading or writing (purpose: MMU) */
	COP0_REG_Index = 0,
	/** Pseudo-random counter for TLB replacement (purpose: MMU) */
	COP0_REG_Random = 1,
	/** Low half of TLB entry for even PFN (Physical page number) (purpose: MMU) */
	COP0_REG_EntryLo0 = 2,
	/** Low half of TLB entry for odd PFN (Physical page number) (purpose: MMU) */
	COP0_REG_EntryLo1 = 3,
	/** Pointer to kernel virtual PTE table (purpose: Exception) */
	COP0_REG_Context = 4,
	/** Mask that sets the TLB page size (purpose: MMU) */
	COP0_REG_PageMask = 5,
	/** Number of wired TLB entries (purpose: MMU) */
	COP0_REG_Wired = 6,
	/** Bad virtual address (purpose: Exception) */
	COP0_REG_BadVAddr = 8,
	/** Timer compare (purpose: Exception) */
	COP0_REG_Count = 9,
	/** High half of TLB entry(Virtual page number and ASID) (purpose: MMU) */
	COP0_REG_EntryHi = 10,
	/** Timer compare (purpose: Exception) */
	COP0_REG_Compare = 11,
	/** Processor Status Register (purpose: Exception) */
	COP0_REG_Status = 12,
	/** Cause of the last exception taken (purpose: Exception) */
	COP0_REG_Cause = 13,
	/** Exception Program Counter (purpose: Exception) */
	COP0_REG_EPC = 14,
	/** Processor Revision Identifier (purpose: MMU) */
	COP0_REG_PRId = 15,
	/** Configuration Register (purpose: MMU) */
	COP0_REG_Config = 16,
	/** Bad Physical Address (purpose: Exception) */
	COP0_REG_BadPAddr = 23,
	/** This is used for Debug function (purpose: Debug) */
	COP0_REG_Debug = 24,
	/** Performance Counter and Control Register (purpose: Exception) */
	COP0_REG_Perf = 25,
	/** Cache Tag register(low bits) (purpose: MMU) */
	COP0_REG_TagLo = 28,
	/** Cache Tag register(high bits) (purpose: MMU) */
	COP0_REG_TagHi = 29,
	/** Error Exception Program Counter (purpose: Exception) */
	COP0_REG_ErrorPC = 30,
};

static inline __attribute__((__always_inline__)) u32 get_mips_cop_reg(const u32 cop, const u32 idx)
{
	u32 val;

	__asm__ __volatile__("mfc%[cop]\t%[val], $%[idx]\n" : [val] "=r"(val) : [cop] "i"(cop), [idx] "i"(idx));
	return val;
}

static inline __attribute__((__always_inline__)) void set_mips_cop_reg(const u32 cop, const u32 idx, u32 val)
{
	__asm__ __volatile__("mtc%[cop]\t%[val], $%[idx]\n" :: [val] "r"(val), [cop] "i"(cop), [idx] "i"(idx));
}

#endif /* __MIPSCOPACCESS_H__ */

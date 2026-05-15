/*
# _____     ___ ____     ___ ____
#  ____|   |    ____|   |        | |____|
# |     ___|   |____ ___|    ____| |    \    PS2DEV Open Source Project.
#-----------------------------------------------------------------------
# Licenced under Academic Free License version 2.0
# Review ps2sdk README & LICENSE files for further details.
*/

#include <kernel.h>
#include <ee_cop0_defs.h>
#include <mipscopaccess.h>

/* // Doesn't work, but here are the COP0 register definitions:
#define Index    $0
#define EntryLo0 $2
#define EntryLo1 $3
#define PageMask $5
#define Wired    $6
#define EntryHi  $10 */

struct SyscallPatchData
{
    unsigned int syscall;
    void *function;
};

char *_kExecArg[] __attribute__((section(".kExecArg")));
char *_kExecArg[] = {NULL}; /* 0x80075330 */

// Function prototypes:
int PutTLBEntry(unsigned int PageMask, unsigned int EntryHi, unsigned int EntryLo0, unsigned int EntryLo1);
int SetTLBEntry(unsigned int index, unsigned int PageMask, unsigned int EntryHi, unsigned int EntryLo0, unsigned int EntryLo1);
int GetTLBEntry(unsigned int index, unsigned int *PageMask, unsigned int *EntryHi, unsigned int *EntryLo0, unsigned int *EntryLo1);
int ProbeTLBEntry(unsigned int EntryHi, unsigned int *PageMask, unsigned int *EntryLo0, unsigned int *EntryLo1);
int ExpandScratchPad(unsigned int page);

static const struct SyscallPatchData SyscallPatchData[] = {
    {0x55, &PutTLBEntry},
    {0x56, &SetTLBEntry},
    {0x57, &GetTLBEntry},
    {0x58, &ProbeTLBEntry},
    {0x59, &ExpandScratchPad},
    {0x03, &_kExecArg},
};

int _start(int syscall) __attribute__((section(".start")));

/* 0x80075000 */
int _start(int syscall)
{
    unsigned int i;

    for (i = 0; i < 6; i++) {
        if (SyscallPatchData[i].syscall == syscall) {
            return ((unsigned int)SyscallPatchData[i].function);
        }
    }

    return 0;
}

/* 0x80075038 */
int PutTLBEntry(unsigned int PageMask, unsigned int EntryHi, unsigned int EntryLo0, unsigned int EntryLo1)
{
    int result;

    switch (EntryHi >> 24) {
        case 0x40:
        case 0x30:
        case 0x20:
        case 0x00:
            set_mips_cop_reg(0, COP0_REG_PageMask, PageMask);
            set_mips_cop_reg(0, COP0_REG_EntryHi, EntryHi);
            set_mips_cop_reg(0, COP0_REG_EntryLo0, EntryLo0);
            set_mips_cop_reg(0, COP0_REG_EntryLo1, EntryLo1);
            EE_SYNCP();
            __asm__ __volatile__("tlbwr\n");
            EE_SYNCP();
            __asm__ __volatile__("tlbp\n");
            EE_SYNCP();
            result = get_mips_cop_reg(0, COP0_REG_Index);
            break;
        case 0x50:
        case 0x10:
        default: // SP193: I don't remember seeing a default case. Anyway... Keeping Compilers Happy (TM).
            result = -1;
    }

    return result;
}

/* 0x800750c8 */
int SetTLBEntry(unsigned int index, unsigned int PageMask, unsigned int EntryHi, unsigned int EntryLo0, unsigned int EntryLo1)
{
    int result;

    if (index < 0x30) {
        set_mips_cop_reg(0, COP0_REG_Index, index);
        set_mips_cop_reg(0, COP0_REG_PageMask, PageMask);
        set_mips_cop_reg(0, COP0_REG_EntryHi, EntryHi);
        set_mips_cop_reg(0, COP0_REG_EntryLo0, EntryLo0);
        set_mips_cop_reg(0, COP0_REG_EntryLo1, EntryLo1);
        EE_SYNCP();
        __asm__ __volatile__("tlbwi\n");
        EE_SYNCP();

        result = index;
    } else
        result = -1;

    return result;
}

/* 0x80075108 */
int GetTLBEntry(unsigned int index, unsigned int *PageMask, unsigned int *EntryHi, unsigned int *EntryLo0, unsigned int *EntryLo1)
{
    int result;

    if (index < 0x30) {
        set_mips_cop_reg(0, COP0_REG_Index, index);
        EE_SYNCP();
        __asm__ __volatile__("tlbr\n");
        EE_SYNCP();
        *PageMask = get_mips_cop_reg(0, COP0_REG_PageMask);
        *EntryHi = get_mips_cop_reg(0, COP0_REG_EntryHi);
        *EntryLo0 = get_mips_cop_reg(0, COP0_REG_EntryLo0);
        *EntryLo1 = get_mips_cop_reg(0, COP0_REG_EntryLo1);

        result = index;
    } else
        result = -1;

    return result;
}

/* 0x80075158 */
int ProbeTLBEntry(unsigned int EntryHi, unsigned int *PageMask, unsigned int *EntryLo0, unsigned int *EntryLo1)
{
    int result, index;

    set_mips_cop_reg(0, COP0_REG_EntryHi, EntryHi);
    EE_SYNCP();
    __asm__ __volatile__("tlbp\n");
    EE_SYNCP();
    index = get_mips_cop_reg(0, COP0_REG_Index);

    if (index >= 0) {
        __asm__ __volatile__("tlbr\n");
        EE_SYNCP();
        *PageMask = get_mips_cop_reg(0, COP0_REG_PageMask);
        *EntryLo0 = get_mips_cop_reg(0, COP0_REG_EntryLo0);
        *EntryLo1 = get_mips_cop_reg(0, COP0_REG_EntryLo1);

        result = index;
    } else
        result = -1;

    return result;
}

/* 0x800751a8 */
int ExpandScratchPad(unsigned int page)
{
    int result;

    if (!(page & 0xFFF)) {
        if (0xFFFFE < page - 1) {
            int index;
            unsigned int PageMask, EntryHi, EntryLo0, EntryLo1;
            if ((index = ProbeTLBEntry(0x70004000, &PageMask, &EntryLo0, &EntryLo1)) >= 0) {
#if 0
                // This condition is always false due to the preceding check on the "page" variable.
                if (page == 0) {
                    EntryHi = 0xE0010000 + ((index - 1) << 13);

                    set_mips_cop_reg(0, COP0_REG_Wired, get_mips_cop_reg(0, COP0_REG_Wired) + 0xFFFF);
                    set_mips_cop_reg(0, COP0_REG_Index, index);
                    set_mips_cop_reg(0, COP0_REG_PageMask, 0);
                    set_mips_cop_reg(0, COP0_REG_EntryHi, EntryHi);
                    set_mips_cop_reg(0, COP0_REG_EntryLo0, 0);
                    set_mips_cop_reg(0, COP0_REG_EntryLo1, 0);
                    EE_SYNCP();
                    __asm__ __volatile__("tlbwi\n");
                    EE_SYNCP();
                } else
#endif
                {
                    set_mips_cop_reg(0, COP0_REG_Wired, get_mips_cop_reg(0, COP0_REG_Wired) + 1);
                }
            }

            if (page != 0) {
                /*	Not sure why this code saves the EntryLo0 and EntryLo1 values on the stack, and sets a word on the stack to zero, but does not use them:

                    0($sp)=0
                    4($sp)=$v0=(page+0x1000&0xFFFFF000)>>6|0x1F
                    8($sp)=$a0=(page&0xFFFFF000)>>6|0x1F */

                EntryHi  = 0x70004000;
                EntryLo0 = ((page + 0x1000) & 0xFFFFF000) >> 6 | 0x1F;
                EntryLo1 = (page & 0xFFFFF000) >> 6 | 0x1F;

                set_mips_cop_reg(0, COP0_REG_Index, index);
                set_mips_cop_reg(0, COP0_REG_PageMask, 0);
                set_mips_cop_reg(0, COP0_REG_EntryHi, EntryHi);
                set_mips_cop_reg(0, COP0_REG_EntryLo0, EntryLo0);
                set_mips_cop_reg(0, COP0_REG_EntryLo1, EntryLo1);
                EE_SYNCP();
                __asm__ __volatile__("tlbwi\n");
                EE_SYNCP();

                result = index;
            } else
                result = 0;
        } else
            result = -1;
    } else
        result = -1;

    return result;
}

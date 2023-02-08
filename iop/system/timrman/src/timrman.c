#include "timrman.h"
#include "kerr.h"
#include "xtimrman.h"

#include "irx_imports.h"

#include <tamtypes.h>

IRX_ID("Timer_Manager", 2, 2);
extern struct irx_export_table _exp_timrman;

#define PRID $15

#define _mfc0(reg)                        \
    ({                                    \
        u32 val;                          \
        __asm__ volatile("mfc0 %0, " #reg \
                         : "=r"(val));    \
        val;                              \
    })

#define mfc0(reg) _mfc0(reg)

enum {
    NUM_TIMERS = 6,

    TMR0_COUNT = 0xBF801100,
    TMR1_COUNT = 0xBF801110,
    TMR2_COUNT = 0xBF801120,
    TMR3_COUNT = 0xBF801480,
    TMR4_COUNT = 0xBF801490,
    TMR5_COUNT = 0xBF8014A0,

    TMR_CTRL_OFFSET = 4,
    TMR_CMP_OFFSET  = 8,

    TMR_UNK = 0xBF801450,

    TMR_HOLD_REG  = 0xBF8014B0,
    TMR_HOLD_MODE = 0xBF8014C0,

    // reśet counter on irq
    TMR_CTRL_ZERO_RETURN = 1 << 3,

    // IRQ enable
    TMR_CTRL_CMP_IRQ_ENABLE  = 1 << 4,
    TMR_CTRL_OVFL_IRQ_ENABLE = 1 << 5,
    TMR_CTRL_IRQ_ENABLE      = 1 << 11,
    TMR_CTRL_IRQ_REPEAT      = 1 << 6,

    TMR_CTRL_EXT_SIGNAL = 1 << 8,

    // IRQ status
    TMR_CTRL_CMP_IRQ  = 1 << 11,
    TMR_CTRL_OVFL_IRQ = 1 << 12,
};

struct Timer
{
    u32 addr;
    u8 sources;
    u8 size;
    u16 max_prescale;
    u8 irq;
    s8 users;
    u8 has_irq_handler;
    u32 mode;
    u32 ctrl_config;
    u16 timeup_flags;
    u16 overflow_flags;
    u32 compare_value;
    unsigned int (*timeup_handler)(void *);
    void *timeup_common;
    unsigned int (*overflow_handler)(void *);
    void *overflow_common;
};

static struct Timer sTimerTable[NUM_TIMERS] = {
    {
        .addr         = TMR0_COUNT,
        .sources      = 0xB,
        .size         = 16,
        .max_prescale = 1,
        .irq          = 4,
    },
    {
        .addr         = TMR1_COUNT,
        .sources      = 0xD,
        .size         = 16,
        .max_prescale = 1,
        .irq          = 5,
    },
    {
        .addr         = TMR2_COUNT,
        .sources      = 1,
        .size         = 16,
        .max_prescale = 8,
        .irq          = 6,
    },
    {
        .addr         = TMR3_COUNT,
        .sources      = 5,
        .size         = 32,
        .max_prescale = 1,
        .irq          = 0xE,
    },
    {
        .addr         = TMR4_COUNT,
        .sources      = 1,
        .size         = 32,
        .max_prescale = 256,
        .irq          = 0xF,
    },
    {
        .addr         = TMR5_COUNT,
        .sources      = 1,
        .size         = 32,
        .max_prescale = 256,
        .irq          = 0x10,
    },
};

static int sIndexMap[NUM_TIMERS] = {2, 5, 4, 3, 0, 1};

u32 timer0() { return _lh(TMR0_COUNT); }
u32 timer1() { return _lh(TMR1_COUNT); }
u32 timer2() { return _lh(TMR2_COUNT); }
u32 timer3() { return _lw(TMR3_COUNT); }
u32 timer4() { return _lw(TMR4_COUNT); }
u32 timer5() { return _lw(TMR5_COUNT); }

static u32 (*sTimerCountFun[NUM_TIMERS])() = {timer0, timer1, timer2, timer3, timer4, timer5};

int _start(int argc, char **argv)
{
    int prid, ret;
    prid = mfc0(PRID);

    if (prid < 16) {
        return MODULE_NO_RESIDENT_END;
    }

    if ((_lw(TMR_UNK) & 8) != 0) {
        return MODULE_NO_RESIDENT_END;
    }

    for (int i = 5; i >= 0; i--) {
        sTimerTable[i].users = 0;
    }

    ret = RegisterLibraryEntries(&_exp_timrman);
    if (ret != 0) {
        return MODULE_NO_RESIDENT_END;
    }

    return MODULE_RESIDENT_END;
}

void *GetTimersTable()
{
    return sTimerTable;
}

int AllocHardTimer(int source, int size, int prescale)
{
    struct Timer *timer;
    int oldstat, timid;

    if (QueryIntrContext()) {
        return KE_ILLEGAL_CONTEXT;
    }

    CpuSuspendIntr(&oldstat);

    for (int i = 0; i < NUM_TIMERS; i++) {
        timer = &sTimerTable[sIndexMap[i]];
        if (timer->users != 0) {
            continue;
        }

        if ((timer->sources & source) == 0) {
            continue;
        }

        if (timer->size != size || timer->max_prescale < prescale) {
            continue;
        }

        timid = ((sIndexMap[i] + 1) << 28) | (timer->addr >> 4);
        SetTimerMode(timid, 0);

        DisableIntr(timer->irq, NULL);
        timer->users++;
        timer->ctrl_config      = 0;
        timer->timeup_flags     = 0;
        timer->overflow_flags   = 0;
        timer->timeup_handler   = 0;
        timer->overflow_handler = 0;
        CpuResumeIntr(oldstat);

        return timid;
    }

    CpuResumeIntr(oldstat);
    return KE_NO_TIMER;
}

int ReferHardTimer(int source, int size, int mode, int modemask)
{
    struct Timer *timer;
    int oldstat;

    if (QueryIntrContext()) {
        return KE_ILLEGAL_CONTEXT;
    }

    CpuSuspendIntr(&oldstat);


    for (int i = 0; i < NUM_TIMERS; i++) {
        timer = &sTimerTable[i];
        if (timer->users == 0) {
            continue;
        }

        if (timer->mode == 0) {
            continue;
        }

        if ((timer->sources & source) == 0) {
            continue;
        }

        if (timer->size != size || (timer->mode & modemask) != mode) {
            continue;
        }

        timer->users++;
        CpuResumeIntr(oldstat);

        // Should be correct, but reads very poorly in decompilers
        return ((i << 28) + 1) | (timer->addr >> 4);
    }

    CpuResumeIntr(oldstat);
    return KE_NO_TIMER;
}

int FreeHardTimer(int timid)
{
    struct Timer *timer;
    u32 timer_idx = (timid >> 28) - 1;
    int oldstat;

    if (QueryIntrContext()) {
        return KE_ILLEGAL_CONTEXT;
    }

    timer = &sTimerTable[timer_idx];
    CpuSuspendIntr(&oldstat);

    if (!timer->users) {
        CpuResumeIntr(oldstat);
        return KE_ILLEGAL_TIMERID;
    }

    timer->users--;
    // what is this shift for (it's a bnez, not sign compare)?
    if ((timer->users << 24) != 0) {
        SetTimerMode(timid, 0);
        DisableIntr(timer->irq, NULL);
        timer->ctrl_config      = 0;
        timer->timeup_flags     = 0;
        timer->overflow_flags   = 0;
        timer->timeup_handler   = 0;
        timer->overflow_handler = 0;

        if (timer->has_irq_handler) {
            ReleaseIntrHandler(timer->irq);
        }

        timer->has_irq_handler = 0;
    }

    CpuResumeIntr(oldstat);
    return 0;
}

void SetTimerMode(int timid, int mode)
{
    u32 timer = (timid >> 28) - 1;
    if (timer >= NUM_TIMERS) {
        return;
    }

    sTimerTable[timer].mode = mode;
    _sh(mode, (timid << 4) + 4);
}

u32 GetTimerMode(int timid)
{
    u32 timer = (timid >> 28) - 1;
    if (timer >= NUM_TIMERS) {
        return KE_ILLEGAL_TIMERID;
    }

    return sTimerTable[timer].mode;
}

u32 GetTimerStatus(int timid)
{
    return _lh((timid << 4) + 4);
}

void SetTimerCounter(int timid, u32 count)
{
    u32 addr = timid << 4;

    if (addr & 0x400) {
        _sw(count, addr);
    } else {
        _sh(count, addr);
    }
}

u32 GetTimerCounter(int timid)
{
    u32 addr = timid << 4;

    if (addr & 0x400) {
        return _lw(addr);
    } else {
        return _lh(addr);
    }
}

void SetTimerCompare(int timid, u32 compare)
{
    u32 addr = timid << 4;

    if (addr & 0x400) {
        _sw(compare, addr + 8);
    } else {
        _sh(compare, addr + 8);
    }
}

u32 GetTimerCompare(int timid)
{
    u32 addr = timid << 4;

    if (addr & 0x400) {
        return _lw(addr + 8);
    } else {
        return _lh(addr + 8);
    }
}

void SetHoldMode(int holdnum, int mode)
{
    u32 hold = _lw(TMR_HOLD_MODE);
    hold &= ~(0xF << (4 * holdnum));
    hold |= (mode & 0xF) << (4 * holdnum);
    _sw(TMR_HOLD_MODE, hold);
}

u32 GetHoldMode(int holdnum)
{
    return _lw(TMR_HOLD_MODE >> (holdnum * 4)) & 0xF;
}

u32 GetHoldReg(int holdnum)
{
    return _lw(TMR_HOLD_REG + (holdnum * 4));
}

int GetHardTimerIntrCode(int timid)
{
    u32 timer = (timid >> 28) - 1;
    if (timer >= NUM_TIMERS) {
        return KE_ILLEGAL_TIMERID;
    }

    return sTimerTable[timer].irq;
}

// no clue on official name
u32 (*GetTimerReadFunc(int timid))()
{
    u32 timer = (timid >> 28) - 1;
    if (timer >= NUM_TIMERS) {
        return (void *)KE_ILLEGAL_TIMERID;
    }

    return sTimerCountFun[timer];
}


int SetTimerHandler(int timid, unsigned long comparevalue, unsigned int (*timeuphandler)(void *), void *common)
{
    struct Timer *timer;
    int oldstat;
    u32 timer_idx = (timid >> 28) - 1;
    if (timer_idx >= NUM_TIMERS) {
        return KE_ILLEGAL_TIMERID;
    }

    CpuSuspendIntr(&oldstat);
    timer = &sTimerTable[timer_idx];

    if (timer->mode != 0) {
        CpuResumeIntr(oldstat);
        return -156;
    }

    timer->compare_value  = comparevalue;
    timer->timeup_handler = timeuphandler;
    timer->timeup_common  = common;
    if (timeuphandler) {
        timer->timeup_flags = TMR_CTRL_ZERO_RETURN | TMR_CTRL_CMP_IRQ_ENABLE | TMR_CTRL_IRQ_REPEAT;
    } else {
        timer->timeup_flags = 0;
    }

    CpuResumeIntr(oldstat);
    return 0;
}

int SetOverflowHandler(int timid, unsigned int (*handler)(void *), void *common)
{
    struct Timer *timer;
    int oldstat;

    u32 timer_idx = (timid >> 28) - 1;
    if (timer_idx >= NUM_TIMERS) {
        return KE_ILLEGAL_TIMERID;
    }

    CpuSuspendIntr(&oldstat);
    timer = &sTimerTable[timer_idx];

    if (timer->mode != 0) {
        CpuResumeIntr(oldstat);
        return -156;
    }

    timer->overflow_handler = handler;
    timer->overflow_common  = common;
    if (handler != NULL) {
        timer->overflow_flags = TMR_CTRL_OVFL_IRQ_ENABLE | TMR_CTRL_IRQ_REPEAT;
    } else {
        timer->overflow_flags = 0;
    }

    return 0;
}

static int irqHandler(void *arg)
{
    struct Timer *timer = arg;
    u32 stop            = 0;
    u32 cb_ret;

    u32 ctrl = _lh(timer->addr + TMR_CTRL_OFFSET);
    if (ctrl & TMR_CTRL_OVFL_IRQ) {
        cb_ret = timer->overflow_handler(timer->overflow_common);
        if (cb_ret == 0) {
            stop = 1 << 5;
        }
    }

    if (ctrl & TMR_CTRL_CMP_IRQ) {
        cb_ret = timer->timeup_handler(timer->timeup_common);
        if (cb_ret == 0) {
            stop |= 1 << 4;
        } else {
            if (timer->size == 16) {
                _sh(cb_ret, timer->addr + TMR_CMP_OFFSET);
            } else {
                _sw(cb_ret, timer->addr + TMR_CMP_OFFSET);
            }
        }
    }

    if (stop || timer->mode == 0) {
        timer->mode = 0;
        _sh(0, timer->addr + TMR_CTRL_OFFSET);
        return 0;
    }

    return 1;
}

int SetupHardTimer(int timid, int source, int mode, int prescale)
{
    u32 timer_idx = (timid >> 28) - 1;
    struct Timer *timer;
    int ret, oldstat;
    u32 new_mode;

    if (QueryIntrContext()) {
        return KE_ILLEGAL_CONTEXT;
    }

    if (timer_idx >= NUM_TIMERS) {
        return KE_ILLEGAL_TIMERID;
    }

    CpuSuspendIntr(&oldstat);
    timer = &sTimerTable[timer_idx];

    if (timer->mode != 0) {
        CpuResumeIntr(oldstat);
        return -154;
    }

    timer->ctrl_config = 0;
    if (!timer->has_irq_handler) {
        DisableIntr(timer->irq, NULL);

        ret = RegisterIntrHandler(timer->irq, 1, irqHandler, timer);
        if (ret < 0) {
            CpuResumeIntr(oldstat);
            return ret;
        }

        timer->has_irq_handler = 1;
    }

    switch (mode) {
        case 0:
        case 1:
        case 3:
        case 5:
        case 7:
            break;
        default:
            CpuResumeIntr(oldstat);
            return KE_ILLEGAL_MODE;
    }

    new_mode = mode | 0x80000000;

    switch (source) {
        case 1:
            break;
        case 2:
        case 4:
            new_mode |= TMR_CTRL_EXT_SIGNAL;
            break;
        default:
            CpuResumeIntr(oldstat);
            return -152;
    }

    switch (prescale) {
        case 8:
            if (timer_idx >= 3) {
                new_mode |= 0x2000;
            } else {
                new_mode |= 0x200;
            }
            break;
        case 16:
            new_mode |= 0x4000;
            break;
        case 256:
            new_mode |= 0x6000;
            break;
        default:
            CpuResumeIntr(oldstat);
            return -153;
    }

    timer->ctrl_config = new_mode;
    CpuResumeIntr(oldstat);
    return 0;
}

int StartHardTimer(int timid)
{
    int oldstat;
    struct Timer *timer;
    u32 timer_idx = (timid >> 28) - 1;
    if (timer_idx >= NUM_TIMERS) {
        return KE_ILLEGAL_TIMERID;
    }

    CpuSuspendIntr(&oldstat);
    timer = &sTimerTable[timer_idx];
    if (timer->mode != 0) {
        CpuResumeIntr(oldstat);
        return -154;
    }

    if (timer->ctrl_config != 0) {
        CpuResumeIntr(oldstat);
        return -155;
    }

    _sh(0, timer->addr + TMR_CTRL_OFFSET);

    if (timer->timeup_flags) {
        if (timer_idx > 3) {
            _sw(timer->compare_value, timer->addr + TMR_CMP_OFFSET);
        } else {
            _sh(timer->compare_value, timer->addr + TMR_CMP_OFFSET);
        }
    }

    timer->mode = timer->ctrl_config | timer->timeup_flags | timer->overflow_flags;
    CpuResumeIntr(oldstat);
    _sh(timer->mode, timer->addr + TMR_CTRL_OFFSET);

    if (timer->mode & 0x30) {
        EnableIntr(timer->irq);
    }

    return 0;
}

int StopHardTimer(int timid)
{
    int oldstat;
    struct Timer *timer;
    u32 timer_idx = (timid >> 28) - 1;
    if (timer_idx >= NUM_TIMERS) {
        return KE_ILLEGAL_TIMERID;
    }

    CpuSuspendIntr(&oldstat);
    timer = &sTimerTable[timer_idx];
    if (timer->mode == 0) {
        CpuResumeIntr(oldstat);
        return -156;
    }

    if (timer->mode & 0x30) {
        DisableIntr(timer->irq, NULL);
    }

    timer->mode = 0;
    _sh(0, timer->addr + 4);

    return 0;
}

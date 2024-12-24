/*
# _____     ___ ____     ___ ____
#  ____|   |    ____|   |        | |____|
# |     ___|   |____ ___|    ____| |    \    PS2DEV Open Source Project.
#-----------------------------------------------------------------------
# Copyright ps2dev - http://www.ps2dev.org
# Licenced under Academic Free License version 2.0
# Review ps2sdk README & LICENSE files for further details.
*/

#include "irx_imports.h"

#include "thcommon.h"

IRX_ID("Temporarily_Thread_monitor", 1, 3);
// Based on the module from SCE SDK 3.1.0.

struct thmon_thcpuwatch_param
{
    volatile int m_start_stop_status;
    struct thread *m_thread;
    int m_sample_count;
    // cppcheck-suppress unusedStructMember
    int m_unused_c;
    int m_barchart;
    int m_verbose;
};

struct thmon_rtlist_item
{
    struct thread *m_thread_ptr;
    int m_thread_id;
    int m_flags;
    iop_sys_clock_t m_old_clocks;
    iop_sys_clock_t m_new_clocks;
};

struct thmon_almlist_item
{
    iop_sys_clock_t m_target;
    unsigned int m_alarm_id;
    void *m_cb;
    void *m_userptr;
};

static int CreateThmonThread(u32 attr, void (*thread)(void *), u32 priority, u32 stacksize, u32 option);
static int do_get_thread_count();
static void ThmonMonitorThread(struct thmon_thcpuwatch_param *usrptr);
static void ThmonMainThread(void *arg);

static const char *g_help_msg[] =
    {
        "\nsimple thread monitor program =====================================\n",
        "-- thlist [-v] [-a] [<thid>]  -- thread list\n",
        "-- rdlist [-v]                -- ready thread list\n",
        "-- sllist [-v]                -- sleeping thread list\n",
        "-- dllist [-v]                -- delayed thread list\n",
        "-- rtlist [-v] [<times>]      -- thread running time list\n",
        "-- semlist [-w] [-v]          -- semaphore list\n",
        "-- evlist  [-w] [-v]          -- eventflag list\n",
        "-- msglist                    -- messagebox list\n",
        "-- vpllist                    -- Vpool list\n",
        "-- fpllist                    -- Fpool list\n",
        "-- almlist                    -- Alarm list\n",
        "-- cpuwatch [-b] [-br] [-v] [<samples>] -- cpu time watching <samples/sec>\n",
        "-- thwatch [-b] [-br] [-v] <thid> [<samples>] -- thread time watching <samples/sec>\n",
        "-- freemem                    -- report free memory size\n",
        "-- vblank <on/off>            -- vblank interrupt on/off\n",
        "-- /                          -- repeat last command\n",
        NULL,
};
static const char *g_th_status_short = "000RunRdy333Wat555666777Sus999aaabbbWSudddeeefffDom???";
static const char *g_th_wait_short   = "  SlDlSeEvMbVpFp  ";
static struct thread_context *g_ThbaseInternal;
static int g_monitorThreadId;
static int g_tty9_fd;
static struct thmon_rtlist_item *g_rtlist_nondormant;
static char g_progspace1[52];
static char g_progspace2[52];
static struct thmon_thcpuwatch_param g_thcpuwatch_info;
static char g_PreviousCommand[208];
static char g_CommandBuffer[200];

int _start(int ac, char **av)
{
    int margen_val;
    int main_thread_id;

    margen_val = 10;
    if (ac >= 2 && !strncmp(av[1], "-th=", 4)) {
        margen_val = strtol(av[1] + 4, 0, 10) + 10;
        printf("margen = %d\n", margen_val);
    }
    g_thcpuwatch_info.m_start_stop_status = 0;
    g_tty9_fd                             = open("tty9:", 3);
    g_ThbaseInternal                      = (struct thread_context *)GetThreadmanData();
    main_thread_id                        = CreateThmonThread(
        0x2000000u,
        ThmonMainThread,
        6u,
        28 * (do_get_thread_count() + margen_val) + 3584,
        0);
    g_monitorThreadId = CreateThmonThread(0x2000000u, (void (*)(void *))ThmonMonitorThread, 4u, 0x800u, 0);
    if (main_thread_id <= 0 || g_monitorThreadId <= 0)
        return 1;
    StartThread(main_thread_id, 0);
    return 0;
}

static int CreateThmonThread(u32 attr, void (*thread)(void *), u32 priority, u32 stacksize, u32 option)
{
    iop_thread_t thparam;

    thparam.attr      = attr;
    thparam.thread    = thread;
    thparam.priority  = priority;
    thparam.stacksize = stacksize;
    thparam.option    = option;
    return CreateThread(&thparam);
}

static int do_get_thread_count()
{
    struct thread *thread;
    int i;
    int state;

    i = 0;
    CpuSuspendIntr(&state);
    list_for_each (thread, &g_ThbaseInternal->thread_list, thread_list) {
        i += 1;
    }
    CpuResumeIntr(state);
    return i;
}

static void PrintMessage(const char *msg)
{
    fdprintf(g_tty9_fd, "%s\n", msg);
}

static char *RemoveLeadingWhitespaces(char *str)
{
    for (; *str == ' ' || *str == '\t'; str += 1) {
        *str = 0;
    }
    return str;
}

static char *GetEndOfString(char *str)
{
    for (; *str && *str != ' ' && *str != '\t'; str += 1)
        ;
    return str;
}

static void DisplayHelpMessage()
{
    int i;

    for (i = 0; g_help_msg[i]; i += 1) {
        fdprintf(g_tty9_fd, g_help_msg[i]);
        if (i + 1 == 23 * ((i + 1) / 23) && g_help_msg[i + 1]) {
            fdprintf(g_tty9_fd, " more ? ");
            if (fdgetc(g_tty9_fd) != 'y') {
                fdprintf(g_tty9_fd, "\n");
                break;
            }
            fdprintf(g_tty9_fd, "\r");
        }
    }
}

static int do_parse_hex(char *str)
{
    int retres;

    retres = 0;
    for (; *str && isxdigit(*str); str += 1) {
        retres *= 16;
        retres += toupper(*str);
        retres -= isdigit(*str) ? '0' : '7';
    }
    return retres;
}

static char *get_module_name_from_addr(unsigned int entry_addr)
{
    ModuleInfo_t *image_info;

    for (image_info = GetLoadcoreInternalData()->image_info; image_info; image_info = image_info->next) {
        if (entry_addr >= image_info->text_start && entry_addr < image_info->text_start + image_info->text_size)
            return image_info->name;
    }
    return 0;
}

static void DisplayThreadInformation(struct thread *threadstruct, int is_verbose, int is_brief)
{
    if (is_brief)
        fdprintf(g_tty9_fd, "    ");
    fdprintf(
        g_tty9_fd,
        "%3x %07x %08x %07x %.3s ",
        threadstruct->tag.id,
        MAKE_HANDLE(threadstruct),
        threadstruct->attr,
        threadstruct->option,
        &g_th_status_short[3 * (char)threadstruct->status]);
    fdprintf(
        g_tty9_fd,
        "%06x %06x %04x %06x %3d %3d",
        threadstruct->entry,
        threadstruct->stack_top,
        threadstruct->stack_size,
        threadstruct->gp,
        (s16)threadstruct->priority,
        (s16)threadstruct->init_priority);
    if (is_brief) {
        fdprintf(g_tty9_fd, "\n");
    } else {
        fdprintf(
            g_tty9_fd,
            " %c%c ",
            g_th_wait_short[2 * (s16)threadstruct->wait_type],
            g_th_wait_short[2 * (s16)threadstruct->wait_type + 1]);
        switch ((s16)threadstruct->wait_type) {
            case 1:
                fdprintf(g_tty9_fd, "%7s%2x\n", "", threadstruct->wakeup_count);
                break;
            case 2:
                fdprintf(g_tty9_fd, "%6x %2x\n", threadstruct->wait_usecs, threadstruct->wakeup_count);
                break;
            case 0:
            default:
                fdprintf(
                    g_tty9_fd,
                    "%07x%2x\n",
                    MAKE_HANDLE(threadstruct->wait_usecs),
                    threadstruct->wakeup_count);
                break;
        }
    }
    if (is_verbose) {
        char *module_name_from_addr;

        module_name_from_addr = get_module_name_from_addr((unsigned int)threadstruct->entry);
        if (module_name_from_addr) {
            if (is_brief)
                fdprintf(g_tty9_fd, "    ");
            fdprintf(g_tty9_fd, "    Name=%s\n", module_name_from_addr);
        }
    }
    if (threadstruct->saved_regs && threadstruct->status != 16) {
        if (is_brief)
            fdprintf(g_tty9_fd, "    ");
        fdprintf(
            g_tty9_fd,
            "    PC=%06x  RA=%06x  SP=%06x  Context_addr/mask=%08x/%08x\n",
            threadstruct->saved_regs->pc,
            threadstruct->saved_regs->ra,
            threadstruct->saved_regs->sp,
            threadstruct->saved_regs,
            threadstruct->saved_regs->unk);
    }
    if (is_verbose) {
        unsigned int i;

        for (i = 0; i < (threadstruct->stack_size / 4); i += 1) {
            if (((u32 *)threadstruct->stack_top)[i] != 0xFFFFFFFF && ((u32 *)threadstruct->stack_top)[i] != 0x11111111)
                break;
        }
        if (is_brief)
            fdprintf(g_tty9_fd, "    ");
        fdprintf(g_tty9_fd, "    Free_Stack_Size=%06x", i * 4);
        fdprintf(
            g_tty9_fd,
            "  I-preempt/T-preenmpt/release counts=%d,%d,%d\n",
            threadstruct->irq_preemption_count,
            threadstruct->thread_preemption_count,
            threadstruct->release_count);
        if (is_brief && threadstruct->wait_type == 4) {
            fdprintf(g_tty9_fd, "    ");
            fdprintf(
                g_tty9_fd,
                "    Event bitpattern, mode=0x%08x, 0x%x\n",
                threadstruct->event_bits,
                (s16)threadstruct->event_mode);
        }
    }
}

static void thlist_cmd_handler(char *cmdparam)
{
    char *cmdparam_cur;
    int is_flag_v;
    int is_flag_a;
    int is_flag_i;
    int is_hexval;
    char *EndOfString;
    struct thread *thread;
    iop_thread_info_t thstatus;

    is_flag_v = 0;
    is_flag_a = 0;
    is_flag_i = 0;
    is_hexval = 0;
    for (cmdparam_cur = cmdparam; *cmdparam_cur; cmdparam_cur = EndOfString) {
        EndOfString = GetEndOfString(cmdparam_cur);
        if (*EndOfString) {
            *EndOfString = 0;
            EndOfString  = RemoveLeadingWhitespaces(EndOfString + 1);
        }
        if (!strcmp(cmdparam_cur, "-v"))
            is_flag_v = 1;
        if (!strcmp(cmdparam_cur, "-a"))
            is_flag_a = 1;
        if (!strcmp(cmdparam_cur, "-i"))
            is_flag_i = 1;
        if (*cmdparam_cur != '-')
            is_hexval = do_parse_hex(cmdparam_cur);
    }
    if (is_hexval) {
        if (is_hexval < g_ThbaseInternal->thread_id) {
            list_for_each (thread, &g_ThbaseInternal->thread_list, thread_list) {
                if (thread->tag.id == is_hexval) {
                    break;
                }
            }
            if (!thread || thread->tag.id != is_hexval) {
                fdprintf(g_tty9_fd, "thread #%d not found \n", is_hexval);
                return;
            }
            is_hexval = MAKE_HANDLE(thread);
        }
        if (ReferThreadStatus(is_hexval, &thstatus) < 0) {
            fdprintf(g_tty9_fd, "thid:%x not found \n", is_hexval);
            return;
        }
        fdprintf(g_tty9_fd, "    THID    ATTR     OPTION  STS ENTRY  STACK  SSIZE GP     CP  IP WT WID   WUC\n");
        DisplayThreadInformation(
            (struct thread *)HANDLE_PTR(is_hexval),
            is_flag_v,
            0);
    } else {
        fdprintf(g_tty9_fd, "    THID    ATTR     OPTION  STS ENTRY  STACK  SSIZE GP     CP  IP WT WID   WUC\n");
        list_for_each (thread, &g_ThbaseInternal->thread_list, thread_list) {
            if (ReferThreadStatus(MAKE_HANDLE(thread), &thstatus) < 0) {
                fdprintf(
                    g_tty9_fd,
                    "thid = %x not found \n",
                    MAKE_HANDLE(thread));
                continue;
            }
            if ((is_flag_a || thstatus.status != 16) && (is_flag_i || thread != g_ThbaseInternal->idle_thread)) {
                DisplayThreadInformation(thread, is_flag_v, 0);
            }
        }
    }
}

static void do_rtlist_handle_clock(int is_flag_v_or_c)
{
    struct thread *thread;
    char *module_name_from_addr;
    iop_thread_info_t thstatus;
    iop_sys_clock_t clks;
    u32 sec_val;
    u32 usec_val;

    fdprintf(g_tty9_fd, "    THID    CLOCK               SEC\n");
    list_for_each (thread, &g_ThbaseInternal->thread_list, thread_list) {
        if (ReferThreadStatus(MAKE_HANDLE(thread), &thstatus) < 0 || thstatus.status == 16) {
            fdprintf(
                g_tty9_fd,
                "thid = %x not found \n",
                (unsigned int)MAKE_HANDLE(thread));
            continue;
        }
        clks.hi = thread->run_clocks_hi;
        clks.lo = thread->run_clocks_lo;
        SysClock2USec(&clks, &sec_val, &usec_val);
        if (thread == g_ThbaseInternal->idle_thread) {
            fdprintf(
                g_tty9_fd,
                "            %08x_%08x %5d.%06d",
                thread->run_clocks_hi,
                thread->run_clocks_lo,
                sec_val,
                usec_val);
            fdprintf(g_tty9_fd, "  System Idle time\n");
        } else {
            fdprintf(
                g_tty9_fd,
                "%3x %07x %08x_%08x %5d.%06d",
                thread->tag.id,
                (unsigned int)MAKE_HANDLE(thread),
                thread->run_clocks_hi,
                thread->run_clocks_lo,
                sec_val,
                usec_val);
            if (is_flag_v_or_c) {
                module_name_from_addr = get_module_name_from_addr((unsigned int)thstatus.entry);
                if (module_name_from_addr)
                    fdprintf(g_tty9_fd, "    Name=%s", module_name_from_addr);
            }
            fdprintf(g_tty9_fd, "\n");
        }
    }
    fdprintf(
        g_tty9_fd,
        "  Total thread switch count = %d, comes out of idle count = %d\n",
        g_ThbaseInternal->thread_switch_count,
        g_ThbaseInternal->idle_thread->irq_preemption_count);
}

static void sys_clock_subtract(
    const iop_sys_clock_t *pStart,
    const iop_sys_clock_t *pEnd,
    iop_sys_clock_t *pRet)
{
    iop_sys_clock_t diff;

    diff.lo = pEnd->lo - pStart->lo;
    diff.hi = pEnd->hi - pStart->hi - (pStart->lo > pEnd->lo);
    memcpy(pRet, &diff, sizeof(diff));
}

static void do_rtlist_handle_priority(int is_flag_v_or_c, int int_value)
{
    struct thread *thread;
    struct thread *curptr;
    char *module_name_from_addr;
    iop_sys_clock_t timebefore;
    iop_sys_clock_t timeafter;
    iop_sys_clock_t timeres;
    u32 sec;
    u32 usec;

    g_rtlist_nondormant = __builtin_alloca(sizeof(struct thmon_rtlist_item) * do_get_thread_count());
    for (; int_value > 0; int_value -= 1) {
        struct thmon_rtlist_item *curelem1;
        struct thmon_rtlist_item *curelem2;

        curelem1 = g_rtlist_nondormant;
        list_for_each (thread, &g_ThbaseInternal->thread_list, thread_list) {
            curelem1->m_thread_ptr = 0;
            if (thread->status != 16) {
                curelem1->m_thread_ptr    = thread;
                curelem1->m_thread_id     = thread->tag.id;
                curelem1->m_old_clocks.hi = thread->run_clocks_hi;
                curelem1->m_old_clocks.lo = thread->run_clocks_lo;
                curelem1 += 1;
                curelem1->m_thread_ptr = 0;
            }
        }
        GetSystemTime(&timebefore);
        DelayThread(1000000);
        GetSystemTime(&timeafter);
        for (curelem2 = g_rtlist_nondormant; curelem2->m_thread_ptr; curelem2 += 1) {
            if (curelem2->m_thread_id == curelem2->m_thread_ptr->tag.id) {
                curelem2->m_flags         = 0;
                curelem2->m_new_clocks.hi = curelem2->m_thread_ptr->run_clocks_hi;
                curelem2->m_new_clocks.lo = curelem2->m_thread_ptr->run_clocks_lo;
            } else {
                curelem2->m_flags = -1;
            }
        }
        sys_clock_subtract(&timeafter, &timebefore, &timeafter);
        fdprintf(g_tty9_fd, "    THID    PRIO   USE\n");
        for (curelem2 = g_rtlist_nondormant; curelem2->m_thread_ptr; curelem2 += 1) {
            if ((curelem2->m_flags & 0x80000000) != 0) {
                continue;
            }
            curptr = curelem2->m_thread_ptr;
            sys_clock_subtract(&curelem2->m_new_clocks, &curelem2->m_old_clocks, &timeres);
            // Unofficial: use SysClock2USec
            // TODO: giant magic math function timeafter, timeres
            SysClock2USec(&timeafter, &sec, &usec);
            if (curptr != g_ThbaseInternal->idle_thread) {
                fdprintf(
                    g_tty9_fd,
                    "%3x %07x  %3d %3d.%03d %%",
                    curptr->tag.id,
                    MAKE_HANDLE(curptr),
                    (s16)curptr->priority,
                    sec,
                    usec);
                if (is_flag_v_or_c) {
                    module_name_from_addr = get_module_name_from_addr((unsigned int)curptr->entry);
                    if (module_name_from_addr)
                        fdprintf(g_tty9_fd, "    Name=%s", module_name_from_addr);
                }
                fdprintf(g_tty9_fd, "\n");
                DelayThread(400);
            } else {
                fdprintf(
                    g_tty9_fd,
                    "  %6d/%6d/ %3d.%03d %%",
                    g_ThbaseInternal->thread_switch_count,
                    curptr->irq_preemption_count,
                    sec,
                    usec);
                fdprintf(g_tty9_fd, " Total switch / comes out of idle / System Idle time\n");
            }
        }
    }
}

static void rtlist_cmd_handler(char *cmdparam)
{
    char *cmdparam_cur;
    int is_flag_v_or_c;
    int int_value;
    char *EndOfString;

    is_flag_v_or_c = 0;
    int_value      = 0;
    for (cmdparam_cur = cmdparam; *cmdparam_cur; cmdparam_cur = EndOfString) {
        EndOfString = GetEndOfString(cmdparam_cur);
        if (*EndOfString) {
            *EndOfString = 0;
            EndOfString  = RemoveLeadingWhitespaces(EndOfString + 1);
        }
        if (!strcmp(cmdparam_cur, "-v"))
            is_flag_v_or_c |= 1u;
        if (!strcmp(cmdparam_cur, "-c"))
            is_flag_v_or_c |= 2u;
        if (*cmdparam_cur != '-')
            int_value = strtol(cmdparam_cur, 0, 10);
    }
    if (int_value)
        do_rtlist_handle_priority(is_flag_v_or_c, int_value);
    else
        do_rtlist_handle_clock(is_flag_v_or_c);
}

static char *do_output_progress_bar_newline(int curlen)
{
    int curlen_div_2000;

    curlen_div_2000 = curlen / 2000;
    strncpy(g_progspace1, "----+----+----+----+----+----+----+----+----+----+", curlen / 2000);
    g_progspace1[curlen_div_2000]     = '\n';
    g_progspace1[curlen_div_2000 + 1] = '\0';
    return g_progspace1;
}

static char *do_output_progress_bar_carriageret(int curlen)
{
    int curlen_div_2000;

    curlen_div_2000 = curlen / 2000;
    strncpy(g_progspace2, "----+----+----+----+----+----+----+----+----+----+", curlen / 2000);
    strncpy(&g_progspace2[curlen_div_2000], "                                                  ", 50 - curlen_div_2000);
    strcpy(&g_progspace2[50], "\r");
    return g_progspace2;
}

static void ThmonMonitorThread(struct thmon_thcpuwatch_param *usrptr)
{
    int chrcnt;
    int delayval;
    struct thread *target_thread;
    int math_magic_3;
    iop_sys_clock_t sys_time_clks_2;
    iop_sys_clock_t sys_time_clks_1;
    iop_sys_clock_t target_thread_clks_2;
    iop_sys_clock_t target_thread_clks_1;
    iop_sys_clock_t this_thread_clks_2;
    iop_sys_clock_t this_thread_clks_1;
    iop_sys_clock_t tmp_time_clks_1;
    iop_sys_clock_t tmp_time_clks_2;
    struct thread *this_thread;
    u32 sec;
    u32 usec;

    chrcnt                      = 0;
    delayval                    = 1000 * (1000 / usrptr->m_sample_count);
    usrptr->m_start_stop_status = 1;
    target_thread               = usrptr->m_thread;
    this_thread                 = (struct thread *)HANDLE_PTR(GetThreadId());
    this_thread_clks_2.lo       = 0;
    this_thread_clks_2.hi       = 0;
    while (usrptr->m_start_stop_status != 2) {
        if (target_thread) {
            target_thread_clks_1.hi = target_thread->run_clocks_hi;
            target_thread_clks_1.lo = target_thread->run_clocks_lo;
            DelayThread(delayval);
            GetSystemTime(&sys_time_clks_1);
            sys_clock_subtract(
                &sys_time_clks_1,
                &sys_time_clks_2,
                &tmp_time_clks_1);
            sys_time_clks_2         = sys_time_clks_1;
            target_thread_clks_2.hi = target_thread->run_clocks_hi;
            target_thread_clks_2.lo = target_thread->run_clocks_lo;
            this_thread_clks_1.hi   = this_thread->run_clocks_hi;
            this_thread_clks_1.lo   = this_thread->run_clocks_lo;
            sys_clock_subtract(
                &this_thread_clks_1,
                &this_thread_clks_2,
                &this_thread_clks_2);
            sys_clock_subtract(
                &tmp_time_clks_1,
                &this_thread_clks_2,
                &tmp_time_clks_1);
            this_thread_clks_2 = this_thread_clks_1;
            sys_clock_subtract(
                &target_thread_clks_2,
                &target_thread_clks_1,
                &tmp_time_clks_2);
            // Unofficial: use SysClock2USec
            // TODO: giant magic math function tmp_time_clks_1, tmp_time_clks_2
            SysClock2USec(&tmp_time_clks_1, &sec, &usec);
            switch (usrptr->m_barchart) {
                case 0:
                    Kprintf("%3d.%03d %07x", sec, usec, target_thread->saved_regs->pc);
                    if (usrptr->m_verbose) {
                        Kprintf(
                            " %6d/%6d/%6d ",
                            target_thread->irq_preemption_count,
                            target_thread->thread_preemption_count,
                            target_thread->release_count);
                        Kprintf("%c", (chrcnt <= 0) ? ' ' : '\n');
                        ++chrcnt;
                        if (chrcnt >= 2)
                            chrcnt = 0;
                    } else {
                        Kprintf("%c", (chrcnt < 4) ? ' ' : '\n');
                        ++chrcnt;
                        if (chrcnt >= 5)
                            chrcnt = 0;
                    }
                    break;
                case 1:
                case 2:
                    Kprintf(
                        "%3d.%03d:%s",
                        sec,
                        usec,
                        (usrptr->m_barchart == 1) ? do_output_progress_bar_newline(sec * 1000 + usec) : do_output_progress_bar_carriageret(sec * 1000 + usec));
                    break;
                default:
                    break;
            }
        } else {
            target_thread_clks_1.hi = g_ThbaseInternal->idle_thread->run_clocks_hi;
            target_thread_clks_1.lo = g_ThbaseInternal->idle_thread->run_clocks_lo;
            GetSystemTime(&sys_time_clks_2);
            DelayThread(delayval);
            GetSystemTime(&sys_time_clks_1);
            target_thread_clks_2.hi = g_ThbaseInternal->idle_thread->run_clocks_hi;
            target_thread_clks_2.lo = g_ThbaseInternal->idle_thread->run_clocks_lo;
            sys_clock_subtract(
                &sys_time_clks_1,
                &sys_time_clks_2,
                &tmp_time_clks_1);
            sys_clock_subtract(
                &target_thread_clks_2,
                &target_thread_clks_1,
                &tmp_time_clks_2);
            // Unofficial: use SysClock2USec
            // TODO: giant magic math function tmp_time_clks_1, tmp_time_clks_2
            SysClock2USec(&tmp_time_clks_1, &sec, &usec);
            math_magic_3 = 100000 - (sec * 1000 + usec);
            switch (usrptr->m_barchart) {
                case 0:
                    Kprintf("%3d.%03d", math_magic_3 / 1000, math_magic_3 % 1000);
                    if (usrptr->m_verbose) {
                        Kprintf(
                            " %6d/%6d%s",
                            g_ThbaseInternal->thread_switch_count,
                            g_ThbaseInternal->idle_thread->irq_preemption_count,
                            (chrcnt < 2) ? "    " : "\n");
                        ++chrcnt;
                        if (chrcnt >= 3)
                            chrcnt = 0;
                    } else {
                        Kprintf((chrcnt < 8) ? "  " : "\n");
                        ++chrcnt;
                        if (chrcnt >= 9)
                            chrcnt = 0;
                    }
                    break;
                case 1:
                case 2:
                    Kprintf(
                        "%3d.%03d:%s",
                        math_magic_3 / 1000,
                        math_magic_3 % 1000,
                        (usrptr->m_barchart == 1) ? do_output_progress_bar_newline(math_magic_3) : do_output_progress_bar_carriageret(math_magic_3));
                    break;
                default:
                    break;
            }
        }
    }
    usrptr->m_start_stop_status = 0;
}

static void do_stop_current_thcpuwatch()
{
    if (!g_thcpuwatch_info.m_start_stop_status) {
        return;
    }
    g_thcpuwatch_info.m_start_stop_status = 2;
    while (g_thcpuwatch_info.m_start_stop_status) {
        DelayThread(1000);
    }
}

static void thwatch_cmd_handler(char *cmdparam)
{
    char *cmdparam_cur;
    int is_flag_b_or_br;
    int integer_value;
    int hex_value;
    int is_flag_v;
    char *EndOfString;

    iop_thread_info_t thstatus;

    is_flag_b_or_br = 0;
    integer_value   = 10;
    hex_value       = 0;
    is_flag_v       = 0;
    for (cmdparam_cur = cmdparam; *cmdparam_cur; cmdparam_cur = EndOfString) {
        EndOfString = GetEndOfString(cmdparam_cur);
        if (*EndOfString) {
            *EndOfString = 0;
            EndOfString  = RemoveLeadingWhitespaces(EndOfString + 1);
        }
        if (!strcmp(cmdparam_cur, "-b"))
            is_flag_b_or_br = 1;
        if (!strcmp(cmdparam_cur, "-br"))
            is_flag_b_or_br = 2;
        if (!strcmp(cmdparam_cur, "-v"))
            is_flag_v = 1;
        if (*cmdparam_cur != '-') {
            if (hex_value)
                integer_value = strtol(cmdparam_cur, 0, 10);
            else
                hex_value = do_parse_hex(cmdparam_cur);
        }
    }
    do_stop_current_thcpuwatch();
    if (!hex_value) {
        return;
    }
    if (hex_value < g_ThbaseInternal->thread_id) {
        struct thread *thread;

        list_for_each (thread, &g_ThbaseInternal->thread_list, thread_list) {
            if (thread->tag.id == hex_value)
                break;
        }
        if (!thread || thread->tag.id != hex_value) {
            fdprintf(g_tty9_fd, "thread #%d not found \n", hex_value);
            return;
        }
        hex_value = MAKE_HANDLE(thread);
    }
    g_thcpuwatch_info.m_barchart     = is_flag_b_or_br;
    g_thcpuwatch_info.m_sample_count = integer_value;
    g_thcpuwatch_info.m_verbose      = is_flag_v;
    if (hex_value <= 0 || ReferThreadStatus(hex_value, &thstatus)) {
        fdprintf(g_tty9_fd, "thid:%x not found \n", hex_value);
    } else if (integer_value > 0) {
        g_thcpuwatch_info.m_thread = (struct thread *)HANDLE_PTR(hex_value);
        fdprintf(g_tty9_fd, "Thread %x time watching and print to ITTYK\n", hex_value);
        StartThread(g_monitorThreadId, &g_thcpuwatch_info);
    }
}

static void cpuwatch_cmd_handler(char *cmdparam)
{
    char *cmdparam_cur;
    int is_flag_b_or_br;
    int int_value;
    int is_flag_v;
    char *EndOfString;

    is_flag_b_or_br = 0;
    int_value       = 10;
    is_flag_v       = 0;
    for (cmdparam_cur = cmdparam; *cmdparam_cur; cmdparam_cur = EndOfString) {
        EndOfString = GetEndOfString(cmdparam_cur);
        if (*EndOfString) {
            *EndOfString = 0;
            EndOfString  = RemoveLeadingWhitespaces(EndOfString + 1);
        }
        if (!strcmp(cmdparam_cur, "-b"))
            is_flag_b_or_br = 1;
        if (!strcmp(cmdparam_cur, "-br"))
            is_flag_b_or_br = 2;
        if (!strcmp(cmdparam_cur, "-v"))
            is_flag_v = 1;
        if (*cmdparam_cur != '-')
            int_value = strtol(cmdparam_cur, 0, 10);
    }
    do_stop_current_thcpuwatch();
    if (int_value > 0) {
        g_thcpuwatch_info.m_barchart     = is_flag_b_or_br;
        g_thcpuwatch_info.m_sample_count = int_value;
        g_thcpuwatch_info.m_verbose      = is_flag_v;
        g_thcpuwatch_info.m_thread       = 0;
        fdprintf(g_tty9_fd, "CPU time watching and print to ITTYK\n");
        StartThread(g_monitorThreadId, &g_thcpuwatch_info);
    }
}

static void rdlist_cmd_handler(char *cmdparam)
{
    char *cmdparam_cur;
    int is_flag_v;
    int is_flag_i;
    char *EndOfString;
    int i;
    struct thread *thread;
    iop_thread_info_t thstatus;

    is_flag_v = 0;
    is_flag_i = 0;
    for (cmdparam_cur = cmdparam; *cmdparam_cur; cmdparam_cur = EndOfString) {
        EndOfString = GetEndOfString(cmdparam_cur);
        if (*EndOfString) {
            *EndOfString = 0;
            EndOfString  = RemoveLeadingWhitespaces(EndOfString + 1);
        }
        if (!strcmp(cmdparam_cur, "-v"))
            is_flag_v = 1;
        if (!strcmp(cmdparam_cur, "-i"))
            is_flag_i = 1;
    }
    fdprintf(g_tty9_fd, "    THID    ATTR     OPTION  STS ENTRY  STACK  SSIZE GP     CP  IP WT WID   WUC\n");
    for (i = 1; i < (is_flag_i + 127); i += 1) {
        if (list_empty(&g_ThbaseInternal->ready_queue[i])) {
            continue;
        }
        list_for_each (thread, &g_ThbaseInternal->ready_queue[i], queue) {
            if (!ReferThreadStatus(MAKE_HANDLE(thread), &thstatus))
                DisplayThreadInformation(thread, is_flag_v, 0);
        }
    }
}

static void sllist_cmd_handler(char *cmdparam)
{
    char *cmdparam_cur;
    int is_flag_v;
    char *EndOfString;
    struct thread *thread;
    iop_thread_info_t thstatus;

    is_flag_v = 0;
    for (cmdparam_cur = cmdparam; *cmdparam_cur; cmdparam_cur = EndOfString) {
        EndOfString = GetEndOfString(cmdparam_cur);
        if (*EndOfString) {
            *EndOfString = 0;
            EndOfString  = RemoveLeadingWhitespaces(EndOfString + 1);
        }
        if (!strcmp(cmdparam_cur, "-v"))
            is_flag_v = 1;
    }
    fdprintf(g_tty9_fd, "    THID    ATTR     OPTION  STS ENTRY  STACK  SSIZE GP     CP  IP WT WID   WUC\n");
    list_for_each (thread, &g_ThbaseInternal->sleep_queue, queue) {
        if (ReferThreadStatus(MAKE_HANDLE(thread), &thstatus) < 0) {
            fdprintf(g_tty9_fd, "thid = %x not found \n", MAKE_HANDLE(thread));
            continue;
        }
        if (thread != g_ThbaseInternal->idle_thread)
            DisplayThreadInformation(thread, is_flag_v, 0);
    }
}

static void dlist_print_thread_info(struct thread *threadstruct, int is_verbose)
{
    fdprintf(
        g_tty9_fd,
        "%3x %07x %08x %07x %.3s ",
        threadstruct->tag.id,
        MAKE_HANDLE(threadstruct),
        threadstruct->attr,
        threadstruct->option,
        &g_th_status_short[3 * (char)threadstruct->status]);
    fdprintf(
        g_tty9_fd,
        "%06x %06x %04x %06x %3d %3d %d\n",
        threadstruct->entry,
        threadstruct->stack_top,
        threadstruct->stack_size,
        threadstruct->gp,
        (s16)threadstruct->priority,
        (s16)threadstruct->init_priority,
        threadstruct->wait_usecs);
    if (is_verbose) {
        char *module_name_from_addr;

        module_name_from_addr = get_module_name_from_addr((unsigned int)threadstruct->entry);
        if (module_name_from_addr)
            fdprintf(g_tty9_fd, "    Name=%s\n", module_name_from_addr);
    }
    if (threadstruct->saved_regs && threadstruct->status != 16)
        fdprintf(
            g_tty9_fd,
            "    PC=%06x  RA=%06x  SP=%06x  Context_addr/mask=%08x/%08x\n",
            threadstruct->saved_regs->pc,
            threadstruct->saved_regs->ra,
            threadstruct->saved_regs->sp,
            threadstruct->saved_regs,
            threadstruct->saved_regs->unk);
    if (is_verbose) {
        unsigned int i;

        for (i = 0; i < (threadstruct->stack_size / 4); i += 1) {
            if (((u32 *)threadstruct->stack_top)[i] != 0xFFFFFFFF && ((u32 *)threadstruct->stack_top)[i] != 0x11111111)
                break;
        }
        fdprintf(g_tty9_fd, "    Free_Stack_Size=%06x\n", i * 4);
    }
}

static void dllist_cmd_handler(char *cmdparam)
{
    char *cmdparam_cur;
    int is_flag_v;
    char *EndOfString;
    struct thread *thread;
    iop_thread_info_t thstatus;

    is_flag_v = 0;
    for (cmdparam_cur = cmdparam; *cmdparam_cur; cmdparam_cur = EndOfString) {
        EndOfString = GetEndOfString(cmdparam_cur);
        if (*EndOfString) {
            *EndOfString = 0;
            EndOfString  = RemoveLeadingWhitespaces(EndOfString + 1);
        }
        if (!strcmp(cmdparam_cur, "-v"))
            is_flag_v = 1;
    }
    fdprintf(g_tty9_fd, "    THID    ATTR     OPTION  STS ENTRY  STACK  SSIZE GP     CP  IP USEC\n");
    list_for_each (thread, &g_ThbaseInternal->delay_queue, queue) {
        if (ReferThreadStatus(MAKE_HANDLE(thread), &thstatus) < 0) {
            fdprintf(g_tty9_fd, "thid = %x not found \n", MAKE_HANDLE(thread));
            continue;
        }
        if (thread != g_ThbaseInternal->idle_thread)
            dlist_print_thread_info(thread, is_flag_v);
    }
}

static void ShowSemList(char *cmdparam)
{
    char *cmdparam_cur;
    int is_flag_v_count;
    int is_flag_w_count;
    char *EndOfString;
    struct semaphore *sema;
    struct thread *waiter;
    iop_sema_info_t semastatus;
    iop_thread_info_t thstatus;

    is_flag_v_count = 0;
    is_flag_w_count = 0;
    for (cmdparam_cur = cmdparam; *cmdparam_cur; cmdparam_cur = EndOfString) {
        EndOfString = GetEndOfString(cmdparam_cur);
        if (*EndOfString) {
            *EndOfString = 0;
            EndOfString  = RemoveLeadingWhitespaces(EndOfString + 1);
        }
        if (!strcmp(cmdparam_cur, "-v"))
            ++is_flag_v_count;
        if (!strcmp(cmdparam_cur, "-w"))
            ++is_flag_w_count;
    }
    if (list_empty(&g_ThbaseInternal->semaphore)) {
        return;
    }
    fdprintf(g_tty9_fd, "    SEMID   ATTR     OPTION   iCnt  cCnt  mCnt  waitThreads\n");
    list_for_each (sema, &g_ThbaseInternal->semaphore, sema_list) {
        if (ReferSemaStatus(MAKE_HANDLE(sema), &semastatus) < 0) {
            continue;
        }
        if (semastatus.numWaitThreads > 0 || !is_flag_w_count) {
            fdprintf(
                g_tty9_fd,
                "%3x %07x %08x %08x %05d %5d %5d  %5d\n",
                sema->tag.id,
                MAKE_HANDLE(sema),
                semastatus.attr,
                semastatus.option,
                semastatus.initial,
                semastatus.current,
                semastatus.max,
                semastatus.numWaitThreads);
        }
        if (semastatus.numWaitThreads > 0 && is_flag_w_count) {
            fdprintf(g_tty9_fd, "        THID    ATTR     OPTION  STS ENTRY  STACK  SSIZE GP     CP  IP\n");
            list_for_each_safe (waiter, &sema->event.waiters, queue) {
                ReferThreadStatus(MAKE_HANDLE(waiter), &thstatus);
                DisplayThreadInformation(waiter, is_flag_v_count, 1);
            }
        }
    }
}

static void ShowEventFlagList(char *cmdparam)
{
    char *cmdparam_cur;
    int is_flag_v_count;
    int is_flag_w_count;
    char *EndOfString;
    struct event_flag *evf;
    struct thread *waiter;
    iop_event_info_t efstatus;
    iop_thread_info_t thstatus;

    is_flag_v_count = 0;
    is_flag_w_count = 0;
    for (cmdparam_cur = cmdparam; *cmdparam_cur; cmdparam_cur = EndOfString) {
        EndOfString = GetEndOfString(cmdparam_cur);
        if (*EndOfString) {
            *EndOfString = 0;
            EndOfString  = RemoveLeadingWhitespaces(EndOfString + 1);
        }
        if (!strcmp(cmdparam_cur, "-v"))
            ++is_flag_v_count;
        if (!strcmp(cmdparam_cur, "-w"))
            ++is_flag_w_count;
    }
    if (list_empty(&g_ThbaseInternal->event_flag)) {
        return;
    }
    fdprintf(g_tty9_fd, "    EVID    ATTR     OPTION   iPattern cPattern waitThreads\n");
    list_for_each (evf, &g_ThbaseInternal->event_flag, evf_list) {
        if (ReferEventFlagStatus(MAKE_HANDLE(evf), &efstatus) < 0) {
            continue;
        }
        if (efstatus.numThreads > 0 || !is_flag_w_count) {
            fdprintf(
                g_tty9_fd,
                "%3x %07x %08x %08x %08x %08x  %5d\n",
                evf->tag.id,
                MAKE_HANDLE(evf),
                efstatus.attr,
                efstatus.option,
                efstatus.initBits,
                efstatus.currBits,
                efstatus.numThreads);
        }
        if (efstatus.numThreads > 0 && is_flag_w_count) {
            fdprintf(g_tty9_fd, "        THID    ATTR     OPTION  STS ENTRY  STACK  SSIZE GP     CP  IP\n");
            list_for_each_safe (waiter, &evf->event.waiters, queue) {
                ReferThreadStatus(MAKE_HANDLE(waiter), &thstatus);
                DisplayThreadInformation(waiter, is_flag_v_count, 1);
            }
        }
    }
}

static void ShowMsgbxList()
{
    struct mbox *mbx;
    iop_mbx_status_t mbxstatus;

    if (list_empty(&g_ThbaseInternal->mbox)) {
        return;
    }
    fdprintf(g_tty9_fd, "    MSGID   ATTR     OPTION   waitThreads  messages\n");
    list_for_each (mbx, &g_ThbaseInternal->mbox, mbox_list) {
        ReferMbxStatus(MAKE_HANDLE(mbx), &mbxstatus);
        fdprintf(
            g_tty9_fd,
            "%3x %07x %08x %08x %5d        %5d\n",
            mbx->tag.id,
            MAKE_HANDLE(mbx),
            mbxstatus.attr,
            mbxstatus.option,
            mbxstatus.numWaitThreads,
            mbxstatus.numMessage);
    }
}

static void ShowVplList()
{
    struct vpool *vpl;
    iop_vpl_info_t vplstatus;

    if (list_empty(&g_ThbaseInternal->vpool)) {
        return;
    }
    fdprintf(g_tty9_fd, "    VPLID   ATTR     OPTION   Size   Free   waitThreads\n");
    list_for_each (vpl, &g_ThbaseInternal->vpool, vpl_list) {
        ReferVplStatus(MAKE_HANDLE(vpl), &vplstatus);
        fdprintf(
            g_tty9_fd,
            "%3x %07x %08x %08x %06x %06x   %5d\n",
            vpl->tag.id,
            MAKE_HANDLE(vpl),
            vplstatus.attr,
            vplstatus.option,
            vplstatus.size,
            vplstatus.freeSize,
            vplstatus.numWaitThreads);
    }
}

static void ShowFplList()
{
    struct fpool *fpl;
    iop_fpl_info_t fplstatus;

    if (list_empty(&g_ThbaseInternal->fpool)) {
        return;
    }
    fdprintf(g_tty9_fd, "    FPLID   ATTR     OPTION   BlkSIze AllBlocks FreeSize waitThreads\n");
    list_for_each (fpl, &g_ThbaseInternal->fpool, fpl_list) {
        ReferFplStatus(MAKE_HANDLE(fpl), &fplstatus);
        fdprintf(
            g_tty9_fd,
            "%3x %07x %08x %08x %06x  %06x    %06x    %5d\n",
            fpl->tag.id,
            MAKE_HANDLE(fpl),
            fplstatus.attr,
            fplstatus.option,
            fplstatus.blockSize,
            fplstatus.numBlocks,
            fplstatus.freeBlocks,
            fplstatus.numWaitThreads);
    }
}

static void DumpReadyQueue()
{
    struct thread *thread;
    int i;
    int state;

    fdprintf(g_tty9_fd, "ready queue ----\n");
    for (i = 0; i < 127; i += 1) {
        if (list_empty(&g_ThbaseInternal->ready_queue[i])) {
            continue;
        }
        CpuSuspendIntr(&state);
        fdprintf(g_tty9_fd, " %3d:%x ", i, &g_ThbaseInternal->ready_queue[i]);
        list_for_each (thread, &g_ThbaseInternal->ready_queue[i], queue)
            fdprintf(g_tty9_fd, " %d(%x) ", thread->tag.id, thread);
        fdprintf(g_tty9_fd, "\n");
        CpuResumeIntr(state);
    }
    fdprintf(g_tty9_fd, "ready map = ");
    for (i = 0; i < 4; i += 1) {
        fdprintf(g_tty9_fd, "%08x ", g_ThbaseInternal->queue_map[i]);
    }
    fdprintf(g_tty9_fd, "\n");
}

static void ToggleThswdisp(const char *cmdparam)
{
    if (!strcmp(cmdparam, "on"))
        g_ThbaseInternal->debug_flags |= 1u;
    if (!strcmp(cmdparam, "off"))
        g_ThbaseInternal->debug_flags &= ~1u;
    if (isdigit(*cmdparam))
        g_ThbaseInternal->debug_flags = strtol(cmdparam, 0, 10);
}

static void ToggleThLED(const char *cmdparam)
{
    if (!strcmp(cmdparam, "on"))
        g_ThbaseInternal->debug_flags |= 0x20u;
    if (!strcmp(cmdparam, "off"))
        g_ThbaseInternal->debug_flags &= ~0x20u;
    if (isdigit(*cmdparam))
        g_ThbaseInternal->debug_flags = strtol(cmdparam, 0, 10);
}

static void ToggleWarnDisp(const char *cmdparam)
{
    if (!strcmp(cmdparam, "on"))
        g_ThbaseInternal->debug_flags |= 8u;
    if (!strcmp(cmdparam, "off"))
        g_ThbaseInternal->debug_flags &= ~8u;
    if (isdigit(*cmdparam))
        g_ThbaseInternal->debug_flags = strtol(cmdparam, 0, 10);
}

static void DumpThreads()
{
    struct thread *thread;

    fdprintf(g_tty9_fd, "===============================\n");
    fdprintf(g_tty9_fd, "tcb list ----\n");
    list_for_each (thread, &g_ThbaseInternal->thread_list, thread_list) {
        fdprintf(
            g_tty9_fd,
            "  %d: tcb=%x sts=0x%x pri=%d ",
            thread->tag.id,
            thread,
            (char)thread->status,
            (s16)thread->priority);
        if ((char)thread->status == 4)
            fdprintf(g_tty9_fd, "wType = %x:%x ", (s16)thread->wait_type, thread->wait_usecs);
        if ((char)thread->status != 1)
            fdprintf(g_tty9_fd, "contx=%x sp=%x", thread->saved_regs, thread->saved_regs->sp);
        fdprintf(g_tty9_fd, "\n");
    }
    fdprintf(g_tty9_fd, "sleep list ----\n");
    if (list_empty(&g_ThbaseInternal->sleep_queue)) {
        return;
    }
    fdprintf(g_tty9_fd, " %x: ", &g_ThbaseInternal->sleep_queue);
    list_for_each (thread, &g_ThbaseInternal->sleep_queue, queue) {
        fdprintf(g_tty9_fd, " %d(%x) ", thread->tag.id, thread);
    }
    fdprintf(g_tty9_fd, "\n");
}

static void ShowAlarmList()
{
    struct thmon_almlist_item *alarm_ents;
    int alarm_cnt;
    struct alarm *alarm;
    int i;
    int state;

    alarm_ents = 0;
    CpuSuspendIntr(&state);
    alarm_cnt = 0;
    if (!list_empty(&g_ThbaseInternal->alarm)) {
        list_for_each (alarm, &g_ThbaseInternal->alarm, alarm_list) {
            alarm_cnt += 1;
        }
        alarm_ents = __builtin_alloca(alarm_cnt * sizeof(struct thmon_almlist_item));
        i          = 0;
        list_for_each (alarm, &g_ThbaseInternal->alarm, alarm_list) {
            alarm_ents[i].m_alarm_id  = alarm->tag.id;
            alarm_ents[i].m_target.lo = alarm->target;
            alarm_ents[i].m_target.hi = alarm->target >> 32;
            alarm_ents[i].m_cb        = alarm->cb;
            alarm_ents[i].m_userptr   = alarm->userptr;
            i += 1;
        }
    }
    CpuResumeIntr(state);
    fdprintf(g_tty9_fd, " NUM    TIME           HANDLER COMMON\n");
    for (i = 0; i < alarm_cnt; i += 1) {
        fdprintf(
            g_tty9_fd,
            "%3x %08x_%08x  %06x, %08x\n",
            alarm_ents[i].m_alarm_id,
            alarm_ents[i].m_target.hi,
            alarm_ents[i].m_target.lo,
            alarm_ents[i].m_cb,
            alarm_ents[i].m_userptr);
    }
}

static void freemem_cmd_handler()
{
    u32 TotalFreeMemSize;

    TotalFreeMemSize = QueryTotalFreeMemSize();
    fdprintf(
        g_tty9_fd,
        "IOP system memory  0x%x(%d) byte free, Max free block size 0x%x\n",
        TotalFreeMemSize,
        TotalFreeMemSize,
        QueryMaxFreeMemSize());
}

static void ThmonMainThread(void *arg)
{
    char *cmdbufsrc;
    char *cmdbufdst;
    char *cmdbuf_trim;
    char *cmdparam;

    (void)arg;

    fdprintf(g_tty9_fd, "\n\n========= simple thread monitor program =========\n");
    fdprintf(g_tty9_fd, "help command is 'help'\n");
    g_PreviousCommand[0] = 0;
    for (;;) {
        fdprintf(g_tty9_fd, " > ");
        if (!fdgets(g_CommandBuffer, g_tty9_fd))
            break;
        PrintMessage(g_CommandBuffer);
        if (!strlen(g_CommandBuffer))
            continue;
        if (!strcmp(g_CommandBuffer, "/")) {
            cmdbufsrc = g_PreviousCommand;
            cmdbufdst = g_CommandBuffer;
        } else {
            cmdbufsrc = g_CommandBuffer;
            cmdbufdst = g_PreviousCommand;
        }
        strcpy(cmdbufdst, cmdbufsrc);
        cmdbuf_trim = RemoveLeadingWhitespaces(g_CommandBuffer);
        cmdparam    = RemoveLeadingWhitespaces(GetEndOfString(cmdbuf_trim));
        if (*cmdbuf_trim == '#')
            continue;
        if (!strncmp("quit", cmdbuf_trim, strlen(cmdbuf_trim)))
            break;
        else if (!strncmp("?", cmdbuf_trim, strlen(cmdbuf_trim)) || !strncmp("help", cmdbuf_trim, strlen(cmdbuf_trim)))
            DisplayHelpMessage();
        else if (!strncmp("thlist", cmdbuf_trim, strlen(cmdbuf_trim)))
            thlist_cmd_handler(cmdparam);
        else if (!strncmp("rdlist", cmdbuf_trim, strlen(cmdbuf_trim)))
            rdlist_cmd_handler(cmdparam);
        else if (!strncmp("sllist", cmdbuf_trim, strlen(cmdbuf_trim)))
            sllist_cmd_handler(cmdparam);
        else if (!strncmp("dllist", cmdbuf_trim, strlen(cmdbuf_trim)))
            dllist_cmd_handler(cmdparam);
        else if (!strncmp("rtlist", cmdbuf_trim, strlen(cmdbuf_trim)))
            rtlist_cmd_handler(cmdparam);
        else if (!strncmp("semlist", cmdbuf_trim, strlen(cmdbuf_trim)))
            ShowSemList(cmdparam);
        else if (!strncmp("evlist", cmdbuf_trim, strlen(cmdbuf_trim)))
            ShowEventFlagList(cmdparam);
        else if (!strncmp("msglist", cmdbuf_trim, strlen(cmdbuf_trim)))
            ShowMsgbxList();
        else if (!strncmp("vpllist", cmdbuf_trim, strlen(cmdbuf_trim)))
            ShowVplList();
        else if (!strncmp("fpllist", cmdbuf_trim, strlen(cmdbuf_trim)))
            ShowFplList();
        else if (!strncmp("almlist", cmdbuf_trim, strlen(cmdbuf_trim)))
            ShowAlarmList();
        else if (!strncmp("freemem", cmdbuf_trim, strlen(cmdbuf_trim)))
            freemem_cmd_handler();
        else if (!strncmp("cpuwatch", cmdbuf_trim, strlen(cmdbuf_trim)))
            cpuwatch_cmd_handler(cmdparam);
        else if (!strncmp("thwatch", cmdbuf_trim, strlen(cmdbuf_trim)))
            thwatch_cmd_handler(cmdparam);
        else if (!strncmp("thswdisp", cmdbuf_trim, strlen(cmdbuf_trim)))
            ToggleThswdisp(cmdparam);
        else if (!strncmp("thled", cmdbuf_trim, strlen(cmdbuf_trim)))
            ToggleThLED(cmdparam);
        else if (!strncmp("warndisp", cmdbuf_trim, strlen(cmdbuf_trim)))
            ToggleWarnDisp(cmdparam);
        else if (!strncmp("dumpthread", cmdbuf_trim, strlen(cmdbuf_trim)))
            DumpThreads();
        else if (!strncmp("dumpready", cmdbuf_trim, strlen(cmdbuf_trim)))
            DumpReadyQueue();
        else
            fdprintf(g_tty9_fd, " ?? \n");
    }
}

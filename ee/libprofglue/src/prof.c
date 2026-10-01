/*
# _____     ___ ____     ___ ____
#  ____|   |    ____|   |        | |____|
# |     ___|   |____ ___|    ____| |    \    PS2DEV Open Source Project.
#-----------------------------------------------------------------------
# Copyright 2001-2004, ps2dev - http://www.ps2dev.org
# Licenced under Academic Free License version 2.0
# Review ps2sdk README & LICENSE files for further details.
*/

#include <stdlib.h>
#include <malloc.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#define GMON_PROF_ON    0
#define GMON_PROF_BUSY  1
#define GMON_PROF_ERROR 2
#define GMON_PROF_OFF   3

#define GMONVERSION 0x00051879

#include <kernel.h>
#include <timer_alarm.h>
#include <ps2prof.h>

/** gmon.out file header */
struct gmonhdr
{
    int lpc;      /* lowest pc address */
    int hpc;      /* highest pc address */
    int ncnt;     /* size of samples + size of header */
    int version;  /* version number */
    int profrate; /* profiling clock rate */
    int resv[3];  /* reserved */
};

/** frompc -> selfpc graph */
struct rawarc
{
    unsigned int frompc;
    unsigned int selfpc;
    unsigned int count;
};

/** context */
struct gmonparam
{
    int state;
    unsigned int lowpc;
    unsigned int highpc;
    unsigned int textsize;
    unsigned int hashfraction;

    int narcs;
    struct rawarc *arcs;

    int nsamples;
    unsigned int *samples;

    int timerId;

    unsigned int pc;
};

/// holds context statistics
static struct gmonparam gp;

/// one histogram per four bytes of text space
#define HISTFRACTION 4

/// define sample frequency - 1000 hz = 1ms
#define SAMPLE_FREQ 1000

/// defined by linker
extern int _ftext;
extern int _etext;

/** Internal timer handler
 */
__attribute__((__no_instrument_function__, __no_profile_instrument_function__))
static uint64_t timer_handler(int id, uint64_t scheduled_time, uint64_t actual_time, void *arg, void *pc_value)
{
    struct gmonparam *current_gp = (struct gmonparam *)arg;

    unsigned int frompc = current_gp->pc;

    if (current_gp->state == GMON_PROF_ON) {
        /* call might come from stack */
        if (frompc >= current_gp->lowpc && frompc <= current_gp->highpc) {
            int e = (frompc - current_gp->lowpc) / current_gp->hashfraction;
            current_gp->samples[e]++;
        }
    }


    current_gp->timerId = iSetTimerAlarm(USec2TimerBusClock(SAMPLE_FREQ), &timer_handler, arg);
    return 0;
}

/** Initializes pg library

    After calculating the text size, __gprof_init() allocates enough
    memory to allow fastest access to arc structures, and some more
    for sampling statistics. Note that this also installs a timer that
    runs at 1000 hert.
*/
__attribute__((__no_instrument_function__, __no_profile_instrument_function__))
void __gprof_init()
{
    memset(&gp, '\0', sizeof(gp));
    gp.state        = GMON_PROF_ON;
    gp.lowpc        = (unsigned int)&_ftext;
    gp.highpc       = (unsigned int)&_etext;
    gp.textsize     = gp.highpc - gp.lowpc;
    gp.hashfraction = HISTFRACTION;

    gp.narcs = (gp.textsize + gp.hashfraction - 1) / gp.hashfraction;
    gp.arcs  = (struct rawarc *)malloc(sizeof(struct rawarc) * gp.narcs);
    if (gp.arcs == NULL) {
        gp.state = GMON_PROF_ERROR;
        return;
    }

    gp.nsamples = (gp.textsize + gp.hashfraction - 1) / gp.hashfraction;
    gp.samples  = (unsigned int *)malloc(sizeof(unsigned int) * gp.nsamples);
    if (gp.samples == NULL) {
        free(gp.arcs);
        gp.arcs  = 0;
        gp.state = GMON_PROF_ERROR;
        return;
    }

    memset((void *)gp.arcs, '\0', gp.narcs * (sizeof(struct rawarc)));
    memset((void *)gp.samples, '\0', gp.nsamples * (sizeof(unsigned int)));


    gp.state   = GMON_PROF_ON;
    gp.timerId = SetTimerAlarm(USec2TimerBusClock(SAMPLE_FREQ), &timer_handler, &gp);
    if (gp.timerId < 0) {
        free(gp.arcs);
        free(gp.samples);
        gp.arcs    = 0;
        gp.samples = 0;
        gp.state   = GMON_PROF_ERROR;
        return;
    }
}

__attribute__((__no_instrument_function__, __no_profile_instrument_function__))
void gprof_start(void)
{
    // There is already a profiling session running, let's stop it and ignore the result
    if (gp.state == GMON_PROF_ON) {
        gprof_stop(NULL, 0);
    }
    __gprof_init();
}

__attribute__((__no_instrument_function__, __no_profile_instrument_function__))
void gprof_stop(const char *filename, int should_dump)
{
    FILE *fp;
    int i;
    struct gmonhdr hdr;

    if (gp.state != GMON_PROF_ON) {
        /* profiling was disabled anyway */
        return;
    }

    /* disable profiling before we make plenty of libc calls */
    gp.state = GMON_PROF_OFF;

    ReleaseTimerAlarm(gp.timerId);

    if (should_dump) {
        fp           = fopen(filename, "wb");
        hdr.lpc      = gp.lowpc;
        hdr.hpc      = gp.highpc;
        hdr.ncnt     = sizeof(hdr) + (sizeof(unsigned int) * gp.nsamples);
        hdr.version  = GMONVERSION;
        hdr.profrate = SAMPLE_FREQ;
        hdr.resv[0]  = 0;
        hdr.resv[1]  = 0;
        hdr.resv[2]  = 0;
        fwrite(&hdr, 1, sizeof(hdr), fp);
        fwrite(gp.samples, gp.nsamples, sizeof(unsigned int), fp);

        for (i = 0; i < gp.narcs; i++) {
            if (gp.arcs[i].count > 0) {
                fwrite(gp.arcs + i, sizeof(struct rawarc), 1, fp);
            }
        }

        fclose(fp);
    }

    // free memory
    free(gp.arcs);
    free(gp.samples);
}

/** Writes gmon.out dump file and stops profiling
    Called from _libcglue_deinit() function; will dump out a gmon.out file 
    at cwd with all collected information.
*/
__attribute__((__no_instrument_function__, __no_profile_instrument_function__))
void __gprof_cleanup()
{
    gprof_stop("gmon.out", 1);
}

/** Internal C handler for _mcount()
    @param frompc    pc address of caller
    @param selfpc    pc address of current function

    Called from mcount.S to make life a bit easier. __mcount is called
    right before a function starts. GCC generates a tiny stub at the very
    beginning of each compiled routine, which eventually brings the
    control to here.
*/
__attribute__((__no_instrument_function__, __no_profile_instrument_function__))
void __mcount(unsigned int frompc, unsigned int selfpc)
{
    int e;
    struct rawarc *arc;

    if (gp.state != GMON_PROF_ON) {
        /* returned off for some reason */
        return;
    }

    frompc = frompc & 0x0FFFFFFF;
    selfpc = selfpc & 0x0FFFFFFF;

    /* call might come from stack */
    if (frompc >= gp.lowpc && frompc <= gp.highpc) {
        gp.pc       = selfpc;
        e           = (frompc - gp.lowpc) / gp.hashfraction;
        arc         = gp.arcs + e;
        arc->frompc = frompc;
        arc->selfpc = selfpc;
        arc->count++;
    }
}

__asm__
(
    "\t" ".set push" "\n"
    "\t" ".set noreorder" "\n"
    "\t" ".set noat" "\n"

    "\t" ".global _mcount" "\n"
    "\t" ".ent _mcount" "\n"

    "\t" "_mcount:" "\n"

    // Generated code already substracts 8 bytes
    // We store our ra, at and a0-a3
    "\t" "\t" "daddiu $sp, $sp, -56" "\n" // Adjust stack pointer for 64-bit registers, 7 registers * 8 bytes each
    "\t" "\t" "sd   $ra, 0($sp)" "\n" // store ra
    "\t" "\t" "sd   $at, 8($sp)" "\n" // at = ra of caller
    "\t" "\t" "sd   $a0, 16($sp)" "\n"
    "\t" "\t" "sd   $a1, 24($sp)" "\n"
    "\t" "\t" "sd   $a2, 32($sp)" "\n"
    "\t" "\t" "sd   $a3, 40($sp)" "\n"

    // Call internal C handler
    "\t" "\t" "move $a0, $at" "\n"
    "\t" "\t" "move $a1, $ra" "\n"
    "\t" "\t" "jal  __mcount" "\n"
    "\t" "\t" "nop" "\n"

    // Restore registers
    "\t" "\t" "ld   $ra, 0($sp)" "\n"
    "\t" "\t" "ld   $at, 8($sp)" "\n"
    "\t" "\t" "ld   $a0, 16($sp)" "\n"
    "\t" "\t" "ld   $a1, 24($sp)" "\n"
    "\t" "\t" "ld   $a2, 32($sp)" "\n"
    "\t" "\t" "ld   $a3, 40($sp)" "\n"
    "\t" "\t" "daddiu $sp, $sp, 56" "\n" // Adjust stack pointer back
    "\t" "\t" "jr   $ra" "\n"
    "\t" "\t" "move $ra, $at" "\n" // restore caller's ra

    "\t" ".end _mcount" "\n"

    "\t" ".set pop" "\n"
);

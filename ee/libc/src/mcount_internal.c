/*
# _____     ___ ____     ___ ____
#  ____|   |    ____|   |        | |____|
# |     ___|   |____ ___|    ____| |    \    PS2DEV Open Source Project.
#-----------------------------------------------------------------------
# Copyright 2001-2005, ps2dev - http://www.ps2dev.org
# Licenced under Academic Free License version 2.0
# Review ps2sdk README & LICENSE files for further details.
#
# $Id: $
# __mcount implementation
*/
#include <stdio.h>
#include <string.h>
#include <kernel.h>
#include <timer.h>

#define	GMON_PROF_ON	0
#define	GMON_PROF_BUSY	1
#define	GMON_PROF_ERROR	2
#define	GMON_PROF_OFF	3

#define GMONVERSION	0x00051879

#ifndef MCOUNT_USE_T1
#define INTC_TIM       kINTC_TIMER0
#define T_COUNT        T0_COUNT
#define T_MODE         T0_MODE
#define T_COMP         T0_COMP
#else
#define INTC_TIM       kINTC_TIMER1
#define T_COUNT        T1_COUNT
#define T_MODE         T1_MODE
#define T_COMP         T1_COMP
#endif

/** gmon.out file header */
struct gmonhdr 
{
	int lpc;        /* lowest pc address */
	int hpc;        /* highest pc address */
	int ncnt;       /* size of samples + size of header */
	int version;    /* version number */
	int profrate;   /* profiling clock rate */
	int resv[3];    /* reserved */
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

	int timer;
};

/// holds context statistics
static struct gmonparam gp;

/// one histogram per four bytes of text space
#define	HISTFRACTION	4

/// have we allocated memory and registered already
static int initialized = 0;

/// defined by linker
extern int _ftext;
extern int _etext;

/* forward declarations */
static void cleanup();
static int profil(int, void *, void *);

/** Initializes pg library

    After calculating the text size, initialize() allocates enough
    memory to allow fastest access to arc structures, and some more
    for sampling statistics. Note that this also installs a timer that
    runs at 1000 hertz on TIM0. You can change the definition above to
    hook on TIM1 instead.
*/
static void initialize()
{
	initialized = 1;
	atexit(cleanup);

	memset(&gp, '\0', sizeof(gp));
	gp.state = GMON_PROF_ON;
	gp.lowpc = (unsigned int)&_ftext;
	gp.highpc = (unsigned int)&_etext;
	gp.textsize = gp.highpc - gp.lowpc;
	gp.hashfraction = HISTFRACTION;

	gp.narcs = (gp.textsize + gp.hashfraction - 1) / gp.hashfraction;
	gp.arcs = (struct rawarc *)malloc(sizeof(struct rawarc) * gp.narcs);
	if (gp.arcs == NULL)
	{
		gp.state = GMON_PROF_ERROR;
		return;
	}

	gp.nsamples = (gp.textsize + gp.hashfraction - 1) / gp.hashfraction;
	gp.samples = (unsigned int *)malloc(sizeof(unsigned int) * gp.nsamples);
	if (gp.samples == NULL)
	{
		free(gp.arcs);
		gp.arcs = 0;
		gp.state = GMON_PROF_ERROR;
		return;
	}

	memset((void *)gp.arcs, '\0', gp.narcs * (sizeof(struct rawarc)));
	memset((void *)gp.samples, '\0', gp.nsamples * (sizeof(unsigned int )));

	gp.timer = AddIntcHandler2(INTC_TIM, profil, 0, 0);
	EnableIntc(INTC_TIM);

	/* fire up timer, every 1 ms */
	*T_COUNT = 0;
	*T_COMP = 586; /* 150MHZ / 256 / 1000 */
	*T_MODE = 2 | (0<<2) | (0<<6) | (1<<7) | (1<<8);
}

/** Writes gmon.out dump file and stops profiling

    Called from atexit() handler; will dump out a host:gmon.out file 
    with all collected information.
*/
static void cleanup()
{
	FILE *fp;
	int i;
	struct gmonhdr hdr;

	if (gp.state != GMON_PROF_ON)
	{
		/* profiling was disabled anyway */
		return;
	}

	/* disable profiling before we make plenty of libc calls */
	gp.state = GMON_PROF_OFF;

	/* kill timer */
	DisableIntc(INTC_TIM);
	if (gp.timer > 0)
	{
		RemoveIntcHandler(INTC_TIM, gp.timer);
	}

	fp = fopen("host:gmon.out", "wb");
	hdr.lpc = gp.lowpc;
	hdr.hpc = gp.highpc;
	hdr.ncnt = sizeof(hdr) + (sizeof(unsigned int) * gp.nsamples);
	hdr.version = GMONVERSION;
	hdr.profrate = 1000; /* 1000 hz = 1ms */
	hdr.resv[0] = 0;
	hdr.resv[1] = 0;
	hdr.resv[2] = 0;
	fwrite(&hdr, 1, sizeof(hdr), fp);
	fwrite(gp.samples, gp.nsamples, sizeof(unsigned int), fp);

	for (i=0; i<gp.narcs; i++)
	{
		if (gp.arcs[i].count > 0)
		{
			fwrite(gp.arcs + i, sizeof(struct rawarc), 1, fp);
		}
	}

	fclose(fp);
}

/** Internal C handler for _mcount()
    @param frompc    pc address of caller
    @param selfpc    pc address of current function

    Called from mcount.S to make life a bit easier. __mcount is called
    right before a function starts. GCC generates a tiny stub at the very
    beginning of each compiled routine, which eventually brings the 
    control to here. 
*/
void __mcount(unsigned int frompc, unsigned int selfpc)
{ 
	int e;
	struct rawarc *arc;

	if (initialized == 0)
	{
		initialize();
	}

	if (gp.state != GMON_PROF_ON)
	{
		/* returned off for some reason */
		return;
	}

	/* call might come from stack */
	if (frompc >= gp.lowpc && frompc <= gp.highpc)
	{
		e = (frompc - gp.lowpc) / gp.hashfraction;
		arc = gp.arcs + e;
		arc->frompc = frompc;
		arc->selfpc = selfpc;
		arc->count++;
	}
}

/** Internal timer handler
    @param ca
    @param arg
    @param addr    pointer to code, when timer expired
    @returns zero

    Called by the Playstation 2 kernel when the timer expires. This
    callback will reset the counter, and update the sample statistics
    of the addr passed as argument.
*/
static int profil(int ca, void *arg, void *addr)
{
	unsigned int frompc = (unsigned int)addr;

	if (gp.state == GMON_PROF_ON)
	{
		/* call might come from stack */
		if (frompc >= gp.lowpc && frompc <= gp.highpc)
		{
			int e = (frompc - gp.lowpc) / gp.hashfraction;
			gp.samples[e]++;
		}
	}

	/* reset counter */
	*T_COUNT = 0;
	/* reset interrupt */
	*T_MODE |= (1 << 10);
	__asm__ volatile("sync.l; ei");
	return 0;
}

/*
# _____     ___ ____     ___ ____
#  ____|   |    ____|   |        | |____|
# |     ___|   |____ ___|    ____| |    \    PS2DEV Open Source Project.
#-----------------------------------------------------------------------
# Copyright 2001-2004, ps2dev - http://www.ps2dev.org
# Licenced under Academic Free License version 2.0
# Review ps2sdk README & LICENSE files for further details.
#
# $Id$
# Logging code used by sio2log.
*/

#include <types.h>
#include <defs.h>
#include <stdio.h>
#include <ioman.h>

#include "log.h"

#define DPRINTF(format, args...) printf("%s: " format, __FUNCTION__ , ## args)

#define FLUSH_COUNT_MAX	4
#define DMA_MAX		1024

static int init = 0;
static int logging = 1;
static int writesize = 0;
static int flushcount = 0;
static u8 log[10 * 1024];
static int logfd = -1;

static const char *logfile = "host0:sio2.log";

static int log_init(void)
{
	logging = 1;

	if ((logfd = open(logfile, O_WRONLY | O_CREAT | O_TRUNC)) < 0)
		logging = 0;

	init = 1;
	return 1;
}

void log_default(int type)
{
	if (!logging) return;

	log_write8(type);
}

void log_portdata(u32 *pd1, u32 *pd2)
{
	int i;

	if (!logging) return;

	log_default(LOG_TRS_PD);

	for (i = 0; i < 4; i++) {
		log_write32(pd1[i]);
		log_write32(pd2[i]);
	}
}

void log_regdata(u32 *rd)
{
	int i;

	if (!logging) return;

	log_default(LOG_TRS_RD);

	for (i = 0; i < 16; i++)
		log_write32(rd[i]);
}

void log_data(int type, u8 *data, u32 size)
{
	int i;

	if (!logging) return;

	log_default(type);

	log_write32(size);

	for (i = 0; i < size; i++)
		log_write8(data[i]);
}

void log_dma(int type, struct _sio2_dma_arg *arg)
{
	u8 *p;
	int i, effective;

	if (!logging) return;

	log_default(type);

	log_write32((u32)arg->addr);
	log_write32(arg->size);
	log_write32(arg->count);

	effective = arg->size * 4 * arg->count;
	if (effective > DMA_MAX)
		effective = DMA_MAX;

	for (i = 0, p = (u8 *)arg->addr; i < effective; i++)
		log_write8(p[i]);
}

void log_stat(u32 stat6c, u32 stat70, u32 stat74)
{
	if (!logging) return;

	log_default(LOG_TRR_STAT);

	log_write32(stat6c); log_write32(stat70); log_write32(stat74);
}

void log_write8(u8 val)
{
	if (!init && !log_init())
		return;

	log[writesize++] = val;
}

void log_write32(u32 val)
{
	if (!init && !log_init())
		return;

	log[writesize] = val & 0xff;
	log[writesize + 1] = (val >> 8) & 0xff;
	log[writesize + 2] = (val >> 16) & 0xff;
	log[writesize + 3] = (val >> 24) & 0xff;
	writesize += 4;
}

void log_flush(int now)
{
	if (!init && !log_init())
		return;

	if (!logging || logfd < 0) return;

	flushcount++;

	if (now || flushcount >= FLUSH_COUNT_MAX) {
		write(logfd, log, writesize);
		close(logfd);
		logfd = open(logfile, O_WRONLY | O_APPEND);
		if (logfd < 0)
			logging = 0;

		writesize = 0;
	}

	if (flushcount >= FLUSH_COUNT_MAX)
		flushcount = 0;
}

/*
# _____     ___ ____     ___ ____
#  ____|   |    ____|   |        | |____|
# |     ___|   |____ ___|    ____| |    \    PS2DEV Open Source Project.
#-----------------------------------------------------------------------
# Copyright ps2dev - http://www.ps2dev.org
# Licenced under Academic Free License version 2.0
# Review ps2sdk README & LICENSE files for further details.
*/

#include "acram_internal.h"

static int ram_dma_xfer(acDmaT dma, int intr, acDmaOp op);
static void ram_dma_done(acDmaT dma);
static void ram_dma_error(acDmaT dma, int intr, acDmaState state, int result);

static acUint32 Ram_limits[] = {0x2000000, 0x4000000, 0x4000000};
static const acDmaOpsData ops_22 = {&ram_dma_xfer, &ram_dma_done, &ram_dma_error};
static struct ram_softc Ramc;

static int ram_dma_xfer(acDmaT dma, int intr, acDmaOp op)
{
	int i;
	int v5;
	int bank;
	int size;
	acRamAddr output;
	acRamAddr addr;
	acRamAddr v10;
	int v11;
	acRamReg ramreg;
	void *buf;
	void *ioaddr;
	int v16;
	struct ram_softc *dmatmp;

	(void)intr;
	dmatmp = (struct ram_softc *)dma;
	i = 0;
	v5 = 0;
	while ( 1 )
	{
		bank = i;
		if ( dmatmp->addr < Ram_limits[v5] )
			break;
		v5 = ++i;
		if ( (unsigned int)i >= 3 )
		{
			bank = -14;
			break;
		}
	}
	size = dmatmp->size;
	output = dmatmp->addr & 1;
	addr = dmatmp->addr - output;
	if ( bank < 0 )
		return -34;
	v10 = Ram_limits[bank];
	if ( addr + size >= v10 )
		size = v10 - addr;
	v11 = bank << 21;
	if ( size <= 0 )
		return -34;
	ramreg = (acRamReg)(v11 + 0xB4000000);
	dmatmp->result = size;
	buf = dmatmp->buf;
	*(acUint16 *)((size & 0x7FC) + v11 + 0xB4000000 + 0x20000) = size >> 11;
	ioaddr = (void *)(v11 + 0xB4000000 + 0x100000);
	if ( bank > 0 )
		addr -= Ram_limits[bank - 1];
	if ( output )
		v16 = 0x70000;
	else
		v16 = 0x60000;
	*(acRamReg)((char *)ramreg + (addr & 0x7FC) + v16) = addr >> 11;
	return op(dma, ioaddr, buf, size);
}

static void ram_dma_done(acDmaT dma)
{
	int thid;
	struct ram_softc *dmatmp;

	dmatmp = (struct ram_softc *)dma;
	thid = dmatmp->thid;
	dmatmp->size = 0;
	if ( thid )
		iWakeupThread(thid);
}

static void ram_dma_error(acDmaT dma, int intr, acDmaState state, int result)
{
	int thid;
	struct ram_softc *dmatmp;

	dmatmp = (struct ram_softc *)dma;
	thid = dmatmp->thid;
	dmatmp->result = result;
	dmatmp->size = 0;
	if ( thid )
	{
		if ( intr )
			iWakeupThread(thid);
		else
			WakeupThread(thid);
	}
	Kprintf("acram:dma_error: state=%d ret=%d\n", state, result);
}

static int ram_dma_request(struct ram_softc *ramc, acRamT ram)
{
	int count;
	acRamAddr r_addr;
	int ret;
	acUint8 *r_buf;
	int output;
	acUint8 *buf;

	count = ram->r_count;
	r_addr = ram->r_addr;
	ret = 0;
	ramc->addr = r_addr;
	r_buf = (acUint8 *)ram->r_buf;
	output = r_addr & 1;
	ramc->size = count;
	ramc->result = 0;
	for ( ramc->buf = r_buf; count > 0; ramc->buf = &buf[ret] )
	{
		acDmaT dma;
		acRamAddr addr;

		dma = acDmaSetup(&ramc->dma, (acDmaOpsData *)&ops_22, 7, 64, output);
		CancelWakeupThread(0);
		ret = acDmaRequest(dma);
		if ( ret < 0 )
			break;
		while ( ramc->size )
			SleepThread();
		ret = ramc->result;
		count -= ret;
		if ( ret < 0 )
			break;
		addr = ramc->addr;
		buf = ramc->buf;
		ramc->size = count;
		ramc->addr = addr + ret;
	}
	return ret;
}

static void ram_thread(void *arg)
{
	acQueueHeadData *qh;
	int thid;
	acRamT ram;
	int ret;
	acQueueT q_next;
	acQueueT q_prev;
	acRamDone r_done;
	acSpl state;
	struct ram_softc *argt;

	argt = (struct ram_softc *)arg;
	qh = &argt->requestq;
	thid = GetThreadId();
	argt->thid = thid;
	while ( 1 )
	{
		CpuSuspendIntr(&state);
		ram = 0;
		if ( qh != qh->q_next )
			ram = (acRamT)qh->q_next;
		CpuResumeIntr(state);
		if ( ram )
		{
			ret = ram_dma_request(argt, ram);
			CpuSuspendIntr(&state);
			q_next = ram->r_chain.q_next;
			q_prev = ram->r_chain.q_prev;
			q_prev->q_next = q_next;
			q_next->q_prev = q_prev;
			CpuResumeIntr(state);
			r_done = ram->r_done;
			if ( r_done )
				r_done(ram, ram->r_arg, ret);
		}
		else
		{
			SleepThread();
		}
		if ( thid != argt->thid )
			ExitThread();
	}
}

acRamT acRamSetup(acRamData *ram, acRamDone done, void *arg, int tmout)
{
	if ( ram )
	{
		ram->r_tmout = tmout;
		ram->r_done = done;
		ram->r_arg = arg;
	}
	return ram;
}

static int ram_request(acRamT ram, acRamAddr addr, void *buf, int count, int (*wakeup)(int thid), int out)
{
	int thid;
	acQueueT out_v8;
	acSpl state;

	if ( !Ramc.thid )
		return -6;
	if ( !ram )
		return -22;
	if ( !buf )
		return -22;
	if ( (addr & 3) != 0 )
	{
		return -14;
	}
	if ( ((uiptr)buf & 3) != 0 )
	{
		return -14;
	}
	if ( count < 3 )
	{
		return 0;
	}
	ram->r_buf = buf;
	ram->r_count = count;
	ram->r_addr = addr | out;
	CpuSuspendIntr(&state);
	thid = Ramc.thid;
	if ( Ramc.thid )
	{
		if ( Ramc.requestq.q_next != &Ramc.requestq )
			wakeup = 0;
		out_v8 = Ramc.requestq.q_prev;
		ram->r_chain.q_next = &Ramc.requestq;
		ram->r_chain.q_prev = out_v8;
		out_v8->q_next = &ram->r_chain;
		Ramc.requestq.q_prev = &ram->r_chain;
	}
	else
	{
		wakeup = 0;
	}
	CpuResumeIntr(state);
	if ( !thid )
		return -6;
	if ( wakeup )
	{
		wakeup(thid);
		return 1;
	}
	return 0;
}

int acRamRead(acRamT ram, acRamAddr addr, void *buf, int count)
{
	return ram_request(ram, addr, buf, count, WakeupThread, 0);
}

int acRamReadI(acRamT ram, acRamAddr addr, void *buf, int count)
{
	return ram_request(ram, addr, buf, count, iWakeupThread, 0);
}

int acRamWrite(acRamT ram, acRamAddr addr, void *buf, int count)
{
	return ram_request(ram, addr, buf, count, WakeupThread, 1);
}

int acRamWriteI(acRamT ram, acRamAddr addr, void *buf, int count)
{
	return ram_request(ram, addr, buf, count, iWakeupThread, 1);
}

static int ram_thread_init(struct ram_softc *ramc, int prio)
{
	int th;
	iop_thread_t param;

	param.attr = 0x2000000;
	param.thread = ram_thread;
	param.priority = prio;
	param.stacksize = 0x800;
	param.option = 0;
	th = CreateThread(&param);
	if ( th > 0 )
		StartThread(th, ramc);
	return th;
}

static unsigned int ram_refresh(void *arg)
{
	const struct ram_softc *argt;

	argt = (struct ram_softc *)arg;
	return argt->refresh;
}

int acRamModuleStart(int argc, char **argv)
{
	int index;
	int prio;
	char **v8;
	char *opt;
	int v10;
	const char *opt_v7;
	int value;
	int retry;
	int ret;
	char *msg;
	iop_sys_clock_t v16;
	char *next;

	if ( acRamModuleStatus() != 0 )
	{
		return -16;
	}
	index = 1;
	prio = 82;
	v8 = argv + 1;
	while ( index < argc )
	{
		opt = *v8;
		if ( **v8 == '-' )
		{
			v10 = opt[1];
			opt_v7 = opt + 2;
			if ( v10 == 'p' )
			{
				value = strtol(opt_v7, &next, 0);
				if ( next != opt_v7 )
					prio = value;
			}
		}
		++index;
		++v8;
	}
	retry = 0xFFFFF;
	*((volatile acUint16 *)0xB2416012) = 0;
	Ramc.requestq.q_prev = &Ramc.requestq;
	Ramc.requestq.q_next = &Ramc.requestq;
	Ramc.addr = 0;
	Ramc.buf = 0;
	Ramc.size = 0;
	Ramc.result = 0;
	Ramc.thid = 0;
	Ramc.refresh = 0;
	while ( retry >= 0 )
	{
		if ( (*((volatile acUint16 *)0xB241C000) & 0x1000) == 0 )
		{
			break;
		}
		--retry;
	}
	*((volatile acUint16 *)0xB4640000) = 0;
	ChangeThreadPriority(0, 123);
	USec2SysClock(0x411Bu, &v16);
	Ramc.refresh = v16.lo;
	ret = SetAlarm(&v16, ram_refresh, &Ramc);
	if ( ret )
	{
		msg = "ram refresh";
	}
	else
	{
		ret = ram_thread_init(&Ramc, prio);
		if ( ret <= 0 )
		{
			CancelAlarm(ram_refresh, &Ramc);
			msg = "thread";
		}
		else
		{
			Ramc.thid = ret;
			return 0;
		}
	}
	printf("acram:module: %s: error %d\n", msg, ret);
	return -6;
}

int acRamModuleStop()
{
	int thid;
	int ret_v3;

	if ( !acRamModuleStatus() )
		return 0;
	thid = Ramc.thid;
	*((volatile acUint16 *)0xB2416010) = 0;
	if ( Ramc.thid > 0 )
	{
		int i;

		Ramc.thid = 0;
		WakeupThread(thid);
		for ( i = 1000;; i = 1000000 )
		{
			int ret;

			DelayThread(i);
			ret = DeleteThread(thid);
			if ( !ret )
				break;
			printf("acram:term_thread: DELETE ret=%d\n", ret);
		}
	}
	ret_v3 = CancelAlarm(ram_refresh, &Ramc);
	if ( ret_v3 != 0 )
		return -6;
	return 0;
}

int acRamModuleRestart(int argc, char **argv)
{
	(void)argc;
	(void)argv;

	return -88;
}

int acRamModuleStatus()
{
	int ret;
	int state;

	CpuSuspendIntr(&state);
	if ( Ramc.thid )
	{
		ret = 2;
		if ( Ramc.requestq.q_next == &Ramc.requestq )
			ret = 1;
	}
	else
	{
		ret = 0;
	}
	CpuResumeIntr(state);
	return ret;
}

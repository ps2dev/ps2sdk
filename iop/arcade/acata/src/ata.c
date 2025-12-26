/*
# _____     ___ ____     ___ ____
#  ____|   |    ____|   |        | |____|
# |     ___|   |____ ___|    ____| |    \    PS2DEV Open Source Project.
#-----------------------------------------------------------------------
# Copyright ps2dev - http://www.ps2dev.org
# Licenced under Academic Free License version 2.0
# Review ps2sdk README & LICENSE files for further details.
*/

#include "acata_internal.h"

static struct ata_softc Atac;

static void ata_timer_done(acTimerT timer, struct ata_softc *arg)
{
	struct ac_ata_h *atah;
	int thid;

	(void)timer;
	atah = arg->atah;
	thid = arg->thid;
	Kprintf("acata:timer_done\n");
	atah->a_state = 511;
	if ( thid )
		iReleaseWaitThread(thid);
}

static void ata_thread(void *arg)
{
	int thid;
	acQueueT q_next;
	struct ac_ata_h *atah;
	struct ac_ata_h *_next_;
	acUint32 tmout;
	acAtaOpsT a_ops;
	int v10;
	acSpl state;
	struct ata_softc *argt;

	argt = (struct ata_softc *)arg;
	thid = GetThreadId();
	argt->thid = thid;
	while ( 1 )
	{
		CpuSuspendIntr(&state);
		q_next = argt->requestq.q_next;
		atah = 0;
		if ( q_next )
		{
			atah = (struct ac_ata_h *)argt->requestq.q_next;
			if ( argt == (struct ata_softc *)q_next )
			{
				atah = 0;
				argt->requestq.q_prev = 0;
				argt->requestq.q_next = 0;
			}
			else
			{
				_next_ = (struct ac_ata_h *)q_next->q_next;
				_next_->a_chain.q_prev = q_next->q_prev;
				argt->requestq.q_next = &_next_->a_chain;
			}
		}
		argt->atah = atah;
		CpuResumeIntr(state);
		if ( atah )
		{
			tmout = atah->a_tmout;
			a_ops = atah->a_ops;
			if ( !tmout )
				tmout = 5000000;
			atah->a_state = 3;
			acTimerAdd(&argt->timer, (acTimerDone)ata_timer_done, argt, tmout);
			v10 = a_ops->ao_command(atah, 32, 78);
			acTimerRemove(&argt->timer);
			if ( v10 < 0 && a_ops->ao_error )
			{
				acTimerAdd(&argt->timer, (acTimerDone)ata_timer_done, argt, 0xF4240u);
				v10 = a_ops->ao_error(atah, v10);
				acTimerRemove(&argt->timer);
			}
			argt->atah = 0;
			a_ops->ao_done(atah, v10);
			CancelWakeupThread(0);
		}
		else
		{
			SleepThread();
		}
		if ( thid != argt->thid )
			ExitThread();
	}
}

static int ata_intr(const struct ata_softc *arg)
{
	if ( arg )
	{
		int thid;

		thid = arg->thid;
		*((volatile acUint16 *)0xB3000000) = 0;
		if ( thid )
			iWakeupThread(thid);
	}
	return 1;
}

int ata_request(struct ac_ata_h *atah, int (*wakeup)(int thid))
{
	int unit;
	int thid;
	acQueueT q_prev;
	acSpl state;

	CpuSuspendIntr(&state);
	unit = 1;
	if ( (atah->a_flag & 0x10) != 0 )
		unit = 2;
	thid = Atac.thid;
	if ( (Atac.active & unit) == 0 )
	{
		thid = 0;
	}
	else
	{
		if ( Atac.thid == 0 )
		{
			wakeup = 0;
		}
		else
		{
			if ( Atac.requestq.q_next )
			{
				wakeup = 0;
			}
			else
			{
				Atac.requestq.q_prev = (acQueueT)&Atac;
				Atac.requestq.q_next = (acQueueT)&Atac;
			}
			q_prev = Atac.requestq.q_prev;
			atah->a_chain.q_next = (acQueueT)&Atac;
			atah->a_chain.q_prev = q_prev;
			q_prev->q_next = &atah->a_chain;
			Atac.requestq.q_prev = &atah->a_chain;
			if ( wakeup )
				atah->a_state = 3;
			else
				atah->a_state = 1;
		}
	}
	CpuResumeIntr(state);
	if ( thid == 0 )
	{
		return -6;
	}
	if ( wakeup == 0 )
	{
		return 0;
	}
	wakeup(thid);
	return 1;
}

static int ata_thread_init(struct ata_softc *atac, int priority)
{
	int th;
	iop_thread_t param;

	param.attr = 0x2000000;
	param.thread = ata_thread;
	param.priority = priority;
	param.stacksize = 0x800;
	param.option = 0;
	th = CreateThread(&param);
	if ( th > 0 )
		StartThread(th, atac);
	return th;
}

int ata_probe(acAtaReg atareg)
{
	int active;
	int unit;
	int count;

	(void)atareg;
	while ( (*((volatile acUint16 *)0xB6070000) & 0x80) != 0 )
		;
	*((volatile acUint16 *)0xB6020000) = 4660;
	// cppcheck-suppress knownConditionTrueFalse
	if ( *((volatile acUint16 *)0xB6020000) != 52 )
		return 0;
	*((volatile acUint16 *)0xB6030000) = 18;
	// cppcheck-suppress knownConditionTrueFalse
	if ( *((volatile acUint16 *)0xB6030000) != 18 )
		return 0;
	active = 0;
	unit = 0;
	*((volatile acUint16 *)0xB6160000) = 2;
	*((volatile acUint16 *)0xB6010000) = 0;
	count = 0;
	while ( unit < 2 )
	{
		*((volatile acUint16 *)0xB6060000) = 16 * (unit != 0);
		*((volatile acUint16 *)0xB6070000) = 0;
		*((volatile acUint16 *)0xB6070000) = 0;
		while ( count <= 1999999 )
		{
			// cppcheck-suppress knownConditionTrueFalse
			if ( (*((volatile acUint16 *)0xB6070000) & 0x80) == 0 )
				break;
			++count;
		}
		if ( count )
			active |= 1 << unit;
		++unit;
		count = 0;
	}
	return active;
}

static int ata_module_optarg(const char *str, int default_value)
{
	int result;
	char *next;

	result = strtol(str, &next, 0);
	if ( next == str )
		return default_value;
	return result;
}

int acAtaModuleStart(int argc, char **argv)
{
	int cmdprio;
	int prio;
	int delay;
	int index;
	char **v11;
	char *opt;
	int v13;
	const char *opt_v10;
	const char *opt_v11;
	int index_v12;
	char *msg;

	if ( acAtaModuleStatus() != 0 )
	{
		return -16;
	}
	cmdprio = Atac.cprio;
	prio = Atac.prio;
	delay = 2000000;
	if ( !Atac.cprio )
		cmdprio = 32;
	index = 1;
	if ( !Atac.prio )
	{
		delay = 1000000;
		prio = 78;
	}
	v11 = argv + 1;
	while ( index < argc )
	{
		opt = *v11;
		if ( **v11 == 45 )
		{
			v13 = opt[1];
			if ( v13 == 100 )
			{
				delay = ata_module_optarg(opt + 2, delay);
			}
			else if ( v13 >= 101 )
			{
				opt_v11 = opt + 2;
				if ( v13 == 112 )
				{
					prio = ata_module_optarg(opt_v11, prio);
				}
			}
			else
			{
				opt_v10 = opt + 2;
				if ( v13 == 99 )
				{
					cmdprio = ata_module_optarg(opt_v10, cmdprio);
				}
			}
		}
		++v11;
		++index;
	}
	if ( prio < cmdprio )
		cmdprio = prio;
	Atac.thid = 0;
	Atac.active = 0;
	Atac.cprio = cmdprio;
	Atac.prio = prio;
	Atac.requestq.q_prev = 0;
	Atac.requestq.q_next = 0;
	DelayThread(delay);
	ChangeThreadPriority(0, 123);
	index_v12 = ata_probe((acAtaReg)0xB6000000);
	Atac.active = index_v12;
	if ( index_v12 == 0 )
	{
		msg = "probe device";
		Atac.active = 0;
	}
	else
	{
		index_v12 = acIntrRegister(AC_INTR_NUM_ATA, (acIntrHandler)ata_intr, &Atac);
		if ( index_v12 < 0 && index_v12 != -11 )
		{
			msg = "register intr";
			Atac.active = 0;
		}
		else
		{
			int v18;

			v18 = ata_thread_init(&Atac, prio);
			index_v12 = v18;
			if ( v18 <= 0 )
			{
				acIntrRelease(AC_INTR_NUM_ATA);
				msg = "init thread";
				Atac.active = 0;
			}
			else
			{
				Atac.thid = v18;
				if ( acIntrEnable(AC_INTR_NUM_ATA) < 0 )
				{
					int thid;

					thid = Atac.thid;
					if ( thid > 0 )
					{
						int i;

						Atac.thid = 0;
						WakeupThread(thid);
						for ( i = 1000;; i = 1000000 )
						{
							int ret;

							DelayThread(i);
							ret = DeleteThread(thid);
							if ( !ret )
								break;
							printf("acata:term_thread: DELETE ret=%d\n", ret);
						}
					}
					acIntrRelease(AC_INTR_NUM_ATA);
					msg = "enable intr";
					Atac.active = 0;
				}
				else
				{
					return 0;
				}
			}
		}
	}
	printf("acata: %s: error %d\n", msg, index_v12);
	return -6;
}

int acAtaModuleStop()
{
	int thid;

	if ( acAtaModuleStatus() == 0 )
	{
		return 0;
	}
	thid = Atac.thid;
	if ( Atac.thid > 0 )
	{
		int i;

		Atac.thid = 0;
		WakeupThread(thid);
		for ( i = 1000;; i = 1000000 )
		{
			int ret;

			DelayThread(i);
			ret = DeleteThread(thid);
			if ( !ret )
				break;
			printf("acata:term_thread: DELETE ret=%d\n", ret);
		}
	}
	acIntrDisable(AC_INTR_NUM_ATA);
	acIntrRelease(AC_INTR_NUM_ATA);
	Atac.active = 0;
	return 0;
}

int acAtaModuleRestart(int argc, char **argv)
{
	(void)argc;
	(void)argv;
	return -88;
}

int acAtaModuleStatus()
{
	int ret;
	int state;

	CpuSuspendIntr(&state);
	ret = 0;
	if ( Atac.thid )
	{
		ret = 2;
		if ( !Atac.requestq.q_next )
			ret = 1;
	}
	CpuResumeIntr(state);
	return ret;
}

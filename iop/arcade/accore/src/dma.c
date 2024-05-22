/*
# _____     ___ ____     ___ ____
#  ____|   |    ____|   |        | |____|
# |     ___|   |____ ___|    ____| |    \    PS2DEV Open Source Project.
#-----------------------------------------------------------------------
# Copyright ps2dev - http://www.ps2dev.org
# Licenced under Academic Free License version 2.0
# Review ps2sdk README & LICENSE files for further details.
*/

#include "accore_internal.h"

static struct dma_softc Dmac;

static int dma_cancel(struct dma_softc *dmac, acDmaT dma, int intr, int result)
{
	int d_state;
	acQueueT q_next;
	acQueueT q_prev;
	acSpl state;

	(void)dmac;
	CpuSuspendIntr(&state);
	d_state = dma->d_state;
	if ( d_state )
	{
		if ( (unsigned int)d_state <= 3u )
		{
			if ( d_state == 3 )
			{
				dmac_disable(8u);
			}
			q_next = dma->d_chain.q_next;
			q_prev = dma->d_chain.q_prev;
			q_prev->q_next = q_next;
			q_next->q_prev = q_prev;
		}
		else
		{
			d_state = -13;
		}
	}
	else
	{
		d_state = -13;
	}
	CpuResumeIntr(state);
	if ( d_state >= 0 )
	{
		acDmaOpsT ops;

		ops = dma->d_ops;
		dma->d_state = 0;
		ops->do_error(dma, intr, (acDmaState)d_state, result);
	}
	return d_state;
}

static int dma_xfer(acDmaT dma, void *ioptr, void *buf, int count)
{
	int slice;
	int ret;
	int slice_v2;
	int ret_v3;
	int state;

	slice = dma->d_slice;
	if ( count >= 4 << slice )
	{
		ret = count >> (slice + 2);
		slice_v2 = 1 << slice;
	}
	else
	{
		ret = count >> 2;
		slice_v2 = 1;
	}
	CpuSuspendIntr(&state);
	if ( dma == (acDmaT)Dmac.requestq.q_next && dma->d_state == 2 )
	{
		if ( ret > 0 )
		{
			int attr;
			unsigned int v12;
			int v13;
			int v14;

			attr = dma->d_attr;
			v12 = GetAcIoDelayReg() & 0x80FFDFFF;
			v13 = 0x62000000;
			if ( ioptr == (void *)0xB6000000 )
				v13 = 0x24000000;
			SetAcIoDelayReg(v12 | v13);
			dmac_ch_set_dpcr(8u, (unsigned int)attr >> 5);
			dmac_enable(8u);
			v14 = dmac_request(8u, buf, slice_v2, ret, attr & 1);
			if ( v14 == 1 )
			{
				*((volatile acUint32 *)0x1F801410) = ((unsigned int)ioptr & 0x3FFFFFF) | 0x14000000;
				Dmac.status = 2;
				dma->d_state = 3;
				dmac_transfer(8u);
				ret_v3 = 1;
			}
			else
			{
				Dmac.status = 1;
				ret_v3 = -5;
			}
		}
		else
		{
			ret_v3 = -22;
		}
	}
	else
	{
		ret_v3 = -13;
	}
	CpuResumeIntr(state);
	return ret_v3;
}

static int dma_request(struct dma_softc *dmac, acDmaT dma, int intr)
{
	acDmaState d_state;
	acDmaOpsT ops;
	int ret;
	acSpl state;

	CpuSuspendIntr(&state);
	if ( dma )
	{
		d_state = dma->d_state;
	}
	else
	{
		acDmaT q_next;

		q_next = (acDmaT)dmac->requestq.q_next;
		d_state = AC_DMA_STATE_READY;
		if ( dmac == (struct dma_softc *)q_next )
		{
			d_state = AC_DMA_STATE_FREE;
		}
		else
		{
			dma = (acDmaT)dmac->requestq.q_next;
			q_next->d_state = 2;
		}
	}
	ret = 0;
	if ( dma )
	{
		int flg;
		flg = 0;
		ops = dma->d_ops;
		if ( d_state == AC_DMA_STATE_QUEUE )
		{
			flg = 1;
		}
		else
		{
			if ( d_state )
			{
				if ( d_state != AC_DMA_STATE_READY )
				{
					ret = -13;
					if ( d_state == AC_DMA_STATE_XFER )
					{
						ret = 0;
						flg = 1;
					}
				}
			}
			else
			{
				acQueueT v10;
				acDmaT state_v5;
				unsigned int v12;

				v10 = dmac->requestq.q_next;
				state_v5 = (acDmaT)dmac->requestq.q_prev;
				dma->d_chain.q_next = &dmac->requestq;
				v12 = (unsigned int)dmac ^ (unsigned int)v10;
				ret = v12 == 0;
				flg = v12 != 0;
				dma->d_chain.q_prev = (acQueueT)state_v5;
				state_v5->d_chain.q_next = &dma->d_chain;
				dmac->requestq.q_prev = &dma->d_chain;
			}
			if ( ret < 0 )
			{
				flg = 0;
			}
		}
		if ( !flg )
		{
			ret = -13;
			if ( dma == (acDmaT)dmac->requestq.q_next )
			{
				if ( !ops->do_xfer )
				{
					ret = -14;
				}
				else
				{
					dma->d_state = 2;
					CpuResumeIntr(state);
					ret = ops->do_xfer(dma, intr, dma_xfer);
					CpuSuspendIntr(&state);
					if ( dma->d_state == 3 )
					{
						ret = 0;
					}
				}
			}
			flg = 1;
		}
		if ( ret < 0 )
		{
			flg = 0;
		}
		if ( flg )
		{
			acUint8 v13;

			v13 = dma->d_state + ret;
			ret = dma->d_state;
			dma->d_state = v13;
		}
	}
	CpuResumeIntr(state);
	if ( ret < 0 )
	{
		dma_cancel(dmac, dma, intr, ret);
	}
	return ret;
}

int acDmaRequest(acDmaT dma)
{
	int ret;

	ret = dma_request(&Dmac, dma, 0);
	if ( ret < 0 )
	{
		while ( dma_request(&Dmac, 0, 0) < 0 )
			;
	}
	return ret;
}

int acDmaRequestI(acDmaT dma)
{
	int ret;

	ret = dma_request(&Dmac, dma, 1);
	if ( ret < 0 )
	{
		while ( dma_request(&Dmac, 0, 1) < 0 )
			;
	}
	return ret;
}

int acDmaCancel(acDmaT dma, int result)
{
	int ret;

	ret = dma_cancel(&Dmac, dma, 0, result);
	if ( ret >= 0 )
	{
		while ( dma_request(&Dmac, 0, 0) < 0 )
			;
	}
	return ret;
}

int acDmaCancelI(acDmaT dma, int result)
{
	int ret;

	ret = dma_cancel(&Dmac, dma, 1, result);
	if ( ret >= 0 )
	{
		while ( dma_request(&Dmac, 0, 1) < 0 )
			;
	}
	return ret;
}

static int dma_intr(void *arg)
{
	acDmaData *q_next;
	acQueueData *v3;
	acQueueT q_prev;
	struct dma_softc *argt;

	argt = (struct dma_softc *)arg;
	if ( argt )
	{
		dmac_disable(8u);
		q_next = (acDmaData *)argt->requestq.q_next;
		if ( argt != (struct dma_softc *)q_next )
		{
			acDmaOpsT ops;
			acDmaState state;

			v3 = q_next->d_chain.q_next;
			q_prev = q_next->d_chain.q_prev;
			ops = q_next->d_ops;
			state = q_next->d_state;
			q_prev->q_next = v3;
			v3->q_prev = q_prev;
			q_next->d_state = 0;
			if ( state == AC_DMA_STATE_XFER )
				ops->do_done(q_next);
			else
				ops->do_error(q_next, 1, state, -13);
		}
		while ( dma_request(&Dmac, 0, 1) < 0 )
			;
	}
	return 1;
}

acDmaT acDmaSetup(acDmaData *dma, acDmaOpsData *ops, int priority, int slice, int output)
{
	if ( dma )
	{
		acUint32 x;
		unsigned int v6;

		dma->d_ops = ops;
		dma->d_attr = (32 * priority) | (output != 0);
		x = slice / 4;
		v6 = 0;
		if ( x > 1 )
		{
			v6 = 1;
			while ( ((unsigned int)1 << v6) < x )
			{
				v6 = v6 + 1;
			}
			v6 = v6 - 1;
		}
		dma->d_slice = v6;
		dma->d_state = 0;
	}
	return dma;
}

int acDmaModuleStart(int argc, char **argv)
{
	int ret;
	char *msg;

	(void)argc;
	(void)argv;
	if ( Dmac.status )
	{
		return -16;
	}
	Dmac.requestq.q_prev = (acQueueT)&Dmac;
	Dmac.requestq.q_next = (acQueueT)&Dmac;
	ret = RegisterIntrHandler(41, 1, dma_intr, &Dmac);
	if ( !ret || ret == -104 )
	{
		ret = EnableIntr(41);
		if ( !ret )
		{
			dmac_disable(8u);
			Dmac.status = 1;
			return 0;
		}
		ReleaseIntrHandler(41);
		msg = "dma_intr_enable";
	}
	else
	{
		msg = "dma_intr_register";
	}
	printf("accore:dma_init:%s: error %d\n", msg, ret);
	return -6;
}

int acDmaModuleStop()
{
	int dummy;

	if ( Dmac.status )
	{
		DisableIntr(41, &dummy);
		ReleaseIntrHandler(41);
		Dmac.status = 0;
	}
	return 0;
}

int acDmaModuleRestart(int argc, char **argv)
{
	(void)argc;
	(void)argv;
	return -88;
}

int acDmaModuleStatus()
{
	return Dmac.status;
}

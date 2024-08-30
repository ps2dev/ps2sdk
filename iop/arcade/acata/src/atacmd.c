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

static int ata_dma_xfer(acDmaT dma, int intr, acDmaOp op);
static void ata_dma_done(acDmaT dma);
static void ata_dma_error(acDmaT dma, int intr, acDmaState state, int result);
static int ata_ops_command(struct ac_ata_h *atah, int cmdpri, int pri);
static void ata_ops_done(struct ac_ata_h *atah, int result);

static acDmaOpsData ops_8 = {&ata_dma_xfer, &ata_dma_done, &ata_dma_error};
static struct ac_ata_ops ops_37 = {&ata_ops_command, &ata_ops_done, NULL};

static int ata_dma_xfer(acDmaT dma, int intr, acDmaOp op)
{
	struct ata_dma *dmatmp;
	int thid;

	dmatmp = (struct ata_dma *)dma;
	if ( dmatmp->ad_state == 31 )
	{
		dmatmp->ad_result = dmatmp->ad_ata->ac_h.a_size;
		return op(dma, (void *)0xB6000000, dmatmp->ad_ata->ac_h.a_buf, dmatmp->ad_ata->ac_h.a_size);
	}
	thid = dmatmp->ad_thid;
	dmatmp->ad_state = 3;
	if ( intr )
		iWakeupThread(thid);
	else
		WakeupThread(thid);
	return 0;
}

static void ata_dma_done(acDmaT dma)
{
	struct ata_dma *dmatmp;
	int thid;

	dmatmp = (struct ata_dma *)dma;

	thid = dmatmp->ad_thid;
	dmatmp->ad_state = 127;
	if ( thid )
		iWakeupThread(thid);
}

static void ata_dma_error(acDmaT dma, int intr, acDmaState state, int result)
{
	struct ata_dma *dmatmp;
	int thid;

	dmatmp = (struct ata_dma *)dma;
	thid = dmatmp->ad_thid;
	dmatmp->ad_state = 1023;
	dmatmp->ad_result = result;
	if ( thid )
	{
		if ( intr )
			iWakeupThread(thid);
		else
			WakeupThread(thid);
	}
	Kprintf("acata:A:dma_error: state=%d ret=%d\n", state, result);
}

static int ata_ops_command(struct ac_ata_h *atah, int cmdpri, int pri)
{
	int flag;
	int ret;
	const acAtaCommandData *cmd;
	int count;
	int flag_v8;
	int v16;
	int ret_v22;
	int sr_v34;
	int ret_v35;
	acAtaCommandData *cmd_v36;
	int rest_v37;
	struct ata_dma dma_data;
	acAtaT ata;

	ata = (acAtaT)atah;
	flag = atah->a_flag;
	ret = 0;
	if ( atah->a_state >= 0x1FFu )
	{
		return -116;
	}
	atah->a_state = 7;
	if ( (flag & 1) != 0 )
	{
		acDmaT dma;
		int ret_v3;

		dma = acDmaSetup(&dma_data.ad_dma, &ops_8, 4, 8, flag & 4);
		dma_data.ad_result = 0;
		// cppcheck-suppress unreadVariable
		dma_data.ad_thid = GetThreadId();
		// cppcheck-suppress unreadVariable
		dma_data.ad_ata = ata;
		dma_data.ad_state = 0;
		ret_v3 = acDmaRequest(dma);
		if ( ret_v3 < 0 )
		{
			printf("acata:A:dma_wait: error %d\n", ret_v3);
			return ret_v3;
		}
		while ( 1 )
		{
			int ret_v5;

			ret_v5 = dma_data.ad_state;
			if ( ret_v5 )
			{
				if ( (int)ret_v5 <= 0 )
				{
					printf("acata:A:dma_wait: TIMEDOUT %d\n", ret_v5);
					acDmaCancel(&dma_data.ad_dma, -116);
					ret = ret_v5;
					if ( ret_v5 >= 0 )
						ret = -116;
					if ( ret < 0 )
					{
						return ret;
					}
				}
				break;
			}
			if ( SleepThread() )
				break;
		}
	}
	ChangeThreadPriority(0, cmdpri);
	cmd = ata->ac_command;
	count = 5;
	flag_v8 = atah->a_flag;
	*((volatile acUint16 *)0xB6060000) = flag_v8 & 0x10;
	*((volatile acUint16 *)0xB6160000) = (flag_v8 & 2) ^ 2;
	*((volatile acUint16 *)0xB6010000) = 0;
	while ( count >= 0 )
	{
		int data;

		data = *cmd++;
		*(acUint16 *)(((2 * ((data >> 8) & 8) + ((data >> 8) & 7)) << 16) + 0xB6000000) = data & 0xFF;
		--count;
		if ( ((data >> 8) & 0xF) == 7 )
		{
			break;
		}
	}
	ChangeThreadPriority(0, pri);
	// cppcheck-suppress knownConditionTrueFalse
	if ( atah->a_state >= 0x1FFu )
	{
		return -116;
	}
	atah->a_state = 31;
	if ( atah->a_buf )
	{
		if ( (flag_v8 & 1) != 0 )
		{
			dma_data.ad_state = 31;
			acDmaRequest(&dma_data.ad_dma);
			printf("acata:dma start\n");
		}
		else
		{
			acUint16 *a_buf;

			a_buf = (acUint16 *)atah->a_buf;
			if ( (flag_v8 & 4) != 0 )
			{
				int a_size;

				a_size = atah->a_size;
				while ( (*((volatile acUint16 *)0xB6070000) & 0x80) != 0 )
					;
				while ( a_size > 0 )
				{
					int xlen;
					int xlen_v15;
					int xlen_v16;
					int sr;
					int ret_v20;

					xlen = 512;
					if ( a_size < 513 )
						xlen = a_size;
					a_size -= xlen;
					xlen_v15 = (unsigned int)xlen >> 1;
					xlen_v16 = xlen_v15 - 1;
					while ( (*((volatile acUint16 *)0xB6070000) & 8) != 0 )
					{
						int ret_v17;

						ret_v17 = 0;
						if ( xlen_v16 >= 0 )
						{
							ret_v17 = *a_buf;
							a_buf++;
						}
						*((volatile acUint16 *)0xB6000000) = ret_v17;
						--xlen_v16;
					}
					if ( (flag_v8 & 2) != 0 )
					{
						sr = *((volatile acUint16 *)0xB6160000);
						ret_v20 = sr & 0xFF;
						while ( (*((volatile acUint16 *)0xB6160000) & 0x80) != 0 )
						{
							ret_v20 = -116;
							if ( SleepThread() != 0 )
								break;
							ret_v20 = *((volatile acUint16 *)0xB6160000);
						}
					}
					else
					{
						ret_v20 = *((volatile acUint16 *)0xB6070000);
						while ( (*((volatile acUint16 *)0xB6070000) & 0x80) != 0 )
						{
							sr = *((volatile acUint16 *)0xB6070000);
							ret_v20 = sr & 0xFF;
						}
					}
					if ( ret_v20 < 0 )
						break;
					if ( (ret_v20 & 8) == 0 )
						break;
				}
			}
			else
			{
				acUint16 *buf_v23;
				int rest;

				buf_v23 = (acUint16 *)atah->a_buf;
				rest = atah->a_size;
				while ( rest > 0 )
				{
					int sr_v25;
					int xlen_v27;
					int xlen_v28;
					int ret_v29;
					int xlen_v30;

					if ( (flag_v8 & 2) != 0 )
					{
						sr_v25 = *((volatile acUint16 *)0xB6160000);
						sr_v25 = sr_v25 & 0xFF;
						while ( (*((volatile acUint16 *)0xB6160000) & 0x80) != 0 )
						{
							sr_v25 = -116;
							if ( SleepThread() )
								break;
							sr_v25 = *((volatile acUint16 *)0xB6160000);
						}
					}
					else
					{
						sr_v25 = *((volatile acUint16 *)0xB6070000);
						while ( (*((volatile acUint16 *)0xB6070000) & 0x80) != 0 )
						{
							sr_v25 = *((volatile acUint16 *)0xB6070000);
							sr_v25 = sr_v25 & 0xFF;
						}
					}
					if ( sr_v25 < 0 )
						break;
					xlen_v27 = 512;
					if ( rest < 513 )
						xlen_v27 = rest;
					rest -= xlen_v27;
					xlen_v28 = (unsigned int)xlen_v27 >> 1;
					if ( (sr_v25 & 1) != 0 )
						xlen_v28 = 0;
					(void)(*((volatile acUint16 *)0xB6070000) & 0x80);
					xlen_v30 = xlen_v28 - 1;
					while ( (*((volatile acUint16 *)0xB6070000) & 8) != 0 )
					{
						if ( xlen_v30 >= 0 )
						{
							*buf_v23 = *((volatile acUint16 *)0xB6000000);
							buf_v23++;
						}
						--xlen_v30;
					}
					ret_v29 = *((volatile acUint16 *)0xB6070000) & 0x80;
					if ( !ret_v29 )
						break;
				}
			}
		}
	}
	v16 = flag_v8 & 2;
	ret_v22 = flag_v8 & 1;
	if ( ret_v22 )
	{
		signed int state;

		state = 63;
		while ( 1 )
		{
			int v38;

			v38 = *((volatile acUint16 *)0xB6160000) & 1;
			if ( v38 || state >= 511 )
			{
				printf(
					"acata:A:dma_iowait: TIMEDOUT %04x:%02x:%02x\n",
					state,
					*((volatile acUint16 *)0xB6160000),
					*((volatile acUint16 *)0xB6010000));
				if ( state < 1023 )
					acDmaCancel(&dma_data.ad_dma, -116);
				ret = 0;
				if ( !v38 )
					ret = -116;
				break;
			}
			state = dma_data.ad_state;
			if ( (*((volatile acUint16 *)0xB6160000) & 0x80) == 0 && (int)dma_data.ad_state >= 64 )
			{
				ret = dma_data.ad_result;
				break;
			}
			if ( SleepThread() )
			{
				state = 511;
				if ( dma_data.ad_state == 31 )
					state = 1023;
			}
		}
	}
	else
	{
		ret = 0;
		if ( v16 )
		{
			while ( (*((volatile acUint16 *)0xB6160000) & 0x81) == 128 )
			{
				if ( SleepThread() )
				{
					ret = -116;
					break;
				}
			}
		}
		else
		{
			int tmout;

			tmout = 99999;
			while ( (*((volatile acUint16 *)0xB6070000) & 0x81) == 128 )
			{
				if ( tmout < 0 )
				{
					ret = -116;
					break;
				}
				--tmout;
			}
		}
		if ( ret < 0 )
		{
			printf("acata:C:io_done: TIMEDOUT\n");
		}
	}
	if ( ret < 0 )
	{
		return ret;
	}
	sr_v34 = *((volatile acUint16 *)0xB6070000);
	if ( (*((volatile acUint16 *)0xB6070000) & 1) != 0 )
		return -((sr_v34 << 8) + *((volatile acUint16 *)0xB6010000));
	if ( atah->a_state < 0x1FFu )
	{
		atah->a_state = 127;
		ret_v35 = 0;
	}
	else
	{
		ret_v35 = -116;
	}
	cmd_v36 = ata->ac_command;
	if ( ret_v35 < 0 )
	{
		return -((sr_v34 << 8) + *((volatile acUint16 *)0xB6010000));
	}
	for ( rest_v37 = 6; rest_v37 > 0; --rest_v37 )
	{
		if ( (((int)*cmd_v36 >> 12) & 0xF) == 0 )
			break;
		*cmd_v36 =
			((((int)*cmd_v36 >> 12) & 0xF) << 12)
			| ((*(acUint16 *)(((2 * (((int)*cmd_v36 >> 12) & 8) + (((int)*cmd_v36 >> 12) & 7)) << 16) + 0xB6000000)) & 0xFF);
		++cmd_v36;
	}
	return 6 - rest_v37;
}

static void ata_ops_done(struct ac_ata_h *atah, int result)
{
	acAtaDone ac_done;
	acAtaT ata;

	ata = (acAtaT)atah;
	ac_done = ata->ac_done;
	if ( ac_done )
		ac_done(ata, atah->a_arg, result);
}

acAtaT acAtaSetup(acAtaData *ata, acAtaDone done, void *arg, unsigned int tmout)
{
	if ( ata )
	{
		ata->ac_h.a_ops = &ops_37;
		ata->ac_h.a_arg = arg;
		ata->ac_h.a_tmout = tmout;
		ata->ac_h.a_state = 0;
		ata->ac_h.a_flag = 0;
		ata->ac_done = done;
	}
	return ata;
}

acAtaCommandData *acAtaReply(acAtaT ata)
{
	if ( ata == 0 )
		return 0;
	return ata->ac_command;
}

int acAtaRequest(acAtaT ata, int flag, acAtaCommandData *cmd, int item, void *buf, int size)
{
	int v7;
	const acAtaCommandData *v8;
	char *v9;

	flag = (flag | 8) ^ 8;
	if ( !ata || !cmd )
	{
		return -22;
	}
	if ( (unsigned int)item >= 7 )
	{
		return -34;
	}
	v7 = item - 1;
	ata->ac_h.a_flag = flag;
	v8 = &cmd[v7];
	v9 = (char *)ata + 2 * v7;
	while ( v7 >= 0 )
	{
		--v7;
		*((acUint16 *)v9 + 18) = *v8--;
		v9 -= 2;
	}
	ata->ac_h.a_buf = buf;
	ata->ac_h.a_size = size;
	return ata_request(&ata->ac_h, WakeupThread);
}

int acAtaRequestI(acAtaT ata, int flag, acAtaCommandData *cmd, int item, void *buf, int size)
{
	int v7;
	const acAtaCommandData *v8;
	char *v9;

	flag = (flag | 8) ^ 8;
	if ( !ata || !cmd )
	{
		return -22;
	}
	if ( (unsigned int)item >= 7 )
	{
		return -34;
	}
	v7 = item - 1;
	ata->ac_h.a_flag = flag;
	v8 = &cmd[v7];
	v9 = (char *)ata + 2 * v7;
	while ( v7 >= 0 )
	{
		--v7;
		*((acUint16 *)v9 + 18) = *v8--;
		v9 -= 2;
	}
	ata->ac_h.a_buf = buf;
	ata->ac_h.a_size = size;
	return ata_request(&ata->ac_h, iWakeupThread);
}

int acAtaStatus(acAtaT ata)
{
	int state;

	if ( ata == 0 )
	{
		return -22;
	}
	state = ata->ac_h.a_state;
	if ( (unsigned int)(state - 1) >= 0x7E )
	{
		return 0;
	}
	if ( state != 1 )
		return 2;
	return 1;
}

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

static int atapi_dma_xfer(acDmaT dma, int intr, acDmaOp op);
static void atapi_dma_done(acDmaT dma);
static void atapi_dma_error(acDmaT dma, int intr, acDmaState state, int result);
static int atapi_ops_command(struct ac_ata_h *atah, int cmdpri, int pri);
static void atapi_ops_done(struct ac_ata_h *atah, int result);
static int atapi_ops_error(struct ac_ata_h *atah, int ret);

static acDmaOpsData ops_8_0 = {&atapi_dma_xfer, &atapi_dma_done, &atapi_dma_error};
static struct ac_ata_ops ops_48 = {&atapi_ops_command, &atapi_ops_done, &atapi_ops_error};

static int atapi_dma_xfer(acDmaT dma, int intr, acDmaOp op)
{
	struct atapi_dma *dmatmp;
	int thid;

	dmatmp = (struct atapi_dma *)dma;
	if ( dmatmp->ad_state == 31 )
	{
		dmatmp->ad_result = dmatmp->ad_atapi->ap_h.a_size;
		return op(dma, (void *)0xB6000000, dmatmp->ad_atapi->ap_h.a_buf, dmatmp->ad_atapi->ap_h.a_size);
	}
	thid = dmatmp->ad_thid;
	dmatmp->ad_state = 3;
	if ( intr )
		iWakeupThread(thid);
	else
		WakeupThread(thid);
	return 0;
}

static void atapi_dma_done(acDmaT dma)
{
	struct atapi_dma *dmatmp;
	int thid;

	dmatmp = (struct atapi_dma *)dma;
	thid = dmatmp->ad_thid;
	dmatmp->ad_state = 127;
	if ( thid )
		iWakeupThread(thid);
}

static void atapi_dma_error(acDmaT dma, int intr, acDmaState state, int result)
{
	struct atapi_dma *dmatmp;
	int thid;

	dmatmp = (struct atapi_dma *)dma;
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
	Kprintf("acata:P:dma_error: state=%d ret=%d\n", state, result);
}

static int atapi_packet_send(acAtaReg atareg, acAtapiPacketData *pkt, int flag)
{
	int count;
	int tmout;
	int v6;

	(void)atareg;
	count = 6;
	*((volatile acUint16 *)0xB6050000) = 0;
	*((volatile acUint16 *)0xB6040000) = 64;
	*((volatile acUint16 *)0xB6060000) = flag & 0x10;
	*((volatile acUint16 *)0xB6160000) = (flag & 2) ^ 2;
	*((volatile acUint16 *)0xB6010000) = flag & 1;
	*((volatile acUint16 *)0xB6070000) = 160;
	tmout = 999;
	v6 = 1000;
	// cppcheck-suppress knownConditionTrueFalse
	while ( (*((volatile acUint16 *)0xB6070000) & 0x80) != 0 )
	{
		if ( tmout < 0 )
		{
			printf("acata:P:wait: TIMEDOUT\n");
			v6 = -116;
			break;
		}
		--tmout;
		v6 = tmout + 1;
	}
	if ( v6 < 0 )
	{
		printf("acata:P:packet_send: TIMEDOUT\n");
		return -116;
	}
	// cppcheck-suppress knownConditionTrueFalse
	while ( (*((volatile acUint16 *)0xB6070000) & 8) != 0 )
	{
		--count;
		if ( count < 0 )
			break;
		*((volatile acUint16 *)0xB6000000) = pkt->u_h[0];
		pkt = (acAtapiPacketData *)((char *)pkt + 2);
	}
	return 0;
}

static int atapi_pio_read(acAtaReg atareg, acUint16 *buf, int count, int flag)
{
	char v6;
	int rest;
	int sr;
	int xlen;
	int sr_v5;
	int drop_v10;

	(void)atareg;
	v6 = flag;
	rest = count;
	sr = flag & 2;
	while ( 1 )
	{
		int xlen_v6;
		int drop;
		int xlen_v8;
		int sr_v9;

		if ( !sr )
		{
			sr_v5 = *((volatile acUint16 *)0xB6070000);
			while ( (sr_v5 & 0x80) != 0 )
			{
				xlen = *((volatile acUint16 *)0xB6070000);
				sr_v5 = xlen & 0xFF;
			}
		}
		else
		{
			xlen = *((volatile acUint16 *)0xB6160000);
			sr_v5 = xlen & 0xFF;
			while ( (sr_v5 & 0x80) != 0 )
			{
				sr_v5 = -116;
				if ( SleepThread() != 0 )
					break;
				sr_v5 = *((volatile acUint16 *)0xB6160000);
			}
		}
		if ( sr_v5 < 0 )
			return sr_v5;
		if ( (sr_v5 & 8) == 0 )
			break;
		xlen_v6 = (*((volatile acUint16 *)0xB6050000) << 8) + *((volatile acUint16 *)0xB6040000);
		drop = xlen_v6 - rest;
		if ( rest >= xlen_v6 )
			drop = 0;
		else
			xlen_v6 = rest;
		rest -= xlen_v6;
		xlen_v8 = (xlen_v6 + 1) / 2 - 1;
		while ( xlen_v8 >= 0 )
		{
			--xlen_v8;
			*buf++ = *((volatile acUint16 *)0xB6000000);
		}
		sr_v9 = drop + 1;
		for ( drop_v10 = sr_v9 / 2 - 1; drop_v10 >= 0; --drop_v10 )
			;
		sr = v6 & 2;
		if ( (*((volatile acUint16 *)0xB6070000) & 0x80) == 0 )
		{
			break;
		}
	}
	return count - rest;
}

static int atapi_ops_command(struct ac_ata_h *atah, int cmdpri, int pri)
{
	int flag;
	int ret_v5;
	int v28;
	struct atapi_dma dma_data;
	acAtapiT atapi;

	atapi = (acAtapiT)atah;
	flag = atah->a_flag;
	if ( (flag & 1) == 0 )
	{
		ret_v5 = 0;
	}
	else
	{
		acDmaT dma;
		int v8;
		int ret;

		dma = acDmaSetup(&dma_data.ad_dma, &ops_8_0, 4, 64, flag & 4);
		dma_data.ad_result = 0;
		// cppcheck-suppress unreadVariable
		dma_data.ad_thid = GetThreadId();
		// cppcheck-suppress unreadVariable
		dma_data.ad_atapi = atapi;
		dma_data.ad_state = 0;
		v8 = acDmaRequest(dma);
		ret = v8;
		if ( v8 < 0 )
		{
			printf("acata:P:dma_wait: error %d\n", v8);
			ret_v5 = ret;
		}
		else
		{
			while ( 1 )
			{
				int flg;

				flg = 0;
				ret_v5 = dma_data.ad_state;
				if ( ret_v5 )
				{
					if ( (int)ret_v5 > 0 )
					{
						ret_v5 = 0;
						break;
					}
					flg = 1;
				}
				if ( flg == 0 && SleepThread() )
					flg = 1;
				if ( flg != 0 )
				{
					printf("acata:P:dma_wait: TIMEDOUT %d\n", ret_v5);
					acDmaCancel(&dma_data.ad_dma, -116);
					if ( ret_v5 >= 0 )
					{
						ret_v5 = -116;
					}
					break;
				}
			}
		}
	}
	if ( ret_v5 < 0 )
		return ret_v5;
	ChangeThreadPriority(0, cmdpri);
	ret_v5 = atapi_packet_send((acAtaReg)0xB6000000, &atapi->ap_packet, flag);
	if ( ret_v5 >= 0 )
	{
		int v12;
		if ( atah->a_state < 0x1FFu )
		{
			atah->a_state = 31;
			v12 = 0;
		}
		else
		{
			v12 = -116;
		}
		ret_v5 = -116;
		if ( v12 >= 0 )
		{
			acUint16 *a_buf;

			a_buf = (acUint16 *)atah->a_buf;
			if ( !a_buf )
			{
				ret_v5 = 0;
			}
			else if ( (flag & 1) != 0 )
			{
				dma_data.ad_state = 31;
				ret_v5 = acDmaRequest(&dma_data.ad_dma);
			}
			else
			{
				int size;
				acUint16 *v15;

				size = atah->a_size;
				v15 = (acUint16 *)atah->a_buf;
				if ( (flag & 4) == 0 )
				{
					ret_v5 = atapi_pio_read((acAtaReg)0xB6000000, a_buf, atah->a_size, flag);
				}
				else
				{
					signed int a_size;
					int sr;

					a_size = atah->a_size;
					sr = flag & 2;
					while ( 1 )
					{
						int xlen;
						int sr_v14;
						int xlen_v15;
						int drop;
						int xlen_v17;
						int sr_v18;
						int drop_v20;

						if ( sr )
						{
							xlen = *((volatile acUint16 *)0xB6160000);
							sr_v14 = xlen & 0xFF;
							while ( (sr_v14 & 0x80) != 0 )
							{
								sr_v14 = -116;
								if ( SleepThread() != 0 )
									break;
								sr_v14 = *((volatile acUint16 *)0xB6160000);
							}
						}
						else
						{
							sr_v14 = *((volatile acUint16 *)0xB6070000);
							while ( (sr_v14 & 0x80) != 0 )
							{
								xlen = *((volatile acUint16 *)0xB6070000);
								sr_v14 = xlen & 0xFF;
							}
						}
						ret_v5 = sr_v14;
						if ( sr_v14 < 0 )
							break;
						if ( (sr_v14 & 8) == 0 )
						{
							ret_v5 = size - a_size;
							break;
						}
						xlen_v15 = (*((volatile acUint16 *)0xB6050000) << 8) + *((volatile acUint16 *)0xB6040000);
						drop = xlen_v15 - a_size;
						if ( a_size >= xlen_v15 )
							drop = 0;
						else
							xlen_v15 = a_size;
						a_size -= xlen_v15;
						xlen_v17 = (xlen_v15 + 1) / 2 - 1;
						for ( sr_v18 = drop + 1; xlen_v17 >= 0; sr_v18 = drop + 1 )
						{
							*((volatile acUint16 *)0xB6000000) = *v15;
							v15++;
							--xlen_v17;
						}
						for ( drop_v20 = sr_v18 / 2 - 1; drop_v20 >= 0; --drop_v20 )
							*((volatile acUint16 *)0xB6000000) = 0;
						sr = flag & 2;
						if ( (*((volatile acUint16 *)0xB6070000) & 0x80) == 0 )
						{
							ret_v5 = size - a_size;
							break;
						}
					}
				}
			}
		}
	}
	ChangeThreadPriority(0, pri);
	if ( ret_v5 < 0 )
		return ret_v5;
	if ( atah->a_state < 0x1FFu )
	{
		atah->a_state = 63;
		v28 = 0;
	}
	else
	{
		v28 = -116;
	}
	if ( v28 < 0 )
		return -116;
	if ( (flag & 1) == 0 )
	{
		int v32;

		v32 = 0;
		if ( (flag & 2) != 0 )
		{
			while ( (*((volatile acUint16 *)0xB6160000) & 0x81) == 128 )
			{
				if ( SleepThread() )
				{
					v32 = -116;
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
					v32 = -116;
					break;
				}
				--tmout;
			}
		}
		if ( v32 < 0 )
		{
			printf("acata:C:io_done: TIMEDOUT\n");
			ret_v5 = -116;
		}
	}
	else
	{
		signed int ret_v23;
		acInt32 ad_result;

		ret_v23 = 63;
		while ( 1 )
		{
			int v30;

			v30 = *((volatile acUint16 *)0xB6160000) & 1;
			if ( v30 || ret_v23 >= 511 )
			{
				printf(
					"acata:P:dma_iowait: TIMEDOUT %04x:%02x:%02x\n",
					ret_v23,
					*((volatile acUint16 *)0xB6160000),
					*((volatile acUint16 *)0xB6010000));
				if ( ret_v23 < 1023 )
					acDmaCancel(&dma_data.ad_dma, -116);
				ad_result = 0;
				if ( !v30 )
					ad_result = -116;
				break;
			}
			ret_v23 = dma_data.ad_state;
			if ( (*((volatile acUint16 *)0xB6160000) & 0x80) == 0 && (int)dma_data.ad_state >= 64 )
			{
				ad_result = dma_data.ad_result;
				break;
			}
			if ( SleepThread() )
			{
				ret_v23 = 511;
				if ( dma_data.ad_state == 31 )
					ret_v23 = 1023;
			}
		}
		ret_v5 = ad_result;
	}
	if ( ret_v5 < 0 )
		return ret_v5;
	if ( (*((volatile acUint16 *)0xB6070000) & 1) != 0 )
		return -((*((volatile acUint16 *)0xB6070000) << 8) + *((volatile acUint16 *)0xB6010000));
	if ( atah->a_state >= 0x1FFu )
	{
		return -116;
	}
	atah->a_state = 127;
	return ret_v5;
}

static void atapi_ops_done(struct ac_ata_h *atah, int result)
{
	acAtapiDone ap_done;
	acAtapiT atapi;

	atapi = (acAtapiT)atah;
	ap_done = atapi->ap_done;
	if ( ap_done )
		ap_done(atapi, atah->a_arg, result);
}

static int atapi_ops_error(struct ac_ata_h *atah, int ret)
{
	int flag;
	int v3;
	struct atapi_sense sense;
	union
	{
		struct
		{
			acUint8 opcode;
			acUint8 lun;
			// cppcheck-suppress unusedStructMember
			acUint8 padding[2];
			acUint8 len;
			// cppcheck-suppress unusedStructMember
			acUint8 padding2[7];
		};
		acAtapiPacketData pkt;
	} u;
	acAtapiT atapi;

	atapi = (acAtapiT)atah;
	if ( (*((volatile acUint16 *)0xB6070000) & 1) == 0 )
		return ret;
	memset(&sense, 0, sizeof(sense));
	memset(&u, 0, sizeof(u));
	flag = atah->a_flag;
	u.opcode = 0x03;
	u.len = 0x12;
	u.lun = atapi->ap_packet.u_b[1];
	*((volatile acUint16 *)0xB6160000) = (flag & 2) ^ 2;
	*((volatile acUint16 *)0xB6010000) = 0;
	v3 = atapi_packet_send((acAtaReg)0xB6000000, &u.pkt, flag);
	if ( v3 < 0 )
	{
		ret = v3;
	}
	else
	{
		int v4;
		int v5;

		v4 = atapi_pio_read((acAtaReg)0xB6000000, (acUint16 *)&sense, sizeof(sense), flag);
		v5 = v4;
		if ( v4 > 0 )
		{
			int v6;

			v6 = 0;
			if ( (flag & 2) != 0 )
			{
				while ( (*((volatile acUint16 *)0xB6160000) & 0x81) == 128 )
				{
					if ( SleepThread() )
					{
						v6 = -116;
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
						v6 = -116;
						break;
					}
					--tmout;
				}
			}
			if ( v6 < 0 )
			{
				printf("acata:C:io_done: TIMEDOUT\n");
				v6 = -116;
			}
			v3 = v5;
			if ( v6 < 0 )
				v3 = -116;
			ret = v3;
		}
		else
		{
			ret = v4;
			if ( !v4 )
			{
				v3 = -5;
				ret = v3;
			}
		}
	}
	if ( ret > 0 )
		return -((sense.s_key << 16) | (sense.s_asc << 8) | sense.s_ascq);
	if ( !ret )
		return -5;
	return ret;
}

acAtapiT acAtapiSetup(acAtapiData *atapi, acAtapiDone done, void *arg, unsigned int tmout)
{
	if ( atapi )
	{
		atapi->ap_h.a_ops = &ops_48;
		atapi->ap_h.a_arg = arg;
		atapi->ap_h.a_tmout = tmout;
		atapi->ap_h.a_state = 0;
		atapi->ap_h.a_flag = 0;
		atapi->ap_done = done;
	}
	return atapi;
}

int acAtapiRequest(acAtapiT atapi, int flag, acAtapiPacketData *pkt, void *buf, int size)
{
	flag = flag | 8;
	if ( !atapi || !pkt )
	{
		return -22;
	}
	atapi->ap_h.a_flag = flag;
	atapi->ap_packet.u_w[0] = pkt->u_w[0];
	atapi->ap_packet.u_w[1] = pkt->u_w[1];
	atapi->ap_packet.u_w[2] = pkt->u_w[2];
	atapi->ap_h.a_buf = buf;
	atapi->ap_h.a_size = size;
	return ata_request(&atapi->ap_h, WakeupThread);
}

int acAtapiRequestI(acAtapiT atapi, int flag, acAtapiPacketData *pkt, void *buf, int size)
{
	flag = flag | 8;
	if ( !atapi || !pkt )
	{
		return -22;
	}
	atapi->ap_h.a_flag = flag;
	atapi->ap_packet.u_w[0] = pkt->u_w[0];
	atapi->ap_packet.u_w[1] = pkt->u_w[1];
	atapi->ap_packet.u_w[2] = pkt->u_w[2];
	atapi->ap_h.a_buf = buf;
	atapi->ap_h.a_size = size;
	return ata_request(&atapi->ap_h, iWakeupThread);
}

int acAtapiStatus(acAtapiT atapi)
{
	int state;

	if ( atapi == 0 )
	{
		return -22;
	}
	state = atapi->ap_h.a_state;
	if ( (unsigned int)(state - 1) >= 0x7E )
	{
		return 0;
	}
	if ( state != 1 )
		return 2;
	return 1;
}

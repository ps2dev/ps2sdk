/*
# _____     ___ ____     ___ ____
#  ____|   |    ____|   |        | |____|
# |     ___|   |____ ___|    ____| |    \    PS2DEV Open Source Project.
#-----------------------------------------------------------------------
# Copyright ps2dev - http://www.ps2dev.org
# Licenced under Academic Free License version 2.0
# Review ps2sdk README & LICENSE files for further details.
*/

#include "accdvd_internal.h"

static struct acd_softc Acdc;

static void acd_done(struct acd *acd, struct acd_softc *arg, int ret)
{
	acInt32 tmout;
	int thid;
	int tmout_v2;
	acd_done_t done;
	acSpl state;

	CpuSuspendIntr(&state);
	tmout = arg->active;
	arg->sense = ret;
	tmout--;
	arg->active = tmout;
	if ( tmout < 0 )
		arg->active = 0;
	CpuResumeIntr(state);
	thid = acd->c_thid;
	tmout_v2 = acd->c_tmout;
	done = acd->c_done;
	acd->c_thid = ret;
	if ( !tmout_v2 )
		acd->c_tmout = 1;
	if ( done )
		done(acd, acd->c_arg, ret);
	if ( thid )
		WakeupThread(thid);
}

static void acd_atapi_done(acAtapiT atapi, void *arg, int ret)
{
	struct acd_softc *argt;

	argt = (struct acd_softc *)arg;
	acd_done((struct acd *)atapi, argt, ret);
}

static int
acd_request_in(struct acd *acd, int flag, acAtapiPacketT packet, void *buffer, int size, acAtapiDone done, char *name)
{
	int status;
	int ret;
	int ret_v10;
	int tmout;
	int v16;
	int ret_v13;
	int ret_v15;
	int state;
	int state_2;

	(void)name;
	if ( (Acdc.status & 1) == 0 )
		return -6;
	ret = acd->c_tmout;
	if ( ret <= 0 )
	{
		acd->c_thid = 0;
		ret = -ret;
	}
	else
	{
		acd->c_thid = GetThreadId();
	}
	acAtapiSetup(&acd->c_atapi, done ? done : acd_atapi_done, &Acdc, ret);
	CpuSuspendIntr(&state);
	ret_v10 = Acdc.drive;
	++Acdc.active;
	CpuResumeIntr(state);
	status = ret_v10;
	if ( status < 0 )
	{
		return status;
	}
	tmout = acd->c_tmout;
	v16 = flag | ret_v10;
	if ( tmout > 0 )
		acd->c_tmout = 0;
	status = acAtapiRequest(&acd->c_atapi, v16, packet, buffer, size);
	ret_v13 = status;
	if ( status < 0 )
	{
		CpuSuspendIntr(&state_2);
		Acdc.active--;
		if ( Acdc.active < 0 )
			Acdc.active = 0;
		CpuResumeIntr(state_2);
		return ret_v13;
	}
	if ( tmout <= 0 )
	{
		return status;
	}
	while ( 1 )
	{
		int status_v14;

		status_v14 = SleepThread();
		if ( status_v14 == -418 )
		{
			ret_v15 = -116;
			break;
		}
		if ( status_v14 )
		{
			ret_v15 = -5;
			break;
		}
		if ( acd->c_tmout )
		{
			ret_v15 = acd->c_thid;
			acd->c_tmout = tmout;
			return ret_v15;
		}
	}
	acd->c_tmout = tmout;
	return ret_v15;
}

static void acd_read_done(acAtapiT atapi, struct acd_softc *arg, int ret)
{
	if ( ret >= 0 )
		ret >>= 11;
	acd_done((struct acd *)atapi, arg, ret);
}

int acd_read(struct acd *acd, acd_lsn_t lsn, void *buf, int sectors)
{
	int flag;
	union
	{
		struct
		{
			acUint8 opcode;
			acUint8 lun;
			acUint8 lba[4];
			// cppcheck-suppress unusedStructMember
			acUint8 padding;
			acUint8 len[2];
			// cppcheck-suppress unusedStructMember
			acUint8 padding2[3];
		};
		acAtapiPacketData pkt;
	} u;

	if ( !acd || !buf )
		return -22;
	if ( sectors == 0 )
	{
		return 0;
	}
	memset(&u, 0, sizeof(u));
	flag = 2;
	if ( Acdc.dma )
		flag = 3;
	u.opcode = 40;
	u.lun = 0;
	u.lba[0] = (lsn & 0xFF000000) >> 24;
	u.lba[1] = (lsn & 0xFF0000) >> 16;
	u.lba[2] = (lsn & 0xFF00) >> 8;
	u.lba[3] = lsn;
	u.len[0] = (sectors & 0xFF00) >> 8;
	u.len[1] = sectors & 0xFF;
	return acd_request_in(acd, flag, &u.pkt, buf, sectors << 11, (acAtapiDone)acd_read_done, "read");
}

int acd_readtoc(struct acd *acd, void *buf, int size)
{
	acAtapiPacketData v4;

	if ( !acd || !buf )
		return -22;
	if ( !size )
		return 0;
	memset(&v4, 0, sizeof(v4));
	v4.u_h[2] = 0;
	v4.u_w[0] = 579;
	v4.u_b[7] = (size & 0xFF00) >> 8;
	v4.u_b[6] = 0;
	v4.u_w[2] = size & 0xFF;
	return acd_request_in(acd, 2, &v4, buf, size, 0, "readtoc");
}

int acd_seek(struct acd *acd, acd_lsn_t lsn)
{
	acAtapiPacketData v3;

	if ( !acd )
		return -22;
	memset(&v3, 0, sizeof(v3));
	v3.u_h[0] = 40;
	v3.u_b[2] = (lsn & 0xFF000000) >> 24;
	v3.u_b[3] = (lsn & 0xFF0000) >> 16;
	v3.u_w[1] = (lsn & 0xFF00) >> 8;
	v3.u_w[2] = 0;
	v3.u_b[5] = lsn;
	return acd_request_in(acd, 2, &v3, 0, 0, 0, "seek");
}

static void acd_opentray_done(acAtapiT atapi, struct acd_softc *arg, int ret)
{
	acSpl state;

	if ( ret >= 0 )
	{
		CpuSuspendIntr(&state);
		arg->status = (arg->status | 6) ^ 4;
		CpuResumeIntr(state);
	}
	acd_done((struct acd *)atapi, arg, ret);
}

static void acd_closetray_done(acAtapiT atapi, struct acd_softc *arg, int ret)
{
	acSpl state;

	if ( ret >= 0 )
	{
		CpuSuspendIntr(&state);
		arg->status = (arg->status | 6) ^ 2;
		CpuResumeIntr(state);
	}
	acd_done((struct acd *)atapi, arg, ret);
}

int acd_ioctl(struct acd *acd, int cmd)
{
	acAtapiDone done;
	char *name;
	union
	{
		struct
		{
			acUint8 opcode;
			acUint8 imm;
			// cppcheck-suppress unusedStructMember
			acUint8 padding3[2];
			acUint8 op;
			// cppcheck-suppress unusedStructMember
			acUint8 padding4[7];
		};
		acAtapiPacketData pkt;
	} u;

	if ( !acd )
		return -22;
	memset(&u, 0, sizeof(u));
	done = 0;
	if ( cmd == 17 )
	{
		u.opcode = 40;
		name = "ioctl:seek+start";
	}
	else if ( (cmd & 5) != 0 )
	{
		u.opcode = 27;
		u.imm = (cmd & 0x80) != 0;
		u.op = cmd & 3;
		if ( (cmd & 2) != 0 )
		{
			done = (acAtapiDone)acd_opentray_done;
			if ( (cmd & 1) != 0 )
				done = (acAtapiDone)acd_closetray_done;
		}
		name = "ioctl:startstop";
	}
	else
	{
		return -22;
	}
	return acd_request_in(acd, 2, &u.pkt, 0, 0, done, name);
}

static int acd_mode_sense(struct acd *acd, int pgcode, void *buffer, int size, acAtapiDone done, char *name)
{
	acAtapiPacketData v7;

	memset(&v7, 0, sizeof(v7));
	v7.u_b[0] = 0x5A;
	v7.u_b[2] = pgcode;
	v7.u_b[7] = (size & 0xFF00) >> 8;
	v7.u_w[2] = size & 0xFF;
	return acd_request_in(acd, 2, &v7, buffer, size, done, name);
}

static int acd_mode_select(struct acd *acd, void *buffer, int size, acAtapiDone done, char *name)
{
	acAtapiPacketData v6;

	memset(&v6, 0, sizeof(v6));
	v6.u_b[0] = 0x55;
	v6.u_b[1] = 0x10;
	v6.u_b[7] = (size & 0xFF00) >> 8;
	v6.u_w[2] = size & 0xFF;
	return acd_request_in(acd, 6, &v6, buffer, size, done, name);
}

static void acd_getmedium_done(acAtapiT atapi, struct acd_softc *arg, int ret)
{
	int h_mtype;
	acSpl state;

	h_mtype = ret;
	if ( ret >= 0 )
	{
		acUint32 status;
		int v7;

		CpuSuspendIntr(&state);
		h_mtype = arg->retry.me_h.h_mtype;
		status = arg->status | 6;
		arg->medium = h_mtype;
		if ( h_mtype == 113 )
			v7 = status ^ 4;
		else
			v7 = status ^ 2;
		arg->status = v7 | 0xA00;
		CpuResumeIntr(state);
	}
	acd_done((struct acd *)atapi, arg, h_mtype);
}

int acd_getmedium(struct acd *acd)
{
	int ret;
	acSpl state;

	if ( acd )
		return acd_mode_sense(acd, 1, &Acdc.retry, 20, (acAtapiDone)acd_getmedium_done, "getmedium");
	CpuSuspendIntr(&state);
	ret = -61;
	if ( (Acdc.status & 0x800) != 0 )
		ret = Acdc.medium;
	CpuResumeIntr(state);
	return ret;
}

static void acd_retry_done(acAtapiT atapi, struct acd_softc *arg, int ret)
{
	int me_rretry;
	acSpl state;

	me_rretry = ret;
	if ( ret >= 0 )
	{
		acCdvdsifId h_mtype;
		acUint32 status;
		int v8;

		CpuSuspendIntr(&state);
		h_mtype = arg->retry.me_h.h_mtype;
		status = arg->status | 6;
		arg->medium = h_mtype;
		if ( h_mtype == 113 )
			v8 = status ^ 4;
		else
			v8 = status ^ 2;
		arg->status = v8 | 0xA00;
		me_rretry = arg->retry.me_rretry;
		CpuResumeIntr(state);
	}
	acd_done((struct acd *)atapi, arg, me_rretry);
}

int acd_getretry(struct acd *acd)
{
	if ( !acd )
		return -22;
	return acd_mode_sense(acd, 1, &Acdc.retry, 20, (acAtapiDone)acd_retry_done, "getretry");
}

int acd_setretry(struct acd *acd, int rretry)
{
	int size;
	acSpl state;

	if ( acd == 0 )
	{
		return -22;
	}
	if ( (Acdc.status & 0x200) == 0 )
	{
		return -61;
	}
	CpuSuspendIntr(&state);
	size = 0;
	if ( rretry != Acdc.retry.me_rretry )
	{
		Acdc.retry.me_rretry = rretry;
		size = (Acdc.retry.me_h.h_len[0] << 8) + Acdc.retry.me_h.h_len[1] + 2;
	}
	CpuResumeIntr(state);
	if ( !size )
		return rretry;
	return acd_mode_select(acd, &Acdc.retry, size, (acAtapiDone)acd_retry_done, "setretry");
}

static void acd_getspeed_done(acAtapiT atapi, struct acd_softc *arg, int ret)
{
	int v4;
	acSpl state;

	v4 = ret;
	if ( ret >= 0 )
	{
		acCdvdsifId speed;
		acUint32 status;
		int v8;

		CpuSuspendIntr(&state);
		speed = arg->speed.mc_h.h_mtype;
		v4 = (arg->speed.mc_speed[0] << 8) + arg->speed.mc_speed[1];
		status = arg->status | 6;
		arg->medium = speed;
		if ( speed == 113 )
			v8 = status ^ 4;
		else
			v8 = status ^ 2;
		arg->status = v8 | 0xC00;
		CpuResumeIntr(state);
	}
	acd_done((struct acd *)atapi, arg, v4);
}

static void acd_getmaxspeed_done(acAtapiT atapi, struct acd_softc *arg, int ret)
{
	int v4;
	acSpl state;

	v4 = ret;
	if ( ret >= 0 )
	{
		acCdvdsifId speed;
		acUint32 status;
		int v8;

		CpuSuspendIntr(&state);
		speed = arg->speed.mc_h.h_mtype;
		v4 = (arg->speed.mc_maxspeed[0] << 8) + arg->speed.mc_maxspeed[1];
		status = arg->status | 6;
		arg->medium = speed;
		if ( speed == 113 )
			v8 = status ^ 4;
		else
			v8 = status ^ 2;
		arg->status = v8 | 0xC00;
		CpuResumeIntr(state);
	}
	acd_done((struct acd *)atapi, arg, v4);
}

int acd_getspeed(struct acd *acd, int maxspeed)
{
	acAtapiDone done;

	if ( acd == 0 )
	{
		return -22;
	}
	done = (acAtapiDone)acd_getspeed_done;
	if ( maxspeed )
		done = (acAtapiDone)acd_getmaxspeed_done;
	return acd_mode_sense(acd, 42, &Acdc.speed, 28, done, "getspeed");
}

int acd_setspeed(struct acd *acd, int speed)
{
	int v3;
	int ospeed;
	union
	{
		struct
		{
			acUint8 opcode;
			acUint8 lun;
			acUint8 speed[2];
			// cppcheck-suppress unusedStructMember
			acUint8 padding[8];
		};
		acAtapiPacketData pkt;
	} u;
	acSpl state;

	v3 = speed;
	if ( speed < 0 )
		return -22;
	CpuSuspendIntr(&state);
	if ( (Acdc.status & 0x400) != 0 )
		ospeed = (Acdc.speed.mc_speed[0] << 8) + Acdc.speed.mc_speed[1];
	else
		ospeed = -1;
	CpuResumeIntr(state);
	if ( v3 > 0xFFFF )
		v3 = 0xFFFF;
	if ( v3 == ospeed )
		return 0;
	memset(&u, 0, sizeof(u));
	u.opcode = 0xbb;
	u.lun = 0;
	u.speed[0] = (v3 & 0xFF00) >> 8;
	u.speed[1] = v3;
	return acd_request_in(acd, 2, &u.pkt, 0, 0, 0, "setspeed");
}

static void acd_timer_done(acAtapiT atapi, struct acd_softc *arg, int ret)
{
	int v4;
	acSpl state;

	v4 = ret;
	if ( ret >= 0 )
	{
		acCdvdsifId h_mtype;
		acUint32 status;
		int status_v3;

		CpuSuspendIntr(&state);
		h_mtype = arg->timer.md_h.h_mtype;
		status = arg->status | 6;
		arg->medium = h_mtype;
		if ( h_mtype == 113 )
			status_v3 = status ^ 4;
		else
			status_v3 = status ^ 2;
		arg->status = status_v3 | 0x900;
		v4 = arg->timer.md_timer & 0xF;
		CpuResumeIntr(state);
	}
	acd_done((struct acd *)atapi, arg, v4);
}

int acd_gettimer(struct acd *acd)
{
	if ( !acd )
		return -22;
	return acd_mode_sense(acd, 13, &Acdc.timer, 16, (acAtapiDone)acd_timer_done, "gettimer");
}

int acd_settimer(struct acd *acd, int time)
{
	char v3;
	int acdc;
	int state;

	v3 = time;
	if ( acd == 0 )
	{
		return -22;
	}
	if ( (Acdc.status & 0x100) == 0 )
	{
		return -61;
	}
	CpuSuspendIntr(&state);
	Acdc.timer.md_timer = v3 & 0xF;
	acdc = ((Acdc.timer.md_h.h_len[0] << 8) + Acdc.timer.md_h.h_len[1] + 2);
	CpuResumeIntr(state);
	return acd_mode_select(acd, &Acdc.timer, acdc, (acAtapiDone)acd_timer_done, "settimer");
}

int acd_ready(struct acd *acd)
{
	acAtapiPacketData v2;

	if ( !acd )
		return -22;
	memset(&v2, 0, sizeof(v2));
	return acd_request_in(acd, 2, &v2, 0, 0, 0, "ready");
}

int acd_readcapacity()
{
	int ret;
	struct acd acd_data;
	struct atapi_read_capacity capacity;
	union
	{
		struct
		{
			acUint8 opcode;
			acUint8 lun;
			// cppcheck-suppress unusedStructMember
			acUint8 padding1[10];
		};
		acAtapiPacketData pkt;
	} u;

	acd_setup(&acd_data, 0, 0, 5000000);
	memset(&u, 0, sizeof(u));
	u.opcode = 0x25;
	u.lun = 0;
	ret = acd_request_in(&acd_data, 2, &u.pkt, &capacity, 8, 0, "readcapacity");
	if ( ret >= 0 )
	{
		unsigned int val;

		val = *(unsigned int *)capacity.lba;
		return (val << 24) + ((val & 0xFF00) << 8) + ((val >> 8) & 0xFF00) + ((val >> 24) & 0xFF);
	}
	return ret;
}

int acd_delay()
{
	iop_sys_clock_t t;
	u32 s;
	u32 us;

	GetSystemTime(&t);
	SysClock2USec(&t, &s, &us);
	if ( s >= 0xD )
		return 0;
	return 1000000 * (13 - s) - us;
}

int acd_getsense()
{
	return Acdc.sense;
}

acCdvdsifId acd_gettray()
{
	if ( (Acdc.status & 8) != 0 )
		return 3;
	if ( (Acdc.status & 2) != 0 )
		return 1;
	return (Acdc.status >> 1) & 2;
}

struct acd *acd_setup(struct acd *acd, acd_done_t done, void *arg, int tmout)
{
	if ( acd )
	{
		acd->c_done = done;
		acd->c_arg = arg;
		acd->c_thid = 0;
		acd->c_tmout = tmout;
		if ( !tmout )
			acd->c_tmout = 5000000;
	}
	return acd;
}

int acd_sync(struct acd *acd, int nblocking, int *resultp)
{
	int ret;
	acSpl state;

	CpuSuspendIntr(&state);
	ret = -11;
	if ( !acd->c_thid )
	{
		int v7;

		v7 = acAtapiStatus(&acd->c_atapi);
		ret = v7;
		if ( v7 > 0 )
		{
			ret = 0;
			acd->c_thid = GetThreadId();
		}
		else if ( v7 == 0 )
		{
			ret = 1;
		}
	}
	CpuResumeIntr(state);
	if ( !(ret | nblocking) )
	{
		ret = 1;
		SleepThread();
	}
	if ( ret > 0 && resultp )
	{
		*resultp = acd->c_thid;
	}
	return ret;
}

static void acd_ata_done(acAtaT ata, void *arg, int ret)
{
	int thid;
	struct acd_ata *acdata;

	(void)arg;
	acdata = (struct acd_ata *)ata;
	thid = acdata->a_thid;
	acdata->a_result = ret;
	if ( thid )
		WakeupThread(thid);
}

int acd_ata_request(int flag, acAtaCommandT cmd, int items, acAtaDone done, char *name)
{
	int ret;
	struct acd_ata acdata;

	(void)name;
	acAtaSetup(&acdata.a_ata, done ? done : acd_ata_done, 0, 0x4C4B40u);
	// cppcheck-suppress unreadVariable
	acdata.a_thid = GetThreadId();
	acdata.a_result = 0;
	ret = acAtaRequest(&acdata.a_ata, flag, cmd, items, 0, 0);
	if ( ret < 0 )
	{
		return ret;
	}
	SleepThread();
	return acdata.a_result;
}

static void acd_getstatus_done(acAtaT ata, void *arg, int ret)
{
	int thid;
	struct acd_ata *acdata;
	(void)arg;
	acdata = (struct acd_ata *)ata;
	if ( ret >= 0 )
		ret = *(acUint8 *)acAtaReply(&acdata->a_ata);
	thid = acdata->a_thid;
	acdata->a_result = ret;
	if ( thid )
		WakeupThread(thid);
}

int acd_getstatus()
{
	acAtaCommandData v1[4];

	v1[1] = 2021;
	v1[0] = (Acdc.drive & 0xFF) | 0x2600;
	return acd_ata_request(Acdc.drive | 2, v1, 2, acd_getstatus_done, "getstatus");
}

static int acd_setfeatures(int feature, int value)
{
	acAtaCommandData v3[4];

	v3[2] = (value & 0xFF) | 0x200;
	v3[1] = (feature & 0xFF) | 0x100;
	v3[3] = 2031;
	v3[0] = (Acdc.drive & 0xFF) | 0x600;
	return acd_ata_request(Acdc.drive | 2, v3, 4, 0, "setfeatures");
}

int acd_setdma(acCdvdsifId dma)
{
	acCdvdsifId v1;
	int mode;
	int ret;
	int ret_v3;

	v1 = dma;
	mode = dma;
	if ( !dma )
	{
		Acdc.dma = AC_CDVDSIF_ID_NOP;
		return 0;
	}
	if ( dma == 32 )
	{
		mode = 18;
		ret_v3 = 0x40000;
		while ( mode >= 0 )
		{
			if ( (Acdc.dmamap & ret_v3) != 0 )
				break;
			mode--;
			ret_v3 = 1 << mode;
		}
		v1 = mode;
		if ( mode < 0 )
			return -134;
	}
	if ( (Acdc.dmamap & (1 << v1)) == 0 )
	{
		return -134;
	}
	if ( (unsigned int)v1 < AC_CDVDSIF_ID_PAUSES )
	{
		if ( (unsigned int)v1 < AC_CDVDSIF_ID_STOP )
		{
			if ( (unsigned int)v1 >= AC_CDVDSIF_ID_PAUSE )
				mode = v1 + 8;
		}
		else
		{
			mode = v1 + 16;
		}
	}
	else
	{
		mode = v1 + 40;
	}
	ret = acd_setfeatures(3, mode);
	if ( ret < 0 )
	{
		return ret;
	}
	ret = acd_setfeatures(102, 0);
	if ( ret < 0 )
	{
		return ret;
	}
	Acdc.dma = v1;
	return v1;
}

acCdvdsifId acd_getdma()
{
	return Acdc.dma;
}

static int acd_identify(int drive)
{
	int flag;
	int ret;
	acUint16 ident[256];
	struct acd_ata acdata;
	acAtaCommandData cmd[2];

	flag = 16 * (drive != 0);
	cmd[0] = flag | 0x600;
	cmd[1] = 1953;
	acAtaSetup(&acdata.a_ata, acd_ata_done, 0, 0x4C4B40u);
	// cppcheck-suppress unreadVariable
	acdata.a_thid = GetThreadId();
	acdata.a_result = 0;
	ret = acAtaRequest(&acdata.a_ata, flag | 2, cmd, 2, ident, 512);
	if ( ret >= 0 )
	{
		SleepThread();
		ret = acdata.a_result;
	}
	if ( ret < 0 )
	{
		return ret;
	}
	if ( (ident[0] & 0xFF00) != 0x8500 )
		return -6;
	return ((ident[62] & 0xFF) << 8) | ((ident[63] & 0xFF) << 16) | ((ident[88] & 0xFF) << 24);
}

static int acd_softreset(int drive)
{
	(void)drive;

	return 0;
}

int acd_module_status()
{
	int ret;
	int state;

	CpuSuspendIntr(&state);
	ret = 0;
	if ( (Acdc.status & 1) != 0 )
	{
		ret = 1;
		if ( Acdc.active > 0 )
			ret = 2;
	}
	CpuResumeIntr(state);
	return ret;
}

int acd_module_start(int argc, char **argv)
{
	int ret;
	int drive;
	int ret_v2;

	(void)argc;
	(void)argv;
	ret = acd_module_status();
	drive = 0;
	if ( ret )
	{
		return ret;
	}
	while ( 1 )
	{
		acd_softreset(drive);
		ret_v2 = acd_identify(drive);
		if ( ret_v2 >= 0 )
			break;
		++drive;
		ret = -6;
		if ( drive >= 2 )
			return ret;
	}
	if ( drive )
		Acdc.drive = 16;
	else
		Acdc.drive = 0;
	Acdc.status = 1;
	Acdc.medium = -1;
	Acdc.dmamap = ret_v2;
	acd_setdma((acCdvdsifId)32);
	return 0;
}

int acd_module_stop()
{
	int ret;

	ret = acd_module_status();
	if ( ret )
	{
		return ret;
	}
	Acdc.status = 0;
	return 0;
}

int acd_module_restart(int argc, char **argv)
{
	(void)argc;
	(void)argv;

	return -88;
}

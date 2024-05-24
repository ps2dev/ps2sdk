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

static struct cdc_module mods_120[3] = {
	{&acAtaModuleStart, &acAtaModuleStop, &acAtaModuleStatus},
	{&acd_module_start, &acd_module_stop, &acd_module_status},
	{&cdfs_module_start, &cdfs_module_stop, &cdfs_module_status}};
static struct cdc_softc Cdcc;

static int cdc_errno_set(struct cdc_softc *cdcc, int ret)
{
	if ( ret )
	{
		if ( ret >= 256 )
		{
			if ( ret >> 16 == 6 )
			{
				cdfs_umount();
				ret = 49;
			}
			else
			{
				int asc;

				asc = ret & 0xFFFF;
				ret = 48;
				if ( asc == 8448 )
					ret = 50;
			}
		}
		else
		{
			switch ( ret )
			{
				case 14:
					ret = 32;
					break;
				case 16:
					ret = 19;
					break;
				case 22:
					ret = 33;
					break;
				case 34:
					ret = 50;
					break;
				default:
					ret = 48;
					break;
			}
		}
	}
	cdcc->error = ret;
	return ret;
}

static int cdc_eve_alloc()
{
	int ret;
	iop_event_t param;

	param.attr = 2;
	param.bits = 0;
	param.option = 0;
	ret = CreateEventFlag(&param);
	if ( ret <= 0 )
	{
		printf("accdvd:cdc:eve_alloc: error %d\n", ret);
	}
	return ret;
}

static int cdc_sem_alloc()
{
	int ret;
	iop_sema_t param;

	param.attr = 0;
	param.initial = 1;
	param.max = 1;
	param.option = 0;
	ret = CreateSema(&param);
	if ( ret <= 0 )
	{
		printf("accdvd:cdc:sem_alloc: error %d\n", ret);
	}
	return ret;
}

static int cdc_unlock(struct cdc_softc *cdcc, int ret, acCdvdsifId fno)
{
	int lockid;
	unsigned int v8;

	lockid = cdcc->lockid;
	if ( fno )
	{
		int v6;
		int eveid;

		v6 = 0;
		if ( ret < 0 )
			v6 = -ret;
		cdc_errno_set(cdcc, v6);
		eveid = cdcc->syncid;
		cdcc->stat = 0;
		cdcc->fno = AC_CDVDSIF_ID_NOP;
		if ( eveid > 0 )
			SetEventFlag(eveid, 3u);
	}
	if ( lockid > 0 )
	{
		SignalSema(lockid);
	}
	v8 = ~ret;
	return v8 >> 31;
}

int cdc_getfno()
{
	return Cdcc.fno;
}

int cdc_geterrno()
{
	return Cdcc.error;
}

int cdc_seterrno(int ret)
{
	Cdcc.error = ret;
	return 0;
}

static void cdc_done(struct acd *acd, struct cdc_softc *arg, int ret)
{
	int v3;
	cdc_done_t done;
	int eveid;

	(void)acd;
	v3 = 0;
	if ( ret < 0 )
		v3 = -ret;
	cdc_errno_set(arg, v3);
	done = arg->done;
	arg->done = 0;
	if ( done )
	{
		int type;

		switch ( arg->fno )
		{
			case AC_CDVDSIF_ID_PAUSE:
				type = 7;
				break;
			case AC_CDVDSIF_ID_READ:
				type = 1;
				break;
			case AC_CDVDSIF_ID_SEEK:
				type = 4;
				break;
			case AC_CDVDSIF_ID_STANDBY:
				type = 5;
				break;
			case AC_CDVDSIF_ID_STOP:
				type = 6;
				break;
			default:
				type = 0;
				break;
		}
		done(type);
	}
	eveid = arg->syncid;
	arg->stat = 0;
	arg->fno = AC_CDVDSIF_ID_NOP;
	if ( eveid > 0 )
		SetEventFlag(eveid, 3u);
}

int cdc_sync(int nonblocking)
{
	u32 bits;

	if ( !Cdcc.fno )
	{
		return 0;
	}
	if ( nonblocking > 0 )
	{
		return 1;
	}
	bits = 1;
	if ( nonblocking < 0 )
		bits = 3;
	if ( Cdcc.syncid > 0 )
		WaitEventFlag(Cdcc.syncid, bits, 1, &bits);
	return ((bits & 0xFF) ^ 1) & 1;
}

int cdc_ready(int nonblocking)
{
	struct acd *acd;
	int ret;
	int v4;
	int asc;
	struct acd acd_data;
	struct acd acd_data_v8;

	acd = acd_setup(&acd_data, 0, 0, 5000000);
	Cdcc.error = 0;
	ret = -1;
	while ( ret < 0 )
	{
		int delay;

		ret = acd_ready(acd);
		delay = -1;
		if ( ret >= 0 )
		{
			v4 = acd_readcapacity();
			if ( v4 < 0 )
				v4 = 0;
			Cdcc.cdsize = v4;
			ret = 2;
		}
		else if ( ret < -255 )
		{
			asc = (acUint16) - (ret & 0xFFFF);
			if ( -ret >> 16 != 6 )
			{
				if ( asc != 0x401 )
				{
					if ( asc == 0x3A00 )
					{
						Cdcc.cdsize = 0;
						ret = 2;
					}
				}
				else
				{
					delay = 1000000;
				}
			}
			else
			{
				delay = 0;
				if ( asc == 10496 )
					delay = acd_delay();
			}
		}
		else
		{
			if ( ret == -116 )
			{
				delay = 1000000;
			}
			else
			{
				ret = 6;
			}
		}
		if ( nonblocking )
			break;
		if ( delay > 0 )
			DelayThread(delay);
	}
	if ( ret == 2 )
	{
		acd_getmedium(acd_setup(&acd_data_v8, 0, 0, 5000000));
		Cdcc.tray = acd_gettray();
	}
	return ret;
}

int cdc_medium()
{
	struct acd *v0;
	int v1;
	int ret;
	int v3;
	struct acd acd_data;

	v0 = acd_setup(&acd_data, 0, 0, 5000000);
	v1 = acd_getmedium(v0);
	ret = v1;
	v3 = 0;
	if ( v1 < 0 )
		v3 = -v1;
	cdc_errno_set(&Cdcc, v3);
	switch ( ret )
	{
		case 1:
		case 5:
		case 33:
		case 37:
			return 18;
		case 2:
		case 6:
		case 34:
		case 38:
			return 253;
		case 3:
		case 7:
		case 35:
		case 39:
			return 19;
		case 65:
		case 69:
			return 20;
		case 112:
		case 113:
			return 0;
		default:
			return 255;
	}
}

int cdc_error()
{
	return Cdcc.error;
}

int cdc_stat()
{
	int ret;
	int v2;
	struct acd acd_data;

	ret = acd_getmedium(0);
	acd_setup(&acd_data, 0, 0, 5000000);
	if ( ret == -61 )
		ret = acd_getmedium(&acd_data);
	if ( ret == 113 )
	{
		return 1;
	}
	if ( Cdcc.stat )
	{
		return Cdcc.stat;
	}
	v2 = acd_getstatus();
	if ( v2 < 0 )
		return 32;
	return (v2 != 0) ? 2 : 0;
}

int cdc_readtoc(unsigned char *toc, cdc_xfer_t xfer)
{
	acInt32 ret;
	int v7;
	int ret_v3;

	if ( Cdcc.lockid )
	{
		ret = 0;
		if ( WaitSema(Cdcc.lockid) )
			ret = 19;
	}
	else
	{
		ret = 17;
	}
	if ( ret )
	{
		Cdcc.error = ret;
		return 0;
	}
	v7 = -16;
	if ( Cdcc.fno == AC_CDVDSIF_ID_NOP )
	{
		v7 = 6;
		Cdcc.fno = AC_CDVDSIF_ID_READTOC;
		Cdcc.stat = 6;
		if ( Cdcc.syncid > 0 )
			ClearEventFlag(Cdcc.syncid, 0);
	}
	ret_v3 = v7;
	if ( v7 > 0 )
	{
		ret_v3 = acd_readtoc(acd_setup(&Cdcc.acd, 0, 0, 5000000), Cdcc.buf, 1024);
		if ( ret_v3 > 0 )
		{
			if ( ret_v3 < 1024 )
				memset(&Cdcc.buf[ret_v3], 0, 1024 - ret_v3);
			ret_v3 = xfer(toc, Cdcc.buf, 1024, CDC_XFER_OUT);
		}
	}
	return cdc_unlock(&Cdcc, ret_v3, AC_CDVDSIF_ID_READTOC);
}

int cdc_lookup(sceCdlFILE *fp, const char *name, int namlen, cdc_xfer_t xfer)
{
	acInt32 ret;
	int ret_v2;
	int ret_v3;
	struct cdfs_dirent dirent;

	if ( Cdcc.lockid )
	{
		ret = 0;
		if ( WaitSema(Cdcc.lockid) )
			ret = 19;
	}
	else
	{
		ret = 17;
	}
	if ( ret )
	{
		Cdcc.error = ret;
		return 0;
	}
	ret_v2 = -16;
	if ( Cdcc.fno )
	{
		ret_v3 = ret_v2;
	}
	else
	{
		ret_v2 = 12;
		ret_v3 = 12;
		Cdcc.fno = AC_CDVDSIF_ID_LOOKUP;
		Cdcc.stat = 0;
		if ( Cdcc.syncid > 0 )
		{
			ClearEventFlag(Cdcc.syncid, 0);
			ret_v3 = ret_v2;
		}
	}
	if ( ret_v3 > 0 )
	{
		ret_v3 = 0;
		if ( xfer )
		{
			ret_v3 = xfer(
				Cdcc.buf,
				(void *)((uiptr)name - ((uiptr)name & 0xF)),
				(((uiptr)name & 0xF) + namlen + 0xF) & 0xFFFFFFF0,
				CDC_XFER_IN);
			name = (char *)&Cdcc.buf[(uiptr)name & 0xF];
		}
		while ( ret_v3 > 0 )
		{
			int ret_v4;

			ret_v4 = cdfs_lookup(&dirent, name, namlen);
			if ( ret_v4 >= 0 )
			{
				int d_namlen;

				*(acUint32 *)fp->date = *(acUint32 *)&dirent.d_time.t_padding;
				*(acUint32 *)&fp->date[4] = *(acUint32 *)&dirent.d_time.t_day;
				d_namlen = dirent.d_namlen;
				fp->lsn = dirent.d_lsn;
				fp->size = dirent.d_size;
				fp->name[d_namlen] = 59;
				fp->name[d_namlen + 1] = (dirent.d_vol & 0xFF) + 48;
				fp->name[d_namlen + 2] = 0;
				memcpy(fp->name, dirent.d_name, d_namlen);
				ret_v3 = 0;
			}
			else
			{
				ret_v3 = cdfs_recover(ret_v4);
			}
		}
	}
	return cdc_unlock(&Cdcc, ret_v3, AC_CDVDSIF_ID_LOOKUP);
}

int cdc_seek(unsigned int lsn, cdc_done_t done)
{
	acCdvdsifId fno;
	acInt32 v5;
	int v8;
	int ret_v4;

	fno = AC_CDVDSIF_ID_SEEK;
	if ( Cdcc.lockid )
	{
		v5 = 0;
		if ( WaitSema(Cdcc.lockid) )
			v5 = 19;
	}
	else
	{
		v5 = 17;
	}
	if ( v5 )
	{
		Cdcc.error = v5;
		return 0;
	}
	v8 = -16;
	if ( Cdcc.fno == AC_CDVDSIF_ID_NOP )
	{
		v8 = 13;
		Cdcc.fno = AC_CDVDSIF_ID_SEEK;
		Cdcc.stat = 18;
		if ( Cdcc.syncid > 0 )
			ClearEventFlag(Cdcc.syncid, 0);
	}
	ret_v4 = v8;
	if ( v8 <= 0 )
	{
		fno = AC_CDVDSIF_ID_NOP;
	}
	else if ( lsn < Cdcc.cdsize )
	{
		Cdcc.done = done;
		ret_v4 = acd_seek(acd_setup(&Cdcc.acd, (acd_done_t)cdc_done, &Cdcc, -5000000), lsn);
		if ( ret_v4 >= 0 )
			fno = AC_CDVDSIF_ID_NOP;
	}
	else
	{
		ret_v4 = 0;
	}
	return cdc_unlock(&Cdcc, ret_v4, fno);
}

static int cdc_ioctl(acCdvdsifId fno, int state, int tmout, cdc_done_t done)
{
	acInt32 ret;
	int ret_v2;
	acCdvdsifId v12;
	int ret_v4;

	if ( Cdcc.lockid )
	{
		ret = 0;
		if ( WaitSema(Cdcc.lockid) )
			ret = 19;
	}
	else
	{
		ret = 17;
	}
	if ( ret )
	{
		Cdcc.error = ret;
		return 0;
	}
	if ( state == 4 )
	{
		ret_v2 = 0;
	}
	else if ( state >= 5 )
	{
		if ( state == 6 )
		{
			ret_v2 = 1;
		}
		else
		{
			ret_v2 = 10;
			if ( state == 16 )
				ret_v2 = 18;
		}
	}
	else
	{
		ret_v2 = 10;
		if ( state == 1 )
			ret_v2 = 2;
	}
	if ( fno )
	{
		v12 = -16;
		if ( Cdcc.fno == AC_CDVDSIF_ID_NOP )
		{
			v12 = fno;
			Cdcc.fno = fno;
			Cdcc.stat = ret_v2;
			if ( Cdcc.syncid > 0 )
				ClearEventFlag(Cdcc.syncid, 0);
		}
	}
	else
	{
		Cdcc.stat = 0;
		Cdcc.fno = AC_CDVDSIF_ID_NOP;
		if ( Cdcc.syncid > 0 )
			SetEventFlag(Cdcc.syncid, 3u);
		v12 = AC_CDVDSIF_ID_NOP;
	}
	ret_v4 = v12;
	if ( v12 < AC_CDVDSIF_ID_NOP )
	{
		fno = AC_CDVDSIF_ID_NOP;
	}
	else
	{
		Cdcc.done = done;
		ret_v4 = acd_ioctl(acd_setup(&Cdcc.acd, (acd_done_t)cdc_done, &Cdcc, tmout), state);
		if ( ret_v4 >= 0 )
		{
			fno = AC_CDVDSIF_ID_NOP;
		}
	}
	return cdc_unlock(&Cdcc, ret_v4, fno);
}

int cdc_pause(cdc_done_t done)
{
	return cdc_ioctl(AC_CDVDSIF_ID_PAUSE, 1, -5000000, done);
}

int cdc_standby(cdc_done_t done)
{
	return cdc_ioctl(AC_CDVDSIF_ID_STANDBY, 17, -5000000, done);
}

int cdc_stop(cdc_done_t done)
{
	return cdc_ioctl(AC_CDVDSIF_ID_STOP, 4, -5000000, done);
}

int cdc_tray(int mode, u32 *traycnt)
{
	acCdvdsifId tray;
	int v5;
	acCdvdsifId v8;
	struct acd v10;

	tray = Cdcc.tray;
	v5 = 1;
	if ( mode == 2 )
	{
		acd_getmedium(acd_setup(&v10, 0, 0, 5000000));
	}
	else
	{
		int ret;

		ret = 134;
		if ( mode )
			ret = 131;
		v5 = cdc_ioctl(AC_CDVDSIF_ID_TRAY, ret, 5000000, 0);
	}
	v8 = acd_gettray();
	Cdcc.tray = v8;
	*traycnt = tray != v8;
	return v5;
}

int cdc_getpos()
{
	acInt32 v0;
	int ret;

	v0 = 17;
	if ( Cdcc.lockid )
	{
		v0 = 0;
		if ( WaitSema(Cdcc.lockid) )
			v0 = 19;
	}
	if ( v0 )
	{
		Cdcc.error = v0;
		return 0;
	}
	if ( Cdcc.fno )
	{
		if ( Cdcc.fno == AC_CDVDSIF_ID_READ )
		{
			ret = Cdcc.rd.pos << 11;
		}
		else
		{
			Cdcc.error = 19;
			ret = 0;
		}
	}
	else
	{
		ret = 0;
	}
	if ( Cdcc.lockid > 0 )
	{
		SignalSema(Cdcc.lockid);
	}
	return ret;
}

static int cdc_rmode(struct cdc_softc *cdcc, const sceCdRMode *mode)
{
	int spindlctrl;
	struct acd *v5;
	int speed;
	struct acd *acd;
	int v8;
	int trycount;
	struct acd *acd_v7;

	spindlctrl = mode->spindlctrl;
	v5 = acd_setup(&cdcc->acd, 0, 0, 5000000);
	speed = cdcc->rd.maxspeed;
	acd = v5;
	v8 = 0;
	if ( !cdcc->rd.maxspeed )
	{
		v8 = acd_getspeed(v5, 1);
		speed = v8;
	}
	if ( v8 >= 0 )
	{
		cdcc->rd.maxspeed = v8;
		if ( spindlctrl == 1 )
		{
			int v9;

			v9 = 3 * (speed - 1);
			speed = v9 >> 2;
			if ( v9 < 0 )
				speed = (v9 + 3) >> 2;
		}
		if ( acd_setspeed(acd, speed) >= 0 )
			cdcc->rd.spindle = spindlctrl;
	}
	trycount = mode->trycount - 1;
	acd_v7 = acd_setup(&cdcc->acd, 0, 0, 5000000);
	if ( trycount < 0 )
		trycount = 254;
	while ( acd_setretry(acd_v7, trycount) == -61 && acd_getretry(acd_v7) >= 0 )
		;
	return 0;
}

static void cdc_read_done(struct acd *acd, struct cdc_softc *arg, int ret)
{
	cdc_xfer_t xfer;
	acInt32 npos;
	unsigned char *buf;
	int rlen;
	unsigned char *buf_v9;
	unsigned char *src;

	if ( ret >= 0 )
	{
		acInt32 cpos;
		acInt32 size;
		acInt32 xlen;
		acInt32 bsize;

		cpos = arg->rd.pos;
		size = arg->rd.size;
		xfer = arg->rd.xfer;
		npos = cpos + ret;
		xlen = size - cpos;
		buf = &arg->rd.buf[2048 * cpos];
		bsize = arg->rd.bsize;
		rlen = size - (cpos + ret);
		if ( size < cpos + ret )
			npos = arg->rd.size;
		if ( bsize < rlen )
			rlen = arg->rd.bsize;
		if ( bsize < xlen )
			xlen = arg->rd.bsize;
		arg->rd.pos = npos;
		if ( xfer )
		{
			unsigned char *size_v10;
			acInt32 cpos_v11;
			acInt32 bsize_v13;

			size_v10 = buf;
			cpos_v11 = bsize << 11;
			src = arg->buf;
			bsize_v13 = arg->rd.bank;
			buf_v9 = &src[cpos_v11];
			if ( bsize_v13 )
			{
				src += cpos_v11;
				buf_v9 = arg->buf;
			}
			arg->rd.bank = bsize_v13 ^ 1;
			ret = xfer(size_v10, src, xlen << 11, CDC_XFER_OUT);
		}
		else
		{
			buf_v9 = &buf[2048 * ret];
		}
	}
	if ( ret >= 0 )
	{
		ret = npos;
		if ( rlen )
		{
			int size_v14;

			size_v14 = arg->syncid;
			if ( size_v14 > 0 )
				SetEventFlag(size_v14, 2u);
			ret = acd_read(acd, arg->rd.lsn + npos, buf_v9, rlen);
		}
	}
	if ( ret >= 0 )
	{
		if ( rlen )
		{
			int size_v15;

			size_v15 = arg->syncid;
			if ( size_v15 > 0 )
				ClearEventFlag(size_v15, 0xFFFFFFFD);
			return;
		}
	}
	cdc_done(acd, arg, ret);
}

int cdc_read(unsigned int lsn, void *buf, int sectors, const sceCdRMode *mode, cdc_xfer_t xfer, cdc_done_t done)
{
	acCdvdsifId fno;
	acInt32 v11;
	int v14;
	int ret_v4;
	unsigned int n;

	fno = AC_CDVDSIF_ID_READ;
	if ( Cdcc.lockid )
	{
		v11 = 0;
		if ( WaitSema(Cdcc.lockid) )
			v11 = 19;
	}
	else
	{
		v11 = 17;
	}
	if ( v11 )
	{
		Cdcc.error = v11;
		return 0;
	}
	v14 = -16;
	if ( Cdcc.fno == AC_CDVDSIF_ID_NOP )
	{
		v14 = 9;
		Cdcc.fno = AC_CDVDSIF_ID_READ;
		Cdcc.stat = 6;
		if ( Cdcc.syncid > 0 )
			ClearEventFlag(Cdcc.syncid, 0);
	}
	ret_v4 = v14;
	if ( v14 <= 0 )
	{
		fno = AC_CDVDSIF_ID_NOP;
		return cdc_unlock(&Cdcc, ret_v4, fno);
	}
	n = 16;
	if ( lsn >= Cdcc.cdsize )
	{
		ret_v4 = -34;
	}
	else
	{
		if ( lsn + sectors >= Cdcc.cdsize )
			sectors = Cdcc.cdsize - lsn;
		Cdcc.rd.buf = (acUint8 *)buf;
		Cdcc.done = done;
		Cdcc.rd.size = sectors;
		Cdcc.rd.bsize = 16;
		Cdcc.rd.lsn = lsn;
		Cdcc.rd.xfer = xfer;
		Cdcc.rd.pos = 0;
		Cdcc.rd.bank = 0;
		if ( xfer )
			buf = Cdcc.buf;
		if ( (unsigned int)sectors < 0x10 )
			n = sectors;
		ret_v4 = cdc_rmode(&Cdcc, mode);
		// cppcheck-suppress knownConditionTrueFalse
		if ( ret_v4 < 0 )
			return cdc_unlock(&Cdcc, ret_v4, fno);
		ret_v4 = acd_read(acd_setup(&Cdcc.acd, (acd_done_t)cdc_read_done, &Cdcc, -5000000), lsn, buf, n);
		if ( ret_v4 >= 0 )
		{
			fno = AC_CDVDSIF_ID_NOP;
		}
	}
	return cdc_unlock(&Cdcc, ret_v4, fno);
}

int cdc_xfer(void *dst, void *src, int len, enum cdc_xfer_dir dir)
{
	if ( dir )
		memcpy(dst, src, len);
	return len;
}

static int cdc_unlocks(struct cdc_softc *cdcc, int ret, acCdvdsifId fno)
{
	int lockid;

	lockid = cdcc->lockid;
	if ( ret < 0 )
		cdc_errno_set(cdcc, -ret);
	if ( cdcc->fno == fno )
	{
		int eveid;

		eveid = cdcc->syncid;
		cdcc->fno = AC_CDVDSIF_ID_NOP;
		if ( eveid > 0 )
			SetEventFlag(eveid, 3u);
	}
	if ( lockid > 0 )
	{
		SignalSema(lockid);
	}
	return ret >= 0;
}

static void cdc_stream_done(struct acd *acd, struct cdc_softc *arg, int ret)
{
	int v4;
	int v21;
	acSpl state;
	int flg;

	flg = 0;
	v4 = ret;
	if ( ret <= 0 )
	{
		arg->st.flag = 0;
		arg->fno = AC_CDVDSIF_ID_NOP;
		v21 = 0;
		flg = 1;
	}
	else
	{
		unsigned int flag;
		acInt32 tail;
		acInt32 size;
		acInt32 bsize;
		unsigned int cdsize;
		acInt32 head;
		unsigned int lsn;
		int xlen;
		unsigned int v15;
		unsigned char *buf;
		unsigned int size_v12;

		CpuSuspendIntr(&state);
		flag = arg->st.flag;
		tail = arg->st.tail;
		size = arg->st.size;
		bsize = arg->st.bsize;
		cdsize = arg->cdsize;
		head = arg->st.head + v4;
		if ( head >= size )
			head = 0;
		lsn = arg->st.lsn + v4;
		if ( (flag & 0x10) != 0 )
		{
			head = 0;
			tail = 0;
			lsn = arg->st.reqlsn;
			flag = (flag | 0x14) ^ 0x14;
			arg->st.tail = 0;
		}
		else
		{
			if ( head == tail )
			{
				flag = (flag | 6) ^ 2;
			}
		}
		xlen = tail - head;
		if ( head >= tail )
			xlen = size - head;
		v15 = lsn + xlen;
		if ( bsize < xlen )
		{
			xlen = bsize;
			v15 = lsn + bsize;
		}
		if ( v15 >= cdsize )
			xlen = cdsize - lsn;
		buf = &arg->st.buf[2048 * head];
		size_v12 = flag & 0x28;
		if ( size_v12 == 32 )
		{
			arg->st.flag = flag & 0xFFFFFFFD;
		}
		else
		{
			if ( ((flag & 0x28) >= 0x21) ? (size_v12 != 40) : (size_v12 != 8) )
			{
				arg->st.flag = flag;
			}
			else
			{
				arg->st.flag = flag & 4;
			}
		}
		arg->st.head = head;
		arg->st.lsn = lsn;
		CpuResumeIntr(state);
		if ( (flag & 8) != 0 )
		{
			arg->fno = AC_CDVDSIF_ID_NOP;
			v4 = 0;
			v21 = 0;
			flg = 1;
		}
		else
		{
			v4 = 0;
			if ( (flag & 0x20) != 0 )
			{
				v21 = 0;
				flg = 1;
			}
			else
			{
				v4 = -34;
				if ( !xlen )
				{
					v21 = 0;
					flg = 1;
				}
				else
				{
					int eveid;
					eveid = arg->syncid;
					if ( eveid > 0 )
						SetEventFlag(arg->syncid, 3u);
					if ( (flag & 4) == 0 )
					{
						if ( eveid > 0 )
							ClearEventFlag(eveid, 0);
						v4 = acd_read(acd, lsn, buf, xlen);
						if ( v4 < 0 )
						{
							v21 = 0;
							flg = 1;
						}
					}
				}
			}
		}
	}
	if ( flg )
	{
		int eveid_v17;

		if ( v4 < 0 )
			v21 = -v4;
		cdc_errno_set(arg, v21);
		eveid_v17 = arg->syncid;
		if ( eveid_v17 > 0 )
			SetEventFlag(eveid_v17, 3u);
	}
}

static int cdc_stream(struct cdc_softc *cdcc)
{
	int ret;
	int head;
	int tail;
	int xlen;
	int flag;
	unsigned char *buf;
	unsigned int lsn;
	int tail_v9;
	struct acd *ret_v10;
	acSpl state;

	if ( cdcc->fno != AC_CDVDSIF_ID_STREAM )
	{
		return -16;
	}
	CpuSuspendIntr(&state);
	head = cdcc->st.head;
	tail = cdcc->st.tail;
	xlen = tail - head;
	if ( head >= tail )
		xlen = cdcc->st.size - head;
	flag = cdcc->st.flag;
	if ( cdcc->st.bsize < xlen )
		xlen = cdcc->st.bsize;
	buf = &cdcc->st.buf[2048 * head];
	if ( (flag & 6) != 0 || (flag & 0x28) != 0 || (flag & 1) == 0 )
		xlen = 0;
	lsn = 0;
	if ( xlen )
	{
		unsigned int head_v7;

		flag |= 2u;
		head_v7 = cdcc->cdsize;
		if ( (flag & 0x10) != 0 )
		{
			lsn = cdcc->st.reqlsn;
			flag ^= 0x10u;
			cdcc->st.tail = 0;
			cdcc->st.head = 0;
			cdcc->st.lsn = lsn;
		}
		else
		{
			lsn = cdcc->st.lsn;
		}
		if ( lsn + xlen >= head_v7 )
		{
			xlen = head_v7 - lsn;
			if ( head_v7 == lsn )
				flag ^= 2u;
		}
	}
	cdcc->st.flag = flag;
	CpuResumeIntr(state);
	if ( !xlen )
	{
		return 0;
	}
	tail_v9 = cdcc->syncid;
	if ( tail_v9 > 0 )
		ClearEventFlag(tail_v9, 0);
	ret_v10 = acd_setup(&cdcc->acd, (acd_done_t)cdc_stream_done, cdcc, -5000000);
	ret = acd_read(ret_v10, lsn, buf, xlen);
	if ( ret < 0 )
		cdcc->st.flag = 0;
	return ret;
}

int cdc_reads(void *buf, int sectors, int mode, int *errp, cdc_xfer_t xfer)
{
	int v5;
	int v7;
	unsigned char *v11;
	acInt32 rlen;
	int ret_v8;
	acInt32 head;
	int rlen_v10;
	int v17;
	int rlen_v13;
	int xlen;
	unsigned char *tmp;
	int v21;
	acInt32 ret_v17;
	int xlen_v20;
	int ret_v21;
	int rlen_v22;
	int rlen_v23;
	acSpl state;
	int state_2;
	int state_3;
	int v33;
	unsigned char *dst;
	u32 *resbits;

	v5 = sectors;
	dst = (unsigned char *)buf;
	resbits = (u32 *)&v33;
	while ( v5 > 0 )
	{
		acInt32 v6;
		acInt32 ret;
		int len;

		v6 = 17;
		if ( Cdcc.lockid )
		{
			v7 = WaitSema(Cdcc.lockid);
			v6 = 0;
			if ( v7 )
				v6 = 19;
		}
		if ( v6 )
		{
			Cdcc.error = v6;
			break;
		}
		ret = 27;
		if ( Cdcc.fno != AC_CDVDSIF_ID_STREAM )
		{
			ret = -16;
			if ( Cdcc.fno == AC_CDVDSIF_ID_NOP )
			{
				ret = 19;
				Cdcc.fno = AC_CDVDSIF_ID_READS;
				if ( Cdcc.syncid > 0 )
					ClearEventFlag(Cdcc.syncid, 0);
			}
		}
		len = v5;
		if ( ret <= 0 )
		{
			rlen_v23 = 0;
			printf("accdvd:cdc:reads: ACTIVE\n");
		}
		else
		{
			v11 = dst;
			CpuSuspendIntr(&state);
			rlen = Cdcc.st.tail;
			ret_v8 = Cdcc.st.flag;
			head = Cdcc.st.head;
			rlen_v10 = Cdcc.st.lsn >= Cdcc.cdsize;
			CpuResumeIntr(state);
			if ( (ret_v8 & 0x10) != 0 )
			{
				ret = 0;
			}
			if ( ((ret_v8 & 4) == 0) && (head == rlen) )
			{
				v17 = 0;
				if ( rlen_v10 )
					v17 = -34;
				ret = v17;
			}
			else
			{
				int flg;

				flg = 1;
				rlen_v13 = 0;
				if ( rlen >= head )
				{
					xlen = Cdcc.st.size - rlen;
					tmp = v11;
					if ( v5 < Cdcc.st.size - rlen )
						xlen = v5;
					len = v5 - xlen;
					v11 += 2048 * xlen;
					rlen_v13 = xlen;
					v21 = xlen;
					if ( xlen )
					{
						v21 = xfer(tmp, &Cdcc.st.buf[2048 * rlen], xlen * 2048, CDC_XFER_OUT);
						if ( v21 >= 0 )
						{
							CpuSuspendIntr(&state_2);
							ret_v17 = rlen + rlen_v13;
							if ( rlen + rlen_v13 >= Cdcc.st.size )
								ret_v17 = 0;
							Cdcc.st.tail = ret_v17;
							Cdcc.st.flag &= ~4u;
							CpuResumeIntr(state_2);
							v21 = ret_v17;
						}
					}
					rlen = v21;
					if ( rlen < 0 )
					{
						ret = rlen;
						flg = 0;
					}
				}
				if ( rlen < head )
				{
					xlen_v20 = head - rlen;
					ret = head - rlen;
					if ( len < head - rlen )
					{
						xlen_v20 = len;
						ret = len;
					}
					rlen_v13 += xlen_v20;
					rlen_v22 = 0;
					if ( ret )
					{
						ret_v21 = xfer(v11, &Cdcc.st.buf[2048 * rlen], ret << 11, CDC_XFER_OUT);
						if ( ret_v21 >= 0 )
						{
							CpuSuspendIntr(&state_3);
							ret += rlen;
							if ( ret >= Cdcc.st.size )
								ret = 0;
							Cdcc.st.tail = ret;
							Cdcc.st.flag &= ~4u;
							CpuResumeIntr(state_3);
							rlen_v22 = ret;
						}
						ret = ret_v21;
					}
					else
					{
						rlen_v22 = ret;
					}
					if ( rlen_v22 < 0 )
						flg = 0;
				}
				if ( flg )
				{
					ret = rlen_v13;
					if ( rlen_v13 > 0 && xfer )
					{
						xfer(0, 0, 0, CDC_XFER_SYNC);
					}
				}
			}
			rlen_v23 = ret;
			if ( ret < 0 )
			{
				rlen_v23 = 0;
				printf("accdvd:cdc:reads: READ\n");
			}
			else
			{
				v5 -= ret;
				dst += 2048 * ret;
				ret = cdc_stream(&Cdcc);
			}
		}
		cdc_unlocks(&Cdcc, ret, AC_CDVDSIF_ID_READS);
		if ( ret < 0 )
			break;
		if ( rlen_v23 <= 0 )
		{
			if ( !mode )
				break;
			v33 = 1;
			if ( Cdcc.syncid > 0 )
				WaitEventFlag(Cdcc.syncid, 1u, 1, resbits);
		}
	}
	*errp = Cdcc.error;
	return sectors - v5;
}

int cdc_starts(unsigned int lsn, const sceCdRMode *mode)
{
	acInt32 v4;
	int v7;
	int ret;
	acSpl state;

	if ( Cdcc.lockid )
	{
		v4 = 0;
		if ( WaitSema(Cdcc.lockid) )
			v4 = 19;
	}
	else
	{
		v4 = 17;
	}
	if ( v4 )
	{
		Cdcc.error = v4;
		return 0;
	}
	v7 = 27;
	if ( Cdcc.fno != AC_CDVDSIF_ID_STREAM )
	{
		v7 = -16;
		if ( Cdcc.fno == AC_CDVDSIF_ID_NOP )
		{
			v7 = 21;
			Cdcc.fno = AC_CDVDSIF_ID_STARTS;
			if ( Cdcc.syncid > 0 )
				ClearEventFlag(Cdcc.syncid, 0);
		}
	}
	ret = v7;
	if ( v7 > 0 )
	{
		acInt32 flag;

		CpuSuspendIntr(&state);
		flag = Cdcc.st.flag;
		if ( (Cdcc.st.flag & 2) != 0 )
		{
			Cdcc.st.reqlsn = lsn;
			Cdcc.st.flag |= 0x10u;
		}
		else
		{
			flag = 0;
			Cdcc.st.flag = 1;
			Cdcc.st.lsn = lsn;
		}
		CpuResumeIntr(state);
		if ( !flag )
		{
			acCdvdsifId flag_v5;

			flag_v5 = Cdcc.fno;
			Cdcc.fno = AC_CDVDSIF_ID_STREAM;
			ret = cdc_rmode(&Cdcc, mode);
			// cppcheck-suppress knownConditionTrueFalse
			if ( ret < 0 || (ret = cdc_stream(&Cdcc), ret < 0) )
				Cdcc.fno = flag_v5;
		}
	}
	return cdc_unlocks(&Cdcc, ret, AC_CDVDSIF_ID_STARTS);
}

int cdc_stops()
{
	acInt32 v0;
	int sync;
	int ret;
	int sync_v5;
	int ret_v6;
	int state;
	u32 pattern;

	v0 = 17;
	if ( Cdcc.lockid )
	{
		v0 = 0;
		if ( WaitSema(Cdcc.lockid) )
			v0 = 19;
	}
	if ( v0 )
	{
		Cdcc.error = v0;
		return 0;
	}
	sync = 27;
	if ( Cdcc.fno != AC_CDVDSIF_ID_STREAM )
	{
		sync = -16;
		if ( Cdcc.fno == AC_CDVDSIF_ID_NOP )
		{
			sync = 23;
			Cdcc.fno = AC_CDVDSIF_ID_STOPS;
			if ( Cdcc.syncid > 0 )
				ClearEventFlag(Cdcc.syncid, 0);
		}
	}
	ret = sync;
	sync_v5 = 0;
	if ( sync > 0 )
	{
		CpuSuspendIntr(&state);
		sync_v5 = Cdcc.st.flag | 8;
		if ( (Cdcc.st.flag & 2) == 0 )
		{
			Cdcc.fno = AC_CDVDSIF_ID_STOPS;
			sync_v5 = 0;
		}
		Cdcc.st.flag = sync_v5;
		CpuResumeIntr(state);
	}
	ret_v6 = cdc_unlocks(&Cdcc, ret, AC_CDVDSIF_ID_STOPS);
	if ( sync_v5 )
	{
		pattern = 1;
		if ( Cdcc.syncid > 0 )
			WaitEventFlag(Cdcc.syncid, 1u, 1, &pattern);
	}
	return ret_v6;
}

int cdc_pauses()
{
	acInt32 v0;
	int sync;
	int ret;
	int sync_v5;
	int ret_v7;
	int state;
	u32 pattern;

	v0 = 17;
	if ( Cdcc.lockid )
	{
		v0 = 0;
		if ( WaitSema(Cdcc.lockid) )
			v0 = 19;
	}
	if ( v0 )
	{
		Cdcc.error = v0;
		return 0;
	}
	sync = 27;
	if ( Cdcc.fno != AC_CDVDSIF_ID_STREAM )
	{
		sync = -16;
		if ( Cdcc.fno == AC_CDVDSIF_ID_NOP )
		{
			sync = 24;
			Cdcc.fno = AC_CDVDSIF_ID_PAUSES;
			if ( Cdcc.syncid > 0 )
				ClearEventFlag(Cdcc.syncid, 0);
		}
	}
	ret = sync;
	sync_v5 = 0;
	if ( sync > 0 )
	{
		int sync_v6;

		CpuSuspendIntr(&state);
		sync_v6 = Cdcc.st.flag;
		if ( (Cdcc.st.flag & 2) != 0 )
		{
			sync_v6 = Cdcc.st.flag | 0x20;
			Cdcc.st.flag |= 0x20u;
		}
		sync_v5 = sync_v6 & 2;
		CpuResumeIntr(state);
	}
	ret_v7 = cdc_unlocks(&Cdcc, ret, AC_CDVDSIF_ID_PAUSES);
	if ( sync_v5 )
	{
		pattern = 1;
		if ( Cdcc.syncid > 0 )
			WaitEventFlag(Cdcc.syncid, 1u, 1, &pattern);
	}
	return ret_v7;
}

int cdc_resumes()
{
	acInt32 v0;
	int ret_v2;
	int ret_v3;
	acSpl state;

	v0 = 17;
	if ( Cdcc.lockid )
	{
		v0 = 0;
		if ( WaitSema(Cdcc.lockid) )
			v0 = 19;
	}
	if ( v0 )
	{
		Cdcc.error = v0;
		return 0;
	}
	ret_v2 = 27;
	if ( Cdcc.fno == AC_CDVDSIF_ID_STREAM )
	{
		ret_v3 = ret_v2;
	}
	else
	{
		ret_v2 = -16;
		if ( Cdcc.fno )
		{
			ret_v3 = ret_v2;
		}
		else
		{
			ret_v2 = 25;
			ret_v3 = 25;
			Cdcc.fno = AC_CDVDSIF_ID_RESUMES;
			if ( Cdcc.syncid > 0 )
			{
				ClearEventFlag(Cdcc.syncid, 0);
				ret_v3 = ret_v2;
			}
		}
	}
	if ( ret_v3 > 0 )
	{
		int flag;

		CpuSuspendIntr(&state);
		flag = Cdcc.st.flag;
		if ( (Cdcc.st.flag & 2) == 0 && (Cdcc.st.flag & 0x20) != 0 )
			Cdcc.st.flag ^= 0x20u;
		CpuResumeIntr(state);
		ret_v3 = 0;
		if ( (flag & 0x20) != 0 )
			ret_v3 = cdc_stream(&Cdcc);
	}
	return cdc_unlocks(&Cdcc, ret_v3, AC_CDVDSIF_ID_RESUMES);
}

int cdc_inits(void *buf, unsigned int size, unsigned int bsize)
{
	acInt32 ret;
	int v9;
	int ret_v3;

	if ( Cdcc.lockid )
	{
		ret = 0;
		if ( WaitSema(Cdcc.lockid) )
			ret = 19;
	}
	else
	{
		ret = 17;
	}
	if ( ret )
	{
		Cdcc.error = ret;
		return 0;
	}
	v9 = -16;
	if ( Cdcc.fno == AC_CDVDSIF_ID_NOP )
	{
		v9 = 18;
		Cdcc.fno = AC_CDVDSIF_ID_INITS;
		Cdcc.stat = 0;
		if ( Cdcc.syncid > 0 )
			ClearEventFlag(Cdcc.syncid, 0);
	}
	ret_v3 = v9;
	if ( v9 > 0 )
	{
		ret_v3 = 1;
		Cdcc.st.buf = (acUint8 *)buf;
		Cdcc.st.size = size;
		Cdcc.st.bsize = bsize;
		Cdcc.st.head = 0;
		Cdcc.st.tail = 0;
		Cdcc.st.lsn = 0;
		Cdcc.st.reqlsn = 0;
		Cdcc.st.flag = 0;
	}
	return cdc_unlock(&Cdcc, ret_v3, AC_CDVDSIF_ID_INITS);
}

int cdc_seeks(unsigned int lsn)
{
	acInt32 v2;
	int v5;
	int ret;
	acSpl state;

	v2 = 17;
	if ( Cdcc.lockid )
	{
		v2 = 0;
		if ( WaitSema(Cdcc.lockid) )
			v2 = 19;
	}
	if ( v2 )
	{
		Cdcc.error = v2;
		return 0;
	}
	v5 = 27;
	if ( Cdcc.fno != AC_CDVDSIF_ID_STREAM )
	{
		v5 = -16;
		if ( Cdcc.fno == AC_CDVDSIF_ID_NOP )
		{
			v5 = 20;
			Cdcc.fno = AC_CDVDSIF_ID_SEEKS;
			if ( Cdcc.syncid > 0 )
				ClearEventFlag(Cdcc.syncid, 0);
		}
	}
	ret = v5;
	if ( v5 > 0 )
	{
		CpuSuspendIntr(&state);
		Cdcc.st.reqlsn = lsn;
		Cdcc.st.flag |= 0x10u;
		CpuResumeIntr(state);
		ret = 1;
	}
	return cdc_unlocks(&Cdcc, ret, AC_CDVDSIF_ID_SEEKS);
}

int cdc_stats()
{
	acInt32 v0;
	int ret;
	acSpl state;

	v0 = 17;
	if ( Cdcc.lockid )
	{
		v0 = 0;
		if ( WaitSema(Cdcc.lockid) )
			v0 = 19;
	}
	if ( v0 )
	{
		Cdcc.error = v0;
		return 0;
	}
	ret = 27;
	if ( Cdcc.fno != AC_CDVDSIF_ID_STREAM )
	{
		ret = -16;
		if ( Cdcc.fno == AC_CDVDSIF_ID_NOP )
		{
			ret = 22;
			Cdcc.fno = AC_CDVDSIF_ID_STATS;
			if ( Cdcc.syncid > 0 )
				ClearEventFlag(Cdcc.syncid, 0);
		}
	}
	if ( ret > 0 )
	{
		CpuSuspendIntr(&state);
		ret = Cdcc.st.size;
		if ( Cdcc.st.head == Cdcc.st.tail )
		{
			if ( (Cdcc.st.flag & 4) == 0 )
				ret = 0;
		}
		else if ( Cdcc.st.head < Cdcc.st.tail )
		{
			ret = Cdcc.st.size + Cdcc.st.head - Cdcc.st.tail;
		}
		else
		{
			ret = Cdcc.st.head - Cdcc.st.tail;
		}
		CpuResumeIntr(state);
	}
	if ( Cdcc.fno == AC_CDVDSIF_ID_STATS )
	{
		Cdcc.fno = AC_CDVDSIF_ID_NOP;
		if ( Cdcc.syncid > 0 )
			SetEventFlag(Cdcc.syncid, 3u);
	}
	if ( Cdcc.lockid > 0 )
	{
		SignalSema(Cdcc.lockid);
	}
	if ( ret < 0 )
		return 0;
	return ret;
}

int cdc_module_status()
{
	int ret;
	int state;

	CpuSuspendIntr(&state);
	if ( Cdcc.fno )
		ret = 2;
	else
		ret = Cdcc.lockid > 0;
	CpuResumeIntr(state);
	return ret;
}

int cdc_module_start(int argc, char **argv)
{
	int ret;
	int index;
	int v6;
	int ret_v4;
	int ret_v5;
	int lockid;
	acUint8 *buf_v7;
	int syncid;
	acUint8 *buf;

	(void)argc;
	(void)argv;
	ret = cdc_module_status();
	index = 0;
	if ( ret )
	{
		return ret;
	}
	v6 = 0;
	ret_v4 = 0;
	while ( (unsigned int)index < 3 )
	{
		int ret_v3;

		ret_v3 = mods_120[v6].status();
		ret_v4 = ret_v3;
		if ( ret_v3 <= 0 )
		{
			if ( !ret_v3 )
				ret_v4 = mods_120[v6].start(argc, argv);
		}
		else
		{
			ret_v4 = 0;
		}
		if ( ret_v4 < 0 )
		{
			printf("cdc:init_modules:S:%d: error %d\n", index, ret_v4);
			break;
		}
		++index;
		++v6;
	}
	ret_v5 = ret_v4;
	if ( ret_v5 <= 0 )
	{
		return ret_v5;
	}
	lockid = cdc_sem_alloc();
	syncid = cdc_eve_alloc();
	buf_v7 = (acUint8 *)AllocSysMemory(0, 0x10000, 0);
	buf = buf_v7;
	if ( lockid > 0 )
	{
		if ( buf_v7 )
		{
			memset(&Cdcc, 0, sizeof(Cdcc));
			Cdcc.syncid = syncid;
			Cdcc.lockid = lockid;
			Cdcc.buf = buf;
			Cdcc.cdsize = 0;
			Cdcc.rd.spindle = -1;
			Cdcc.rd.maxspeed = 0;
			if ( syncid > 0 )
				SetEventFlag(syncid, 3u);
			return ret_v5;
		}
		DeleteSema(lockid);
	}
	if ( syncid > 0 )
		DeleteEventFlag(syncid);
	if ( buf )
	{
		FreeSysMemory(buf);
	}
	return -6;
}

int cdc_module_stop()
{
	int ret;
	int lockid;
	int i;
	int lockid_v6;
	u32 pattern;

	ret = cdc_module_status();
	if ( ret == 0 )
	{
		return 0;
	}
	lockid = Cdcc.lockid;
	if ( Cdcc.lockid <= 0 )
		return 0;
	cdc_stops();
	WaitSema(lockid);
	pattern = 1;
	if ( Cdcc.syncid > 0 )
		WaitEventFlag(Cdcc.syncid, 1u, 1, &pattern);
	if ( Cdcc.syncid > 0 )
		DeleteEventFlag(Cdcc.syncid);
	if ( Cdcc.buf )
		FreeSysMemory(Cdcc.buf);
	memset(&Cdcc, 0, sizeof(Cdcc));
	SignalSema(lockid);
	DeleteSema(lockid);
	for ( i = 2;; --i )
	{
		int ret_v5;

		ret_v5 = mods_120[i].status();
		lockid_v6 = ret_v5;
		if ( ret_v5 < 0 )
		{
			lockid_v6 = 0;
		}
		if ( ret_v5 > 0 )
		{
			lockid_v6 = mods_120[i].stop();
			if ( lockid_v6 < 0 )
			{
				printf("cdc:init_modules:E:%d: error %d\n", i, lockid_v6);
				return lockid_v6;
			}
		}
	}
	return lockid_v6;
}

int cdc_module_restart(int argc, char **argv)
{
	(void)argc;
	(void)argv;

	return -88;
}

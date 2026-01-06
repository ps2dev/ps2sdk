/*
# _____     ___ ____     ___ ____
#  ____|   |    ____|   |        | |____|
# |     ___|   |____ ___|    ____| |    \    PS2DEV Open Source Project.
#-----------------------------------------------------------------------
# Copyright ps2dev - http://www.ps2dev.org
# Licenced under Academic Free License version 2.0
# Review ps2sdk README & LICENSE files for further details.
*/

#include "irx_imports.h"
#include <errno.h>
#include <loadcore.h>
#include <s147_mmio_hwport.h>
#include <sys/fcntl.h>

IRX_ID("S147CTRL", 2, 8);
// Text section hash:
// 0cc30d14ced9b7ccec282df9d56d0bf4

typedef struct watchdog_info_
{
	int g_watchdog_started;
	iop_sys_clock_t g_watchdog_clock;
} watchdog_info_t;

typedef struct sram_drv_privdata_
{
	u32 m_curpos;
	u32 m_maxpos;
} sram_drv_privdata_t;

static void setup_ac_delay_regs(void);
static int setup_ctrl_ioman_drv(const char *devpfx, const char *devname);
static unsigned int watchdog_alarm_cb(void *userdata);
static int ctrl_drv_op_open(const iop_file_t *f, const char *name, int flags);
static int ctrl_drv_op_close(iop_file_t *f);
static int ctrl_drv_op_read(const iop_file_t *f, void *ptr, int size);
static int ctrl_drv_op_write(const iop_file_t *f, void *ptr, int size);
static int create_ctrl_sema(void);
static int ctrl_do_rtc_read(u32 *rtcbuf);
static int ctrl_do_rtc_read_inner(int flgcnt, int flgmsk);
static int ctrl_do_rtc_write(const u32 *rtcbuf);
static void ctrl_do_rtc_write_inner(int inflg, int flgcnt, int flgmsk);
static int setup_sram_ioman_drv(const char *devpfx, const char *devname);
static int sram_drv_op_open(iop_file_t *f, const char *name, int flags);
static int sram_drv_op_close(iop_file_t *f);
static int sram_drv_op_read(iop_file_t *f, void *ptr, int size);
static int sram_drv_op_write(iop_file_t *f, void *ptr, int size);
static int sram_drv_op_lseek(iop_file_t *f, int offset, int mode);
static int do_rpc_start1(void);
static void rpc_thread1(void *userdata);
static void *rpc_1470000_handler(int fno, void *buffer, int length);
static void *rpc_1470001_handler(int fno, void *buffer, int length);
static void *rpc_1470002_handler(int fno, void *buffer, int length);
static void *rpc_1470003_handler(int fno, void *buffer, int length);
static int do_rpc_start2(void);
static void rpc_thread2(void *userdata);
static void *rpc_1470200_handler(int fno, void *buffer, int length);
static void *rpc_1470201_handler(int fno, void *buffer, int length);

// Unofficial: merge callbacks that use the same return value
IOMAN_RETURN_VALUE_IMPL(0);

static iop_device_ops_t g_ops_ctrl_ioman = {
	IOMAN_RETURN_VALUE(0),
	IOMAN_RETURN_VALUE(0),
	IOMAN_RETURN_VALUE(0),
	(void *)&ctrl_drv_op_open,
	(void *)&ctrl_drv_op_close,
	(void *)&ctrl_drv_op_read,
	(void *)&ctrl_drv_op_write,
	IOMAN_RETURN_VALUE(0),
	IOMAN_RETURN_VALUE(0),
	IOMAN_RETURN_VALUE(0),
	IOMAN_RETURN_VALUE(0),
	IOMAN_RETURN_VALUE(0),
	IOMAN_RETURN_VALUE(0),
	IOMAN_RETURN_VALUE(0),
	IOMAN_RETURN_VALUE(0),
	IOMAN_RETURN_VALUE(0),
	IOMAN_RETURN_VALUE(0),
};
// Unofficial: move to bss
static int g_rpc_started;
// Unofficial: move to bss
static int g_watchdog_count_1;
// Unofficial: move to bss
static char g_watchdog_flag_1;
// Unofficial: move to bss
static u32 g_max_timer_counter;
static iop_device_ops_t g_ops_sram_ioman = {
	IOMAN_RETURN_VALUE(0),
	IOMAN_RETURN_VALUE(0),
	IOMAN_RETURN_VALUE(0),
	(void *)&sram_drv_op_open,
	(void *)&sram_drv_op_close,
	(void *)&sram_drv_op_read,
	(void *)&sram_drv_op_write,
	(void *)&sram_drv_op_lseek,
	IOMAN_RETURN_VALUE(0),
	IOMAN_RETURN_VALUE(0),
	IOMAN_RETURN_VALUE(0),
	IOMAN_RETURN_VALUE(0),
	IOMAN_RETURN_VALUE(0),
	IOMAN_RETURN_VALUE(0),
	IOMAN_RETURN_VALUE(0),
	IOMAN_RETURN_VALUE(0),
	IOMAN_RETURN_VALUE(0),
};
static iop_device_t g_drv_ctrl_ioman;
static int g_rtc_flag;
static int g_timer_id;
static iop_sema_t g_ctrl_sema_param;
static int g_ctrl_sema_id;
static iop_device_t g_drv_sram_ioman;
static int g_rpc1_buf[8];
static int g_rpc2_buf[260];
static watchdog_info_t g_watchdog_info;

int _start(int ac, char **av)
{
	(void)ac;
	(void)av;
	Kprintf("\n");
	Kprintf("s147ctrl.irx: System147 Control/SRAM Driver v%d.%d\n", 2, 8);
	setup_ac_delay_regs();
	if ( setup_ctrl_ioman_drv("ctrl", "Ctrl") < 0 )
	{
		Kprintf("s147ctrl.irx: Ctrl initialize failed\n");
		return MODULE_NO_RESIDENT_END;
	}
	// cppcheck-suppress knownConditionTrueFalse
	if ( setup_sram_ioman_drv("sram", "SRAM") < 0 )
	{
		Kprintf("s147ctrl.irx: Sram initialize failed\n");
		return MODULE_NO_RESIDENT_END;
	}
	return MODULE_RESIDENT_END;
}

static void setup_ac_delay_regs(void)
{
	SetAcMemDelayReg(0x261A2122);
	SetAcIoDelayReg(0xA61A0166);
}

static int setup_ctrl_ioman_drv(const char *devpfx, const char *devname)
{
	g_watchdog_info.g_watchdog_started = 1;
	USec2SysClock(0x4E20, &g_watchdog_info.g_watchdog_clock);
	SetAlarm(&g_watchdog_info.g_watchdog_clock, watchdog_alarm_cb, &g_watchdog_info);
	if ( create_ctrl_sema() < 0 )
		return -1;
	g_drv_ctrl_ioman.name = devpfx;
	g_drv_ctrl_ioman.type = IOP_DT_FS;
	g_drv_ctrl_ioman.version = 0;
	g_drv_ctrl_ioman.desc = devname;
	g_drv_ctrl_ioman.ops = &g_ops_ctrl_ioman;
	DelDrv(devpfx);
	AddDrv(&g_drv_ctrl_ioman);
	return 0;
}

static unsigned int watchdog_alarm_cb(void *userdata)
{
	int state;
	u8 unk34_tmp;
	watchdog_info_t *wdi;
	USE_S147_DEV9_MEM_MMIO();
	USE_S147LINK_DEV9_MEM_MMIO();

	wdi = (watchdog_info_t *)userdata;
	if ( wdi->g_watchdog_started != 1 )
	{
		s147_dev9_mem_mmio->m_led = 3;
		return 0;
	}
	CpuSuspendIntr(&state);
	s147link_dev9_mem_mmio->m_watchdog_flag_unk34 = 0;
	unk34_tmp = s147link_dev9_mem_mmio->m_watchdog_flag_unk34;
	CpuResumeIntr(state);
	if ( unk34_tmp == 0x3E )
	{
		s147_dev9_mem_mmio->m_watchdog_flag2 = 0;
		// Unofficial: add 1 here
		s147_dev9_mem_mmio->m_led = g_watchdog_flag_1 + 1;
		g_watchdog_flag_1 = ((((unsigned int)g_watchdog_count_1 >> 3) & 1) != 0) ? 1 : 0;
		g_watchdog_count_1 += 1;
	}
	return wdi->g_watchdog_clock.lo;
}

static int ctrl_drv_op_open(const iop_file_t *f, const char *name, int flags)
{
	int state;

	(void)flags;
	if ( f->unit != 99 )
		return 0;
	if ( !strcmp(name, "watchdog-start") )
	{
		Kprintf("s147ctrl.irx: wdt-start\n");
		CpuSuspendIntr(&state);
		g_watchdog_info.g_watchdog_started = 1;
		CpuResumeIntr(state);
	}
	else if ( !strcmp(name, "watchdog-stop") )
	{
		Kprintf("s147ctrl.irx: wdt-stop\n");
		CpuSuspendIntr(&state);
		g_watchdog_info.g_watchdog_started = 0;
		CpuResumeIntr(state);
	}
	else if ( !strcmp(name, "rpcserv-start") )
	{
		Kprintf("s147ctrl.irx: rpcserv-start\n");
		if ( !g_rpc_started )
		{
			do_rpc_start1();
			do_rpc_start2();
			CpuSuspendIntr(&state);
			g_rpc_started = 1;
			CpuResumeIntr(state);
		}
	}
	return 0;
}

static int ctrl_drv_op_close(iop_file_t *f)
{
	(void)f;
	return 0;
}

static int ctrl_drv_op_read(const iop_file_t *f, void *ptr, int size)
{
	int unit;
	int retres;
	USE_S147_DEV9_MEM_MMIO();

	unit = f->unit;
	switch ( unit )
	{
		case 4:
			if ( size != 0x1C )
				return -EINVAL;
			retres = ctrl_do_rtc_read(ptr);
			if ( retres < 0 )
				Kprintf("s147ctrl.irx: RTC Read failed (%d)\n", retres);
			return retres;
		case 12:
			if ( size != 2 )
				return -EINVAL;
			*(u8 *)ptr = s147_dev9_mem_mmio->m_security_unlock_set1;
			*((u8 *)ptr + 1) = s147_dev9_mem_mmio->m_security_unlock_set2;
			return 2;
		default:
			if ( size != 1 )
				return -EINVAL;
			*(u8 *)ptr = *(u8 *)(unit + 0xB0000000);
			return 1;
	}
}

static int ctrl_drv_op_write(const iop_file_t *f, void *ptr, int size)
{
	int unit;
	int retres;
	USE_S147_DEV9_MEM_MMIO();

	unit = f->unit;
	switch ( unit )
	{
		case 4:
			if ( size != 0x1C )
				return -EINVAL;
			retres = ctrl_do_rtc_write(ptr);
			if ( retres < 0 )
				Kprintf("s147ctrl.irx: RTC Write failed (%d)\n", retres);
			return retres;
		case 12:
			if ( size != 2 )
				return -EINVAL;
			s147_dev9_mem_mmio->m_security_unlock_set1 = *(u8 *)ptr;
			s147_dev9_mem_mmio->m_security_unlock_set2 = *((u8 *)ptr + 1);
			return 2;
		default:
			if ( size != 1 )
				return -EINVAL;
			*(u8 *)(unit + 0xB0000000) = *(u8 *)ptr;
			return 1;
	}
}

static int create_ctrl_sema(void)
{
	g_ctrl_sema_param.initial = 1;
	g_ctrl_sema_param.max = 1;
	g_ctrl_sema_param.attr = SA_THPRI;
	g_ctrl_sema_id = CreateSema(&g_ctrl_sema_param);
	if ( g_ctrl_sema_id < 0 )
	{
		Kprintf("s147ctrl.irx: CreateSema error (%d)\n", g_ctrl_sema_id);
		return -1;
	}
	return 0;
}

static int ctrl_do_rtc_read(u32 *rtcbuf)
{
	USE_S147_DEV9_MEM_MMIO();

	WaitSema(g_ctrl_sema_id);
	g_timer_id = AllocHardTimer(1, 0x20, 1);
	if ( g_timer_id < 0 )
		return g_timer_id;
	// Unofficial: omit SetupHardTimer/StartHardTimer
	g_max_timer_counter = 0x40;
	while ( GetTimerCounter(g_timer_id) < g_max_timer_counter )
		;
	g_max_timer_counter += 0x40;
	s147_dev9_mem_mmio->m_rtc_flag = 1;
	while ( GetTimerCounter(g_timer_id) < g_max_timer_counter )
		;
	g_max_timer_counter += 0x40;
	s147_dev9_mem_mmio->m_rtc_flag = 9;
	while ( GetTimerCounter(g_timer_id) < g_max_timer_counter )
		;
	g_max_timer_counter += 0x40;
	rtcbuf[6] = ctrl_do_rtc_read_inner(8, 0x7F);
	rtcbuf[5] = ctrl_do_rtc_read_inner(8, 0x7F);
	rtcbuf[4] = ctrl_do_rtc_read_inner(8, 0x3F);
	rtcbuf[3] = ctrl_do_rtc_read_inner(4, 7);
	rtcbuf[2] = ctrl_do_rtc_read_inner(8, 0x3F);
	rtcbuf[1] = ctrl_do_rtc_read_inner(8, 0x1F);
	*rtcbuf = ctrl_do_rtc_read_inner(8, 0xFF);
	s147_dev9_mem_mmio->m_rtc_flag = 1;
	while ( GetTimerCounter(g_timer_id) < g_max_timer_counter )
		;
	g_max_timer_counter += 0x40;
	s147_dev9_mem_mmio->m_rtc_flag = 1;
	while ( GetTimerCounter(g_timer_id) < g_max_timer_counter )
		;
	g_max_timer_counter += 0x40;
	// Unofficial: omit StopHardTimer
	FreeHardTimer(g_timer_id);
	g_max_timer_counter = 0;
	SignalSema(g_ctrl_sema_id);
	return 0x1C;
}

static int ctrl_do_rtc_read_inner(int flgcnt, int flgmsk)
{
	int i;
	USE_S147_DEV9_MEM_MMIO();

	g_rtc_flag = 0;
	for ( i = 0; i < flgcnt; i += 1 )
	{
		s147_dev9_mem_mmio->m_rtc_flag = 0xB;
		while ( GetTimerCounter(g_timer_id) < g_max_timer_counter )
			;
		g_max_timer_counter += 0x40;
		g_rtc_flag |= (s147_dev9_mem_mmio->m_rtc_flag & 1) << i;
		s147_dev9_mem_mmio->m_rtc_flag = 9;
		while ( GetTimerCounter(g_timer_id) < g_max_timer_counter )
			;
		g_max_timer_counter += 0x40;
	}
	return g_rtc_flag & flgmsk;
}

static int ctrl_do_rtc_write(const u32 *rtcbuf)
{
	USE_S147_DEV9_MEM_MMIO();

	WaitSema(g_ctrl_sema_id);
	g_timer_id = AllocHardTimer(1, 0x20, 1);
	if ( g_timer_id < 0 )
		return g_timer_id;
	// Unofficial: omit SetupHardTimer/StartHardTimer
	g_max_timer_counter = 0x40;
	while ( GetTimerCounter(g_timer_id) < g_max_timer_counter )
		;
	g_max_timer_counter += 0x40;
	s147_dev9_mem_mmio->m_rtc_flag = 5;
	while ( GetTimerCounter(g_timer_id) < g_max_timer_counter )
		;
	g_max_timer_counter += 0x40;
	s147_dev9_mem_mmio->m_rtc_flag = 0xD;
	while ( GetTimerCounter(g_timer_id) < g_max_timer_counter )
		;
	g_max_timer_counter += 0x40;
	ctrl_do_rtc_write_inner(rtcbuf[6], 8, 0x7F);
	ctrl_do_rtc_write_inner(rtcbuf[5], 8, 0x7F);
	ctrl_do_rtc_write_inner(rtcbuf[4], 8, 0x3F);
	ctrl_do_rtc_write_inner(rtcbuf[3], 4, 7);
	ctrl_do_rtc_write_inner(rtcbuf[2], 8, 0x3F);
	ctrl_do_rtc_write_inner(rtcbuf[1], 8, 0x1F);
	ctrl_do_rtc_write_inner(*rtcbuf, 8, 0xFF);
	s147_dev9_mem_mmio->m_rtc_flag = 5;
	while ( GetTimerCounter(g_timer_id) < g_max_timer_counter )
		;
	g_max_timer_counter += 0x40;
	s147_dev9_mem_mmio->m_rtc_flag = 1;
	while ( GetTimerCounter(g_timer_id) < g_max_timer_counter )
		;
	g_max_timer_counter += 0x40;
	// Unofficial: omit StopHardTimer
	FreeHardTimer(g_timer_id);
	g_max_timer_counter = 0;
	SignalSema(g_ctrl_sema_id);
	return 0x1C;
}

static void ctrl_do_rtc_write_inner(int inflg, int flgcnt, int flgmsk)
{
	int i;
	unsigned int xval;
	USE_S147_DEV9_MEM_MMIO();

	xval = inflg & flgmsk;
	for ( i = 0; i < flgcnt; i += 1 )
	{
		s147_dev9_mem_mmio->m_rtc_flag = (xval & 1) | 0xC;
		while ( GetTimerCounter(g_timer_id) < g_max_timer_counter )
			;
		g_max_timer_counter += 0x40;
		s147_dev9_mem_mmio->m_rtc_flag = (xval & 1) | 0xE;
		while ( GetTimerCounter(g_timer_id) < g_max_timer_counter )
			;
		g_max_timer_counter += 0x40;
		xval >>= 1;
	}
	s147_dev9_mem_mmio->m_rtc_flag = (xval & 1) | 0xC;
	while ( GetTimerCounter(g_timer_id) < g_max_timer_counter )
		;
	g_max_timer_counter += 0x40;
}

static int setup_sram_ioman_drv(const char *devpfx, const char *devname)
{
	g_drv_sram_ioman.name = devpfx;
	g_drv_sram_ioman.type = IOP_DT_FS;
	g_drv_sram_ioman.version = 0;
	g_drv_sram_ioman.desc = devname;
	g_drv_sram_ioman.ops = &g_ops_sram_ioman;
	DelDrv(devpfx);
	AddDrv(&g_drv_sram_ioman);
	return 0;
}

static int sram_drv_op_open(iop_file_t *f, const char *name, int flags)
{
	sram_drv_privdata_t *privdata;
	int state;

	(void)name;
	(void)flags;
	CpuSuspendIntr(&state);
	f->privdata = AllocSysMemory(ALLOC_FIRST, sizeof(sram_drv_privdata_t), 0);
	CpuResumeIntr(state);
	privdata = (sram_drv_privdata_t *)f->privdata;
	privdata->m_curpos = 0;
	privdata->m_maxpos = 0x8000;
	return 0;
}

static int sram_drv_op_close(iop_file_t *f)
{
	int state;

	if ( !f->privdata )
		return 0;
	CpuSuspendIntr(&state);
	FreeSysMemory(f->privdata);
	CpuResumeIntr(state);
	f->privdata = 0;
	return 0;
}

static int sram_drv_op_read(iop_file_t *f, void *ptr, int size)
{
	int sizeb;
	sram_drv_privdata_t *privdata;

	privdata = (sram_drv_privdata_t *)f->privdata;
	if ( (s32)privdata->m_curpos >= (s32)privdata->m_maxpos )
		return 0;
	sizeb = ((s32)privdata->m_maxpos < (s32)(privdata->m_curpos + size)) ? (privdata->m_maxpos - privdata->m_curpos) :
																																				 (u32)size;
	memcpy(ptr, (const void *)(privdata->m_curpos + 0xB0C00000), sizeb);
	privdata->m_curpos += sizeb;
	return sizeb;
}

static int sram_drv_op_write(iop_file_t *f, void *ptr, int size)
{
	int sizeb;
	sram_drv_privdata_t *privdata;
	USE_S147_DEV9_MEM_MMIO();

	privdata = (sram_drv_privdata_t *)f->privdata;
	if ( (s32)privdata->m_curpos >= (s32)privdata->m_maxpos )
		return 0;
	sizeb = ((s32)privdata->m_maxpos < (s32)(privdata->m_curpos + size)) ? (privdata->m_maxpos - privdata->m_curpos) :
																																				 (u32)size;
	s147_dev9_mem_mmio->m_sram_write_flag = 1;
	memcpy((void *)(privdata->m_curpos + 0xB0C00000), ptr, sizeb);
	s147_dev9_mem_mmio->m_sram_write_flag = 0;
	privdata->m_curpos += sizeb;
	return sizeb;
}

static int sram_drv_op_lseek(iop_file_t *f, int offset, int mode)
{
	sram_drv_privdata_t *privdata;

	privdata = (sram_drv_privdata_t *)f->privdata;
	switch ( mode )
	{
		case SEEK_SET:
			privdata->m_curpos = offset;
			break;
		case SEEK_CUR:
			privdata->m_curpos += offset;
			break;
		case SEEK_END:
			privdata->m_curpos = privdata->m_maxpos + offset;
			break;
		default:
			return -EINVAL;
	}
	if ( (s32)privdata->m_maxpos < (s32)privdata->m_curpos )
	{
		privdata->m_curpos = privdata->m_maxpos;
		return -EINVAL;
	}
	return privdata->m_curpos;
}

static int do_rpc_start1(void)
{
	iop_thread_t thparam;
	int thid;

	thparam.attr = TH_C;
	thparam.thread = rpc_thread1;
	thparam.priority = 10;
	thparam.stacksize = 0x800;
	thparam.option = 0;
	thid = CreateThread(&thparam);
	if ( thid <= 0 )
		return 1;
	StartThread(thid, 0);
	return 0;
}

static void rpc_thread1(void *userdata)
{
	SifRpcDataQueue_t qd;
	SifRpcServerData_t sd[4];

	(void)userdata;
	sceSifInitRpc(0);
	sceSifSetRpcQueue(&qd, GetThreadId());
	sceSifRegisterRpc(&sd[0], 0x1470000, rpc_1470000_handler, g_rpc1_buf, 0, 0, &qd);
	sceSifRegisterRpc(&sd[1], 0x1470001, rpc_1470001_handler, g_rpc1_buf, 0, 0, &qd);
	sceSifRegisterRpc(&sd[2], 0x1470002, rpc_1470002_handler, g_rpc1_buf, 0, 0, &qd);
	sceSifRegisterRpc(&sd[3], 0x1470003, rpc_1470003_handler, g_rpc1_buf, 0, 0, &qd);
	sceSifRpcLoop(&qd);
}

static void *rpc_1470000_handler(int fno, void *buffer, int length)
{
	USE_S147_DEV9_MEM_MMIO();

	(void)length;
	switch ( fno )
	{
		case 1:
			s147_dev9_mem_mmio->m_led = *(u8 *)buffer;
			*(u32 *)buffer = 0;
			break;
		case 2:
			s147_dev9_mem_mmio->m_security_unlock_unlock = *(u8 *)buffer;
			*(u32 *)buffer = 0;
			break;
		case 3:
			s147_dev9_mem_mmio->m_unk03 = *(u8 *)buffer;
			*(u32 *)buffer = 0;
			break;
		case 4:
			s147_dev9_mem_mmio->m_rtc_flag = *(u8 *)buffer;
			*(u32 *)buffer = 0;
			break;
		case 5:
			s147_dev9_mem_mmio->m_watchdog_flag2 = *(u8 *)buffer;
			*(u32 *)buffer = 0;
			break;
		case 12:
			s147_dev9_mem_mmio->m_security_unlock_set1 = *(u8 *)buffer;
			*(u32 *)buffer = 0;
			break;
		case 13:
			s147_dev9_mem_mmio->m_security_unlock_set2 = *(u8 *)buffer;
			*(u32 *)buffer = 0;
			break;
		default:
			*(u32 *)buffer = -22;
			break;
	}
	return buffer;
}

static void *rpc_1470001_handler(int fno, void *buffer, int length)
{
	USE_S147_DEV9_MEM_MMIO();

	(void)length;
	switch ( fno )
	{
		case 0:
			*(u8 *)buffer = s147_dev9_mem_mmio->m_unk00;
			*((u32 *)buffer + 1) = 0;
			break;
		case 1:
			*(u8 *)buffer = s147_dev9_mem_mmio->m_led;
			*((u32 *)buffer + 1) = 0;
			break;
		case 2:
			*(u8 *)buffer = s147_dev9_mem_mmio->m_security_unlock_unlock;
			*((u32 *)buffer + 1) = 0;
			break;
		case 3:
			*(u8 *)buffer = s147_dev9_mem_mmio->m_unk03;
			*((u32 *)buffer + 1) = 0;
			break;
		case 4:
			*(u8 *)buffer = s147_dev9_mem_mmio->m_rtc_flag;
			*((u32 *)buffer + 1) = 0;
			break;
		case 5:
			*(u8 *)buffer = s147_dev9_mem_mmio->m_watchdog_flag2;
			*((u32 *)buffer + 1) = 0;
			break;
		case 6:
			*(u8 *)buffer = s147_dev9_mem_mmio->m_unk06;
			*((u32 *)buffer + 1) = 0;
			break;
		case 12:
			*(u8 *)buffer = s147_dev9_mem_mmio->m_security_unlock_set1;
			*((u32 *)buffer + 1) = 0;
			break;
		case 13:
			*(u8 *)buffer = s147_dev9_mem_mmio->m_security_unlock_set2;
			*((u32 *)buffer + 1) = 0;
			break;
		default:
			*(u8 *)buffer = 0;
			*((u32 *)buffer + 1) = -22;
			break;
	}
	return buffer;
}

static void *rpc_1470002_handler(int fno, void *buffer, int length)
{
	(void)length;
	*(u32 *)buffer = (fno == 4) ? ctrl_do_rtc_write(buffer) : -22;
	return buffer;
}

static void *rpc_1470003_handler(int fno, void *buffer, int length)
{
	(void)length;
	if ( fno != 4 )
	{
		*(u32 *)buffer = 0;
		*((u32 *)buffer + 1) = 0;
		*((u32 *)buffer + 2) = 0;
		*((u32 *)buffer + 3) = 0;
		*((u32 *)buffer + 4) = 0;
		*((u32 *)buffer + 5) = 0;
		*((u32 *)buffer + 6) = 0;
		*((u32 *)buffer + 7) = -22;
		return buffer;
	}
	*((u32 *)buffer + 7) = ctrl_do_rtc_read(buffer);
	return buffer;
}

static int do_rpc_start2(void)
{
	iop_thread_t thparam;
	int thid;

	thparam.attr = TH_C;
	thparam.thread = rpc_thread2;
	thparam.priority = 10;
	thparam.stacksize = 0x800;
	thparam.option = 0;
	thid = CreateThread(&thparam);
	if ( thid <= 0 )
		return 1;
	StartThread(thid, 0);
	return 0;
}

static void rpc_thread2(void *userdata)
{
	SifRpcDataQueue_t qd;
	SifRpcServerData_t sd[2];

	(void)userdata;
	sceSifInitRpc(0);
	sceSifSetRpcQueue(&qd, GetThreadId());
	sceSifRegisterRpc(&sd[0], 0x1470200, rpc_1470200_handler, g_rpc2_buf, 0, 0, &qd);
	sceSifRegisterRpc(&sd[1], 0x1470201, rpc_1470201_handler, g_rpc2_buf, 0, 0, &qd);
	sceSifRpcLoop(&qd);
}

static void *rpc_1470200_handler(int fno, void *buffer, int length)
{
	USE_S147_DEV9_MEM_MMIO();

	(void)length;
	if ( (unsigned int)fno >= 3 )
	{
		*(u32 *)buffer = -1;
		return buffer;
	}
	s147_dev9_mem_mmio->m_sram_write_flag = 1;
	memcpy((void *)(*((u32 *)buffer + 256) + 0xB0C00000), buffer, *((u32 *)buffer + 257));
	s147_dev9_mem_mmio->m_sram_write_flag = 0;
	*(u32 *)buffer = 0;
	return buffer;
}

static void *rpc_1470201_handler(int fno, void *buffer, int length)
{
	(void)length;
	if ( (unsigned int)fno >= 3 )
	{
		memset(buffer, 0, *((u32 *)buffer + 1));
		*((u32 *)buffer + 256) = -1;
		return buffer;
	}
	memcpy(buffer, (const void *)(*(u32 *)buffer + 0xB0C00000), *((u32 *)buffer + 1));
	*((u32 *)buffer + 256) = 0;
	return buffer;
}

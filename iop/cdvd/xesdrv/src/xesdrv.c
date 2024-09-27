/*
# _____     ___ ____     ___ ____
#  ____|   |    ____|   |        | |____|
# |     ___|   |____ ___|    ____| |    \    PS2DEV Open Source Project.
#-----------------------------------------------------------------------
# Copyright ps2dev - http://www.ps2dev.org
# Licenced under Academic Free License version 2.0
# Review ps2sdk README & LICENSE files for further details.
*/

#include "errno.h"
#include "irx_imports.h"
#include "loadcore.h"
#define MODNAME "XESDRV"

#ifdef DEBUG
#define DPRINTF(x...) printf(MODNAME ": " x)
#else
#define DPRINTF(x...)
#endif

static int module_start(int argc, char *argv[], void *startaddr, ModuleInfo_t *mi);
static int module_stop(int argc, char *argv[], void *startaddr, ModuleInfo_t *mi);
static int esdrv_df_init(iomanX_iop_device_t *dev);
static int esdrv_df_exit(iomanX_iop_device_t *dev);
static int esdrv_df_ioctl(iomanX_iop_file_t *f, int cmd, void *param);
static int esdrv_df_devctl(
	iomanX_iop_file_t *f, const char *name, int cmd, void *arg, unsigned int arglen, void *buf, unsigned int buflen);
static int
esdrv_df_ioctl2(iomanX_iop_file_t *f, int cmd, void *arg, unsigned int arglen, void *buf, unsigned int buflen);
static s64 esdrv_df_null_long();
static int
esioctl2_func_1(iomanX_iop_file_t *f, int cmd, void *arg, unsigned int arglen, void *buf, unsigned int buflen);
static int
esioctl2_func_10(iomanX_iop_file_t *f, int cmd, void *arg, unsigned int arglen, void *buf, unsigned int buflen);
static int
esioctl2_func_2(iomanX_iop_file_t *f, int cmd, void *arg, unsigned int arglen, void *buf, unsigned int buflen);
static int
esioctl2_func_3(iomanX_iop_file_t *f, int cmd, void *arg, unsigned int arglen, void *buf, unsigned int buflen);
static int
esioctl2_func_4(iomanX_iop_file_t *f, int cmd, void *arg, unsigned int arglen, void *buf, unsigned int buflen);
static int
esioctl2_func_5(iomanX_iop_file_t *f, int cmd, void *arg, unsigned int arglen, void *buf, unsigned int buflen);
static int
esioctl2_func_6(iomanX_iop_file_t *f, int cmd, void *arg, unsigned int arglen, void *buf, unsigned int buflen);
static int
esioctl2_func_7(iomanX_iop_file_t *f, int cmd, void *arg, unsigned int arglen, void *buf, unsigned int buflen);
static int
esioctl2_func_8(iomanX_iop_file_t *f, int cmd, void *arg, unsigned int arglen, void *buf, unsigned int buflen);
static int
esioctl2_func_9(iomanX_iop_file_t *f, int cmd, void *arg, unsigned int arglen, void *buf, unsigned int buflen);
static int
esioctl2_func_a(iomanX_iop_file_t *f, int cmd, void *arg, unsigned int arglen, void *buf, unsigned int buflen);
static int
esioctl2_func_b(iomanX_iop_file_t *f, int cmd, void *arg, unsigned int arglen, void *buf, unsigned int buflen);
static int
esioctl2_func_c(iomanX_iop_file_t *f, int cmd, void *arg, unsigned int arglen, void *buf, unsigned int buflen);
static int
esioctl2_func_d(iomanX_iop_file_t *f, int cmd, void *arg, unsigned int arglen, void *buf, unsigned int buflen);
static int
esioctl2_func_e(iomanX_iop_file_t *f, int cmd, void *arg, unsigned int arglen, void *buf, unsigned int buflen);
static int
esioctl2_func_f(iomanX_iop_file_t *f, int cmd, void *arg, unsigned int arglen, void *buf, unsigned int buflen);

typedef struct es_regs_
{
	vu8 r_es00;
	vu8 padding01;
	vu8 r_es02;
	vu8 padding03;
	vu8 r_es04;
	vu8 padding05;
	vu8 r_es06;
	vu8 padding07;
	vu8 r_es08;
	vu8 padding09;
	vu8 padding0A;
	vu8 padding0B;
	vu8 r_es0C;
	vu8 padding0D;
	vu8 r_es0E;
	vu8 padding0F;
	vu8 r_es10;
	vu8 padding11;
	vu8 r_es12;
	vu8 padding13;
	vu8 r_es14;
	vu8 padding15;
	vu8 padding16;
	vu8 padding17;
	vu8 r_es18;
	vu8 padding19;
	vu8 r_es1A;
	vu8 padding1B;
	vu8 r_es1C;
	vu8 padding1D;
	vu8 padding1E;
	vu8 padding1F;
	vu8 r_es20;
	vu8 padding21;
	vu8 r_es22;
	vu8 padding23;
	vu8 padding24;
	vu8 padding25;
	vu8 padding26;
	vu8 padding27;
	vu8 r_es28;
	vu8 padding29;
	vu8 r_es2A;
	vu8 padding2B;
	vu8 r_es2C;
	vu8 padding2D;
	vu8 padding2E;
	vu8 padding2F;
	vu8 padding30;
	vu8 padding31;
	vu8 padding32;
	vu8 padding33;
	vu8 r_es34;
	vu8 padding35;
	vu8 r_es36;
	vu8 padding37;
	vu8 r_es38;
	vu8 padding39;
	vu8 r_es3A;
	vu8 padding3B;
	vu8 r_es3C;
	vu8 padding3D;
	vu8 r_es3E;
	vu8 padding3F;
	vu8 r_es40;
	vu8 padding41;
	vu8 r_es42;
	vu8 padding43;
	vu8 r_es44;
	vu8 padding45;
	vu8 r_es46;
	vu8 padding47;
	vu8 r_es48;
	vu8 padding49;
	vu8 r_es4A;
	vu8 padding4B;
	vu8 r_es4C;
	vu8 padding4D;
	vu8 r_es4E;
	vu8 padding4F;
	vu8 r_es50;
	vu8 padding51;
	vu8 r_es52;
	vu8 padding53;
	vu8 r_es54;
	vu8 padding55;
	vu8 r_es56;
	vu8 padding57;
	vu8 r_es58;
	vu8 padding59;
	vu8 r_es5A;
	vu8 padding5B;
	vu8 r_es5C;
	vu8 padding5D;
	vu8 r_es5E;
	vu8 padding5F;
	vu8 r_es60;
	vu8 padding61;
	vu8 r_es62;
	vu8 padding63;
	vu8 r_es64;
	vu8 padding65;
	vu8 r_es66;
	vu8 padding67;
	vu8 r_es68;
	vu8 padding69;
	vu8 r_es6A;
	vu8 padding6B;
	vu8 r_es6C;
	vu8 padding6D;
	vu8 r_es6E;
	vu8 padding6F;
	vu8 r_es70;
	vu8 padding71;
	vu8 r_es72;
	vu8 padding73;
	vu8 r_es74;
	vu8 padding75;
	vu8 r_es76;
	vu8 padding77;
	vu8 r_es78;
	vu8 padding79;
	vu8 r_es7A;
	vu8 padding7B;
	vu8 r_es7C;
	vu8 padding7D;
	vu8 r_es7E;
	vu8 padding7F;
} es_regs_t;

static es_regs_t *const es_regs = (void *)0xBF415000;

struct DevctlCmdTbl_t
{
	u16 cmd;
	int (*fn)(iomanX_iop_file_t *, int, void *, unsigned int, void *, unsigned int);
} DevctlCmdTbl[16] = {
	{0x5464, &esioctl2_func_1},
	{0x5465, &esioctl2_func_2},
	{0x5466, &esioctl2_func_3},
	{0x5467, &esioctl2_func_4},
	{0x5468, &esioctl2_func_5},
	{0x5469, &esioctl2_func_6},
	{0x546A, &esioctl2_func_7},
	{0x546B, &esioctl2_func_8},
	{0x546C, &esioctl2_func_9},
	{0x546D, &esioctl2_func_a},
	{0x546E, &esioctl2_func_b},
	{0x546F, &esioctl2_func_c},
	{0x5470, &esioctl2_func_d},
	{0x5471, &esioctl2_func_e},
	{0x5472, &esioctl2_func_f},
	{0x5473, &esioctl2_func_10},
};

static iomanX_iop_device_ops_t DvrFuncTbl = {
	&esdrv_df_init,
	&esdrv_df_exit,
	NOT_SUPPORTED,
	NOT_SUPPORTED,
	NOT_SUPPORTED,
	NOT_SUPPORTED,
	NOT_SUPPORTED,
	NOT_SUPPORTED,
	&esdrv_df_ioctl,
	NOT_SUPPORTED,
	NOT_SUPPORTED,
	NOT_SUPPORTED,
	NOT_SUPPORTED,
	NOT_SUPPORTED,
	NOT_SUPPORTED,
	NOT_SUPPORTED,
	NOT_SUPPORTED,
	NOT_SUPPORTED,
	NOT_SUPPORTED,
	NOT_SUPPORTED,
	NOT_SUPPORTED,
	NOT_SUPPORTED,
	(void *)&esdrv_df_null_long,
	&esdrv_df_devctl,
	NOT_SUPPORTED,
	NOT_SUPPORTED,
	&esdrv_df_ioctl2,
};
static iomanX_iop_device_t ESDRV = {
	.name = "es_drv",
	.desc = "PSX ES DRIVER",
	.type = (IOP_DT_FS | IOP_DT_FSEXT),
	.ops = &DvrFuncTbl,
};
static s32 sema_id;

// Based off of DESR / PSX DVR system software version 2.11.
IRX_ID(MODNAME, 1, 1);

int _start(int argc, char *argv[], void *startaddr, ModuleInfo_t *mi)
{
	if ( argc >= 0 )
		return module_start(argc, argv, startaddr, mi);
	else
		return module_stop(argc, argv, startaddr, mi);
}

static int module_start(int argc, char *argv[], void *startaddr, ModuleInfo_t *mi)
{
	(void)argc;
	(void)argv;
	(void)startaddr;

	if ( iomanX_AddDrv(&ESDRV) != 0 )
		return MODULE_NO_RESIDENT_END;
#if 0
	return MODULE_REMOVABLE_END;
#else
	if ( mi && ((mi->newflags & 2) != 0) )
		mi->newflags |= 0x10;
	return MODULE_RESIDENT_END;
#endif
}

static int module_stop(int argc, char *argv[], void *startaddr, ModuleInfo_t *mi)
{
	(void)argc;
	(void)argv;
	(void)startaddr;
	(void)mi;

	if ( iomanX_DelDrv(ESDRV.name) != 0 )
		return MODULE_REMOVABLE_END;
	return MODULE_NO_RESIDENT_END;
}

static int esdrv_df_init(iomanX_iop_device_t *dev)
{
	int sema_id_tmp;
	iop_sema_t semaparam;

	(void)dev;

	semaparam.attr = 0;
	semaparam.initial = 1;
	semaparam.max = 1;
	semaparam.option = 0;
	sema_id_tmp = CreateSema(&semaparam);
	if ( sema_id_tmp < 0 )
		return -1;
	sema_id = sema_id_tmp;
	return 0;
}

static int esdrv_df_exit(iomanX_iop_device_t *dev)
{
	(void)dev;

	if ( DeleteSema(sema_id) != 0 )
		return -1;
	return 0;
}

static int esdrv_df_ioctl(iomanX_iop_file_t *f, int cmd, void *param)
{
	(void)f;
	(void)cmd;
	(void)param;

	WaitSema(sema_id);
	SignalSema(sema_id);
	return -EINVAL;
}

static int esdrv_df_devctl(
	iomanX_iop_file_t *f, const char *name, int cmd, void *arg, unsigned int arglen, void *buf, unsigned int buflen)
{
	int retres;
	unsigned int i;

	(void)name;

	WaitSema(sema_id);
	retres = -EINVAL;
	for ( i = 0; i < (sizeof(DevctlCmdTbl) / sizeof(DevctlCmdTbl[0])); i += 1 )
	{
		if ( DevctlCmdTbl[i].cmd == cmd )
		{
			retres = DevctlCmdTbl[i].fn(f, cmd, arg, arglen, buf, buflen);
			break;
		}
	}
	SignalSema(sema_id);
	return retres;
}

static int
esdrv_df_ioctl2(iomanX_iop_file_t *f, int cmd, void *arg, unsigned int arglen, void *buf, unsigned int buflen)
{
	(void)f;
	(void)cmd;
	(void)arg;
	(void)arglen;
	(void)buf;
	(void)buflen;

	WaitSema(sema_id);
	SignalSema(sema_id);
	return -EINVAL;
}

static s64 esdrv_df_null_long()
{
	return -48;
}

static void EsAcsSetAaryptorIoMode(void)
{
	es_regs->r_es0C = 0;
}

#if 0
static u8 EsAcsGetAaryptorIoMode(void)
{
	return es_regs->r_es0C;
}
#endif

static void EsAcsAaryptorIoEnable(void)
{
	es_regs->r_es0C |= 1;
}

#if 0
static void EsAcsAaryptorIoDisable(void)
{
	es_regs->r_es0C &= ~1;
}
#endif

#if 0
static u8 EsAcsIsAaryptorIoEnable(void)
{
	return es_regs->r_es0C & 1;
}
#endif

static void EsAcsSetBbryptorIoMode(void)
{
	es_regs->r_es0E = 0x58;
}

#if 0
static u8 EsAcsGetBbryptorIoMode(void)
{
	return es_regs->r_es0E;
}
#endif

static void EsAcsBbryptorIoEnable(void)
{
	es_regs->r_es0E |= 1;
}

#if 0
static u8 EsAcsIsBbryptorIoEnable(void)
{
	return es_regs->r_es0E & 1;
}
#endif

#if 0
static void EsAcsBbryptorIoDisable(void)
{
	es_regs->r_es0E &= ~1;
}
#endif

#if 0
static u8 EsAcsGetProcessCall(void)
{
	return es_regs->r_es28;
}
#endif

static void EsAcsAaStreamOn(void)
{
	es_regs->r_es28 = 2;
}

#if 0
static u8 EsAcsIsAaStreamOn(void)
{
	return es_regs->r_es28 & 2;
}
#endif

static void EsAcsAaTreminate(void)
{
	es_regs->r_es2C = 2;
}

#if 0
static u8 EsAcsIsAaEmpty(void)
{
	return es_regs->r_es1C & 2;
}
#endif

static u8 EsAcsGetStatusAaEmpty(void)
{
	return es_regs->r_es1C & 2;
}

static u8 EsAcsIsAaComplete(void)
{
	return es_regs->r_es1A & 2;
}

static void EsAcsSetAaCompleteIntrClear(void)
{
	es_regs->r_es1A &= 2;
}

static u8 EsAcsIsAaPckError(void)
{
	return es_regs->r_es18 & 2;
}

static void EsAcsSetAaPckErrorIntrClear(void)
{
	es_regs->r_es18 &= 2;
}

static void EsAcsBbStreamOn(void)
{
	es_regs->r_es28 = 1;
}

#if 0
static u8 EsAcsIsBbStreamOn(void)
{
	return es_regs->r_es28 & 1;
}
#endif

static void EsAcsBbTreminate(void)
{
	es_regs->r_es2C = 1;
}

static u8 EsAcsIsBbComplete(void)
{
	return es_regs->r_es1A & 1;
}

static void EsAcsSetBbCompleteIntrClear(void)
{
	es_regs->r_es1A &= 1;
}

static u8 EsAcsIsBbPckError(void)
{
	return es_regs->r_es18 & 1;
}

static void EsAcsSetBbPckErrorIntrClear(void)
{
	es_regs->r_es18 &= 1;
}

#if 0
static u8 EsAcsIsBbEmpty(void)
{
	return es_regs->r_es1C & 1;
}
#endif

#if 0
static void EsAcsBbThroughEnable(void)
{
	es_regs->r_es08 |= 8;
}
#endif

static void EsAcsBbThroughDisable(void)
{
	es_regs->r_es08 &= ~8;
}

static u8 EsAcsIsBbBlockThrough(void)
{
	return es_regs->r_es08 & 8;
}

#if 0
static u8 EsAcsGetStatusBbPesScrambleControl(void)
{
	u8 r_es22;

	r_es22 = es_regs->r_es22;
	es_regs->r_es22 = r_es22 & 0xC;
	return (u8)(r_es22 & 0xC) >> 2;
}
#endif

static u8 EsAcsGetStatusBbEmpty(void)
{
	return es_regs->r_es1C & 1;
}

#if 0
static int EsAcsGetSerialNumber(void)
{
	return es_regs->r_es3E | (es_regs->r_es38 << 24) | (es_regs->r_es3A << 16) | (es_regs->r_es3C << 8);
}
#endif

#if 0
static void EsAcsSetSerialNumber(unsigned int a1)
{
	es_regs->r_es38 = (a1 >> 24) & 0xFF;
	es_regs->r_es3A = (a1 >> 16) & 0xFF;
	es_regs->r_es3C = (a1 >> 8) & 0xFF;
	es_regs->r_es3E = a1 & 0xFF;
}
#endif

static void EsAcsSetSerialNumberReg1(u8 a1)
{
	es_regs->r_es38 = a1;
}

static void EsAcsSetSerialNumberReg2(u8 a1)
{
	es_regs->r_es3A = a1;
}

static void EsAcsSetSerialNumberReg3(u8 a1)
{
	es_regs->r_es3C = a1;
}

static void EsAcsSetSerialNumberReg4(u8 a1)
{
	es_regs->r_es3E = a1;
}

#if 0
static u8 EsAcsGetStatusA(void)
{
	return es_regs->r_es20;
}
#endif

#if 0
static u8 EsAcsIsBbStatusInit(void)
{
	return (es_regs->r_es20 & 0xF) == 0;
}
#endif

static u8 EsAcsIsBbStatusIdle(void)
{
	return (es_regs->r_es20 & 0xF) == 1;
}

#if 0
static u8 EsAcsIsBbStatusNn(void)
{
	return (es_regs->r_es20 & 0xF) == 2;
}
#endif

static u8 EsAcsIsBbStatusUUTT(void)
{
	return (es_regs->r_es20 & 0xF) == 3;
}

#if 0
static u8 EsAcsIsBbStatusDec(void)
{
	return (es_regs->r_es20 & 0xF) == 4;
}
#endif

#if 0
static u8 EsAcsIsBbStatusRandom(void)
{
	return (es_regs->r_es20 & 0xF) == 6;
}
#endif

static void EsAcsSetSdeData(u8 a1)
{
	es_regs->r_es34 = a1;
}

static void EsAcsAllIntDisableA(void)
{
	es_regs->r_es10 = 0;
}

static void EsAcsAllIntDisableB(void)
{
	es_regs->r_es12 = 0;
}

static void EsAcsAllIntDisableC(void)
{
	es_regs->r_es14 = 0;
}

static void EsAcsAllIntDisable(void)
{
	EsAcsAllIntDisableA();
	EsAcsAllIntDisableB();
	EsAcsAllIntDisableC();
}

static void EsAcsAllIntStatClearA(void)
{
	es_regs->r_es18 = 0x9F;
}

static void EsAcsAllIntStatClearB(void)
{
	es_regs->r_es1A = 0xDF;
}

static void EsAcsAllIntStatClearC(void)
{
	es_regs->r_es1C = 0x83;
}

static void EsAcsAllIntStatClear(void)
{
	EsAcsAllIntStatClearA();
	EsAcsAllIntStatClearB();
	EsAcsAllIntStatClearC();
}

#if 0
static void EsAcsIntEnableInitComplete(void)
{
	es_regs->r_es12 |= 0x40;
}
#endif

#if 0
static void EsAcsIntDisableInitComplete(void)
{
	es_regs->r_es12 &= ~0x40;
}
#endif

#if 0
static void EsAcsIntEnableProcessRandom(void)
{
	es_regs->r_es14 |= 0x80;
}
#endif

#if 0
static void EsAcsIntDisableProcessRandom(void)
{
	es_regs->r_es14 &= ~0x80;
}
#endif

#if 0
static void EsAcsIntEnableNnInvalid(void)
{
	es_regs->r_es10 |= 0x10;
}
#endif

#if 0
static void EsAcsIntDisableNnInvalid(void)
{
	es_regs->r_es10 &= ~0x10;
}
#endif

#if 0
static void EsAcsIntEnableNnRevoked(void)
{
	es_regs->r_es10 |= 8;
}
#endif

#if 0
static void EsAcsIntDisableNnRevoked(void)
{
	es_regs->r_es10 &= ~8;
}
#endif

#if 0
static void EsAcsIntEnableNnReplaced(void)
{
	es_regs->r_es10 |= 4;
}
#endif

#if 0
static void EsAcsIntDisableNnReplaced(void)
{
	es_regs->r_es10 &= ~4;
}
#endif

#if 0
static void EsAcsIntEnableNnComplete(void)
{
	es_regs->r_es12 |= 0x10;
}
#endif

#if 0
static void EsAcsIntDisableNnComplete(void)
{
	es_regs->r_es12 &= ~0x10;
}
#endif

#if 0
static void EsAcsIntEnableProcessNn(void)
{
	EsAcsIntEnableNnInvalid();
	EsAcsIntEnableNnRevoked();
	EsAcsIntEnableNnReplaced();
	EsAcsIntEnableNnComplete();
}
#endif

#if 0
static void EsAcsIntDisableProcessNn(void)
{
	EsAcsIntDisableNnInvalid();
	EsAcsIntDisableNnRevoked();
	EsAcsIntDisableNnReplaced();
	EsAcsIntDisableNnComplete();
}
#endif

#if 0
static void EsAcsIntEnableUUComplete(void)
{
	es_regs->r_es12 |= 8;
}
#endif

#if 0
static void EsAcsIntDisableUUComplete(void)
{
	es_regs->r_es12 &= ~8;
}
#endif

#if 0
static void EsAcsIntEnableProcessUU(void)
{
	EsAcsIntEnableUUComplete();
}
#endif

#if 0
static void EsAcsIntDisableProcessUU(void)
{
	EsAcsIntDisableUUComplete();
}
#endif

#if 0
static void EsAcsIntEnableTTComplete(void)
{
	es_regs->r_es12 |= 4;
}
#endif

#if 0
static void EsAcsIntDisableTTComplete(void)
{
	es_regs->r_es12 &= ~4;
}
#endif

#if 0
static void EsAcsIntEnableProcessTT(void)
{
	EsAcsIntEnableTTComplete();
}
#endif

#if 0
static void EsAcsIntDisableProcessTT(void)
{
	EsAcsIntDisableTTComplete();
}
#endif

#if 0
static void EsAcsIntEnableBbEmpty(void)
{
	es_regs->r_es14 |= 1;
}
#endif

#if 0
static void EsAcsIntDisableBbEmpty(void)
{
	es_regs->r_es14 &= ~1;
}
#endif

#if 0
static void EsAcsIntEnableBbPackErr(void)
{
	es_regs->r_es10 |= 1;
}
#endif

#if 0
static void EsAcsIntDisableBbPackErr(void)
{
	es_regs->r_es10 &= ~1;
}
#endif

#if 0
static void EsAcsIntEnableAaPackErr(void)
{
	es_regs->r_es10 |= 2;
}
#endif

#if 0
static void EsAcsIntDisableAaPackErr(void)
{
	es_regs->r_es10 &= ~2;
}
#endif

static void EsAcsSoftReset(void)
{
	es_regs->r_es00 = 0x80;
}

static void EsAcsInitTerminate(void)
{
	es_regs->r_es2C = 0x40;
}

static void EsAcsSetInitComplete(void)
{
	es_regs->r_es1A &= 0x40;
}

static void EsAcsRandSeedSn(void)
{
	es_regs->r_es08 |= 0x10;
}

static u8 EsAcsIsAccessProhibit(void)
{
	return es_regs->r_es22 & 0x80;
}

static u8 EsAcsIsInidecError(void)
{
	return es_regs->r_es18 & 0x80;
}

static void EsAcsSetInidecErrorIntrClear(void)
{
	es_regs->r_es18 &= 0x80;
}

static u8 EsAcsIsLrsComplete(void)
{
	return es_regs->r_es1A & 0x80;
}

static void EsAcsSetLrsComplete(void)
{
	es_regs->r_es1A &= 0x80;
}

static u8 EsAcsIsInitComplete(void)
{
	return es_regs->r_es1A & 0x40;
}

static void EsAcsStartRandom(void)
{
	es_regs->r_es2A = 0x80;
}

static u8 EsAcsIsRandomComplete(void)
{
	return es_regs->r_es1C & 0x80;
}

static void EsAcsSetRandomComplete(void)
{
	es_regs->r_es1C &= 0x80;
}

static void EsAcsStartNn(void)
{
	es_regs->r_es28 = 0x10;
}

static void EsAcsTerminateNn(void)
{
	es_regs->r_es2C = 0x10;
}

static u8 EsAcsIsNnInvalid(void)
{
	return es_regs->r_es18 & 0x10;
}

static u8 EsAcsIsNnRevoked(void)
{
	return es_regs->r_es18 & 8;
}

static u8 EsAcsIsNnReplaced(void)
{
	return es_regs->r_es18 & 4;
}

static u8 EsAcsIsNnComplete(void)
{
	return es_regs->r_es1A & 0x10;
}

static void EsAcsSetNnInvalid(void)
{
	es_regs->r_es18 &= 0x10;
}

static void EsAcsSetNnRevoked(void)
{
	es_regs->r_es18 &= 8;
}

static void EsAcsSetNnReplaced(void)
{
	es_regs->r_es18 &= 4;
}

static void EsAcsSetNnComplete(void)
{
	es_regs->r_es1A &= 0x10;
}

static void EsAcsSetNnIntrAllClear(void)
{
	EsAcsSetNnInvalid();
	EsAcsSetNnRevoked();
	EsAcsSetNnReplaced();
	EsAcsSetNnComplete();
}

static void EsAcsSetNnPacksUsed(u8 a1)
{
	es_regs->r_es36 = a1;
}

static void EsAcsSetNnValidationData(const u8 *a1)
{
	es_regs->r_es40 = *a1;
	es_regs->r_es42 = a1[1];
	es_regs->r_es44 = a1[2];
	es_regs->r_es46 = a1[3];
	es_regs->r_es48 = a1[4];
	es_regs->r_es4A = a1[5];
	es_regs->r_es4C = a1[6];
	es_regs->r_es4E = a1[7];
}

static void EsAcsStartUU(void)
{
	es_regs->r_es28 = 8;
}

static void EsAcsStartTT(void)
{
	es_regs->r_es28 = 4;
}

static u8 EsAcsIsUUComplete(void)
{
	return es_regs->r_es1A & 8;
}

static u8 EsAcsIsTTComplete(void)
{
	return es_regs->r_es1A & 4;
}

static void EsAcsSetUUComplete(void)
{
	es_regs->r_es1A = 8;
}

static void EsAcsSetTTComplete(void)
{
	es_regs->r_es1A = 4;
}

static void EsAcsSetBbryptedTitleKey(void)
{
	es_regs->r_es08 |= 4;
}

static void EsAcsSetRandomTitleKey(void)
{
	es_regs->r_es08 &= ~4;
}

static void EsAcsSetMediaId(const u8 *a1)
{
	es_regs->r_es50 = a1[0];
	es_regs->r_es52 = a1[1];
	es_regs->r_es54 = a1[2];
	es_regs->r_es56 = a1[3];
	es_regs->r_es58 = a1[4];
	es_regs->r_es5A = a1[5];
	es_regs->r_es5C = a1[6];
	es_regs->r_es5E = a1[7];
}

static void EsAcsSetTTe(const u8 *a1)
{
	es_regs->r_es60 = a1[0];
	es_regs->r_es62 = a1[1];
	es_regs->r_es64 = a1[2];
	es_regs->r_es66 = a1[3];
	es_regs->r_es68 = a1[4];
	es_regs->r_es6A = a1[5];
	es_regs->r_es6C = a1[6];
	es_regs->r_es6E = a1[7];
}

static void EsAcsGetTTe(u8 *a1)
{
	a1[0] = (u8)(es_regs->r_es70);
	a1[1] = (u8)(es_regs->r_es72);
	a1[2] = (u8)(es_regs->r_es74);
	a1[3] = (u8)(es_regs->r_es76);
	a1[4] = (u8)(es_regs->r_es78);
	a1[5] = (u8)(es_regs->r_es7A);
	a1[6] = (u8)(es_regs->r_es7C);
	a1[7] = (u8)(es_regs->r_es7E);
}

#if 0
static u8 EsAcsGetIntrSrcA(void)
{
	return es_regs->r_es18;
}
#endif

#if 0
static u8 EsAcsGetIntrSrcB(void)
{
	return es_regs->r_es1A;
}
#endif

#if 0
static u8 EsAcsGetIntrSrcC(void)
{
	return es_regs->r_es1C;
}
#endif

static u8 EsAcsGetChipVersion(void)
{
	return es_regs->r_es02;
}

static u8 EsAcsGetRevision1(void)
{
	return es_regs->r_es04;
}

static u8 EsAcsGetRevision2(void)
{
	return es_regs->r_es06;
}

#if 0
static u16 EsAcsGetReg(int a1)
{
	return *(u16 *)(&es_regs->r_es00 + a1);
}
#endif

#if 0
static void EsAcsSetReg(u8 a1, u8 a2)
{
	*(u16 *)(&es_regs->r_es00 + a1) = a2;
}
#endif

static u8 EsDrvNnResultCheck(void)
{
	if ( EsAcsIsNnInvalid() != 0 )
	{
		return 4;
	}
	if ( EsAcsIsNnRevoked() != 0 )
	{
		return 3;
	}
	if ( EsAcsIsNnReplaced() != 0 )
	{
		return 2;
	}
	if ( EsAcsIsNnComplete() != 0 )
	{
		return 1;
	}
	return 0;
}

static void EsDrvSoftReset(void)
{
	EsAcsSoftReset();
	DelayThread(1000);
}

static void EsDrvSetSerialNumber(u8 *a1)
{
	EsAcsSetSerialNumberReg1(a1[0]);
	EsAcsSetSerialNumberReg2(a1[1]);
	EsAcsSetSerialNumberReg3(a1[2]);
	EsAcsSetSerialNumberReg4(a1[3]);
}

static u8 EsDrvSetSde(u8 *a1)
{
	s16 i;

	EsAcsSetInidecErrorIntrClear();
	EsAcsSetLrsComplete();
	for ( i = 0; i < 0x1be; i += 1 )
	{
		EsAcsSetSdeData(a1[i]);
		if ( i == 0x1bd )
			break;
		if ( EsAcsIsAccessProhibit() != 0 )
			return -1;
	}
	DelayThread(5000);
	return 0;
}

static u8 EsDrvCheckSdeProcess(int a1)
{
	if ( EsAcsIsInidecError() )
	{
		EsAcsSetInidecErrorIntrClear();
		return -1;
	}
	if ( EsAcsIsLrsComplete() == 0 )
	{
		return -1;
	}
	EsAcsSetLrsComplete();
	DelayThread(a1);
	return 0;
}

static u8 EsDrvTrmntInitProcess(void)
{
	u16 i;

	EsAcsSetInitComplete();
	EsAcsInitTerminate();
	for ( i = 0; i < 0x65; i += 1 )
	{
		if ( EsAcsIsInitComplete() )
		{
			EsAcsSetInitComplete();
			return 0;
		}
		DelayThread(100000);
	}
	return -1;
}

static void EsDrvCmpltInitProcess(void)
{
	EsAcsRandSeedSn();
	EsAcsSetAaryptorIoMode();
	EsAcsSetBbryptorIoMode();
	EsAcsAllIntDisable();
	EsAcsAllIntStatClear();
	EsAcsAaryptorIoEnable();
	EsAcsBbryptorIoEnable();
}

static u8 EsDrvRndNnCheck(void)
{
	if ( EsAcsIsBbStatusIdle() == 0 )
		return -1;
	return 0;
}

static u8 EsDrvRandom(void)
{
	int i;

	for ( i = 0; i < 15; ++i )
	{
		u16 j;

		j = 0;
		EsAcsSetRandomComplete();
		EsAcsStartRandom();
		for ( j = 0; j < 6; j += 1 )
		{
			if ( EsAcsIsRandomComplete() )
			{
				return 0;
			}
			DelayThread(2);
		}
		return -1;
	}
	return 0;
}

static void EsDrvSetNnParam(u8 a1, const u8 *a2)
{
	EsAcsSetNnPacksUsed(a1);
	EsAcsSetNnValidationData(a2);
}

static void EsDrvSetNnStart(void)
{
	EsAcsSetNnIntrAllClear();
	EsAcsBbThroughDisable();
	EsAcsStartNn();
}

static int EsDrvStartNnCheckStat(void)
{
	return EsDrvNnResultCheck();
}

static void EsDrvSetNnStop(void)
{
	DelayThread(1000);
}

static int EsDrvStopNnCheckStat(void)
{
	u8 retval;

	retval = EsDrvNnResultCheck();
	if ( retval != 1 )
	{
		int i;

		EsAcsTerminateNn();
		for ( i = 0; i < 10; i += 1 )
		{
			retval = EsDrvNnResultCheck();
			if ( retval != 0 )
				break;
			DelayThread(1000);
		}
	}
	EsAcsSetNnIntrAllClear();
	return retval;
}

static u8 EsDrvSetUUStart(const u8 *a1)
{
	if ( EsAcsIsBbStatusUUTT() == 0 )
	{
		return -1;
	}
	EsAcsBbThroughDisable();
	EsAcsSetMediaId(a1);
	EsAcsSetUUComplete();
	EsAcsStartUU();
	return 0;
}

static u8 EsDrvCmpltUUProcess(void)
{
	u16 i;

	for ( i = 0; i < 0x65; i += 1 )
	{
		if ( EsAcsIsUUComplete() )
		{
			EsAcsSetUUComplete();
			return 0;
		}
		DelayThread(10000);
	}
	return -1;
}

static u8 EsDrvSetTTStart(u8 a1, const u8 *a2)
{
	if ( a1 )
	{
		EsAcsSetBbryptedTitleKey();
		EsAcsSetTTe(a2);
	}
	else
	{
		EsAcsSetRandomTitleKey();
	}
	EsAcsSetTTComplete();
	EsAcsStartTT();
	return 0;
}

static u8 EsDrvCmpltTTProcess(u8 a1, u8 *a2)
{
	u16 i;

	for ( i = 0; i < 0x65; i += 1 )
	{
		if ( EsAcsIsTTComplete() )
		{
			EsAcsSetTTComplete();
			if ( !a1 )
			{
				EsAcsGetTTe(a2);
			}
			return 0;
		}
		DelayThread(10000);
	}
	return -1;
}

static u8 EsDrvBbStart(void)
{
	if ( EsAcsIsBbBlockThrough() != 0 )
	{
		return -1;
	}
	EsAcsSetBbPckErrorIntrClear();
	EsAcsSetBbCompleteIntrClear();
	EsAcsBbStreamOn();
	return 0;
}

static int EsDrvBbBsCheck(void)
{
	if ( EsAcsIsBbPckError() != 0 )
	{
		EsAcsSetBbPckErrorIntrClear();
		return 1;
	}
	return 0;
}

static u8 EsDrvBbStop(void)
{
	u16 i;

	EsAcsBbTreminate();
	for ( i = 0; i < 0x65; i += 1 )
	{
		if ( EsAcsIsBbComplete() )
		{
			EsAcsSetBbCompleteIntrClear();
			return 0;
		}
		DelayThread(100000);
	}
	return -1;
}

static void EsDrvAaStart(void)
{
	EsAcsSetAaPckErrorIntrClear();
	EsAcsSetAaCompleteIntrClear();
	EsAcsAaStreamOn();
}

static int EsDrvAaBsCheck(void)
{
	if ( EsAcsIsAaPckError() != 0 )
	{
		EsAcsSetAaPckErrorIntrClear();
		return 1;
	}
	return 0;
}

static u8 EsDrvAaStop(void)
{
	u16 i;

	EsAcsAaTreminate();
	for ( i = 0; i < 0x65; i += 1 )
	{
		if ( EsAcsIsAaComplete() )
		{
			EsAcsSetAaCompleteIntrClear();
			return 0;
		}
		DelayThread(100000);
	}
	return -1;
}

#if 0
static void EsDrvBbFifoClear(void)
{
	if ( !EsAcsIsBbBlockThrough() )
	{
		EsAcsIntDisableBbPackErr();
		EsAcsBbTreminate();
	}
}
#endif

#if 0
static void EsDrvAaFifoClear(void)
{
	EsAcsAaTreminate();
}
#endif

#if 0
static int EsDrvGetBbPesScrambleControl(void)
{
	return EsAcsGetStatusBbPesScrambleControl();
}
#endif

static int EsDrvAaEmptyCheck(void)
{
	return EsAcsGetStatusAaEmpty() == 0;
}

static int EsDrvBbEmptyCheck(void)
{
	return EsAcsGetStatusBbEmpty() == 0;
}

static int EsDrvGetChipVersion(void)
{
	return EsAcsGetChipVersion();
}

static int EsDrvGetRevision1(void)
{
	return EsAcsGetRevision1();
}

static int EsDrvGetRevision2(void)
{
	return EsAcsGetRevision2();
}

#if 0
static int EsDrvGetReg(int a1)
{
	return EsAcsGetReg(a1);
}
#endif

#if 0
static void EsDrvSetReg(u8 a1, u8 a2)
{
	EsAcsSetReg(a1, a2);
}
#endif

static int
esioctl2_func_1(iomanX_iop_file_t *f, int cmd, void *arg, unsigned int arglen, void *buf, unsigned int buflen)
{
	(void)f;
	(void)cmd;
	(void)arg;
	(void)arglen;
	(void)buflen;

	*(u8 *)buf = EsDrvGetChipVersion();
	*((u8 *)buf + 1) = EsDrvGetRevision1();
	*((u8 *)buf + 2) = EsDrvGetRevision2();
	return 0;
}

static int
esioctl2_func_10(iomanX_iop_file_t *f, int cmd, void *arg, unsigned int arglen, void *buf, unsigned int buflen)
{
	(void)f;
	(void)cmd;
	(void)arg;
	(void)arglen;
	(void)buflen;

	*(u32 *)buf = 0x4071200;
	return 0;
}

static int
esioctl2_func_2(iomanX_iop_file_t *f, int cmd, void *arg, unsigned int arglen, void *buf, unsigned int buflen)
{
	(void)f;
	(void)cmd;
	(void)arg;
	(void)arglen;
	(void)buf;
	(void)buflen;

	return 0;
}

static int
esioctl2_func_3(iomanX_iop_file_t *f, int cmd, void *arg, unsigned int arglen, void *buf, unsigned int buflen)
{
	(void)f;
	(void)cmd;
	(void)arg;
	(void)arglen;
	(void)buf;
	(void)buflen;

	return 0;
}

static int
esioctl2_func_4(iomanX_iop_file_t *f, int cmd, void *arg, unsigned int arglen, void *buf, unsigned int buflen)
{
	(void)f;
	(void)cmd;
	(void)arglen;
	(void)buf;
	(void)buflen;

	EsDrvSoftReset();
	EsDrvSetSerialNumber((u8 *)arg);
	if ( EsDrvSetSde((u8 *)arg + 4) != 0 )
	{
		return -EPERM;
	}
	if (
		EsDrvCheckSdeProcess(((*((u8 *)arg + 452) << 16) & 0x1FFFF) | (u16)(*((u8 *)arg + 451) << 8) | *((u8 *)arg + 450))
		!= 0 )
	{
		return -ENOENT;
	}
	if ( EsDrvTrmntInitProcess() != 0 )
	{
		return -ESRCH;
	}
	EsDrvCmpltInitProcess();
	return 0;
}

static int
esioctl2_func_5(iomanX_iop_file_t *f, int cmd, void *arg, unsigned int arglen, void *buf, unsigned int buflen)
{
	(void)f;
	(void)cmd;
	(void)arg;
	(void)arglen;
	(void)buf;
	(void)buflen;

	if ( EsDrvRndNnCheck() )
		return -EINTR;
	if ( EsDrvRandom() != 0 )
		return -EIO;
	return 0;
}

static int
esioctl2_func_6(iomanX_iop_file_t *f, int cmd, void *arg, unsigned int arglen, void *buf, unsigned int buflen)
{
	(void)f;
	(void)cmd;
	(void)arglen;
	(void)buf;
	(void)buflen;

	if ( EsDrvRndNnCheck() != 0 )
	{
		return -EINTR;
	}
	if ( (unsigned int)*(u8 *)arg - 1 >= 0x10 )
	{
		return -EAGAIN;
	}
	EsDrvSetNnParam(*(u8 *)arg, (u8 *)arg + 1);
	EsDrvSetNnStart();
	return 0;
}

static int
esioctl2_func_7(iomanX_iop_file_t *f, int cmd, void *arg, unsigned int arglen, void *buf, unsigned int buflen)
{
	(void)f;
	(void)cmd;
	(void)arg;
	(void)arglen;
	(void)buflen;

	*(u8 *)buf = EsDrvStartNnCheckStat();
	return 0;
}

static int
esioctl2_func_8(iomanX_iop_file_t *f, int cmd, void *arg, unsigned int arglen, void *buf, unsigned int buflen)
{
	(void)f;
	(void)cmd;
	(void)arg;
	(void)arglen;
	(void)buflen;

	EsDrvSetNnStop();
	*(u8 *)buf = EsDrvStopNnCheckStat();
	return 0;
}

static int
esioctl2_func_9(iomanX_iop_file_t *f, int cmd, void *arg, unsigned int arglen, void *buf, unsigned int buflen)
{
	int i;

	(void)f;
	(void)cmd;
	(void)arglen;
	(void)buflen;

	if ( EsDrvSetUUStart((u8 *)arg) != 0 )
	{
		return -ENXIO;
	}
	if ( EsDrvCmpltUUProcess() != 0 )
	{
		return -E2BIG;
	}
	// cppcheck-suppress knownConditionTrueFalse
	if ( EsDrvSetTTStart(*((u8 *)arg + 8), (u8 *)arg + 9) != 0 )
	{
		return -ENOEXEC;
	}
	if ( EsDrvCmpltTTProcess(*((u8 *)arg + 8), (u8 *)arg + 9) != 0 )
	{
		return -EBADF;
	}

	for ( i = 0; i < 8; i += 1 )
	{
		((u8 *)buf)[i] = ((u8 *)arg)[i + 9];
	}
	return 0;
}

static int
esioctl2_func_a(iomanX_iop_file_t *f, int cmd, void *arg, unsigned int arglen, void *buf, unsigned int buflen)
{
	(void)f;
	(void)cmd;
	(void)arg;
	(void)arglen;
	(void)buf;
	(void)buflen;

	EsDrvAaStart();
	return 0;
}

static int
esioctl2_func_b(iomanX_iop_file_t *f, int cmd, void *arg, unsigned int arglen, void *buf, unsigned int buflen)
{
	(void)f;
	(void)cmd;
	(void)arg;
	(void)arglen;
	(void)buflen;

	*(u8 *)buf = EsDrvAaBsCheck();
	*((u8 *)buf + 1) = EsDrvAaEmptyCheck();
	return 0;
}

static int
esioctl2_func_c(iomanX_iop_file_t *f, int cmd, void *arg, unsigned int arglen, void *buf, unsigned int buflen)
{
	(void)f;
	(void)cmd;
	(void)arg;
	(void)arglen;
	(void)buf;
	(void)buflen;

	if ( EsDrvAaStop() != 0 )
		return -ECHILD;
	return 0;
}

static int
esioctl2_func_d(iomanX_iop_file_t *f, int cmd, void *arg, unsigned int arglen, void *buf, unsigned int buflen)
{
	(void)f;
	(void)cmd;
	(void)arg;
	(void)arglen;
	(void)buf;
	(void)buflen;

	EsDrvBbStart();
	return 0;
}

static int
esioctl2_func_e(iomanX_iop_file_t *f, int cmd, void *arg, unsigned int arglen, void *buf, unsigned int buflen)
{
	(void)f;
	(void)cmd;
	(void)arg;
	(void)arglen;
	(void)buflen;

	*(u8 *)buf = EsDrvBbBsCheck();
	*((u8 *)buf + 1) = EsDrvBbEmptyCheck();
	return 0;
}

static int
esioctl2_func_f(iomanX_iop_file_t *f, int cmd, void *arg, unsigned int arglen, void *buf, unsigned int buflen)
{
	(void)f;
	(void)cmd;
	(void)arg;
	(void)arglen;
	(void)buf;
	(void)buflen;

	if ( EsDrvBbStop() != 0 )
		return -ECHILD;
	return 0;
}

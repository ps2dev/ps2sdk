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
#include <kerr.h>
#include <xatapi.h>

#ifdef _IOP
IRX_ID("cdvd_xatapi_driver", 2, 3);
#endif
// Based off of DESR / PSX DVR system software version 2.11.

typedef struct _ata_cmd_state
{
	s32 type;
	union
	{
		void *buf;
		u8 *buf8;
		u16 *buf16;
	};
	u32 blkcount;
	s32 dir;
	s32 type_atapi;
	void *buf_atapi;
	u32 blkcount_atapi;
	u32 blksize_atapi;
	s32 dir_atapi;
} ata_cmd_state_t;

typedef struct _ata_devinfo
{
	s32 exists;
	s32 has_packet;
	u32 total_sectors;
	u32 security_status;
	u32 lba48;
} ata_devinfo_t;

struct dev5_speed_regs_
{
	vu16 unv00;
	vu16 r_spd_rev_1;
	vu16 r_spd_rev_3;
	vu16 unv06;
	vu16 unv08;
	vu16 unv0a;
	vu16 unv0c;
	vu16 r_spd_rev_8;
	vu16 unv10;
	vu16 unv12;
	vu16 unv14;
	vu16 unv16;
	vu16 unv18;
	vu16 unv1a;
	vu16 unv1c;
	vu16 unv1e;
	vu16 unv20;
	vu16 unv22;
	vu16 r_spd_dma_ctrl;
	vu16 unv26;
	vu16 r_spd_intr_stat;
	vu16 r_spd_intr_mask;
	vu16 unv2c;
	vu16 r_spd_pio_data;
	vu16 unv30;
	vu16 r_spd_xfr_ctrl;
	vu16 unv34;
	vu16 m_spd_unk36;
	vu16 r_spd_dbuf_stat;
	vu16 unv3a;
	vu16 unv3c;
	vu16 unv3e;
	vu16 r_spd_ata_data;
	vu16 r_spd_ata_error;
	vu16 r_spd_ata_nsector;
	vu16 r_spd_ata_sector;
	vu16 r_spd_ata_lcyl;
	vu16 r_spd_ata_hcyl;
	vu16 r_spd_ata_select;
	vu16 r_spd_ata_status;
	vu16 unv50;
	vu16 unv52;
	vu16 unv54;
	vu16 unv56;
	vu16 unv58;
	vu16 unv5a;
	vu16 r_spd_ata_control;
	vu16 unv5e;
	vu16 unv60;
	vu16 unv62;
	vu16 r_spd_if_ctrl;
	vu16 unv66;
	vu16 unv68;
	vu16 unv6a;
	vu16 unv6c;
	vu16 unv6e;
	vu16 r_spd_pio_mode;
	vu16 r_spd_mwdma_mode;
	vu16 r_spd_udma_mode;
};
// cppcheck-suppress-macro constVariablePointer
#define USE_DEV5_SPEED_REGS() struct dev5_speed_regs_ *const dev5_speed_regs = (void *)0xBF410000

struct dev5_fpga_regs_
{
	vu16 r_fpga_layer1;
	vu16 r_fpga_xfrdir;
	vu16 r_fpga_xfren;
	vu16 r_fpga_layer2;
	vu16 r_fpga_sl3bufe;
	vu16 r_fpga_sl3bufd;
	vu16 unv0c;
	vu16 unv0e;
	vu16 unv10;
	vu16 r_fpga_exbufd;
	vu16 r_fpga_exbufe;
	vu16 unv16;
	vu16 unv18;
	vu16 unv1a;
	vu16 unv1c;
	vu16 unv1e;
	vu16 unv20;
	vu16 unv22;
	vu16 unv24;
	vu16 unv26;
	vu16 unv28;
	vu16 unv2a;
	vu16 unv2c;
	vu16 unv2e;
	vu16 r_fpga_unk30;
	vu16 r_fpga_spckmode;
	vu16 r_fpga_spckcnt;
	vu16 unvpad[2013];
	vu16 r_fpga_revision;
};
// cppcheck-suppress-macro constVariablePointer
#define USE_DEV5_FPGA_REGS() struct dev5_fpga_regs_ *const dev5_fpga_regs = (void *)0xBF414000

static int xatapi_dev_devctl(
	const iop_file_t *f, const char *name, int cmd, void *args, unsigned int arglen, void *buf, unsigned int buflen);
static void speed_init(void);
static void speed_device_init(void);
static void do_hex_dump(void *ptr, int len);
static void ata_pio_mode(int mode);
static void ata_multiword_dma_mode(int mode);
static void ata_ultra_dma_mode(int mode);
static void AtaEjectIntrHandle(void);
static int sceCdAtapiExecCmd_local(s16 n, void *buf, int nsec, int secsize, void *pkt, unsigned int pkt_len, int proto);
static int sceCdAtapiExecCmd(s16 n, void *buf, int nsec, int secsize, void *pkt, int pkt_len, int proto);
static void DmaRun_spck(char *buf, unsigned int secsize);
static int sceCdAtapiWaitResult_local(void);
static void atapi_device_set_transfer_mode_outer(int device);
static void ata_device_set_transfer_mode_outer(int device);
static void sceAtapiInit(int device);
static void sceAtInterInit(void);
static int create_event_flags(void);
static void FpgaLayer1On(void);
static void FpgaLayer1Off(void);
static void FpgaLayer2Off(void);
static void FpgaXfrenOn(void);
static void FpgaXfrenOff(void);
static void FpgaSpckmodeOn(void);
static void FpgaSpckmodeOff(void);
static void FpgaXfdir(int dir);
static int FpgaGetRevision(void);
static int do_fpga_check_spckcnt(void);
static void FpgaCheckWriteBuffer(void);
static void FpgaCheckWriteBuffer2(void);
static void FpgaClearBuffer(void);
static int Mpeg2CheckPadding(char *buf, unsigned int bufsize, int *retptr, int *pesscramblingpackptr);
static int Mpeg2CheckScramble(char *buf, unsigned int bufsize);

extern struct irx_export_table _exp_xatapi;
// Unofficial: move to bss
static int g_devctl_retonly_unset;
static vu16 *const g_dev9_reg_1460 = (void *)0xBF801460;
static vu16 *const g_dev9_reg_power = (void *)0xBF80146C;
// Unofficial: move to bss
static int (*p_dev5_intr_cb)(int flag);

// cppcheck-suppress unusedFunction
IOMANX_RETURN_VALUE_IMPL(0);
// unofficial: don't print on nulldev0 call
IOMANX_RETURN_VALUE_IMPL(EIO);

static iop_device_ops_t ata_ioman_devops = {
	IOMANX_RETURN_VALUE(0),   IOMANX_RETURN_VALUE(0),   IOMANX_RETURN_VALUE(EIO),     IOMANX_RETURN_VALUE(EIO),
	IOMANX_RETURN_VALUE(EIO), IOMANX_RETURN_VALUE(EIO), IOMANX_RETURN_VALUE(EIO),     IOMANX_RETURN_VALUE(EIO),
	IOMANX_RETURN_VALUE(EIO), IOMANX_RETURN_VALUE(EIO), IOMANX_RETURN_VALUE(EIO),     IOMANX_RETURN_VALUE(EIO),
	IOMANX_RETURN_VALUE(EIO), IOMANX_RETURN_VALUE(EIO), IOMANX_RETURN_VALUE(EIO),     IOMANX_RETURN_VALUE(EIO),
	IOMANX_RETURN_VALUE(EIO), IOMANX_RETURN_VALUE(EIO), IOMANX_RETURN_VALUE(EIO),     IOMANX_RETURN_VALUE(EIO),
	IOMANX_RETURN_VALUE(EIO), IOMANX_RETURN_VALUE(EIO), IOMANX_RETURN_VALUE_S64(EIO), (void *)&xatapi_dev_devctl,
	IOMANX_RETURN_VALUE(EIO), IOMANX_RETURN_VALUE(EIO), IOMANX_RETURN_VALUE(EIO),
};
static iop_device_t ata_ioman_device = {"xatapi", IOP_DT_FS | IOP_DT_FSEXT, 1, "CD-ROM_ATAPI", &ata_ioman_devops};
// Unofficial: move to bss
static int g_reset_scrambling_pack;
// Unofficial: move to bss
static int g_pes_scrambling_control_pack;
// Unofficial: move to bss
static int g_dma_mode_value;
// Unofficial: move to bss
static int g_dma_speed_value;
// Unofficial: move to bss
static int g_should_wait_for_dma_flag;
// Unofficial: move to bss
static int g_is_wait_busy;
// Unofficial: move to bss
static int g_verbose_level;
// Unofficial: move to bss
static int g_ata_devinfo_init;
static int g_bf40200a_is_set_ptr;
static u32 *g_cd_sc_ffffffd9_ptr;
static int g_is_in_read_info;
static int g_io_event_flag;
static int g_adma_evfid;
static int g_acmd_evfid;
static int g_dma_lock_sema;
static void (*g_dev5_intr_cbs[16])(int flag);
static void (*g_dev5_predma_cbs[4])(u32 bcr_in, int dir);
static void (*g_dev5_postdma_cbs[4])(u32 bcr_in, int dir);
static int g_atapi_event_flag;
static ata_devinfo_t atad_devinfo[2];
static ata_cmd_state_t atad_cmd_state;
static int ata_param[128];
static int g_atapi_xfer_buf[130];

static int do_atapi_cmd_inquiry_12h(s16 dev_nr)
{
	int i;
	int retres;
	char pkt[12];
	char outbuf[56];

	retres = 1;
	for ( i = 0; retres && i < 16; i += 1 )
	{
		memset(pkt, 0, sizeof(pkt));
		pkt[0] = 0x12;
		pkt[4] = sizeof(outbuf);
		retres = sceCdAtapiExecCmd_local(dev_nr, outbuf, 1, sizeof(outbuf), pkt, sizeof(pkt), 2);
		if ( retres )
			continue;
		retres = sceCdAtapiWaitResult_local();
		if ( retres )
		{
			VERBOSE_KPRINTF(1, "Atapi Drive ATAPI_CMD_READ_EXT_INFO  NG\n");
			DelayThread(10000);
		}
	}
	if ( retres )
		return retres;
	do_hex_dump(outbuf, sizeof(outbuf));
	return strncmp(&outbuf[32], "BOOT", 4) ? 1 : 0;
}

static int do_atapi_request_test_unit_ready(s16 dev_nr, int *errptr, int *ctrlptr)
{
	int retres;
	char pkt[12];

	*errptr = 0;
	*ctrlptr = 0;
	memset(pkt, 0, sizeof(pkt));
	pkt[0] = 0;
	retres = xatapi_7_sceCdAtapiExecCmd(dev_nr, 0, 0, 0, pkt, sizeof(pkt), 1);
	if ( retres )
		return retres;
	retres = xatapi_8_sceCdAtapiWaitResult();
	*errptr = (u8)xatapi_11_sceAtaGetError();
	*ctrlptr = (u8)xatapi_12_get_ata_control();
	return retres;
}

static int atapi_req_sense_get(s16 dev_nr, int *retptr)
{
	int result;
	char pkt[12];
	char outbuf[18];

	*retptr = 0;
	memset(pkt, 0, sizeof(pkt));
	pkt[0] = 3;
	pkt[4] = sizeof(outbuf);
	result = xatapi_7_sceCdAtapiExecCmd(dev_nr, outbuf, 1, sizeof(outbuf), pkt, sizeof(pkt), 2);
	if ( result )
		return result;
	result = xatapi_8_sceCdAtapiWaitResult();
	if ( result )
		return result;
	*retptr = ((outbuf[2] & 0xF) << 16) | ((u8)outbuf[12] << 8) | (u8)outbuf[13];
	return 0;
}

#ifdef UNUSED_FUNC
static int atapi_exec_cmd_request_sense_03h_unused(s16 dev_nr, int *retptr)
{
	int result;
	char pkt[12];
	char outbuf[18];

	*retptr = 0;
	memset(pkt, 0, sizeof(pkt));
	pkt[0] = 3;
	pkt[4] = sizeof(outbuf);
	result = sceCdAtapiExecCmd_local(dev_nr, outbuf, 1, sizeof(outbuf), pkt, sizeof(pkt), 2);
	if ( result )
		return result;
	result = sceCdAtapiWaitResult_local();
	if ( result )
		return result;
	*retptr = ((outbuf[2] & 0xF) << 16) | ((u8)outbuf[12] << 8) | (u8)outbuf[13];
	return 0;
}
#endif

#ifdef UNUSED_FUNC
static int do_start_stop_unit_1bh_unused(void)
{
	int i;
	int retres;
	char pkt[12];

	retres = 1;
	for ( i = 0; i < 16 && retres && retres != -550; i += 1 )
	{
		memset(pkt, 0, sizeof(pkt));
		pkt[0] = 0x1B;
		pkt[4] = 2;
		pkt[5] = 0;
		retres = sceCdAtapiExecCmd_local(0, 0, 0, 0, pkt, sizeof(pkt), 1);
		if ( retres )
			continue;
		retres = sceCdAtapiWaitResult_local();
		if ( retres )
		{
			VERBOSE_KPRINTF(1, "Atapi Drive EJECT NG\n");
			DelayThread(10000);
			continue;
		}
		VERBOSE_KPRINTF(1, "Atapi Drive EJECT OK\n");
	}
	if ( !retres )
	{
		retres = 1;
		for ( i = 0; i < 16 && retres && retres != -550; i += 1 )
		{
			memset(pkt, 0, sizeof(pkt));
			pkt[0] = 0x1B;
			pkt[4] = 3;
			pkt[5] = 0;
			retres = sceCdAtapiExecCmd_local(0, 0, 0, 0, pkt, sizeof(pkt), 1);
			if ( retres )
				continue;
			retres = sceCdAtapiWaitResult_local();
			if ( retres )
			{
				VERBOSE_KPRINTF(1, "Atapi Drive Spindle  Start  NG %d\n", i);
				DelayThread(10000);
				continue;
			}
			VERBOSE_KPRINTF(1, "Atapi Drive Spindle  Start  OK\n");
		}
	}
	VERBOSE_KPRINTF(1, "  PS2 Eject On Atapi Unit Dummy Eject Ret %d\n", retres);
	return retres;
}
#endif

static int chgsys_callback_cb(int *mediaptr, int want_atapi)
{
	int i;
	int tryres1;
	int retres;
	char pkt[12];
	char outbuf1[6];

	*mediaptr = 4;
	if ( want_atapi )
	{
		VERBOSE_KPRINTF(1, "Ps2 Drive Spindle -> Atapi \n");
		retres = 1;
		for ( i = 0; i < 10 && retres && retres != -550; i += 1 )
		{
			memset(pkt, 0, sizeof(pkt));
			pkt[0] = 0x1B;
			pkt[4] = 3;
			pkt[5] = 0;
			retres = sceCdAtapiExecCmd(0, 0, 0, 0, pkt, sizeof(pkt), 1);
			if ( retres )
				continue;
			retres = sceCdAtapiWaitResult_local();
			if ( retres )
			{
				VERBOSE_KPRINTF(1, "Atapi Drive Spindle Start NG %d\n", i);
				DelayThread(10000);
				continue;
			}
			VERBOSE_KPRINTF(1, "Atapi Drive Spindle Start OK\n");
		}
		return retres;
	}
	VERBOSE_KPRINTF(1, "Atapi Drive Spindle -> Ps2\n");
	tryres1 = 1;
	for ( i = 0; i < 16 && tryres1 && tryres1 != -550; i += 1 )
	{
		memset(pkt, 0, sizeof(pkt));
		pkt[0] = 0xF6;
		pkt[2] = 0xA0;
		pkt[8] = 6;
		tryres1 = sceCdAtapiExecCmd_local(0, outbuf1, 1, sizeof(outbuf1), pkt, sizeof(pkt), 2);
		if ( tryres1 )
			continue;
		tryres1 = sceCdAtapiWaitResult_local();
		if ( tryres1 )
		{
			VERBOSE_KPRINTF(1, "Atapi Drive ATAPI_CMD_READ_EXT_INFO  NG\n");
			DelayThread(10000);
		}
	}
	if ( !tryres1 )
	{
		int maskchk;

		maskchk = (((u8)outbuf1[5] >> 1) ^ 1) & 1;
		if ( (outbuf1[3] & 0x10) )
		{
			char outbuf[8];

			VERBOSE_KPRINTF(1, "Atapi Drive Media found.\n");
			memset(pkt, 0, sizeof(pkt));
			pkt[0] = 0xF6;
			pkt[2] = 0xA2;
			pkt[8] = sizeof(outbuf);
			if (
				!sceCdAtapiExecCmd_local(0, outbuf, 1, sizeof(outbuf), pkt, sizeof(pkt), 2) && !sceCdAtapiWaitResult_local() )
			{
				VERBOSE_KPRINTF(1, "Atapi Drive Media DVD:%d.\n", outbuf[5] & 0x20);
				*mediaptr = (outbuf[5] & 0x20) ? 3 : 2;
			}
		}
		else
		{
			VERBOSE_KPRINTF(1, "Atapi Drive No Media or Now checking.\n");
			VERBOSE_KPRINTF(1, "Atapi Drive No Media or No SpinUp.\n");
			*mediaptr = 4;
		}
		if ( maskchk )
		{
			VERBOSE_KPRINTF(1, "Atapi Drive Spin Up.\n");
			retres = 1;
			for ( i = 0; i < 16 && retres && retres != -550; i += 1 )
			{
				memset(pkt, 0, sizeof(pkt));
				pkt[0] = 0x1B;
				pkt[5] = 0;
				retres = sceCdAtapiExecCmd(0, 0, 0, 0, pkt, sizeof(pkt), 1);
				if ( retres )
					continue;
				retres = sceCdAtapiWaitResult_local();
				if ( retres )
				{
					VERBOSE_KPRINTF(1, "Atapi Drive Spindle  Stop  NG\n");
					DelayThread(10000);
					continue;
				}
				VERBOSE_KPRINTF(1, "Atapi Drive Spindle  Stop  OK\n");
			}
			return retres;
		}
		VERBOSE_KPRINTF(1, "Atapi Drive Not Spin Up.\n");
		retres = 1;
		for ( i = 0; i < 16 && retres && retres != -550; i += 1 )
		{
			memset(pkt, 0, sizeof(pkt));
			pkt[0] = 0x1B;
			pkt[4] = 2;
			pkt[5] = 0;
			retres = sceCdAtapiExecCmd(0, 0, 0, 0, pkt, sizeof(pkt), 1);
			if ( retres )
				continue;
			retres = sceCdAtapiWaitResult_local();
			if ( retres )
			{
				VERBOSE_KPRINTF(1, "Atapi Drive EJECT NG\n");
				DelayThread(10000);
				continue;
			}
			VERBOSE_KPRINTF(1, "Atapi Drive EJECT OK\n");
			*mediaptr = 256;
		}
		return retres;
	}
	return tryres1;
}

static int sceCdAtapi_SC(void)
{
	int i;
	int retres;
	char pkt[12];

	memset(pkt, 0, sizeof(pkt));
	retres = 1;
	pkt[0] = 0xF9;
	pkt[2] = 0xB1;
	pkt[8] = 0;
	for ( i = 0; i < 10 && retres; i += 1 )
	{
		retres = sceCdAtapiExecCmd_local(0, 0, 0, 0, pkt, sizeof(pkt), 1);
		if ( retres || (retres = sceCdAtapiWaitResult_local()) )
			DelayThread(10000);
	}
	if ( retres )
	{
		VERBOSE_KPRINTF(1, "sceCdAtapi SC fail\n");
		return retres;
	}
	*g_cd_sc_ffffffd9_ptr = 0;
	VERBOSE_KPRINTF(1, "sceCdAtapi SC OK\n");
	return 0;
}

int xatapi_15_exec_f6_f9_scsi(void)
{
	int i;
	int retres1;
	int retres2;
	char pkt[12];
	char outbuf[6];

	memset(pkt, 0, sizeof(pkt));
	retres1 = 1;
	pkt[0] = 0xF6;
	pkt[2] = 0xB1;
	pkt[8] = 6;
	for ( i = 0; i < 4 && retres1; i += 1 )
	{
		retres1 = sceCdAtapiExecCmd_local(0, outbuf, 1, sizeof(outbuf), pkt, sizeof(pkt), 2);
		if ( retres1 || (retres1 = sceCdAtapiWaitResult_local()) )
			DelayThread(10000);
	}
	if ( retres1 && !(outbuf[4] & 0x80) )
		return retres1;
	memset(pkt, 0, sizeof(pkt));
	retres2 = 1;
	pkt[0] = 0xF9;
	pkt[2] = 0xB2;
	pkt[8] = 0;
	for ( i = 0; i < 10 && retres2; i += 1 )
	{
		retres2 = sceCdAtapiExecCmd_local(0, 0, 0, 0, pkt, sizeof(pkt), 1);
		if ( !retres2 )
		{
			retres2 = sceCdAtapiWaitResult_local();
			if ( !retres2 )
				continue;
		}
		DelayThread(10000);
	}
	return retres2;
}

static int sceCdAtapi_BC(void)
{
	int i;
	int retres1;
	int retres4;
	char pkt[12];
	iop_event_info_t efinfo;
	u32 waresontmp;
	u32 efbits;
	char outbuf1[6];
	{
		memset(pkt, 0, sizeof(pkt));
		retres1 = 1;
		pkt[0] = 0xF6;
		pkt[2] = 0xB1;
		pkt[8] = sizeof(outbuf1);
		for ( i = 0; i < 10 && retres1; i += 1 )
		{
			retres1 = sceCdAtapiExecCmd_local(0, outbuf1, 1, sizeof(outbuf1), pkt, sizeof(pkt), 2);
			if ( retres1 || (retres1 = sceCdAtapiWaitResult_local()) )
				DelayThread(10000);
		}
	}
	if ( !retres1 || (outbuf1[4] & 0x80) )
	{
		int retres2;

		memset(pkt, 0, sizeof(pkt));
		retres2 = 1;
		pkt[0] = 0xF9;
		pkt[2] = 0xB2;
		pkt[8] = 0;
		for ( i = 0; i < 10 && retres2; i += 1 )
		{
			retres2 = sceCdAtapiExecCmd_local(0, 0, 0, 0, pkt, sizeof(pkt), 1);
			if ( !retres2 )
			{
				retres2 = sceCdAtapiWaitResult_local();
				if ( !retres2 )
					continue;
			}
			DelayThread(10000);
		}
	}
	{
		int retres3;
		char outbuf[6];

		memset(pkt, 0, sizeof(pkt));
		retres3 = 1;
		pkt[0] = 0xF6;
		pkt[2] = 0xB1;
		pkt[8] = sizeof(outbuf);
		for ( i = 0; i < 10 && retres3; i += 1 )
		{
			retres3 = sceCdAtapiExecCmd_local(0, outbuf, 1, sizeof(outbuf), pkt, sizeof(pkt), 2);
			if ( retres3 || (retres3 = sceCdAtapiWaitResult_local()) )
				DelayThread(10000);
		}
		if ( !retres3 && (outbuf[4] & 0x81) )
		{
			*g_cd_sc_ffffffd9_ptr = 1;
			return 1;
		}
	}
	{
		char outbuf[16];

		memset(pkt, 0, sizeof(pkt));
		retres4 = 1;
		pkt[0] = 0xF6;
		pkt[2] = 0xB0;
		pkt[8] = sizeof(outbuf);
		for ( i = 0; i < 10 && retres4; i += 1 )
		{
			retres4 = sceCdAtapiExecCmd_local(0, outbuf, 1, sizeof(outbuf), pkt, sizeof(pkt), 2);
			if ( retres4 || (retres4 = sceCdAtapiWaitResult_local()) )
				DelayThread(10000);
		}
		if ( retres4 )
		{
			VERBOSE_KPRINTF(1, "sceCdAtapi BC 0 fail\n");
		}
		else
		{
			int flg;

			if ( g_should_wait_for_dma_flag && !ReferEventFlagStatus(g_adma_evfid, &efinfo) && !efinfo.currBits )
				SetEventFlag(g_adma_evfid, 1);
			SetEventFlag(g_acmd_evfid, 1);
			retres4 = 0;
			for ( i = 0; i < 100 && !retres4; i += 1 )
			{
				retres4 = cdvdman_167_atapi2dragon((u8 *)outbuf, &waresontmp);
				if ( !retres4 || waresontmp )
					DelayThread(10000);
			}
			flg = 0;
			if ( !retres4 )
			{
				VERBOSE_KPRINTF(0, "sceCdAtapi BC 1 fail\n");
			}
			else
			{
				DelayThread(10000);
				retres4 = 0;
				for ( i = 0; i < 100 && !retres4; i += 1 )
				{
					retres4 = cdvdman_169_dragon2atapi((u8 *)outbuf, &waresontmp);
					if ( !retres4 || waresontmp )
						DelayThread(10000);
				}
				if ( !retres4 )
				{
					VERBOSE_KPRINTF(0, "sceCdAtapi BC 2 fail\n");
				}
				else
				{
					if ( g_should_wait_for_dma_flag )
						WaitEventFlag(g_adma_evfid, 1, WEF_AND | WEF_CLEAR, &efbits);
					retres4 = 1;
					WaitEventFlag(g_acmd_evfid, 1, WEF_AND | WEF_CLEAR, &efbits);
					memset(pkt, 0, sizeof(pkt));
					pkt[0] = 0xF9;
					pkt[2] = 0xB0;
					pkt[8] = sizeof(outbuf);
					for ( i = 0; i < 10 && retres4; i += 1 )
					{
						retres4 = sceCdAtapiExecCmd_local(0, outbuf, 1, sizeof(outbuf), pkt, sizeof(pkt), 3);
						if ( retres4 || (retres4 = sceCdAtapiWaitResult_local()) )
							DelayThread(10000);
					}
					if ( retres4 )
					{
						VERBOSE_KPRINTF(1, "sceCdAtapi BC 3 fail\n");
					}
					else
					{
						VERBOSE_KPRINTF(1, "sceCdAtapi BC OK\n");
					}
					flg = 1;
				}
			}
			if ( !flg )
			{
				if ( g_should_wait_for_dma_flag )
					WaitEventFlag(g_adma_evfid, 1, WEF_AND | WEF_CLEAR, &efbits);
				WaitEventFlag(g_acmd_evfid, 1, WEF_AND | WEF_CLEAR, &efbits);
			}
		}
	}
	{
		char outbuf[6];
		int retres5;

		memset(pkt, 0, sizeof(pkt));
		retres5 = 1;
		pkt[0] = 0xF6;
		pkt[2] = 0xB1;
		pkt[8] = sizeof(outbuf);
		for ( i = 0; i < 10 && retres5; i += 1 )
		{
			retres5 = sceCdAtapiExecCmd_local(0, outbuf, 1, sizeof(outbuf), pkt, sizeof(pkt), 2);
			if ( retres5 || (retres5 = sceCdAtapiWaitResult_local()) )
				DelayThread(10000);
		}
		if ( retres5 )
			return retres4;
		*g_cd_sc_ffffffd9_ptr = (outbuf[4] & 0x81) ? 1 : 0;
	}
	return retres4;
}

static int atapi_spin_status_get(int unused_arg1, void *buf)
{
	int result;
	char pkt[12];

	(void)unused_arg1;
	memset(pkt, 0, sizeof(pkt));
	pkt[0] = 0xF6;
	pkt[2] = 0xA0;
	pkt[8] = 6;
	result = xatapi_7_sceCdAtapiExecCmd(0, buf, 1, 6, pkt, sizeof(pkt), 2);
	return (!result) ? xatapi_8_sceCdAtapiWaitResult() : result;
}

static int atapi_check_if_drive_ready(int check_nowait)
{
	u8 ata_control;
	int drive_err;
	int unitreadyctrl;
	int senseret;

	drive_err = 0;
	VERBOSE_KPRINTF(1, "Atapi Drive Ready Call %d\n", check_nowait);
	ata_control = xatapi_12_get_ata_control();
	VERBOSE_KPRINTF(1, "Atapi Drive Ready %04x\n", ata_control);
	if ( !check_nowait )
	{
		int req_test_unit_ready_tmp1;

		req_test_unit_ready_tmp1 = -1;
		while ( req_test_unit_ready_tmp1 < 0 )
		{
			char spinstatus_tmp[6];

			while ( (ata_control & 0xC0) != 64 )
			{
				VERBOSE_KPRINTF(1, "Drive Not Ready %04x\n", ata_control);
				DelayThread(2000);
				ata_control = xatapi_12_get_ata_control();
			}
			req_test_unit_ready_tmp1 = do_atapi_request_test_unit_ready(0, &drive_err, &unitreadyctrl);
			if ( req_test_unit_ready_tmp1 < 0 )
				DelayThread(100000);
			if ( !atapi_req_sense_get(0, &senseret) )
			{
				VERBOSE_KPRINTF(1, "ReqSense %08x\n", senseret);
				if ( (senseret & 0xFFFFFF00) == 0x23A00 )
					break;
			}
			if ( !atapi_spin_status_get(0, spinstatus_tmp) )
			{
				VERBOSE_KPRINTF(1, "Spin Status 3:%02x 5:%02x\n", (u8)spinstatus_tmp[3], (u8)spinstatus_tmp[5]);
				if ( (spinstatus_tmp[3] & 2) || (spinstatus_tmp[3] & 0x80) )
					break;
			}
			VERBOSE_KPRINTF(1, "Atapi Drive err %08x\n", drive_err);
		}
		return 2;
	}
	return ((ata_control & 0xC0) == 64 && do_atapi_request_test_unit_ready(0, &drive_err, &unitreadyctrl) >= 0
					&& !drive_err) ?
					 2 :
					 6;
}

static int sceFsDevctlBlkIO(s16 dev_nr, void *buf, void *rwbuf, unsigned int nsec, int secsize, int rwtype)
{
	char *rwbuf_tmp;
	unsigned int nsec_tmp;
	int retres1;
	int seccnt;
	int i;
	char pkt[12];

	rwbuf_tmp = (char *)rwbuf;
	retres1 = 0;
	VERBOSE_KPRINTF(1, "dma %c %08x, nsec %d\n", rwtype ? 'w' : 'r', rwbuf, nsec);
	for ( nsec_tmp = nsec; !retres1 && nsec_tmp; nsec_tmp -= seccnt )
	{
		seccnt = (nsec_tmp >= 0x21) ? 32 : nsec_tmp;
		for ( i = 3; i < 3; i += 1 )
		{
			xatapi_9_sceCdSpdAtaDmaStart(rwtype);
			memset(pkt, 0, sizeof(pkt));
			pkt[0] = (!rwtype) ? 0x28 : 0x2A;
			pkt[2] = ((uiptr)rwbuf_tmp >> 24) & 0xFF;
			pkt[3] = ((uiptr)rwbuf_tmp >> 16) & 0xFF;
			pkt[4] = ((uiptr)rwbuf_tmp >> 8) & 0xFF;
			pkt[5] = ((uiptr)rwbuf_tmp) & 0xFF;
			pkt[8] = seccnt;
			retres1 = xatapi_7_sceCdAtapiExecCmd(dev_nr, buf, seccnt, secsize, pkt, sizeof(pkt), 4);
			if ( retres1 )
			{
				xatapi_10_sceCdSpdAtaDmaEnd();
				break;
			}
			retres1 = xatapi_8_sceCdAtapiWaitResult();
			xatapi_10_sceCdSpdAtaDmaEnd();
			if ( retres1 != -510 )
			{
				break;
			}
		}
		rwbuf_tmp += seccnt;
		buf = (char *)buf + seccnt * secsize;
	}
	return retres1;
}

static void expbay_device_reset(void)
{
	if ( (*g_dev9_reg_power & 4) )
	{
		Kprintf("xatapi already Power On\n");
		*g_dev9_reg_1460 |= 2;
		return;
	}
	Kprintf("xatapi Power On Start\n");
	*g_dev9_reg_power = (*g_dev9_reg_power & ~5) | 4;
	DelayThread(500000);
	*g_dev9_reg_1460 |= 2;
	*g_dev9_reg_power |= 1;
	DelayThread(500000);
}

static int cd_atapi_intr_callback_cb(int cbarg)
{
	VERBOSE_KPRINTF(1, "dev5 interrupt\n");
	if ( p_dev5_intr_cb )
		p_dev5_intr_cb(cbarg);
	VERBOSE_KPRINTF(1, "dev5 interrupt end\n");
	return 1;
}

static void speedRegisterIntrDispatchCb(void *callback)
{
	p_dev5_intr_cb = (int (*)(int flag))callback;
}

static void sceDev5Init(void)
{
	VERBOSE_KPRINTF(1, "dev5 atapi Init start\n");
	sceCdSC(0xFFFFFFE5, (int *)cd_atapi_intr_callback_cb);
	speed_device_init();
	speed_init();
	VERBOSE_KPRINTF(1, "dev5 atapi Init end\n");
}

static int atapi_eject_interrupt_handler(int is_eject, void *userdata)
{
	u32 buzzerres;

	(void)userdata;
	if ( is_eject != 1 )
		return 1;
	VERBOSE_KPRINTF(1, "Eject intr : media removal\n");
	return sceCdBuzzerCtl(&buzzerres);
}

static int xatapi_do_init(void)
{
	int (*oldcb)(int is_eject, void *userdata);
	u32 *sc_tmp;
	u32 trylocktmp;
	u32 traylock_ret;

	if ( !create_event_flags() )
		return 0;
	sceCdSC(0xFFFFFFD9, (int *)&sc_tmp);
	g_cd_sc_ffffffd9_ptr = sc_tmp;
	sceCdSC(0xFFFFFFD7, (int *)&sc_tmp);
	g_bf40200a_is_set_ptr = (int)sc_tmp;
	oldcb = (int (*)(int is_eject, void *userdata))sceCdSetAtapiEjectCallback(
		(int (*)(int is_eject, void *userdata))atapi_eject_interrupt_handler, 0);
	if ( !sceCdGetMediumRemoval(&trylocktmp, &traylock_ret) )
	{
		VERBOSE_KPRINTF(0, "xatapi:sceCdGetMediumRemoval NG(%x) !!\n", traylock_ret);
		trylocktmp = 0;
	}
	if ( trylocktmp )
	{
		VERBOSE_KPRINTF(0, "xatapi:Tray locked !!\n");
	}
	else if ( sceCdSetMediumRemoval(1, &traylock_ret) )
	{
		VERBOSE_KPRINTF(0, "xatapi:Tray lock\n");
	}
	else
	{
		VERBOSE_KPRINTF(0, "xatapi:Tray lock NG(%x) !!\n", traylock_ret);
		trylocktmp = 0;
	}
	expbay_device_reset();
	VERBOSE_KPRINTF(0, "xatapi Dev5->Rainbow\n");
	sceCdChgSys(1);
	VERBOSE_KPRINTF(1, "xatapi Dev5->Rainbow end\n");
	VERBOSE_KPRINTF(1, "sceDev5Init Call\n");
	sceDev5Init();
	VERBOSE_KPRINTF(1, "sceAtInterInit Call\n");
	sceAtInterInit();
	VERBOSE_KPRINTF(1, "sceAtapiInit Call\n");
	sceAtapiInit(0);
	VERBOSE_KPRINTF(1, "sceAtapiInit end\n");
	sceCdSC(0xFFFFFFE1, (int *)chgsys_callback_cb);
	DelayThread(10000);
	if ( !trylocktmp )
	{
		if ( !sceCdSetMediumRemoval(0, &traylock_ret) )
		{
			VERBOSE_KPRINTF(0, "xatapi:Tray unlock NG(%x) !!\n", traylock_ret);
			trylocktmp = 0;
		}
		else
		{
			VERBOSE_KPRINTF(0, "xatapi:Tray unlock\n");
		}
	}
	sceCdSetAtapiEjectCallback(oldcb, 0);
	return 1;
}

int xatapi_2_terminate(int with_quit)
{
	int sc_tmp;

	(void)with_quit;
	sc_tmp = 0;
	sceCdSC(0xFFFFFFE1, &sc_tmp);
	sceCdSC(0xFFFFFFE5, &sc_tmp);
	sceCdSC(0xFFFFFFE0, &sc_tmp);
	sceCdSC(0xFFFFFFDF, &sc_tmp);
	sceCdSC(0xFFFFFFE4, &sc_tmp);
	Kprintf("libxatapi_terminate\n");
	return 0;
}

static int xatapi_dev_devctl(
	const iop_file_t *f, const char *name, int cmd, void *args, unsigned int arglen, void *buf, unsigned int buflen)
{
	int retres1;
	u32 efbits;

	(void)name;
	retres1 = 0;
	VERBOSE_KPRINTF(1, "xatapi devctl: cmd:%08x arg:%d\n", cmd, *(u32 *)args);
	if ( cmd == 0x439B && PollEventFlag(g_io_event_flag, 1, WEF_AND, &efbits) == KE_EVF_COND && *(u32 *)args == 1 )
	{
		*(u32 *)buf = 6;
		return 0;
	}
	WaitEventFlag(g_io_event_flag, 1, WEF_AND | WEF_CLEAR, &efbits);
	if ( g_devctl_retonly_unset )
	{
		SetEventFlag(g_io_event_flag, 1);
		return -EIO;
	}
	switch ( cmd )
	{
		case 0x4332:
			retres1 = -EINVAL;
			if ( *(u32 *)args )
			{
				switch ( *((u32 *)args + 1) )
				{
					case 2:
					case 5:
						if ( !buflen )
						{
							retres1 = -EINVAL;
							break;
						}
						retres1 =
							xatapi_7_sceCdAtapiExecCmd(f->unit, buf, 1, buflen, (char *)args + 8, *(u32 *)args, *((u32 *)args + 1));
						break;
					case 3:
					case 6:
						if ( *((u32 *)args + 250) )
						{
							retres1 = xatapi_7_sceCdAtapiExecCmd(
								f->unit,
								(char *)args + 40,
								1,
								*((u32 *)args + 250),
								(char *)args + 8,
								*(u32 *)args,
								*((u32 *)args + 1));
						}
						else
						{
							retres1 = -EINVAL;
						}
						break;
					default:
						retres1 = xatapi_7_sceCdAtapiExecCmd(f->unit, 0, 0, 0, (char *)args + 8, *(u32 *)args, *((u32 *)args + 1));
						break;
				}
				if ( !retres1 )
					retres1 = xatapi_8_sceCdAtapiWaitResult();
			}
			break;
		case 0x4333:
			*(u32 *)buf = (u8)xatapi_11_sceAtaGetError();
			break;
		case 0x4334:
			*(u32 *)buf = (u8)xatapi_12_get_ata_control();
			break;
		case 0x4335:
			retres1 = sceCdAtapi_BC();
			if ( retres1 == 1 )
				retres1 = 0;
			break;
		case 0x4336:
			retres1 = sceCdAtapi_SC();
			break;
		case 0x4337:  //?
			g_verbose_level = *(u32 *)args;
			break;
		case 0x4338:
			g_reset_scrambling_pack = *(u32 *)args;
			if ( g_reset_scrambling_pack )
			{
				g_pes_scrambling_control_pack = 0;
			}
			else
			{
				VERBOSE_KPRINTF(1, "pes scrambling control pack = %d\n", g_pes_scrambling_control_pack);
			}
			break;
		case 0x4339:
			DmaRun_spck((char *)args, arglen);
			break;
		case 0x433A:
			g_dma_mode_value = (*(u32 *)args & 0x40) ? 1 : 0;
			g_dma_speed_value = *(u32 *)args & 7;
			atapi_device_set_transfer_mode_outer(0);
			break;
		case 0x433B:
			if ( buf && buflen >= 2 )
				*(u16 *)buf = FpgaGetRevision();
			else
				Kprintf("CDIOC_ATAPI_GETFPGAREV:buffer NG\n");
			break;
		case 0x433C:
			if ( buf && buflen >= 4 )
				*(u32 *)buf = 0x4121300;
			else
				Kprintf("CDIOC_ATAPI_VERSION:buffer NG\n");
			break;
		case 0x433D:
			if ( buf && buflen >= 4 )
				*(u32 *)buf = g_pes_scrambling_control_pack;
			else
				Kprintf("CDIOC_ATAPI_GETPSCNT:buffer NG\n");
			break;
		case 0x439B:
			*(u32 *)buf = atapi_check_if_drive_ready(*(u32 *)args);
			break;
		case 0x4601:
			if ( *((u32 *)args + 3) != 2048 )
			{
				retres1 = -EINVAL;
				break;
			}
			VERBOSE_KPRINTF(
				1,
				"sceFsDevctlBlkIO Lsn:%d nsec:%d buffer:%08x Type:%d\n",
				*(u32 *)args,
				*((u32 *)args + 1),
				*((u32 *)args + 2),
				*((u32 *)args + 4));
			retres1 =
				sceFsDevctlBlkIO(f->unit, *((void **)args + 2), *(void **)args, *((u32 *)args + 1), 2048, *((u32 *)args + 4));
			break;
		default:
			Kprintf("Un-support devctl %08x\n", cmd);
			retres1 = -EIO;
			break;
	}
	SetEventFlag(g_io_event_flag, 1);
	VERBOSE_KPRINTF(1, "xatapi devctl: cmd:%08x End.\n", cmd);
	return retres1;
}

int _start(int ac, char **av)
{
	(void)ac;
	(void)av;

	Kprintf("xatapi_init Call\n");
	// Unofficial: initialize variable here
	g_dma_mode_value = 1;
	// Unofficial: initialize variable here
	g_dma_speed_value = 2;
	if ( RegisterLibraryEntries(&_exp_xatapi) )
		return MODULE_NO_RESIDENT_END;
	DelDrv("xatapi");
	if ( AddDrv(&ata_ioman_device) )
	{
		// Unofficial: omitted call to empty dev deinit function
		return MODULE_NO_RESIDENT_END;
	}
	return !xatapi_do_init() ? MODULE_NO_RESIDENT_END : MODULE_RESIDENT_END;
}

static int expbay_get_has_power(void)
{
	return *g_dev9_reg_power & 4;
}

static void speedRegisterIntrCb(int intr, void *cb)
{
	g_dev5_intr_cbs[intr] = cb;
}

static void speedRegisterPreDmaCb(int ctrl, void *cb)
{
	g_dev5_predma_cbs[ctrl] = cb;
}

static void speedRegisterPostDmaCb(int ctrl, void *cb)
{
	g_dev5_postdma_cbs[ctrl] = cb;
}

static int speed_intr_dispatch(int flag)
{
	int i;
	int j;
	USE_DEV5_SPEED_REGS();

	if ( flag == 1 )
	{
		AtaEjectIntrHandle();
		return 0;
	}
	if ( (dev5_speed_regs->r_spd_intr_stat & 0x3EFC) )
	{
		VERBOSE_KPRINTF(
			0, "SL3 register access failed(%x:%x) !!\n", dev5_speed_regs->r_spd_intr_stat, dev5_speed_regs->r_spd_intr_mask);
		return 0;
	}
	for ( i = 0; i < 3 && (u16)(dev5_speed_regs->r_spd_intr_stat & dev5_speed_regs->r_spd_intr_mask); i += 1 )
	{
		for ( j = 0; j < (int)(sizeof(g_dev5_intr_cbs) / sizeof(g_dev5_intr_cbs[0])); j += 1 )
		{
			if (
				g_dev5_intr_cbs[j]
				&& (((int)(u16)(dev5_speed_regs->r_spd_intr_stat & dev5_speed_regs->r_spd_intr_mask) >> j) & 1) )
				g_dev5_intr_cbs[j](flag);
		}
	}
	return 0;
}

static void speedIntrEnable(s16 mask)
{
	int state;
	USE_DEV5_SPEED_REGS();

	CpuSuspendIntr(&state);
	dev5_speed_regs->r_spd_intr_mask &= ~mask;
	dev5_speed_regs->r_spd_intr_mask |= mask;
	CpuResumeIntr(state);
}

static void speedIntrDisable(s16 mask)
{
	int state;
	USE_DEV5_SPEED_REGS();

	CpuSuspendIntr(&state);
	dev5_speed_regs->r_spd_intr_mask &= ~mask;
	CpuResumeIntr(state);
}

static int SpdDmaTransfer(unsigned int device, void *buf, u32 bcr_in, int dir)
{
	int result;
	USE_DEV5_SPEED_REGS();

	dmac_ch_set_chcr(IOP_DMAC_CDVD, 0);
	dmac_ch_get_chcr(IOP_DMAC_CDVD);
	if ( device >= 2 && (!g_dev5_predma_cbs[device] || !g_dev5_postdma_cbs[device]) )
		return -1;
	VERBOSE_KPRINTF(1, "Wait Intr\n");
	result = WaitSema(g_dma_lock_sema);
	if ( result < 0 )
		return result;
	dev5_speed_regs->r_spd_dma_ctrl = (dev5_speed_regs->r_spd_rev_1 >= 0x11) ? ((device & 1) | 6) : ((device & 3) | 4);
	if ( g_dev5_predma_cbs[device] )
		g_dev5_predma_cbs[device](bcr_in, dir);
	VERBOSE_KPRINTF(1, "DMA Ch3 Set.\n");
	VERBOSE_KPRINTF(1, "Set MADR3:%08x Set BCR3:%08x\n", buf, bcr_in);
	dmac_ch_set_madr(IOP_DMAC_CDVD, (u32)buf);
	dmac_ch_set_bcr(IOP_DMAC_CDVD, bcr_in);
	dmac_ch_set_chcr(IOP_DMAC_CDVD, dir | 0x41000200);
	dmac_ch_get_chcr(IOP_DMAC_CDVD);
	VERBOSE_KPRINTF(
		1,
		"CHCR3:%08x MADR3:%08x BCR3:%08x\n",
		dmac_ch_get_chcr(IOP_DMAC_CDVD),
		dmac_ch_get_madr(IOP_DMAC_CDVD),
		dmac_ch_get_bcr(IOP_DMAC_CDVD));
	while ( (dmac_ch_get_chcr(IOP_DMAC_CDVD) & 0x1000000) )
	{
	}
	VERBOSE_KPRINTF(1, "MADR3= %08x\n", dmac_ch_get_madr(IOP_DMAC_CDVD));
	if ( g_dev5_postdma_cbs[device] )
		g_dev5_postdma_cbs[device](bcr_in, dir);
	VERBOSE_KPRINTF(1, "SpdDmaTransfer End.\n");
	SignalSema(g_dma_lock_sema);
	return 0;
}

static int SpdDmaTransfer_extrans_1(unsigned int device, void *buf, u32 bcr_in, int dir)
{
	int result;
	USE_DEV5_SPEED_REGS();

	dmac_ch_set_chcr(IOP_DMAC_CDVD, 0);
	dmac_ch_get_chcr(IOP_DMAC_CDVD);
	if ( device >= 2 && (!g_dev5_predma_cbs[device] || !g_dev5_postdma_cbs[device]) )
		return -1;
	VERBOSE_KPRINTF(1, "Wait Intr\n");
	result = WaitSema(g_dma_lock_sema);
	if ( result < 0 )
		return result;
	dev5_speed_regs->r_spd_dma_ctrl = (dev5_speed_regs->r_spd_rev_1 >= 0x11) ? ((device & 1) | 6) : ((device & 3) | 4);
	if ( g_dev5_predma_cbs[device] )
		g_dev5_predma_cbs[device](bcr_in, dir);
	speedIntrDisable(256);
	FpgaLayer1On();
	FpgaXfrenOn();
	VERBOSE_KPRINTF(1, "DMA Ch3 Set.\n");
	VERBOSE_KPRINTF(1, "Set MADR3:%08x Set BCR3:%08x\n", buf, bcr_in);
	dmac_ch_set_madr(IOP_DMAC_CDVD, (u32)buf);
	dmac_ch_set_bcr(IOP_DMAC_CDVD, bcr_in);
	dmac_ch_set_chcr(IOP_DMAC_CDVD, dir | 0x41000200);
	dmac_ch_get_chcr(IOP_DMAC_CDVD);
	VERBOSE_KPRINTF(
		1,
		"CHCR3:%08x MADR3:%08x BCR3:%08x\n",
		dmac_ch_get_chcr(IOP_DMAC_CDVD),
		dmac_ch_get_madr(IOP_DMAC_CDVD),
		dmac_ch_get_bcr(IOP_DMAC_CDVD));
	while ( (dmac_ch_get_chcr(IOP_DMAC_CDVD) & 0x1000000) )
	{
	}
	VERBOSE_KPRINTF(1, "MADR3= %08x\n", dmac_ch_get_madr(IOP_DMAC_CDVD));
	FpgaCheckWriteBuffer();
	FpgaXfrenOff();
	FpgaLayer1Off();
	speedIntrEnable(256);
	if ( g_dev5_postdma_cbs[device] )
		g_dev5_postdma_cbs[device](bcr_in, dir);
	VERBOSE_KPRINTF(1, "SpdDmaTransfer_extrans End.\n");
	SignalSema(g_dma_lock_sema);
	return 0;
}

static int SpdDmaTransfer_extrans_2(unsigned int device, void *buf, u32 bcr_in, int dir)
{
	int result;

	dmac_ch_set_chcr(IOP_DMAC_CDVD, 0);
	dmac_ch_get_chcr(IOP_DMAC_CDVD);
	if ( device >= 2 && (!g_dev5_predma_cbs[device] || !g_dev5_postdma_cbs[device]) )
		return -1;
	VERBOSE_KPRINTF(1, "Wait Intr\n");
	result = WaitSema(g_dma_lock_sema);
	if ( result < 0 )
		return result;
	VERBOSE_KPRINTF(1, "DMA Ch3 Set.\n");
	VERBOSE_KPRINTF(1, "Set MADR3:%08x Set BCR3:%08x\n", buf, bcr_in);
	dmac_ch_set_madr(IOP_DMAC_CDVD, (u32)buf);
	dmac_ch_set_bcr(IOP_DMAC_CDVD, bcr_in);
	dmac_ch_set_chcr(IOP_DMAC_CDVD, dir | 0x41000200);
	dmac_ch_get_chcr(IOP_DMAC_CDVD);
	VERBOSE_KPRINTF(
		1,
		"CHCR3:%08x MADR3:%08x BCR3:%08x\n",
		dmac_ch_get_chcr(IOP_DMAC_CDVD),
		dmac_ch_get_madr(IOP_DMAC_CDVD),
		dmac_ch_get_bcr(IOP_DMAC_CDVD));
	while ( (dmac_ch_get_chcr(IOP_DMAC_CDVD) & 0x1000000) )
	{
	}
	VERBOSE_KPRINTF(1, "MADR3= %08x\n", dmac_ch_get_madr(IOP_DMAC_CDVD));
	VERBOSE_KPRINTF(1, "SpdDmaTransfer_extrans End.\n");
	SignalSema(g_dma_lock_sema);
	return 0;
}

static int SpdDmaTransfer_extrans_3(unsigned int device, void *buf, u32 bcr_in, int dir)
{
	int result;

	dmac_ch_set_chcr(IOP_DMAC_CDVD, 0);
	dmac_ch_get_chcr(IOP_DMAC_CDVD);
	if ( device >= 2 && (!g_dev5_predma_cbs[device] || !g_dev5_postdma_cbs[device]) )
		return -1;
	VERBOSE_KPRINTF(1, "Wait Intr\n");
	result = WaitSema(g_dma_lock_sema);
	if ( result < 0 )
		return result;
	FpgaLayer1On();
	FpgaXfrenOn();
	VERBOSE_KPRINTF(1, "DMA Ch3 Set.\n");
	VERBOSE_KPRINTF(1, "Set MADR3:%08x Set BCR3:%08x\n", buf, bcr_in);
	dmac_ch_set_madr(IOP_DMAC_CDVD, (u32)buf);
	dmac_ch_set_bcr(IOP_DMAC_CDVD, bcr_in);
	dmac_ch_set_chcr(IOP_DMAC_CDVD, dir | 0x41000200);
	dmac_ch_get_chcr(IOP_DMAC_CDVD);
	VERBOSE_KPRINTF(
		1,
		"CHCR3:%08x MADR3:%08x BCR3:%08x\n",
		dmac_ch_get_chcr(IOP_DMAC_CDVD),
		dmac_ch_get_madr(IOP_DMAC_CDVD),
		dmac_ch_get_bcr(IOP_DMAC_CDVD));
	while ( (dmac_ch_get_chcr(IOP_DMAC_CDVD) & 0x1000000) )
	{
	}
	VERBOSE_KPRINTF(1, "MADR3= %08x\n", dmac_ch_get_madr(IOP_DMAC_CDVD));
	FpgaCheckWriteBuffer2();
	FpgaXfrenOff();
	FpgaLayer1Off();
	VERBOSE_KPRINTF(1, "SpdDmaTransfer_extrans End.\n");
	SignalSema(g_dma_lock_sema);
	return 0;
}

static void speedLEDCtl(int ctl)
{
	USE_DEV5_SPEED_REGS();

	// Unofficial: was 8 bit access
	dev5_speed_regs->r_spd_pio_data = !ctl;
}

static void speed_init(void)
{
	int i;
	iop_sema_t semaparam;

	semaparam.attr = SA_THPRI;
	semaparam.initial = 1;
	semaparam.max = 1;
	semaparam.option = 0;
	g_dma_lock_sema = CreateSema(&semaparam);
	if ( g_dma_lock_sema <= 0 )
		return;
	speedIntrDisable(0xFFFF);
	speedRegisterIntrDispatchCb(speed_intr_dispatch);
	for ( i = 0; i < (int)(sizeof(g_dev5_intr_cbs) / sizeof(g_dev5_intr_cbs[0])); i += 1 )
	{
		g_dev5_intr_cbs[i] = 0;
	}
	for ( i = 0; i < (int)(sizeof(g_dev5_predma_cbs) / sizeof(g_dev5_predma_cbs[0])); i += 1 )
	{
		g_dev5_predma_cbs[i] = 0;
	}
	for ( i = 0; i < (int)(sizeof(g_dev5_postdma_cbs) / sizeof(g_dev5_postdma_cbs[0])); i += 1 )
	{
		g_dev5_postdma_cbs[i] = 0;
	}
	speedLEDCtl(0);
	return;
}

static void speed_device_init(void)
{
	int idx;
	const char *revtypes[4];
	USE_DEV5_SPEED_REGS();

	revtypes[0] = "unknown";
	revtypes[1] = "TS";
	revtypes[2] = "ES1";
	revtypes[3] = "ES2";
	switch ( dev5_speed_regs->r_spd_rev_1 )
	{
		case 9:
			idx = 1;
			break;
		case 16:
			idx = 2;
			break;
		case 17:
			idx = 3;
			break;
		default:
			idx = 0;
			break;
	}
	if ( idx )
	{
		VERBOSE_KPRINTF(1, "Speed chip: %s\n", revtypes[idx]);
	}
	else
	{
		VERBOSE_KPRINTF(1, "Speed chip: Rev %x\n", dev5_speed_regs->r_spd_rev_1);
	}
	VERBOSE_KPRINTF(
		1, "Speed version(rev3.rev8) = %04x.%04x\n", dev5_speed_regs->r_spd_rev_3, dev5_speed_regs->r_spd_rev_8);
}

static void do_hex_dump(void *ptr, int len)
{
	int i;
	int charbuf_offs;
	int j;
	char charbuf[17];

	if ( !g_verbose_level )
	{
		return;
	}
	Kprintf("Hex dump 00 01 02 03 04 05 06 07 08 09 0a 0b 0c 0d 0e 0f");
	j = 0;
	for ( i = 0; i < len; i += 256 )
	{
		for ( j = 0; j < (((len - i) > 256) ? 256 : (len - i)); j += 1 )
		{
			charbuf_offs = j & 0xF;
			if ( !charbuf_offs && j )
			{
				charbuf[16] = 0;
				Kprintf(" %s\n%08x", charbuf, i + j);
			}
			Kprintf(" %02x", (u8)((char *)ptr)[i + j]);
			charbuf[charbuf_offs] = (((char *)ptr)[i + j] >= 0 && isgraph(((char *)ptr)[i + j])) ? ((char *)ptr)[i + j] : '.';
		}
	}
	charbuf_offs = j & 0xF;
	for ( i = 0; (unsigned int)i < (unsigned int)((sizeof(charbuf) - 1) - charbuf_offs); i += 1 )
	{
		Kprintf("\t  ");
	}
	charbuf[charbuf_offs] = 0;
	Kprintf("%s\n", charbuf);
}

static void ata_pre_dma_cb(void)
{
	USE_DEV5_SPEED_REGS();

	VERBOSE_KPRINTF(1, "ata_pre_dma_handler:old %x\n", dev5_speed_regs->r_spd_xfr_ctrl);
	dev5_speed_regs->r_spd_xfr_ctrl |= 0x80;
	VERBOSE_KPRINTF(1, "ata_pre_dma_handler:new %x\n", dev5_speed_regs->r_spd_xfr_ctrl);
}

static void ata_post_dma_cb(void)
{
	USE_DEV5_SPEED_REGS();

	VERBOSE_KPRINTF(1, "ata_post_dma_handler:old %x\n", dev5_speed_regs->r_spd_xfr_ctrl);
	dev5_speed_regs->r_spd_xfr_ctrl &= ~0x80;
	VERBOSE_KPRINTF(1, "ata_post_dma_handler:new %x\n", dev5_speed_regs->r_spd_xfr_ctrl);
}

static int vReferEventFlagStatus(int ef, iop_event_info_t *info)
{
	return (QueryIntrContext()) ? iReferEventFlagStatus(ef, info) : ReferEventFlagStatus(ef, info);
}

void xatapi_9_sceCdSpdAtaDmaStart(int dir)
{
	int spd_if_ctrl_manip_2;
	u32 efbits;
	USE_DEV5_SPEED_REGS();

	g_is_wait_busy = 0;
	WaitEventFlag(g_adma_evfid, 1, WEF_AND | WEF_CLEAR, &efbits);
	g_should_wait_for_dma_flag = 1;
	VERBOSE_KPRINTF(1, "sceCdSpdAtaDmaStart Call %d :Read 0:Write 1\n", dir);
	dev5_speed_regs->r_spd_dbuf_stat = 3;
	spd_if_ctrl_manip_2 = (dev5_speed_regs->r_spd_if_ctrl & 1) | (dir ? 0x4C : 0x4E);
	dev5_speed_regs->r_spd_if_ctrl = spd_if_ctrl_manip_2;
	VERBOSE_KPRINTF(1, "sceCdSpdAtaDmaStart Write R_IF_CTR:%x\n", spd_if_ctrl_manip_2);
	dev5_speed_regs->r_spd_xfr_ctrl = dir | 6;
	VERBOSE_KPRINTF(
		1,
		"sceCdSpdAtaDmaStart R_IF_CTR:%x R_XFR_CTRL:%x\n",
		dev5_speed_regs->r_spd_if_ctrl,
		dev5_speed_regs->r_spd_xfr_ctrl);
}

void xatapi_10_sceCdSpdAtaDmaEnd(void)
{
	iop_event_info_t efinfo;
	USE_DEV5_SPEED_REGS();

	VERBOSE_KPRINTF(1, "sceCdSpdAtaDmaEnd Call\n");
	if ( !g_should_wait_for_dma_flag )
	{
		VERBOSE_KPRINTF(1, "sceCdSpdAtaDmaEnd No Start(flag)\n");
		return;
	}
	if ( vReferEventFlagStatus(g_adma_evfid, &efinfo) || (efinfo.currBits & 1) )
	{
		VERBOSE_KPRINTF(1, "sceCdSpdAtaDmaEnd No Start\n");
		g_should_wait_for_dma_flag = 0;
		return;
	}
	dev5_speed_regs->r_spd_xfr_ctrl = 1;
	dev5_speed_regs->r_spd_if_ctrl &= ~0x4;
	dev5_speed_regs->r_spd_dbuf_stat = 3;
	dev5_speed_regs->r_spd_if_ctrl &= ~0x84;
	dev5_speed_regs->r_spd_if_ctrl |= 0x80;
	dev5_speed_regs->r_spd_if_ctrl &= ~0x84;
	ata_pio_mode(0);
	if ( g_dma_mode_value )
		ata_ultra_dma_mode(g_dma_speed_value);
	else
		ata_multiword_dma_mode(g_dma_speed_value);
	g_should_wait_for_dma_flag = 0;
	SetEventFlag(g_adma_evfid, 1);
}

static void ata_pio_mode(int mode)
{
	USE_DEV5_SPEED_REGS();

	VERBOSE_KPRINTF(1, "SpdAtaSetPioTiming %d\n", mode);
	switch ( mode )
	{
		case 0:
		default:
			dev5_speed_regs->r_spd_pio_mode = 146;
			break;
		case 1:
			dev5_speed_regs->r_spd_pio_mode = 114;
			break;
		case 2:
			dev5_speed_regs->r_spd_pio_mode = 50;
			break;
		case 3:
			dev5_speed_regs->r_spd_pio_mode = 36;
			break;
		case 4:
			dev5_speed_regs->r_spd_pio_mode = 35;
			break;
	}
}

static void ata_multiword_dma_mode(int mode)
{
	USE_DEV5_SPEED_REGS();

	VERBOSE_KPRINTF(1, "SpdAtaSetMdmaTiming %d\n", mode);
	switch ( mode )
	{
		case 0:
		default:
			dev5_speed_regs->r_spd_mwdma_mode = 255;
			break;
		case 1:
			dev5_speed_regs->r_spd_mwdma_mode = 69;
			break;
		case 2:
			dev5_speed_regs->r_spd_mwdma_mode = 36;
			break;
	}
	dev5_speed_regs->r_spd_if_ctrl = (dev5_speed_regs->r_spd_if_ctrl & ~0x49) | 0x48;
}

static void ata_ultra_dma_mode(int mode)
{
	USE_DEV5_SPEED_REGS();

	VERBOSE_KPRINTF(1, "SpdAtaSetUdmaTiming %d\n", mode);
	switch ( mode )
	{
		case 0:
		default:
			dev5_speed_regs->r_spd_udma_mode = 167;
			break;
		case 1:
			dev5_speed_regs->r_spd_udma_mode = 133;
			break;
		case 2:
			dev5_speed_regs->r_spd_udma_mode = 99;
			break;
		case 3:
			dev5_speed_regs->r_spd_udma_mode = 98;
			break;
		case 4:
			dev5_speed_regs->r_spd_udma_mode = 97;
			break;
	}
	dev5_speed_regs->r_spd_if_ctrl = dev5_speed_regs->r_spd_if_ctrl | 0x49;
}

static int ata_intr_cb(int flag)
{
	VERBOSE_KPRINTF(1, "call AtaIntrHandle %d\n", flag);
	speedIntrDisable(3);
	iSetEventFlag(g_atapi_event_flag, 2);
	return 1;
}

static void AtaEjectIntrHandle(void)
{
	VERBOSE_KPRINTF(1, "call AtaEjectIntrHandle\n");
	g_is_wait_busy = 1;
	iSetEventFlag(g_atapi_event_flag, 4);
}

static unsigned int AtaAlarmrHandle(void *usrdat)
{
	(void)usrdat;

	VERBOSE_KPRINTF(1, "call AtaAlarmrHandle\n");
	iSetEventFlag(g_atapi_event_flag, 1);
	return 0;
}

int xatapi_14_set_speed_reg(int regaddr, u16 regval)
{
	u32 efbits;
	USE_DEV5_SPEED_REGS();

	if ( (unsigned int)(regaddr - 64) < 0x1D )
	{
		WaitEventFlag(g_acmd_evfid, 1, WEF_AND | WEF_CLEAR, &efbits);
		*(vu16 *)((char *)&dev5_speed_regs->unv00 + regaddr) = regval;
		SetEventFlag(g_acmd_evfid, 1);
	}
	return regval;
}

int xatapi_13_get_speed_reg(int regaddr)
{
	int tmpval;
	u32 efbits;
	USE_DEV5_SPEED_REGS();

	if ( (unsigned int)(regaddr - 64) >= 0x1D )
		return 0;
	WaitEventFlag(g_acmd_evfid, 1, WEF_AND | WEF_CLEAR, &efbits);
	tmpval = *(u16 *)((char *)&dev5_speed_regs->unv00 + regaddr);
	SetEventFlag(g_acmd_evfid, 1);
	return tmpval;
}

int xatapi_11_sceAtaGetError(void)
{
	u8 r_spd_ata_error;
	u32 efbits;
	USE_DEV5_SPEED_REGS();

	WaitEventFlag(g_acmd_evfid, 1, WEF_AND | WEF_CLEAR, &efbits);
	r_spd_ata_error = dev5_speed_regs->r_spd_ata_error;
	SetEventFlag(g_acmd_evfid, 1);
	return r_spd_ata_error;
}

int xatapi_12_get_ata_control(void)
{
	u8 r_spd_ata_control;
	u32 efbits;
	USE_DEV5_SPEED_REGS();

	WaitEventFlag(g_acmd_evfid, 1, WEF_AND | WEF_CLEAR, &efbits);
	r_spd_ata_control = dev5_speed_regs->r_spd_ata_control;
	SetEventFlag(g_acmd_evfid, 1);
	return r_spd_ata_control;
}

static int sceAtaGetError(void)
{
	USE_DEV5_SPEED_REGS();

	return (u8)dev5_speed_regs->r_spd_ata_error;
}

static int ata_wait_busy1_busy(void)
{
	unsigned int i;
	USE_DEV5_SPEED_REGS();

	for ( i = 0; i < 0x50; i += 1 )
	{
		if ( !(dev5_speed_regs->r_spd_ata_control & 0x80) )
		{
			return 0;
		}
		if ( !(u16)expbay_get_has_power() )
		{
			VERBOSE_KPRINTF(1, "DEV5 ATA: error: wait busy, power off.\n");
			return -551;
		}
		if ( g_is_wait_busy )
		{
			VERBOSE_KPRINTF(1, "DEV5 ATA: error: wait busy, tray eject.\n");
			return -550;
		}
		switch ( i / 0xA )
		{
			case 0:
				break;
			case 1:
				DelayThread(100);
				break;
			case 2:
				DelayThread(1000);
				break;
			case 3:
				DelayThread(10000);
				break;
			case 4:
				DelayThread(100000);
				break;
			default:
				DelayThread(1000000);
				break;
		}
	}
	VERBOSE_KPRINTF(1, "DEV5 ATA: error: wait busy, timedout.\n", i / 0xA);
	return -502;
}

static int ata_wait_busy2_busy(void)
{
	unsigned int i;
	USE_DEV5_SPEED_REGS();

	for ( i = 0; i < 55; i += 1 )
	{
		if ( !(dev5_speed_regs->r_spd_ata_control & 0x80) )
		{
			return 0;
		}
		if ( !(u16)expbay_get_has_power() )
		{
			VERBOSE_KPRINTF(1, "DEV5 ATA: error: wait busy, power off.\n");
			return -551;
		}
		if ( g_is_wait_busy )
		{
			VERBOSE_KPRINTF(1, "DEV5 ATA: error: wait busy, tray eject.\n");
			return -550;
		}
		switch ( i / 0xA )
		{
			case 0:
				break;
			case 1:
				DelayThread(100);
				break;
			case 2:
				DelayThread(1000);
				break;
			case 3:
				DelayThread(10000);
				break;
			case 4:
				DelayThread(100000);
				break;
			default:
				DelayThread(1000000);
				break;
		}
	}
	VERBOSE_KPRINTF(1, "DEV5 ATA: error: wait busy, timedout.\n", i / 0xA);
	return -502;
}

static int ata_wait_bus_busy_busbusy(void)
{
	unsigned int i;
	USE_DEV5_SPEED_REGS();

	for ( i = 0; i < 80; i += 1 )
	{
		if ( !(dev5_speed_regs->r_spd_ata_control & 0x88) )
		{
			return 0;
		}
		if ( !(u16)expbay_get_has_power() )
		{
			VERBOSE_KPRINTF(1, "DEV5 ATA: error: wait busy, power off.\n");
			return -551;
		}
		if ( g_is_wait_busy )
		{
			VERBOSE_KPRINTF(1, "DEV5 ATA: error: wait busbusy, tray eject.\n");
			return -550;
		}
		switch ( i / 0xA )
		{
			case 0:
				break;
			case 1:
				DelayThread(100);
				break;
			case 2:
				DelayThread(1000);
				break;
			case 3:
				DelayThread(10000);
				break;
			case 4:
				DelayThread(100000);
				break;
			default:
				DelayThread(1000000);
				break;
		}
	}
	VERBOSE_KPRINTF(1, "DEV5 ATA: error: wait busbusy, timedout.\n", i / 0xA);
	return -502;
}

static int ata_device_select(int device)
{
	int result;
	USE_DEV5_SPEED_REGS();

	result = ata_wait_bus_busy_busbusy();
	if ( result < 0 )
	{
		return result;
	}
	if ( ((dev5_speed_regs->r_spd_ata_select >> 4) & 1) == device )
	{
		return 0;
	}
	dev5_speed_regs->r_spd_ata_select = (device ? 1 : 0) << 4;
	return ata_wait_bus_busy_busbusy();
}

static int sceAtaExecCmd(
	void *buf, u32 blkcount, u16 feature, u16 nsector, u16 sector, u16 lcyl, u16 hcyl, u16 select, u16 command, u32 unk10)
{
	int result;
	int using_timeout;
	iop_sys_clock_t sysclk;
	USE_DEV5_SPEED_REGS();

	ClearEventFlag(g_atapi_event_flag, 0);
	g_is_wait_busy = 0;
	if ( !atad_devinfo[(select & 0x10) ? 1 : 0].exists )
		return -505;
	result = ata_device_select((select >> 4) & 1);
	if ( result )
	{
		return result;
	}
	if ( command == 142 || command == 176 )
	{
		VERBOSE_KPRINTF(1, "Not support Ata CMD\n");
		return -503;
	}
	atad_cmd_state.type = unk10;
	if ( !unk10 )
		return -506;
	atad_cmd_state.buf = buf;
	atad_cmd_state.blkcount = blkcount;
	if (
		!(dev5_speed_regs->r_spd_ata_control & 0x40)
		&& ((command < 0x90 && command != 8) || (command >= 0xA2 || command < 0xA0)) )
	{
		VERBOSE_KPRINTF(1, "DEV5 ATA: error: device not ready\n");
		return -501;
	}
	switch ( atad_cmd_state.type )
	{
		case 1:
		case 8:
			using_timeout = 1;
			break;
		case 4:
			atad_cmd_state.dir = command != 0xC8;
			using_timeout = 1;
			break;
		case 5:
		case 6:
			using_timeout = 1;
			atad_cmd_state.dir = atad_cmd_state.type != 5;
			break;
		default:
			using_timeout = 0;
			break;
	}
	if ( atad_cmd_state.type != 9 )
	{
		if ( using_timeout )
		{
			USec2SysClock((command != 142 || feature != 244) ? 155000000 : 180000000, &sysclk);
			result = SetAlarm(&sysclk, AtaAlarmrHandle, 0);
			if ( result < 0 )
				return result;
		}
		if ( atad_cmd_state.type == 1 )
			speedIntrEnable(1);
		dev5_speed_regs->r_spd_ata_control = (!using_timeout) << 1;
	}
	dev5_speed_regs->r_spd_ata_error = feature;
	dev5_speed_regs->r_spd_ata_nsector = nsector;
	dev5_speed_regs->r_spd_ata_sector = sector;
	dev5_speed_regs->r_spd_ata_lcyl = lcyl;
	dev5_speed_regs->r_spd_ata_hcyl = hcyl;
	dev5_speed_regs->r_spd_ata_select = select | 0x40;
	dev5_speed_regs->r_spd_ata_status = command;
	speedLEDCtl(1);
	return 0;
}

int xatapi_5_sceAtaExecCmd(
	void *buf, u32 blkcount, u16 feature, u16 nsector, u16 sector, u16 lcyl, u16 hcyl, u16 select, u16 command, u32 unk10)
{
	int retres;
	u32 efbits;

	WaitEventFlag(g_acmd_evfid, 1, WEF_AND | WEF_CLEAR, &efbits);
	retres = sceAtaExecCmd(buf, blkcount, feature, nsector, sector, lcyl, hcyl, select, command, unk10);
	if ( retres )
	{
		SetEventFlag(g_acmd_evfid, 1);
	}
	return retres;
}

static int sceCdAtapiExecCmd_local(s16 n, void *buf, int nsec, int secsize, void *pkt, unsigned int pkt_len, int proto)
{
	u16 feature_tmp;
	int result;
	int using_timeout;
	int retres1;
	char ata_status_1;
	unsigned int i;
	iop_sys_clock_t sysclk;
	USE_DEV5_SPEED_REGS();

	feature_tmp = 0;
	VERBOSE_KPRINTF(1, "sceCdAtapiExecCmd Start. pkt_len %d proto %d\n", pkt_len, proto);
	do_hex_dump(pkt, 12);
	g_is_wait_busy = 0;
	if ( !proto )
		return -506;
	atad_cmd_state.type_atapi = proto;
	atad_cmd_state.buf_atapi = buf;
	atad_cmd_state.blkcount_atapi = nsec;
	atad_cmd_state.blksize_atapi = secsize;
	switch ( proto )
	{
		case 1:
		case 8:
			using_timeout = 1;
			break;
		case 4:
			switch ( *(u8 *)pkt )
			{
				case 0x03:
				case 0x12:
				case 0x23:
				case 0x25:
				case 0x28:
				case 0x3C:
				case 0x46:
				case 0x4A:
				case 0x5A:
				case 0xA4:
				case 0xA8:
				case 0xB9:
				case 0xDA:
					atad_cmd_state.dir_atapi = 0;
					break;
				default:
					atad_cmd_state.dir_atapi = 1;
					break;
			}
			using_timeout = 1;
			feature_tmp = 1;
			break;
		case 5:
		case 6:
			using_timeout = 1;
			feature_tmp = 1;
			atad_cmd_state.dir_atapi = atad_cmd_state.type_atapi != 5;
			break;
		default:
			using_timeout = 0;
			break;
	}
	if (
		!g_reset_scrambling_pack && atad_cmd_state.type_atapi == 4 && atad_cmd_state.dir_atapi == 1
		&& Mpeg2CheckScramble(
			(char *)atad_cmd_state.buf_atapi, atad_cmd_state.blkcount_atapi * atad_cmd_state.blksize_atapi) )
	{
		VERBOSE_KPRINTF(0, "illegal stream\n");
		return -560;
	}
	if ( using_timeout )
	{
		USec2SysClock(0x93D1CC0, &sysclk);
		result = SetAlarm(&sysclk, AtaAlarmrHandle, 0);
		if ( result < 0 )
			return result;
	}
	if ( atad_cmd_state.type_atapi == 1 )
		speedIntrEnable(1);
	dev5_speed_regs->r_spd_ata_control = (!using_timeout) << 1;
	result = sceAtaExecCmd(
		0,
		0,
		feature_tmp,
		0,
		0,
		(u8)((atad_cmd_state.blkcount_atapi & 0xFF) * (atad_cmd_state.blksize_atapi & 0xFF)),
		(u8)((u16)((atad_cmd_state.blkcount_atapi & 0xFFFF) * (atad_cmd_state.blksize_atapi & 0xFFFF)) >> 8),
		n << 4,
		0xA0,
		9);
	retres1 = result;
	if ( retres1 )
	{
		if ( using_timeout )
		{
			CancelAlarm(AtaAlarmrHandle, 0);
		}
		return retres1;
	}
	ata_status_1 = 0;
	while ( (ata_status_1 & 0x88) != 8 )
	{
		int ata_status_2;

		DelayThread(10000);
		ata_status_1 = dev5_speed_regs->r_spd_ata_status;
		ata_status_2 = dev5_speed_regs->r_spd_ata_status;
		VERBOSE_KPRINTF(1, "Status 0x%02x BSY %x DRQ %x\n", ata_status_2, ata_status_1 & 0x80, ata_status_1 & 8);
		if ( g_is_wait_busy )
		{
			if ( using_timeout )
				CancelAlarm(AtaAlarmrHandle, 0);
			VERBOSE_KPRINTF(1, "sceCdAtapiExecCmd Tray Eject while\n", ata_status_2);
			return -550;
		}
		if ( !(u16)expbay_get_has_power() )
		{
			if ( using_timeout )
			{
				CancelAlarm(AtaAlarmrHandle, 0);
			}
			return -551;
		}
	}
	if ( (ata_status_1 & 1) )
	{
		VERBOSE_KPRINTF(1, "iocmd err 0x%02x, 0x%02x\n", ata_status_1, sceAtaGetError());
		if ( using_timeout )
		{
			CancelAlarm(AtaAlarmrHandle, 0);
		}
		return -503;
	}
	if ( !(ata_status_1 & 8) )
	{
		VERBOSE_KPRINTF(1, "sceCdAtapiExecCmd_local ATA_NO_DREQ\n");
	}
	for ( i = 0; i < (pkt_len >> 1); i += 1 )
	{
		VERBOSE_KPRINTF(1, "sceCdAtapiExecCmd_local Packet %04x\n", ((u16 *)pkt)[i]);
		dev5_speed_regs->r_spd_ata_data = ((u16 *)pkt)[i];
	}
	VERBOSE_KPRINTF(1, "sceCdAtapiExecCmd End. cmd %02x\n", *(u8 *)pkt);
	if ( g_is_wait_busy )
	{
		if ( using_timeout )
			CancelAlarm(AtaAlarmrHandle, 0);
		VERBOSE_KPRINTF(1, "sceCdAtapiExecCmd Tray Eject last\n", pkt_len >> 1);
		return -550;
	}
	VERBOSE_KPRINTF(1, "sceCdAtapiExecCmd OK\n");
	return 0;
}

static int sceCdAtapiExecCmd(s16 n, void *buf, int nsec, int secsize, void *pkt, int pkt_len, int proto)
{
	if ( pkt_len )
	{
		int pkt_scsi_cmd_2;

		pkt_scsi_cmd_2 = *(u8 *)pkt;
		VERBOSE_KPRINTF(1, "sceCdAtapiExecCmd %08x\n", pkt_scsi_cmd_2);
		if (
			(
				!(pkt_scsi_cmd_2 == 0x1B || pkt_scsi_cmd_2 == 0x12 || !pkt_scsi_cmd_2 || pkt_scsi_cmd_2 == 3
					|| *g_cd_sc_ffffffd9_ptr))
			&& !g_is_in_read_info && !sceCdAtapi_BC() )
		{
			ata_device_set_transfer_mode_outer(0);
		}
	}
	return sceCdAtapiExecCmd_local(n, buf, nsec, secsize, pkt, pkt_len, proto);
}

int xatapi_7_sceCdAtapiExecCmd(s16 n, void *buf, int nsec, int secsize, void *pkt, int pkt_len, int proto)
{
	int retres;
	u32 efbits;

	WaitEventFlag(g_acmd_evfid, 1, WEF_AND | WEF_CLEAR, &efbits);
	retres = sceCdAtapiExecCmd(n, buf, nsec, secsize, pkt, pkt_len, proto);
	if ( retres )
	{
		SetEventFlag(g_acmd_evfid, 1);
	}
	return retres;
}

static int ata_pio_transfer(ata_cmd_state_t *cmd_state)
{
	char r_spd_ata_status;
	USE_DEV5_SPEED_REGS();

	r_spd_ata_status = dev5_speed_regs->r_spd_ata_status;
	if ( (r_spd_ata_status & 1) )
	{
		VERBOSE_KPRINTF(1, "DEV5 ATA: error: ATA PIO iocmd err 0x%02x, 0x%02x\n", r_spd_ata_status, sceAtaGetError());
		return -503;
	}
	else if ( (r_spd_ata_status & 8) )
	{
		int i;

		switch ( cmd_state->type )
		{
			case 2:
			{
				for ( i = 0; i < 256; i += 1 )
				{
					cmd_state->buf16[i] = dev5_speed_regs->r_spd_ata_data;
				}
				cmd_state->buf16 += 256;
				break;
			}
			case 3:
			{
				for ( i = 0; i < 256; i += 1 )
				{
					dev5_speed_regs->r_spd_ata_data = cmd_state->buf16[i];
				}
				cmd_state->buf16 += 256;
				break;
			}
			case 11:
			{
				for ( i = 0; i < 256; i += 1 )
				{
					dev5_speed_regs->r_spd_ata_data = cmd_state->buf16[i];
				}
				cmd_state->buf16 += 256;
				for ( i = 0; i < 4; i += 1 )
				{
					dev5_speed_regs->r_spd_ata_data = cmd_state->buf8[512 + i];
				}
				cmd_state->buf8 += 4;
				break;
			}
			default:
				break;
		}
	}
	else
	{
		return -504;
	}
	return 0;
}

static int IoRun_atapi(ata_cmd_state_t *cmd_state)
{
	int result;
	u32 blktotal;
	unsigned int lhcyl;
	unsigned int i;
	USE_DEV5_SPEED_REGS();

	VERBOSE_KPRINTF(1, "Pio trans %d\n", cmd_state->blkcount_atapi * cmd_state->blksize_atapi);
	result = ata_wait_busy1_busy();
	if ( result < 0 )
		return result;
	result = 0;
	for ( blktotal = cmd_state->blkcount_atapi * cmd_state->blksize_atapi; result >= 0 && blktotal; blktotal -= lhcyl )
	{
		char r_spd_ata_status;

		r_spd_ata_status = dev5_speed_regs->r_spd_ata_status;
		if ( (r_spd_ata_status & 1) )
		{
			VERBOSE_KPRINTF(1, "DEV5 ATA: error: ATAPI PIO iocmd err 0x%02x, 0x%02x\n", r_spd_ata_status, sceAtaGetError());
			return -503;
		}
		if ( !(r_spd_ata_status & 8) )
			return -504;
		// Unofficial: was 8 bit access
		lhcyl = (dev5_speed_regs->r_spd_ata_lcyl & 0xFF) | ((dev5_speed_regs->r_spd_ata_hcyl & 0xFF) << 8);
		VERBOSE_KPRINTF(1, "ByteCount Trans byte %04x\n", lhcyl);
		switch ( cmd_state->type_atapi )
		{
			case 2:
			{
				VERBOSE_KPRINTF(1, "IoRun_atapi input trans %d\n", cmd_state->blksize_atapi);
				for ( i = 0; i < (lhcyl >> 1); i += 1 )
				{
					((u16 *)((char *)cmd_state->buf_atapi))[i] = dev5_speed_regs->r_spd_ata_data;
				}
				if ( (lhcyl & 1) )
					*((u8 *)cmd_state->buf_atapi + 2 * i) = dev5_speed_regs->r_spd_ata_data;
				cmd_state->buf_atapi = (char *)cmd_state->buf_atapi + lhcyl;
				break;
			}
			case 3:
			{
				VERBOSE_KPRINTF(1, "IoRun_atapi output trans %d\n", cmd_state->blksize_atapi);
				for ( i = 0; i < (lhcyl >> 1); i += 1 )
				{
					dev5_speed_regs->r_spd_ata_data = ((u16 *)cmd_state->buf_atapi)[i];
				}
				if ( (lhcyl & 1) )
					dev5_speed_regs->r_spd_ata_data = *((u8 *)cmd_state->buf_atapi + 2 * i);
				cmd_state->buf_atapi = (char *)cmd_state->buf_atapi + lhcyl;
				break;
			}
			default:
				break;
		}
		VERBOSE_KPRINTF(1, "IoRun_atapi trans End\n");
		result = ata_wait_busy1_busy();
	}
	return result;
}

static int atapi_transfer_wrapper(char *buf, unsigned int blkcount, int dir)
{
	unsigned int blkcount_tmp;
	int i;
	unsigned int dbuf_stat_mask;
	int result;
	u8 Error;
	char spd_ata_status_tmp;
	u32 efbits;
	int flg;
	USE_DEV5_SPEED_REGS();

	for ( blkcount_tmp = blkcount; blkcount_tmp; blkcount_tmp -= (flg ? dbuf_stat_mask : 0) )
	{
		dbuf_stat_mask = 0;
		for ( i = 0; i < 20 && !dbuf_stat_mask; i += 1 )
		{
			dbuf_stat_mask = dev5_speed_regs->r_spd_dbuf_stat & 0x1F;
		}
		VERBOSE_KPRINTF(1, "*SPD_RINTR_STAT %02x\n", dev5_speed_regs->r_spd_intr_stat);
		VERBOSE_KPRINTF(1, "*R_DBUF_STAT %02x\n", dev5_speed_regs->r_spd_dbuf_stat);
		flg = 1;
		if ( !dbuf_stat_mask )
		{
			speedIntrEnable(3);
			VERBOSE_KPRINTF(1, "Wait Event\n");
			WaitEventFlag(g_atapi_event_flag, 7, WEF_OR | WEF_CLEAR, &efbits);
			VERBOSE_KPRINTF(1, "Event come\n");
			if ( (efbits & 1) )
			{
				VERBOSE_KPRINTF(1, "DEV5 ATA: error: DmaRun, ata timedout\n");
				return -502;
			}
			if ( (efbits & 4) )
			{
				VERBOSE_KPRINTF(1, "DEV5 ATA: error: DmaRun, Media Eject\n");
				return -550;
			}
			if ( !(dev5_speed_regs->r_spd_intr_stat & 2) )
			{
				if ( (dev5_speed_regs->r_spd_ata_control & 1) )
				{
					spd_ata_status_tmp = dev5_speed_regs->r_spd_ata_status;
					Error = sceAtaGetError();
					VERBOSE_KPRINTF(1, "DEV5 ATA: error: cmd err 0x%02x, 0x%02x, while DmaRun\n", spd_ata_status_tmp, Error);
					return (!(Error & 0x80)) ? -503 : -510;
				}
				VERBOSE_KPRINTF(1, "DEV5 ATA: warning: ata intr without error.\n");
				flg = 0;
			}
			else
			{
				dbuf_stat_mask = dev5_speed_regs->r_spd_dbuf_stat & 0x1F;
			}
		}
		if ( flg )
		{
			if ( blkcount_tmp < dbuf_stat_mask )
				dbuf_stat_mask = blkcount_tmp;
			result = SpdDmaTransfer(0, buf, (dbuf_stat_mask << 18) | 0x20, dir);
			buf += dbuf_stat_mask << 9;
			if ( result < 0 )
				return result;
		}
	}
	return 0;
}

static int DmaRun_atapi(char *buf, int blkcount, int blksize, int dir)
{
	unsigned int blkremainder;
	unsigned int blksectors;
	int i;
	unsigned int dbuf_stat_mask;
	int result;
	u8 Error;
	char spd_ata_status_tmp;
	unsigned int dbuf_stat_sectors;
	u32 efbits;
	USE_DEV5_SPEED_REGS();

	VERBOSE_KPRINTF(1, "DmaRun_atapi start\n");
	blkremainder = (blkcount * blksize) & 0x1FF;
	for ( blksectors = (unsigned int)(blkcount * blksize) >> 9; blksectors; blksectors -= dbuf_stat_mask )
	{
		dbuf_stat_mask = 0;
		for ( i = 0; i < 20 && !dbuf_stat_mask; i += 1 )
		{
			dbuf_stat_mask = dev5_speed_regs->r_spd_dbuf_stat & 0x1F;
		}
		if ( !dbuf_stat_mask )
		{
			speedIntrEnable(3);
			WaitEventFlag(g_atapi_event_flag, 7, WEF_OR | WEF_CLEAR, &efbits);
			if ( (efbits & 1) )
			{
				VERBOSE_KPRINTF(1, "DEV5 ATA: error: DmaRun, ata timedout\n");
				return -502;
			}
			if ( (efbits & 4) )
			{
				VERBOSE_KPRINTF(1, "DEV5 ATA: error: DmaRun, Media Eject\n");
				return -550;
			}
			if ( (dev5_speed_regs->r_spd_intr_stat & 2) )
			{
				dbuf_stat_mask = dev5_speed_regs->r_spd_dbuf_stat & 0x1F;
			}
			else
			{
				if ( (dev5_speed_regs->r_spd_ata_control & 1) )
				{
					spd_ata_status_tmp = dev5_speed_regs->r_spd_ata_status;
					Error = sceAtaGetError();
					VERBOSE_KPRINTF(1, "DEV5 ATA: error: cmd err 0x%02x, 0x%02x, while DmaRun\n", spd_ata_status_tmp, Error);
					return (!(Error & 0x80)) ? -503 : -510;
				}
				VERBOSE_KPRINTF(1, "DEV5 ATA: warning: ata intr without error.\n");
				continue;
			}
		}
		if ( blksectors < dbuf_stat_mask )
		{
			dbuf_stat_mask = blksectors;
		}
		dbuf_stat_sectors = dbuf_stat_mask << 9;
		VERBOSE_KPRINTF(
			1,
			"DmaRun_atapi  cnt %d nblk %d secsize %d bcr %08x\n",
			dbuf_stat_mask,
			blksectors,
			blksize,
			(dbuf_stat_sectors << 9) | 0x20);
		result = SpdDmaTransfer(0, buf, (dbuf_stat_sectors << 9) | 0x20, dir);
		buf += dbuf_stat_sectors;
		if ( result < 0 )
			return result;
	}
	if ( blkremainder )
	{
		while ( !(dev5_speed_regs->r_spd_intr_stat & 1) )
			;
		dev5_speed_regs->m_spd_unk36 += 512;
		VERBOSE_KPRINTF(1, "SpdDmaTransfer buf:%08x bcr:%d dir:%d\n", buf, 0x40020, dir);
		if ( dir )
		{
			memcpy(g_atapi_xfer_buf, buf, blkremainder);
			result = SpdDmaTransfer(0, g_atapi_xfer_buf, 0x40020, dir);
			if ( result < 0 )
				return result;
		}
		else
		{
			result = SpdDmaTransfer(0, g_atapi_xfer_buf, 0x40020, 0);
			if ( result < 0 )
				return result;
			memcpy(buf, g_atapi_xfer_buf, blkremainder);
		}
	}
	VERBOSE_KPRINTF(1, "DmaRun_atapi End.\n");
	return 0;
}

static int DmaRun_atapi_extrans1(char *buf, int blkcount, int blksize, int dir)
{
	unsigned int blksectors;
	int blkremainder;
	int i;
	unsigned int dbuf_stat_mask;
	int result;
	u8 Error;
	char spd_ata_status_tmp;
	unsigned int dbuf_stat_sectors;
	u32 efbits;
	USE_DEV5_SPEED_REGS();

	VERBOSE_KPRINTF(1, "DmaRun_atapi_extrans start\n");
	FpgaLayer2Off();
	FpgaClearBuffer();
	FpgaXfdir(dir);
	blkremainder = (blkcount * blksize) & 0x1FF;
	for ( blksectors = (unsigned int)(blkcount * blksize) >> 9; blksectors; blksectors -= dbuf_stat_mask )
	{
		dbuf_stat_mask = 0;
		for ( i = 0; i < 20 && !dbuf_stat_mask; i += 1 )
		{
			dbuf_stat_mask = dev5_speed_regs->r_spd_dbuf_stat & 0x1F;
		}
		if ( !dbuf_stat_mask )
		{
			speedIntrEnable(3);
			WaitEventFlag(g_atapi_event_flag, 7, WEF_OR | WEF_CLEAR, &efbits);
			if ( (efbits & 1) )
			{
				VERBOSE_KPRINTF(1, "DEV5 ATA: error: DmaRun_atapi_extrans, ata timedout\n");
				return -502;
			}
			if ( (efbits & 4) )
			{
				VERBOSE_KPRINTF(1, "DEV5 ATA: error: DmaRun_atapi_extrans, Media Eject\n");
				return -550;
			}
			if ( (dev5_speed_regs->r_spd_intr_stat & 2) )
			{
				dbuf_stat_mask = dev5_speed_regs->r_spd_dbuf_stat & 0x1F;
			}
			else
			{
				if ( (dev5_speed_regs->r_spd_ata_control & 1) )
				{
					spd_ata_status_tmp = dev5_speed_regs->r_spd_ata_status;
					Error = sceAtaGetError();
					VERBOSE_KPRINTF(1, "DEV5 ATA: error: cmd err 0x%02x, 0x%02x, while DmaRun\n", spd_ata_status_tmp, Error);
					return (!(Error & 0x80)) ? -503 : -510;
				}
				VERBOSE_KPRINTF(1, "DEV5 ATA: warning: ata intr without error.\n");
				continue;
			}
		}
		if ( blksectors < dbuf_stat_mask )
		{
			dbuf_stat_mask = blksectors;
		}
		dbuf_stat_sectors = dbuf_stat_mask << 9;
		VERBOSE_KPRINTF(
			1,
			"DmaRun_atapi_extrans  cnt %d nblk %d secsize %d bcr %08x\n",
			dbuf_stat_mask,
			blksectors,
			blksize,
			(dbuf_stat_sectors << 9) | 0x20);
		result = SpdDmaTransfer_extrans_1(0, buf, (dbuf_stat_sectors << 9) | 0x20, dir);
		buf += dbuf_stat_sectors;
		if ( result < 0 )
			return result;
	}
	if ( blkremainder )
	{
		while ( !(dev5_speed_regs->r_spd_intr_stat & 1) )
			;
		dev5_speed_regs->m_spd_unk36 += 512;
		VERBOSE_KPRINTF(1, "SpdDmaTransfer buf:%08x bcr:%d dir:%d\n", buf, 0x40020, dir);
		if ( dir )
		{
			memcpy(g_atapi_xfer_buf, buf, blkremainder);
			result = SpdDmaTransfer_extrans_1(0, g_atapi_xfer_buf, 0x40020, dir);
			if ( result < 0 )
				return result;
		}
		else
		{
			result = SpdDmaTransfer_extrans_1(0, g_atapi_xfer_buf, 0x40020, 0);
			if ( result < 0 )
				return result;
			memcpy(buf, g_atapi_xfer_buf, blkremainder);
		}
	}
	VERBOSE_KPRINTF(1, "DmaRun_atapi_extrans End.\n");
	return 0;
}

static int DmaRun_atapi_extrans2(char *buf, int blkcount, int blksize, int dir)
{
	int result;
	unsigned int blksectors;
	int blkremainder;
	int i;
	unsigned int fpga_spckcnt;
	u8 Error;
	int extransres;
	iop_sys_clock_t sysclk;
	u32 efbits;
	USE_DEV5_SPEED_REGS();

	VERBOSE_KPRINTF(1, "DmaRun_atapi_extrans start\n");
	FpgaLayer2Off();
	FpgaClearBuffer();
	FpgaXfdir(dir);
	CancelAlarm(AtaAlarmrHandle, 0);
	USec2SysClock(0x989680, &sysclk);
	result = SetAlarm(&sysclk, AtaAlarmrHandle, 0);
	if ( result < 0 )
	{
		return result;
	}
	while ( 1 )
	{
		speedIntrEnable(3);
		WaitEventFlag(g_atapi_event_flag, 7, WEF_OR | WEF_CLEAR, &efbits);
		if ( (efbits & 1) )
		{
			VERBOSE_KPRINTF(1, "DEV5 ATA: error: DmaRun_atapi_extrans, ata timedout\n");
			return -502;
		}
		if ( (efbits & 4) )
		{
			VERBOSE_KPRINTF(1, "DEV5 ATA: error: DmaRun_atapi_extrans, Media Eject\n");
			return -550;
		}
		if ( (dev5_speed_regs->r_spd_intr_stat & 2) )
		{
			break;
		}
		if ( (dev5_speed_regs->r_spd_ata_control & 1) )
		{
			Error = sceAtaGetError();
			VERBOSE_KPRINTF(
				1, "DEV5 ATA: error: cmd err 0x%02x, 0x%02x, while DmaRun\n", dev5_speed_regs->r_spd_ata_status, Error);
			return (!(Error & 0x80)) ? -503 : -510;
		}
		VERBOSE_KPRINTF(1, "DEV5 ATA: warning: ata intr without error.\n");
	}
	dev5_speed_regs->r_spd_dma_ctrl = (dev5_speed_regs->r_spd_rev_1 >= 0x11) ? 6 : 4;
	ata_pre_dma_cb();
	FpgaLayer1On();
	FpgaXfrenOn();
	blkremainder = (blkcount * blksize) & 0x1FF;
	extransres = 0;
	for ( blksectors = (unsigned int)(blkcount * blksize) >> 9; blksectors; blksectors -= fpga_spckcnt )
	{
		unsigned int fpga_spckcnt_bytes;

		for ( i = 0;; i += 1 )
		{
			fpga_spckcnt = do_fpga_check_spckcnt();
			if ( fpga_spckcnt >= 3 || fpga_spckcnt >= blksectors )
				break;
			switch ( i / 500 )
			{
				case 0:
					break;
				case 1:
					DelayThread(100);
					break;
				default:
					DelayThread(10000);
					break;
			}
			PollEventFlag(g_atapi_event_flag, 5, WEF_OR | WEF_CLEAR, &efbits);
			if ( (efbits & 1) )
			{
				VERBOSE_KPRINTF(1, "DEV5 ATA: error: DmaRun_atapi_extrans, ata timedout\n");
				FpgaXfrenOff();
				FpgaLayer1Off();
				ata_post_dma_cb();
				if ( !(dev5_speed_regs->r_spd_intr_stat & 2) && (dev5_speed_regs->r_spd_ata_control & 1) )
				{
					Error = sceAtaGetError();
					VERBOSE_KPRINTF(
						1, "DEV5 ATA: error: cmd err 0x%02x, 0x%02x, while DmaRun\n", dev5_speed_regs->r_spd_ata_status, Error);
					return (!(Error & 0x80)) ? -503 : -510;
				}
				return -502;
			}
			if ( (efbits & 4) )
			{
				VERBOSE_KPRINTF(1, "DEV5 ATA: error: DmaRun_atapi_extrans, Media Eject\n");
				FpgaXfrenOff();
				FpgaLayer1Off();
				ata_post_dma_cb();
				return -550;
			}
		}
		if ( blksectors < fpga_spckcnt )
		{
			fpga_spckcnt = blksectors;
		}
		fpga_spckcnt_bytes = fpga_spckcnt << 9;
		VERBOSE_KPRINTF(
			1,
			"DmaRun_atapi_extrans  cnt %d nblk %d secsize %d bcr %08x\n",
			fpga_spckcnt,
			blksectors,
			blksize,
			(fpga_spckcnt_bytes << 9) | 0x20);
		extransres = SpdDmaTransfer_extrans_2(0, buf, (fpga_spckcnt_bytes << 9) | 0x20, dir);
		if ( extransres < 0 )
			break;
		buf += fpga_spckcnt_bytes;
	}
	FpgaXfrenOff();
	FpgaLayer1Off();
	ata_post_dma_cb();
	if ( extransres >= 0 && blkremainder )
	{
		while ( !(dev5_speed_regs->r_spd_intr_stat & 1) )
			;
		dev5_speed_regs->m_spd_unk36 += 512;
		VERBOSE_KPRINTF(1, "SpdDmaTransfer buf:%08x bcr:%d dir:%d\n", buf, 0x40020, dir);
		ata_pre_dma_cb();
		FpgaLayer1On();
		FpgaXfrenOn();
		if ( dir )
		{
			memcpy(g_atapi_xfer_buf, buf, blkremainder);
			extransres = SpdDmaTransfer_extrans_2(0, g_atapi_xfer_buf, 0x40020, dir);
		}
		else
		{
			extransres = SpdDmaTransfer_extrans_2(0, g_atapi_xfer_buf, 0x40020, 0);
			if ( extransres >= 0 )
			{
				memcpy(buf, g_atapi_xfer_buf, blkremainder);
			}
		}
		FpgaXfrenOff();
		FpgaLayer1Off();
		ata_post_dma_cb();
	}
	if ( extransres < 0 )
	{
		return extransres;
	}
	VERBOSE_KPRINTF(1, "DmaRun_atapi_extrans End.\n");
	return 0;
}

static void DmaRun_spck(char *buf, unsigned int secsize)
{
	unsigned int secsize_sectors;
	unsigned int fpga_spckcnt;
	USE_DEV5_SPEED_REGS();

	VERBOSE_KPRINTF(1, "DmaRun_spck start\n");
	FpgaSpckmodeOn();
	FpgaLayer2Off();
	FpgaClearBuffer();
	FpgaXfdir(0);
	for ( secsize_sectors = secsize >> 9; secsize_sectors; secsize_sectors -= fpga_spckcnt )
	{
		unsigned int fpga_spckcnt_bytes;

		for ( fpga_spckcnt = 0; fpga_spckcnt < 4; fpga_spckcnt = do_fpga_check_spckcnt() )
			;
		if ( secsize_sectors < fpga_spckcnt )
		{
			fpga_spckcnt = secsize_sectors;
		}
		fpga_spckcnt_bytes = fpga_spckcnt << 9;
		VERBOSE_KPRINTF(
			1,
			"DmaRun_spck  cnt %d nblk %d secsize %d bcr %08x\n",
			fpga_spckcnt,
			secsize_sectors,
			secsize,
			(fpga_spckcnt_bytes << 9) | 0x20);
		if ( SpdDmaTransfer_extrans_3(0, buf, (fpga_spckcnt_bytes << 9) | 0x20, 1) < 0 )
		{
			FpgaSpckmodeOff();
			return;
		}
		buf += fpga_spckcnt_bytes;
	}
	if ( (secsize & 0x1FF) )
	{
		dev5_speed_regs->m_spd_unk36 += 512;
		VERBOSE_KPRINTF(1, "SpdDmaTransfer buf:%08x bcr:%d dir:%d\n", buf, 0x40020, 1);
		memcpy(g_atapi_xfer_buf, buf, secsize & 0x1FF);
		if ( SpdDmaTransfer_extrans_3(0, g_atapi_xfer_buf, 0x40020, 1) < 0 )
		{
			return;
		}
	}
	FpgaSpckmodeOff();
	VERBOSE_KPRINTF(1, "DmaRun_spck End.\n");
}

static int sceAtaWaitResult(void)
{
	int res;
	int i;
	int intr_stat_msk;
	u32 efbits;
	int suc;
	USE_DEV5_SPEED_REGS();

	suc = 0;
	res = 0;
	switch ( atad_cmd_state.type )
	{
		case 1:
		case 8:
			WaitEventFlag(g_atapi_event_flag, 7, WEF_OR | WEF_CLEAR, &efbits);
			if ( (efbits & 1) )
			{
				res = -502;
				VERBOSE_KPRINTF(1, "DEV5 ATA: error: ata timedout while non data command\n");
			}
			if ( (efbits & 4) )
			{
				res = -550;
				VERBOSE_KPRINTF(1, "DEV5 ATA: error: DmaRun, Media Eject\n");
			}
			suc = 1;
			break;
		case 4:
		case 5:
		case 6:
			res = atapi_transfer_wrapper((char *)atad_cmd_state.buf, atad_cmd_state.blkcount, atad_cmd_state.dir);
			if ( res )
			{
				break;
			}
			intr_stat_msk = 0;
			for ( i = 0; i < 100 && !intr_stat_msk; i += 1 )
			{
				intr_stat_msk = dev5_speed_regs->r_spd_intr_stat & 1;
			}
			if ( !intr_stat_msk )
			{
				speedIntrEnable(1);
				WaitEventFlag(g_atapi_event_flag, 7, WEF_OR | WEF_CLEAR, &efbits);
				if ( (efbits & 1) )
				{
					VERBOSE_KPRINTF(1, "DEV5 ATA: error: ata timedout, buffer stat %04x\n", dev5_speed_regs->r_spd_dbuf_stat);
					VERBOSE_KPRINTF(
						1,
						"DEV5 ATA: error: istat %x, ienable %x\n",
						dev5_speed_regs->r_spd_intr_stat,
						dev5_speed_regs->r_spd_intr_mask);
					res = -502;
				}
				if ( (efbits & 4) )
				{
					VERBOSE_KPRINTF(1, "DEV5 ATA: error: ata eject, buffer stat %04x\n", dev5_speed_regs->r_spd_dbuf_stat);
					VERBOSE_KPRINTF(
						1,
						"DEV5 ATA: error: istat %x, ienable %x\n",
						dev5_speed_regs->r_spd_intr_stat,
						dev5_speed_regs->r_spd_intr_mask);
					res = -550;
				}
			}
			suc = 1;
			break;
		default:
			while ( 1 )
			{
				res = ata_wait_busy1_busy();
				if ( res < 0 )
				{
					break;
				}
				atad_cmd_state.blkcount -= 1;
				if ( atad_cmd_state.blkcount == 0xFFFFFFFF )
				{
					suc = 1;
					break;
				}
				res = ata_pio_transfer(&atad_cmd_state);
				if ( res < 0 )
				{
					break;
				}
			}
			break;
	}
	if ( suc && !res )
	{
		char status_tmp;

		status_tmp = dev5_speed_regs->r_spd_ata_status;
		if ( (status_tmp & 0x80) )
		{
			res = ata_wait_busy1_busy();
			status_tmp = dev5_speed_regs->r_spd_ata_status;
		}
		if ( (status_tmp & 1) )
		{
			u8 Error;

			Error = sceAtaGetError();
			VERBOSE_KPRINTF(1, "DEV5 ATA: error: cmd err 0x%02x, 0x%02x\n", status_tmp, Error);
			res = (Error & 0x80) ? -510 : -503;
		}
	}
	CancelAlarm(AtaAlarmrHandle, 0);
	speedLEDCtl(0);
	if ( res )
	{
		VERBOSE_KPRINTF(1, "DEV5 ATA: error: ATA failed, %d\n", res);
	}
	return res;
}

int xatapi_6_sceAtaWaitResult(void)
{
	int restmp;
	iop_event_info_t efinfo;

	if ( vReferEventFlagStatus(g_acmd_evfid, &efinfo) || (efinfo.currBits & 1) )
	{
		VERBOSE_KPRINTF(1, "sceCdAtapiWaitResult Call Error\n");
		return -511;
	}
	restmp = sceAtaWaitResult();
	SetEventFlag(g_acmd_evfid, 1);
	return restmp;
}

static int sceCdAtapiWaitResult_local(void)
{
	int res;
	int i;
	int intr_stat_msk;
	u32 efbits;
	int padinfo;
	USE_DEV5_SPEED_REGS();

	res = 0;
	switch ( atad_cmd_state.type_atapi )
	{
		case 1:
		case 8:
			VERBOSE_KPRINTF(1, "waitresult\n");
			WaitEventFlag(g_atapi_event_flag, 7, WEF_OR | WEF_CLEAR, &efbits);
			if ( (efbits & 1) )
			{
				res = -502;
				VERBOSE_KPRINTF(1, "DEV5 ATA: error: ata timedout while non data command\n");
			}
			if ( (efbits & 4) )
			{
				res = -550;
				VERBOSE_KPRINTF(1, "DEV5 ATA: error: ata eject while non data command\n");
			}
			break;
		case 4:
		case 5:
		case 6:
			VERBOSE_KPRINTF(1, "waitresult dma\n");
			if ( !g_reset_scrambling_pack )
			{
				res = DmaRun_atapi(
					(char *)atad_cmd_state.buf_atapi,
					atad_cmd_state.blkcount_atapi,
					atad_cmd_state.blksize_atapi,
					atad_cmd_state.dir_atapi);
			}
			else if ( atad_cmd_state.dir_atapi )
			{
				int blktotal1;
				int blkoffs;

				blktotal1 = atad_cmd_state.blkcount_atapi * atad_cmd_state.blksize_atapi;
				for ( blkoffs = 0; blkoffs < blktotal1;
							blkoffs += atad_cmd_state.blkcount_atapi * atad_cmd_state.blksize_atapi )
				{
					int padres;

					padres = Mpeg2CheckPadding(
						(char *)atad_cmd_state.buf_atapi + blkoffs, blktotal1 - blkoffs, &padinfo, &g_pes_scrambling_control_pack);
					if ( padres < 0 )
					{
						res = DmaRun_atapi(
							(char *)atad_cmd_state.buf_atapi,
							atad_cmd_state.blkcount_atapi,
							atad_cmd_state.blksize_atapi,
							atad_cmd_state.dir_atapi);
						break;
					}
					atad_cmd_state.blksize_atapi = 2048;
					atad_cmd_state.blkcount_atapi = padinfo;
					res = padres ?
									DmaRun_atapi_extrans1(
										(char *)atad_cmd_state.buf_atapi + blkoffs, padinfo, 2048, atad_cmd_state.dir_atapi) :
									DmaRun_atapi((char *)atad_cmd_state.buf_atapi + blkoffs, padinfo, 2048, atad_cmd_state.dir_atapi);
				}
			}
			else
			{
				res = DmaRun_atapi_extrans2(
					(char *)atad_cmd_state.buf_atapi, atad_cmd_state.blkcount_atapi, atad_cmd_state.blksize_atapi, 0);
			}
			if ( res )
			{
				break;
			}
			intr_stat_msk = 0;
			for ( i = 0; i < 100 && !intr_stat_msk; i += 1 )
			{
				intr_stat_msk = dev5_speed_regs->r_spd_intr_stat & 1;
			}
			if ( intr_stat_msk )
			{
				break;
			}
			VERBOSE_KPRINTF(1, "ata command not finished yet\n");
			speedIntrEnable(1);
			WaitEventFlag(g_atapi_event_flag, 7, WEF_OR | WEF_CLEAR, &efbits);
			if ( (efbits & 1) )
			{
				VERBOSE_KPRINTF(1, "DEV5 ATA: error: ata timedout, buffer stat %04x\n", dev5_speed_regs->r_spd_dbuf_stat);
				VERBOSE_KPRINTF(
					1,
					"DEV5 ATA: error: istat %x, ienable %x\n",
					dev5_speed_regs->r_spd_intr_stat,
					dev5_speed_regs->r_spd_intr_mask);
				res = -502;
			}
			if ( (efbits & 4) )
			{
				VERBOSE_KPRINTF(1, "DEV5 ATA: error: ata eject, buffer stat %04x\n", dev5_speed_regs->r_spd_dbuf_stat);
				VERBOSE_KPRINTF(
					1,
					"DEV5 ATA: error: istat %x, ienable %x\n",
					dev5_speed_regs->r_spd_intr_stat,
					dev5_speed_regs->r_spd_intr_mask);
				res = -550;
			}
			break;
		default:
			res = IoRun_atapi(&atad_cmd_state);
			break;
	}
	if ( !res )
	{
		char ata_status_tmp;

		ata_status_tmp = dev5_speed_regs->r_spd_ata_status;
		if ( (ata_status_tmp & 0x80) )
		{
			res = ata_wait_busy1_busy();
			ata_status_tmp = dev5_speed_regs->r_spd_ata_status;
		}
		if ( (ata_status_tmp & 1) )
		{
			u8 Error;

			Error = sceAtaGetError();
			VERBOSE_KPRINTF(1, "DEV5 ATA: error: cmd status 0x%02x, error 0x%02x\n", ata_status_tmp, Error);
			res = (Error & 0x80) ? -510 : -503;
		}
	}
	CancelAlarm(AtaAlarmrHandle, 0);
	speedLEDCtl(0);
	VERBOSE_KPRINTF(1, "sceCdAtapiWaitResult_local() End.\n");
	if ( res )
	{
		VERBOSE_KPRINTF(1, "DEV5 ATA: error: ATA failed, %d\n", res);
	}
	return res;
}

int xatapi_8_sceCdAtapiWaitResult(void)
{
	int restmp;
	iop_event_info_t efinfo;

	if ( vReferEventFlagStatus(g_acmd_evfid, &efinfo) || (efinfo.currBits & 1) )
	{
		VERBOSE_KPRINTF(1, "sceCdAtapiWaitResult Call Error\n");
		return -511;
	}
	restmp = sceCdAtapiWaitResult_local();
	SetEventFlag(g_acmd_evfid, 1);
	return restmp;
}

static void ata_bus_reset_inner(void)
{
	USE_DEV5_SPEED_REGS();

	dev5_speed_regs->r_spd_if_ctrl = 128;
	DelayThread(100);
	dev5_speed_regs->r_spd_if_ctrl = 0;
	dev5_speed_regs->r_spd_if_ctrl = 72;
	DelayThread(3000);
	VERBOSE_KPRINTF(1, "hard reset\n");
}

static int ata_bus_reset(void)
{
	USE_DEV5_SPEED_REGS();

	if ( !(dev5_speed_regs->r_spd_if_ctrl & 0x40) )
		ata_bus_reset_inner();
	return ata_wait_busy2_busy();
}

int xatapi_4_sceAtaSoftReset(void)
{
	USE_DEV5_SPEED_REGS();

	if ( (dev5_speed_regs->r_spd_ata_control & 0x80) )
		return -501;
	dev5_speed_regs->r_spd_ata_control = 6;
	DelayThread(100);
	dev5_speed_regs->r_spd_ata_control = 2;
	DelayThread(3000);
	VERBOSE_KPRINTF(1, "soft reset\n");
	return ata_wait_busy1_busy();
}

#ifdef UNUSED_FUNC
static int sceAtaFlushCache(int device)
{
	int result;

	result = xatapi_5_sceAtaExecCmd(0, 1, 0, 0, 0, 0, 0, device << 4, 0xE7, 1);
	return result ? result : xatapi_6_sceAtaWaitResult();
}
#endif

static int ata_device_identify(int device, void *info)
{
	int result;

	result = xatapi_5_sceAtaExecCmd(info, 1, 0, 0, 0, 0, 0, device << 4, 0xEC, 2);
	return result ? result : xatapi_6_sceAtaWaitResult();
}

static int ata_device_pkt_identify(int device, void *info)
{
	int result;

	result = xatapi_5_sceAtaExecCmd(info, 1, 0, 0, 0, 0, 0, device << 4, 0xA1, 2);
	return result ? result : xatapi_6_sceAtaWaitResult();
}

static int atapi_device_set_transfer_mode(int device, int type, int mode)
{
	int result;

	result = xatapi_5_sceAtaExecCmd(0, 1, 3, (u8)(mode | type), 0, 0, 0, device << 4, 0xEF, 1);
	if ( result )
		return result;
	result = xatapi_6_sceAtaWaitResult();
	if ( result )
		return result;
	switch ( type )
	{
		case 8:
			ata_pio_mode(mode);
			break;
		case 0x20:
			ata_multiword_dma_mode(mode);
			break;
		case 0x40:
			ata_ultra_dma_mode(mode);
			break;
		default:
			break;
	}
	return 0;
}

static int ata_device_set_transfer_mode(int device, int type, int mode)
{
	int result;

	result = sceAtaExecCmd(0, 1, 3, (u8)(mode | type), 0, 0, 0, device << 4, 0xEF, 1);
	if ( result )
		return result;
	result = sceAtaWaitResult();
	if ( result )
		return result;
	switch ( type )
	{
		case 8:
			ata_pio_mode(mode);
			break;
		case 32:
			ata_multiword_dma_mode(mode);
			break;
		case 64:
			ata_ultra_dma_mode(mode);
			break;
		default:
			break;
	}
	return 0;
}

static void ata_device_probe(ata_devinfo_t *devinfo)
{
	char r_spd_ata_lcyl;
	u8 r_spd_ata_hcyl;
	USE_DEV5_SPEED_REGS();

	devinfo->exists = 0;
	devinfo->has_packet = 2;
	if ( (dev5_speed_regs->r_spd_ata_control & 0x88) )
	{
		VERBOSE_KPRINTF(1, "FindDev ATA_BUSY\n");
		return;
	}
	r_spd_ata_lcyl = dev5_speed_regs->r_spd_ata_lcyl;
	r_spd_ata_hcyl = dev5_speed_regs->r_spd_ata_hcyl;
	if ( dev5_speed_regs->r_spd_ata_nsector != 1 || dev5_speed_regs->r_spd_ata_sector != 1 )
	{
		VERBOSE_KPRINTF(1, "FindDev ATA_NOT_CONNECT\n");
		return;
	}
	devinfo->exists = 1;
	if ( !r_spd_ata_lcyl && !r_spd_ata_hcyl )
		devinfo->has_packet = 0;
	if ( r_spd_ata_lcyl == 20 && r_spd_ata_hcyl == 235 )
		devinfo->has_packet = 1;
	dev5_speed_regs->r_spd_ata_lcyl = 85;
	dev5_speed_regs->r_spd_ata_hcyl = 170;
	// Unofficial: was 8 bit access
	// cppcheck-suppress knownConditionTrueFalse
	if ( (dev5_speed_regs->r_spd_ata_lcyl & 0xFF) != 85 || (dev5_speed_regs->r_spd_ata_hcyl & 0xFF) != 170 )
		devinfo->exists = 0;
}

static void atapi_device_set_transfer_mode_outer(int device)
{
	if ( g_dma_mode_value )
	{
		VERBOSE_KPRINTF(0, "UDMA_mode Mode%d\n", 2);
		while ( atapi_device_set_transfer_mode(device, 64, (u8)g_dma_speed_value) < 0 )
		{
			DelayThread(2000000);
		}
	}
	else
	{
		VERBOSE_KPRINTF(0, "MDMA_mode Mode%d\n", 2);
		while ( atapi_device_set_transfer_mode(device, 32, (u8)g_dma_speed_value) < 0 )
		{
			DelayThread(2000000);
		}
	}
}

static void ata_device_set_transfer_mode_outer(int device)
{
	int i;

	if ( g_dma_mode_value )
	{
		VERBOSE_KPRINTF(0, "UDMA_mode Mode%d\n", 2);
		for ( i = 0; i < 3 && ata_device_set_transfer_mode(device, 64, (u8)g_dma_speed_value); i += 1 )
		{
			DelayThread(2000000);
		}
	}
	else
	{
		VERBOSE_KPRINTF(0, "MDMA_mode Mode%d\n", 2);
		for ( i = 0; i < 3 && ata_device_set_transfer_mode(device, 32, (u8)g_dma_speed_value); i += 1 )
		{
			DelayThread(2000000);
		}
	}
}

static void ata_init_devices(ata_devinfo_t *devinfo)
{
	int i;
	USE_DEV5_SPEED_REGS();

	if ( xatapi_4_sceAtaSoftReset() )
		return;
	ata_device_probe(&devinfo[0]);
	if ( !devinfo[0].exists )
	{
		VERBOSE_KPRINTF(1, "DEV5 ATA: error: there is no device0\n");
		devinfo[1].exists = 0;
		return;
	}
	if ( ata_device_select(1) )
		return;
	// Unofficial: was 8 bit access
	if ( (dev5_speed_regs->r_spd_ata_control & 0xFF) )
		ata_device_probe(&devinfo[1]);
	else
		devinfo[1].exists = 0;
	ata_pio_mode(0);
	g_is_in_read_info = 0;
	for ( i = 0; i < 2; i += 1 )
	{
		if ( !devinfo[i].exists )
		{
			continue;
		}
		if ( !devinfo[i].has_packet )
		{
			devinfo[i].exists = !ata_device_identify(i, ata_param);
		}
		if ( devinfo[i].has_packet == 1 )
			devinfo[i].exists = !ata_device_pkt_identify(i, ata_param);
		VERBOSE_KPRINTF(1, "device%d connected, kind %d.\n", i, devinfo[i].has_packet);
		if ( !devinfo[i].exists || devinfo[i].has_packet != 1 )
		{
			continue;
		}
		do_hex_dump(ata_param, sizeof(ata_param));
		devinfo[i].lba48 = !do_atapi_cmd_inquiry_12h(i);
		if ( devinfo[i].lba48 )
		{
			g_is_in_read_info = 1;
			VERBOSE_KPRINTF(0, "Atapi Program Aria Brokun.\n");
			continue;
		}
		atapi_device_set_transfer_mode_outer(i);
	}
}

static void sceAtapiInit(int device)
{
	int resetval;
	USE_DEV5_SPEED_REGS();

	(void)device;
	if ( g_ata_devinfo_init )
		return;
	g_ata_devinfo_init = 1;
	while ( 1 )
	{
		while ( 1 )
		{
			resetval = ata_bus_reset();
			if ( resetval != -550 )
				break;
			g_is_wait_busy = 0;
		}
		if ( !resetval )
			break;
		dev5_speed_regs->r_spd_if_ctrl = 0;
	}
	ata_init_devices(atad_devinfo);
}

static void sceAtInterInit(void)
{
	int sc_tmp;

	speedRegisterIntrCb(1, ata_intr_cb);
	speedRegisterIntrCb(0, ata_intr_cb);
	speedRegisterPreDmaCb(0, ata_pre_dma_cb);
	speedRegisterPostDmaCb(0, ata_post_dma_cb);
	if ( sceCdSC(0xFFFFFFDC, &sc_tmp) )
		speedIntrEnable(256);
}

static int create_event_flags(void)
{
	iop_event_t efparam;

	efparam.attr = EA_SINGLE;
	efparam.bits = 0;
	g_atapi_event_flag = CreateEventFlag(&efparam);
	if ( g_atapi_event_flag < 0 )
		return 1;
	efparam.attr = EA_MULTI;
	efparam.option = 0;
	efparam.bits = 1;
	g_acmd_evfid = CreateEventFlag(&efparam);
	if ( g_acmd_evfid >= 0 )
	{
		efparam.bits = 1;
		g_adma_evfid = CreateEventFlag(&efparam);
		if ( g_adma_evfid >= 0 )
		{
			efparam.bits = 1;
			g_io_event_flag = CreateEventFlag(&efparam);
			if ( g_io_event_flag >= 0 )
			{
				sceCdSC(0xFFFFFFE0, &g_acmd_evfid);
				sceCdSC(0xFFFFFFDF, &g_adma_evfid);
				return 1;
			}
		}
	}
	if ( g_atapi_event_flag >= 0 )
		DeleteEventFlag(g_atapi_event_flag);
	if ( g_acmd_evfid >= 0 )
		DeleteEventFlag(g_acmd_evfid);
	if ( g_adma_evfid >= 0 )
		DeleteEventFlag(g_adma_evfid);
	return 0;
}

static void FpgaLayer1On(void)
{
	USE_DEV5_FPGA_REGS();
	VERBOSE_KPRINTF(1, "%s():old:FPGA_LAYER1 %x\n", "FpgaLayer1On", dev5_fpga_regs->r_fpga_layer1);
	dev5_fpga_regs->r_fpga_layer1 &= ~1;
	dev5_fpga_regs->r_fpga_layer1 |= 1;
	VERBOSE_KPRINTF(1, "%s():new:FPGA_LAYER1 %x\n", "FpgaLayer1On", dev5_fpga_regs->r_fpga_layer1);
}

static void FpgaLayer1Off(void)
{
	USE_DEV5_FPGA_REGS();

	VERBOSE_KPRINTF(1, "%s():old:FPGA_LAYER1 %x\n", "FpgaLayer1Off", dev5_fpga_regs->r_fpga_layer1);
	dev5_fpga_regs->r_fpga_layer1 &= ~1;
	VERBOSE_KPRINTF(1, "%s():new:FPGA_LAYER1 %x\n", "FpgaLayer1Off", dev5_fpga_regs->r_fpga_layer1);
}

#ifdef UNUSED_FUNC
static void FpgaLayer2On(void)
{
	USE_DEV5_FPGA_REGS();

	VERBOSE_KPRINTF(1, "%s():old:FPGA_LAYER2 %x\n", "FpgaLayer2On", dev5_fpga_regs->r_fpga_layer2);
	dev5_fpga_regs->r_fpga_layer2 &= ~1;
	dev5_fpga_regs->r_fpga_layer2 |= 1;
	VERBOSE_KPRINTF(1, "%s():new:FPGA_LAYER2 %x\n", "FpgaLayer2On", dev5_fpga_regs->r_fpga_layer2);
}
#endif

static void FpgaLayer2Off(void)
{
	USE_DEV5_FPGA_REGS();

	VERBOSE_KPRINTF(1, "%s():old:FPGA_LAYER2 %x\n", "FpgaLayer2Off", dev5_fpga_regs->r_fpga_layer2);
	dev5_fpga_regs->r_fpga_layer2 &= ~1;
	VERBOSE_KPRINTF(1, "%s():new:FPGA_LAYER2 %x\n", "FpgaLayer2Off", dev5_fpga_regs->r_fpga_layer2);
}

static void FpgaXfrenOn(void)
{
	USE_DEV5_FPGA_REGS();

	VERBOSE_KPRINTF(1, "%s():old:FPGA_XFREN %x\n", "FpgaXfrenOn", dev5_fpga_regs->r_fpga_xfren);
	dev5_fpga_regs->r_fpga_xfren &= ~1;
	dev5_fpga_regs->r_fpga_xfren |= 1;
	VERBOSE_KPRINTF(1, "%s():new:FPGA_XFREN %x\n", "FpgaXfrenOn", dev5_fpga_regs->r_fpga_xfren);
}

static void FpgaXfrenOff(void)
{
	USE_DEV5_FPGA_REGS();

	VERBOSE_KPRINTF(1, "%s():old:FPGA_XFREN %x\n", "FpgaXfrenOff", dev5_fpga_regs->r_fpga_xfren);
	dev5_fpga_regs->r_fpga_xfren &= ~1;
	VERBOSE_KPRINTF(1, "%s():new:FPGA_XFREN %x\n", "FpgaXfrenOff", dev5_fpga_regs->r_fpga_xfren);
}

static void FpgaSpckmodeOn(void)
{
	USE_DEV5_FPGA_REGS();

	VERBOSE_KPRINTF(1, "%s():old:FPGA_SPCKMODE %x\n", "FpgaSpckmodeOn", dev5_fpga_regs->r_fpga_spckmode);
	dev5_fpga_regs->r_fpga_spckmode &= ~1;
	dev5_fpga_regs->r_fpga_spckmode |= 1;
	VERBOSE_KPRINTF(1, "%s():new:FPGA_SPCKMODE %x\n", "FpgaSpckmodeOn", dev5_fpga_regs->r_fpga_spckmode);
}

static void FpgaSpckmodeOff(void)
{
	USE_DEV5_FPGA_REGS();

	VERBOSE_KPRINTF(1, "%s():old:FPGA_SPCKMODE %x\n", "FpgaSpckmodeOff", dev5_fpga_regs->r_fpga_spckmode);
	dev5_fpga_regs->r_fpga_spckmode &= ~1;
	VERBOSE_KPRINTF(1, "%s():new:FPGA_SPCKMODE %x\n", "FpgaSpckmodeOff", dev5_fpga_regs->r_fpga_spckmode);
}

static void FpgaXfdir(int dir)
{
	USE_DEV5_FPGA_REGS();

	VERBOSE_KPRINTF(1, "%s():old:FPGA_XFRDIR %x\n", "FpgaXfrdir", dev5_fpga_regs->r_fpga_xfrdir);
	dev5_fpga_regs->r_fpga_xfrdir &= ~1;
	if ( dir )
	{
		dev5_fpga_regs->r_fpga_xfrdir |= 1;
	}
	VERBOSE_KPRINTF(1, "%s():new:FPGA_XFRDIR %x\n", "FpgaXfrdir", dev5_fpga_regs->r_fpga_xfrdir);
}

static int FpgaGetRevision(void)
{
	USE_DEV5_FPGA_REGS();

	VERBOSE_KPRINTF(1, "%s():FPGA_REVISION %x\n", "FpgaGetRevision", dev5_fpga_regs->r_fpga_revision);
	return (u16)dev5_fpga_regs->r_fpga_revision;
}

#ifdef UNUSED_FUNC
static unsigned int do_fpga_add_unused8120(void)
{
	USE_DEV5_FPGA_REGS();

	return (dev5_fpga_regs->r_fpga_sl3bufd + (unsigned int)dev5_fpga_regs->r_fpga_exbufd) >> 7;
}
#endif

static int do_fpga_check_spckcnt(void)
{
	USE_DEV5_FPGA_REGS();

	return (u16)dev5_fpga_regs->r_fpga_spckcnt;
}

static void FpgaCheckWriteBuffer(void)
{
	int i;
	USE_DEV5_FPGA_REGS();

	VERBOSE_KPRINTF(1, "%s():in ...\n", "FpgaCheckWriteBuffer");
	for ( i = 0; i < 10000 && (dev5_fpga_regs->r_fpga_exbufe || dev5_fpga_regs->r_fpga_sl3bufe); i += 1 )
		;
	if ( i == 10000 )
	{
		VERBOSE_KPRINTF(0, "exbuf enc=%x, sl3buf enc=%x\n", dev5_fpga_regs->r_fpga_exbufe, dev5_fpga_regs->r_fpga_sl3bufe);
	}
	VERBOSE_KPRINTF(1, "%s():out ...\n", "FpgaCheckWriteBuffer");
}

static void FpgaCheckWriteBuffer2(void)
{
	USE_DEV5_FPGA_REGS();

	VERBOSE_KPRINTF(1, "%s():in ...\n", "FpgaCheckWriteBuffer2");
	while ( dev5_fpga_regs->r_fpga_sl3bufd )
	{
		VERBOSE_KPRINTF(1, "%s():FPGA_SL3BUFD %x\n", "FpgaCheckWriteBuffer2", dev5_fpga_regs->r_fpga_sl3bufd);
		VERBOSE_KPRINTF(1, "%s():FPGA_SL3BUFE %x\n", "FpgaCheckWriteBuffer2", dev5_fpga_regs->r_fpga_sl3bufe);
		VERBOSE_KPRINTF(1, "%s():FPGA_EXBUFD %x\n", "FpgaCheckWriteBuffer2", dev5_fpga_regs->r_fpga_exbufd);
		VERBOSE_KPRINTF(1, "%s():FPGA_EXBUFE %x\n", "FpgaCheckWriteBuffer2", dev5_fpga_regs->r_fpga_exbufe);
	}
	VERBOSE_KPRINTF(1, "%s():out ...\n", "FpgaCheckWriteBuffer2");
}

static void FpgaClearBuffer(void)
{
	USE_DEV5_FPGA_REGS();

	VERBOSE_KPRINTF(1, "%s():old:FPGA_SL3BUFD %x\n", "FpgaClearBuffer", dev5_fpga_regs->r_fpga_sl3bufd);
	VERBOSE_KPRINTF(1, "%s():old:FPGA_SL3BUFE %x\n", "FpgaClearBuffer", dev5_fpga_regs->r_fpga_sl3bufe);
	VERBOSE_KPRINTF(1, "%s():old:FPGA_EXBUFD %x\n", "FpgaClearBuffer", dev5_fpga_regs->r_fpga_exbufd);
	VERBOSE_KPRINTF(1, "%s():old:FPGA_EXBUFE %x\n", "FpgaClearBuffer", dev5_fpga_regs->r_fpga_exbufe);
	dev5_fpga_regs->r_fpga_unk30 &= ~1;
	dev5_fpga_regs->r_fpga_unk30 |= 1;
	while ( (u16)(dev5_fpga_regs->r_fpga_exbufd) || dev5_fpga_regs->r_fpga_sl3bufd )
		;
	while ( dev5_fpga_regs->r_fpga_exbufe || dev5_fpga_regs->r_fpga_sl3bufe )
		;
	dev5_fpga_regs->r_fpga_unk30 &= ~1;
	VERBOSE_KPRINTF(1, "%s():new:FPGA_SL3BUFD %x\n", "FpgaClearBuffer", dev5_fpga_regs->r_fpga_sl3bufd);
	VERBOSE_KPRINTF(1, "%s():new:FPGA_SL3BUFE %x\n", "FpgaClearBuffer", dev5_fpga_regs->r_fpga_sl3bufe);
	VERBOSE_KPRINTF(1, "%s():new:FPGA_EXBUFD %x\n", "FpgaClearBuffer", dev5_fpga_regs->r_fpga_exbufd);
	VERBOSE_KPRINTF(1, "%s():new:FPGA_EXBUFE %x\n", "FpgaClearBuffer", dev5_fpga_regs->r_fpga_exbufe);
}

static int Mpeg2CheckPadding(char *buf, unsigned int bufsize, int *retptr, int *pesscramblingpackptr)
{
	int bufchk;
	char *bufoffs1;
	const char *bufoffs2;
	int i;

	bufchk = 0;
	VERBOSE_KPRINTF(1, "%s():in\n", "Mpeg2CheckPadding");
	if ( (bufsize & 0x7FF) )
	{
		VERBOSE_KPRINTF(1, "%s():Buffer size not aligne !!\n", "Mpeg2CheckPadding");
		return -1;
	}
	if ( !*buf && !buf[1] && buf[2] == 1 && (u8)buf[3] == 0xBA )
		bufchk = 1;
	for ( i = 0; (unsigned int)i < (bufsize >> 11); i += 1 )
	{
		bufoffs1 = &buf[i << 11];
		if ( *bufoffs1 || bufoffs1[1] || bufoffs1[2] != 1 || (u8)bufoffs1[3] != 0xBA )
		{
			if ( bufchk )
				break;
		}
		else
		{
			if ( !bufchk )
				break;
			bufoffs2 = bufoffs1 + 14;
			if ( !bufoffs1[14] )
			{
				if ( !bufoffs1[15] && bufoffs1[16] == 1 && (u8)bufoffs1[17] == 0xBB )
					bufoffs2 = bufoffs1 + 38;
				if ( !*bufoffs2 && !bufoffs2[1] && bufoffs2[2] == 1 && (u8)bufoffs2[3] != 0xBF && (bufoffs2[6] & 0x60) )
				{
					*pesscramblingpackptr += 1;
				}
			}
		}
	}
	VERBOSE_KPRINTF(1, "%s():out %d %s Pack\n", "Mpeg2CheckPadding", i, bufchk ? "RDI" : "NULL");
	*retptr = i;
	return bufchk;
}

static int Mpeg2CheckScramble(char *buf, unsigned int bufsize)
{
	int restmp;
	signed int i;
	char *bufcur;
	const char *bufbuf;
	int buf3;

	restmp = 0;
	VERBOSE_KPRINTF(1, "%s():in\n", "Mpeg2CheckScramble");
	if ( (bufsize & 0x7FF) )
	{
		VERBOSE_KPRINTF(1, "%s():Buffer size not aligne !!\n", "Mpeg2CheckScramble");
		return 0;
	}
	for ( i = 0; i < (int)(bufsize >> 11); i += 1 )
	{
		bufcur = buf + (i << 11);
		if ( !*bufcur && !bufcur[1] && bufcur[2] == 1 && (u8)bufcur[3] == 0xBA )
		{
			bufbuf = bufcur + 14;
			if ( !bufcur[14] )
			{
				if ( !bufcur[15] && bufcur[16] == 1 && (u8)bufcur[17] == 0xBB )
					bufbuf = bufcur + 38;
				if ( !*bufbuf && !bufbuf[1] && bufbuf[2] == 1 )
				{
					buf3 = (u8)bufbuf[3];
					if ( (buf3 == 0xE0 || buf3 == 0xBD || buf3 == 0xC0 || buf3 == 0xD0) && (bufbuf[6] & 0x30) )
					{
						restmp = 0xFFFFFFFF;
						break;
					}
				}
			}
		}
	}
	VERBOSE_KPRINTF(1, "%s():out\n", "Mpeg2CheckScramble");
	return restmp;
}

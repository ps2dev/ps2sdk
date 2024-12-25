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
#include <dev5_mmio_hwport.h>
#include <iop_mmio_hwport.h>

IRX_ID("EEConfig", 1, 1);
// Based on the module from ROM 2.20+.

static int eeconf_config_numblocks;

static int eeconf_writeread_scmd(char cmd, const char *sdata, int sdlen, char *rdata, int rdlen)
{
	int rdlen_tmp;
	int i;
	char rdata_tmp[64];
	USE_DEV5_MMIO_HWPORT();

	rdlen_tmp = rdlen;
	if ( (dev5_mmio_hwport->m_dev5_reg_017 & 0x80) != 0 )
		return 0;
	while ( (dev5_mmio_hwport->m_dev5_reg_017 & 0x40) == 0 )
		;
	for ( i = 0; i < sdlen; i += 1 )
		dev5_mmio_hwport->m_dev5_reg_017 = sdata[i];
	dev5_mmio_hwport->m_dev5_reg_016 = cmd;
	while ( (dev5_mmio_hwport->m_dev5_reg_017 & 0x80) != 0 )
		;
	for ( i = 0; (dev5_mmio_hwport->m_dev5_reg_017 & 0x40) == 0; i += 1 )
		rdata_tmp[i] = dev5_mmio_hwport->m_dev5_reg_018;
	if ( rdlen_tmp > i )
		rdlen_tmp = i;
	for ( i = 0; i < rdlen_tmp; i += 1 )
		rdata[i] = rdata_tmp[i];
	return 1;
}

static int eeconf_open_config(int block, int mode, int NumBlocks, char *status)
{
	int wdata;

	wdata = (u8)mode | ((u8)block << 8) | ((u8)NumBlocks << 16);
	eeconf_config_numblocks = NumBlocks;
	return eeconf_writeread_scmd(64, (const char *)&wdata, 3, status, 1);
}

static int eeconf_close_config(char *status)
{
	int result;

	result = eeconf_writeread_scmd(67, 0, 0, status, 1);
	eeconf_config_numblocks = 0;
	return result;
}

static int read_config_process(u32 *arg1, char *status)
{
	int result;
	char rdata[16];

	result = eeconf_writeread_scmd(65, 0, 0, rdata, 16);
	*status = (u8)(rdata[14] + rdata[13] + rdata[12] + rdata[11] + rdata[10] + rdata[9] + rdata[8] + rdata[7] + rdata[6]
								 + rdata[5] + rdata[4] + rdata[3] + rdata[2] + rdata[0] + rdata[1])
				 != (u8)rdata[15];
	arg1[0] = *(u32 *)&rdata[0];
	arg1[1] = *(u32 *)&rdata[4];
	arg1[2] = *(u32 *)&rdata[8];
	*((u8 *)arg1 + 12) = rdata[12];
	*((u8 *)arg1 + 13) = rdata[13];
	*((u8 *)arg1 + 14) = rdata[14];
	return result;
}

static int eeconf_read_config(void *buffer, char *status)
{
	int i;

	for ( i = 0; i < eeconf_config_numblocks; i += 1 )
		if ( !read_config_process((u32 *)((char *)buffer + (15 * i)), status) || *status )
			break;
	return i;
}

static int write_config_process(char *inval, char *status)
{
	char wdata[16];

	*status = 0;
	*(u32 *)&wdata[0] = *(u32 *)inval;
	*(u32 *)&wdata[4] = *((u32 *)inval + 1);
	*(u32 *)&wdata[8] = *((u32 *)inval + 2);
	wdata[12] = inval[12];
	wdata[13] = inval[13];
	wdata[14] = inval[14];
	wdata[15] = inval[14] + inval[13] + inval[12] + inval[11] + inval[10] + inval[9] + inval[8] + inval[7] + inval[6]
						+ inval[5] + inval[4] + inval[3] + inval[2] + inval[0] + inval[1];
	return eeconf_writeread_scmd(66, wdata, 16, status, 1);
}

static int eeconf_write_config(const void *buffer, char *status)
{
	int i;

	for ( i = 0; i < eeconf_config_numblocks; i += 1 )
		if ( !write_config_process((char *)buffer + (15 * i), status) || *status )
			break;
	return i;
}

static void eeconf_ieee1394_reset()
{
	USE_IOP_MMIO_HWPORT();

	iop_mmio_hwport->ieee1394.PHYAccess = 0x417F0000;
	while ( !(iop_mmio_hwport->ieee1394.NodeID & 1) )
		;
	iop_mmio_hwport->ieee1394.ubufReceiveClear = -1;
	iop_mmio_hwport->ieee1394.intr0 = -1;
	iop_mmio_hwport->ieee1394.intr1 = -1;
	iop_mmio_hwport->ieee1394.intr2 = -1;
}

static int eeconf_ieee1394_available()
{
	USE_IOP_MMIO_HWPORT();

	return (iop_mmio_hwport->ieee1394.UnknownRegister7C & 0xF0000000) == 0x10000000;
}

static int eeconf_ee_ssbus_available()
{
	USE_IOP_MMIO_HWPORT();

	return iop_mmio_hwport->iop_sbus_ctrl[0] & 0x80000000;
}

static void eeconf_dve_magic()
{
	USE_IOP_MMIO_HWPORT();

	if ( (unsigned int)(iop_mmio_hwport->exp2_r2[4612]) - 96 < 2 )
	{
		iop_mmio_hwport->exp2_r2[4632] = 0;
		return;
	}
	if ( (iop_mmio_hwport->dev9c[14] & 0xF0) != '0' )
	{
		*(vu16 *)&iop_mmio_hwport->dev9c[16] = 0;
		*(vu16 *)&iop_mmio_hwport->dev9c[18] = 0;
		return;
	}
	*((vu16 *)0xB600000A) = 0;
}

static void eeconf_handle_mac_address()
{
	char rdata[16];
	int i;

	for ( i = 0; i < 100; i += 1 )
		if ( eeconf_writeread_scmd(55, 0, 0, rdata, 9) )
			break;
	if ( i >= 100 || (rdata[0] & 0x80) != 0 )
		memset(rdata, 255, sizeof(rdata));
	*((vu32 *)0xFFFE0188) = ((u8)rdata[4] << 24) | ((u8)rdata[3] << 16) | ((u8)rdata[2] << 8) | (u8)rdata[1];
	*((vu32 *)0xFFFE018C) = ((u8)rdata[8] << 24) | ((u8)rdata[7] << 16) | ((u8)rdata[6] << 8) | (u8)rdata[5];
}

static void eeconf_handle_region_param()
{
	char rdata[16];
	int i;

	for ( i = 0; i < 100; i += 1 )
		if ( eeconf_writeread_scmd(54, 0, 0, rdata, 15) )
			break;
	if ( i < 100 && (rdata[0] & 0x80) == 0 )
	{
		// The following address is the byte at index 5 of ROMVER
		*((vu32 *)0xFFFE0180) = 0xBFC7FF04;
		*((vu32 *)0xFFFE0184) = (u8)rdata[3];
		// The following address is the byte at index 34 of VERSTR
		*((vu32 *)0xFFFE0180) = 0xBFC7FF52;
		*((vu32 *)0xFFFE0184) = (u8)rdata[8];
	}
	*((vu32 *)0xFFFE0180) = 0xFFFFFFFF;
}

static void eeconf_handle_eegs_config(const u8 *buf)
{
	*((vu32 *)0xFFFE0190) = buf ? (buf[1] & 0xF) : 3;
}

int _start(int ac, char **av)
{
	const int *p_bootmode3;
	int read_conf_succeeded;
	char *cfgblock_mem;
	int any_ps1drv_flag_set;
	int i;
	char cfgblock_tmp[16];
	char cfgstatus;
	USE_DEV5_MMIO_HWPORT();

	(void)ac;
	(void)av;

	p_bootmode3 = QueryBootMode(3);
	if ( p_bootmode3 && (p_bootmode3[1] & (1 | 2)) )
		return 1;
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Warray-bounds"
	*((void **)0x3C0) = 0;
#pragma GCC diagnostic pop
	for ( i = 0; i < 0x30000; i += 1 )
		if ( !(dev5_mmio_hwport->m_dev5_reg_005 & 8) )
			break;
	if ( i >= 0x30000 )
		return 1;
	read_conf_succeeded = 0;
	cfgblock_mem = (char *)AllocSysMemory(1, 96, 0);
	if ( cfgblock_mem )
	{
		read_conf_succeeded = 1;
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Warray-bounds"
		*((void **)0x3C0) = cfgblock_mem;
#pragma GCC diagnostic pop
		for ( i = 0; i < 90; i += 1 )
			cfgblock_mem[i] = 0;
		for ( i = 0; i < 0x10000; i += 1 )
			if ( eeconf_open_config(0, 0, 4, &cfgstatus) && !(cfgstatus & 9) )
				break;
		if ( i >= 0x10000 )
			read_conf_succeeded = 0;
		for ( i = 0; i < 0x10000; i += 1 )
			if ( eeconf_read_config(cfgblock_mem, &cfgstatus) && !(cfgstatus & 9) )
				break;
		if ( i >= 0x10000 )
			read_conf_succeeded = 0;
		for ( i = 0; i < 0x10000; i += 1 )
			if ( eeconf_close_config(&cfgstatus) && !(cfgstatus & 9) )
				break;
		if ( i >= 0x10000 )
			read_conf_succeeded = 0;
		for ( i = 0; i < 0x10000; i += 1 )
			if ( eeconf_open_config(1u, 0, 2, &cfgstatus) && !(cfgstatus & 9) )
				break;
		if ( i >= 0x10000 )
			read_conf_succeeded = 0;
		for ( i = 0; i < 0x10000; i += 1 )
			if ( eeconf_read_config(cfgblock_mem + 60, &cfgstatus) && !(cfgstatus & 9) )
				break;
		if ( i >= 0x10000 )
			read_conf_succeeded = 0;
		for ( i = 0; i < 0x10000; i += 1 )
			if ( eeconf_close_config(&cfgstatus) && !(cfgstatus & 9) )
				break;
		if ( i >= 0x10000 )
			read_conf_succeeded = 0;
		cfgblock_mem[15] &= ~8;
		cfgblock_mem[15] |= ((cfgblock_mem[75] & 8) != 0);
	}
	if ( !read_conf_succeeded )
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Warray-bounds"
		*((void **)0x3C0) = 0;
#pragma GCC diagnostic pop
	// The following block is only in EECONF module in 1.60+, 1.70+, 2.20+, PS3 ps2_emu ROMs.
	if ( !*(u16 *)QueryBootMode(4) )
	{
		// The following block is only in EECONF module in 1.60+, 1.70+, 2.20+ ROMs.
		eeconf_dve_magic();
		// The following block is only in EECONF module in 2.20+ ROMs.
		if ( eeconf_ee_ssbus_available() )
		{
			eeconf_handle_region_param();
			eeconf_handle_mac_address();
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Warray-bounds"
			eeconf_handle_eegs_config((u8 *)(void **)0x3C0);
#pragma GCC diagnostic pop
		}
		// The following block is only in EECONF module in 1.70+, 2.20+, PS3 ps2_emu ROMs.
		if ( eeconf_ieee1394_available() )
			eeconf_ieee1394_reset();
	}
	if ( !(dev5_mmio_hwport->m_dev5_reg_005 & 2) || (dev5_mmio_hwport->m_dev5_reg_005 & 4) )
		return 1;
	for ( i = 0; i < 0x10000; i += 1 )
		if ( eeconf_open_config(1u, 0, 1, &cfgstatus) && !(cfgstatus & 9) )
			break;
	for ( i = 0; i < 0x10000; i += 1 )
		if ( eeconf_read_config(cfgblock_tmp, &cfgstatus) && !(cfgstatus & 9) )
			break;
	for ( i = 0; i < 0x10000; i += 1 )
		if ( eeconf_close_config(&cfgstatus) && !(cfgstatus & 9) )
			break;
	any_ps1drv_flag_set = 0;
	for ( i = 0; i < 15; i += 1 )
	{
		if ( cfgblock_tmp[i] )
		{
			any_ps1drv_flag_set = 1;
			cfgblock_tmp[i] = 0;
		}
	}
	if ( any_ps1drv_flag_set )
	{
		for ( i = 0; i < 0x10000; i += 1 )
			if ( eeconf_open_config(1u, 1u, 1, &cfgstatus) && !(cfgstatus & 9) )
				break;
		for ( i = 0; i < 0x10000; i += 1 )
			if ( eeconf_write_config(cfgblock_tmp, &cfgstatus) && !(cfgstatus & 9) )
				break;
		for ( i = 0; i < 0x10000; i += 1 )
			if ( eeconf_close_config(&cfgstatus) && !(cfgstatus & 9) )
				break;
	}
	return 1;
}

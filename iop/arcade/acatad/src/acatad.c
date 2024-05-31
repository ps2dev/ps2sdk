/*
# _____     ___ ____     ___ ____
#  ____|   |    ____|   |        | |____|
# |     ___|   |____ ___|    ____| |    \    PS2DEV Open Source Project.
#-----------------------------------------------------------------------
# Copyright ps2dev - http://www.ps2dev.org
# Licenced under Academic Free License version 2.0
# Review ps2sdk README & LICENSE files for further details.
*/

#include <atad.h>
#include <atahw.h>
#include <irx_imports.h>

#define MODNAME "ARCADE_ATAD_driver"
IRX_ID(MODNAME, 1, 80);
// Text section hash:
// 7320ef1d104cc8a53b66846111f81621

extern struct irx_export_table _exp_atad;

struct acatad_request_data
{
	int thid;
	int ret;
};

#define sceAtaEntry _start

static ata_devinfo_t g_devinfo;
static u16 g_sectorbuf[256];

void on_request_done(acAtaT ata, struct acatad_request_data *arg, int ret)
{
	int thid;

	(void)ata;
	thid = arg->thid;
	arg->ret = ret;
	arg->thid = 0;
	if ( thid )
		WakeupThread(thid);
}

int do_request(int flag, acAtaCommandData *cmd, int item, void *buf, int size)
{
	acAtaData *ata_ptr;
	int result;
	acAtaData ata;
	struct acatad_request_data reqd;

	ata_ptr = acAtaSetup(&ata, (acAtaDone)on_request_done, &reqd, 5000000u);
	reqd.ret = 0;
	reqd.thid = GetThreadId();
	result = acAtaRequest(ata_ptr, flag, cmd, item, buf, size);
	if ( result < 0 )
	{
		return result;
	}
	while ( reqd.thid )
		SleepThread();
	return reqd.ret;
}

int do_identify(int device, u32 *total_sectors)
{
	int devident;
	int err;
	int model_no_i;
	acAtaCommandData cmd[4];

	devident = (device != 0) << 4;
	memset(g_sectorbuf, 0, sizeof(g_sectorbuf));
	cmd[0] = devident | 0x600;
	cmd[1] = ATA_C_IDENTIFY_DEVICE | 0x700;
	err = do_request(devident | 2, cmd, 2, g_sectorbuf, 512);
	model_no_i = 27;
	if ( err < 0 )
	{
		if ( err != -6 )
		{
			printf("%d:identify: unexpected error %d\n", device, err);
			return err;
		}
		return 0;
	}
	while ( model_no_i < 47 )
	{
		g_sectorbuf[model_no_i] = ((g_sectorbuf[model_no_i] & 0xFF00) >> 8) | (g_sectorbuf[model_no_i] << 8);
		model_no_i += 1;
	}
	g_sectorbuf[47] = 0;
	*total_sectors = g_sectorbuf[60] + (g_sectorbuf[61] << 16);
	printf("%d:identify:%d: %s\n", device, err, (const char *)&g_sectorbuf[27]);
	return 1;
}

int do_setfeatures(int device, u8 features)
{
	int err;
	acAtaCommandData cmd[4];

	cmd[0] = features | 0x200;
	cmd[1] = 0x03 | 0x100;  // Set transfer mode
	cmd[2] = ATA_C_SET_FEATURES | 0x700;
	err = do_request(((device != 0) << 4) | 0x42, cmd, 3, 0, 0);
	if ( err < 0 )
	{
		if ( err != -6 )
		{
			printf("%d:setfeatures: unexpected error %d\n", device, err);
			return err;
		}
		return 0;
	}
	return 1;
}

int do_dma_xfer_read(int device, u32 lba, void *buf, u32 nsectors)
{
	int devident;
	int err;
	int busywait_tmp;
	acAtaCommandData cmd[8];

	devident = (device != 0) << 4;
	cmd[0] = (nsectors & 0xFF) | 0x200;
	cmd[1] = (lba & 0xFF) | 0x300;
	cmd[2] = ((lba & 0xFF00) >> 8) | 0x400;
	cmd[3] = ((lba & 0xFF0000) >> 16) | 0x500;
	cmd[4] = devident | 0x40 | ((lba & 0xFF000000) >> 24) | 0x600;
	cmd[5] = ATA_C_READ_DMA_WITHOUT_RETRIES | 0x700;
	err = do_request(devident | 0x43, cmd, 6, buf, nsectors << 9);
	if ( err < 0 )
	{
		if ( err != -6 )
		{
			printf("%d:read: unexpected error %d\n", device, err);
			return err;
		}
		return 0;
	}
	busywait_tmp = 0;
	while ( busywait_tmp++ < (int)(nsectors + 32) )
		;
	return 1;
}

int do_dma_xfer_write(int device, u32 lba, void *buf, u32 nsectors)
{
	int devident;
	int err;
	int busywait_tmp;
	acAtaCommandData cmd[8];

	devident = (device != 0) << 4;
	cmd[0] = (nsectors & 0xFF) | 0x200;
	cmd[1] = (lba & 0xFF) | 0x300;
	cmd[2] = ((lba & 0xFF00) >> 8) | 0x400;
	cmd[3] = ((lba & 0xFF0000) >> 16) | 0x500;
	cmd[4] = devident | 0x40 | ((lba & 0xFF000000) >> 24) | 0x600;
	cmd[5] = ATA_C_WRITE_DMA | 0x700;
	err = do_request(devident | 0x47, cmd, 6, buf, nsectors << 9);
	if ( err < 0 )
	{
		if ( err != -6 )
		{
			printf("%d:read: unexpected error %d\n", device, err);
			return err;
		}
		return 0;
	}
	busywait_tmp = 0;
	while ( busywait_tmp++ < (int)(nsectors + 32) )
		;
	return 1;
}

int do_flushcache(int device)
{
	int err;
	acAtaCommandData cmd[4];

	cmd[0] = 0x200;
	cmd[1] = 0x103;
	cmd[2] = ATA_C_FLUSH_CACHE | 0x700;
	err = do_request(((device != 0) << 4) | 0x42, cmd, 3, 0, 0);
	if ( err < 0 )
	{
		if ( err != -6 )
		{
			printf("%d:flushcache: unexpected error %d\n", device, err);
			return err;
		}
		return 0;
	}
	return 1;
}

ata_devinfo_t *sceAtaInit(int device)
{
	int probe_res;
	int ident_err;
	u32 total_sectors[2];

	probe_res = ata_probe((acAtaReg)0xB6000000);
	g_devinfo.has_packet = 0;
	g_devinfo.exists = device == probe_res - 1;
	if ( device != probe_res - 1 )
	{
		g_devinfo.total_sectors = 0;
		return &g_devinfo;
	}
	ident_err = do_identify(device, total_sectors);
	if ( ident_err < 0 )
	{
		printf("identify error %d\n", ident_err);
		return 0;
	}
	do_setfeatures(device, 0x22 /* MDMA2 */);
	g_devinfo.security_status = 255;
	g_devinfo.total_sectors = total_sectors[0];
	printf("sceAtaInit %x %x %x\n", device, (unsigned int)g_devinfo.exists, (unsigned int)total_sectors[0]);
	return &g_devinfo;
}

int sceAtaSoftReset(void)
{
	printf("sceAtaSoftReset\n");
	while ( 1 )
		;
}

int sceAtaExecCmd(
	void *buf, u32 blkcount, u16 feature, u16 nsector, u16 sector, u16 lcyl, u16 hcyl, u16 select, u16 command)
{
	(void)buf;
	(void)blkcount;
	(void)feature;
	(void)nsector;
	(void)sector;
	(void)lcyl;
	(void)hcyl;
	(void)select;
	(void)command;
	printf("sceAtaExecCmd\n");
	while ( 1 )
		;
}

int sceAtaWaitResult(void)
{
	printf("sceAtaWaitResult\n");
	while ( 1 )
		;
}

int sceAtaGetError(void)
{
	printf("sceAtaGetError\n");
	while ( 1 )
		;
}

int sceAtaDmaTransfer(int device, void *buf, u32 lba, u32 nsectors, int dir)
{
	int err;

	if ( dir == 1 )
		err = do_dma_xfer_write(device, lba, buf, nsectors);
	else
		err = do_dma_xfer_read(device, lba, buf, nsectors);
	if ( err != 1 )
	{
		printf("dma error\n");
		while ( 1 )
			;
	}
	return 0;
}

int sceAtaSecuritySetPassword(int device, void *password)
{
	(void)device;
	(void)password;

	printf("sceAtaSecuritySetPassword\n");
	while ( 1 )
		;
}

int sceAtaSecurityUnLock(int device, void *password)
{
	(void)device;
	(void)password;

	printf("sceAtaSecurityUnLock\n");
	return 0;
}

int sceAtaSecurityEraseUnit(int device)
{
	(void)device;

	printf("sceAtaSecurityEraseUnit\n");
	while ( 1 )
		;
}

int sceAtaIdle(int device, int period)
{
	(void)device;
	(void)period;

	printf("sceAtaIdle\n");
	while ( 1 )
		;
}

int sceAtaGetSceId(int device, void *data)
{
	(void)device;
	(void)data;

	printf("sceAtaGetSceId\n");
	while ( 1 )
		;
}

int sceAtaSmartReturnStatus(int device)
{
	(void)device;

	printf("sceAtaSmartReturnStatus\n");
	while ( 1 )
		;
}

int sceAtaSmartSaveAttr(int device)
{
	(void)device;

	printf("sceAtaSmartSaveAttr\n");
	while ( 1 )
		;
}

int sceAtaFlushCache(int device)
{
	do_flushcache(device);
	return 0;
}

int sceAtaIdleImmediate(int device)
{
	(void)device;

	printf("sceAtaIdleImmediate\n");
	while ( 1 )
		;
}

int sceAtaEntry()
{
	if ( RegisterLibraryEntries(&_exp_atad) )
		printf("atad: module already loaded\n");
	printf("sceAtaEntry\n");
	return 0;
}

int sceAtaTerminate()
{
	printf(" sceAtaTerminate\n");
	return 0;
}

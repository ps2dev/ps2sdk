/*
# _____     ___ ____     ___ ____
#  ____|   |    ____|   |        | |____|
# |     ___|   |____ ___|    ____| |    \    PS2DEV Open Source Project.
#-----------------------------------------------------------------------
# Copyright (c) 2003 Marcus R. Brown <mrbrown@0xd6.org>
# Licenced under Academic Free License version 2.0
# Review ps2sdk README & LICENSE files for further details.
*/

/**
 * @file
 * ATA device driver.
 * This module provides the low-level ATA support for hard disk drives, based on ATAD v2.7.
 * It is 100% compatible with its proprietary counterpart called atad.irx.
 *
 * This module also include support for 48-bit feature set (done by Clement).
 * To avoid causing an "emergency park" for some HDDs, shutdown callback 15 of dev9
 * is used for issuing the STANDBY IMMEDIATE command prior to DEV9 getting shut down.
 */

#include <types.h>
#include <defs.h>
#include <irx.h>
#include <loadcore.h>
#include <thbase.h>
#include <thevent.h>
#include <stdio.h>
#include <sysclib.h>
#include <dev9.h>
#include <atad.h>

#include <speedregs.h>
#include <atahw.h>

#define MODNAME "atad"
IRX_ID(MODNAME, 2, 7);

#define M_PRINTF(format, args...)	\
	printf(MODNAME ": " format, ## args)

#define BANNER "ATA device driver %s - Copyright (c) 2003 Marcus R. Brown\n"
#define VERSION "v1.2"

#define ATA_XFER_MODE_PIO	0x08
#ifdef ATA_MWDMA_MODES
#define ATA_XFER_MODE_MDMA	0x20
#endif
#define ATA_XFER_MODE_UDMA	0x40

#define ATA_EV_TIMEOUT	1
#define ATA_EV_COMPLETE	2

static int ata_devinfo_init = 0;
static int ata_evflg = -1;

//Workarounds
static u8 ata_disable_lba48 = 0;	//Please read the comments in _start().
#ifdef ATA_GAMESTAR_WORKAROUND
static u8 ata_gamestar_workaround = 0;
#endif

/* Local device info kept for drives 0 and 1.  */
static ata_devinfo_t atad_devinfo[2];

/* Data returned from DEVICE IDENTIFY is kept here.  Also, this is used by the
   security commands to set and unlock the password.  */
static u16 ata_param[256];

/* ATA command info.  */
typedef struct _ata_cmd_info {
	u8 command;
	u8 type;
} ata_cmd_info_t;

static const ata_cmd_info_t ata_cmd_table[] = {
	{ATA_C_NOP,1},
	{ATA_C_CFA_REQUEST_EXTENDED_ERROR_CODE,1},
	{ATA_C_DEVICE_RESET,5},
	{ATA_C_READ_SECTOR,2},
	{ATA_C_READ_DMA_EXT,0x84},
	{ATA_C_WRITE_SECTOR,3},
	{ATA_C_WRITE_LONG,8},
	{ATA_C_WRITE_DMA_EXT,0x84},
	{ATA_C_CFA_WRITE_SECTORS_WITHOUT_ERASE,3},
	{ATA_C_READ_VERIFY_SECTOR,1},
	{ATA_C_READ_VERIFY_SECTOR_EXT,0x81},
	{ATA_C_SEEK,1},
	{ATA_C_CFA_TRANSLATE_SECTOR,2},
	{ATA_C_SCE_SECURITY_CONTROL,7},
	{ATA_C_EXECUTE_DEVICE_DIAGNOSTIC,6},
	{ATA_C_INITIALIZE_DEVICE_PARAMETERS,1},
	{ATA_C_DOWNLOAD_MICROCODE,3},
	{ATA_C_IDENTIFY_PACKET_DEVICE,2},
	{ATA_C_SMART,7},
	{ATA_C_CFA_ERASE_SECTORS,1},
	{ATA_C_READ_MULTIPLE,2},
	{ATA_C_WRITE_MULTIPLE,3},
	{ATA_C_SET_MULTIPLE_MODE,1},
	{ATA_C_READ_DMA,4},
	{ATA_C_WRITE_DMA,4},
	{ATA_C_CFA_WRITE_MULTIPLE_WITHOUT_ERASE,3},
	{ATA_C_GET_MEDIA_STATUS,1},
	{ATA_C_MEDIA_LOCK,1},
	{ATA_C_MEDIA_UNLOCK,1},
	{ATA_C_STANDBY_IMMEDIATE,1},
	{ATA_C_IDLE_IMMEDIATE,1},
	{ATA_C_STANDBY,1},
	{ATA_C_IDLE,1},
	{ATA_C_READ_BUFFER,2},
	{ATA_C_CHECK_POWER_MODE,1},
	{ATA_C_SLEEP,1},
	{ATA_C_FLUSH_CACHE,1},
	{ATA_C_WRITE_BUFFER,3},
	{ATA_C_FLUSH_CACHE_EXT,1},
	{ATA_C_IDENTIFY_DEVICE,2},
	{ATA_C_MEDIA_EJECT,1},
	{ATA_C_SET_FEATURES,1},
	{ATA_C_SECURITY_SET_PASSWORD,3},
	{ATA_C_SECURITY_UNLOCK,3},
	{ATA_C_SECURITY_ERASE_PREPARE,1},
	{ATA_C_SECURITY_ERASE_UNIT,3},
	{ATA_C_SECURITY_FREEZE_LOCK,1},
	{ATA_C_SECURITY_DISABLE_PASSWORD,3},
	{ATA_C_READ_NATIVE_MAX_ADDRESS,1},
	{ATA_C_SET_MAX_ADDRESS,1}
};
#define ATA_CMD_TABLE_SIZE	(sizeof ata_cmd_table/sizeof(ata_cmd_info_t))

static const ata_cmd_info_t sec_ctrl_cmd_table[] = {
	{ATA_SCE_IDENTIFY_DRIVE,2},
	{ATA_SCE_SECURITY_ERASE_PREPARE,1},
	{ATA_SCE_SECURITY_ERASE_UNIT,1},
	{ATA_SCE_SECURITY_FREEZE_LOCK,1},
	{ATA_SCE_SECURITY_SET_PASSWORD,3},
	{ATA_SCE_SECURITY_UNLOCK,3},
	{ATA_SCE_SECURITY_WRITE_ID,3},
	{ATA_SCE_SECURITY_READ_ID,2}
};
#define SEC_CTRL_CMD_TABLE_SIZE	(sizeof sec_ctrl_cmd_table/sizeof(ata_cmd_info_t))

static const ata_cmd_info_t smart_cmd_table[] = {
	{ATA_S_SMART_READ_DATA,2},
	{ATA_S_SMART_ENABLE_DISABLE_AUTOSAVE,1},
	{ATA_S_SMART_SAVE_ATTRIBUTE_VALUES,1},
	{ATA_S_SMART_EXECUTE_OFF_LINE,1},
	{ATA_S_SMART_READ_LOG,2},
	{ATA_S_SMART_WRITE_LOG,3},
	{ATA_S_SMART_ENABLE_OPERATIONS,1},
	{ATA_S_SMART_DISABLE_OPERATIONS,1},
	{ATA_S_SMART_RETURN_STATUS,1}
};
#define SMART_CMD_TABLE_SIZE	(sizeof smart_cmd_table/sizeof(ata_cmd_info_t))

/* This is the state info tracked between ata_io_start() and ata_io_finish().  */
typedef struct _ata_cmd_state {
	s32	type;		/* The ata_cmd_info_t type field. */
	union {
		void	*buf;
		u8	*buf8;
		u16	*buf16;
	};
	u32	blkcount;	/* The number of 512-byte blocks (sectors) to transfer.  */
	s32	dir;		/* DMA direction: 0 - to RAM, 1 - from RAM.  */
} ata_cmd_state_t;

static ata_cmd_state_t atad_cmd_state;

static int ata_intr_cb(int flag);
static unsigned int ata_alarm_cb(void *unused);

static void ata_set_dir(int dir);

static void ata_pio_mode(int mode);
#ifdef ATA_MWDMA_MODES
static void ata_multiword_dma_mode(int mode);
#endif
static void ata_ultra_dma_mode(int mode);
static void ata_shutdown_cb(void);

extern struct irx_export_table _exp_atad;

//In v1.04, DMA was enabled in ata_set_dir() instead.
static void ata_pre_dma_cb(int bcr, int dir)
{
	USE_SPD_REGS;

	SPD_REG16(SPD_R_XFR_CTRL)|=0x80;
}

static void ata_post_dma_cb(int bcr, int dir)
{
	USE_SPD_REGS;

	SPD_REG16(SPD_R_XFR_CTRL)&=~0x80;
}

static int ata_create_event_flag(void)
{
	iop_event_t event;

	event.attr = EA_SINGLE;	//In v1.04, EA_MULTI was specified.
	event.bits = 0;
	return CreateEventFlag(&event);
}

int _start(int argc, char *argv[])
{
	USE_SPD_REGS;
	int res = 1;

	printf(BANNER, VERSION);

	if (!(SPD_REG16(SPD_R_REV_3) & SPD_CAPS_ATA) || !(SPD_REG16(SPD_R_REV_8) & 0x02)) {
		M_PRINTF("HDD is not connected, exiting.\n");
		goto out;
	}

	/*	The PSX (Not the PlayStation or PSOne, but a PS2 with a DVR unit) has got an extra processor (Fujitsu MB91302A, aka the "DVRP") that seems to be emulating the console's PS2 ATA interface.
		The stock disks of all PSX units are definitely 48-bit LBA compliant because of their capacities, but the DVRP's emulation seems to have a design problem:
			1. It indicates that 48-bit LBA is supported.
			2. The 48-bit LBA capacity fields show the true capacity of the disk.
			3. Accesses to beyond the 28-bit LBA capacity (Which is 40.000GB by default) will result in I/O errors.
			4. (For some PSX units like the DESR-7500): Double-writes to the ATA registers seem to cause the ATA interface to stall.

		The problem is obviously in the DVRP's firmware, but we currently have no way to fix these bugs because the DVRP is even more heavily secured that the IOP.
		In the eyes of Sony, there isn't a problem because none of their retail PlayStation 2 software ever supported 48-bit LBA.

		The obvious workaround here would be to disable 48-bit LBA support when ATAD is loaded on a PSX. */
	ata_disable_lba48 = (SPD_REG16(SPD_R_REV_3) & SPD_CAPS_DVR)?1:0;

#ifdef ATA_GAMESTAR_WORKAROUND
	/*	Some compatible adaptors may malfunction if transfers are not done according to the old ps2atad design.
		Official adaptors appear to have a 0x0001 set for this register, but not compatibles.
		While official I/O to this register are 8-bit, some compatibles have a 0x01 for the lower 8-bits,
		but the upper 8-bits contain some random value. Hence perform a 16-bit read instead. */
	if(SPD_REG16(0x20) != 1) {
		ata_gamestar_workaround = 1;
		M_PRINTF("Compatible adaptor detected.\n");
	} else {
		ata_gamestar_workaround = 0;
	}   
#endif

	if ((ata_evflg = ata_create_event_flag()) < 0) {
		M_PRINTF("Couldn't create event flag, exiting.\n");
		res = 1;
		goto out;
	}

	/* In v1.04, PIO mode 0 was set here. In late versions, it is set in ata_init_devices(). */
	dev9RegisterIntrCb(1, &ata_intr_cb);
	dev9RegisterIntrCb(0, &ata_intr_cb);
#ifdef ATA_GAMESTAR_WORKAROUND
	if (!ata_gamestar_workaround) {
#endif
	dev9RegisterPreDmaCb(0, &ata_pre_dma_cb);
	dev9RegisterPostDmaCb(0, &ata_post_dma_cb);
#ifdef ATA_GAMESTAR_WORKAROUND
	}
#endif
	/* Register this at the last position, as it should be the last thing done before shutdown. */
	dev9RegisterShutdownCb(15, &ata_shutdown_cb);

	if ((res = RegisterLibraryEntries(&_exp_atad)) != 0) {
		M_PRINTF("Library is already registered, exiting.\n");
		goto out;
	}

	res = 0;
	M_PRINTF("Driver loaded.\n");
out:
	return res;
}

int _exit(void) { return 0; }

static int ata_intr_cb(int flag)
{
	if (flag == 1) {	/* New card, invalidate device info.  */
		memset(atad_devinfo, 0, sizeof atad_devinfo);
	} else {
		dev9IntrDisable(SPD_INTR_ATA);
		iSetEventFlag(ata_evflg, ATA_EV_COMPLETE);
	}

	return 1;
}

static unsigned int ata_alarm_cb(void *unused)
{
	iSetEventFlag(ata_evflg, ATA_EV_TIMEOUT);
	return 0;
}

/* Export 8 */
int ata_get_error(void)
{
	USE_ATA_REGS;
	return ata_hwport->r_error & 0xff;
}

#define ATA_WAIT_BUSY		0x80
#define ATA_WAIT_BUSBUSY	0x88

#define ata_wait_busy()		gen_ata_wait_busy(ATA_WAIT_BUSY)
#define ata_wait_bus_busy()	gen_ata_wait_busy(ATA_WAIT_BUSBUSY)

/* 0x80 for busy, 0x88 for bus busy.
	In the original ATAD, the busy and bus-busy functions were separate, but similar.  */
static int gen_ata_wait_busy(int bits)
{
	USE_ATA_REGS;
	int i, didx, delay;

	for (i = 0; i < 80; i++) {
		if (!(ata_hwport->r_control & bits))
			return 0;

		didx = i / 10;
		switch (didx) {
			case 0:
				continue;
			case 1:
				delay = 100;
				break;
			case 2:
				delay = 1000;
				break;
			case 3:
				delay = 10000;
				break;
			case 4:
				delay = 100000;
				break;
			default:
				delay = 1000000;
		}

		DelayThread(delay);
	}

	M_PRINTF("Timeout while waiting on busy (0x%02x).\n", bits);
	return ATA_RES_ERR_TIMEOUT;
}

static int ata_device_select(int device)
{
	USE_ATA_REGS;
	int res;

	if ((res = ata_wait_bus_busy()) < 0)
		return res;

	/* If the device was already selected, nothing to do.  */
	if (((ata_hwport->r_select >> 4) & 1) == device)
		return 0;

	/* Select the device.  */
	ata_hwport->r_select = (device & 1) << 4;
	res = ata_hwport->r_control;
	res = ata_hwport->r_control;	//Only done once in v1.04.

	return ata_wait_bus_busy();
}

/* Export 6 */
/*
	28-bit LBA:
		sector	(7:0)	-> LBA (7:0)
		lcyl	(7:0)	-> LBA (15:8)
		hcyl	(7:0)	-> LBA (23:16)
		device	(3:0)	-> LBA (27:24)

	48-bit LBA just involves writing the upper 24 bits in the format above into each respective register on the first write pass, before writing the lower 24 bits in the 2nd write pass. The LBA bits within the device field are not used in either write pass.
*/
int ata_io_start(void *buf, u32 blkcount, u16 feature, u16 nsector, u16 sector, u16 lcyl, u16 hcyl, u16 select, u16 command)
{
	USE_ATA_REGS;
	iop_sys_clock_t cmd_timeout;
	const ata_cmd_info_t *cmd_table;
	int i, res, type, cmd_table_size;
	int using_timeout, device = (select >> 4) & 1;
	u8 searchcmd;

	ClearEventFlag(ata_evflg, 0);

	if (!atad_devinfo[device].exists)
		return ATA_RES_ERR_NODEV;

	if ((res = ata_device_select(device)) != 0)
		return res;

	/* For the SCE and SMART commands, we need to search on the subcommand
	specified in the feature register.  */
	if (command == ATA_C_SCE_SECURITY_CONTROL) {
		cmd_table = sec_ctrl_cmd_table;
		cmd_table_size = SEC_CTRL_CMD_TABLE_SIZE;
		searchcmd = (u8)feature;
	} else if (command == ATA_C_SMART) {
		cmd_table = smart_cmd_table;
		cmd_table_size = SMART_CMD_TABLE_SIZE;
		searchcmd = (u8)feature;
	} else {
		cmd_table = ata_cmd_table;
		cmd_table_size = ATA_CMD_TABLE_SIZE;
		searchcmd = (u8)command;
	}

	type = 0;
	for (i = 0; i < cmd_table_size; i++) {
		if (searchcmd == cmd_table[i].command) {
			type = cmd_table[i].type;
			break;
		}
	}

	if (!(atad_cmd_state.type = type & 0x7F))	//Non-SONY: ignore the 48-bit LBA flag.
		return ATA_RES_ERR_CMD;

	atad_cmd_state.buf = buf;
	atad_cmd_state.blkcount = blkcount;

	/* Check that the device is ready if this the appropiate command.  */
	if (!(ata_hwport->r_control & 0x40)) {
		switch (command) {
			case ATA_C_DEVICE_RESET:
			case ATA_C_EXECUTE_DEVICE_DIAGNOSTIC:
			case ATA_C_INITIALIZE_DEVICE_PARAMETERS:
			case ATA_C_PACKET:
			case ATA_C_IDENTIFY_PACKET_DEVICE:
				break;
			default:
				M_PRINTF("Error: Device %d is not ready.\n", device);
				return ATA_RES_ERR_NOTREADY;
		}
	}

	/* Does this command need a timeout?  */
	using_timeout = 0;
	switch (type & 0x7F) {	//Non-SONY: ignore the 48-bit LBA flag.
		case 1:
		case 6:
			using_timeout = 1;
			break;
		case 4:
			atad_cmd_state.dir = (command != ATA_C_READ_DMA && command != ATA_C_READ_DMA_EXT);
			using_timeout = 1;
			break;
	}

	if (using_timeout) {
		cmd_timeout.lo = 0x41eb0000;
		cmd_timeout.hi = 0;

		/* SECURITY ERASE UNIT needs a bit more time.  */
		if (command == ATA_C_SCE_SECURITY_CONTROL && feature == ATA_SCE_SECURITY_ERASE_UNIT)
			USec2SysClock(180000000, &cmd_timeout);

		if ((res = SetAlarm(&cmd_timeout, &ata_alarm_cb, NULL)) < 0)
			return res;
	}

	/* Enable the command completion interrupt.  */
	if ((type & 0x7F) == 1)
		dev9IntrEnable(SPD_INTR_ATA0);

	/* Finally!  We send off the ATA command with arguments.  */
	ata_hwport->r_control = (using_timeout == 0) << 1;

	if(type&0x80) {	//For the sake of achieving improved performance, write the registers twice only if required! This is also required for compatibility with the buggy firmware of certain PSX units.
		/* 48-bit LBA requires writing to the address registers twice,
		   24 bits of the LBA address is written each time.
		   Writing to registers twice does not affect 28-bit LBA since
		   only the latest data stored in address registers is used.  */
		ata_hwport->r_feature = (feature >> 8) & 0xff;
		ata_hwport->r_nsector = (nsector >> 8) & 0xff;
		ata_hwport->r_sector  = (sector >> 8) & 0xff;
		ata_hwport->r_lcyl    = (lcyl >> 8) & 0xff;
		ata_hwport->r_hcyl    = (hcyl >> 8) & 0xff;
	}

	ata_hwport->r_feature = feature & 0xff;
	ata_hwport->r_nsector = nsector & 0xff;
	ata_hwport->r_sector  = sector & 0xff;
	ata_hwport->r_lcyl    = lcyl & 0xff;
	ata_hwport->r_hcyl    = hcyl & 0xff;
	ata_hwport->r_select  = (select | ATA_SEL_LBA) & 0xff;	//In v1.04, LBA was enabled in the ata_device_sector_io function.
	ata_hwport->r_command = command & 0xff;

	/* Turn on the LED.  */
	dev9LEDCtl(1);

	return 0;
}

/* Do a PIO transfer, to or from the device.  */
static int ata_pio_transfer(ata_cmd_state_t *cmd_state)
{
	USE_ATA_REGS;
	u8 *buf8;
	u16 *buf16;
	int i, type;
	u16 status = ata_hwport->r_status & 0xff;

	if (status & ATA_STAT_ERR) {
		M_PRINTF("Error: PIO cmd error: status 0x%02x, error 0x%02x.\n", status, ata_get_error());
		return ATA_RES_ERR_IO;
	}

	/* DRQ must be set (data request).  */
	if (!(status & ATA_STAT_DRQ))
		return ATA_RES_ERR_NODATA;

	type = cmd_state->type;

	if (type == 3 || type == 8) {
		/* PIO data out */
		buf16 = cmd_state->buf16;
		for (i = 0; i < 256; i++) {
			ata_hwport->r_data = *buf16;
			cmd_state->buf16 = ++buf16;
		}
		if (cmd_state->type == 8) {
			buf8 = cmd_state->buf8;
			for (i = 0; i < 4; i++) {
				ata_hwport->r_data = *buf8;
				cmd_state->buf8 = ++buf8;
			}
		}
	} else if (type == 2) {
		/* PIO data in  */
		buf16 = cmd_state->buf16;
		for (i = 0; i < 256; i++) {
			*buf16 = ata_hwport->r_data;
			cmd_state->buf16 = ++buf16;
		}
	}

	return 0;
}

/* Complete a DMA transfer, to or from the device.  */
static int ata_dma_complete(void *buf, u32 blkcount, int dir)
{
	USE_ATA_REGS;
	USE_SPD_REGS;
	u32 bits, count, nbytes;
	int i, res;
	u16 dma_stat;

	while (blkcount) {
		for (i = 0; i < 20; i++)
			if ((dma_stat = SPD_REG16(0x38) & 0x1f))
				goto next_transfer;

		if (dma_stat)
			goto next_transfer;

		dev9IntrEnable(SPD_INTR_ATA);
		/* Wait for the previous transfer to complete or a timeout.  */
		WaitEventFlag(ata_evflg, ATA_EV_TIMEOUT|ATA_EV_COMPLETE, WEF_CLEAR|WEF_OR, &bits);

		if (bits & ATA_EV_TIMEOUT) {	/* Timeout.  */
			M_PRINTF("Error: DMA timeout.\n");
			return ATA_RES_ERR_TIMEOUT;
		}
		/* No DMA completion bit? Spurious interrupt.  */
		if (!(SPD_REG16(SPD_R_INTR_STAT) & 0x02)) {
			if (ata_hwport->r_control & 0x01) {
				M_PRINTF("Error: Command error while doing DMA.\n");
				M_PRINTF("Error: Command error status 0x%02x, error 0x%02x.\n", ata_hwport->r_status, ata_get_error());
				/* In v1.04, there was no check for ICRC. */
				return((ata_get_error() & ATA_ERR_ICRC) ? ATA_RES_ERR_ICRC : ATA_RES_ERR_IO);
			} else {
				M_PRINTF("Warning: Got command interrupt, but not an error.\n");
				continue;
			}
		}

		dma_stat = SPD_REG16(0x38) & 0x1f;

next_transfer:
		count = (blkcount < dma_stat) ? blkcount : dma_stat;
		nbytes = count * 512;
		if ((res = dev9DmaTransfer(0, buf, (nbytes << 9)|32, dir)) < 0)
			return res;

		buf = (void*)((u8 *)buf + nbytes);
		blkcount -= count;
	}

	return 0;
}

/* Export 7 */
int ata_io_finish(void)
{
	USE_SPD_REGS;
	USE_ATA_REGS;
	ata_cmd_state_t *cmd_state = &atad_cmd_state;
	u32 bits;
	int i, res = 0, type = cmd_state->type;
	u16 stat;

	if (type == 1 || type == 6) {	/* Non-data commands.  */
		WaitEventFlag(ata_evflg, ATA_EV_TIMEOUT|ATA_EV_COMPLETE, WEF_CLEAR|WEF_OR, &bits);
		if (bits & ATA_EV_TIMEOUT) {	/* Timeout.  */
			M_PRINTF("Error: ATA timeout on a non-data command.\n");
			return ATA_RES_ERR_TIMEOUT;
		}
	} else if (type == 4) {		/* DMA.  */
		if ((res = ata_dma_complete(cmd_state->buf, cmd_state->blkcount,
						cmd_state->dir)) < 0)
			goto finish;

		for (i = 0; i < 100; i++)
			if ((stat = SPD_REG16(SPD_R_INTR_STAT) & 0x01))
				break;
		if (!stat) {
			dev9IntrEnable(SPD_INTR_ATA0);
			WaitEventFlag(ata_evflg, ATA_EV_TIMEOUT|ATA_EV_COMPLETE, WEF_CLEAR|WEF_OR, &bits);
			if (bits & ATA_EV_TIMEOUT) {
				M_PRINTF("Error: ATA timeout on DMA completion, buffer stat %04x\n", SPD_REG16(0x38));
				M_PRINTF("Error: istat %x, ienable %x\n", SPD_REG16(SPD_R_INTR_STAT), SPD_REG16(SPD_R_INTR_MASK));
				res = ATA_RES_ERR_TIMEOUT;
			}
		}
	} else {			/* PIO transfers.  */
		stat = ata_hwport->r_control;
		if ((res = ata_wait_busy()) < 0)
			goto finish;

		/* Transfer each PIO data block.  */
		while (--cmd_state->blkcount != -1) {
			if ((res = ata_pio_transfer(cmd_state)) < 0)
				goto finish;
			if ((res = ata_wait_busy()) < 0)
				goto finish;
		}
	}

	if (res)
		goto finish;

	/* Wait until the device isn't busy.  */
	if (ata_hwport->r_status & ATA_STAT_BUSY)
		res = ata_wait_busy();
	if ((stat = ata_hwport->r_status) & ATA_STAT_ERR) {
		M_PRINTF("Error: Command error: status 0x%02x, error 0x%02x.\n", stat, ata_get_error());
		/* In v1.04, there was no check for ICRC. */
		res = (ata_get_error() & ATA_ERR_ICRC) ? ATA_RES_ERR_ICRC : ATA_RES_ERR_IO;
	}

finish:
	/* The command has completed (with an error or not), so clean things up.  */
	CancelAlarm(&ata_alarm_cb, NULL);
	/* Turn off the LED.  */
	dev9LEDCtl(0);

	if(res)
		M_PRINTF("error: ATA failed, %d\n", res);

	return res;
}

/* Reset the ATA controller/bus.  */
static int ata_bus_reset(void)
{
	USE_SPD_REGS;
	SPD_REG16(SPD_R_IF_CTRL) = SPD_IF_ATA_RESET;
	DelayThread(100);
	SPD_REG16(SPD_R_IF_CTRL) = 0;	//Not present in v1.04.
	SPD_REG16(SPD_R_IF_CTRL) = 0x48;
	DelayThread(3000);
	return ata_wait_busy();
}

/* Export 5 */
int ata_reset_devices(void)
{
	USE_ATA_REGS;

	if (ata_hwport->r_control & 0x80)
		return ATA_RES_ERR_NOTREADY;

	/* Disables device interrupt assertion and asserts SRST. */
	ata_hwport->r_control = 6;
	DelayThread(100);

	/* Disable ATA interrupts.  */
	ata_hwport->r_control = 2;
	DelayThread(3000);

	return ata_wait_busy();
}

/* Export 17 */
int ata_device_flush_cache(int device)
{
	int res;

	if(!(res = ata_io_start(NULL, 1, 0, 0, 0, 0, 0, (device << 4) & 0xffff, atad_devinfo[device].lba48?ATA_C_FLUSH_CACHE_EXT:ATA_C_FLUSH_CACHE))) res=ata_io_finish();

	return res;
}

/* Export 13 */
int ata_device_idle(int device, int period)
{
	int res;

	if(!(res = ata_io_start(NULL, 1, 0, period & 0xff, 0, 0, 0, (device << 4) & 0xffff, ATA_C_IDLE))) res=ata_io_finish();

	return res;
}

static int ata_device_identify(int device, void *info)
{
	int res;

	if(!(res = ata_io_start(info, 1, 0, 0, 0, 0, 0, (device << 4) & 0xffff, ATA_C_IDENTIFY_DEVICE))) res=ata_io_finish();

	return res;
}

static int ata_device_pkt_identify(int device, void *info)
{
	int res;

	res = ata_io_start(info, 1, 0, 0, 0, 0, 0, (device << 4) & 0xffff, ATA_C_IDENTIFY_PACKET_DEVICE);
	if (!res)
		return ata_io_finish();
	return res;
}

/* Export 14 */
int ata_device_sce_identify_drive(int device, void *data)
{
	int res;

	if(!(res = ata_io_start(data, 1, ATA_SCE_IDENTIFY_DRIVE, 0, 0, 0, 0, (device << 4) & 0xffff, ATA_C_SCE_SECURITY_CONTROL))) res=ata_io_finish();

	return res;
}

static int ata_device_smart_enable(int device)
{
	int res;

	if(!(res = ata_io_start(NULL, 1, ATA_S_SMART_ENABLE_OPERATIONS, 0, 0, 0x4f, 0xc2, (device << 4) & 0xffff, ATA_C_SMART))) res=ata_io_finish();

	return res;
}

/* Export 16 */
int ata_device_smart_save_attr(int device)
{
	int res;

	if(!(res = ata_io_start(NULL, 1, ATA_S_SMART_SAVE_ATTRIBUTE_VALUES, 0, 0, 0x4f, 0xc2, (device << 4) & 0xffff, ATA_C_SMART))) res=ata_io_finish();

	return res;
}

/* Export 15 */
int ata_device_smart_get_status(int device)
{
	USE_ATA_REGS;
	int res;

	res = ata_io_start(NULL, 1, ATA_S_SMART_RETURN_STATUS, 0, 0, 0x4f, 0xc2, (device << 4) & 0xffff, ATA_C_SMART);
	if (res)
		return res;

	res = ata_io_finish();
	if (res)
		return res;

	/* Check to see if the report exceeded the threshold.  */
	if (((ata_hwport->r_lcyl&0xFF) != 0x4f) || ((ata_hwport->r_hcyl&0xFF) != 0xc2)) {
		M_PRINTF("Error: SMART report exceeded threshold.\n");
		return 1;
	}

	return res;
}

/* Set features - set transfer mode.  */
static int ata_device_set_transfer_mode(int device, int type, int mode)
{
	int res;

	res = ata_io_start(NULL, 1, 3, (type|mode) & 0xff, 0, 0, 0, (device << 4) & 0xffff, ATA_C_SET_FEATURES);
	if (res)
		return res;

	res = ata_io_finish();
	if (res)
		return res;

	switch(type){
#ifdef ATA_MWDMA_MODES
		case ATA_XFER_MODE_MDMA:
			ata_multiword_dma_mode(mode);
			break;
#endif
		case ATA_XFER_MODE_UDMA:
			ata_ultra_dma_mode(mode);
			break;
		case ATA_XFER_MODE_PIO:
			ata_pio_mode(mode);
			break;
	}

	return 0;
}

/* Export 9 */
/* Note: this can only support DMA modes, due to the commands issued. */
int ata_device_sector_io(int device, void *buf, u32 lba, u32 nsectors, int dir)
{
	USE_SPD_REGS;
	int res = 0, retries;
	u16 sector, lcyl, hcyl, select, command, len;

	while (res == 0 && nsectors > 0) {
		/* Variable lba is only 32 bits so no change for lcyl and hcyl.  */
		lcyl = (lba >> 8) & 0xff;
		hcyl = (lba >> 16) & 0xff;

		if (atad_devinfo[device].lba48) {
			/* Setup for 48-bit LBA.  */
			len = (nsectors > 65536) ? 65536 : nsectors;

			/* Combine bits 24-31 and bits 0-7 of lba into sector.  */
			sector = ((lba >> 16) & 0xff00) | (lba & 0xff);
			/* In v1.04, LBA was enabled here.  */
			select = (device << 4) & 0xffff;
			command = (dir == 1) ? ATA_C_WRITE_DMA_EXT : ATA_C_READ_DMA_EXT;
		} else {
			/* Setup for 28-bit LBA.  */
			len = (nsectors > 256) ? 256 : nsectors;
			sector = lba & 0xff;
			/* In v1.04, LBA was enabled here.  */
			select = ((device << 4) | ((lba >> 24) & 0xf)) & 0xffff;
			command = (dir == 1) ? ATA_C_WRITE_DMA : ATA_C_READ_DMA;
		}

		for(retries = 3; retries > 0; retries--) {
#ifdef ATA_GAMESTAR_WORKAROUND
			/* Due to the retry loop, put this call (for the GameStar workaround) here instead of the old location. */
			if (ata_gamestar_workaround)
				ata_set_dir(dir);
#endif

			if ((res = ata_io_start(buf, len, 0, len, sector, lcyl, hcyl, select, command)) != 0)
				break;

#ifdef ATA_GAMESTAR_WORKAROUND
			if (!ata_gamestar_workaround)
				ata_set_dir(dir);
#else
			/* Set up (part of) the transfer here. In v1.04, this was called at the top of the outer loop. */
			ata_set_dir(dir);
#endif

			res = ata_io_finish();

			/* In v1.04, this was not done. Neither was there a mechanism to retry if a non-permanent error occurs. */
			SPD_REG16(SPD_R_IF_CTRL) &= ~SPD_IF_DMA_ENABLE;

			if(res != ATA_RES_ERR_ICRC)
				break;
		}

		buf = (void*)((u8 *)buf + len * 512);
		lba += len;
		nsectors -= len;
	}

	return res;
}

static void ata_get_security_status(int device, ata_devinfo_t *devinfo, u16 *param)
{
	if (ata_device_identify(device, param) == 0)
		devinfo[device].security_status = param[ATA_ID_SECURITY_STATUS];
}

/* Export 10 */
int ata_device_sce_sec_set_password(int device, void *password)
{
	ata_devinfo_t *devinfo = atad_devinfo;
	u16 *param = ata_param;
	int res;

	if (devinfo[device].security_status & ATA_F_SEC_ENABLED) return 0;

	memset(param, 0, 512);
	memcpy(param + 1, password, 32);

	res = ata_io_start(param, 1, ATA_SCE_SECURITY_SET_PASSWORD, 0, 0, 0, 0, (device << 4) & 0xffff, ATA_C_SCE_SECURITY_CONTROL);
	if (res == 0)
		res = ata_io_finish();

	ata_get_security_status(device, devinfo, param);
	return res;
}

/* Export 11 */
int ata_device_sce_sec_unlock(int device, void *password)
{
	ata_devinfo_t *devinfo = atad_devinfo;
	u16 *param = ata_param;
	int res;

	if (!(devinfo[device].security_status & ATA_F_SEC_LOCKED)) return 0;

	memset(param, 0, 512);
	memcpy(param + 1, password, 32);

	if ((res = ata_io_start(param, 1, ATA_SCE_SECURITY_UNLOCK, 0, 0, 0, 0, (device << 4) & 0xffff, ATA_C_SCE_SECURITY_CONTROL)) != 0)
		return res;
	if ((res = ata_io_finish()) != 0)
		return res;

	/* Check to see if the drive was actually unlocked.  */
	ata_get_security_status(device, devinfo, param);
	if (devinfo[device].security_status & ATA_F_SEC_LOCKED)
		return ATA_RES_ERR_LOCKED;

	return 0;
}

/* Export 12 */
int ata_device_sce_sec_erase(int device)
{
	ata_devinfo_t *devinfo = atad_devinfo;
	int res;

	if (!(devinfo[device].security_status & ATA_F_SEC_ENABLED) || !(devinfo[device].security_status & ATA_F_SEC_LOCKED)) return 0;

	/* First send the mandatory ERASE PREPARE command.  */
	if ((res = ata_io_start(NULL, 1, ATA_SCE_SECURITY_ERASE_PREPARE, 0, 0, 0, 0, (device << 4) & 0xffff, ATA_C_SCE_SECURITY_CONTROL)) != 0)
		goto finish;
	if ((res = ata_io_finish()) != 0)
		goto finish;

	if ((res = ata_io_start(NULL, 1, ATA_SCE_SECURITY_ERASE_UNIT, 0, 0, 0, 0, (device << 4) & 0xffff, ATA_C_SCE_SECURITY_CONTROL)) == 0)
		res = ata_io_finish();

finish:
	ata_get_security_status(device, devinfo, NULL);
	return res;
}

static void ata_device_probe(ata_devinfo_t *devinfo)
{
	USE_ATA_REGS;
	u16 nsector, sector, lcyl, hcyl, select;

	devinfo->exists = 0;
	devinfo->has_packet = 2;

	if (ata_hwport->r_control & 0x88)
		return;

	nsector = ata_hwport->r_nsector & 0xff;
	sector = ata_hwport->r_sector & 0xff;
	lcyl = ata_hwport->r_lcyl & 0xff;
	hcyl = ata_hwport->r_hcyl & 0xff;
	select = ata_hwport->r_select;

	if ((nsector != 1) || (sector != 1))
		return;
	devinfo->exists = 1;

	if ((lcyl == 0x00) && (hcyl == 0x00))
		devinfo->has_packet = 0;
	else if ((lcyl == 0x14) && (hcyl == 0xeb))
		devinfo->has_packet = 1;

	/* Seems to be for ensuring that there is a device connected.
		Not sure why this has to be done, but is present in v2.4.  */
	ata_hwport->r_lcyl = 0x55;
	ata_hwport->r_hcyl = 0xaa;
	lcyl = ata_hwport->r_lcyl & 0xff;
	hcyl = ata_hwport->r_hcyl & 0xff;

	if((lcyl != 0x55) || (hcyl != 0xaa))
		devinfo->exists = 0;
}

static int ata_init_devices(ata_devinfo_t *devinfo)
{
	USE_ATA_REGS;
	int i, res;

	if((res = ata_reset_devices()) != 0)
		return res;

	ata_device_probe(&devinfo[0]);
	if (!devinfo[0].exists) {
		M_PRINTF("Error: Unable to detect HDD 0.\n");
		devinfo[1].exists = 0;
		return ATA_RES_ERR_NODEV;	//Returns 0 in v1.04.
	}

	/* If there is a device 1, grab it's info too.  */
	if ((res = ata_device_select(1)) != 0)
		return res;
	if (ata_hwport->r_control & 0xff)
		ata_device_probe(&devinfo[1]);
	else
		devinfo[1].exists = 0;

	ata_pio_mode(0);	//PIO mode is set here, in late ATAD versions.

	for (i = 0; i < 2; i++) {
		if (!devinfo[i].exists)
			continue;

		/* Send the IDENTIFY DEVICE command. if it doesn't succeed
		   devinfo is disabled.  */
		if (!devinfo[i].has_packet) {
			res = ata_device_identify(i, ata_param);
			devinfo[i].exists = (res == 0);
		} else if (devinfo[i].has_packet == 1) {
			/* If it's a packet device, send the IDENTIFY PACKET
			   DEVICE command.  */
			res = ata_device_pkt_identify(i, ata_param);
			devinfo[i].exists = (res == 0);
		}
		/* Otherwise, do nothing if has_packet = 2. */

		/* This next section is HDD-specific: if no device or it's a
		   packet (ATAPI) device, we're done.  */
		if (!devinfo[i].exists || devinfo[i].has_packet)
			continue;

		/* This section is to detect whether the HDD supports 48-bit LBA
		   (IDENITFY DEVICE bit 10 word 83) and get the total sectors from
		   either words(61:60) for 28-bit or words(103:100) for 48-bit.  */
		if (!ata_disable_lba48 && (ata_param[ATA_ID_COMMAND_SETS_SUPPORTED] & 0x0400)) {
			devinfo[i].lba48 = 1;
			/* I don't think anyone would use a >2TB HDD but just in case.  */
			if (ata_param[ATA_ID_48BIT_SECTOTAL_HI]) {
				devinfo[i].total_sectors = 0xffffffff;
			} else {
				devinfo[i].total_sectors =
					(ata_param[ATA_ID_48BIT_SECTOTAL_MI] << 16)|
					ata_param[ATA_ID_48BIT_SECTOTAL_LO];
			}
		} else {
			devinfo[i].lba48 = 0;
			devinfo[i].total_sectors = (ata_param[ATA_ID_SECTOTAL_HI] << 16)|
				ata_param[ATA_ID_SECTOTAL_LO];
		}
		devinfo[i].security_status = ata_param[ATA_ID_SECURITY_STATUS];

		/* Ultra DMA mode 4.  */
		ata_device_set_transfer_mode(i, ATA_XFER_MODE_UDMA, 4);
		ata_device_smart_enable(i);
		/* Set standby timer to 21min 15s.  */
		ata_device_idle(i, 0xff);

		/* Call the proprietary identify command. */
#ifdef ATA_SCE_AUTH_HDD
		if(ata_device_sce_identify_drive(i, ata_param) != 0) {
			M_PRINTF("error: This is not SCE genuine HDD.\n");
			memset(&devinfo[i], 0, sizeof(devinfo[i]));
		}
#endif
	}
	return 0;
}

/* Export 4 */
ata_devinfo_t * ata_get_devinfo(int device)
{
	if(!ata_devinfo_init){
		ata_devinfo_init = 1;
		if(ata_bus_reset() || ata_init_devices(atad_devinfo)) return NULL;
	}

	return &atad_devinfo[device];
}

static void ata_set_dir(int dir)
{
	USE_SPD_REGS;
	u16 val;

	SPD_REG16(0x38) = 3;
	val = SPD_REG16(SPD_R_IF_CTRL) & 1;
	val |= (dir == ATA_DIR_WRITE) ? 0x4c : 0x4e;
	SPD_REG16(SPD_R_IF_CTRL) = val;
#ifdef ATA_GAMESTAR_WORKAROUND
	SPD_REG16(SPD_R_XFR_CTRL) = dir | (ata_gamestar_workaround ? 0x86 : 0x6);
#else
	SPD_REG16(SPD_R_XFR_CTRL) = dir | 0x6;	//In v1.04, DMA was enabled here (0x86 instead of 0x6)
#endif
}

static void ata_pio_mode(int mode)
{
	USE_SPD_REGS;
#ifdef ATA_ALL_PIO_MODES
	u16 val;

	switch (mode) {
		case 1:
			val = 0x72;
			break;
		case 2:
			val = 0x32;
			break;
		case 3:
			val = 0x24;
			break;
		case 4:
			val = 0x23;
			break;
		default:
			val = 0x92;
	}

	SPD_REG16(SPD_R_PIO_MODE) = val;
#else
	/* In the late ATAD versions, PIO mode 0 is always used if PIO mode is utilized. */
	SPD_REG16(SPD_R_PIO_MODE) = 0x92;
#endif
}

#ifdef ATA_MWDMA_MODES
static void ata_multiword_dma_mode(int mode)
{
	USE_SPD_REGS;
	u16 val;

	switch(mode){
		case 1:
			val = 0x45;
			break;
		case 2:
			val = 0x24;
			break;
		default:
			val = 0xff;
	}

	SPD_REG16(SPD_R_MWDMA_MODE) = val;
	SPD_REG16(SPD_R_IF_CTRL) = (SPD_REG16(SPD_R_IF_CTRL) & 0xfffe)|0x48;
}
#endif

static void ata_ultra_dma_mode(int mode)
{
	USE_SPD_REGS;
	u16 val;

	switch (mode)
	{
		case 1:
			val = 0x85;
			break;
		case 2:
			val = 0x63;
			break;
		case 3:
			val = 0x62;
			break;
		case 4:
			val = 0x61;
			break;
		default:
			val = 0xa7;
	}

	SPD_REG16(SPD_R_UDMA_MODE) = val;
	SPD_REG16(SPD_R_IF_CTRL) |= 0x49;
}

/* Export 18 */
int ata_device_idle_immediate(int device)
{
	int res;

	if (!(res = ata_io_start(NULL, 1, 0, 0, 0, 0, 0, (device << 4)&0xFFFF, ATA_C_IDLE_IMMEDIATE))) res = ata_io_finish();

	return res;
}

static int ata_device_standby_immediate(int device)
{
	int res;

	if (!(res = ata_io_start(NULL, 1, 0, 0, 0, 0, 0, (device << 4)&0xFFFF, ATA_C_STANDBY_IMMEDIATE))) res = ata_io_finish();

	return res;
}

static void ata_shutdown_cb(void)
{
	int i;

	for (i = 0; i < 2; i++)
	{
		if (atad_devinfo[i].exists)
			ata_device_standby_immediate(i);
	}
}


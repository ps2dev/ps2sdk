/*
  Copyright 2011, jimmikaelkael
  Licenced under Academic Free License version 3.0

  ATA Driver for the HD Pro Kit, based on original ATAD form ps2sdk:

  Copyright (c) 2003 Marcus R. Brown <mrbrown@0xd6.org>
  Licenced under Academic Free License version 2.0
  Review ps2sdk README & LICENSE files for further details.

  ATA device driver.
*/

#include <types.h>
#include <defs.h>
#include <irx.h>
#include <loadcore.h>
#include <intrman.h>
#include <thbase.h>
#include <thevent.h>
#include <stdio.h>
#include <sysclib.h>
#include <atad.h>

#include <atahw.h>

#define MODNAME "atad"
IRX_ID(MODNAME, 2, 4);

#define M_PRINTF(format, args...)	\
	printf(MODNAME ": " format, ## args)

#define BANNER "ATA device driver for HD Pro Kit %s\n"
#define VERSION "v1.0"

#define ATA_XFER_MODE_PIO	0x08

#define ATA_EV_TIMEOUT	1
#define ATA_EV_COMPLETE	2

// HD Pro Kit is mapping the 1st word in ROM0 seg as a main ATA controller,
// The pseudo ATA controller registers are accessed (input/ouput) by writing
// an id to the main ATA controller (id specific to HDpro, see registers id below).
#define HDPROreg_IO8	      (*(volatile unsigned char *)0xBFC00000)
#define HDPROreg_IO32	      (*(volatile unsigned int  *)0xBFC00000)

#define CDVDreg_STATUS        (*(volatile unsigned char *)0xBF40200A)

// Pseudo ATA controller registers id - Output
#define ATAreg_CONTROL_RD	0x68
#define ATAreg_SELECT_RD	0x70
#define ATAreg_STATUS_RD	0xf0
#define ATAreg_ERROR_RD		0x90
#define ATAreg_NSECTOR_RD	0x50
#define ATAreg_SECTOR_RD	0xd0
#define ATAreg_LCYL_RD 		0x30
#define ATAreg_HCYL_RD		0xb0
#define ATAreg_DATA_RD		0x41

// Pseudo ATA controller registers id - Input
#define ATAreg_CONTROL_WR	0x6a
#define ATAreg_SELECT_WR	0x72
#define ATAreg_COMMAND_WR	0xf2
#define ATAreg_FEATURE_WR	0x92
#define ATAreg_NSECTOR_WR	0x52
#define ATAreg_SECTOR_WR	0xd2
#define ATAreg_LCYL_WR 		0x32
#define ATAreg_HCYL_WR		0xb2
#define ATAreg_DATA_WR		0x12

static int ata_devinfo_init = 0;
static int ata_evflg = -1;

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
	{ATA_C_READ_SECTOR_EXT,0x83},
	{ATA_C_READ_DMA_EXT,0x84},
	{ATA_C_WRITE_SECTOR,3},
	{ATA_C_WRITE_LONG,8},
	{ATA_C_WRITE_SECTOR_EXT,0x83},
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

static int hdpro_io_active = 0;
static int intr_suspended = 0;
static int intr_state;

static int hdpro_io_start(void);
static int hdpro_io_finish(void);
static void hdpro_io_write(unsigned char cmd, unsigned short int val);
static int hdpro_io_read(unsigned char cmd);
static int ata_bus_reset(void);
static int ata_init_devices(ata_devinfo_t *devinfo);
static int gen_ata_wait_busy(int bits);

extern struct irx_export_table _exp_atad;

static unsigned int ata_alarm_cb(void *unused)
{
	iSetEventFlag(ata_evflg, ATA_EV_TIMEOUT);
	return 0;
}

static void suspend_intr(void)
{
	if (!intr_suspended) {
		CpuSuspendIntr(&intr_state);

		intr_suspended = 1;
	}
}

static void resume_intr(void)
{
	if (intr_suspended) {
		CpuResumeIntr(intr_state);

		intr_suspended = 0;
	}
}

static int ata_create_event_flag(void) {
	iop_event_t event;

	event.attr = EA_SINGLE;	//In v1.04, EA_MULTI was specified.
	event.bits = 0;
	return CreateEventFlag(&event);
}

int _start(int argc, char *argv[])
{
	int res = MODULE_NO_RESIDENT_END;

	printf(BANNER, VERSION);

	if (!hdpro_io_start()) {
		M_PRINTF("Failed to detect HD Pro, exiting.\n");
		goto out;
	}

	hdpro_io_finish();

	if ((ata_evflg = ata_create_event_flag()) < 0) {
		M_PRINTF("Couldn't create event flag, exiting.\n");
		goto out;
	}

	if ((res = RegisterLibraryEntries(&_exp_atad)) != 0) {
		M_PRINTF("Library is already registered, exiting.\n");
		goto out;
	}

	res = MODULE_RESIDENT_END;
	M_PRINTF("Driver loaded.\n");

out:
	return res;
}

int shutdown(void)
{
	return 0;
}

/* Export 8 */
int ata_get_error(void)
{
	return hdpro_io_read(ATAreg_ERROR_RD) & 0xff;
}

#define ATA_WAIT_BUSY		0x80
#define ATA_WAIT_BUSBUSY	0x88

#define ata_wait_busy()		gen_ata_wait_busy(ATA_WAIT_BUSY)
#define ata_wait_bus_busy()	gen_ata_wait_busy(ATA_WAIT_BUSBUSY)

/* 0x80 for busy, 0x88 for bus busy.
	In the original ATAD, the busy and bus-busy functions were separate, but similar.  */
static int gen_ata_wait_busy(int bits)
{
	int i, didx, delay;
	int res = 0;

	for (i = 0; i < 80; i++) {

		hdpro_io_start();

		u16 r_control = hdpro_io_read(ATAreg_CONTROL_RD);

		hdpro_io_finish();

		if (r_control == 0xffff)	//Differs from the normal ATAD here.
			return ATA_RES_ERR_TIMEOUT;

		if (!(r_control & bits))
			goto out;

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

	res = ATA_RES_ERR_TIMEOUT;
	M_PRINTF("Timeout while waiting on busy (0x%02x).\n", bits);

out:
	hdpro_io_start();

	return res;
}

//Must be called before any I/O is done with HDPro
static int hdpro_io_start(void)
{
	if (hdpro_io_active)
		return 0;

	hdpro_io_active = 0;

	suspend_intr();

	// HD Pro IO start commands sequence
	HDPROreg_IO8 = 0x72;
	CDVDreg_STATUS = 0;
	HDPROreg_IO8 = 0x34;
	CDVDreg_STATUS = 0;
	HDPROreg_IO8 = 0x61;
	CDVDreg_STATUS = 0;
	unsigned int res = HDPROreg_IO8;
	CDVDreg_STATUS = 0;

	resume_intr();

	// check result
	if ((res & 0xff) == 0xe7)
		hdpro_io_active = 1;

	return hdpro_io_active;
}

//Must be called after I/O is done with HDPro
static int hdpro_io_finish(void)
{
	if (!hdpro_io_active)
		return 0;

	suspend_intr();

	// HD Pro IO finish commands sequence
	HDPROreg_IO8 = 0xf3;
	CDVDreg_STATUS = 0;

	resume_intr();

	DelayThread(200);

	if (HDPROreg_IO32 == 0x401a7800) // check the 1st in ROM0 seg get
		hdpro_io_active = 0;	 // back to it's original state

	return hdpro_io_active ^ 1;
}

static void hdpro_io_write(u8 cmd, u16 val)
{
	suspend_intr();

	// IO write to HD Pro
	HDPROreg_IO8 = cmd;
	CDVDreg_STATUS = 0;
	HDPROreg_IO8 = val;
	CDVDreg_STATUS = 0;
	HDPROreg_IO8 = (val & 0xffff) >> 8;
	CDVDreg_STATUS = 0;

	resume_intr();
}

static int hdpro_io_read(u8 cmd)
{
	suspend_intr();

	// IO read from HD Pro
	HDPROreg_IO8 = cmd;
	CDVDreg_STATUS = 0;
	unsigned int res0 = HDPROreg_IO8;
	CDVDreg_STATUS = 0;
	unsigned int res1 = HDPROreg_IO8;
	CDVDreg_STATUS = 0;
	res0 = (res0 & 0xff) | (res1 << 8);

	resume_intr();

	return res0 & 0xffff;
}

/* Reset the ATA controller/bus.  */
static int ata_bus_reset(void)
{
	suspend_intr();

	// HD Pro IO initialization commands sequence
	HDPROreg_IO8 = 0x13;
	CDVDreg_STATUS = 0;
	HDPROreg_IO8 = 0x00;
	CDVDreg_STATUS = 0;

	resume_intr();

	DelayThread(100);

	suspend_intr();

	HDPROreg_IO8 = 0x13;
	CDVDreg_STATUS = 0;
	HDPROreg_IO8 = 0x01;
	CDVDreg_STATUS = 0;

	resume_intr();

	DelayThread(3000);

	return ata_wait_busy();
}

static int ata_device_select(int device)
{
	int res;

	if ((res = ata_wait_bus_busy()) < 0)
		return res;

	/* If the device was already selected, nothing to do.  */
	if (((hdpro_io_read(ATAreg_SELECT_RD) >> 4) & 1) == device)
		return 0;

	/* Select the device.  */
	hdpro_io_write(ATAreg_SELECT_WR, (device & 1) << 4);
	res = hdpro_io_read(ATAreg_CONTROL_RD);

	return ata_wait_bus_busy();
}

/* Export 6 */
int ata_io_start(void *buf, u32 blkcount, u16 feature, u16 nsector, u16 sector, u16 lcyl, u16 hcyl, u16 select, u16 command)
{
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

	/* For the SMART commands, we need to search on the subcommand
	specified in the feature register.  */
	if (command == ATA_C_SMART) {
		cmd_table = smart_cmd_table;
		cmd_table_size = SMART_CMD_TABLE_SIZE;
		searchcmd = feature;
	} else {
		cmd_table = ata_cmd_table;
		cmd_table_size = ATA_CMD_TABLE_SIZE;
		searchcmd = command & 0xff;
	}

	type = 0;
	for (i = 0; i < cmd_table_size; i++) {
		if (searchcmd == cmd_table[i].command) {
			type = cmd_table[i].type;
			break;
		}
	}

	if (!(atad_cmd_state.type = type & 0x7F))
		return ATA_RES_ERR_CMD;

	atad_cmd_state.buf = buf;
	atad_cmd_state.blkcount = blkcount;

	/* Check that the device is ready if this the appropiate command.  */
	if (!(hdpro_io_read(ATAreg_CONTROL_RD) & 0x40)) {
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
	switch (type & 0x7F) {
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

		if ((res = SetAlarm(&cmd_timeout, &ata_alarm_cb, NULL)) < 0)
			return res;
	}

	/* Enable the command completion interrupt.  */
	suspend_intr();
	HDPROreg_IO8 = 0x21;
	CDVDreg_STATUS = 0;
	unsigned int dummy __attribute__((unused)) = HDPROreg_IO8;
	CDVDreg_STATUS = 0;
	resume_intr();
	dummy = 0;

	/* Finally!  We send off the ATA command with arguments.  */
	hdpro_io_write(ATAreg_CONTROL_WR, (using_timeout == 0) << 1);

	if(type&0x80){	//For the sake of achieving improved performance, write the registers twice only if required!
		/* 48-bit LBA requires writing to the address registers twice,
		   24 bits of the LBA address is written each time.
		   Writing to registers twice does not affect 28-bit LBA since
		   only the latest data stored in address registers is used.  */
		hdpro_io_write(ATAreg_FEATURE_WR, (feature & 0xffff) >> 8);
		hdpro_io_write(ATAreg_NSECTOR_WR, (nsector & 0xffff) >> 8);
		hdpro_io_write(ATAreg_SECTOR_WR, (sector & 0xffff) >> 8);
		hdpro_io_write(ATAreg_LCYL_WR, (lcyl & 0xffff) >> 8);
		hdpro_io_write(ATAreg_HCYL_WR, (hcyl & 0xffff) >> 8);
	}

	hdpro_io_write(ATAreg_FEATURE_WR, feature & 0xff);
	hdpro_io_write(ATAreg_NSECTOR_WR, nsector & 0xff);
	hdpro_io_write(ATAreg_SECTOR_WR, sector & 0xff);
	hdpro_io_write(ATAreg_LCYL_WR, lcyl & 0xff);
	hdpro_io_write(ATAreg_HCYL_WR, hcyl & 0xff);

	hdpro_io_write(ATAreg_SELECT_WR, (select | ATA_SEL_LBA) & 0xff);	//In v1.04, LBA was enabled in the ata_device_sector_io function.
	hdpro_io_write(ATAreg_COMMAND_WR, command & 0xff);

	return 0;
}

/* Do a PIO transfer, to or from the device.  */
static int ata_pio_transfer(ata_cmd_state_t *cmd_state)
{
	u8 *buf8;
	u16 *buf16;
	int i, type;
	int res = 0, chk = 0;
	u16 status = hdpro_io_read(ATAreg_STATUS_RD);

	if (status & ATA_STAT_ERR) {
		M_PRINTF("Error: Command error: status 0x%02x, error 0x%02x.\n", status, ata_get_error());
		return ATA_RES_ERR_IO;
	}

	/* DRQ must be set (data request).  */
	if (!(status & ATA_STAT_DRQ))
		return ATA_RES_ERR_NODATA;

	type = cmd_state->type;

	if (type == 3 || type == 8) {
		/* PIO data out */
		buf16 = cmd_state->buf16;

		HDPROreg_IO8 = 0x43;
		CDVDreg_STATUS = 0;

		for (i = 0; i < 256; i++) {
			u16 r_data = *buf16;
			hdpro_io_write(ATAreg_DATA_WR, r_data);
			chk ^= r_data + i;
			cmd_state->buf = ++buf16;
		}

		u16 out = hdpro_io_read(ATAreg_DATA_RD) & 0xffff;
		if (out != (chk & 0xffff))
			return ATA_RES_ERR_IO;

		if (cmd_state->type == 8) {
			buf8 = cmd_state->buf8;
			for (i = 0; i < 4; i++) {
				hdpro_io_write(ATAreg_DATA_WR, *buf8);
				cmd_state->buf = ++buf8;
			}
		}

	} else if (type == 2) {
		/* PIO data in  */
		buf16 = cmd_state->buf16;

		suspend_intr();

		HDPROreg_IO8 = 0x53;
		CDVDreg_STATUS = 0;
		CDVDreg_STATUS = 0;

		for (i = 0; i < 256; i++) {

			unsigned int res0 = HDPROreg_IO8;
			CDVDreg_STATUS = 0;
			unsigned int res1 = HDPROreg_IO8;
			CDVDreg_STATUS = 0;

			res0 = (res0 & 0xff) | (res1 << 8);
			chk ^= res0 + i;

			*buf16 = res0 & 0xffff;
			cmd_state->buf16 = ++buf16;
		}

		HDPROreg_IO8 = 0x51;
		CDVDreg_STATUS = 0;
		CDVDreg_STATUS = 0;

		resume_intr();

		u16 r_data = hdpro_io_read(ATAreg_DATA_RD) & 0xffff;
		if (r_data != (chk & 0xffff))
			return ATA_RES_ERR_IO;
	}

	return res;
}

/* Export 5 */
int ata_reset_devices(void)
{
	if (hdpro_io_read(ATAreg_CONTROL_RD) & 0x80)
		return ATA_RES_ERR_NOTREADY;

	/* Disables device interrupt assertion and asserts SRST. */
	hdpro_io_write(ATAreg_CONTROL_WR, 6);
	DelayThread(100);

	/* Disable ATA interrupts.  */
	hdpro_io_write(ATAreg_CONTROL_WR, 2);
	DelayThread(3000);

	return ata_wait_busy();
}

static void ata_device_probe(ata_devinfo_t *devinfo)
{
	u16 nsector, lcyl, hcyl, sector, select;

	devinfo->exists = 0;
	devinfo->has_packet = 2;

	if (hdpro_io_read(ATAreg_CONTROL_RD) & 0x88)
		return;

	nsector = hdpro_io_read(ATAreg_NSECTOR_RD) & 0xff;
	sector = hdpro_io_read(ATAreg_SECTOR_RD) & 0xff;
	lcyl = hdpro_io_read(ATAreg_LCYL_RD) & 0xff;
	hcyl = hdpro_io_read(ATAreg_HCYL_RD) & 0xff;
	select = hdpro_io_read(ATAreg_SELECT_RD) & 0xff;

	if (nsector != 1)	//Original did not check sector, although it checked nsector.
		return;
	devinfo->exists = 1;

	if ((lcyl == 0x00) && (hcyl == 0x00))
		devinfo->has_packet = 0;
	else if ((lcyl == 0x14) && (hcyl == 0xeb))
		devinfo->has_packet = 1;
}

/* Export 17 */
int ata_device_flush_cache(int device)
{
	int res;

	if (!hdpro_io_start())
		return -1;

	if(!(res = ata_io_start(NULL, 1, 0, 0, 0, 0, 0, (device << 4) & 0xffff, atad_devinfo[device].lba48?ATA_C_FLUSH_CACHE_EXT:ATA_C_FLUSH_CACHE))) res=ata_io_finish();

	if (!hdpro_io_finish())
		return -2;

	return res;
}

/* Export 13 */
int ata_device_idle(int device, int period)
{
	int res;

	res = ata_io_start(NULL, 1, 0, period & 0xff, 0, 0, 0, (device << 4) & 0xffff, ATA_C_IDLE);
	if (res)
		return res;

	return ata_io_finish();
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

	return 0;
}

static int ata_device_identify(int device, void *info)
{
	int res;

	res = ata_io_start(info, 1, 0, 0, 0, 0, 0, (device << 4) & 0xffff, ATA_C_IDENTIFY_DEVICE);
	if (res)
		return res;

	return ata_io_finish();
}

static int ata_device_smart_enable(int device)
{
	int res;

	if(!(res = ata_io_start(NULL, 1, ATA_S_SMART_ENABLE_OPERATIONS, 0, 0, 0x4f, 0xc2, (device << 4) & 0xffff, ATA_C_SMART))) res=ata_io_finish();

	return res;
}

static int ata_init_devices(ata_devinfo_t *devinfo)
{
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
	if (hdpro_io_read(ATAreg_CONTROL_RD) & 0xff)
		ata_device_probe(&devinfo[1]);
	else
		devinfo[1].exists = 0;

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
			/*	Packet devices are not supported:

				The original HDPro driver issues the IDENTIFY PACKET DEVICE command,
				but does not export ata_io_start and ata_io_finish.
				This makes packet device support impossible. */
			res = -1;
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
		if (ata_param[ATA_ID_COMMAND_SETS_SUPPORTED] & 0x0400) {
			atad_devinfo[i].lba48 = 1;
			/* I don't think anyone would use a >2TB HDD but just in case.  */
			if (ata_param[ATA_ID_48BIT_SECTOTAL_HI]) {
				devinfo[i].total_sectors = 0xffffffff;
			} else {
				devinfo[i].total_sectors =
					(ata_param[ATA_ID_48BIT_SECTOTAL_MI] << 16)|
					ata_param[ATA_ID_48BIT_SECTOTAL_LO];
			}
		} else {
			atad_devinfo[i].lba48 = 0;
			devinfo[i].total_sectors = (ata_param[ATA_ID_SECTOTAL_HI] << 16)|
				ata_param[ATA_ID_SECTOTAL_LO];
		}
		devinfo[i].security_status = ata_param[ATA_ID_SECURITY_STATUS];

		/* PIO mode 0 (flow control).  */
		ata_device_set_transfer_mode(i, ATA_XFER_MODE_PIO, 0);
		ata_device_smart_enable(i);
		/* Disable idle timeout.  */
		ata_device_idle(i, 0);
	}
	return 0;
}

/* Export 7 */
int ata_io_finish(void)
{
	ata_cmd_state_t *cmd_state = &atad_cmd_state;
	u32 bits;
	int res = 0, type = cmd_state->type;
	u16 stat;

	if (type == 1 || type == 6) {	/* Non-data commands.  */

		while(1)
		{	//Unlike ATAD, poll until either the operation is done or a timeout occurs.
			suspend_intr();

			HDPROreg_IO8 = 0x21;
			CDVDreg_STATUS = 0;
			unsigned int ret = HDPROreg_IO8;
			CDVDreg_STATUS = 0;

			resume_intr();

			if (((ret & 0xff) & 1) != 0) {
				//Command completed.
				break;
			}

			//PollEventFlag is used instead of WaitEventFlag, as there is no hardware interrupt.
			PollEventFlag(ata_evflg, ATA_EV_TIMEOUT | ATA_EV_COMPLETE, WEF_CLEAR|WEF_OR, &bits);
			if (bits & ATA_EV_TIMEOUT) {	/* Timeout.  */
				M_PRINTF("Error: ATA timeout on a non-data command.\n");
				return ATA_RES_ERR_TIMEOUT;
			}

			DelayThread(500);
		}

	} else if (type == 4) {		/* DMA.  */
			M_PRINTF("Error: DMA mode not implemented.\n");
			res = ATA_RES_ERR_TIMEOUT;
	} else {			/* PIO transfers.  */
		stat = hdpro_io_read(ATAreg_CONTROL_RD);
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
	if (hdpro_io_read(ATAreg_STATUS_RD) & ATA_STAT_BUSY)
		res = ata_wait_busy();
	if ((stat = hdpro_io_read(ATAreg_STATUS_RD)) & ATA_STAT_ERR) {
		M_PRINTF("Error: Command error: status 0x%02x, error 0x%02x.\n", stat, ata_get_error());
		res = ATA_RES_ERR_IO;
	}

finish:
	/* The command has completed (with an error or not), so clean things up.  */
	CancelAlarm(&ata_alarm_cb, NULL);

	return res;
}

/* Export 9 */
int ata_device_sector_io(int device, void *buf, u32 lba, u32 nsectors, int dir)
{
	int res = 0;
	u16 sector, lcyl, hcyl, select, command, len;

	if (!hdpro_io_start())
		return -1;

	while (nsectors) {
		len = (nsectors > 256) ? 256 : nsectors;

		/* Variable lba is only 32 bits so no change for lcyl and hcyl.  */
		lcyl = (lba >> 8) & 0xff;
		hcyl = (lba >> 16) & 0xff;

		if (atad_devinfo[device].lba48) {
			/* Setup for 48-bit LBA.  */
			/* Combine bits 24-31 and bits 0-7 of lba into sector.  */
			sector = ((lba >> 16) & 0xff00) | (lba & 0xff);
			/* In v1.04, LBA was enabled here.  */
			select = (device << 4) & 0xffff;
			command = (dir == 1) ? ATA_C_WRITE_SECTOR_EXT : ATA_C_READ_SECTOR_EXT;
		} else {
			/* Setup for 28-bit LBA.  */
			sector = lba & 0xff;
			/* In v1.04, LBA was enabled here.  */
			select = ((device << 4) | ((lba >> 24) & 0xf)) & 0xffff;
			command = (dir == 1) ? ATA_C_WRITE_SECTOR : ATA_C_READ_SECTOR;
		}

		if ((res = ata_io_start(buf, len, 0, len, sector, lcyl,
					hcyl, select, command)) != 0)
			continue;
		if ((res = ata_io_finish()) != 0)
			continue;

		buf = (void*)((u8 *)buf + len * 512);
		lba += len;
		nsectors -= len;
	}

	if (!hdpro_io_finish())
		return -2;

	return res;
}

/* Export 4 */
ata_devinfo_t * ata_get_devinfo(int device)
{
	if(!ata_devinfo_init){
		ata_devinfo_init = 1;

		if (!hdpro_io_start())
			return NULL;

		HDPROreg_IO8 = 0xe3;
		CDVDreg_STATUS = 0;

		if ((ata_bus_reset() != 0) || (ata_init_devices(atad_devinfo) != 0) || (!hdpro_io_finish()))
			return NULL;
	}

	return &atad_devinfo[device];
}

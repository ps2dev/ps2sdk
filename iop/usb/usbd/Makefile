# _____     ___ ____     ___ ____
#  ____|   |    ____|   |        | |____|
# |     ___|   |____ ___|    ____| |    \    PS2DEV Open Source Project.
#-----------------------------------------------------------------------
# Copyright ps2dev - http://www.ps2dev.org
# Licenced under Academic Free License version 2.0
# Review ps2sdk README & LICENSE files for further details.

IOP_PREFER_GPOPT = 16384

IOP_IMPORT_INCS += \
	system/intrman \
	system/loadcore \
	system/stdio \
	system/sysclib \
	system/sysmem \
	system/threadman

IOP_OBJS = \
	hub.o \
	mem.o \
	timer.o \
	endpoint.o \
	io_request.o \
	hub_resets.o \
	td_queue.o \
	hcd.o \
	usbd_sys.o \
	usbd_api.o \
	usbd_main.o \
	report_descriptor_init.o \
	device_driver.o \
	device.o \
	imports.o \
	exports.o

include $(PS2SDKSRC)/Defs.make
include $(PS2SDKSRC)/iop/Rules.bin.make
include $(PS2SDKSRC)/iop/Rules.make
include $(PS2SDKSRC)/iop/Rules.release

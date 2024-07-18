# _____     ___ ____     ___ ____
#  ____|   |    ____|   |        | |____|
# |     ___|   |____ ___|    ____| |    \    PS2DEV Open Source Project.
#-----------------------------------------------------------------------
# Copyright 2001-2004, ps2dev - http://www.ps2dev.org
# Licenced under Academic Free License version 2.0
# Review ps2sdk README & LICENSE files for further details.

MODHOOK_PATH = $(PS2SDKSRC)/iop/utilities/modhook/
IOP_INCS += -I$(PS2SDKSRC)/iop/fs/bdm/include -I$(PS2SDKSRC)/iop/system/sio2man/include -I$(MODHOOK_PATH)include
IOP_LIB_ARCHIVES = $(MODHOOK_PATH)lib/libmodhook.a

IOP_OBJS += spi_sdcard_crc7.o spi_sdcard_driver.o crc16.o sio2man_hook.o mx4sio.o imports.o

IOP_LDFLAGS = -lgcc


$(MODHOOK_PATH)lib/libmodhook.a:
	$(MAKEREC) $(MODHOOK_PATH)

.NOTPARALLEL:: \
	$(IOP_LIB_ARCHIVES)


include $(PS2SDKSRC)/Defs.make
include $(PS2SDKSRC)/iop/Rules.bin.make
include $(PS2SDKSRC)/iop/Rules.make
include $(PS2SDKSRC)/iop/Rules.release

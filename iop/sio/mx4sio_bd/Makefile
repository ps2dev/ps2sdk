# _____     ___ ____     ___ ____
#  ____|   |    ____|   |        | |____|
# |     ___|   |____ ___|    ____| |    \    PS2DEV Open Source Project.
#-----------------------------------------------------------------------
# Copyright 2001-2004, ps2dev - http://www.ps2dev.org
# Licenced under Academic Free License version 2.0
# Review ps2sdk README & LICENSE files for further details.

IOP_IMPORT_INCS += \
	fs/bdm \
	system/dmacman \
	system/intrman \
	system/loadcore \
	system/sio2man \
	system/stdio \
	system/sysclib \
	system/threadman

IOP_LIBS += -lgcc

IOP_OBJS += spi_sdcard_crc7.o spi_sdcard_driver.o crc16.o ioplib.o sio2man_hook.o mx4sio.o imports.o

include $(PS2SDKSRC)/Defs.make
include $(PS2SDKSRC)/iop/Rules.bin.make
include $(PS2SDKSRC)/iop/Rules.make
include $(PS2SDKSRC)/iop/Rules.release

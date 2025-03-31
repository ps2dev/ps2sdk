# _____     ___ ____     ___ ____
#  ____|   |    ____|   |        | |____|
# |     ___|   |____ ___|    ____| |    \    PS2DEV Open Source Project.
#-----------------------------------------------------------------------
# Copyright 2001-2004, ps2dev - http://www.ps2dev.org
# Licenced under Academic Free License version 2.0
# Review ps2sdk README & LICENSE files for further details.

# IOP_CFLAGS += -DDEBUG

IOP_CFLAGS += -mno-check-zero-division # -DSIF_CALLBACKS_12_13

IOP_IMPORT_INCS += \
	cdvd/cdvdman \
	fs/bdm \
	iLink/iLinkman \
	system/intrman \
	system/ioman \
	system/loadcore \
	system/sifcmd \
	system/stdio \
	system/sysclib \
	system/sysmem \
	system/threadman

IOP_LIBS += -lgcc

IOP_OBJS = sbp2_driver.o imports.o main.o scsi.o

include $(PS2SDKSRC)/Defs.make
include $(PS2SDKSRC)/iop/Rules.bin.make
include $(PS2SDKSRC)/iop/Rules.make
include $(PS2SDKSRC)/iop/Rules.release

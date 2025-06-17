# _____     ___ ____     ___ ____
#  ____|   |    ____|   |        | |____|
# |     ___|   |____ ___|    ____| |    \    PS2DEV Open Source Project.
#-----------------------------------------------------------------------
# Copyright 2001-2004, ps2dev - http://www.ps2dev.org
# Licenced under Academic Free License version 2.0
# Review ps2sdk README & LICENSE files for further details.

IOP_BIN_ALTNAMES ?= rmman-2000.irx

IOP_IMPORT_INCS += \
	cdvd/cdvdman \
	sio/sio2man \
	system/intrman \
	system/iomanx \
	system/loadcore \
	system/sifcmd \
	system/sifman \
	system/stdio \
	system/threadman \
	system/vblank

IOP_OBJS = rmman.o exports.o imports.o

# Build the version of the rmman module that uses the SIRCS receiver connected to the Mechacon instead of the SIO2 dongle?
RMMAN_BUILDING_RMMAN2 ?= 0

# Build the version of the rmman module that uses the SIRCS receiver connected to the DVRP instead of the Mechacon?
RMMAN_BUILDING_RMMANX ?= 0

ifneq (x$(RMMAN_BUILDING_RMMAN2),x0)
IOP_CFLAGS += -DBUILDING_RMMAN2
endif

ifneq (x$(RMMAN_BUILDING_RMMANX),x0)
IOP_CFLAGS += -DBUILDING_RMMANX
endif
include $(PS2SDKSRC)/Defs.make
include $(PS2SDKSRC)/iop/Rules.bin.make
include $(PS2SDKSRC)/iop/Rules.make
include $(PS2SDKSRC)/iop/Rules.release

# _____     ___ ____     ___ ____
#  ____|   |    ____|   |        | |____|
# |     ___|   |____ ___|    ____| |    \    PS2DEV Open Source Project.
#-----------------------------------------------------------------------
# Copyright ps2dev - http://www.ps2dev.org
# Licenced under Academic Free License version 2.0
# Review ps2sdk README & LICENSE files for further details.

# Enable logging?
SIO2MAN_ENABLE_LOGGING ?= 0

# Build nano version?
SIO2MAN_ENABLE_NANO ?= 0

IOP_BIN_ALTNAMES ?= freesio2.irx rsio2man.irx sio2man-old.irx sio2man-1300.irx sio2man-1400.irx sio2man-2000.irx

IOP_IMPORT_INCS += \
	sio/sio2man \
	system/dmacman \
	system/intrman \
	system/ioman \
	system/loadcore \
	system/stdio \
	system/threadman

IOP_OBJS = sio2man.o log.o imports.o exports.o

ifneq (x$(SIO2MAN_ENABLE_LOGGING),x0)
IOP_CFLAGS += -DSIO2LOG
endif

ifneq (x$(SIO2MAN_ENABLE_NANO),x0)
IOP_CFLAGS += -DSIO2MAN_NANO
endif

include $(PS2SDKSRC)/Defs.make
include $(PS2SDKSRC)/iop/Rules.bin.make
include $(PS2SDKSRC)/iop/Rules.make
include $(PS2SDKSRC)/iop/Rules.release

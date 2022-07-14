# _____     ___ ____     ___ ____
#  ____|   |    ____|   |        | |____|
# |     ___|   |____ ___|    ____| |    \    PS2DEV Open Source Project.
#-----------------------------------------------------------------------
# Copyright 2001-2004, ps2dev - http://www.ps2dev.org
# Licenced under Academic Free License version 2.0
# Review ps2sdk README & LICENSE files for further details.

IOP_OBJS = eesync.o exports.o imports.o

# Wipe EE kernel space?
EESYNC_WIPE_EE_KERNEL_SPACE ?= 0

# Allocate memory based on the presense of rom0:SECRMAN?
EESYNC_SECRMAN_DUMMY ?= 0

# Register eesync exports?
EESYNC_REGISTER_EXPORTS ?= 1

# Check IOP boot parameter to see if initialization should be skipped?
EESYNC_CHECK_ILOADP ?= 1

ifneq (x$(EESYNC_WIPE_EE_KERNEL_SPACE),x0)
IOP_CFLAGS += -DEESYNC_WIPE_EE_KERNEL_SPACE
endif

ifneq (x$(EESYNC_SECRMAN_DUMMY),x0)
IOP_CFLAGS += -DEESYNC_SECRMAN_DUMMY
endif

ifneq (x$(EESYNC_REGISTER_EXPORTS),x0)
IOP_CFLAGS += -DEESYNC_REGISTER_EXPORTS
endif

ifneq (x$(EESYNC_CHECK_ILOADP),x0)
IOP_CFLAGS += -DEESYNC_CHECK_ILOADP
endif

include $(PS2SDKSRC)/Defs.make
include $(PS2SDKSRC)/iop/Rules.bin.make
include $(PS2SDKSRC)/iop/Rules.make
include $(PS2SDKSRC)/iop/Rules.release

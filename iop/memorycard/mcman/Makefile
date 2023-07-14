# _____     ___ ____     ___ ____
#  ____|   |    ____|   |        | |____|
# |     ___|   |____ ___|    ____| |    \    PS2DEV Open Source Project.
#-----------------------------------------------------------------------
# Copyright 2001-2009, ps2dev - http://www.ps2dev.org
# Licenced under Academic Free License version 2.0
# Review ps2sdk README & LICENSE files for further details.

# IOP_CFLAGS += -DSIO_DEBUG -DDEBUG

IOP_INCS += -I$(PS2SDKSRC)/iop/system/sio2man/include

IOP_OBJS = \
    main.o \
    mcdev.o \
    mciomanx_backing.o \
    mcsio2.o \
    ps2mc_fio.o \
    ps1mc_fio.o \
    imports.o \
    exports.o

# Build the newer version of the memory card module?
MCMAN_BUILDING_XMCMAN ?= 1

# Read from the dev9 external flash ROM instead of the memory card?
MCMAN_BUILDING_XFROMMAN ?= 0

# Read from a file on the filesystem insead of the memory card?
MCMAN_BUILDING_VMCMAN ?= 0

ifneq (x$(MCMAN_BUILDING_XMCMAN),x0)
IOP_CFLAGS += -DBUILDING_XMCMAN
endif

ifneq (x$(MCMAN_BUILDING_XFROMMAN),x0)
IOP_CFLAGS += -DBUILDING_XFROMMAN
IOP_INCS += -I$(PS2SDKSRC)/iop/dev9/extflash/include
endif

ifneq (x$(MCMAN_BUILDING_VMCMAN),x0)
IOP_CFLAGS += -DBUILDING_VMCMAN
IOP_CFLAGS += -DIOMANX_OLD_NAME_COMPATIBILITY=0
IOP_CFLAGS += -DIOMANX_OLD_NAME_ADDDELDRV=0
endif

include $(PS2SDKSRC)/Defs.make
include $(PS2SDKSRC)/iop/Rules.bin.make
include $(PS2SDKSRC)/iop/Rules.make
include $(PS2SDKSRC)/iop/Rules.release

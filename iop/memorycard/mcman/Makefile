# _____     ___ ____     ___ ____
#  ____|   |    ____|   |        | |____|
# |     ___|   |____ ___|    ____| |    \    PS2DEV Open Source Project.
#-----------------------------------------------------------------------
# Copyright 2001-2009, ps2dev - http://www.ps2dev.org
# Licenced under Academic Free License version 2.0
# Review ps2sdk README & LICENSE files for further details.

# Build the newer version of the memory card module?
MCMAN_BUILDING_XMCMAN ?= 1

# Read from the dev9 external flash ROM instead of the memory card?
MCMAN_BUILDING_XFROMMAN ?= 0

# Read from a file on the filesystem insead of the memory card?
MCMAN_BUILDING_VMCMAN ?= 0

# Support for arcade PS2
MCMAN_BUILDING_DONGLEMAN ?= 0

# IOP_CFLAGS += -DSIO_DEBUG -DDEBUG

IOP_IMPORT_INCS += \
    cdvd/cdvdman \
    debug/sior \
    security/secrman \
    sio/sio2man \
    system/intrman \
    system/ioman \
    system/iomanx \
    system/loadcore \
    system/modload \
    system/stdio \
    system/sysclib \
    system/threadman \
    system/timrman

IOP_OBJS = \
    main.o \
    mcdev.o \
    mciomanx_backing.o \
    mcsio2.o \
    ps2mc_fio.o \
    ps1mc_fio.o \
    imports.o \
    exports.o

ifneq (x$(MCMAN_BUILDING_XMCMAN),x0)
IOP_CFLAGS += -DBUILDING_XMCMAN
endif

ifneq (x$(MCMAN_BUILDING_XFROMMAN),x0)
IOP_CFLAGS += -DBUILDING_XFROMMAN
IOP_IMPORT_INCS += dev9/extflash
endif

ifneq (x$(MCMAN_BUILDING_VMCMAN),x0)
IOP_CFLAGS += -DBUILDING_VMCMAN
IOP_CFLAGS += -DIOMANX_OLD_NAME_COMPATIBILITY=0
IOP_CFLAGS += -DIOMANX_OLD_NAME_ADDDELDRV=0
endif

ifneq (x$(MCMAN_BUILDING_DONGLEMAN),x0)
IOP_CFLAGS += -DBUILDING_DONGLEMAN
endif

include $(PS2SDKSRC)/Defs.make
include $(PS2SDKSRC)/iop/Rules.bin.make
include $(PS2SDKSRC)/iop/Rules.make
include $(PS2SDKSRC)/iop/Rules.release

# _____     ___ ____     ___ ____
#  ____|   |    ____|   |        | |____|
# |     ___|   |____ ___|    ____| |    \    PS2DEV Open Source Project.
#-----------------------------------------------------------------------
# Copyright 2001-2004, ps2dev - http://www.ps2dev.org
# Licenced under Academic Free License version 2.0
# Review ps2sdk README & LICENSE files for further details.

# Enable to perform the SCE HDD check at the end of ATA device initialization.
ATA_SCE_AUTH_HDD ?= 0

# Enable to enable support for Multi-Word DMA (MWDMA) modes, as in early ATAD modules.
ATA_MWDMA_MODES ?= 0

# Enable to enable support for PIO modes 0-4 (as in early ATAD modules), instead of just PIO mode 0.
ATA_ALL_PIO_MODES ?= 0

# Disable to disable the workaround for bad GameStar compatible adaptors.
ATA_GAMESTAR_WORKAROUND ?= 1

# Enable to enable support for the BDM interface.
ATA_ENABLE_BDM ?= 0

IOP_BIN ?= ps2atad.irx

IOP_INCS += -I$(PS2SDKSRC)/iop/dev9/dev9/include
IOP_OBJS = ps2atad.o imports.o exports.o

ifeq ($(ATA_SCE_AUTH_HDD),1)
IOP_CFLAGS += -DATA_SCE_AUTH_HDD=1
endif

ifeq ($(ATA_MWDMA_MODES),1)
IOP_CFLAGS += -DATA_MWDMA_MODES=1
endif

ifeq ($(ATA_ALL_PIO_MODES),1)
IOP_CFLAGS += -DATA_ALL_PIO_MODES=1
endif

ifeq ($(ATA_GAMESTAR_WORKAROUND),1)
IOP_CFLAGS += -DATA_GAMESTAR_WORKAROUND=1
endif

ifeq ($(ATA_ENABLE_BDM),1)
IOP_CFLAGS += -DATA_ENABLE_BDM=1 -Wno-strict-aliasing
IOP_INCS += -I$(PS2SDKSRC)/iop/fs/bdm/include
endif

include $(PS2SDKSRC)/Defs.make
include $(PS2SDKSRC)/iop/Rules.bin.make
include $(PS2SDKSRC)/iop/Rules.make
include $(PS2SDKSRC)/iop/Rules.release

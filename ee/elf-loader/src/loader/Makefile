# _____     ___ ____     ___ ____
#  ____|   |    ____|   |        | |____|
# |     ___|   |____ ___|    ____| |    \    PS2DEV Open Source Project.
#-----------------------------------------------------------------------
# (c) 2020 Francisco Javier Trujillo Mata <fjtrujy@gmail.com>
# Licenced under Academic Free License version 2.0
# Review ps2sdk README & LICENSE files for further details.

EE_OBJS = loader.o

# Enable debug colors?
LOADER_ENABLE_DEBUG_COLORS ?= 1

# Use nano newlib for saving space
EE_NEWLIB_NANO ?= 1
EE_COMPACT_EXECUTABLE ?= 1

EE_LINKFILE ?= linkfile

ifneq (x$(LOADER_ENABLE_DEBUG_COLORS), x0)
EE_CFLAGS += -DLOADER_ENABLE_DEBUG_COLORS
endif

include $(PS2SDKSRC)/Defs.make
include $(PS2SDKSRC)/ee/Rules.bin.make
include $(PS2SDKSRC)/ee/Rules.make
include $(PS2SDKSRC)/ee/Rules.release

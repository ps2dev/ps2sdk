# _____     ___ ____     ___ ____
#  ____|   |    ____|   |        | |____|
# |     ___|   |____ ___|    ____| |    \    PS2DEV Open Source Project.
#-----------------------------------------------------------------------
# Copyright 2001-2005, ps2dev - http://www.ps2dev.org
# Licenced under Academic Free License version 2.0
# Review ps2sdk README & LICENSE files for further details.

# EE_INCS += -I$(PS2SDKSRC)/ee/math/include -I$(PS2SDKSRC)/ee/graph/include
EE_BIN2C = bin2c

EE_OBJS = elf.o loader.o

# all:: prepareLoader

prepareLoader:
	$(MAKE) -C src/loader
	$(EE_BIN2C) src/loader/loader.elf src/loader.c loader_elf

include $(PS2SDKSRC)/Defs.make
include $(PS2SDKSRC)/ee/Rules.lib.make
include $(PS2SDKSRC)/ee/Rules.make
include $(PS2SDKSRC)/ee/Rules.release

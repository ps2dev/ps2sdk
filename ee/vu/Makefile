# _____     ___ ____     ___ ____
#  ____|   |    ____|   |        | |____|
# |     ___|   |____ ___|    ____| |    \    PS2DEV Open Source Project.
#-----------------------------------------------------------------------
# Copyright 2001-2005, ps2dev - http://www.ps2dev.org
# Licenced under Academic Free License version 2.0
# Review ps2sdk README & LICENSE files for further details.

EE_OBJS = vu.o erl-support.o

EE_INCS := $(EE_INCS) -I$(PS2SDKSRC)/ee/packet2/include
EE_INCS := $(EE_INCS) -I$(PS2SDKSRC)/ee/dma/include
EE_INCS := $(EE_INCS) -I$(PS2SDKSRC)/ee/draw/include
EE_INCS := $(EE_INCS) -I$(PS2SDKSRC)/ee/math3d/include

include $(PS2SDKSRC)/Defs.make
include $(PS2SDKSRC)/ee/Rules.lib.make
include $(PS2SDKSRC)/ee/Rules.make
include $(PS2SDKSRC)/ee/Rules.release

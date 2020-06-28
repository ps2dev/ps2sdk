# _____     ___ ____     ___ ____
#  ____|   |    ____|   |        | |____|
# |     ___|   |____ ___|    ____| |    \    PS2DEV Open Source Project.
#-----------------------------------------------------------------------
# (c) 2020 Francisco Javier Trujillo Mata <fjtrujy@gmail.com>
# Licenced under Academic Free License version 2.0
# Review ps2sdk README & LICENSE files for further details.

EE_OBJS = elf.o loader.o

all:: prepareLoader $(EE_LIB)

clean:: cleanLoader

prepareLoader:
	$(MAKE) -C src/loader
	$(PS2SDKSRC)/tools/bin2c/bin/bin2c src/loader/loader.elf src/loader.c loader_elf

cleanLoader:
	$(MAKE) -C src/loader clean

include $(PS2SDKSRC)/Defs.make
include $(PS2SDKSRC)/ee/Rules.lib.make
include $(PS2SDKSRC)/ee/Rules.make
include $(PS2SDKSRC)/ee/Rules.release

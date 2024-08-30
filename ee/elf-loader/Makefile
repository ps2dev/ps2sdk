# _____     ___ ____     ___ ____
#  ____|   |    ____|   |        | |____|
# |     ___|   |____ ___|    ____| |    \    PS2DEV Open Source Project.
#-----------------------------------------------------------------------
# (c) 2020 Francisco Javier Trujillo Mata <fjtrujy@gmail.com>
# Licenced under Academic Free License version 2.0
# Review ps2sdk README & LICENSE files for further details.

EE_OBJS = elf.o loader.o

include $(PS2SDKSRC)/Defs.make
include $(PS2SDKSRC)/ee/Rules.lib.make
include $(PS2SDKSRC)/ee/Rules.make
include $(PS2SDKSRC)/ee/Rules.release

$(PS2SDKSRC)/tools/bin2c/bin/bin2c: $(PS2SDKSRC)/tools/bin2c
	$(MAKEREC) $<

src/loader/bin/loader.elf:
	$(MAKEREC) src/loader

$(EE_OBJS_DIR)loader.c: src/loader/bin/loader.elf $(PS2SDKSRC)/tools/bin2c/bin/bin2c
	$(PS2SDKSRC)/tools/bin2c/bin/bin2c $< $@ loader_elf

$(EE_OBJS_DIR)loader.o: $(EE_OBJS_DIR)loader.c
	$(DIR_GUARD)
	$(EE_C_COMPILE) $< -c -o $@

clean::
	$(MAKEREC) src/loader clean

.NOTPARALLEL:: \
	$(PS2SDKSRC)/tools/bin2c/bin/bin2c \
	src/loader/bin/loader.elf

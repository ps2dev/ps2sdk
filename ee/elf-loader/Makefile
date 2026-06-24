# _____     ___ ____     ___ ____
#  ____|   |    ____|   |        | |____|
# |     ___|   |____ ___|    ____| |    \    PS2DEV Open Source Project.
#-----------------------------------------------------------------------
# (c) 2020 Francisco Javier Trujillo Mata <fjtrujy@gmail.com>
# Licenced under Academic Free License version 2.0
# Review ps2sdk README & LICENSE files for further details.

EE_OBJS = elf.o loader.o loader_noreset.o

include $(PS2SDKSRC)/Defs.make
include $(PS2SDKSRC)/ee/Rules.lib.make
include $(PS2SDKSRC)/ee/Rules.make
include $(PS2SDKSRC)/ee/Rules.release

$(PS2SDKSRC)/tools/bin2c/bin/bin2c: $(PS2SDKSRC)/tools/bin2c
	$(MAKEREC) $<

src/loader/bin/loader.elf:
	$(MAKEREC) src/loader

src/loader/bin_noreset/loader.elf:
	$(MAKEREC) src/loader EE_BIN_DIR=bin_noreset/ EE_OBJS_DIR=obj_noreset/ LOADER_SKIP_IOP_RESET=1

$(EE_OBJS_DIR)loader.c: src/loader/bin/loader.elf $(PS2SDKSRC)/tools/bin2c/bin/bin2c
	$(PS2SDKSRC)/tools/bin2c/bin/bin2c $< $@ loader_elf

$(EE_OBJS_DIR)loader_noreset.c: src/loader/bin_noreset/loader.elf $(PS2SDKSRC)/tools/bin2c/bin/bin2c
	$(PS2SDKSRC)/tools/bin2c/bin/bin2c $< $@ loader_noreset_elf

$(EE_OBJS_DIR)loader.o: $(EE_OBJS_DIR)loader.c
	$(DIR_GUARD)
	$(EE_C_COMPILE) $< -c -o $@

$(EE_OBJS_DIR)loader_noreset.o: $(EE_OBJS_DIR)loader_noreset.c
	$(DIR_GUARD)
	$(EE_C_COMPILE) $< -c -o $@

clean::
	$(MAKEREC) src/loader clean
	$(MAKEREC) src/loader EE_BIN_DIR=bin_noreset/ EE_OBJS_DIR=obj_noreset/ clean

.NOTPARALLEL:: \
	$(PS2SDKSRC)/tools/bin2c/bin/bin2c \
	src/loader/bin/loader.elf \
	src/loader/bin_noreset/loader.elf

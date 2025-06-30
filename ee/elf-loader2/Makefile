# _____     ___ ____     ___ ____
#  ____|   |    ____|   |        | |____|
# |     ___|   |____ ___|    ____| |    \    PS2DEV Open Source Project.
#-----------------------------------------------------------------------
# Copyright ps2dev - http://www.ps2dev.org
# Licenced under Academic Free License version 2.0
# Review ps2sdk README & LICENSE files for further details.

EE_OBJS = \
	elf.o \
	elf_loader_compat.o \
	elf_loader_reader.o \
	elf_loader_stdio.o \
	ldrsrc.o

include $(PS2SDKSRC)/Defs.make
include $(PS2SDKSRC)/ee/Rules.lib.make
include $(PS2SDKSRC)/ee/Rules.make
include $(PS2SDKSRC)/ee/Rules.release

$(PS2SDKSRC)/tools/bin2c/bin/bin2c: $(PS2SDKSRC)/tools/bin2c
	$(MAKEREC) $<

src/ldrsrc/bin/ldrsrc.bin:
	$(MAKEREC) src/ldrsrc

$(EE_OBJS_DIR)ldrsrc.c: src/ldrsrc/bin/ldrsrc.bin $(PS2SDKSRC)/tools/bin2c/bin/bin2c
	$(PS2SDKSRC)/tools/bin2c/bin/bin2c $< $@ ldrsrc

$(EE_OBJS_DIR)ldrsrc.o: $(EE_OBJS_DIR)ldrsrc.c
	$(DIR_GUARD)
	$(EE_C_COMPILE) $< -c -o $@

clean::
	$(MAKEREC) src/ldrsrc clean

.NOTPARALLEL:: \
	$(PS2SDKSRC)/tools/bin2c/bin/bin2c \
	src/ldrsrc/bin/ldrsrc.bin

# _____     ___ ____     ___ ____
#  ____|   |    ____|   |        | |____|
# |     ___|   |____ ___|    ____| |    \    PS2DEV Open Source Project.
#-----------------------------------------------------------------------
# Copyright ps2dev - http://www.ps2dev.org
# Licenced under Academic Free License version 2.0
# Review ps2sdk README & LICENSE files for further details.

EE_BIN_DIR = bin/
EE_OBJS_DIR = obj/

EE_LDRSRC_BIN = ldrsrc.bin
EE_LDRSRC_BIN := $(EE_LDRSRC_BIN:%=$(EE_BIN_DIR)%)
EE_LDRSRC_ELF = ldrsrc.elf
EE_LDRSRC_ELF := $(EE_LDRSRC_ELF:%=$(EE_BIN_DIR)%)
EE_OBJS = ldrsrc.o

# Enable debug colors?
LOADER_ENABLE_DEBUG_COLORS ?= 0

EE_LIBS += -lc_nano
EE_INCS += -I$(PS2SDKSRC)/ee/elf-loader2/include -I$(PS2SDKSRC)/ee/elf-loader2/src/include
EE_CFLAGS += -mno-gpopt

EE_LINKFILE ?= $(PS2SDKSRC)/ee/elf-loader2/src/ldrsrc/linkfile

ifneq (x$(LOADER_ENABLE_DEBUG_COLORS), x0)
EE_CFLAGS += -DLOADER_ENABLE_DEBUG_COLORS
endif

EE_LIB_ARCHIVES ?= $(PS2SDKSRC)/ee/kernel/lib/libkernel.a

$(EE_LDRSRC_BIN): $(EE_LDRSRC_ELF)
	$(DIR_GUARD)
	$(EE_OBJCOPY) -Obinary $< $@

clean:
	rm -f -r $(EE_OBJS_DIR) $(EE_BIN_DIR)

include $(PS2SDKSRC)/Defs.make
include $(PS2SDKSRC)/ee/Rules.make

$(EE_LDRSRC_ELF): $(EE_OBJS) $(EE_LIB_ARCHIVES)
	$(DIR_GUARD)
	$(EE_CC) $(EE_CFLAGS) -o $@ $^ -nostdlib -nostartfiles -T$(EE_LINKFILE) -s $(EE_LIBS) $(EE_LIB_ARCHIVES)

$(PS2SDKSRC)/ee/kernel/lib/libkernel.a:
	$(MAKEREC) $(PS2SDKSRC)/ee/kernel

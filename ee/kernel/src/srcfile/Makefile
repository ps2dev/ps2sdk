# _____     ___ ____     ___ ____
#  ____|   |    ____|   |        | |____|
# |     ___|   |____ ___|    ____| |    \    PS2DEV Open Source Project.
#-----------------------------------------------------------------------
# Licenced under Academic Free License version 2.0
# Review ps2sdk README & LICENSE files for further details.

EE_BIN_DIR = bin/
EE_OBJS_DIR = obj/

EE_SRCFILE_BIN = srcfile.bin
EE_SRCFILE_BIN := $(EE_SRCFILE_BIN:%=$(EE_BIN_DIR)%)
EE_SRCFILE_ELF = srcfile.elf
EE_SRCFILE_ELF := $(EE_SRCFILE_ELF:%=$(EE_BIN_DIR)%)
EE_OBJS = srcfile.o alarm.o dispatch.o

EE_CFLAGS += -mno-gpopt

$(EE_SRCFILE_BIN): $(EE_SRCFILE_ELF)
	$(EE_OBJCOPY) -Obinary $< $@

clean:
	rm -f -r $(EE_OBJS_DIR) $(EE_BIN_DIR)

include $(PS2SDKSRC)/Defs.make
include $(PS2SDKSRC)/ee/Rules.make

$(EE_SRCFILE_ELF): $(EE_OBJS) | $(EE_BIN_DIR)
	$(EE_CC) $(EE_CFLAGS) -o $@ $^ -nostdlib -nostartfiles -Tlinkfile -s $(EE_LIBS)

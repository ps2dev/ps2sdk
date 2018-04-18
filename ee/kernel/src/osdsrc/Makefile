# _____     ___ ____     ___ ____
#  ____|   |    ____|   |        | |____|
# |     ___|   |____ ___|    ____| |    \    PS2DEV Open Source Project.
#-----------------------------------------------------------------------
# Licenced under Academic Free License version 2.0
# Review ps2sdk README & LICENSE files for further details.

EE_BIN_DIR = bin/
EE_OBJS_DIR = obj/

EE_OSDSRC_BIN = osdsrc.bin
EE_OSDSRC_BIN := $(EE_OSDSRC_BIN:%=$(EE_BIN_DIR)%)
EE_OSDSRC_ELF = osdsrc.elf
EE_OSDSRC_ELF := $(EE_OSDSRC_ELF:%=$(EE_BIN_DIR)%)
EE_OBJS = osdsrc.o osd.o ExecPS2.o

EE_INCS += -I$(PS2SDKSRC)/ee/rpc/cdvd/include
EE_CFLAGS += -mno-gpopt -DREUSE_EXECPS2

$(EE_OSDSRC_BIN): $(EE_OSDSRC_ELF)
	$(EE_OBJCOPY) -Obinary $< $@

clean:
	rm -f -r $(EE_OBJS_DIR) $(EE_BIN_DIR)

include $(PS2SDKSRC)/Defs.make
include $(PS2SDKSRC)/ee/Rules.make

$(EE_OSDSRC_ELF): $(EE_OBJS) | $(EE_BIN_DIR)
	$(EE_CC) $(EE_CFLAGS) -o $@ $^ -nostdlib -nostartfiles -Tlinkfile -s $(EE_LIBS)

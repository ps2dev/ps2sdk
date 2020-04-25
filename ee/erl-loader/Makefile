# _____     ___ ____     ___ ____
#  ____|   |    ____|   |        | |____|
# |     ___|   |____ ___|    ____| |    \    PS2DEV Open Source Project.
#-----------------------------------------------------------------------
# Copyright 2001-2004, ps2dev - http://www.ps2dev.org
# Licenced under Academic Free License version 2.0
# Review ps2sdk README & LICENSE files for further details.

EE_OBJS_DIR = obj/
EE_BIN_DIR = bin/

LOADER_BIN = $(EE_BIN_DIR)erl-loader.elf

# Since we are using non-builtin symbols
NO_BUILTIN = memcmp memcpy memset printf strcat strchr strcmp strcpy strlen strncpy strrchr strncmp
EE_CFLAGS += $(NO_BUILTIN:%=-fno-builtin-%) -L$(PS2SDKSRC)/ee/libc/lib -L$(PS2SDKSRC)/ee/kernel/lib

EE_INCS += -I$(PS2SDKSRC)/ee/erl/include -I$(PS2SDKSRC)/ee/libc/include -I$(PS2SDKSRC)/ee/erl-loader/src

EE_LIBS += $(PS2SDKSRC)/ee/erl/lib/liberl.a

EE_OBJS = erl-loader.o

all: $(LOADER_BIN)

clean:
	rm -f -r $(EE_OBJS_DIR) $(EE_BIN_DIR) $(EE_SRC_DIR)exports.c

release:
	$(ECHO) Installing $(LOADER_BIN) to $(PS2SDK)/ee/bin/ ;
	cp -f $(LOADER_BIN) $(PS2SDK)/ee/bin/
	chmod 644 $(PS2SDK)/ee/bin/erl-loader.elf

include $(PS2SDKSRC)/Defs.make
include $(PS2SDKSRC)/ee/Rules.make
include $(PS2SDKSRC)/ee/Rules.release

$(EE_OBJS_DIR)erl-loader.o: | $(EE_OBJS_DIR)

$(LOADER_BIN): $(EE_OBJS) $(EE_LIBS) | $(EE_BIN_DIR)
	$(EE_CC) $(EE_CFLAGS) -o $(EE_BIN_DIR)tmp.elf $(EE_OBJS) $(EE_LIBS) $(EE_SRC_DIR)dummy-exports.c
	$(EE_SRC_DIR)build-exports.sh $(EE_TOOL_PREFIX)readelf
	rm $(EE_BIN_DIR)tmp.elf
	$(EE_CC) $(EE_CFLAGS) -o $(LOADER_BIN) $(EE_OBJS) $(EE_LIBS) $(EE_SRC_DIR)exports.c

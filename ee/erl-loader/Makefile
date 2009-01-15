# _____     ___ ____     ___ ____
#  ____|   |    ____|   |        | |____|
# |     ___|   |____ ___|    ____| |    \    PS2DEV Open Source Project.
#-----------------------------------------------------------------------
# Copyright 2001-2004, ps2dev - http://www.ps2dev.org
# Licenced under Academic Free License version 2.0
# Review ps2sdk README & LICENSE files for further details.
#
# $Id$


EE_OBJS_DIR = obj/
EE_BIN_DIR = bin/
EE_SRC_DIR = src/
EE_INC_DIR = include/

EE_INCS = -I$(PS2SDKSRC)/ee/erl/include -I$(PS2SDKSRC)/ee/libc/include -I$(PS2SDKSRC)/ee/erl-loader/src

EE_OBJS = erl-loader.o
EE_OBJS := $(EE_OBJS:%=$(EE_OBJS_DIR)%)

EE_LDFLAGS = -L$(PS2SDKSRC)/ee/kernel/lib -L$(PS2SDKSRC)/ee/libc/lib -L$(PS2SDKSRC)/ee/erl/lib

# Since we are using non-builtin symbols
EE_LDFLAGS += -fno-builtin-memcmp -fno-builtin-memcpy -fno-builtin-memset
EE_LDFLAGS += -fno-builtin-printf -fno-builtin-strcat -fno-builtin-strchr
EE_LDFLAGS += -fno-builtin-strcmp -fno-builtin-strcpy -fno-builtin-strlen
EE_LDFLAGS += -fno-builtin-strncpy -fno-builtin-strrchr -fno-builtin-strncmp

EE_LIBS = -lerl -lc -lkernel

LOADER_BIN = $(EE_BIN_DIR)erl-loader.elf

all: $(EE_OBJS_DIR) $(EE_BIN_DIR) $(LOADER_BIN)

$(LOADER_BIN) : $(EE_OBJS) $(PS2SDKSRC)/ee/startup/obj/crt0.o
	$(EE_CC) -mno-crt0 -T$(PS2SDKSRC)/ee/startup/src/linkfile $(EE_LDFLAGS) \
	-o bin/tmp.elf $(PS2SDKSRC)/ee/startup/obj/crt0.o $(EE_OBJS) $(EE_LIBS) $(EE_SRC_DIR)dummy-exports.c
	$(EE_SRC_DIR)build-exports.sh
	rm $(EE_BIN_DIR)tmp.elf
	$(EE_CC) -mno-crt0 -T$(PS2SDKSRC)/ee/startup/src/linkfile $(EE_LDFLAGS) \
	-o $(LOADER_BIN) $(PS2SDKSRC)/ee/startup/obj/crt0.o $(EE_OBJS) $(EE_LIBS) $(EE_SRC_DIR)exports.c
#	rm $(EE_SRC_DIR)exports.c

clean:
	rm -f -r $(EE_OBJS_DIR) $(EE_BIN_DIR)

include $(PS2SDKSRC)/Defs.make
include $(PS2SDKSRC)/ee/Rules.make
include $(PS2SDKSRC)/ee/Rules.release

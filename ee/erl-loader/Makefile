# _____     ___ ____     ___ ____
#  ____|   |    ____|   |        | |____|
# |     ___|   |____ ___|    ____| |    \    PS2DEV Open Source Project.
#-----------------------------------------------------------------------
# Copyright 2001-2004, ps2dev - http://www.ps2dev.org
# Licenced under Academic Free License version 2.0
# Review ps2sdk README & LICENSE files for further details.

EE_CC_VERSION := $(shell $(EE_CC) --version 2>&1 | sed -n 's/^.*(GCC) //p')

EE_OBJS_DIR = obj/
EE_BIN_DIR = bin/
EE_SRC_DIR = src/
EE_INC_DIR = include/

LOADER_BIN = $(EE_BIN_DIR)erl-loader.elf

# Since we are using non-builtin symbols
NO_BUILTIN = memcmp memcpy memset printf strcat strchr strcmp strcpy strlen strncpy strrchr strncmp
EE_CFLAGS += $(NO_BUILTIN:%=-fno-builtin-%)

EE_INCS += -I$(PS2SDKSRC)/ee/erl/include -I$(PS2SDKSRC)/ee/libc/include -I$(PS2SDKSRC)/ee/erl-loader/src

EE_LDFLAGS = -L$(PS2SDKSRC)/ee/kernel/lib -L$(PS2SDKSRC)/ee/libc/lib -L$(PS2SDKSRC)/ee/erl/lib

EE_LIBS = -lerl -lc -lkernel

# Extra macro for disabling the automatic inclusion of the built-in CRT object(s)
ifeq ($(EE_CC_VERSION),3.2.2)
EE_NO_CRT = -mno-crt0
endif
ifeq ($(EE_CC_VERSION),3.2.3)
EE_NO_CRT = -mno-crt0
endif
ifneq ($(EE_CC_VERSION),3.2.2)
ifneq ($(EE_CC_VERSION),3.2.3)
EE_NO_CRT = -nostartfiles
endif
endif

EE_OBJS = erl-loader.o
EE_OBJS := $(EE_OBJS:%=$(EE_OBJS_DIR)%)

all: $(EE_OBJS_DIR) $(EE_BIN_DIR) $(LOADER_BIN)

$(LOADER_BIN) : $(EE_OBJS) $(PS2SDKSRC)/ee/startup/obj/crt0.o
	$(EE_CC) $(EE_NO_CRT) -T$(PS2SDKSRC)/ee/startup/src/linkfile $(EE_CFLAGS) \
	-o $(EE_BIN_DIR)tmp.elf $(PS2SDKSRC)/ee/startup/obj/crt0.o $(EE_OBJS) $(EE_LDFLAGS) $(EE_LIBS) $(EE_SRC_DIR)dummy-exports.c
	$(EE_SRC_DIR)build-exports.sh
	rm $(EE_BIN_DIR)tmp.elf
	$(EE_CC) $(EE_NO_CRT) -T$(PS2SDKSRC)/ee/startup/src/linkfile $(EE_CFLAGS) \
	-o $(LOADER_BIN) $(PS2SDKSRC)/ee/startup/obj/crt0.o $(EE_OBJS) $(EE_LDFLAGS) $(EE_LIBS) $(EE_SRC_DIR)exports.c

clean:
	rm -f -r $(EE_OBJS_DIR) $(EE_BIN_DIR) $(EE_SRC_DIR)exports.c

release:
	$(ECHO) Installing $(LOADER_BIN) to $(PS2SDK)/ee/bin/ ;
	cp -f $(LOADER_BIN) $(PS2SDK)/ee/bin/
	chmod 644 $(PS2SDK)/ee/bin/erl-loader.elf

include $(PS2SDKSRC)/Defs.make
include $(PS2SDKSRC)/ee/Rules.make
include $(PS2SDKSRC)/ee/Rules.release

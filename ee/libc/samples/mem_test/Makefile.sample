# _____     ___ ____     ___ ____
#  ____|   |    ____|   |        | |____|
# |     ___|   |____ ___|    ____| |    \    PS2DEV Open Source Project.
#-----------------------------------------------------------------------
# Copyright 2001-2004, ps2dev - http://www.ps2dev.org
# Licenced under Academic Free License version 2.0
# Review ps2sdk README & LICENSE files for further details.

SCREEN_DEBUG = 1
VERBOSE = 0

EE_BIN   = mem_test.elf
EE_OBJS  = main.o

EE_CFLAGS = -Os -g
# EE_CFLAGS += -fdata-sections -ffunction-sections
# EE_LDFLAGS += -s
# EE_LDFLAGS += -Wl,--gc-sections

ifeq ($(SCREEN_DEBUG), 1)
EE_LIBS += -ldebug
EE_CFLAGS += -DSCREEN_DEBUG
endif

ifeq ($(VERBOSE), 1)
EE_CFLAGS += -DVERBOSE
endif

all: $(EE_BIN)

clean:
	rm -rf $(EE_BIN) $(EE_OBJS)

run: $(EE_BIN)
	ps2client execee host:$(EE_BIN)

reset:
	ps2client reset

include $(PS2SDK)/samples/Makefile.pref
include $(PS2SDK)/samples/Makefile.eeglobal

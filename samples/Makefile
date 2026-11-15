# _____     ___ ____     ___ ____
#  ____|   |    ____|   |        | |____|
# |     ___|   |____ ___|    ____| |    \    PS2DEV Open Source Project.
#-----------------------------------------------------------------------
# Copyright 2001-2004, ps2dev - http://www.ps2dev.org
# Licenced under Academic Free License version 2.0
# Review ps2sdk README & LICENSE files for further details.

BIN_TARGET = $(PS2SDK)/bin
SAMPLES_TARGET = $(PS2SDK)/samples
SHARE_TARGET = $(PS2DEV)/share

IS_WINDOWS := $(if $(filter Windows_NT,$(OS)),1,$(if $(filter UCRT64,$(MSYSTEM)),1,0))

ifeq ($(IS_WINDOWS),1)

CC = gcc

OBJS = pkg-config.o

all: pkg-config

clean:
	rm -rf *.o *.exe

pkg-config: $(OBJS)
	$(CC) -O2 -s $(OBJS) -o pkg-config

%.o: %.c
	$(CC) -O2 -s -c $< -o $@

else
all:
	@$(PRINTF) 'Nothing to do in samples\n'

clean:
	@$(PRINTF) 'Nothing to clean in samples\n'
endif

release: release-samples
	@$(PRINTF) 'Released samples dir.\n'

release-samples:
	$(MKDIR) -p $(SAMPLES_TARGET)
	$(MKDIR) -p $(SHARE_TARGET)
	cp -f Makefile_sample $(SAMPLES_TARGET)/Makefile
	cp -f Makefile.pref_sample $(SAMPLES_TARGET)/Makefile.pref
	cp -f Makefile.ioprp_sample $(SAMPLES_TARGET)/Makefile.ioprp
	cp -f Makefile.eeglobal_sample $(SAMPLES_TARGET)/Makefile.eeglobal
	cp -f Makefile.eeglobal_cpp_sample $(SAMPLES_TARGET)/Makefile.eeglobal_cpp
	cp -f Makefile.iopglobal_sample $(SAMPLES_TARGET)/Makefile.iopglobal
	cp -f ps2dev.cmake $(SHARE_TARGET)/ps2dev.cmake
	cp -f ps2dev_iop.cmake $(SHARE_TARGET)/ps2dev_iop.cmake
ifeq ($(IS_WINDOWS),1)
	cp -f pkg-config.exe $(BIN_TARGET)/$(EE_TOOL_PREFIX)pkg-config
else
	cp -f pkg-config.in $(BIN_TARGET)/$(EE_TOOL_PREFIX)pkg-config
	chmod 755 $(BIN_TARGET)/$(EE_TOOL_PREFIX)pkg-config
endif

release-clean:
	rm -f -r $(SAMPLES_TARGET)

include $(PS2SDKSRC)/Defs.make

# _____     ___ ____     ___ ____
#  ____|   |    ____|   |        | |____|
# |     ___|   |____ ___|    ____| |    \    PS2DEV Open Source Project.
#-----------------------------------------------------------------------
# Copyright 2001-2004, ps2dev - http://www.ps2dev.org
# Licenced under Academic Free License version 2.0
# Review ps2sdk README & LICENSE files for further details.

EE_BIN_DIR ?= bin/

EE_BIN ?= $(shell basename $(CURDIR)).elf
EE_BIN := $(EE_BIN:%=$(EE_BIN_DIR)%)

all:: $(EE_BIN)

clean::
	rm -f -r $(EE_OBJS_DIR) $(EE_BIN_DIR)

# Put the addidtional rules for no default libraries here, to ensure that it isn't the default rule
ifneq (x$(EE_NODEFAULTLIBS), x0)
$(PS2SDKSRC)/ee/kernel/lib/libkernel.a:
	$(MAKEREC) $(PS2SDKSRC)/ee/kernel

$(PS2SDKSRC)/ee/rpc/cdvd/lib/libcdvd.a:
	$(MAKEREC) $(PS2SDKSRC)/ee/rpc/cdvd

$(PS2SDKSRC)/ee/libcglue/lib/libcglue.a:
	$(MAKEREC) $(PS2SDKSRC)/ee/libcglue
endif

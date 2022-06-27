# _____     ___ ____     ___ ____
#  ____|   |    ____|   |        | |____|
# |     ___|   |____ ___|    ____| |    \    PS2DEV Open Source Project.
#-----------------------------------------------------------------------
# Copyright 2001-2004, ps2dev - http://www.ps2dev.org
# Licenced under Academic Free License version 2.0
# Review ps2sdk README & LICENSE files for further details.

TOOLS_BIN_DIR ?= bin/

TOOLS_BIN ?= $(shell basename $(CURDIR))
TOOLS_BIN := $(TOOLS_BIN:%=$(TOOLS_BIN_DIR)%)

all:: $(TOOLS_BIN)

clean::
	rm -f -r $(TOOLS_OBJS_DIR) $(TOOLS_BIN_DIR)

# _____     ___ ____     ___ ____
#  ____|   |    ____|   |        | |____|
# |     ___|   |____ ___|    ____| |    \    PS2DEV Open Source Project.
#-----------------------------------------------------------------------
# Copyright 2001-2004, ps2dev - http://www.ps2dev.org
# Licenced under Academic Free License version 2.0
# Review ps2sdk README & LICENSE files for further details.

IOP_LIB_DIR ?= lib/

IOP_LIB ?= lib$(shell basename $(CURDIR)).a
IOP_LIB := $(IOP_LIB:%=$(IOP_LIB_DIR)%)

all:: $(IOP_LIB)

clean::
	rm -f -r $(IOP_OBJS_DIR) $(IOP_LIB_DIR)

# _____     ___ ____     ___ ____
#  ____|   |    ____|   |        | |____|
# |     ___|   |____ ___|    ____| |    \    PS2DEV Open Source Project.
#-----------------------------------------------------------------------
# Copyright 2001-2004, ps2dev - http://www.ps2dev.org
# Licenced under Academic Free License version 2.0
# Review ps2sdk README & LICENSE files for further details.

EE_LIB_DIR ?= lib/

EE_LIB ?= lib$(shell basename $(CURDIR)).a
EE_LIB := $(EE_LIB:%=$(EE_LIB_DIR)%)

all:: $(EE_LIB)

clean::
	rm -f -r $(EE_OBJS_DIR) $(EE_LIB_DIR)

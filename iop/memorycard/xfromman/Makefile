# _____     ___ ____     ___ ____
#  ____|   |    ____|   |        | |____|
# |     ___|   |____ ___|    ____| |    \    PS2DEV Open Source Project.
#-----------------------------------------------------------------------
# Copyright 2001-2004, ps2dev - http://www.ps2dev.org
# Licenced under Academic Free License version 2.0
# Review ps2sdk README & LICENSE files for further details.

IOP_SRC_DIR = $(PS2SDKSRC)/iop/memorycard/mcman/src/
IOP_INC_DIR = $(PS2SDKSRC)/iop/memorycard/mcman/include/

IOP_BIN ?= xfromman.irx

IOP_CFLAGS += -DBUILDING_XFROMMAN

IOP_INCS += -I$(PS2SDKSRC)/iop/dev9/extflash/include

include $(PS2SDKSRC)/iop/memorycard/mcman/Makefile

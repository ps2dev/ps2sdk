# _____     ___ ____     ___ ____
#  ____|   |    ____|   |        | |____|
# |     ___|   |____ ___|    ____| |    \    PS2DEV Open Source Project.
#-----------------------------------------------------------------------
# Copyright 2001-2004, ps2dev - http://www.ps2dev.org
# Licenced under Academic Free License version 2.0
# Review ps2sdk README & LICENSE files for further details.
#
# $Id$

BUILDING_SYSTEM_LIBRARIES = 1

TARGET = libpatches.a
TARGET_DIR = lib
OBJS = smem.o smod.o slib.o patch_enable_lmb.o patch_disable_prefix_check.o

EXTRACLEAN = clean-target

include $(PS2SDKSRC)/ee/Rules.mak

clean-target:
	-rm -rf $(TARGET_DIR)

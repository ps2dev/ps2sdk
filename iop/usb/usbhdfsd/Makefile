# _____     ___ ____     ___ ____
#  ____|   |    ____|   |        | |____|
# |     ___|   |____ ___|    ____| |    \    PS2DEV Open Source Project.
#-----------------------------------------------------------------------
# Copyright 2001-2004, ps2dev - http://www.ps2dev.org
# Licenced under Academic Free License version 2.0
# Review ps2sdk README & LICENSE files for further details.

VFAT_PATH = $(PS2SDKSRC)/iop/fs/vfat/

VFAT_SRC_DIR = $(VFAT_PATH)/src/

# IOP_CFLAGS += -DDEBUG
IOP_CFLAGS += -DBUILDING_USBHDFSD

IOP_INCS += -I$(PS2SDKSRC)/iop/usb/usbd/include -I$(VFAT_SRC_DIR)/include

IOP_OBJS = usb_driver.o fs_driver.o part_driver.o fat_driver.o fat_write.o scache.o usbhdfsd.o api.o imports.o exports.o

include $(PS2SDKSRC)/Defs.make
include $(PS2SDKSRC)/iop/Rules.bin.make
include $(PS2SDKSRC)/iop/Rules.make
include $(PS2SDKSRC)/iop/Rules.release

$(IOP_OBJS_DIR)%.o: $(VFAT_SRC_DIR)%.c
	$(DIR_GUARD)
	$(IOP_CC) $(IOP_CFLAGS) -c $< -o $@

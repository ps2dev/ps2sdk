# _____     ___ ____     ___ ____
#  ____|   |    ____|   |        | |____|
# |     ___|   |____ ___|    ____| |    \    PS2DEV Open Source Project.
#-----------------------------------------------------------------------

IOP_BIN  = IEEE1394_disk.irx
IOP_OBJS = sbp2_driver.o imports.o main.o scsi.o fat_write.o fs_driver.o scache.o fat_driver.o part_driver.o

# Define "SIF_CALLBACKS_12_13" if you want the driver to send a SIF packet to the EE via SIF command channels 12 or 13 whenever a device has been disconnected or connected.
# SIF command channel 12 is used for connection events, while channel 13 is used for disconnection events.

IOP_CFLAGS  += -Wall -fno-builtin -O2 -G0 -mno-check-zero-division #-DSIF_CALLBACKS_12_13
IOP_LDFLAGS += -s
IOP_INCS += -I./include -I$(PS2SDKSRC)/iop/kernel/include -I$(PS2SDKSRC)/common/include

all: $(IOP_BIN)

clean:
	rm -f $(IOP_BIN) $(IOP_OBJS)

include $(PS2SDKSRC)/Defs.make
include Rules.make
include $(PS2SDKSRC)/iop/Rules.release

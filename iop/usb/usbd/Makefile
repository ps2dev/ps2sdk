IOP_OBJS_DIR = obj/
IOP_BIN_DIR = bin/
IOP_SRC_DIR = src/
IOP_INC_DIR = include/

IOP_BIN  = bin/usbd.irx
IOP_OBJS = obj/hcd.o obj/hub.o obj/interface.o obj/mem.o obj/usbd.o obj/usbio.o obj/driver.o obj/imports.o obj/exports.o

IOP_CFLAGS=-Wall
# -DDEBUG
IOP_LDFLAGS=-Wl,-s

all: $(IOP_OBJS_DIR) $(IOP_BIN_DIR) $(IOP_BIN)

clean:
	rm -f -r $(IOP_OBJS_DIR) $(IOP_BIN_DIR)

include $(PS2SDKSRC)/Defs.make
include $(PS2SDKSRC)/iop/Rules.make
include $(PS2SDKSRC)/iop/Rules.release

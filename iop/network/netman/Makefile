IOP_BIN = netman.irx
IOP_OBJS = netman.o rpc_server.o rpc_client.o imports.o exports.o

IOP_INCS += -I$(PS2SDKSRC)/iop/kernel/include -I$(PS2SDKSRC)/common/include
IOP_CFLAGS += -Wall -fno-builtin -D_IOP #-mgpopt -G8192
IOP_LDFLAGS += -s

all: $(IOP_BIN)

clean:
	rm -f $(IOP_OBJS) $(IOP_BIN)

include $(PS2SDKSRC)/Defs.make
include Rules.make
include $(PS2SDKSRC)/iop/Rules.release

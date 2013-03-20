EE_OBJS_DIR = obj/
EE_LIB_DIR = lib/
EE_SRC_DIR = src/
EE_INC_DIR = include/

EE_OBJS = netman.o rpc_server.o rpc_client.o erl-support.o
EE_OBJS := $(EE_OBJS:%=$(EE_OBJS_DIR)%)

EE_INCS = -I$(EE_SRC_DIR)/include
EE_LIB = $(EE_LIB_DIR)libnetman.a

all: $(EE_OBJS_DIR) $(EE_LIB_DIR) $(EE_LIB)

clean:
	rm -f -r $(EE_OBJS_DIR) $(EE_LIB_DIR)


include $(PS2SDKSRC)/Defs.make
include $(PS2SDKSRC)/ee/Rules.make
include $(PS2SDKSRC)/ee/Rules.release

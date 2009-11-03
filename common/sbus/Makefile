include $(PS2SDKSRC)/Defs.make

BIN_DIR=bin
OBJ_DIR=obj
LIB_DIR=lib

## IOP IRX
IOP_OBJ_DIR=iop_obj
IRX_NAME = sbus
IOP_BIN = $(IRX_NAME).irx
IOP_CPPFLAGS = -miop -O2 -G0 -I$(PS2SDKSRC)/iop/kernel/include -I$(PS2SDKSRC)/common/include -I./include -I./src -Wall -fno-builtin-printf -DIRX_NAME=$(IRX_NAME)
IOP_OBJS = ps2_sbus.o iop_sbus.o ps2_sif.o iop_sif2.o sif2cmd.o imports.o exports.o
IOP_OBJS := $(IOP_OBJS:%=$(IOP_OBJ_DIR)/%)

## EE Library
EE_OBJS = ps2_sbus.o ee_sbus.o ps2_sif.o ee_sif2.o sif2cmd.o ee_low.o
EE_OBJS := $(EE_OBJS:%=obj/%)
EE_LIB = $(LIB_DIR)/libsbus.a
EE_CFLAGS = -D_EE -O2 -G0 -fno-builtin-printf
EE_INC = -I. -I./include -I $(PS2SDKSRC)/common/include -I $(PS2SDKSRC)/ee/include -I $(PS2SDKSRC)/ee/kernel/include

all: $(IOP_OBJ_DIR) $(OBJ_DIR) $(BIN_DIR) $(LIB_DIR) $(IOP_BIN) $(EE_LIB)

$(IOP_OBJ_DIR):
	mkdir -p $(IOP_OBJ_DIR)

$(OBJ_DIR):
	mkdir -p $(OBJ_DIR)

$(BIN_DIR):
	mkdir -p $(BIN_DIR)

$(LIB_DIR):
	mkdir -p $(LIB_DIR)

$(IOP_OBJ_DIR)/%.o: src/%.c
	$(IOP_CC) $(IOP_CFLAGS) $(IOP_CPPFLAGS) -c $< -o $@

$(IOP_OBJ_DIR)/%.o: src/%.cpp
	$(IOP_CXX) $(IOP_CXXFLAGS) $(IOP_CPPFLAGS) -c $< -o $@

$(IOP_OBJ_DIR)/%.o: src/%.S
	$(IOP_CC) $(IOP_CPPFLAGS) -c $< -o $@

$(IOP_OBJ_DIR)/%.o: src/%.s
	$(IOP_AS) $(IOP_ASFLAGS) -c $< -o $@

$(IOP_BIN): $(IOP_OBJS) src/imports.lst src/irx_imports.h
	$(IOP_CC) $(IOP_LDFLAGS) -miop -nostdlib -s -o bin/$(IOP_BIN) $(IOP_OBJS)

$(IOP_OBJ_DIR)/%.o: src/%.lst
	echo "#include \"irx_imports.h\"" > src/build-imports.c
	cat $< >> src/build-imports.c
	$(IOP_CC) $(IOP_CFLAGS) $(IOP_CPPFLAGS) -c src/build-imports.c -o $@
	rm -f src/build-imports.c

$(IOP_OBJ_DIR)/%.o: src/%.tab
	$(ECHO) "#include \"irx.h\"" > src/build-exports.c
	cat $< >> src/build-exports.c
	$(IOP_CC) $(IOP_CFLAGS) $(IOP_CPPFLAGS) -Isrc -c src/build-exports.c -o $@
	rm -f src/build-exports.c

$(EE_LIB): $(EE_OBJS)
	$(EE_AR) rc $(EE_LIB) $(EE_OBJS)

clean:
	rm -rf $(OBJ_DIR) $(LIB_DIR) $(BIN_DIR) $(IOP_OBJ_DIR)

obj/%.o: src/%.c
	$(EE_CC) $(EE_CFLAGS) $(EE_INC) -c $< -o $@

obj/%.o: src/%.S
	$(EE_CC) $(EE_CFLAGS) $(EE_INC) -c $< -o $@

release: all
	mkdir -p $(PS2SDK)/ee/include
	cp -f bin/$(IOP_BIN) $(PS2SDK)/iop/irx
	cp -f $(EE_LIB) $(PS2SDK)/ee/lib
	cp -f include/*.h $(PS2SDK)/common/include

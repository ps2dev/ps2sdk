include $(PS2SDK)/Defs.make

IRX_NAME = iop_sbusdbg
IRX_TARGET = $(IRX_NAME).irx

IOP_CPPFLAGS = -miop -O2 -G0 -I$(PS2SDK)/iop/include -I$(PS2SDK)/common/include -I./include -I./src -Wall -fno-builtin-printf -DIRX_NAME=$(IRX_NAME)

IRX_OBJS = main.o sbus_dbg_low.o sbus_tty.o sbus_dbg.o
IRX_OBJS := $(IRX_OBJS:%=obj/%)

all: $(IRX_TARGET)

obj/%.o: src/%.c
	$(IOP_CC) $(IOP_CFLAGS) $(IOP_CPPFLAGS) -c $< -o $@

obj/%.o: src/%.cpp
	$(IOP_CXX) $(IOP_CXXFLAGS) $(IOP_CPPFLAGS) -c $< -o $@

obj/%.o: src/%.S
	$(IOP_CC) $(IOP_CPPFLAGS) -c $< -o $@

obj/%.o: src/%.s
	$(IOP_AS) $(IOP_ASFLAGS) -c $< -o $@

$(IRX_TARGET): $(IRX_OBJS) src/imports.lst src/irx_imports.h
	echo "#include \"irx_imports.h\"" > src/build-imports.c
	cat src/imports.lst >> src/build-imports.c
	$(IOP_CC) $(IOP_CFLAGS) $(IOP_CPPFLAGS) -c src/build-imports.c -o obj/imports.o
	rm -f src/build-imports.c
	$(IOP_CC) $(IOP_LDFLAGS) -miop -nostdlib -s -o bin/$(IRX_TARGET) $(IRX_OBJS) obj/imports.o

clean:
	rm -f obj/*.o bin/$(IRX_TARGET)

rebuild: clean all


include $(PS2SDK)/Defs.make


IRX_NAME = hello
IOP_TARGET = $(IRX_NAME).irx

IOP_CPPFLAGS = -miop -O2 -G0 -I$(PS2SDK)/iop/include -I$(PS2SDK)/common/include -I./include -I./src -Wall -fno-builtin-printf -DIRX_NAME=$(IRX_NAME)

IOP_OBJS = hello_main.o hello_exports.o
IOP_OBJS := $(IOP_OBJS:%=objs/%)

include ../IRX.Rules

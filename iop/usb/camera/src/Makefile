# This makfile should work without ps2lib/ps2sdk makefile.pref......
# I like that.


IOP_INCLUDES  = -I$(PS2SDK)/iop/kernel/include
IOP_INCLUDES += -I$(PS2SDK)/iop/usb/usbd/include
IOP_INCLUDES += -I$(PS2SDK)/common/include
IOP_INCLUDES += -I$(PS2GCC)/iop/lib/gcc-lib/iop/2.8.1-ps2dev/include

IOP_OBJECTS	 = ps2cam.o imports.o

IOP_LINKOPTIONS	= -nostdlib -s

all:
	iop-gcc -c ps2cam.c $(IOP_INCLUDES)
	iop-gcc -c imports.c $(IOP_INCLUDES)
	iop-gcc $(IOP_LINKOPTIONS) -o ps2cam.irx $(IOP_OBJECTS)
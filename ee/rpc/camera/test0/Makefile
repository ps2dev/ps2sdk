EE_BIN = test.elf
EE_OBJS = test.o
EE_LIBS = -lps2cam -lc

all: $(EE_BIN)

#all:
#	ee-gcc -c test.c
#	ee-gcc -W -EL -G0 -O2 -mips3 -nostdlib -Tlinkfile -o test.elf crt0.o test.o libjpg.a ps2_asm.o libkernel.a libgcc.a ps2cam_rpc.o

clean:
	rm -f *.elf *.o *.a

include $(PS2SDK)/samples/Makefile.pref
include $(PS2SDK)/samples/Makefile.eeglobal

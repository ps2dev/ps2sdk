
all:
	ee-gcc -c test.c 
	ee-gcc -W -EL -G0 -O2 -mips3 -nostdlib -Tlinkfile -o test.elf crt0.o test.o libjpg.a ps2_asm.o libkernel.a libgcc.a ps2cam_rpc.o


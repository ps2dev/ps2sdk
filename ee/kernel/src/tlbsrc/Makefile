# _____     ___ ____     ___ ____
#  ____|   |    ____|   |        | |____|
# |     ___|   |____ ___|    ____| |    \    PS2DEV Open Source Project.
#-----------------------------------------------------------------------
# Copyright 2001-2004, ps2dev - http://www.ps2dev.org
# Licenced under Academic Free License version 2.0
# Review ps2sdk README & LICENSE files for further details.
#

EE_BIN = tlbsrc.bin
EE_ELF = tlbsrc.elf
EE_OBJS = tlbsrc.o

EE_INCS := -I$(PS2SDKSRC)/common/include -I$(PS2SDKSRC)/ee/kernel/include -I$(PS2SDKSRC)/ee/rpc/cdvd/include -I.
EE_CFLAGS = -D_EE -G0 -mno-gpopt -Wall -Os
EE_LDFLAGS = -nostartfiles -mno-crt0 -Tlinkfile -s

$(EE_ELF) : $(EE_OBJS)
	$(EE_CC) $(EE_LDFLAGS) \
		-o $(EE_ELF) $(EE_OBJS) $(EE_LIBS)
	ee-objcopy -Obinary $(EE_ELF) $(EE_BIN)

%.o : %.c
	$(EE_CC) $(EE_CFLAGS) $(EE_INCS) -c $< -o $@

%.o : %.s
	$(EE_AS) $(EE_ASFLAGS) $< -o $@

%.o : %.S
	$(EE_CC) $(EE_CFLAGS) $(EE_INCS) -c $< -o $@

clean:
	rm -f $(EE_OBJS) $(EE_ELF) $(EE_BIN)

include $(PS2SDKSRC)/Defs.make


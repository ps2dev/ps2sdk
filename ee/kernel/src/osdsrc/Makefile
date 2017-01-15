# _____     ___ ____     ___ ____
#  ____|   |    ____|   |        | |____|
# |     ___|   |____ ___|    ____| |    \    PS2DEV Open Source Project.
#-----------------------------------------------------------------------
# Copyright 2001-2004, ps2dev - http://www.ps2dev.org
# Licenced under Academic Free License version 2.0
# Review ps2sdk README & LICENSE files for further details.

EE_CC_VERSION := $(shell $(EE_CC) '--version' 2>&1 | sed -n 's/^.*(GCC) //p')

EE_BIN = osdsrc.bin
EE_ELF = osdsrc.elf
EE_OBJS = osdsrc.o osd.o ExecPS2.o

# Extra macros for disabling the automatic inclusion of the built-in CRT object(s)
ifeq ($(IOP_CC_VERSION),3.2.2)
EE_NO_CRT = -mno-crt0
endif
ifeq ($(IOP_CC_VERSION),3.2.3)
EE_NO_CRT = -mno-crt0
endif

EE_INCS := -I$(PS2SDKSRC)/common/include -I$(PS2SDKSRC)/ee/kernel/include -I$(PS2SDKSRC)/ee/rpc/cdvd/include -I.
EE_CFLAGS = -D_EE -G0 -mno-gpopt -Wall -Os -DREUSE_EXECPS2=1
EE_LDFLAGS = -nostartfiles $(EE_NO_CRT) -Tlinkfile -s

$(EE_ELF) : $(EE_OBJS)
	$(EE_CC) $(EE_LDFLAGS) \
		-o $(EE_ELF) $(EE_OBJS) $(EE_LIBS)
	$(EE_OBJCOPY) -Obinary $(EE_ELF) $(EE_BIN)

%.o : %.c
	$(EE_CC) $(EE_CFLAGS) $(EE_INCS) -c $< -o $@

%.o : %.s
	$(EE_AS) $(EE_ASFLAGS) $< -o $@

%.o : %.S
	$(EE_CC) $(EE_CFLAGS) $(EE_INCS) -c $< -o $@

clean:
	rm -f $(EE_OBJS) $(EE_ELF) $(EE_BIN)

include $(PS2SDKSRC)/Defs.make


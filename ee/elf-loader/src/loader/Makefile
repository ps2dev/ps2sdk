# _____     ___ ____     ___ ____
#  ____|   |    ____|   |        | |____|
# |     ___|   |____ ___|    ____| |    \    PS2DEV Open Source Project.
#-----------------------------------------------------------------------
# (c) 2020 Francisco Javier Trujillo Mata <fjtrujy@gmail.com>
# Licenced under Academic Free License version 2.0
# Review ps2sdk README & LICENSE files for further details.

### Adjust these three, for PS2Link compatibility tests
### Old LaunchELF used LA=0x90000, SA=0xB0000, SS=0x08000
LOADADDR  = 0x90000
STACKADDR = 0xA8000
STACKSIZE = 0x04000

ifeq ($(DEBUG),1)
LOADADDR  = 0x1700000
STACKADDR = 0x1720000
STACKSIZE = 0x08000
endif

LDPARAMS := -Wl,--defsym -Wl,_stack_size=$(STACKSIZE) -Wl,--defsym -Wl,_stack=$(STACKADDR)

EE_LDFLAGS += -Wl,-Ttext -Wl,$(LOADADDR) -s $(LDPARAMS)

EE_BIN = loader.elf

EE_OBJS = loader.o

EE_LIBS =
ifeq ($(DEBUG),1)
EE_LIBS += -ldebug
endif

all: $(EE_BIN)

clean:
	rm -f -r $(EE_OBJS) $(EE_BIN)

include $(PS2SDKSRC)/Defs.make
include Rules.make

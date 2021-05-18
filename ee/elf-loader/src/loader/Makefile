# _____     ___ ____     ___ ____
#  ____|   |    ____|   |        | |____|
# |     ___|   |____ ___|    ____| |    \    PS2DEV Open Source Project.
#-----------------------------------------------------------------------
# (c) 2020 Francisco Javier Trujillo Mata <fjtrujy@gmail.com>
# Licenced under Academic Free License version 2.0
# Review ps2sdk README & LICENSE files for further details.

EE_BIN = loader.elf

EE_OBJS = loader.o
MAPFILE = loader.map

# Include directories
EE_INCS := $(EE_INCS) -I$(PS2SDKSRC)/ee/kernel/include -I$(PS2SDKSRC)/common/include -I$(PS2SDKSRC)/ee/libc/include
EE_LIBS := $(EE_LIBS) -L$(PS2SDKSRC)/ee/kernel/lib -L$(PS2SDKSRC)/ee/libc/lib

EE_CFLAGS = -D_EE -Os -G0 -Wall -Werror
EE_CFLAGS += -fdata-sections -ffunction-sections
EE_LDFLAGS = -Tlinkfile -Wl,-zmax-page-size=128 -Wl,-Map,$(MAPFILE)
EE_LDFLAGS += -s -Wl,--gc-sections

%.o: %.c
	$(EE_CC) $(EE_CFLAGS) $(EE_INCS) -c $< -o $@

$(EE_BIN) : $(EE_OBJS)
	$(EE_CC) $(EE_CFLAGS) $(EE_LDFLAGS) -o $(EE_BIN) $(EE_OBJS) $(EE_LIBS)

all: $(EE_BIN)

clean:
	rm -f -r $(EE_OBJS) $(EE_BIN) $(MAPFILE)

clean_all:
	rm -f -r $(EE_OBJS) $(EE_BIN) $(MAPFILE)

include $(PS2SDKSRC)/Defs.make

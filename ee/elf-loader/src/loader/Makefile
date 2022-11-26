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
EE_INCS := $(EE_INCS) -I$(PS2SDKSRC)/ee/kernel/include -I$(PS2SDKSRC)/common/include -I$(PS2SDKSRC)/ee/libcglue/include
EE_NANO_NEWLIB_LIBS := $(PS2SDKSRC)/ee/rpc/cdvd/lib/libcdvd.a $(PS2SDKSRC)/ee/libcglue/lib/libcglue.a $(PS2SDKSRC)/ee/kernel/lib/libkernel.a
EE_ADDITIONAL_DEPS := $(EE_ADDITIONAL_DEPS) $(EE_NANO_NEWLIB_LIBS)

EE_CFLAGS = -D_EE -Os -G0 -Wall -Werror
EE_CFLAGS += -fdata-sections -ffunction-sections
EE_LDFLAGS += -Tlinkfile -Wl,-zmax-page-size=128 -Wl,-Map,$(MAPFILE)
EE_LDFLAGS += -s -Wl,--gc-sections

# Use nano newlib for saving space
EE_LDFLAGS += -nodefaultlibs -lm_nano -lgcc -Wl,--start-group -lc_nano $(EE_NANO_NEWLIB_LIBS) -Wl,--end-group

%.o: %.c
	$(EE_CC) $(EE_CFLAGS) $(EE_INCS) -c $< -o $@

$(EE_BIN) : $(EE_OBJS) $(EE_LIB_ARCHIVES) $(EE_ADDITIONAL_DEPS)
	$(EE_CC) $(EE_CFLAGS) -o $(EE_BIN) $(EE_OBJS) $(EE_LIBS) $(EE_LDFLAGS)

all: $(EE_BIN)

clean:
	rm -f -r $(EE_OBJS) $(EE_BIN) $(MAPFILE)

clean_all: clean

include $(PS2SDKSRC)/Defs.make

$(PS2SDKSRC)/ee/kernel/lib/libkernel.a:
	$(MAKEREC) $(PS2SDKSRC)/ee/kernel

$(PS2SDKSRC)/ee/rpc/cdvd/lib/libcdvd.a:
	$(MAKEREC) $(PS2SDKSRC)/ee/rpc/cdvd

$(PS2SDKSRC)/ee/libcglue/lib/libcglue.a:
	$(MAKEREC) $(PS2SDKSRC)/ee/libcglue

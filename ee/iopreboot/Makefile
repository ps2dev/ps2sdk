# _____     ___ ____     ___ ____
#  ____|   |    ____|   |        | |____|
# |     ___|   |____ ___|    ____| |    \    PS2DEV Open Source Project.
#-----------------------------------------------------------------------
# Licenced under Academic Free License version 2.0
# Review ps2sdk README & LICENSE files for further details.

IOPCONTROL_SPECIAL_EE_OBJS = _iopcontrol_special_internals.o SifIopRebootBufferEncrypted.o SifIopRebootBuffer.o
IOPCONTROL_SPECIAL_IOP_OBJS = imgdrv_irx.o
IOPCONTROL_SPECIAL_OBJS = 

EE_OBJS = $(IOPCONTROL_SPECIAL_EE_OBJS) $(IOPCONTROL_SPECIAL_IOP_OBJS) erl-support.o

include $(PS2SDKSRC)/Defs.make
include $(PS2SDKSRC)/ee/Rules.lib.make
include $(PS2SDKSRC)/ee/Rules.make
include $(PS2SDKSRC)/ee/Rules.release

$(IOPCONTROL_SPECIAL_EE_OBJS:%=$(EE_OBJS_DIR)%): $(EE_SRC_DIR)SifIopRebootBuffer.c
	$(EE_C_COMPILE) -I$(PS2SDKSRC)/ee/sbv/include  -DF_$(*:$(EE_OBJS_DIR)%=%) $< -c -o $@

$(EE_OBJS_DIR)imgdrv_irx.o: $(EE_OBJS_DIR)imgdrv_irx.c
	$(EE_C_COMPILE) $< -c -o $@

$(PS2SDKSRC)/tools/bin2c/bin/bin2c: $(PS2SDKSRC)/tools/bin2c
	$(MAKEREC) $<

$(EE_SRC_DIR)imgdrv/irx/imgdrv.irx: $(EE_SRC_DIR)imgdrv
	$(MAKEREC) $<

$(EE_OBJS_DIR)imgdrv_irx.c: $(EE_SRC_DIR)imgdrv/irx/imgdrv.irx $(PS2SDKSRC)/tools/bin2c/bin/bin2c
	$(PS2SDKSRC)/tools/bin2c/bin/bin2c $< $@ _imgdrv_irx

clean::
	$(MAKEREC) $(EE_SRC_DIR)imgdrv clean

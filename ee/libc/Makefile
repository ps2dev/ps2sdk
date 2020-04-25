# _____     ___ ____     ___ ____
#  ____|   |    ____|   |        | |____|
# |     ___|   |____ ___|    ____| |    \    PS2DEV Open Source Project.
#-----------------------------------------------------------------------
# Copyright 2001-2004, ps2dev - http://www.ps2dev.org
# Licenced under Academic Free License version 2.0
# Review ps2sdk README & LICENSE files for further details.

EE_LIB = libps2sdkc.a

CORE_OBJS = sleep.o sbrk.o terminate.o ps2sdkapi.o

INIT_OBJS = init_libc.o init_args.o

SJIS_OBJS = _sjis_internals.o strcpy_ascii.o strcpy_sjis.o

TIME_OBJS = ps2_clock.o __time_internals.o

EE_OBJS = $(CORE_OBJS) $(SJIS_OBJS) $(TIME_OBJS) $(INIT_OBJS)

include $(PS2SDKSRC)/Defs.make
include $(PS2SDKSRC)/ee/Rules.lib.make
include $(PS2SDKSRC)/ee/Rules.make
include $(PS2SDKSRC)/ee/Rules.release

$(SJIS_OBJS:%=$(EE_OBJS_DIR)%): $(EE_SRC_DIR)sjis.c
	$(EE_C_COMPILE) -DF_$(*:$(EE_OBJS_DIR)%=%) $< -c -o $@

$(TIME_OBJS:%=$(EE_OBJS_DIR)%): $(EE_SRC_DIR)time.c
	$(EE_C_COMPILE) -DF_$(*:$(EE_OBJS_DIR)%=%) $< -c -o $@

$(INIT_OBJS:%=$(EE_OBJS_DIR)%): $(EE_SRC_DIR)init.c
	$(EE_C_COMPILE) -DF_$(*:$(EE_OBJS_DIR)%=%) $< -c -o $@


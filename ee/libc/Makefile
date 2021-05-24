# _____     ___ ____     ___ ____
#  ____|   |    ____|   |        | |____|
# |     ___|   |____ ___|    ____| |    \    PS2DEV Open Source Project.
#-----------------------------------------------------------------------
# Copyright 2001-2004, ps2dev - http://www.ps2dev.org
# Licenced under Academic Free License version 2.0
# Review ps2sdk README & LICENSE files for further details.

EE_LIB = libps2sdkc.a

CORE_OBJS = timezone.o

INIT_OBJS = _ps2sdk_libc_init.o _ps2sdk_libc_deinit.o _ps2sdk_args_parse.o

SLEEP_OBJS = nanosleep.o

TERMINATE_OBJS = abort.o exit.o

SJIS_OBJS = isSpecialSJIS.o isSpecialASCII.o strcpy_ascii.o strcpy_sjis.o

TIME_OBJS = ps2_clock.o __time_internals.o

PS2SDKAPI_OBJS = __direct_pwd.o __transform_errno.o __fill_stat.o _ps2sdk_ioctl.o compile_time_check.o _open.o _close.o _read.o _write.o \
	_fstat.o _stat.o access.o opendir.o readdir.o rewinddir.o closedir.o _lseek.o lseek64.o chdir.o mkdir.o \
	rmdir.o _link.o _unlink.o getcwd.o _getpid.o _kill.o _sbrk.o _gettimeofday.o _times.o

EE_OBJS = $(CORE_OBJS) $(SJIS_OBJS) $(TIME_OBJS) $(INIT_OBJS) $(SLEEP_OBJS) $(TERMINATE_OBJS) $(PS2SDKAPI_OBJS)

include $(PS2SDKSRC)/Defs.make
include $(PS2SDKSRC)/ee/Rules.lib.make
include $(PS2SDKSRC)/ee/Rules.make
include $(PS2SDKSRC)/ee/Rules.release

$(SJIS_OBJS:%=$(EE_OBJS_DIR)%): $(EE_SRC_DIR)sjis.c
	$(EE_C_COMPILE) -DF_$(*:$(EE_OBJS_DIR)%=%) $< -c -o $@

$(SLEEP_OBJS:%=$(EE_OBJS_DIR)%): $(EE_SRC_DIR)sleep.c
	$(EE_C_COMPILE) -DF_$(*:$(EE_OBJS_DIR)%=%) $< -c -o $@

$(TERMINATE_OBJS:%=$(EE_OBJS_DIR)%): $(EE_SRC_DIR)terminate.c
	$(EE_C_COMPILE) -DF_$(*:$(EE_OBJS_DIR)%=%) $< -c -o $@

$(PS2SDKAPI_OBJS:%=$(EE_OBJS_DIR)%): $(EE_SRC_DIR)ps2sdkapi.c
	$(EE_C_COMPILE) -DF_$(*:$(EE_OBJS_DIR)%=%) $< -c -o $@

$(TIME_OBJS:%=$(EE_OBJS_DIR)%): $(EE_SRC_DIR)time.c
	$(EE_C_COMPILE) -DF_$(*:$(EE_OBJS_DIR)%=%) $< -c -o $@

$(INIT_OBJS:%=$(EE_OBJS_DIR)%): $(EE_SRC_DIR)init.c
	$(EE_C_COMPILE) -DF_$(*:$(EE_OBJS_DIR)%=%) $< -c -o $@


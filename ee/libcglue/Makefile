# _____     ___ ____     ___ ____
#  ____|   |    ____|   |        | |____|
# |     ___|   |____ ___|    ____| |    \    PS2DEV Open Source Project.
#-----------------------------------------------------------------------
# Copyright 2001-2004, ps2dev - http://www.ps2dev.org
# Licenced under Academic Free License version 2.0
# Review ps2sdk README & LICENSE files for further details.

EE_LIB = libcglue.a

CORE_OBJS = timezone.o

INIT_OBJS = __libpthreadglue_init.o __libpthreadglue_deinit.o _libcglue_init.o _libcglue_deinit.o _libcglue_args_parse.o

SLEEP_OBJS = nanosleep.o

SJIS_OBJS = isSpecialSJIS.o isSpecialASCII.o strcpy_ascii.o strcpy_sjis.o

GLUE_OBJS = __dummy_passwd.o __direct_pwd.o __transform_errno.o __fill_stat.o _ps2sdk_ioctl.o compile_time_check.o _open.o _close.o _read.o _write.o \
	_fstat.o _stat.o lstat.o access.o opendir.o readdir.o rewinddir.o closedir.o _lseek.o lseek64.o chdir.o mkdir.o \
	rmdir.o _link.o _unlink.o getcwd.o _getpid.o _kill.o _fork.o _wait.o _sbrk.o _gettimeofday.o _times.o ftime.o clock_getres.o clock_gettime.o clock_settime.o \
	_isatty.o symlink.o truncate.o chmod.o fchmod.o fchmodat.o pathconf.o \
	readlink.o utime.o fchown.o getentropy.o getpwuid.o fsync.o getpwnam.o getuid.o geteuid.o

EE_OBJS = $(CORE_OBJS) $(SJIS_OBJS) $(TIME_OBJS) $(INIT_OBJS) $(SLEEP_OBJS) $(TERMINATE_OBJS) $(GLUE_OBJS)

include $(PS2SDKSRC)/Defs.make
include $(PS2SDKSRC)/ee/Rules.lib.make
include $(PS2SDKSRC)/ee/Rules.make
include $(PS2SDKSRC)/ee/Rules.release

$(SJIS_OBJS:%=$(EE_OBJS_DIR)%): $(EE_SRC_DIR)sjis.c
	$(EE_C_COMPILE) -DF_$(*:$(EE_OBJS_DIR)%=%) $< -c -o $@

$(SLEEP_OBJS:%=$(EE_OBJS_DIR)%): $(EE_SRC_DIR)sleep.c
	$(EE_C_COMPILE) -DF_$(*:$(EE_OBJS_DIR)%=%) $< -c -o $@

$(GLUE_OBJS:%=$(EE_OBJS_DIR)%): $(EE_SRC_DIR)glue.c
	$(EE_C_COMPILE) -DF_$(*:$(EE_OBJS_DIR)%=%) $< -c -o $@

$(INIT_OBJS:%=$(EE_OBJS_DIR)%): $(EE_SRC_DIR)init.c
	$(EE_C_COMPILE) -DF_$(*:$(EE_OBJS_DIR)%=%) $< -c -o $@


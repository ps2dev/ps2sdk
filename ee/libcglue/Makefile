# _____     ___ ____     ___ ____
#  ____|   |    ____|   |        | |____|
# |     ___|   |____ ___|    ____| |    \    PS2DEV Open Source Project.
#-----------------------------------------------------------------------
# Copyright 2001-2004, ps2dev - http://www.ps2dev.org
# Licenced under Academic Free License version 2.0
# Review ps2sdk README & LICENSE files for further details.

EE_LIB = libcglue.a

CORE_OBJS = rtc.o

TIMEZONE_OBJS = _libcglue_timezone_update.o ps2sdk_setTimezone.o ps2sdk_setDaylightSaving.o

FDMAN_OBJS = __fdman_sema.o __descriptor_data_pool.o __descriptormap.o __fdman_init.o __fdman_deinit.o __fdman_get_new_descriptor.o \
	__fdman_get_dup_descriptor.o __fdman_release_descriptor.o

INIT_OBJS = __libpthreadglue_init.o __libpthreadglue_deinit.o _libcglue_init.o _libcglue_deinit.o _libcglue_args_parse.o

SLEEP_OBJS = nanosleep.o

SJIS_OBJS = isSpecialSJIS.o isSpecialASCII.o strcpy_ascii.o strcpy_sjis.o

CWD_OBJS = __cwd.o __get_drive.o getcwd.o __path_absolute.o __init_cwd.o

PS2SDKAPI_OBJS = \
	_ps2sdk_close.o _ps2sdk_open.o _ps2sdk_read.o _ps2sdk_lseek.o _ps2sdk_lseek64.o _ps2sdk_write.o _ps2sdk_ioctl.o \
	_ps2sdk_remove.o _ps2sdk_rename.o _ps2sdk_mkdir.o _ps2sdk_rmdir.o _ps2sdk_stat.o _ps2sdk_readlink.o _ps2sdk_symlink.o \
	_ps2sdk_dopen.o _ps2sdk_dread.o _ps2sdk_dclose.o


GLUE_OBJS = __dummy_passwd.o __transform_errno.o __transform64_errno.o compile_time_check.o __normalized_path.o _open.o _close.o _read.o _write.o \
	_stat.o lstat.o _fstat.o access.o _fcntl.o getdents.o _lseek.o lseek64.o chdir.o mkdir.o \
	rmdir.o _link.o _unlink.o _rename.o _getpid.o _kill.o _fork.o _wait.o _sbrk.o _gettimeofday.o _times.o ftime.o clock_getres.o clock_gettime.o clock_settime.o \
	truncate.o symlink.o readlink.o utime.o fchown.o getrandom.o getentropy.o _isatty.o chmod.o fchmod.o fchmodat.o pathconf.o \
	fsync.o getuid.o geteuid.o getpwuid.o getpwnam.o ps2sdk_get_iop_fd.o

LOCK_OBJS = __retarget_lock_init.o __retarget_lock_acquire.o __retarget_lock_release.o __retarget_lock_try_acquire.o __retarget_lock_close.o \
	__retarget_lock_init_recursive.o __retarget_lock_acquire_recursive.o __retarget_lock_release_recursive.o __retarget_lock_try_acquire_recursive.o __retarget_lock_close_recursive.o

EE_OBJS = $(CORE_OBJS) $(TIMEZONE_OBJS) $(SJIS_OBJS) $(TIME_OBJS) $(FDMAN_OBJS) $(INIT_OBJS) $(SLEEP_OBJS) $(TERMINATE_OBJS) $(CWD_OBJS) $(PS2SDKAPI_OBJS) $(GLUE_OBJS) $(LOCK_OBJS)

include $(PS2SDKSRC)/Defs.make
include $(PS2SDKSRC)/ee/Rules.lib.make
include $(PS2SDKSRC)/ee/Rules.make
include $(PS2SDKSRC)/ee/Rules.release

$(TIMEZONE_OBJS:%=$(EE_OBJS_DIR)%): $(EE_SRC_DIR)timezone.c
	$(DIR_GUARD)
	$(EE_C_COMPILE) -DF_$(*:$(EE_OBJS_DIR)%=%) $< -c -o $@

$(SJIS_OBJS:%=$(EE_OBJS_DIR)%): $(EE_SRC_DIR)sjis.c
	$(DIR_GUARD)
	$(EE_C_COMPILE) -DF_$(*:$(EE_OBJS_DIR)%=%) $< -c -o $@

$(SLEEP_OBJS:%=$(EE_OBJS_DIR)%): $(EE_SRC_DIR)sleep.c
	$(DIR_GUARD)
	$(EE_C_COMPILE) -DF_$(*:$(EE_OBJS_DIR)%=%) $< -c -o $@

$(GLUE_OBJS:%=$(EE_OBJS_DIR)%): $(EE_SRC_DIR)glue.c
	$(DIR_GUARD)
	$(EE_C_COMPILE) -DF_$(*:$(EE_OBJS_DIR)%=%) $< -c -o $@

$(CWD_OBJS:%=$(EE_OBJS_DIR)%): $(EE_SRC_DIR)cwd.c
	$(DIR_GUARD)
	$(EE_C_COMPILE) -DF_$(*:$(EE_OBJS_DIR)%=%) $< -c -o $@

$(PS2SDKAPI_OBJS:%=$(EE_OBJS_DIR)%): $(EE_SRC_DIR)ps2sdkapi.c
	$(DIR_GUARD)
	$(EE_C_COMPILE) -DF_$(*:$(EE_OBJS_DIR)%=%) $< -c -o $@

$(FDMAN_OBJS:%=$(EE_OBJS_DIR)%): $(EE_SRC_DIR)fdman.c
	$(DIR_GUARD)
	$(EE_C_COMPILE) -DF_$(*:$(EE_OBJS_DIR)%=%) $< -c -o $@

$(INIT_OBJS:%=$(EE_OBJS_DIR)%): $(EE_SRC_DIR)init.c
	$(DIR_GUARD)
	$(EE_C_COMPILE) -DF_$(*:$(EE_OBJS_DIR)%=%) $< -c -o $@

$(LOCK_OBJS:%=$(EE_OBJS_DIR)%): $(EE_SRC_DIR)lock.c
	$(DIR_GUARD)
	$(EE_C_COMPILE) -DF_$(*:$(EE_OBJS_DIR)%=%) $< -c -o $@


add_library(cglue)
target_include_directories(cglue PUBLIC ${EE_INC} include)
target_sources(cglue PRIVATE src/rtc.c)
target_link_libraries(cglue PRIVATE kernel)

compile_multiple(cglue src/timezone.c OBJECTS
    _libcglue_timezone_update.o ps2sdk_setTimezone.o ps2sdk_setDaylightSaving.o
)

compile_multiple(cglue src/fdman.c OBJECTS
	__fdman_sema.o
	__descriptor_data_pool.o
	__descriptormap.o
	__fdman_init.o
	__fdman_deinit.o
	__fdman_get_new_descriptor.o
	__fdman_get_dup_descriptor.o
	__fdman_get_dup2_descriptor.o
	__fdman_release_descriptor.o
)

compile_multiple(cglue src/init.c OBJECTS
	__gprof_init.o
	__gprof_cleanup.o
	__libpthreadglue_init.o
	__libpthreadglue_deinit.o
	_libcglue_init.o
	_libcglue_deinit.o
	_libcglue_args_parse.o
)

compile_multiple(cglue src/sleep.c OBJECTS
    nanosleep.o
)

compile_multiple(cglue src/sjis.c OBJECTS
    isSpecialSJIS.o isSpecialASCII.o strcpy_ascii.o strcpy_sjis.o
)

compile_multiple(cglue src/cwd.c OBJECTS
    __cwd.o __cwd_len.o __get_drive.o getcwd.o __path_absolute.o __init_cwd.o
)

compile_multiple(cglue src/ps2sdkapi.c OBJECTS
	__fioOpsInitialize.o
	__fioOpenHelper.o
	__fioGetFdHelper.o
	__fioGetFilenameHelper.o
	__fioCloseHelper.o
	__fioDcloseHelper.o
	__fioReadHelper.o
	__fioLseekHelper.o
	__fioWriteHelper.o
	__fioIoctlHelper.o
	__fioDreadHelper.o
	__fioLseekDirHelper.o
	__libcglue_init_stdio.o
	__fioMkdirHelper.o
	__fioGetstatHelper.o
	__fioOpsInitializeImpl.o
	_libcglue_fdman_path_ops.o
	_libcglue_fdman_socket_ops.o
	_libcglue_fdman_inet_ops.o
)

compile_multiple(cglue src/glue.c OBJECTS
	__dummy_passwd.o
	__transform_errno.o
	__transform64_errno.o
	compile_time_check.o
	_open.o
	_close.o
	_read.o
	_write.o
	_stat.o
	lstat.o
	_fstat.o
	access.o
	_fcntl.o
	getdents.o
	_lseek.o
	lseek64.o
	chdir.o
	mkdir.o
	rmdir.o
	_link.o
	_unlink.o
	_rename.o
	_getpid.o
	_kill.o
	_fork.o
	_wait.o
	_execve.o
	_sbrk.o
	_gettimeofday.o
	_times.o
	ftime.o
	clock_getres.o
	clock_gettime.o
	clock_settime.o
	truncate.o
	symlink.o
	readlink.o
	utime.o
	fchown.o
	getrandom.o
	_getentropy.o
	_isatty.o
	chmod.o
	fchmod.o
	pathconf.o
	fsync.o
	getuid.o
	geteuid.o
	getpwuid.o
	getpwnam.o
	libcglue_get_fd_info.o
	ps2sdk_get_iop_fd.o
	ps2sdk_get_iop_filename.o
	_ps2sdk_close.o
	_ps2sdk_dclose.o
	_ps2sdk_read.o
	_ps2sdk_lseek.o
	_ps2sdk_lseek64.o
	_ps2sdk_write.o
	_ps2sdk_ioctl.o
	_ps2sdk_ioctl2.o
	_ps2sdk_dread.o
	openat.o
	renameat.o
	fchmodat.o
	fstatat.o
	mkdirat.o
	faccessat.o
	fchownat.o
	linkat.o
	readlinkat.o
	unlinkat.o
	dup.o
	dup2.o
)

compile_multiple(cglue src/lock.c OBJECTS
	__lock___sfp_recursive_mutex.o
	__lock___atexit_recursive_mutex.o
	__lock___at_quick_exit_mutex.o
	__lock___malloc_recursive_mutex.o
	__lock___env_recursive_mutex.o
	__lock___tz_mutex.o
	__lock___dd_hash_mutex.o
	__lock___arc4random_mutex.o
	__retarget_lock_init.o
	__retarget_lock_init_recursive.o
	__retarget_lock_close.o
	__retarget_lock_close_recursive.o
	__retarget_lock_acquire.o
	__retarget_lock_acquire_recursive.o
	__retarget_lock_try_acquire.o
	__retarget_lock_try_acquire_recursive.o
	__retarget_lock_release.o
	__retarget_lock_release_recursive.o
	__locks_init.o
	__locks_deinit.o
)

compile_multiple(cglue src/netdb.c OBJECTS
	gethostbyaddr.o
	gethostbyname.o
	gethostbyname_r.o
	freeaddrinfo.o
	getaddrinfo.o
)

compile_multiple(cglue src/select.c OBJECTS
    select.o
)

compile_multiple(cglue src/socket.c OBJECTS
	socket.o
	accept.o
	bind.o
	connect.o
	listen.o
	recv.o
	recvfrom.o
	recvmsg.o
	send.o
	sendto.o
	sendmsg.o
	getsockopt.o
	setsockopt.o
	shutdown.o
	getpeername.o
	getsockname.o
	libcglue_inet_addr.o
	libcglue_inet_ntoa.o
	libcglue_inet_ntoa_r.o
	libcglue_inet_aton.o
	libcglue_ps2ip_setconfig.o
	libcglue_ps2ip_getconfig.o
	libcglue_dns_setserver.o
	libcglue_dns_getserver.o
)

install(TARGETS cglue)
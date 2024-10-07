add_library(fileXio)
target_include_directories(fileXio PUBLIC include)
target_include_directories(fileXio PRIVATE ${EE_INC} ${PROJECT_SOURCE_DIR}/libcglue/include)

compile_multiple(fileXio src/fileXio_rpc.c OBJECTS 
	__cd0.o __sbuff.o __intr_data.o __fileXioInited.o __fileXioBlockMode.o __fileXioCompletionSema.o __lock_sema_id.o
	fileXioInit.o fileXioExit.o fileXioStop.o fileXioGetDeviceList.o fileXioGetdir.o fileXioMount.o fileXioUmount.o
	fileXioCopyfile.o fileXioMkdir.o fileXioRmdir.o fileXioRemove.o fileXioRename.o fileXioSymlink.o fileXioReadlink.o
	fileXioChdir.o fileXioOpen.o fileXioClose.o fileXioRead.o fileXioWrite.o fileXioLseek.o fileXioLseek64.o fileXioChStat.o
	fileXioGetStat.o fileXioFormat.o fileXioSync.o fileXioDopen.o fileXioDclose.o fileXioDread.o fileXioDevctl.o
	fileXioIoctl.o fileXioIoctl2.o fileXioWaitAsync.o fileXioSetBlockMode.o fileXioSetRWBufferSize.o
)

compile_multiple(fileXio src/fileXio_ps2sdk.c OBJECTS 
	__fileXioOpsInitialize.o
	__fileXioOpenHelper.o
	__fileXioGetFdHelper.o
	__fileXioGetFilenameHelper.o
	__fileXioGetstatHelper.o
	__fileXioCloseHelper.o
	__fileXioDcloseHelper.o
	__fileXioReadHelper.o
	__fileXioLseekHelper.o
	__fileXioLseek64Helper.o
	__fileXioWriteHelper.o
	__fileXioIoctlHelper.o
	__fileXioIoctl2Helper.o
	__fileXioDreadHelper.o
	__fileXioLseekDirHelper.o
	__fileXioOpsInitializeImpl.o
	_ps2sdk_fileXio_init_deinit.o
)

target_sources(fileXio PRIVATE src/erl-support.c)
install(TARGETS fileXio)
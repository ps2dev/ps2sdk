#include "irx.h"

/* Please keep these in alphabetical order!  */
#include "dmacman.h"
#include "intrman.h"
#include "ioman.h"
#include "libsd.h"
#include "loadcore.h"
#include "sifcmd.h"
#include "stdio.h"
#include "sysclib.h"
#include "sysmem.h"
#include "thbase.h"
#include "thevent.h"
#include "thmsgbx.h"
#include "thsemap.h"
#include "usbd.h"
#include "vblank.h"


stdio_IMPORTS_start
I_printf
stdio_IMPORTS_end

loadcore_IMPORTS_start
I_FlushDcache
loadcore_IMPORTS_end

sysclib_IMPORTS_start
I_memset
I_memcmp
I_memcpy
I_strlen
I_strcmp
I_strcpy
I_strncpy
I_strncmp
sysclib_IMPORTS_end

thsemap_IMPORTS_start
I_CreateSema
I_SignalSema
I_iSignalSema
I_WaitSema
I_DeleteSema
I_PollSema
thsemap_IMPORTS_end

thbase_IMPORTS_start
I_CreateThread 
I_DeleteThread
I_StartThread 
I_SleepThread 
I_GetThreadId 
I_ExitDeleteThread
I_DelayThread 
thbase_IMPORTS_end

sysmem_IMPORTS_start
I_AllocSysMemory
I_FreeSysMemory
sysmem_IMPORTS_end

ioman_IMPORTS_start
I_AddDrv
I_DelDrv
I_open
I_close
I_read
I_write
ioman_IMPORTS_end



sifcmd_IMPORTS_start
I_sceSifInitRpc 
I_sceSifSetRpcQueue 
I_sceSifRegisterRpc 
I_sceSifRpcLoop 
sifcmd_IMPORTS_end


usbd_IMPORTS_start
I_UsbGetDeviceStaticDescriptor
I_UsbOpenEndpoint
I_UsbOpenBulkEndpoint
I_UsbCloseEndpoint
I_UsbSetDevicePrivateData
I_UsbTransfer
I_UsbRegisterDriver
usbd_IMPORTS_end

vblank_IMPORTS_start
I_WaitVblankStart
I_WaitVblankEnd
I_WaitVblank
I_RegisterVblankHandler
vblank_IMPORTS_end
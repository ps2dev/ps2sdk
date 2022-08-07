#include <stdio.h>
#include <string.h>
#include <thsemap.h>

#include "ioplib.h"
#include "sio2man.h"
#include "sio2man_hook.h"

// #define DEBUG  //comment out this line when not debugging
#include "module_debug.h"

#define PORT_NR 3

static int lock_sema      = -1;
static int lock_sema2     = -1;
static u16 hooked_version = 0;


// sio2man function typedefs
typedef void (*psio2_transfer_init)(void);
typedef int (*psio2_transfer)(sio2_transfer_data_t *td);
typedef void (*psio2_transfer_reset)(void);

// Original sio2man function pointers
psio2_transfer_init _23_psio2_pad_transfer_init;
psio2_transfer_init _24_psio2_mc_transfer_init;
psio2_transfer_init _46_psio2_pad_transfer_init;
psio2_transfer_init _47_psio2_mc_transfer_init;
psio2_transfer_init _48_psio2_mtap_transfer_init;
psio2_transfer_init _49_psio2_rm_transfer_init;
psio2_transfer_init _50_psio2_unk_transfer_init;
psio2_transfer _25_psio2_transfer;
psio2_transfer _51_psio2_transfer;
psio2_transfer_reset _26_psio2_transfer_reset;
psio2_transfer_reset _52_psio2_transfer_reset;

// Generic sio2man init function
static void _sio2_transfer_init(psio2_transfer_init init_func)
{
    //M_DEBUG("%s\n", __FUNCTION__);

    WaitSema(lock_sema);
    init_func();
}

// Generic sio2man transfer function
static int _sio2_transfer(psio2_transfer transfer_func, sio2_transfer_data_t *td)
{
    int rv, i;

    //M_DEBUG("%s\n", __FUNCTION__);

    // Do not allow transfers to/from our used port
    for (i = 0; i < 16; i++) {
        // Last transfer, we're ok.
        if (td->regdata[i] == 0)
            break;

        // Wrong port, abort.
        if ((td->regdata[i] & 3) == PORT_NR)
            return 1;
    }

    WaitSema(lock_sema2);
    rv = transfer_func(td);
    SignalSema(lock_sema2);

    return rv;
}

// Generic sio2man transfer_reset function
static void _sio2_transfer_reset(psio2_transfer_reset reset_func)
{
    //M_DEBUG("%s\n", __FUNCTION__);

    reset_func();
    SignalSema(lock_sema);
}

// Hooked sio2man functions
static void _23_sio2_pad_transfer_init() { _sio2_transfer_init(_23_psio2_pad_transfer_init); }
static void _24_sio2_mc_transfer_init() { _sio2_transfer_init(_24_psio2_mc_transfer_init); }
static void _46_sio2_pad_transfer_init() { _sio2_transfer_init(_46_psio2_pad_transfer_init); }
static void _47_sio2_mc_transfer_init() { _sio2_transfer_init(_47_psio2_mc_transfer_init); }
static void _48_sio2_mtap_transfer_init() { _sio2_transfer_init(_48_psio2_mtap_transfer_init); }
static void _49_sio2_rm_transfer_init() { _sio2_transfer_init(_49_psio2_rm_transfer_init); }
static void _50_sio2_unk_transfer_init() { _sio2_transfer_init(_50_psio2_unk_transfer_init); }
static void _26_sio2_transfer_reset() { _sio2_transfer_reset(_26_psio2_transfer_reset); }
static void _52_sio2_transfer_reset() { _sio2_transfer_reset(_52_psio2_transfer_reset); }
static int  _25_sio2_transfer(sio2_transfer_data_t *td) { return _sio2_transfer(_25_psio2_transfer, td); }
static int  _51_sio2_transfer(sio2_transfer_data_t *td) { return _sio2_transfer(_51_psio2_transfer, td); }

static void _sio2man_unhook(iop_library_t *lib)
{
    if (hooked_version == 0) {
        M_DEBUG("Warning: trying to unhook sio2man while not hooked\n");
        return;
    }

    ioplib_hookExportEntry(lib, 23, _23_psio2_pad_transfer_init);
    ioplib_hookExportEntry(lib, 24, _24_psio2_mc_transfer_init);
    ioplib_hookExportEntry(lib, 25, _25_psio2_transfer);
    ioplib_hookExportEntry(lib, 26, _26_psio2_transfer_reset);
    ioplib_hookExportEntry(lib, 46, _46_psio2_pad_transfer_init);
    ioplib_hookExportEntry(lib, 47, _47_psio2_mc_transfer_init);
    ioplib_hookExportEntry(lib, 48, _48_psio2_mtap_transfer_init);

    if ((hooked_version >= IRX_VER(1, 2)) && (hooked_version < IRX_VER(2, 0))) {
        // Only for the newer rom0:XSIO2MAN
        // Assume all v1.x libraries to use this interface (reset at 50)
        ioplib_hookExportEntry(lib, 49, _51_psio2_transfer);
        ioplib_hookExportEntry(lib, 50, _52_psio2_transfer_reset);
    } else /*if (hooked_version >= IRX_VER(2, 3))*/ {
        // Only for the newer rom1:SIO2MAN
        // Assume all v2.x libraries to use this interface (reset at 52)
        ioplib_hookExportEntry(lib, 49, _49_psio2_rm_transfer_init);
        ioplib_hookExportEntry(lib, 50, _50_psio2_unk_transfer_init);
        ioplib_hookExportEntry(lib, 51, _51_psio2_transfer);
        ioplib_hookExportEntry(lib, 52, _52_psio2_transfer_reset);
    }

    hooked_version = 0;
}

static void _sio2man_hook(iop_library_t *lib)
{
    if (hooked_version != 0) {
        M_DEBUG("Warning: trying to hook sio2man version 0x%x\n", lib->version);
        M_DEBUG("         while version 0x%x already hooked\n", hooked_version);
        return;
    }

    // Only the newer sio2man libraries (with reset functions) are supported
    if (lib->version > IRX_VER(1, 1)) {
        M_DEBUG("Installing sio2man hooks for version 0x%x\n", lib->version);

        _23_psio2_pad_transfer_init  = ioplib_hookExportEntry(lib, 23, _23_sio2_pad_transfer_init);
        // Lock sio2 to prevent race conditions with MC/PAD libraries
        _23_psio2_pad_transfer_init();

        _24_psio2_mc_transfer_init   = ioplib_hookExportEntry(lib, 24, _24_sio2_mc_transfer_init);
        _25_psio2_transfer           = ioplib_hookExportEntry(lib, 25, _25_sio2_transfer);
        _26_psio2_transfer_reset     = ioplib_hookExportEntry(lib, 26, _26_sio2_transfer_reset);
        _46_psio2_pad_transfer_init  = ioplib_hookExportEntry(lib, 46, _46_sio2_pad_transfer_init);
        _47_psio2_mc_transfer_init   = ioplib_hookExportEntry(lib, 47, _47_sio2_mc_transfer_init);
        _48_psio2_mtap_transfer_init = ioplib_hookExportEntry(lib, 48, _48_sio2_mtap_transfer_init);

        if ((lib->version >= IRX_VER(1, 2)) && (lib->version < IRX_VER(2, 0))) {
            // Only for the newer rom0:XSIO2MAN
            // Assume all v1.x libraries to use this interface (reset at 50)
            _51_psio2_transfer       = ioplib_hookExportEntry(lib, 49, _51_sio2_transfer);
            _52_psio2_transfer_reset = ioplib_hookExportEntry(lib, 50, _52_sio2_transfer_reset);
        } else /*if (lib->version >= IRX_VER(2, 3))*/ {
            // Only for the newer rom1:SIO2MAN
            // Assume all v2.x libraries to use this interface (reset at 52)
            _49_psio2_rm_transfer_init  = ioplib_hookExportEntry(lib, 49, _49_sio2_rm_transfer_init);
            _50_psio2_unk_transfer_init = ioplib_hookExportEntry(lib, 50, _50_sio2_unk_transfer_init);
            _51_psio2_transfer          = ioplib_hookExportEntry(lib, 51, _51_sio2_transfer);
            _52_psio2_transfer_reset    = ioplib_hookExportEntry(lib, 52, _52_sio2_transfer_reset);
        }

        // Unlock sio2
        _26_psio2_transfer_reset();

        hooked_version = lib->version;
    } else {
        M_DEBUG("ERROR: sio2man version 0x%x not supported\n", lib->version);
    }
}

int (*pRegisterLibraryEntries)(iop_library_t *lib);
static int hookRegisterLibraryEntries(iop_library_t *lib)
{
    M_DEBUG("RegisterLibraryEntries: %s 0x%x\n", lib->name, lib->version);

    if (!strcmp(lib->name, "sio2man"))
        _sio2man_hook(lib);

    return pRegisterLibraryEntries(lib);
}

int sio2man_hook_init()
{
    iop_sema_t sema;
    iop_library_t *lib;

    M_DEBUG("%s\n", __FUNCTION__);

    // Create semaphore for locking sio2man exclusively
    sema.attr    = 1;
    sema.initial = 1;
    sema.max     = 1;
    sema.option  = 0;
    lock_sema    = CreateSema(&sema);
    lock_sema2   = CreateSema(&sema);

    // Hook into 'loadcore' so we know when sio2man is loaded in the future
    lib = ioplib_getByName("loadcore");
    if (lib == NULL) {
        DeleteSema(lock_sema);
        DeleteSema(lock_sema2);
        return -1;
    }
    pRegisterLibraryEntries = ioplib_hookExportEntry(lib, 6, hookRegisterLibraryEntries);

    // Hook into 'sio2man' now if it's already loaded
    lib = ioplib_getByName("sio2man");
    if (lib != NULL) {
        _sio2man_hook(lib);
        ioplib_relinkExports(lib);
    }

    return 0;
}

void sio2man_hook_deinit()
{
    iop_library_t *lib;

    M_DEBUG("%s\n", __FUNCTION__);

    // Unhook 'sio2man'
    lib = ioplib_getByName("sio2man");
    if (lib != NULL) {
        _sio2man_unhook(lib);
        ioplib_relinkExports(lib);
    }

    // Unhook 'loadcore'
    ioplib_hookExportEntry(lib, 6, pRegisterLibraryEntries);

    // Delete locking semaphore
    DeleteSema(lock_sema);
    DeleteSema(lock_sema2);
}

void sio2man_hook_sio2_lock()
{
    // Lock sio2man driver so we can use it exclusively
    WaitSema(lock_sema);
    WaitSema(lock_sema2);
}

void sio2man_hook_sio2_unlock()
{
    // Unlock sio2man driver
    SignalSema(lock_sema2);
    SignalSema(lock_sema);
}

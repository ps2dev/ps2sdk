/*
# _____     ___ ____     ___ ____
#  ____|   |    ____|   |        | |____|
# |     ___|   |____ ___|    ____| |    \    PS2DEV Open Source Project.
#-----------------------------------------------------------------------
# Copyright ps2dev - http://www.ps2dev.org
# Licenced under Academic Free License version 2.0
# Review ps2sdk README & LICENSE files for further details.
#
# taken from MX4SIO driver for simplicity.
# all credits go to maximus32
*/

#include <stdio.h>
#include <string.h>
#include <thsemap.h>
#include <intrman.h>

#include "ioplib.h"
#include "sio2man.h"
#include "sio2man_hook.h"

// #define DEBUG  //comment out this line when not debugging
#include "module_debug.h"

#define PORT_NR 3
#ifndef M_DEBUG
#define M_DEBUG DPRINTF
#endif

// sio2man function typedefs
typedef void (*psio2_transfer_init)(void);
typedef int (*psio2_transfer)(sio2_transfer_data_t *td);
typedef void (*psio2_transfer_reset)(void);

struct sio2man_hook_data
{
    iop_library_t *m_lib;
    // Original sio2man function pointers
    psio2_transfer_init m_23_psio2_pad_transfer_init;
#ifdef PORT_NR
    psio2_transfer m_25_psio2_transfer;
#endif
    psio2_transfer_reset m_26_psio2_transfer_reset;
};

static struct sio2man_hook_data g_hook_data[2];
static iop_library_t *g_loadcore_lib;
#if SIO2MAN_HOOK_SUPPORT_SET_INTR_HANDLER
static iop_library_t *g_intrman_lib;
static int (*g_sio2man_intr_handler_ptr)(void *arg);
static void *g_sio2man_intr_arg_ptr;
#endif

#ifdef PORT_NR
static sio2_transfer_data_t g_null_td;
// Generic sio2man transfer function
static int _sio2_transfer(psio2_transfer transfer_func, sio2_transfer_data_t *td)
{
    unsigned int i;

    //M_DEBUG("%s\n", __FUNCTION__);

    // Do not allow transfers to/from our used port
    for (i = 0; i < (sizeof(td->regdata)/sizeof(td->regdata[0])); i++) {
        // Last transfer, we're ok.
        if (td->regdata[i] == 0)
            break;

        // Wrong port; behave as a no-op.
        // This will allow the transfer reset behavior for old library to work.
        if ((td->regdata[i] & 3) == PORT_NR)
            return transfer_func(&g_null_td);
    }

    return transfer_func(td);
}

// Hooked sio2man functions
static int _1_25_sio2_transfer(sio2_transfer_data_t *td) { return _sio2_transfer(g_hook_data[0].m_25_psio2_transfer, td); }
static int _2_25_sio2_transfer(sio2_transfer_data_t *td) { return _sio2_transfer(g_hook_data[1].m_25_psio2_transfer, td); }
#endif

static void unhook_loadcore(void);
#if SIO2MAN_HOOK_SUPPORT_SET_INTR_HANDLER
static void unhook_intrman(void);
#endif

static void _sio2man_hook(iop_library_t *lib)
{
    int state;

    // Disable interrupts to prevent race conditions with MC/PAD libraries
    CpuSuspendIntr(&state);
    // Only the newest sio2man libraries (using one semaphore for locks for all devices) are supported
    switch (lib->version)
    {
        case IRX_VER(1, 2):
        case IRX_VER(2, 7):
        {
            int major_version;

            M_DEBUG("Installing sio2man hooks for version 0x%x\n", lib->version);

            major_version = ((lib->version >> 8) & 0xFF);
            if (g_hook_data[major_version - 1].m_lib) {
                M_DEBUG("Warning: trying to hook sio2man version 0x%x\n", lib->version);
                M_DEBUG("         while version 0x%x already hooked\n", g_hook_data[major_version - 1].m_lib->version);
            }
            g_hook_data[major_version - 1].m_lib = lib;
            g_hook_data[major_version - 1].m_23_psio2_pad_transfer_init  = lib->exports[23];
#ifdef PORT_NR
            g_hook_data[major_version - 1].m_25_psio2_transfer           = ioplib_hookSameExportEntries(lib, 25, (major_version == 1) ? &_1_25_sio2_transfer : &_2_25_sio2_transfer);
#endif
            g_hook_data[major_version - 1].m_26_psio2_transfer_reset     = lib->exports[26];
#ifdef PORT_NR
            ioplib_relinkExports(lib);
#endif
            break;
        }
        default:
        {
            M_DEBUG("ERROR: sio2man version 0x%x not supported\n", lib->version);
            break;
        }
    }
    if (g_hook_data[0].m_lib && g_hook_data[1].m_lib)
        unhook_loadcore();
    CpuResumeIntr(state);
}

#if SIO2MAN_HOOK_SUPPORT_SET_INTR_HANDLER
// intrman hook
static int (*pRegisterIntrHandler)(int irq, int mode, int (*handler)(void *), void *arg);
static int hookRegisterIntrHandler(int irq, int mode, int (*handler)(void *), void *arg)
{
    int ret;
    M_DEBUG("RegisterIntrHandler irq: %i, handler @ 0x%p\n", irq, handler);

    ret = pRegisterIntrHandler(irq, mode, handler, arg);
    // If handler already exists or other error, then do not save handler
    if (ret)
        return ret;
    // SIO2 interrupt
    if (irq == IOP_IRQ_SIO2 && !g_sio2man_intr_handler_ptr) {
        g_sio2man_intr_handler_ptr = handler;
        g_sio2man_intr_arg_ptr = arg;
        M_DEBUG("Got SIO2MAN intr handler @ 0x%p, arg @ 0x%p\n", handler, arg);
        unhook_intrman();
    }
    return ret;
}
#endif

// loadcore hook
static int (*pRegisterLibraryEntries)(iop_library_t *lib);
static int hookRegisterLibraryEntries(iop_library_t *lib)
{
    int ret;
    M_DEBUG("RegisterLibraryEntries: %s 0x%x\n", lib->name, lib->version);

    ret = pRegisterLibraryEntries(lib);
    // If library already exists or other error, then do not hook
    if (ret)
        return ret;
    if (!strcmp(lib->name, "sio2man"))
        _sio2man_hook(lib);
    return ret;
}

static void unhook_loadcore(void)
{
    int state;

    CpuSuspendIntr(&state);
    if (g_loadcore_lib) {
        if (pRegisterLibraryEntries)
            ioplib_hookSameExportEntries(g_loadcore_lib, 6, pRegisterLibraryEntries);
        g_loadcore_lib = NULL;
    }
    CpuResumeIntr(state);
}

#if SIO2MAN_HOOK_SUPPORT_SET_INTR_HANDLER
static void unhook_intrman(void)
{
    int state;

    CpuSuspendIntr(&state);
    if (g_intrman_lib) {
        if (pRegisterIntrHandler)
            ioplib_hookSameExportEntries(g_intrman_lib, 4, pRegisterIntrHandler);
        g_intrman_lib = NULL;
    }
    CpuResumeIntr(state);
}
#endif

static int generic_hook_iterate_cb(iop_library_t *lib, void *userdata)
{
    (void)userdata;
    if (userdata && !*(iop_library_t **)userdata)
        *(iop_library_t **)userdata = lib;
    return 1;
}

static int sio2man_hook_iterate_cb(iop_library_t *lib, void *userdata)
{
    (void)userdata;
    switch ((lib->version >> 8) & 0xFF)
    {
        case 1:
        case 2:
        {
            _sio2man_hook(lib);
            return (g_hook_data[0].m_lib && g_hook_data[1].m_lib) ? 1 : 0;
        }
        default:
            return 1;
    }
}

int sio2man_hook_init()
{
    M_DEBUG("%s\n", __FUNCTION__);

#if SIO2MAN_HOOK_SUPPORT_SET_INTR_HANDLER
    {
        intrman_internals_t *intrman_internals;

        intrman_internals = GetIntrmanInternalData();
        // Get sio2man intr handler
        g_sio2man_intr_handler_ptr = intrman_internals->interrupt_handler_table[IOP_IRQ_SIO2].handler;
        g_sio2man_intr_arg_ptr = intrman_internals->interrupt_handler_table[IOP_IRQ_SIO2].userdata;
    }
#endif

    // Hook into 'sio2man' now if it's already loaded
    if (!ioplib_iterateByName("sio2man", &sio2man_hook_iterate_cb, NULL)) {
        // Hook into 'loadcore' so we know when sio2man is loaded in the future
        ioplib_iterateByName("loadcore", &generic_hook_iterate_cb, &g_loadcore_lib);
#if SIO2MAN_HOOK_SUPPORT_SET_INTR_HANDLER
        // Hook into 'intrman' so we can get its handlers when they are registered
        ioplib_iterateByName("intrman", &generic_hook_iterate_cb, &g_intrman_lib);
#endif
        if (!g_loadcore_lib)
            return -1;
#if SIO2MAN_HOOK_SUPPORT_SET_INTR_HANDLER
        if (!g_intrman_lib)
            return -1;
#endif
        pRegisterLibraryEntries = ioplib_hookSameExportEntries(g_loadcore_lib, 6, hookRegisterLibraryEntries);
        ioplib_relinkExports(g_loadcore_lib);
#if SIO2MAN_HOOK_SUPPORT_SET_INTR_HANDLER
        pRegisterIntrHandler = ioplib_hookSameExportEntries(g_intrman_lib, 4, hookRegisterIntrHandler);
        ioplib_relinkExports(g_intrman_lib);
#endif
    }

    return 0;
}

void sio2man_hook_deinit()
{
    unsigned int i;

    M_DEBUG("%s\n", __FUNCTION__);

    // Unhook 'sio2man'
    for (i = 0; i < (sizeof(g_hook_data)/sizeof(g_hook_data[0])); i += 1) {
        if (g_hook_data[i].m_lib) {
#ifdef PORT_NR
            if (g_hook_data[i].m_25_psio2_transfer) {
                ioplib_hookSameExportEntries(g_hook_data[i].m_lib, 25, g_hook_data[i].m_25_psio2_transfer);
                g_hook_data[i].m_25_psio2_transfer = NULL;
            }
            ioplib_relinkExports(g_hook_data[i].m_lib);
#endif
            g_hook_data[i].m_lib = NULL;
        }
    }

    unhook_loadcore();
#if SIO2MAN_HOOK_SUPPORT_SET_INTR_HANDLER
    unhook_intrman();
#endif
}

void sio2man_hook_sio2_lock()
{
    // Lock sio2man driver so we can use it exclusively
    if (g_hook_data[1].m_23_psio2_pad_transfer_init)
        g_hook_data[1].m_23_psio2_pad_transfer_init();
}

void sio2man_hook_sio2_unlock()
{
    // Unlock sio2man driver
    if (g_hook_data[1].m_26_psio2_transfer_reset)
        g_hook_data[1].m_26_psio2_transfer_reset();
}

#if SIO2MAN_HOOK_SUPPORT_SET_INTR_HANDLER
void sio2man_hook_sio2_set_intr_handler(int (*handler)(void *userdata), void *userdata)
{
    intrman_internals_t *intrman_internals;
    int state;

    intrman_internals = GetIntrmanInternalData();
    CpuSuspendIntr(&state);
    intrman_internals->interrupt_handler_table[IOP_IRQ_SIO2].handler = handler ? handler : g_sio2man_intr_handler_ptr;
    intrman_internals->interrupt_handler_table[IOP_IRQ_SIO2].userdata = handler ? userdata : g_sio2man_intr_arg_ptr;
    CpuResumeIntr(state);    
}
#endif

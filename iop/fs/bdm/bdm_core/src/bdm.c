#include <bdm.h>
#include <stdio.h>
#include <thbase.h>
#include <thevent.h>

#include <bd_cache.h>

// #define DEBUG  //comment out this line when not debugging
#include "module_debug.h"

struct bdm_mounts
{
    struct block_device *bd; // real block device
    struct block_device *cbd; // cached block device
    struct file_system *fs;
};

#define MAX_CONNECTIONS 20
static struct bdm_mounts g_mount[MAX_CONNECTIONS];
static struct file_system *g_fs[MAX_CONNECTIONS];
static bdm_cb g_cb       = NULL;
static int bdm_event     = -1;
static int bdm_thread_id = -1;

/* Event flag bits */
#define BDM_EVENT_CB_MOUNT  0x01
#define BDM_EVENT_CB_UMOUNT 0x02
#define BDM_EVENT_MOUNT     0x04

void bdm_RegisterCallback(bdm_cb cb)
{
    int i;

    M_DEBUG("%s\n", __func__);

    g_cb = cb;

    if (g_cb == NULL)
        return;

    // Trigger a mount callback if we already have mounts
    for (i = 0; i < MAX_CONNECTIONS; ++i) {
        if ((g_mount[i].bd != NULL) && (g_mount[i].fs != NULL)) {
            SetEventFlag(bdm_event, BDM_EVENT_CB_MOUNT);
            break;
        }
    }
}

void bdm_connect_bd(struct block_device *bd)
{
    int i;

    M_PRINTF("connecting device %s%dp%d id=0x%x\n", bd->name, bd->devNr, bd->parNr, bd->parId);

    for (i = 0; i < MAX_CONNECTIONS; ++i) {
        if (g_mount[i].bd == NULL) {
            g_mount[i].bd = bd;
            // Create cache for entire device only (not for the partitions on it)
            g_mount[i].cbd = (bd->parNr == 0) ? bd_cache_create(bd) : NULL;
            // New block device, try to mount it to a filesystem
            SetEventFlag(bdm_event, BDM_EVENT_MOUNT);
            break;
        }
    }
}

void bdm_disconnect_bd(struct block_device *bd)
{
    int i;

    M_PRINTF("disconnecting device %s%dp%d id=0x%x\n", bd->name, bd->devNr, bd->parNr, bd->parId);

    for (i = 0; i < MAX_CONNECTIONS; ++i) {
        if (g_mount[i].bd == bd) {
            if (g_mount[i].fs != NULL) {
                // Unmount filesystem
                g_mount[i].fs->disconnect_bd(g_mount[i].cbd != NULL ? g_mount[i].cbd : g_mount[i].bd);
                M_PRINTF("%s%dp%d unmounted from %s\n", bd->name, bd->devNr, bd->parNr, g_mount[i].fs->name);
                g_mount[i].fs = NULL;
            }

            if (g_mount[i].cbd != NULL) {
                bd_cache_destroy(g_mount[i].cbd);
                g_mount[i].cbd = NULL;
            }

            g_mount[i].bd = NULL;

            if (g_cb != NULL)
                SetEventFlag(bdm_event, BDM_EVENT_CB_UMOUNT);
        }
    }
}

void bdm_connect_fs(struct file_system *fs)
{
    int i;

    M_PRINTF("connecting fs %s\n", fs->name);

    for (i = 0; i < MAX_CONNECTIONS; ++i) {
        if (g_fs[i] == NULL) {
            g_fs[i] = fs;
            break;
        }
    }

    // New filesystem, try to mount it to the block devices
    SetEventFlag(bdm_event, BDM_EVENT_MOUNT);
}

void bdm_disconnect_fs(struct file_system *fs)
{
    int i;

    M_PRINTF("disconnecting fs %s\n", fs->name);

    // Unmount fs from block devices
    for (i = 0; i < MAX_CONNECTIONS; ++i) {
        if (g_mount[i].fs == fs) {
            g_mount[i].fs = NULL;
            if (g_cb != NULL)
                SetEventFlag(bdm_event, BDM_EVENT_CB_UMOUNT);
        }
    }

    // Remove fs from list
    for (i = 0; i < MAX_CONNECTIONS; ++i) {
        if (g_fs[i] == fs) {
            g_fs[i] = NULL;
            break;
        }
    }
}

void bdm_get_bd(struct block_device **pbd, unsigned int count)
{
    int i;

    M_DEBUG("%s\n", __func__);

    // Fill pointer array with block device pointers
    for (i = 0; (unsigned int)i < count && i < MAX_CONNECTIONS; i++)
        pbd[i] = g_mount[i].bd;
}

static void bdm_try_mount(struct bdm_mounts *mount)
{
    int i;

    M_DEBUG("%s(%s%dp%d)\n", __func__, mount->bd->name, mount->bd->devNr, mount->bd->parNr);

    for (i = 0; i < MAX_CONNECTIONS; ++i) {
        if (g_fs[i] != NULL) {
            if (g_fs[i]->connect_bd(mount->cbd != NULL ? mount->cbd : mount->bd) == 0) {
                M_PRINTF("%s%dp%d mounted to %s\n", mount->bd->name, mount->bd->devNr, mount->bd->parNr, g_fs[i]->name);
                mount->fs = g_fs[i];
                if (g_cb != NULL)
                    SetEventFlag(bdm_event, BDM_EVENT_CB_MOUNT);
                break;
            }
        }
    }
}

static void bdm_thread(void *arg)
{
    u32 EFBits;
    int i;

    (void)arg;

    M_PRINTF("BDM event thread running\n");

    while (1) {
        WaitEventFlag(bdm_event, BDM_EVENT_CB_MOUNT | BDM_EVENT_CB_UMOUNT | BDM_EVENT_MOUNT, WEF_OR | WEF_CLEAR, &EFBits);

        if (EFBits & BDM_EVENT_MOUNT) {
            // Try to mount any unmounted block devices
            for (i = 0; i < MAX_CONNECTIONS; ++i) {
                if ((g_mount[i].bd != NULL) && (g_mount[i].fs == NULL))
                    bdm_try_mount(&g_mount[i]);
            }
        }

        if (EFBits & BDM_EVENT_CB_MOUNT) {
            // Notify callback about changes
            if (g_cb != NULL)
                g_cb(1);
        }

        if (EFBits & BDM_EVENT_CB_UMOUNT) {
            // Notify callback about changes
            if (g_cb != NULL)
                g_cb(0);
        }
    }
}

int bdm_init()
{
    int i, result;
    iop_event_t EventFlagData;
    iop_thread_t ThreadData;

    M_DEBUG("%s\n", __func__);

    for (i = 0; i < MAX_CONNECTIONS; ++i) {
        g_mount[i].bd  = NULL;
        g_mount[i].cbd = NULL;
        g_mount[i].fs  = NULL;
        g_fs[i]        = NULL;
    }

    EventFlagData.attr   = 0;
    EventFlagData.option = 0;
    EventFlagData.bits   = 0;
    result = bdm_event = CreateEventFlag(&EventFlagData);
    if (result < 0) {
        M_DEBUG("ERROR: CreateEventFlag %d\n", result);
        return result;
    }

    ThreadData.attr      = TH_C;
    ThreadData.thread    = bdm_thread;
    ThreadData.option    = 0;
    ThreadData.priority  = 0x30;   // Low priority
    ThreadData.stacksize = 0x1000; // 4KiB
    result = bdm_thread_id = CreateThread(&ThreadData);
    if (result < 0) {
        M_DEBUG("ERROR: CreateThread %d\n", result);
        DeleteEventFlag(bdm_event);
        return result;
    }

    result = StartThread(bdm_thread_id, NULL);
    if (result < 0) {
        M_DEBUG("ERROR: StartThread %d\n", result);
        DeleteThread(bdm_thread_id);
        DeleteEventFlag(bdm_event);
        return result;
    }

    return 0;
}

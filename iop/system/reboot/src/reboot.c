/*
# _____     ___ ____     ___ ____
#  ____|   |    ____|   |        | |____|
# |     ___|   |____ ___|    ____| |    \    PS2DEV Open Source Project.
#-----------------------------------------------------------------------
# Copyright 2001-2009, ps2dev - http://www.ps2dev.org
# Licenced under Academic Free License version 2.0
# Review ps2sdk README & LICENSE files for further details.
*/

#include "irx_imports.h"

#ifdef _IOP
IRX_ID("RebootByEE", 1, 1);
#endif
// Mostly based on the module from SCE SDK 1.3.4

extern void reboot_handler_thread(void *userdata);

int _start(int argc, char *argv[])
{
    int *BootMode;
    int v3;
    iop_thread_t v4;

    FlushDcache();
    BootMode = QueryBootMode(3);
    if (BootMode) {
        int v1;

        v1 = BootMode[1];
        if ((v1 & 1) != 0) {
            printf(" No SIF service(reboot)\n");
            return 1;
        }
        if ((v1 & 2) != 0) {
            printf(" No Reboot by EE service\n");
            return 1;
        }
    }
    CpuEnableIntr();
    v4.attr      = 0x2000000;
    v4.thread    = reboot_handler_thread;
    v4.priority  = 10;
    v4.stacksize = 2048;
    v4.option    = 0;
    v3           = CreateThread(&v4);
    if (v3 <= 0) {
        return 1;
    }
    StartThread(v3, 0);
    return 0;
}

typedef struct reboot_variables_
{
    int system_status_flag;
    int mode;
    char arg[80];
} reboot_variables_t;

reboot_variables_t reboot_variables;

typedef struct iop_reset_pkt_
{
    char not_used[16]; // SifCmdHeader_t contents would go here
    int arglen;
    int mode;
    char arg[80];
} iop_reset_pkt_t;

void reboot_sif_handler(iop_reset_pkt_t *data, reboot_variables_t *harg)
{
    int i;

    for (i = 0; i < data->arglen; i += 1) {
        harg->arg[i] = data->arg[i];
    }
    harg->mode = data->mode;
    iSetEventFlag(harg->system_status_flag, 0x400u);
}

void reboot_handler_thread(void *userdata)
{
    (void)userdata;

    if (!sceSifCheckInit()) {
        sceSifInit();
    }
    printf("Reboot service module.(99/11/10)\n");
    reboot_variables.system_status_flag = GetSystemStatusFlag();
    sceSifInitCmd();
    sceSifAddCmdHandler(0x80000003, (SifCmdHandler_t)reboot_sif_handler, &reboot_variables);
    WaitEventFlag(reboot_variables.system_status_flag, 0x400u, 0, 0);
    printf("Get Reboot Request From EE\n");
    sceSifSetMSFlag(0x20000u);
    *((vu32 *)0xBF801538) = 0;
    ReBootStart(reboot_variables.arg, reboot_variables.mode);
}

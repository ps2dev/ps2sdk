/*
# _____     ___ ____     ___ ____
#  ____|   |    ____|   |        | |____|
# |     ___|   |____ ___|    ____| |    \    PS2DEV Open Source Project.
#-----------------------------------------------------------------------
# Copyright 2001-2004, ps2dev - http://www.ps2dev.org
# Licenced under Academic Free License version 2.0
# Review ps2sdk README & LICENSE files for further details.
#
# ReMote control Manager RPC sample
*/

#include <stdio.h>
#include <kernel.h>
#include <sifrpc.h>
#include <string.h>
#include <iopcontrol.h>
#include <loadfile.h>
#include <sbv_patches.h>
#include <debug.h>
#include <librm.h>

extern unsigned char SIO2MAN_irx[];
extern unsigned int size_SIO2MAN_irx;

extern unsigned char RMMAN_irx[];
extern unsigned int size_RMMAN_irx;

static int VblankStartSema, VblankEndSema;

static s32 VblankStartHandler(s32 cause)
{
    iSignalSema(VblankStartSema);

    /* As per the SONY documentation, call ExitHandler() at the very end of
       your interrupt handler. */
    ExitHandler();
    return 0;
}

static s32 VblankEndHandler(s32 cause)
{
    iSignalSema(VblankEndSema);

    /* As per the SONY documentation, call ExitHandler() at the very end of
       your interrupt handler. */
    ExitHandler();
    return 0;
}

void loadmodules(int free)
{
    SifLoadFileInit();

    if (free == 0)
    {
        if (SifLoadModule("rom0:ADDDRV", 0, NULL) < 0)
        {
            scr_printf("Failed to load ADDDRV!\n");
            SleepThread();
        }

        if (SifLoadModule("rom1:SIO2MAN", 0, NULL) < 0)
        {
            scr_printf("Failed to load SIO2MAN!\n");
            SleepThread();
        }

        if (SifLoadModule("rom1:RMMAN", 0, NULL) < 0)
        {
            scr_printf("Failed to load RMMAN!\n");
            SleepThread();
        }
    }
    else
    {
        sbv_patch_enable_lmb();

        if (SifExecModuleBuffer(SIO2MAN_irx, size_SIO2MAN_irx, 0, NULL, NULL) < 0)
        {
            scr_printf("Failed to load SIO2MAN!\n");
            SleepThread();
        }
        if (SifExecModuleBuffer(RMMAN_irx, size_RMMAN_irx, 0, NULL, NULL) < 0)
        {
            scr_printf("Failed to load RMMAN!\n");
            SleepThread();
        }
    }

    SifLoadFileExit();
}

static const char *getRmStatus(u32 status)
{
    switch (status)
    {
        case RM_INIT:
            return "INITIALIZING";
        case RM_READY:
            return "READY";
        case RM_KEYPRESSED:
            return "PRESSED";
        case RM_NOREMOTE:
            return "DISCONNECTED";
        default:
            return "UNKNOWN";
    }
}

static const char *getRmButton(u32 button)
{
    switch (button)
    {
        case RM_DVD_ONE:
            return "RM_DVD_ONE";
        case RM_DVD_TWO:
            return "RM_DVD_TWO";
        case RM_DVD_THREE:
            return "RM_DVD_THREE";
        case RM_DVD_FOUR:
            return "RM_DVD_FOUR";
        case RM_DVD_FIVE:
            return "RM_DVD_FIVE";
        case RM_DVD_SIX:
            return "RM_DVD_SIX";
        case RM_DVD_SEVEN:
            return "RM_DVD_SEVEN";
        case RM_DVD_EIGHT:
            return "RM_DVD_EIGHT";
        case RM_DVD_NINE:
            return "RM_DVD_NINE";
        case RM_DVD_ZERO:
            return "RM_DVD_ZERO";
        case RM_DVD_ENTER:
            return "RM_DVD_ENTER";
        case RM_DVD_BROWSE:
            return "RM_DVD_BROWSE";
        case RM_DVD_SET:
            return "RM_DVD_SET";
        case RM_DVD_RETURN:
            return "RM_DVD_RETURN";
        case RM_DVD_CLEAR:
            return "RM_DVD_CLEAR";
        case RM_DVD_SOURCE:
            return "RM_DVD_SOURCE";
        case RM_DVD_CHUP:
            return "RM_DVD_CHUP";
        case RM_DVD_CHDOWN:
            return "RM_DVD_CHDOWN";
        case RM_DVD_REC:
            return "RM_DVD_REC";
        case RM_DVD_TITLE:
            return "RM_DVD_TITLE";
        case RM_DVD_MENU:
            return "RM_DVD_MENU";
        case RM_DVD_PROGRAM:
            return "RM_DVD_PROGRAM";
        case RM_DVD_TIME:
            return "RM_DVD_TIME";
        case RM_DVD_ATOB:
            return "RM_DVD_ATOB";
        case RM_DVD_REPEAT:
            return "RM_DVD_REPEAT";
        case RM_DVD_PREV:
            return "RM_DVD_PREV";
        case RM_DVD_NEXT:
            return "RM_DVD_NEXT";
        case RM_DVD_PLAY:
            return "RM_DVD_PLAY";
        case RM_DVD_SCAN_BACK:
            return "RM_DVD_SCAN_BACK";
        case RM_DVD_SCAN_FORW:
            return "RM_DVD_SCAN_FORW";
        case RM_DVD_SHUFFLE:
            return "RM_DVD_SHUFFLE";
        case RM_DVD_STOP:
            return "RM_DVD_STOP";
        case RM_DVD_PAUSE:
            return "RM_DVD_PAUSE";
        case RM_DVD_DISPLAY:
            return "RM_DVD_DISPLAY";
        case RM_DVD_SLOW_BACK:
            return "RM_DVD_SLOW_BACK";
        case RM_DVD_SLOW_FORW:
            return "RM_DVD_SLOW_FORW";
        case RM_DVD_SUBTITLE:
            return "RM_DVD_SUBTITLE";
        case RM_DVD_AUDIO:
            return "RM_DVD_AUDIO";
        case RM_DVD_ANGLE:
            return "RM_DVD_ANGLE";
        case RM_DVD_UP:
            return "RM_DVD_UP";
        case RM_DVD_DOWN:
            return "RM_DVD_DOWN";
        case RM_DVD_LEFT:
            return "RM_DVD_LEFT";
        case RM_DVD_RIGHT:
            return "RM_DVD_RIGHT";
        case RM_PS2_RESET:
            return "RM_PS2_RESET";
        case RM_PS2_EJECT:
            return "RM_PS2_EJECT";
        case RM_PS2_POWERON:
            return "RM_PS2_POWERON";
        case RM_PS2_POWEROFF:
            return "RM_PS2_POWEROFF";
        case RM_PS2_SELECT:
            return "RM_PS2_SELECT";
        case RM_PS2_L3:
            return "RM_PS2_L3";
        case RM_PS2_R3:
            return "RM_PS2_R3";
        case RM_PS2_START:
            return "RM_PS2_START";
        case RM_PS2_UP:
            return "RM_PS2_UP";
        case RM_PS2_RIGHT:
            return "RM_PS2_RIGHT";
        case RM_PS2_DOWN:
            return "RM_PS2_DOWN";
        case RM_PS2_LEFT:
            return "RM_PS2_LEFT";
        case RM_PS2_L2:
            return "RM_PS2_L2";
        case RM_PS2_R2:
            return "RM_PS2_R2";
        case RM_PS2_L1:
            return "RM_PS2_L1";
        case RM_PS2_R1:
            return "RM_PS2_R1";
        case RM_PS2_TRIANGLE:
            return "RM_PS2_TRIANGLE";
        case RM_PS2_CIRCLE:
            return "RM_PS2_CIRCLE";
        case RM_PS2_CROSS:
            return "RM_PS2_CROSS";
        case RM_PS2_SQUARE:
            return "RM_PS2_SQUARE";
        case RM_RELEASED:
            return "RM_RELEASED";
        case RM_IDLE:
            return "RM_IDLE";
        default:
            return "UNKNOWN";
    }
}

int main(int argc, char *argv[])
{
    /* Buffers for receiving input from remote controllers. A 256-byte region
       is required for each possible remote. Buffers must be each aligned
       to a 64-byte boundary due to how the EE data cache works. */
    static u8 rmData[256] __attribute__((aligned(64)));
    struct remote_data data, olddata;
    int startY, wrap;
    ee_sema_t sema;

    /* Initialize RPC services */
    SifInitRpc(0);
    SifIopReset(NULL, 0);
    while (!SifIopSync()) {};
    SifInitRpc(0);

    /* Initialize graphics library */
    init_scr();

    scr_printf("Welcome to the RMMAN sample!\n");
    scr_printf("For this demo, the remote should be plugged into port 2.\n");
    scr_printf("Loading modules...\n");

    /* Load modules */
    if ((argc == 2) && (strncmp(argv[1], "free", 4) == 0))
    {
        scr_printf(" - Using PS2SDK sio2man.irx, rmman.irx modules.\n");
        loadmodules(1);
    }
    else
    {
        scr_printf(" - Using ROM0 ADDRV, ROM1 SIO2MAN and ROM1 RMMAN modules.\n");
        scr_printf("Start this sample with 'free' as an argument to load\n");
        scr_printf("sio2man.irx and rmman.irx\n");
        scr_printf("Example: ps2client execee host:remote_sample.elf free\n");
        loadmodules(0);
    }

    scr_printf("Initializing...\n");

    /* Prepare semaphores, for detecting Vertical-Blanking events. */
    sema.count      = 0;
    sema.max_count  = 1;
    sema.attr       = 0;

    VblankStartSema = CreateSema(&sema);
    VblankEndSema   = CreateSema(&sema);

    /* Register VBlank start and end interrupt handlers. */
    AddIntcHandler(INTC_VBLANK_S, &VblankStartHandler, 0);
    AddIntcHandler(INTC_VBLANK_E, &VblankEndHandler, 0);

    /* Initialize the RMMAN RPC service */
    RMMan_Init();
    scr_printf("Module version: 0x%04x\n", RMMan_GetModuleVersion());

    scr_printf("Opening ports...");

    /* The remote can only be connected to slot 0 of any port (multitaps are not
       supported). For this demo, assume that the remote controller dongle
       is connected to controller port 2 (port = 1). */
    RMMan_Open(1, 0, rmData);

    scr_printf("done!\n");
    scr_printf("New input will be displayed here:\n");

    /* Enable interrupt handlers */
    _EnableIntc(INTC_VBLANK_S);
    _EnableIntc(INTC_VBLANK_E);

    /* In order to preserve the messages above,
       preserve the current Y coordinate. */
    startY = scr_getY();
    wrap   = 0;

    /* Erase old remote state */
    memset(&olddata, 0, sizeof(olddata));

    /* Enter the main loop */
    while (1)
    {
        /* Like with PADMAN, RMMAN only sends updates once every 1/60th (NTSC)
           or 1/50th (PAL) second. Hence, wait for a VBlank cycle
           (1/50th or 1/60th second). */
        WaitSema(VblankStartSema);
        WaitSema(VblankEndSema);

        /* Read data that RMMAN has sent. */
        RMMan_Read(1, 0, &data);

        /* If there was a difference, print it. */
        if ((olddata.status != data.status) || (olddata.button != data.button))
        {
            olddata = data;

            /* Do not draw past the end of the screen. If this is the last line,
               prepare to wrap around. */
            if (scr_getY() + 1 >= 27)
                wrap = 1;

            scr_printf("\t%08x (%s)\t%08x (%s)\n", data.status,
                       getRmStatus(data.status), data.button, getRmButton(data.button));

            /* From libdebug itself */
            if (wrap)
            {
                scr_setXY(0, startY);
                wrap = 0;
            }
        }
    }

    scr_printf("Shutting down...\n");

    /* Prepare for shutdown */

    /* Disable interrupts */
    _DisableIntc(INTC_VBLANK_S);
    _DisableIntc(INTC_VBLANK_S);

    /* Unregister interrupt handlers */

    /* Delete semaphores */
    DeleteSema(VblankStartSema);
    DeleteSema(VblankEndSema);

    /* Close opened ports */
    RMMan_Close(1, 0);

    /* Terminate RPC services */
    RMMan_End();
    SifExitRpc();

    return 0;
}

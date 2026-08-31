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
#include <libcdvd.h>

extern unsigned char SIO2MAN_irx[];
extern unsigned int size_SIO2MAN_irx;

extern unsigned char RMMAN_irx[];
extern unsigned int size_RMMAN_irx;

extern unsigned char RMMAN2_irx[];
extern unsigned int size_RMMAN2_irx;

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
        int minver_50000;
        sbv_patch_enable_lmb();
        {
            u8 out[16];
            u8 in[16];

            in[0] = 0x00;
            // sceCdMV
            sceCdApplySCmd(0x03, in, 1, out);
            minver_50000 = (out[0] & 0x80) ? 0 : ((out[3] | (out[2] << 8) | (out[1] << 16)) >= 0x50000);
        }
        if (minver_50000)
        {
            {
                u8 out[16];
                u8 in[16];

                in[0] = 0;
                // sceCdNoticeGameStart
                // Needed for front button readout for compatible systems
                sceCdApplySCmd(0x29, in, 1, out);
            }
            if (SifExecModuleBuffer(RMMAN2_irx, size_RMMAN2_irx, 0, NULL, NULL) < 0)
            {
                scr_printf("Failed to load RMMAN2!\n");
                SleepThread();
            }
        }
        else
        {
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

#define RMCASE(val) case val: return #val

static const char *getRmButton(u32 button)
{
    switch (button)
    {
        RMCASE(RM_DVD_ONE);
        RMCASE(RM_DVD_TWO);
        RMCASE(RM_DVD_THREE);
        RMCASE(RM_DVD_FOUR);
        RMCASE(RM_DVD_FIVE);
        RMCASE(RM_DVD_SIX);
        RMCASE(RM_DVD_SEVEN);
        RMCASE(RM_DVD_EIGHT);
        RMCASE(RM_DVD_NINE);
        RMCASE(RM_DVD_ZERO);
        RMCASE(RM_DVD_ENTER);
        RMCASE(RM_DVD_BROWSE);
        RMCASE(RM_DVD_SET);
        RMCASE(RM_DVD_RETURN);
        RMCASE(RM_DVD_CLEAR);
        RMCASE(RM_DVD_SOURCE);
        RMCASE(RM_DVD_CHUP);
        RMCASE(RM_DVD_CHDOWN);
        RMCASE(RM_DVD_REC);
        RMCASE(RM_DVD_TITLE);
        RMCASE(RM_DVD_MENU);
        RMCASE(RM_DVD_PROGRAM);
        RMCASE(RM_DVD_TIME);
        RMCASE(RM_DVD_ATOB);
        RMCASE(RM_DVD_REPEAT);
        RMCASE(RM_DVD_PREV);
        RMCASE(RM_DVD_NEXT);
        RMCASE(RM_DVD_PLAY);
        RMCASE(RM_DVD_SCAN_BACK);
        RMCASE(RM_DVD_SCAN_FORW);
        RMCASE(RM_DVD_SHUFFLE);
        RMCASE(RM_DVD_STOP);
        RMCASE(RM_DVD_PAUSE);
        RMCASE(RM_DVD_DISPLAY);
        RMCASE(RM_DVD_SLOW_BACK);
        RMCASE(RM_DVD_SLOW_FORW);
        RMCASE(RM_DVD_SUBTITLE);
        RMCASE(RM_DVD_AUDIO);
        RMCASE(RM_DVD_ANGLE);
        RMCASE(RM_DVD_UP);
        RMCASE(RM_DVD_DOWN);
        RMCASE(RM_DVD_LEFT);
        RMCASE(RM_DVD_RIGHT);
        RMCASE(RM_PS2_POWER);
        RMCASE(RM_PS2_EJECT);
        RMCASE(RM_PS2_POWERON);
        RMCASE(RM_PS2_POWEROFF);
        RMCASE(RM_PS2_RESET);
        RMCASE(RM_PS2_NOLIGHT);
        RMCASE(RM_PS2_SELECT);
        RMCASE(RM_PS2_L3);
        RMCASE(RM_PS2_R3);
        RMCASE(RM_PS2_START);
        RMCASE(RM_PS2_UP);
        RMCASE(RM_PS2_RIGHT);
        RMCASE(RM_PS2_DOWN);
        RMCASE(RM_PS2_LEFT);
        RMCASE(RM_PS2_L2);
        RMCASE(RM_PS2_R2);
        RMCASE(RM_PS2_L1);
        RMCASE(RM_PS2_R1);
        RMCASE(RM_PS2_TRIANGLE);
        RMCASE(RM_PS2_CIRCLE);
        RMCASE(RM_PS2_CROSS);
        RMCASE(RM_PS2_SQUARE);
        RMCASE(RM_DVD_OPEN_CLOSE);
        RMCASE(RM_DVD_POWER);
        RMCASE(RM_DVD_SEARCH_MODE);
        RMCASE(RM_DVD_SUBTITLE_ON_OFF);
        RMCASE(RM_DVD_STEP_BACK);
        RMCASE(RM_DVD_STEP_FORWARD);
        RMCASE(RM_DVD_SET_UP);
        RMCASE(RM_DESR_EJECT);
        RMCASE(RM_DESR_G_GUIDE);
        RMCASE(RM_DESR_QUIT_GAME);
        RMCASE(RM_DESR_POWER);
        RMCASE(RM_DESR_1);
        RMCASE(RM_DESR_2);
        RMCASE(RM_DESR_3);
        RMCASE(RM_DESR_4);
        RMCASE(RM_DESR_5);
        RMCASE(RM_DESR_6);
        RMCASE(RM_DESR_7);
        RMCASE(RM_DESR_8);
        RMCASE(RM_DESR_9);
        RMCASE(RM_DESR_10);
        RMCASE(RM_DESR_11);
        RMCASE(RM_DESR_12);
        RMCASE(RM_DESR_BS_7);
        RMCASE(RM_DESR_BS_11);
        RMCASE(RM_DESR_CLEAR);
        RMCASE(RM_DESR_TOP_MENU);
        RMCASE(RM_DESR_MENU);
        RMCASE(RM_DESR_RETURN);
        RMCASE(RM_DESR_TRIANGLE_OPTION);
        RMCASE(RM_DESR_CIRCLE);
        RMCASE(RM_DESR_SQUARE_VIEW);
        RMCASE(RM_DESR_CROSS_BACK);
        RMCASE(RM_DESR_UP);
        RMCASE(RM_DESR_LEFT);
        RMCASE(RM_DESR_RIGHT);
        RMCASE(RM_DESR_DOWN);
        RMCASE(RM_DESR_ENTER);
        RMCASE(RM_DESR_PROGRAM);
        RMCASE(RM_DESR_HOME);
        RMCASE(RM_DESR_DISPLAY);
        RMCASE(RM_DESR_L1_PREV);
        RMCASE(RM_DESR_L3);
        RMCASE(RM_DESR_R3);
        RMCASE(RM_DESR_R1_NEXT);
        RMCASE(RM_DESR_L2_SCAN_BACK);
        RMCASE(RM_DESR_SELECT);
        RMCASE(RM_DESR_START);
        RMCASE(RM_DESR_R2_SCAN_FORW);
        RMCASE(RM_DESR_PLAY);
        RMCASE(RM_DESR_PAUSE);
        RMCASE(RM_DESR_STOP);
        RMCASE(RM_DESR_RECORDING_MODE);
        RMCASE(RM_DESR_RECORD_START);
        RMCASE(RM_DESR_RECORD_PAUSE);
        RMCASE(RM_DESR_RECORD_STOP);
        RMCASE(RM_DESR_DELETE);
        RMCASE(RM_DESR_G_GUIDE2);
        RMCASE(RM_DESR_FLASH_BACK);
        RMCASE(RM_DESR_FLASH_FORW);
        RMCASE(RM_DESR_DVRP_UNK);
        RMCASE(RM_DESR_POWER_ON);
        RMCASE(RM_DESR_POWER_OFF);
        RMCASE(RM_DESR_NOLIGHT);
        RMCASE(RM_BD_EJECT);
        RMCASE(RM_BD_POWER);
        RMCASE(RM_BD_1);
        RMCASE(RM_BD_2);
        RMCASE(RM_BD_3);
        RMCASE(RM_BD_4);
        RMCASE(RM_BD_5);
        RMCASE(RM_BD_6);
        RMCASE(RM_BD_7);
        RMCASE(RM_BD_8);
        RMCASE(RM_BD_9);
        RMCASE(RM_BD_AUDIO);
        RMCASE(RM_BD_0);
        RMCASE(RM_BD_SUBTITLE);
        RMCASE(RM_BD_DISPLAY);
        RMCASE(RM_BD_YELLOW);
        RMCASE(RM_BD_BLUE);
        RMCASE(RM_BD_RED);
        RMCASE(RM_BD_GREEN);
        RMCASE(RM_BD_TOP_MENU);
        RMCASE(RM_BD_POP_UP_MENU);
        RMCASE(RM_BD_RETURN);
        RMCASE(RM_BD_OPTIONS);
        RMCASE(RM_BD_UP);
        RMCASE(RM_BD_RIGHT);
        RMCASE(RM_BD_DOWN);
        RMCASE(RM_BD_LEFT);
        RMCASE(RM_BD_ENTER);
        RMCASE(RM_BD_HOME);
        RMCASE(RM_BD_PREV);
        RMCASE(RM_BD_PAUSE);
        RMCASE(RM_BD_NEXT);
        RMCASE(RM_BD_RWD);
        RMCASE(RM_BD_PLAY);
        RMCASE(RM_BD_FF);
        RMCASE(RM_BD_NETFLIX);
        RMCASE(RM_BD_STOP);
        RMCASE(RM_BD_SEN);
        default:
            return "UNKNOWN";
    }
}

static const char *getRmFrontButton(u32 button)
{
    switch (button)
    {
        RMCASE(RM_DESR_FB_HOME);
        RMCASE(RM_DESR_FB_UP);
        RMCASE(RM_DESR_FB_DOWN);
        RMCASE(RM_DESR_FB_LEFT);
        RMCASE(RM_DESR_FB_RIGHT);
        RMCASE(RM_DESR_FB_ENTER);
        default:
            return "UNKNOWN";
    }
}

static int ensureRemoteFilter(void)
{
    int minver_50000;
    sbv_patch_enable_lmb();
    {
        u8 out[16];
        u8 in[16];

        in[0] = 0x00;
        // sceCdMV
        sceCdApplySCmd(0x03, in, 1, out);
        minver_50000 = (out[0] & 0x80) ? 0 : ((out[3] | (out[2] << 8) | (out[1] << 16)) >= 0x50000);
    }
    if (!minver_50000)
        return 0;
    {
        u8 out[16];
        u8 in[16];

        // Get the current 1-2-3 filter
        sceCdApplySCmd(0x2B, in, 0, out);
        switch (out[0])
        {
            case 1:
            case 2:
            case 3:
                return out[0];
            case 0x80:
                return 1;
            default:
                break;
        }
    }
    {
        u8 out[16];
        u8 in[16];

        in[0] = 1;
        // Set the 1-2-3 filter to 1 in case it was not set correctly
        sceCdApplySCmd(0x2A, in, 1, out);
    }
    return 1;
}

int main(int argc, char *argv[])
{
    /* Buffers for receiving input from remote controllers. A 256-byte region
       is required for each possible remote. Buffers must be each aligned
       to a 64-byte boundary due to how the EE data cache works. */
    static u8 rmData[256] __attribute__((aligned(64)));
    struct remote_data data, olddata;
    int startY, wrap;
    int rmset;
    ee_sema_t sema;

    /* Initialize RPC services */
    sceSifInitRpc(0);
    SifIopReset(NULL, 0);
    while (!SifIopSync()) {};
    sceSifInitRpc(0);

    /* Initialize graphics library */
    init_scr();

    scr_printf("Welcome to the RMMAN/RMMAN2 sample!\n");
    scr_printf("Loading modules...\n");

    /* Load modules */
    if ((argc >= 2) && !strcmp(argv[1], "rom"))
    {
        scr_printf(" - Using ROM0 ADDRV, ROM1 SIO2MAN and ROM1 RMMAN modules.\n");
        loadmodules(0);
    }
    else
    {
        scr_printf(" - Using PS2SDK modules.\n");
        scr_printf("Start this sample with 'rom' as an argument to load\n");
        scr_printf("RMMAN ROM modules\n");
        scr_printf("Example: ps2client execee host:remote_sample.elf rom\n");
        loadmodules(1);
    }

    rmset = ensureRemoteFilter();
    if (!rmset)
    {
        rmset = 1;
        scr_printf("For this demo, the IR receiver should be plugged into port 2.\n");
    }
    scr_printf("Ensure the switch on the remote, if present, is set to the %d position.\n", rmset);

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
        if (memcmp(&olddata, &data, sizeof(data)))
        {
            olddata = data;

            /* Do not draw past the end of the screen. If this is the last line,
               prepare to wrap around. */
            if (scr_getY() + 1 >= 27)
                wrap = 1;

            scr_printf("\t%08x (%s)\t%08x (%s)\t%08x (%s)\n", data.status,
                       getRmStatus(data.status), data.button, getRmButton(data.button),
                       data.front_button, getRmFrontButton(data.front_button));

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
    sceSifExitRpc();

    return 0;
}

#include <xparam.h>
#include <iop_regs.h>
#include <fcntl.h>
#include <loadfile.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>

char params_DCACHE_OFF[] = {'0', 'x', '1', '0', 0, '0', 0};
char params_CPU_DELAY[] = {'0', 'x', '6', 0, '0', 'x', '7', '8', '0', 0};


static int CheckSpecialDiscXParamTitle(const char *title)
{
    /*
    See if we are dealing with any of the special titles so far.
    Special titles are the ones that have the param specified in the SYSTEM.CNF.
    To get that value one needs to get XPARAM entry from system.cnf and then verify its integrity.
    Since all special games are known already and to avoid the whole MD5 checking and everything let's just go the easy way and apply it.
    Param from the disc always overrides the internal ones so let's do it.
    PS2 special params from the disc do not apply on PS3/PS4. They instead look for XPARAM4 entry (no game ever found with that one).
    */

    int result = 0;

    // Another Century's Episode 2 (Japan)
    result |= !strncmp("SLPS_256.23", title, 11);

    // Critical Velocity (Japan)
    result |= !strncmp("SLPS_255.32", title, 11);

    // Hissatsu Pachinko Station V11 (Japan)
    // Hissatsu Pachinko Station V11 - CR Gyaatoruzu (Japan)
    result |= !strncmp("SLPS_255.56", title, 11);

    // Matantei Loki Ragnarok - Mayouga - Ushinawareta Bishou (Japan)
    // NOTE: this game has same XPARAM values and hash as the game Ibara.
    // The md5 check for this does match but for Ibara does not. For sure that Ibara is just a user error due to it being leftover from this game. Both games are from Taito and Ibara came much later.
    result |= !strncmp("SLPM_661.41", title, 11);

    // Sega Ages 2500 Series Vol. 23 - Sega Memorial Selection (Japan)
    result |= !strncmp("SLPM_627.09", title, 11);

    // Shin Bakusou Dekotora Densetsu - Tenka Touitsu Choujou Kessen (Japan) (Spike the Best)
    result |= !strncmp("SLPM_663.87", title, 11);

    return result;
}


static void ApplyExtraXParamTitle(const char *title, char *params)
{
    int result = 0;

    // Kaidou: Touge no Densetsu (Japan)
    // This config was added twice in newer XPARAM.
    result |= !strncmp("SLPM_660.22", title, 11);

    // Duel Masters: Birth of Super Dragon (Japan)
    result |= !strncmp("SLPM_658.82", title, 11);

    // Shin Bakusou Dekotora Densetsu: Tenka Touitsu Choujou Kessen (Japan)
    result |= !strncmp("SLPM_658.16", title, 11);

    // Shutokou Battle 01 (Japan)
    result |= !strncmp("SLPM_653.08", title, 11);

    // Initial D: Special Stage (Japan)
    result |= !strncmp("SLPM_652.68", title, 11);

    // Bakusou Dekotora Densetsu: Otoko Hanamichi Yume Roman (Japan)
    result |= !strncmp("SLPM_652.34", title, 11);

    if (result) {
        // All of them are new added ones and all use 0x10 0 (DCACHE OFF )
        memcpy(&params[12], params_DCACHE_OFF, 7);
        SifLoadModule("rom0:XPARAM", 19, params);
        return;
    }


    result = 0;

    // This new one was for entries already there that had DCACHE off but now even have a CPU_DELAY param added to it.

    // Tekken 5
    result |= !strncmp("SCAJ_201.25", title, 11);
    result |= !strncmp("SCAJ_201.26", title, 11);
    result |= !strncmp("SCKA_200.49", title, 11);
    result |= !strncmp("SLPS_255.10", title, 11);
    result |= !strncmp("SLUS_210.59", title, 11);

    if (result) {
        memcpy(&params[12], params_CPU_DELAY, 10);
        SifLoadModule("rom0:XPARAM", 22, params);
        return;
    }

    // One single game was then added both cache off and CPU delay.
    if (!strncmp("SCES_532.02", title, 11)) {

        /*
        Tekken 5 (Australia)
        Tekken 5 would have the cache off but not the CPU delay, however, this regional release for the first bios of 750xx had neither the cache nor the delay making it run worse.
        */
        memcpy(&params[12], params_DCACHE_OFF, 7);
        SifLoadModule("rom0:XPARAM", 19, params);

        memcpy(&params[12], params_CPU_DELAY, 10);
        SifLoadModule("rom0:XPARAM", 22, params);
    }
}

// Note TITLE must be as *IS*, not uppercase or anything.
int ApplyDeckardXParam(const char *title)
{
    int fd;

    // Safety check
    if (title == NULL)
        return -EINVAL;

    char params[30];
    memset(params, 0, 30);
    strncpy(params, title, 11);
    params[11] = 0; // Terminate param string.

    if (IOP_CPU_TYPE == IOP_TYPE_MIPSR3000)
        return -ENODECKARD;

    /*
    See if this PS3/4 emu and apply the config to it.
    PS3 and PS4 are missing the regular param file and only have XPARAM2 in them.
    */
    fd = open("rom0:XPARAM2", O_RDONLY);
    if (fd >= 0) {
        close(fd);
        SifLoadModule("rom0:XPARAM2", 12, params);
        return APPLIED_XPARAM_EMU;
    }

    // See if it's the regular PS2 one.
    fd = open("rom0:XPARAM", O_RDONLY);
    if (fd >= 0) {
        close(fd);

        if (CheckSpecialDiscXParamTitle(title)) {
            // All of them use 0x10 0 (DCACHE OFF)
            // Params are sent as direct string null separated.
            memcpy(&params[12], params_DCACHE_OFF, 7);
            SifLoadModule("rom0:XPARAM", 19, params);
            return APPLIED_XPARAM_SPECIAL_TITLE;
        }

        // Load the default configs found in the XPARAM module.
        SifLoadModule("rom0:XPARAM", 12, params);

        /*
        There is a special case here, in all the PS2 bios there are only two XPARAM files.
        The first one is in bios 2.20 with the date 20050620 and has 272 configs in total.
        Starting with 2.20 date 20060210 and all later bios up to the final one there is one single XPARAM which has 286 configs.
        14 configs were added and the previous ones were unchanged.
        Within those 14 configs, some are for new games, the rest is more params for games that already had one.
        If we are running into this early bios we will get bad compatibility so let's make it uniform and reapply the extra configs just in case because they cause no harm.
        */

        ApplyExtraXParamTitle(title, params);

        return APPLIED_XPARAM;
    }
    return -ENOXPARAMF;
}

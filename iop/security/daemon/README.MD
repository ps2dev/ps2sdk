# Daemon (aka: sec_checker)

This module, found on the bootrom of the Arcade PS2s (COH-H models and derivative units) is in charge of checking the security dongle status.

Arcade SYSCON has a 3 to 5 minutes countdown (time may depend on the model) to reset the console.

Arcade Mechacon resets this countdown when the secrAuthDongle routine is successfully completed.

This module setups an IOP thread that calls mcdetectcard2 (DONGLEMAN EXPORT:21) once every minute. effectively keeping the watchdog at bay, but also making access to `mc0:/` rather unstable.

This module is placed here just for hystorical purposes and preservation. there is (as of today) no reason to use a homebrew implementation in favor of `rom0:DAEMON`

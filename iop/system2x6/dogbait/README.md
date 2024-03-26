# Dogbait

This module provides a thread to trick the namco arcade system 246/256 watchdog, to avoit it from shutting down the machine if security dongle is not connected for some time.

its implementation could be considered a fusion of `rom0:LED` and `rom0:DAEMON` (without dongle spamming)

## How to use this module in your program

Use `SifLoadStartModule` or `LoadModuleBuffer` directly.

due to the purpose of this module, checking if the module loaded successfully and stayed resident on IOP is important

like this:
```c
int ret, id;
id = LoadModuleBuffer(dogbait_irx, size_dogbait_irx, 0, NULL, &ret);
if (id<0 || ret == 1) we_have_an_error();
```

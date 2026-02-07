# Dogbait

This module provides a thread to trick the namco arcade system 246/256 watchdog, to avoit it from shutting down the machine if security dongle is not connected for some time.

it's main purpose is to prevent the watchdog from shutting down the system. while keeping a safe access to security dongle

## How to use this module in your program

Use `SifLoadStartModule` or `LoadModuleBuffer` directly.

due to the purpose of this module, checking if the module loaded successfully and stayed resident on IOP is important

like this:
```c
int ret, id;
id = LoadModuleBuffer(dogbait_irx, size_dogbait_irx, 0, NULL, &ret);
if (id<0 || ret == 1) we_have_an_error();
```

If the module fails to load, please let know the user that the machine will shutdown in a few minutes.

Of course, if your app does not involve access to any memory card/security dongle, then loading the following modules in order and keeping a security dongle plugged will also satisfy the watchdog
```
rom0:SIO2MAN
rom0:MCMAN
rom0:LED
rom0:DAEMON
```
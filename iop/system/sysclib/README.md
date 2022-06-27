# System C Library

IOP System C library.\
This library is an open source implementation of the feature set of `SYSCLIB`
in SCE SDK 3.1.0, compared to the feature set of `SYSCLIB` in ROM, which is
based on SCE SDK 1.3.4.

## Pulled sources

This library uses code from open source libraries, so their licenses apply.

embeddedartistry/libc bfa3d49: https://github.com/embeddedartistry/libc\
musl 1.2.2: https://musl.libc.org/\
eyalroz/printf 19a487c: https://github.com/eyalroz/printf\
newlib 4.1.0: https://sourceware.org/newlib/

## Configurations

There are multiple configurations of this library, allowing the choice of
balancing between size, speed, and features.

*   `sysclib` -> The recommended version.\
    Has the accelerated assembly versions of functions.
*   `sysclib-nano` -> For memory savings.\
    Smaller than the original SCE version.
*   `sysclib-full` -> Based on sysclib, but its format series of functions\
    supports `long long` and floating point. Around double the size.

## How to use this module in your program

In order to use this module in your program, you must integrate the module into
an IOPRP image, then load the image with `UDNL`.\
Using `LoadModule` or `LoadModuleBuffer` directly will not work, because the
module is already loaded at IOP boot.

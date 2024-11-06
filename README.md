# PS2SDK - PS2DEV Open Source Project

[![CI](https://github.com/ps2dev/ps2sdk/workflows/CI/badge.svg)](https://github.com/ps2dev/ps2sdk/actions?query=workflow%3ACI)
[![CI-Docker](https://github.com/ps2dev/ps2sdk/workflows/CI-Docker/badge.svg)](https://github.com/ps2dev/ps2sdk/actions?query=workflow%3ACI-Docker)

[PS2SDK Documentation](https://ps2dev.github.io/ps2sdk/)

Copyright 2001-2004, ps2dev - <http://www.ps2dev.org>
All rights reserved.

## **ATTENTION**

If you are confused on how to start developing for PS2, see the [getting started](https://ps2dev.github.io/#getting-started) section on the ps2dev main page.  

## Introduction

`PS2SDK` is a collection of Open Source libraries used for developing applications on Sony's PlayStation 2® (PS2). `PS2SDK` contains the work from a number of PS2 projects, which are now buildable in a single source tree. Review the history section for how ps2sdk came about.

At the time of writing, PS2SDK includes the following libraries and features, allowing:

-   Access to PS2 internal OS functions.
-   Access to PS2 control pad and multitap.
-   Access to PS2 memory card.
-   Access to USB mouse and keyboard.
-   TCP/IP stack & DNS resolution compatible with PS2 Ethernet Adapter.
-   Full PS2 compatible Hard Disk Drive file system.
-   Access to CD and DVD.
-   Required implementation by `newlib`, for fulfilling `libc` functionality.
-   Access to sound library on all PS2 using `freesd`.
-   HTTP client file system.
-   Network File System to load files from HOST pc.

## Standard Libraries

In a continuous effort to make `PS2SDK` and the whole `PS2DEV` a `POSIX` environment; there are some libraries needed to be included inside of the [standard libraries](https://gcc.gnu.org/onlinedocs/gcc/Standard-Libraries.html), which means, libraries than by default are included in any compilation program. From `PS2SDK` we have:

-   `lkernel`: Contains the calls to BIOS functions
-   `lcglue`: Implements most of the required methods by `newlib`
-   `lcdvd`: Implements `ps2time` function indirectly required by `newlib`

If you wanna compile a program without using the `standard libraries` take a look at the [compiler flags](https://gcc.gnu.org/onlinedocs/gcc/Link-Options.html) (`-nodefaultlibs`, `-nolibc` and `-nostdlib`).

## License

PS2SDK has been developed by a large number of individuals who have provided their time and effort. The `AUTHORS` file includes this list.

PS2SDK is licensed under the Academic Free License version 2.0. This is a very liberal license and allows both commercial and non-commercial usage of the SDK. Please read the `LICENSE` file for full details.

## Binary Installation

ps2sdk provides a large number of the basic software libraries required to access the underlying PS2 system. As the PS2 has two independent CPUs - the Emotion Engine (EE) and the IO Processor (IOP), the source tree is split into two different major areas representing the functions available on each processor.

A binary release of PS2SDK will include the following directories:

-   `sdk/ee/include`: EE include files.
-   `sdk/ee/lib`: EE library files.
-   `sdk/ee/startup`: Example crt0.o and linkfile.
-   `sdk/iop/include`: IOP include files.
-   `sdk/iop/irx`: IOP loadable modules.
-   `sdk/common/include`: Common include files between EE and IOP.
-   `sdk/samples`: Samples for both EE and IOP.
-   `sdk/tools`: Tools used during development on host PC.

## Source Installation

ps2sdk source tree is considerably different from the binary or release distribution. You should only use the binary release when using ps2sdk in your projects.

The source tree is built as a collection of separate projects; each with its Makefile. The file `Defs.make` provides the basic definitions required when building PS2SDK. The two main variables required are `PS2SDKSRC`, which points to the source base directory, and `PS2SDK`, which points to the release directory.

The main make file has three targets:

-   `all/default`: compile each of the projects in the tree.
-   `clean`: clean the tree of files created during the build.
-   `release`: release the binaries to the target PS2SDK directory.

Each subproject has a tree structure which can include:

-   `src`: source code
-   `include`: include files that are exported
-   `samples`: samples of using a project. Can include multiple directories
-   `doc`: documentation files to be exported.
-   `test`: Unit Testing or other testing code.
-   `obj`: created during the build to store object files
-   `lib`: created during the build to store library files
-   `bin`: created during the build to store binary files

Please review the Makefiles to see how to create your subproject in the tree.

## History

ps2sdk brings together several open-source projects developed for the Playstation 2®. These projects include ps2lib, ps2drv, libhdd, ps2ip and ps2hid. These projects are now all closed and have been migrated to ps2sdk.

ps2lib was the first library to be released. Created by Gustavo Scotti, the library was released in October 2001. Over the years a number of people have contributed to providing the base functionality required to access the internals of the PS2. ps2lib has gone through a number of versions and was last released as Version 2.1 in October 2003.

ps2drv was started by [Marcus R. Brown](https://github.com/marcusrbrown) to provide an area to look at more of the internals of the IO Processor and related hardware. It was started in June 2003 and over the last year (2004) has grown considerably. ps2drv is where the `irx` imports method was created used in ps2sdk. ps2drv was last released as Version 1.1 in February 2004.

ps2ip was started by [David Ryan (Oobles)](https://github.com/oobles) in late 2002 to provide a TCP/IP stack for open source development. Over the last two years (2003-2004), the stack has improved and matured. boman666 provided the last big improvement to the design and his changes are used in PS2SDK.

libhdd was started by Nick Van Veen (Sjeep) in 2003 to provide a Hard Disk Drive driver and file system that is compatible with the commercial Sony HDD and other non-Sony HDDs. The work was sponsored by DMS3, and the resulting code was kindly provided back to the ps2dev community. The last release before ps2sdk was version 1.2 released in February 2004.

ps2hid was started by [Tyranid](https://github.com/tyranid) in October 2003 to provide USB mouse and keyboard drivers compatible with the Sony USB Driver.

[AUTHORS](AUTHORS2004.md)

## Community

Links for discussion and chat are available [here](https://ps2dev.github.io/#community).  

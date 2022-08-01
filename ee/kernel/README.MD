# PS2SDK - KERNEL LIBRARY

## SUMMARY

This folder creates the `libkernel.a` library, which contains all the references to the BIOS functionality embedded in the hardware of every single PS2.

Probably you've heard, that the `PS2 BIOS` is full of bugs, which totally accurate, however, these bugs aren't blocker you still could execute some program (with limited functionality) without any problem.

## SOLVING BIOS ISSUES

In order to fix all these issues, we're patching the problematic functions by putting some wrappers in the middle before calling the actual BIOS function.

Some of these wrappers are set during the initialization process of the application `_InitSys` function.
Additionally, we also use `_InitSys` to initialize some other functionality that the user may need.


## CONSEQUENCES

All these functions are set during the initialization process of every single binary, this implies unfortunately having a bigger size binary.

Some apps require to have really tiny sizes, and also you may want to don't patch these BIOS functions, therefore we are offering the possibility to disable the functions called during the `_InitSys` function.

Let's first take a look at the current `InitSys` method:

```c
void _InitSys(void)
{
    StartTimerSystemTime();
    InitTimerAlarm();
    InitAlarm();
    InitThread();
    InitExecPS2();
    InitTLBFunctions();
}
```

The nature of the functions called can be split into 2 different sections:

1. Patched functionality
```c
    InitAlarm();
    InitThread();
    InitExecPS2();
    InitTLBFunctions();
```

2. Additional timer utils
```
    StartTimerSystemTime();
    InitTimerAlarm();
```

## NO PATCHED BINARIES AND/OR TINY BINARIES

On the `kernel.h` file at the very end, we have a section where we defined some `macros` for making it super easy the disable these functions.

Let's put an example of how to disable the patched functionality:

```c
.....
#include <kernel.h>
#include <stdio.h>

// With this line we disabled the patched functionalities
DISABLE_PATCHED_FUNCTIONS();

int main(int argc, char *argv[])
{
    printf("Hello world!\n");
    return 0;
}
```

Let's put another example where we disable the extra timer functionality:

```c
.....
#include <kernel.h>
#include <stdio.h>

// With this line we disabled the extra functionalities for timers
DISABLE_EXTRA_TIMERS_FUNCTIONS();

int main(int argc, char *argv[])
{
    printf("Hello world!\n");
    return 0;
}
```

Therefore we can combine both:


```c
.....
#include <kernel.h>
#include <stdio.h>

// With this line we disabled the patched functionalities
DISABLE_PATCHED_FUNCTIONS();
// With this line we disabled the extra functionalities for timers
DISABLE_EXTRA_TIMERS_FUNCTIONS();

int main(int argc, char *argv[])
{
    printf("Hello world!\n");
    return 0;
}
```

## HOW IS THIS WORKING?

It honestly is quite easy. 
All these functions that we may want to disable are defined as `__attribute__((weak))`, so it means that if a "strong" implementation is defined, they will override.
For this reason, the macros defined in the `kernel.h` are helpers that either does dummy implementations or bypass the patched wrappers.
/*
# _____     ___ ____     ___ ____
#  ____|   |    ____|   |        | |____|
# |     ___|   |____ ___|    ____| |    \    PS2DEV Open Source Project.
#-----------------------------------------------------------------------
# Copyright 2001-2005, ps2dev - http://www.ps2dev.org
# Licenced under Academic Free License version 2.0
# Review ps2sdk README & LICENSE files for further details.
#
# Malloc tester
*/

#include <stdio.h>
#include <dirent.h>
#include <unistd.h>
#include <kernel.h>

#if defined(SCREEN_DEBUG)
#include <debug.h>
#endif

#if defined(SCREEN_DEBUG)
#define custom_printf(args...) \
    printf(args);              \
    scr_printf(args);
#else
#define custom_printf(args...) printf(args);
#endif

int main(int argc, char *argv[])
{
    // Buffer to store the current working directory
    char cwd[1024];

#if defined(SCREEN_DEBUG)
    init_scr();
    sleep(3);
#endif
    custom_printf("\n\nStarting rewinddir TESTS...\n");

    // Get the current working directory
    if (getcwd(cwd, sizeof(cwd)) == NULL) {
        custom_printf("Error getting current working directory");
        return 1;
    }

    // Open the current directory
    DIR *directory = opendir(cwd);

    // Check if the directory can be opened
    if (directory == NULL) {
        custom_printf("Error opening directory");
        return 1;
    }

    // Read directory entries
    struct dirent *entry;
    while ((entry = readdir(directory)) != NULL) {
        // Check if the entry is a directory using d_type
        custom_printf("%s -> %s\n", entry->d_name, entry->d_type == DT_DIR ? "Directory" : "File");
    }

    custom_printf("\n\nExecuting rewinddir function...\n");
    rewinddir(directory);

    while ((entry = readdir(directory)) != NULL) {
        // Check if the entry is a directory using d_type
        custom_printf("%s -> %s\n", entry->d_name, entry->d_type == DT_DIR ? "Directory" : "File");
    }

    custom_printf("\n\nTest finished!\n");

    // Close the directory
    closedir(directory);

    SleepThread();

    return 0;
}

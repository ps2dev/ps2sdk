/*
 * Verify the EE crt0/newlib exit path on real hardware with ps2link.
 *
 * Run with no arguments (return from main) and with "exit" (explicit exit).
 * The corresponding host: file must contain all five lines, including the
 * buffered atexit line, after the ELF stops:
 *   init=1
 *   main
 *   atexit
 *   init-atexit
 *   fini
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static FILE *trace;
static int init_called;

static void init_exit(void)
{
    fputs("init-atexit\n", trace);
}

/* Test the startup's explicit _init/_fini hooks without depending on the
 * toolchain's optional crtbegin/crtend objects. */
void _init(void)
{
    init_called = atexit(init_exit) == 0;
}

void _fini(void)
{
    fputs("fini\n", trace);
}

static void record_exit(void)
{
    fputs("atexit\n", trace);
    printf("exit_lifecycle: atexit invoked\n");
}

int main(int argc, char **argv)
{
    int explicit_exit = (argc > 1 && strcmp(argv[1], "exit") == 0);
    const char *path = explicit_exit ? "host:exit_lifecycle_explicit.txt" :
                                       "host:exit_lifecycle_return.txt";

    trace = fopen(path, "w");
    if (!trace) {
        printf("exit_lifecycle: unable to open %s\n", path);
        return 1;
    }

    if (setvbuf(trace, NULL, _IOFBF, BUFSIZ) != 0 || atexit(record_exit) != 0) {
        fclose(trace);
        printf("exit_lifecycle: failed to configure buffered exit tracing\n");
        return 1;
    }

    fprintf(trace, "init=%d\n", init_called);
    fputs("main\n", trace);
    printf("exit_lifecycle: %s, init=%d\n",
           explicit_exit ? "exit()" : "return", init_called);

    if (explicit_exit)
        exit(0);
    return 0;
}

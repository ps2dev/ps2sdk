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
 * With "immediate", only the explicitly flushed init and main lines must
 * be present; no exit handler or destructor may run.
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

static FILE *trace;
static int init_called;

static void init_exit(void)
{
    if (trace)
        fputs("init-atexit\n", trace);
}

/* Exercise the ordinary constructor/destructor path through crtbegin/crtend. */
__attribute__((constructor))
static void init(void)
{
    init_called = atexit(init_exit) == 0;
}

__attribute__((destructor))
static void fini(void)
{
    if (trace)
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
    int immediate_exit = (argc > 1 && strcmp(argv[1], "immediate") == 0);
    const char *path = immediate_exit ? "host:exit_lifecycle_immediate.txt" :
                       explicit_exit ? "host:exit_lifecycle_explicit.txt" :
                                       "host:exit_lifecycle_return.txt";

    trace = fopen(path, "w");
    if (!trace) {
        printf("exit_lifecycle: unable to open %s\n", path);
        return 1;
    }

    if (setvbuf(trace, NULL, _IOFBF, BUFSIZ) != 0 || atexit(record_exit) != 0) {
        fclose(trace);
        trace = NULL;
        printf("exit_lifecycle: failed to configure buffered exit tracing\n");
        return 1;
    }

    fprintf(trace, "init=%d\n", init_called);
    fputs("main\n", trace);
    printf("exit_lifecycle: %s, init=%d\n",
           immediate_exit ? "_exit()" : explicit_exit ? "exit()" : "return", init_called);

    if (immediate_exit) {
        fflush(trace);
        fflush(stdout);
        _exit(0);
    }

    if (explicit_exit)
        exit(0);
    return 0;
}

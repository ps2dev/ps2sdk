#include <windows.h>
#include <process.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Helper to wrap arguments with spaces/quotes for Windows _spawnvp
static char *quote_arg(const char *arg) {
    size_t len = strlen(arg);
    int needs_quotes = (len == 0);
    for (size_t i = 0; i < len; i++) {
        if (arg[i] == ' ' || arg[i] == '\t' || arg[i] == '"') {
            needs_quotes = 1;
            break;
        }
    }
    if (!needs_quotes) {
        return _strdup(arg);
    }

    char *buf = malloc(len * 2 + 3);
    if (!buf) return NULL;

    char *p = buf;
    *p++ = '"';
    for (size_t i = 0; i < len; i++) {
        if (arg[i] == '"') {
            *p++ = '\\';
        }
        *p++ = arg[i];
    }
    *p++ = '"';
    *p = '\0';
    return buf;
}

int main(int argc, char *argv[]) {
    // 1. Read PS2SDK and PS2DEV from the environment
    const char *ps2sdk = getenv("PS2SDK");
    const char *ps2dev = getenv("PS2DEV");

    if (!ps2sdk || !*ps2sdk) {
        fprintf(stderr, "Error: PS2SDK environment variable is not set.\n");
        return 1;
    }
    if (!ps2dev || !*ps2dev) {
        fprintf(stderr, "Error: PS2DEV environment variable is not set.\n");
        return 1;
    }

    // 2. Clear host pkg-config variables
    _putenv("PKG_CONFIG_DIR=");
    _putenv("PKG_CONFIG_PATH=");
    _putenv("PKG_CONFIG_SYSROOT_DIR=");

    // 3. Build PKG_CONFIG_LIBDIR using Windows semicolon (;) path separators
    // Note: Native UCRT64 pkg-config.exe expects ';' instead of ':'
    char libdir_env[4096];
    snprintf(libdir_env, sizeof(libdir_env),
             "PKG_CONFIG_LIBDIR=%s/ports/lib/pkgconfig;%s/gsKit/lib/pkgconfig;%s/ee/lib/pkgconfig",
             ps2sdk, ps2dev, ps2sdk);
    _putenv(libdir_env);

    // 4. Build argument list for pkg-config.exe
    int is_version = (argc > 1 && strcmp(argv[1], "--version") == 0);
    int extra_args = is_version ? 0 : 1; // Add "--static" unless "--version"

    char **args = malloc((argc + extra_args + 1) * sizeof(char *));
    if (!args) return 1;

    int idx = 0;
    args[idx++] = _strdup("pkg-config.exe");
    if (!is_version) {
        args[idx++] = _strdup("--static");
    }
    for (int i = 1; i < argc; i++) {
        args[idx++] = quote_arg(argv[i]);
    }
    args[idx] = NULL;

    // 5. Execute pkg-config.exe from PATH (e.g., C:\msys64\ucrt64\bin\pkg-config.exe)
    int status = _spawnvp(_P_WAIT, "pkg-config.exe", (const char * const *)args);

    if (status == -1) {
        fprintf(stderr, "pkg-config is not found\n");
        status = 1;
    }

    for (int i = 0; i < idx; i++) {
        free(args[i]);
    }
    free(args);

    return status;
}

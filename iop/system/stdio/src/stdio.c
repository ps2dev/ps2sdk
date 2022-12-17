/*
# _____     ___ ____     ___ ____
#  ____|   |    ____|   |        | |____|
# |     ___|   |____ ___|    ____| |    \    PS2DEV Open Source Project.
#-----------------------------------------------------------------------
# Copyright 2001-2009, ps2dev - http://www.ps2dev.org
# Licenced under Academic Free License version 2.0
# Review ps2sdk README & LICENSE files for further details.
*/

#include "irx_imports.h"

extern struct irx_export_table _exp_stdio;

typedef struct stdio_prnt_context_
{
    s16 fd;
    s16 current_length;
    char tmp_buf[64];
} stdio_prnt_context_t;

#ifdef _IOP
IRX_ID("Stdio", 1, 1);
#endif
// Mostly based on the module from SCE SDK 1.3.4 with an addition from SDK 3.1.0.

int _start(int argc, char *argv[])
{
    if (RegisterLibraryEntries(&_exp_stdio) != 0) {
        return MODULE_NO_RESIDENT_END;
    }
    return MODULE_RESIDENT_END;
}

void stdio_prnt_callback(void *userdata, int c)
{
    size_t current_length;
    char *tmp_buf;
    stdio_prnt_context_t *context = (stdio_prnt_context_t *)userdata;

    current_length = context->current_length;
    tmp_buf        = context->tmp_buf;

    switch (c) {
        case 512:
            context->current_length = 0;
            return;
        case 513:
            if (current_length == 0) {
                return;
            }
            goto write_now;
        case 10:
            stdio_prnt_callback(context, '\r');
            break;
    }

    tmp_buf[current_length] = c;
    current_length += 1;
    context->current_length = current_length;
    if (current_length == 64) {
        context->current_length = 0;
    write_now:
        io_write(context->fd, tmp_buf, current_length);
    }
}

int fdprintf(int fd, const char *format, ...)
{
    stdio_prnt_context_t context;
    va_list va;
    int res;

    va_start(va, format);
    context.fd             = fd;
    context.current_length = 0;
    res                    = prnt(&stdio_prnt_callback, &context, format, va);
    va_end(va);
    return res;
}

int vfdprintf(int fd, const char *format, va_list va)
{
    stdio_prnt_context_t context;

    context.fd             = fd;
    context.current_length = 0;
    return prnt(&stdio_prnt_callback, &context, format, va);
}

int printf(const char *format, ...)
{
    stdio_prnt_context_t context;
    va_list va;
    int res;

    va_start(va, format);
    context.fd             = 1;
    context.current_length = 0;
    res                    = prnt(&stdio_prnt_callback, &context, format, va);
    va_end(va);
    return res;
}

static u32 tab_stop_padding = 0;

int fdputc(int c, int fd)
{
    char buf[1];

    buf[0] = c;
    if ((char)c == '\t') {
        io_write(fd, "        ", 8 - (tab_stop_padding & 7));
        tab_stop_padding = (tab_stop_padding & 0xFFFFFFF8) + 8;
    } else if ((char)c == '\n') {
        io_write(fd, "\r\n", 2);
        tab_stop_padding = 0;
    } else {
        if (isprint(c) != 0) {
            tab_stop_padding += 1;
        }
        io_write(fd, buf, 1);
    }
    return c;
}

int putchar(int c)
{
    return fdputc(c, 1);
}

int fdputs(const char *s, int fd)
{
    const char *s_s;

    s_s = s;
    if (s == NULL) {
        s_s = "<NULL>";
    }
    while (*s_s != '\x00') {
        fdputc(*s_s, fd);
        s_s += 1;
    }
    return 0;
}

int puts(const char *s)
{
    return fdputs(s, 1);
}

int fdgetc(int fd)
{
    char buf[1];

    io_read(fd, buf, 1);
    return (u8)(buf[0]);
}

int getchar(void)
{
    return fdgetc(0);
}

char *fdgets(char *buf, int fd)
{
    char *v4;
    char *v5;
    char v6;
    int v7;
    char v8;
    int v9;
    int v10;
    int v11;
    int v13;
    int v14;

    v4 = buf;
    v5 = buf + 125;
    for (;;) {
        v6 = fdgetc(fd);
        if (v6 == '\n') {
            break;
        }
        if (v6 < '\v') {
            if (v6 != '\b') {
                if (v6 == '\t') {
                    v6 = ' ';
                    v7 = ' ';
                } else {
                    v7 = v6;
                }
                goto LABEL_31;
            }
            goto LABEL_21;
        }
        if (v6 == '\x16') {
            v8 = fdgetc(fd);
            v9 = fd;
            if (v4 >= v5) {
                if (!fd) {
                    v9 = 1;
                }
                v10 = 7;
                goto LABEL_29;
            }
            *v4++ = v8;
            v10   = v8;
            if (!fd) {
                v9 = 1;
            }
        LABEL_29:
            fdputc(v10, v9);
        } else {
            if (v6 < '\x17') {
                if (v6 == '\r') {
                    break;
                }
                v7 = v6;
                goto LABEL_31;
            }
            if (v6 != '\x7F') {
                v7 = v6;
            LABEL_31:
                if (isprint(v7) == 0) {
                    v9 = fd;
                LABEL_37:
                    if (!v9) {
                        v9 = 1;
                    }
                    v10 = 7;
                    goto LABEL_29;
                }
                v9 = fd;
                if (v4 >= v5) {
                    goto LABEL_37;
                }
                *v4++ = v6;
                if (!fd) {
                    v9 = 1;
                }
                v10 = v7;
                goto LABEL_29;
            }
        LABEL_21:
            if (buf < v4) {
                --v4;
                v13 = fd;
                if (!fd) {
                    v13 = 1;
                }
                fdputc('\b', v13);
                v14 = fd;
                if (!fd) {
                    v14 = 1;
                }
                fdputc(' ', v14);
                v9 = fd;
                if (!fd) {
                    v9 = 1;
                }
                v10 = 8;
                goto LABEL_29;
            }
        }
    }
    v11 = fd;
    if (!fd) {
        v11 = 1;
    }
    fdputc('\n', v11);
    *v4 = 0;
    return buf;
}

char *gets(char *s)
{
    return fdgets(s, 0);
}

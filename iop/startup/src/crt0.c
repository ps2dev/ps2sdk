#include <stdio.h>
typedef void (*func_ptr)(void);
extern func_ptr __CTOR_END__[];
extern func_ptr __DTOR_LIST__[];
extern func_ptr __DTOR_END__[];

static void __do_global_ctors(void)
{
    func_ptr *p = __CTOR_END__ - 1;

    if (*(int *)p != -1) {
        for (; *(int *)p != -1; p--) {
            (*p)();
        }
    }
}

static void __do_global_dtors(void)
{
    int num = __DTOR_END__ - __DTOR_LIST__ - 1;
    int idx = 0;

    while (idx < num) {
        idx++;
        __DTOR_LIST__[idx]();
    }
}

extern int main(int argc, char *argv[]);

int _start(int argc, char *argv[])
{
    int ret;

    if (argc >= 0) {
        __do_global_ctors();
        ret = main(argc, argv);
    } else {
        ret = main(argc, argv);
        __do_global_dtors();
    }

    return ret;
}

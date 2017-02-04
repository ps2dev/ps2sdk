// This include will allow us to avoid reincluding other headers
#include "irx_imports.h"

#define MODNAME "hello"
IRX_ID("IOP skeleton", 1, 1);

extern struct irx_export_table _exp_hello;


// This contain our prototype for our export, and we need it to call it inside our _start
#include "hello.h"


// This is a bit like a "main" for IRX files.
int _start(int argc, char * argv[]) {
    if (RegisterLibraryEntries(&_exp_hello) != 0)
        return 1;

    hello();

    return 0;
}

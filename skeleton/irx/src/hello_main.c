// This include will allow us to avoid reincluding other headers
#include "irx_imports.h"

#define MODNAME "alloc"
IRX_ID("Basic alloc library", 1, 1);

struct irx_export_table _exp_alloc;


// This contain our prototype for our export, and we need it to call it inside our _start
#include "hello.h"


// This is a bit like a "main" for IRX files.
int _start(int argc, char * argv[]) {
    if (RegisterLibraryEntries(&_exp_alloc) != 0)
        return 1;

    hello();
    
    return 0;
}

// This include will allow us to avoid reincluding other headers
#include "irx_imports.h"

// This contain our prototype for our export, and we need it to call it inside our _start
#include "hello.h"


// This is a bit like a "main" for IRX files.
int _start(int argc, char * argv[]) {
    hello();
    
    return 0;
}

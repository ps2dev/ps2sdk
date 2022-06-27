#define SYSCLIB_DISABLE_BUILTINS
#include <sysclib.h>

// Using char is nonstandard
char _tolower(char c)
{
	if (isupper(c)) return c | 32;
	return c;
}

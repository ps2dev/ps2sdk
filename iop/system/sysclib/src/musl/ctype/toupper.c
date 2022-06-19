#define SYSCLIB_DISABLE_BUILTINS
#include <sysclib.h>

// Using char is nonstandard
char _toupper(char c)
{
	if (islower(c)) return c & 0x5f;
	return c;
}

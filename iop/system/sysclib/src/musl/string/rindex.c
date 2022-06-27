#define SYSCLIB_DISABLE_BUILTINS
#include <sysclib.h>

char *rindex(const char *s, int c)
{
	return strrchr(s, c);
}

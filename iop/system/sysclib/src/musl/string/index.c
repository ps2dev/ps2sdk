#define SYSCLIB_DISABLE_BUILTINS
#include <sysclib.h>

char *index(const char *s, int c)
{
	return strchr(s, c);
}

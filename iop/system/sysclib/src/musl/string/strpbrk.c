#define SYSCLIB_DISABLE_BUILTINS
#include <sysclib.h>

char *strpbrk(const char *s, const char *b)
{
	s += strcspn(s, b);
	return *s ? (char *)s : 0;
}

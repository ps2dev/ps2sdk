#define SYSCLIB_DISABLE_BUILTINS
#include <sysclib.h>

char *__stpncpy(char *restrict d, const char *restrict s, size_t n);

char *strncpy(char *restrict d, const char *restrict s, size_t n)
{
	__stpncpy(d, s, n);
	return d;
}

#define SYSCLIB_DISABLE_BUILTINS
#include <sysclib.h>

char *__stpcpy(char *restrict d, const char *restrict s);

char *strcpy(char *restrict dest, const char *restrict src)
{
	__stpcpy(dest, src);
	return dest;
}

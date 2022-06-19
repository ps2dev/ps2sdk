#define SYSCLIB_DISABLE_BUILTINS
#include <sysclib.h>

void *__memrchr(const void *m, int c, size_t n);

char *strrchr(const char *s, int c)
{
	return __memrchr(s, c, strlen(s) + 1);
}

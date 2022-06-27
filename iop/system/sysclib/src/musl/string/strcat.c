#define SYSCLIB_DISABLE_BUILTINS
#include <sysclib.h>

char *strcat(char *restrict dest, const char *restrict src)
{
	strcpy(dest + strlen(dest), src);
	return dest;
}

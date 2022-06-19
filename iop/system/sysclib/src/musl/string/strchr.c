#define SYSCLIB_DISABLE_BUILTINS
#include <sysclib.h>

char *__strchrnul(const char *s, int c);

char *strchr(const char *s, int c)
{
	char *r = __strchrnul(s, c);
	return *(unsigned char *)r == (unsigned char)c ? r : 0;
}

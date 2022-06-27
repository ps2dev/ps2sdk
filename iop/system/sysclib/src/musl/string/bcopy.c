#include <sysclib.h>

void bcopy(const void *s1, void *s2, size_t n)
{
	memmove(s2, s1, n);
}

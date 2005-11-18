#include <unistd.h>
#include <sys/stat.h>

#ifdef F_getcwd
extern char __direct_pwd[256];

char *getcwd(char *buf, int len)
{
	strncpy(buf, __direct_pwd, len);
	return buf;
}
#endif

#ifdef F_access
int access(const char *path, int mode)
{
	return -1;
}
#endif

#ifdef F_stat
int stat(const char *path, struct stat *sbuf)
{
	return -1;
}
#endif

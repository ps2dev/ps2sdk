/*
  _____     ___ ____
   ____|   |    ____|      PS2 OpenSource Project
  |     ___|   |____       (C) 2003 Hiryu (ps2dev@ntlworld.com)
  
  ------------------------------------------------------------------------
  string.h
  		String function prototypes
*/

#ifndef _STRING_H
#define _STRING_H

#include <stddef.h>
#include <stdarg.h>

/* The maximum length of a string within PS2Lib.  */
#define PS2LIB_STR_MAX	4096

#ifdef __cplusplus
extern "C" {
#endif


/* ASM String functions by Jeff Johnston of Cygnus Solutions */
void *	memchr(const void *, int, size_t);
void *	memcpy(void *, const void *, size_t);
void *	memmove(void *, const void *, size_t);
void *	memset(void *, int, size_t);

int	memcmp(const void *, const void *, size_t);

int	strcmp(const char *, const char *);
int	strncmp(const char *, const char *, size_t);

unsigned int strlen(const char *);

char *	strcat(char *, const char *);
char *	strchr(const char *, int);
char *	strcpy(char *, const char *);
char *	strncat(char *, const char *, size_t);
char *	strncpy(char *, const char *, size_t);



/* C String functions by Hiryu (A.Lee) */
#define stricmp strcasecmp
#define strnicmp strncasecmp

int	 strcasecmp(const char *, const char *);
int	 strncasecmp(const char *, const char *, size_t);

char *	strtok(char *, const char *);
char *	strrchr(const char *, int);

char *	strstr(const char *, const char *);

long strtol(const char *nptr, char **endptr, int base);
unsigned long strtoul(const char *s, char **endptr, int base);

#define atoi(s) strtol(s,NULL,10)

char * strupr(char *);
char * strlwr(char *);

int tolower(int);
int toupper(int);

int isalnum(int);
int isalpha(int);
int iscntrl(int);
int isdigit(int);
int isgraph(int);
int islower(int);
int isprint(int);
int ispunct(int);
int isspace(int);
int isupper(int);
int isxdigit(int);


#ifdef __cplusplus
}
#endif

#endif	// _STRING_H


#ifndef	_STRING_H_H
#define	_STRING_H_H
#include "stdint.h"


extern void memcpy_s(char *des, char *src, uint32 size);
extern int memcmp_s(char *src1, char *src2, uint32 size);
extern void *memset_s(void *s, int c, size_t count);
extern char *strcpy_s(char *dest, const char *src);
extern uint32 strlen_s(const char *s);
int strncmp_s(const char *cs, const char *ct, uint32 count);
int strcmp_s(const char *str1, const char *str2);
uint32 strnlen(const char *s, uint32 maxlen);

char *hexstr32(char buff[9], uint32 val);
char *hexstr64(char buff[17], uint64 val);

void *memcpy (void *__restrict, const void *__restrict, size_t);
void *memmove (void *, const void *, size_t);
void *memset (void *, int, size_t);
int memcmp (const void *, const void *, size_t);
void *memchr (const void *, int, size_t);

char *strcpy (char *__restrict, const char *__restrict);
char *strncpy (char *__restrict, const char *__restrict, size_t);

char *strcat (char *__restrict, const char *__restrict);
char *strncat (char *__restrict, const char *__restrict, size_t);

int strcmp (const char *, const char *);
int strncmp (const char *, const char *, size_t);

int strcoll (const char *, const char *);
size_t strxfrm (char *__restrict, const char *__restrict, size_t);

char *strchr (const char *, int);
char *strrchr (const char *, int);

size_t strcspn (const char *, const char *);
size_t strspn (const char *, const char *);
char *strpbrk (const char *, const char *);
char *strstr (const char *, const char *);
char *strtok (char *__restrict, const char *__restrict);

size_t strlen (const char *);

char *strerror (int);


//从指定位置查找字符串
int IndexStr_KMP(char *str, int strsize, const char *dest, int *nextbuff, int destsize);
#endif
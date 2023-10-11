#ifndef _STDLIB_H_H
#define _STDLIB_H_H
#include "stdint.h"

int atoi (const char *);
long atol (const char *);
long long atoll (const char *);
double atof (const char *);

int mblen (const char *, size_t);
int mbtowc (wchar_t *__restrict, const char *__restrict, size_t);
int wctomb (char *, wchar_t);
size_t mbstowcs (wchar_t *__restrict, const char *__restrict, size_t);
size_t wcstombs (char *__restrict, const wchar_t *__restrict, size_t);

#endif
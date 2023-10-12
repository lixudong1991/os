#ifndef _STDIO_H_H
#define _STDIO_H_H
#include "stdint.h"

int printf(const char *fmt, ...);

void putchar(int _Character);
void puts(const char *str);

int sprintf(char *__restrict, const char *__restrict, ...);
int snprintf(char *__restrict, size_t, const char *__restrict, ...);

int vsprintf(char *__restrict, const char *__restrict, va_list);
int vsnprintf(char *__restrict, size_t, const char *__restrict, va_list);


uint32 getchar();
int fgets(char* s, int size);

#endif
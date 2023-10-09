#ifndef _PRINT_F_HH_
#define _PRINT_F_HH_

int vsprintf(char *buf, const char *fmt, va_list args);
int sprintf(char *buf, const char *fmt, ...);
int printf(const char *fmt, ...);
void putchar(int _Character);
void puts(const char *str);
int putcharPos(int _Character, int pos);

int interruptPrintf(const char *fmt, ...);
#endif // ! _PRINT_F_HH_

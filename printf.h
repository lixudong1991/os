#ifndef _PRINT_F_HH_
#define _PRINT_F_HH_

int vsprintf(char *buf, const char *fmt, va_list args);
int sprintf(char *buf, const char *fmt, ...);
int printf(const char *fmt, ...);
int putchar(int _Character);
void puts(const char *str);

extern LockObj printLock;
#endif // ! _PRINT_F_HH_

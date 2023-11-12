#include "syslib.h"
#include "../include/stdio.h"
#include "../include/string.h"
int printf(const char* fmt, ...)
{
	char printf_buf[1024];
	va_list args;
	int printed;

	va_start(args, fmt);
	printed = vsprintf(printf_buf, fmt, args);
	va_end(args);
	//asm("cli");
	puts(printf_buf);
	//	asm("sti");
	return printed;
}

int _start(int argc,void *argv)
{
  unsigned int count = 0xfffff;
  while(1){
	  printf("%d +++++++++++++++user application run!\n", argc);
	  count = 0xffffff;
	  while (count--) {}
  };
  return 5;
}

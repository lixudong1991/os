#include "syscall.h"
#include "stdint.h"
#include "string.h"
uint32_t sysCallAddr[SYSCALL_COUNT] = { 0 };

typedef int (*InterruptPrintfFun)(const char* fmt, ...);
extern InterruptPrintfFun interrput;
uint32_t sysRead(uint32_t callnum,uint32_t arg1, uint32_t arg2, uint32_t arg3, uint32_t arg4, uint32_t arg5, uint32_t arg6)
{
	interrput("callnum: %d sysRead(%d, 0x%x, %d)\n", callnum, arg1, arg2, arg3);
	asm("hlt");
	return 0;
}
uint32_t sysWirte(uint32_t callnum, uint32_t arg1, uint32_t arg2, uint32_t arg3, uint32_t arg4, uint32_t arg5, uint32_t arg6)
{
	interrput("callnum: %d sysWirte(%d, 0x%x, %d)\n", callnum, arg1, arg2, arg3);
	asm("hlt");
	return 0;
}
uint32_t sysOpen(uint32_t callnum, uint32_t arg1, uint32_t arg2, uint32_t arg3, uint32_t arg4, uint32_t arg5, uint32_t arg6)
{
	interrput("callnum: %d sysOpen(%s,%d,%d)\n", callnum, (char*)arg1, arg2, arg3);
	asm("hlt");
	return 0;
}
uint32_t sysclose(uint32_t callnum, uint32_t arg1, uint32_t arg2, uint32_t arg3, uint32_t arg4, uint32_t arg5, uint32_t arg6)
{
	interrput("callnum: %d sysclose(%d)\n", callnum, arg1);
	asm("hlt");
	return 0;
}
uint32_t syscallTemp(uint32_t callnum, uint32_t arg1, uint32_t arg2, uint32_t arg3, uint32_t arg4, uint32_t arg5, uint32_t arg6)
{
	interrput("callnum: %d syscallTemp(0x%x,0x%x,0x%x,0x%x,0x%x,0x%x)\n", callnum, arg1, arg2, arg3, arg4, arg5, arg6);
	asm("hlt");
	return 0;
}

void initSysCall()
{
	memDWordset_s(sysCallAddr, syscallTemp, SYSCALL_COUNT);
	sysCallAddr[SYS_read] = sysRead;
	sysCallAddr[SYS_write] = sysWirte;
	sysCallAddr[SYS_open] = sysOpen;
	sysCallAddr[SYS_close] = sysclose;
}



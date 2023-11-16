#include "syscall.h"
#include "stdint.h"
#include "string.h"
#include "boot.h"
#include "vbe.h"
uint32_t sysCallAddr[SYSCALL_COUNT] = { 0 };

typedef int (*InterruptPrintfFun)(const char* fmt, ...);
extern InterruptPrintfFun interrput;
extern void x_apicwriteEOI();
extern void taskSleep();
uint32_t sysRead(uint32_t callnum,uint32_t arg1, uint32_t arg2, uint32_t arg3, uint32_t arg4, uint32_t arg5, uint32_t arg6)
{
	interrput("callnum: %d sysRead(%d, 0x%x, %d)\n", callnum, arg1, arg2, arg3);
	asm("hlt");
	x_apicwriteEOI();
	return 0;
}
uint32_t sysWirte(uint32_t callnum, uint32_t arg1, uint32_t arg2, uint32_t arg3, uint32_t arg4, uint32_t arg5, uint32_t arg6)
{
	interrput("callnum: %d sysWirte(%d, 0x%x, %d)\n", callnum, arg1, arg2, arg3);
	asm("hlt");
	x_apicwriteEOI();
	return 0;
}
uint32_t sysOpen(uint32_t callnum, uint32_t arg1, uint32_t arg2, uint32_t arg3, uint32_t arg4, uint32_t arg5, uint32_t arg6)
{
	interrput("callnum: %d sysOpen(%s,%d,%d)\n", callnum, (char*)arg1, arg2, arg3);
	asm("hlt");
	x_apicwriteEOI();
	return 0;
}
uint32_t sysClose(uint32_t callnum, uint32_t arg1, uint32_t arg2, uint32_t arg3, uint32_t arg4, uint32_t arg5, uint32_t arg6)
{
	interrput("callnum: %d sysclose(%d)\n", callnum, arg1);
	asm("hlt");
	x_apicwriteEOI();
	return 0;
}

uint32_t sysIoctl(uint32_t callnum, uint32_t arg1, uint32_t arg2, uint32_t arg3, uint32_t arg4, uint32_t arg5, uint32_t arg6)
{
	x_apicwriteEOI();
	return 0;
}

struct iovec { void* iov_base; size_t iov_len; };
uint32_t sysWirtev(uint32_t callnum, uint32_t arg1, uint32_t arg2, uint32_t arg3, uint32_t arg4, uint32_t arg5, uint32_t arg6)
{
	struct iovec* vec = (struct iovec*)arg2;
	static uint64_t countcall = 0;
	uint32_t ret = 0;
	if (arg1 == 1) 
	{
		//interrput("callnum: %d 0x%llx sysWirtev(%d,0x%x, %d) cr3:%x calladdr:0x%x\n", callnum, countcall, arg1, arg2, arg3,cr3_data(), arg6);
		
		spinlock(lockBuff[PRINT_LOCK].plock);
		for (uint32_t index = 0; index < arg3; index++)
		{
			//interrput("fd:%d iov_base:0x%x iov_len:%d\n", arg1, vec[index].iov_base, vec[index].iov_len);
			if (vec[index].iov_base)
			{
				char* ptr = vec[index].iov_base;
				ret += vec[index].iov_len;
				for (uint32_t j = 0; j < vec[index].iov_len; j++)
				{
					consolePutchar(ptr[j]);
				}
			}
		}
		unlock(lockBuff[PRINT_LOCK].plock);
		//interrput("0x%llx\n", countcall++);

	}
	//asm("hlt");
	x_apicwriteEOI();
	return ret;
}
typedef struct timespec32 {
	long tv_sec;
	long tv_nsec;
}timespec32;
uint32_t sysNanoSleep(uint32_t callnum, uint32_t arg1, uint32_t arg2, uint32_t arg3, uint32_t arg4, uint32_t arg5, uint32_t arg6)
{
	timespec32* req = (timespec32*)arg1;
	timespec32* rem = (timespec32*)arg2;
	interrput("callnum: %d sysNanoSleep(0x%x, 0x%x) calladdr:0x%x  req(0x%x:0x%x) rem(0x%x:0x%x)\n", callnum, arg1, arg2, arg6, req->tv_sec, req->tv_nsec, rem->tv_sec, rem->tv_nsec);
	taskSleep();
	return 0;
}
uint32_t sysClockNanoSleep(uint32_t callnum, uint32_t arg1, uint32_t arg2, uint32_t arg3, uint32_t arg4, uint32_t arg5, uint32_t arg6)
{
	timespec32* req = (timespec32*)arg1;
	timespec32* rem = (timespec32*)arg2;
	interrput("callnum: %d sysClockNanoSleep(0x%x, 0x%x,0x%x, 0x%x) calladdr:0x%x   req(0x%x:0x%x) rem(0x%x:0x%x)\n", callnum, arg1, arg2, arg3, arg4, arg6, req->tv_sec, req->tv_nsec, rem->tv_sec, rem->tv_nsec);
	taskSleep();
	return 0;
}

uint32_t syscallTemp(uint32_t callnum, uint32_t arg1, uint32_t arg2, uint32_t arg3, uint32_t arg4, uint32_t arg5, uint32_t arg6)
{
	interrput("callnum: %d syscallTemp(0x%x,0x%x,0x%x,0x%x,0x%x,0x%x)\n", callnum, arg1, arg2, arg3, arg4, arg5, arg6);
	asm("hlt");
	x_apicwriteEOI();
	return 0;
}

void initSysCall()
{
	memDWordset_s(sysCallAddr, syscallTemp, SYSCALL_COUNT);
	sysCallAddr[SYS_read] = sysRead;
	sysCallAddr[SYS_write] = sysWirte;
	sysCallAddr[SYS_open] = sysOpen;
	sysCallAddr[SYS_close] = sysClose;
	sysCallAddr[SYS_ioctl] = sysIoctl;
	sysCallAddr[SYS_writev] = sysWirtev;
	sysCallAddr[SYS_nanosleep] = sysNanoSleep;
	sysCallAddr[SYS_clock_nanosleep_time32] = sysClockNanoSleep;
}



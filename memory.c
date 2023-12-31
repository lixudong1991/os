#include "boot.h"
#define ALLOC_ALIGN 4
extern  BootParam bootparam;
char* allocate_memory(TaskCtrBlock* task, uint32 size, uint32 prop)
{
	uint32 sizealign = size;
	if(task->AllocateNextAddr%ALLOC_ALIGN!=0)
		task->AllocateNextAddr = (task->AllocateNextAddr/ALLOC_ALIGN+1)*ALLOC_ALIGN;
	if(sizealign%ALLOC_ALIGN!=0)
		sizealign = (sizealign/ALLOC_ALIGN+1)*ALLOC_ALIGN;
	char *ret = allocateVirtual4kPage(sizealign, &(task->AllocateNextAddr), prop);
	return ret;
}

char* allocateVirtual4kPage(uint32 size, uint32* pAddr,uint32 prop)
{
	uint32 addr = *pAddr;
	uint32 startaddr = addr & 0xFFFFF000,endaddr =(addr+ size -1)& 0xFFFFF000;
	prop &= 6;
	for (; startaddr <= endaddr;)
	{
		uint32 pageDiraddr = startaddr >> 22;
		pageDiraddr = pageDiraddr<<2;
		uint32 tableaddr = *(uint32*)(0xfffff000+ pageDiraddr);
		if ((tableaddr & 1) != 1)
		{
			tableaddr = (uint32)allocatePhy4kPage(0);
			tableaddr |= (prop|1);
			*(uint32*)(0xfffff000 + pageDiraddr) = tableaddr;
			resetcr3();
			memset_s((char*)(0xffc00000 | (pageDiraddr << 10)),0,4096);
		}
		else
		{
			tableaddr |= (prop|1);
			*(uint32*)(0xfffff000 + pageDiraddr) = tableaddr;
			resetcr3();	
		}

		uint32 pageAddr = (startaddr & 0x3FF000)>> 12;
		pageAddr =pageAddr<<2;
		uint32 *pagePhyAddr =(uint32*)((0xffc00000|(pageDiraddr<<10))+ pageAddr);
		if (((*pagePhyAddr)&1)!=1)
		{
			*pagePhyAddr = (uint32)allocatePhy4kPage(0);
			(*pagePhyAddr) |= 1;
		}
		(*pagePhyAddr) |= prop;
		startaddr += 0x1000;
		resetcr3();
	}
	*pAddr = addr + size;
	return (char*)addr;
}
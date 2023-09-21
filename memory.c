#include "boot.h"
#include "memcachectl.h"
#define ALLOC_ALIGN 4
#define START_PHY_MEM_PAGE 0x100
extern BootParam bootparam;
extern KernelData kernelData;
char *allocate_memory(TaskCtrBlock *task, uint32 size, uint32 prop)
{

	uint32 sizealign = size;
	if (task->AllocateNextAddr % ALLOC_ALIGN != 0)
		task->AllocateNextAddr = (task->AllocateNextAddr / ALLOC_ALIGN + 1) * ALLOC_ALIGN;
	if (sizealign % ALLOC_ALIGN != 0)
		sizealign = (sizealign / ALLOC_ALIGN + 1) * ALLOC_ALIGN;
	char *ret = allocateVirtual4kPage(sizealign, &(task->AllocateNextAddr), prop);
	return ret;
}

char *allocateVirtual4kPage(uint32 size, uint32 *pAddr, uint32 prop)
{
	uint32 addr = *pAddr;
	uint32 startaddr = addr & 0xFFFFF000, endaddr = (addr + size - 1) & 0xFFFFF000;
	prop &= 6;
	for (; startaddr <= endaddr;)
	{
		uint32 pageDiraddr = startaddr >> 22;
		pageDiraddr = pageDiraddr << 2;
		uint32 *ppageDiraddr = (uint32 *)(0xfffff000 + pageDiraddr);
		asm("mfence");
		uint32 tableaddr = *ppageDiraddr;
		if ((tableaddr & 1) != 1)
		{
			tableaddr = (uint32)allocatePhy4kPage(START_PHY_MEM_PAGE);
			tableaddr |= (prop | 1);
			asm("mfence");
			*ppageDiraddr = tableaddr;
			resetcr3();
			memset_s((char *)(0xffc00000 | (pageDiraddr << 10)), 0, 4096);
		}
		else
		{
			tableaddr |= (prop | 1);
			asm("mfence");
			*ppageDiraddr = tableaddr;
			resetcr3();
		}

		uint32 pageAddr = (startaddr & 0x3FF000) >> 12;
		pageAddr = pageAddr << 2;
		uint32 *pagePhyAddr = (uint32 *)((0xffc00000 | (pageDiraddr << 10)) + pageAddr);
		asm("mfence");
		if (((*pagePhyAddr) & 1) != 1)
		{
			*pagePhyAddr = (uint32)allocatePhy4kPage(0);
			(*pagePhyAddr) |= 1;
		}
		asm("mfence");
		(*pagePhyAddr) |= prop;
		startaddr += 0x1000;
		resetcr3();
	}
	*pAddr = addr + size;
	return (char *)addr;
}

int mem4k_map(uint32 linearaddr, uint32 phyaddr, int memcachType, uint32 prop)
{
	if ((linearaddr & 0x0fff) || (phyaddr & 0x0fff))
		return FALSE;
	uint32 startaddr = linearaddr;
	prop &= 6;
	uint32 pageDiraddr = startaddr >> 22;
	pageDiraddr = pageDiraddr << 2;
	uint32 *ppageDiraddr = (uint32 *)(0xfffff000 + pageDiraddr);
	asm("mfence");
	uint32 tableaddr = *ppageDiraddr;
	if ((tableaddr & 1) != 1)
	{
		tableaddr = (uint32)allocatePhy4kPage(START_PHY_MEM_PAGE);
		tableaddr |= (prop | 1);
		asm("mfence");
		*ppageDiraddr = tableaddr;
		resetcr3();
		memset_s((char *)(0xffc00000 | (pageDiraddr << 10)), 0, 4096);
	}
	else
	{
		tableaddr |= (prop | 1);
		asm("mfence");
		*ppageDiraddr = tableaddr;
		resetcr3();
	}

	uint32 pageAddr = (startaddr & 0x3FF000) >> 12;
	pageAddr = pageAddr << 2;
	uint32 *pagePhyAddr = (uint32 *)((0xffc00000 | (pageDiraddr << 10)) + pageAddr);
	asm("mfence");

	*pagePhyAddr = phyaddr;
	(*pagePhyAddr) |= 1;

	asm("mfence");
	(*pagePhyAddr) |= prop;
	if (memcachType >= 0 || memcachType < 8)
	{
		asm("mfence");
		(*pagePhyAddr) |= mem_type_map_pat[memcachType];
	}
	resetcr3();
	return TRUE;
}

void *kernel_malloc(uint32 size)
{
	return allocate_memory(kernelData.taskList.tcb_Frist, size, PAGE_RW);
}
void kernel_free(void *p)
{
}
void kassert(int expression)
{
}
int mem4k_unmap(uint32 linearaddr, int isFreePhyPage)
{
	if ((linearaddr & 0x0fff))
		return FALSE;
	uint32 startaddr = linearaddr;
	uint32 pageDiraddr = startaddr >> 22;
	pageDiraddr = pageDiraddr << 2;
	uint32 *ppageDiraddr = (uint32 *)(0xfffff000 + pageDiraddr);
	asm("mfence");
	uint32 tableaddr = *ppageDiraddr;
	if ((tableaddr & 1) != 1)
		return TRUE;

	uint32 pageAddr = (startaddr & 0x3FF000) >> 12;
	pageAddr = pageAddr << 2;
	uint32 *pagePhyAddr = (uint32 *)((0xffc00000 | (pageDiraddr << 10)) + pageAddr);
	asm("mfence");
	uint32 phyaddr = *pagePhyAddr;
	*pagePhyAddr = 0;
	if (isFreePhyPage)
		freePhy4kPage(phyaddr);
	resetcr3();
	return TRUE;
}
#include "boot.h"
#include "memcachectl.h"
#include "string.h"
#define ALLOC_ALIGN 4
#define START_PHY_MEM_PAGE 0x100
extern BootParam bootparam;
extern KernelData kernelData;
char *allocate_memory(TaskCtrBlock *task, uint32 size, uint32 prop)
{
	char *ret = NULL;
	uint32 sizealign = size;
	if (sizealign % ALLOC_ALIGN != 0)
		sizealign = (sizealign / ALLOC_ALIGN + 1) * ALLOC_ALIGN;
	sizealign += 4;
	TaskFreeMemList *pFreeMem = (TaskFreeMemList *)(*(task->pFreeListAddr));
	while (pFreeMem)
	{
		if (pFreeMem->status == TaskFreeMemListNodeUnUse)
		{
			pFreeMem = pFreeMem->next;
			continue;
		}
		if (pFreeMem->memSize >= sizealign)
		{
			if (pFreeMem->memSize - sizealign < 8)
			{
				ret = allocateVirtual4kPage(pFreeMem->memSize, &(pFreeMem->memAddr), prop);
				pFreeMem->status = TaskFreeMemListNodeUnUse;
				*(uint32_t *)ret = pFreeMem->memSize;
			}
			else
			{
				pFreeMem->memSize -= sizealign;
				ret = allocateVirtual4kPage(sizealign, &(pFreeMem->memAddr), prop);
				*(uint32_t *)ret = sizealign;
			}
			ret += 4;
			break;
		}
		pFreeMem = pFreeMem->next;
	}

	return ret;
}
void free_memory(TaskCtrBlock *task, void *addr) // 4×Ö½Ú¶ÔÆë
{
	uint32_t startaddr = (uint32_t)addr;
	startaddr -= 4;
	uint32_t size = *(uint32_t *)startaddr, endaddr = startaddr + size;

	uint32_t pageStart = startaddr & PAGE_ADDR_MASK, pageEnd = endaddr & PAGE_ADDR_MASK;
	TaskFreeMemList *pFreeMem = (TaskFreeMemList *)(*(task->pFreeListAddr));
	TaskFreeMemList *punUseNode = NULL;
	TaskFreeMemList *pMergeNodeEnd = NULL, *pMergeNodeStart = NULL;

	int pageStartStat = 0;
	int pageEndStat = 0;
	while (pFreeMem)
	{
		if (pFreeMem->status == TaskFreeMemListNodeUnUse)
		{
			punUseNode = pFreeMem;
			pFreeMem = pFreeMem->next;
			continue;
		}
		if (pMergeNodeEnd == NULL && pMergeNodeStart == NULL)
		{
			if ((pFreeMem->memAddr == endaddr))
			{
				pMergeNodeEnd = pFreeMem;
			}
			else if ((pFreeMem->memAddr < startaddr) && ((pFreeMem->memAddr + pFreeMem->memSize) == startaddr))
			{
				pMergeNodeStart = pFreeMem;
			}
		}
		if (startaddr != pageStart)
		{
			if ((pFreeMem->memAddr <= pageStart) && ((pFreeMem->memAddr + pFreeMem->memSize) >= startaddr))
			{
				pageStartStat = 1;
			}
		}
		if (endaddr != pageEnd)
		{
			if ((pFreeMem->memAddr <= endaddr) && ((pFreeMem->memAddr + pFreeMem->memSize) >= (pageEnd + 0x1000)))
			{
				pageEndStat = 1;
			}
		}
		pFreeMem = pFreeMem->next;
	}
	if (pMergeNodeStart != NULL)
	{
		pMergeNodeStart->memSize += size;
	}
	else if (pMergeNodeEnd != NULL)
	{
		pMergeNodeEnd->memAddr = startaddr;
		pMergeNodeEnd->memSize += size;
	}
	else
	{
		if (punUseNode)
		{
			punUseNode->memAddr = startaddr;
			punUseNode->memSize = size;
			punUseNode->status = TaskFreeMemListNodeUse;
		}
		else
		{
			TaskFreeMemList *ptempNode = kernel_malloc(sizeof(TaskFreeMemList));
			ptempNode->memAddr = startaddr;
			ptempNode->memSize = size;
			ptempNode->status = TaskFreeMemListNodeUse;
			ptempNode->next = *(task->pFreeListAddr);
			*(task->pFreeListAddr) = ptempNode;
		}
	}
	if ((startaddr == pageStart) && (endaddr == pageEnd))
	{
		while (pageStart < pageEnd)
		{
			mem4k_unmap(pageStart, TRUE);
			pageStart += 0x1000;
		}
	}
	else if ((startaddr != pageStart) && (endaddr == pageEnd))
	{
		if (!pageStartStat)
			pageStart += 0x1000;
		while (pageStart < pageEnd)
		{
			mem4k_unmap(pageStart, TRUE);
			pageStart += 0x1000;
		}
	}
	else if ((startaddr == pageStart) && (endaddr != pageEnd))
	{
		if (pageEndStat)
			pageEnd += 0x1000;
		while (pageStart < pageEnd)
		{
			mem4k_unmap(pageStart, TRUE);
			pageStart += 0x1000;
		}
	}
	else
	{
		if (!pageStartStat)
			pageStart += 0x1000;
		if (pageEndStat)
			pageEnd += 0x1000;
		while (pageStart < pageEnd)
		{
			mem4k_unmap(pageStart, TRUE);
			pageStart += 0x1000;
		}
	}
}
char *allocate_memory_align(TaskCtrBlock *task, uint32 size, uint32 prop, uint32 alignsize)
{
	char *ret = NULL;
	uint32 sizealign = size;
	if (sizealign % alignsize != 0)
		sizealign = (sizealign / alignsize + 1) * alignsize;
	sizealign += 4;
	TaskFreeMemList *pFreeMem = (TaskFreeMemList *)(*(task->pFreeListAddr));
	while (pFreeMem)
	{
		if (pFreeMem->status == TaskFreeMemListNodeUnUse)
		{
			pFreeMem = pFreeMem->next;
			continue;
		}
		if (pFreeMem->memSize >= sizealign)
		{
			if (pFreeMem->memSize - sizealign < 8)
			{
				ret = allocateVirtual4kPage(pFreeMem->memSize, &(pFreeMem->memAddr), prop);
				pFreeMem->status = TaskFreeMemListNodeUnUse;
				*(uint32_t *)ret = pFreeMem->memSize;
			}
			else
			{
				pFreeMem->memSize -= sizealign;
				ret = allocateVirtual4kPage(sizealign, &(pFreeMem->memAddr), prop);
				*(uint32_t *)ret = sizealign;
			}
			ret += 4;
			break;
		}
		pFreeMem = pFreeMem->next;
	}

	return ret;
}
char *realloc_memory(TaskCtrBlock *task, uint32_t addr, uint32 newsize, uint32 prop)
{
char* ret = NULL;
	if (addr)
	{
		uint32_t startaddr = (uint32_t)addr;
		startaddr -= 4;
		uint32_t size = *(uint32_t*)startaddr, endaddr = startaddr + size;
		if (newsize <= (size - 4))
		{
			ret = addr;
		}
		else
		{
			uint32_t sizealign = newsize;
			if (sizealign % ALLOC_ALIGN != 0)
				sizealign = (sizealign / ALLOC_ALIGN + 1) * ALLOC_ALIGN;
			sizealign += 4;

			TaskFreeMemList* pFreeMem = (TaskFreeMemList*)(*(task->pFreeListAddr));
			while (pFreeMem)
			{
				if (pFreeMem->status == TaskFreeMemListNodeUnUse)
				{
					pFreeMem = pFreeMem->next;
					continue;
				}
				if (pFreeMem->memAddr == endaddr)
				{
					if ((size + pFreeMem->memSize) >= sizealign)
						break;
				}
				pFreeMem = pFreeMem->next;
			}
			if (pFreeMem)
			{
				if ((size + pFreeMem->memSize) >= sizealign)
				{
					if (pFreeMem->memSize - (sizealign - size) < 8)
					{
						allocateVirtual4kPage(pFreeMem->memSize, &(pFreeMem->memAddr), prop);
						pFreeMem->status = TaskFreeMemListNodeUnUse;
						*(uint32_t*)startaddr = (pFreeMem->memSize + size);
					}
					else
					{
						allocateVirtual4kPage((sizealign - size), &(pFreeMem->memAddr), prop);
						pFreeMem->memSize -= (sizealign - size);
						*(uint32_t*)startaddr = sizealign;
					}
					ret = addr;
				}
			}
			else
			{
				ret = allocate_memory(task, newsize, prop);
				memcpy(ret, addr, (size - 4));
				free_memory(task, addr);
			}
		}
	}
	else
	{
		ret = allocate_memory(task, newsize, prop);
	}
	return ret;
}
char *allocateVirtual4kPage(uint32 size, uint32 *pAddr, uint32 prop)
{
	uint32 addr = *pAddr;
	uint32 startaddr = addr & PAGE_ADDR_MASK, endaddr = (addr + size - 1) & PAGE_ADDR_MASK;
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
			*pagePhyAddr = (uint32)allocatePhy4kPage(START_PHY_MEM_PAGE);
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
void *kernel_realloc(void *mem_address, unsigned int newsize)
{
	// if(kernelData.taskList.tcb_Frist->AllocateNextAddr)
	return realloc_memory(kernelData.taskList.tcb_Frist, mem_address, newsize, PAGE_RW);
}
void *kernel_malloc(uint32 size)
{
	return allocate_memory(kernelData.taskList.tcb_Frist, size, PAGE_RW);
}
void *kernel_malloc_align(uint32 size, uint32 alignsize)
{
	return allocate_memory_align(kernelData.taskList.tcb_Frist, size, PAGE_RW, alignsize);
}
void kernel_free(void *p)
{
	free_memory(kernelData.taskList.tcb_Frist, p);
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
int get_4kpage_phyaddr(phys_addr_t linearaddr, phys_addr_t *phyaddr)
{
	uint32 pageDiraddr = linearaddr >> 22;
	pageDiraddr = pageDiraddr << 2;
	uint32 *ppageDiraddr = (uint32 *)(0xfffff000 + pageDiraddr);
	asm("mfence");
	uint32 tableaddr = *ppageDiraddr;
	if ((tableaddr & 1) != 1)
		return 0;
	uint32 pageAddr = (linearaddr & 0x3FF000) >> 12;
	pageAddr = pageAddr << 2;
	uint32 *pagePhyAddr = (uint32 *)((0xffc00000 | (pageDiraddr << 10)) + pageAddr);
	asm("mfence");
	if (((*pagePhyAddr) & 1) != 1)
		return 0;
	*phyaddr = (*pagePhyAddr) & PAGE_ADDR_MASK;
	return 1;
}
int get_memory_map_etc(phys_addr_t address, size_t numBytes, Physical_entry *table, uint32 *_numEntries)
{
	uint32 numEntries = *_numEntries;
	*_numEntries = 0;
	phys_addr_t addr = address;
	phys_addr_t startPage = addr & PAGE_ADDR_MASK, endPage = (addr + numBytes - 1) & PAGE_ADDR_MASK;
	uint32_t tableIndex = 0;
	uint32_t size = 0;
	int status = 0;
	for (phys_addr_t index = startPage; index <= endPage; index += 0x1000)
	{
		if (tableIndex >= numEntries)
			return 0;
		status = get_4kpage_phyaddr(index, &(table[tableIndex].address)) + (addr - index);
		if (status == 0)
			return 0;
		table[tableIndex].address += (addr - index);
		size = (index + 0x1000) - addr;
		if (numBytes <= size)
		{
			table[tableIndex++].size = numBytes;
			break;
		}
		else
		{
			table[tableIndex++].size = size;
			numBytes -= size;
		}
		addr += size;
	}
	*_numEntries = tableIndex;
	return TRUE;
}
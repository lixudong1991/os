#include "elf.h"
#include "string.h"
void loadElf(char *elfData, ProgramaData *proData, uint32_t privileg)
{
	Elf32_Ehdr *elfhead = (Elf32_Ehdr *)elfData;
	proData->proEntry = elfhead->e_entry;
	uint32 ephoff = elfhead->e_phoff;

	for (uint16_t i = 0; i < elfhead->e_phnum; i++)
	{
		Elf32_Phdr *phdr = (Elf32_Phdr *)(elfData + ephoff);
		if (phdr->p_type == PT_LOAD)
		{
			if (phdr->p_memsz != 0)
			{
				if ((phdr->p_vaddr & PAGE_ADDR_MASK) < proData->vir_base)
					proData->vir_base = (phdr->p_vaddr & PAGE_ADDR_MASK);


				uint32 vend = phdr->p_vaddr + phdr->p_memsz - 1;

				uint32_t masklow = ~((uint32_t)PAGE_ADDR_MASK);
				if (((vend & PAGE_ADDR_MASK)+ masklow)> proData->vir_end)
					proData->vir_end = ((vend & PAGE_ADDR_MASK) + masklow);
				uint32 vstart = phdr->p_vaddr;
				uint32 prop = privileg;
				if (phdr->p_flags & PF_W)
					prop |= PF_W;

				char *destaddr = allocateVirtual4kPage(phdr->p_memsz, &(vstart), prop);
				memset(destaddr,0, phdr->p_memsz);
				if (phdr->p_filesz != 0)
					memcpy_s(destaddr, elfData + phdr->p_offset, phdr->p_filesz);
			}
		}
		ephoff += elfhead->e_phentsize;
	}
}
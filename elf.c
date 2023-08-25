#include"elf.h"

void loadElf(char* elfData, ProgramaData* proData, uint32 privileg)
{
	Elf32_Ehdr* elfhead = (Elf32_Ehdr*)elfData;
	proData->proEntry = elfhead->e_entry;
	uint32 ephoff = elfhead->e_phoff;

	for (uint16_t i = 0;i< elfhead->e_phnum;i++)
	{
		Elf32_Phdr* phdr = (Elf32_Phdr*)(elfData + ephoff);
		if (phdr->p_type == PT_LOAD)
		{
			if ( phdr->p_memsz != 0)
			{
				if (phdr->p_vaddr < proData->vir_base)
					proData->vir_base = phdr->p_vaddr;
				
				int align = phdr->p_memsz / phdr->p_align;
				if (phdr->p_memsz % phdr->p_align != 0)
					align++;
				uint32 sizemem = align * phdr->p_align;
				uint32 vend = phdr->p_vaddr + sizemem - 1;
				if (vend > proData->vir_end)
					proData->vir_end = vend;
				uint32 prop = privileg;
				if (phdr->p_flags & PF_W)
					prop |= PF_W;

				char* destaddr = allocateVirtual4kPage(sizemem,&(phdr->p_vaddr), prop);
				if (phdr->p_filesz != 0)
					memcpy_s(destaddr, elfData+ phdr->p_offset, phdr->p_filesz);	
			}
		}
		ephoff += elfhead->e_phentsize;
	 }
}
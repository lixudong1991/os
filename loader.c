#include "loader.h"


void execPrograma(uint32 startSection)
{
	char buff[512];
	read_sectors(buff, startSection, 1);
	Elf32_Ehdr* elfhead = buff;

	uint32 phoff = elfhead->e_phoff;
	uint32 phnum = elfhead->e_phnum;
	uint32 phSize = phnum * elfhead->e_phentsize;
	 

	uint32 readse = (phoff + phSize - 512) / 512;

	if ((phoff + phSize - 512) % 512!=0)
		readse++;
	read_sectors(addr+512, startSection+1, readse);
	Elf32_Phdr* phhead = addr + phoff;
	
	for (int i=0;i< phnum;i++)
	{
		 
	}

}
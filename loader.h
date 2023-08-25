#ifndef  LOADER_H_H
#define  LOADER_H_H
#include "boot.h"


#define EI_NIDENT (16)

typedef struct 
{
	unsigned char	e_ident[EI_NIDENT];	/* Magic number and other info */
	uint16	e_type;			/* Object file type */
	uint16	e_machine;		/* Architecture */
	uint32	e_version;		/* Object file version */
	uint32	e_entry;		/* Entry point virtual address */
	uint32	e_phoff;		/* Program header table file offset */
	uint32	e_shoff;		/* Section header table file offset */
	uint32	e_flags;		/* Processor-specific flags */
	uint16	e_ehsize;		/* ELF header size in bytes */
	uint16	e_phentsize;		/* Program header table entry size */
	uint16	e_phnum;		/* Program header table entry count */
	uint16	e_shentsize;		/* Section header table entry size */
	uint16	e_shnum;		/* Section header table entry count */
	uint16	e_shstrndx;		/* Section header string table index */
} Elf32_Ehdr;

typedef struct
{
	uint32	sh_name;		/* Section name (string tbl index) */
	uint32	sh_type;		/* Section type */
	uint32	sh_flags;		/* Section flags */
	uint32	sh_addr;		/* Section virtual addr at execution */
	uint32	sh_offset;		/* Section file offset */
	uint32	sh_size;		/* Section size in bytes */
	uint32	sh_link;		/* Link to another section */
	uint32	sh_info;		/* Additional section information */
	uint32	sh_addralign;		/* Section alignment */
	uint32	sh_entsize;		/* Entry size if section holds table */
} Elf32_Shdr;

typedef struct
{
	uint32	p_type;			/* Segment type */
	uint32	p_offset;		/* Segment file offset */
	uint32	p_vaddr;		/* Segment virtual address */
	uint32	p_paddr;		/* Segment physical address */
	uint32	p_filesz;		/* Segment size in file */
	uint32	p_memsz;		/* Segment size in memory */
	uint32	p_flags;		/* Segment flags */
	uint32	p_align;		/* Segment alignment */
} Elf32_Phdr;

#endif // ! LOADER_H_H

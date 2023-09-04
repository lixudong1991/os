#include "memcachectl.h"
#include "boot.h"
#include "printf.h"
#include "cpufeature.h"

#define val_str(a)  ((a)?"enable":"disable")
#define vall_str(a)  ((a)?"support":"not support")
void check_mtrr()
{
    uint32_t eax=0,ebx=0,ecx=0,edx=0;
	if (cpufeatures[cpu_support_mtrr])
	{
		eax = edx = 0;
		rdmsrcall(IA32_MTRRCAP_MSR, &eax, &edx);
		printf("mtrr fix range %s,WC memory %s, SMRR %s, variable rang count:%d\r\n", vall_str(eax&(1<<8)), vall_str(eax&(1<<10)), vall_str(eax&(1<<11)),eax&0xff);
        uint32_t vcnt = eax&0xff;
		eax = edx = 0;
		rdmsrcall(IA32_MTRR_DEF_TYPE_MSR, &eax, &edx);
		uint32 mtrrenable = eax & (1 << 11);
        uint32 fixenable = eax & (1 << 10);
		printf(" MTRR %s, Fixed-range %s, Default mem Type = %d\r\n", val_str(mtrrenable), val_str(fixenable), eax & 0xff);
		cpuidcall(0x80000008, &eax, &ebx, &ecx, &edx);
		printf("MAXPHYADDR: eax=0x%x\r\n", eax);
		if (mtrrenable)
		{
			for (int i = 0; i < vcnt; i++)
			{
				eax = ebx = ecx = edx = 0;
				rdmsrcall(IA32_MTRR_PHYSBASE_ADDR(i), &eax, &ebx);
				rdmsrcall(IA32_MTRR_PHYSMASK_ADDR(i), &ecx, &edx);
				printf("variable range %d:physbase = 0x%x 0x%x  physmask = 0x%x 0x%x\r\n", i, ebx, eax, edx, ecx);
			}
		}
        if(fixenable)
        {
		rdmsrcall(IA32_MTRR_FIX64K_00000_MSR,&eax,&edx);
		printf("00000: %x %x  ",edx,eax);
		eax=edx=0;
		rdmsrcall(IA32_MTRR_FIX16K_80000_MSR,&eax,&edx);
		printf("80000: %x %x  ",edx,eax);
		eax=edx=0;
		rdmsrcall(IA32_MTRR_FIX16K_A0000_MSR,&eax,&edx);
		printf("A0000: %x %x  ",edx,eax);
		eax=edx=0;
		rdmsrcall(IA32_MTRR_FIX4K_C0000_MSR,&eax,&edx);
		printf("C0000: %x %x  ",edx,eax);
		eax=edx=0;
		rdmsrcall(IA32_MTRR_FIX4K_C8000_MSR,&eax,&edx);
		printf("C8000: %x %x  ",edx,eax);
		eax=edx=0;
		rdmsrcall(IA32_MTRR_FIX4K_D0000_MSR,&eax,&edx);
		printf("D0000: %x %x  ",edx,eax);
		eax=edx=0;
		rdmsrcall(IA32_MTRR_FIX4K_D8000_MSR,&eax,&edx);
		printf("D8000: %x %x  ",edx,eax);
		eax=edx=0;
		rdmsrcall(IA32_MTRR_FIX4K_E0000_MSR,&eax,&edx);
		printf("E0000: %x %x  ",edx,eax);
		eax=edx=0;
		rdmsrcall(IA32_MTRR_FIX4K_E8000_MSR,&eax,&edx);
		printf("E8000: %x %x  ",edx,eax);
		eax=edx=0;
		rdmsrcall(IA32_MTRR_FIX4K_F0000_MSR,&eax,&edx);
		printf("F0000: %x %x  ",edx,eax);	
		eax=edx=0;
		rdmsrcall(IA32_MTRR_FIX4K_F8000_MSR,&eax,&edx);
		printf("F8000: %x %x\r\n",edx,eax);	
        }											
	}
}
int get_fix_range_type(uint32 base)
{
    char memtypes[8] = {0};
    if(base<0x80000)
    {
        rdmsrcall(IA32_MTRR_FIX64K_00000_MSR,(uint32*)memtypes,(uint32*)(memtypes+4));
        return memtypes[base>>16];
    }
    else if(base < 0xA0000)
    {
        rdmsrcall(IA32_MTRR_FIX16K_80000_MSR,(uint32*)memtypes,(uint32*)(memtypes+4));
        return memtypes[(base-0x80000)>>14];
    }
    else if(base < 0xC0000)
    {
        rdmsrcall(IA32_MTRR_FIX16K_A0000_MSR,(uint32*)memtypes,(uint32*)(memtypes+4));
        return memtypes[(base-0xA0000)>>14];
    }
    else if(base < 0xC8000)
    {
        rdmsrcall(IA32_MTRR_FIX4K_C0000_MSR,(uint32*)memtypes,(uint32*)(memtypes+4));
        return memtypes[(base-0xC0000)>>12];
    }
    else if(base < 0xD0000)
    {
        rdmsrcall(IA32_MTRR_FIX4K_C8000_MSR,(uint32*)memtypes,(uint32*)(memtypes+4));
        return memtypes[(base-0xC8000)>>12];
    }
    else if(base < 0xD8000)
    {
        rdmsrcall(IA32_MTRR_FIX4K_D0000_MSR,(uint32*)memtypes,(uint32*)(memtypes+4));
        return memtypes[(base-0xD0000)>>12];
    }
    else if(base < 0xE0000)
    {
        rdmsrcall(IA32_MTRR_FIX4K_D8000_MSR,(uint32*)memtypes,(uint32*)(memtypes+4));
        return memtypes[(base-0xD8000)>>12];
    }
    else if(base < 0xE8000)
    {
        rdmsrcall(IA32_MTRR_FIX4K_E0000_MSR,(uint32*)memtypes,(uint32*)(memtypes+4));
        return memtypes[(base-0xE0000)>>12];
    }
    else if(base < 0xF0000)
    {
        rdmsrcall(IA32_MTRR_FIX4K_E8000_MSR,(uint32*)memtypes,(uint32*)(memtypes+4));
        return memtypes[(base-0xE8000)>>12];
    }
    else if(base < 0xF8000)
    {
        rdmsrcall(IA32_MTRR_FIX4K_F0000_MSR,(uint32*)memtypes,(uint32*)(memtypes+4));
        return memtypes[(base-0xF0000)>>12];
    }
    else 
    {
        rdmsrcall(IA32_MTRR_FIX4K_F8000_MSR,(uint32*)memtypes,(uint32*)(memtypes+4));
        return memtypes[(base-0xF8000)>>12];
    }
}
int get_4kmem_cache_type(uint32 base)
{
    uint32_t eax=0,edx=0,eax1=0,edx1=0;
    rdmsrcall(IA32_MTRRCAP_MSR, &eax, &edx);
    rdmsrcall(IA32_MTRR_DEF_TYPE_MSR, &eax1, &edx1);
    int defaultType= eax1 & 0xff;
    if( (eax&(1<<8)) && (eax1 & (1 << 10)))
    {
        if(base<0x100000)
            return get_fix_range_type(base);
    }
    uint32_t vcnt = eax&0xff;
    uint64 phybase =0,phymask =0,lbase =0;
    for(uint32 i=0;i<vcnt;i++)
    {
        eax = edx = eax1 = edx1 = 0;
		rdmsrcall(IA32_MTRR_PHYSBASE_ADDR(i), &eax, &edx);
		rdmsrcall(IA32_MTRR_PHYSMASK_ADDR(i), &eax1, &edx1);
        if(eax1 & (1<<11) == 0)
            continue;
        phybase = edx;
        phybase <<=32;
        phybase |= (eax & 0xFFFFF000);
        phymask = edx1;
        phymask <<=32;
        phymask |= (eax1 & 0xFFFFF000);
        lbase = base;
        if(lbase & phymask == phybase & phymask )
            return eax & 0xff;
    }
    return defaultType;
}
int mem_cache_type_get(uint32 base,uint32 size)
{   
    uint32_t eax=0,edx=0;
    if(0xffffffff -base +1 < size)
        size =0xffffffff-base +1;
    uint32 startaddr = base & 0xFFFFF000,endaddr =(base+ size -1)& 0xFFFFF000;
    if(cpufeatures[cpu_support_mtrr])
    {
        rdmsrcall(IA32_MTRR_DEF_TYPE_MSR, &eax, &edx);
        if(eax & (1 << 11) == 0)
            return MEM_UC;
        int firstType = get_4kmem_cache_type(startaddr);
        for(uint32 index =startaddr+0x1000;index<=endaddr;index+=0x1000)
        {
            if(firstType != get_4kmem_cache_type(index))
                return MEM_MIXED;
        }
        return firstType;
    }
    return MEM_UNKNOWN;
}

int mem_cache_type_set(uint32 base,uint32 size,int type)
{
    uint32_t eax=0,edx=0,eax1=0,edx1=0;
    if(cpufeatures[cpu_support_mtrr])
    {
        if(base&0x0fff || size&0x0fff ||(size ==0))
            return 0;
            
        rdmsrcall(IA32_MTRRCAP_MSR, &eax, &edx);
        rdmsrcall(IA32_MTRR_DEF_TYPE_MSR, &eax1, &edx1);
        if(eax&(1<<8) && (eax1 & (1 << 10)))
        {
            if(base<0x100000)
            {
                uint32 cr4data = pre_mtrr_change();

                post_mtrr_change(cr4data);
                return TRUE;
            }
        }
        if( base %size != 0)
            return 0;
            

    }
    return FALSE;
}
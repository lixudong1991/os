#include "memcachectl.h"
#include "boot.h"
#include "printf.h"
#include "cpufeature.h"

#define val_str(a) ((a) ? "enable" : "disable")
#define vall_str(a) ((a) ? "support" : "not support")

uint32 mem_type_map_pat[8] = {0x18, 0x18, 0x18, 0x18, 0x8, 0x18, 0, 0x10}; // WC,WP映射为UC
MtrrMSRinfo mtrrmsrinfo;

#ifdef TRACE_MTRR
#	define TRACEMTRR(...) printf(__VA_ARGS__)
#else
#	define TRACEMTRR(...)
#endif

uint32 IA32_MTRR_PHYSBASE_ADDR(int index)
{
    return IA32_MTRR_PHYSBASE0_MSR + index * 2;
}
uint32 IA32_MTRR_PHYSMASK_ADDR(int index)
{
    return IA32_MTRR_PHYSMASK0_MSR + index * 2;
}
void check_pat()
{
    uint32_t eax = 0, edx = 0;
    if (cpufeatures[cpu_support_pat])
    {
        rdmsrcall(IA32_PAT_MSR, &eax, &edx);
        TRACEMTRR("pat eax:0x%x edx:0x%x\n", eax, edx);
    }
}
void check_mtrr()
{
    uint32_t eax = 0, ebx = 0, ecx = 0, edx = 0;
    if (cpufeatures[cpu_support_mtrr])
    {
        eax = edx = 0;
        rdmsrcall(IA32_MTRRCAP_MSR, &eax, &edx);
        TRACEMTRR("mtrr fix range %s,WC memory %s, SMRR %s, variable rang count:%d\n", vall_str(eax & (1 << 8)), vall_str(eax & (1 << 10)), vall_str(eax & (1 << 11)), eax & 0xff);
        uint32_t vcnt = eax & 0xff;
        eax = edx = 0;
        rdmsrcall(IA32_MTRR_DEF_TYPE_MSR, &eax, &edx);
        uint32 mtrrenable = eax & (1 << 11);
        uint32 fixenable = eax & (1 << 10);
        TRACEMTRR(" MTRR %s, Fixed-range %s, Default mem Type = %d\n", val_str(mtrrenable), val_str(fixenable), eax & 0xff);
        cpuidcall(0x80000008, &eax, &ebx, &ecx, &edx);
        TRACEMTRR("MAXPHYADDR: eax=0x%x\n", eax);

        mtrrmsrinfo.variableMsrCount = vcnt;
        mtrrmsrinfo.fixMsr[0].msrid = IA32_MTRR_FIX64K_00000_MSR;
        mtrrmsrinfo.fixMsr[1].msrid = IA32_MTRR_FIX16K_80000_MSR;
        mtrrmsrinfo.fixMsr[2].msrid = IA32_MTRR_FIX16K_A0000_MSR;
        mtrrmsrinfo.fixMsr[3].msrid = IA32_MTRR_FIX4K_C0000_MSR;
        mtrrmsrinfo.fixMsr[4].msrid = IA32_MTRR_FIX4K_C8000_MSR;
        mtrrmsrinfo.fixMsr[5].msrid = IA32_MTRR_FIX4K_D0000_MSR;
        mtrrmsrinfo.fixMsr[6].msrid = IA32_MTRR_FIX4K_D8000_MSR;
        mtrrmsrinfo.fixMsr[7].msrid = IA32_MTRR_FIX4K_E0000_MSR;
        mtrrmsrinfo.fixMsr[8].msrid = IA32_MTRR_FIX4K_E8000_MSR;
        mtrrmsrinfo.fixMsr[9].msrid = IA32_MTRR_FIX4K_F0000_MSR;
        mtrrmsrinfo.fixMsr[10].msrid = IA32_MTRR_FIX4K_F8000_MSR;
        mtrrmsrinfo.fixMsr[0].startaddr = 0;
        mtrrmsrinfo.fixMsr[1].startaddr = 0x80000;
        mtrrmsrinfo.fixMsr[2].startaddr = 0xA0000;
        mtrrmsrinfo.fixMsr[3].startaddr = 0xC0000;
        mtrrmsrinfo.fixMsr[4].startaddr = 0xC8000;
        mtrrmsrinfo.fixMsr[5].startaddr = 0xD0000;
        mtrrmsrinfo.fixMsr[6].startaddr = 0xD8000;
        mtrrmsrinfo.fixMsr[7].startaddr = 0xE0000;
        mtrrmsrinfo.fixMsr[8].startaddr = 0xE8000;
        mtrrmsrinfo.fixMsr[9].startaddr = 0xF0000;
        mtrrmsrinfo.fixMsr[10].startaddr = 0xF8000;
    }
}
int fix_range_type(uint32 base, int type)
{
    char memtypes[8] = {0};
    int ret = 0;
    if (base < 0x80000)
    {
        rdmsr_fence(IA32_MTRR_FIX64K_00000_MSR, (uint32 *)memtypes, (uint32 *)(memtypes + 4));
        ret = memtypes[base >> 16];
        if (type == MEM_UC || type == MEM_WC || type == MEM_WT || type == MEM_WP || type == MEM_WB || type == MEM_UC1)
        {
            if (ret != type)
            {
                memtypes[base >> 16] = (char)type;
                wrmsr_fence(IA32_MTRR_FIX64K_00000_MSR, *(uint32 *)memtypes, *(uint32 *)(memtypes + 4));
            }
        }
    }
    else if (base < 0xA0000)
    {
        rdmsr_fence(IA32_MTRR_FIX16K_80000_MSR, (uint32 *)memtypes, (uint32 *)(memtypes + 4));
        ret = memtypes[(base - 0x80000) >> 14];
        if (type == MEM_UC || type == MEM_WC || type == MEM_WT || type == MEM_WP || type == MEM_WB || type == MEM_UC1)
        {
            if (ret != type)
            {
                memtypes[(base - 0x80000) >> 14] = (char)type;
                wrmsr_fence(IA32_MTRR_FIX16K_80000_MSR, *(uint32 *)memtypes, *(uint32 *)(memtypes + 4));
            }
        }
    }
    else if (base < 0xC0000)
    {
        rdmsr_fence(IA32_MTRR_FIX16K_A0000_MSR, (uint32 *)memtypes, (uint32 *)(memtypes + 4));
        ret = memtypes[(base - 0xA0000) >> 14];
        if (type == MEM_UC || type == MEM_WC || type == MEM_WT || type == MEM_WP || type == MEM_WB || type == MEM_UC1)
        {
            if (ret != type)
            {
                memtypes[(base - 0xA0000) >> 14] = (char)type;
                wrmsr_fence(IA32_MTRR_FIX16K_A0000_MSR, *(uint32 *)memtypes, *(uint32 *)(memtypes + 4));
            }
        }
    }
    else if (base < 0xC8000)
    {
        rdmsr_fence(IA32_MTRR_FIX4K_C0000_MSR, (uint32 *)memtypes, (uint32 *)(memtypes + 4));
        ret = memtypes[(base - 0xC0000) >> 12];
        if (type == MEM_UC || type == MEM_WC || type == MEM_WT || type == MEM_WP || type == MEM_WB || type == MEM_UC1)
        {
            if (ret != type)
            {
                memtypes[(base - 0xC0000) >> 12] = (char)type;
                wrmsr_fence(IA32_MTRR_FIX4K_C0000_MSR, *(uint32 *)memtypes, *(uint32 *)(memtypes + 4));
            }
        }
    }
    else if (base < 0xD0000)
    {
        rdmsr_fence(IA32_MTRR_FIX4K_C8000_MSR, (uint32 *)memtypes, (uint32 *)(memtypes + 4));
        ret = memtypes[(base - 0xC8000) >> 12];
        if (type == MEM_UC || type == MEM_WC || type == MEM_WT || type == MEM_WP || type == MEM_WB || type == MEM_UC1)
        {
            if (ret != type)
            {
                memtypes[(base - 0xC8000) >> 12] = (char)type;
                wrmsr_fence(IA32_MTRR_FIX4K_C8000_MSR, *(uint32 *)memtypes, *(uint32 *)(memtypes + 4));
            }
        }
    }
    else if (base < 0xD8000)
    {
        rdmsr_fence(IA32_MTRR_FIX4K_D0000_MSR, (uint32 *)memtypes, (uint32 *)(memtypes + 4));
        ret = memtypes[(base - 0xD0000) >> 12];
        if (type == MEM_UC || type == MEM_WC || type == MEM_WT || type == MEM_WP || type == MEM_WB || type == MEM_UC1)
        {
            if (ret != type)
            {
                memtypes[(base - 0xD0000) >> 12] = (char)type;
                wrmsr_fence(IA32_MTRR_FIX4K_D0000_MSR, *(uint32 *)memtypes, *(uint32 *)(memtypes + 4));
            }
        }
    }
    else if (base < 0xE0000)
    {
        rdmsr_fence(IA32_MTRR_FIX4K_D8000_MSR, (uint32 *)memtypes, (uint32 *)(memtypes + 4));
        ret = memtypes[(base - 0xD8000) >> 12];
        if (type == MEM_UC || type == MEM_WC || type == MEM_WT || type == MEM_WP || type == MEM_WB || type == MEM_UC1)
        {
            if (ret != type)
            {
                memtypes[(base - 0xD8000) >> 12] = (char)type;
                wrmsr_fence(IA32_MTRR_FIX4K_D8000_MSR, *(uint32 *)memtypes, *(uint32 *)(memtypes + 4));
            }
        }
    }
    else if (base < 0xE8000)
    {
        rdmsr_fence(IA32_MTRR_FIX4K_E0000_MSR, (uint32 *)memtypes, (uint32 *)(memtypes + 4));
        ret = memtypes[(base - 0xE0000) >> 12];
        if (type == MEM_UC || type == MEM_WC || type == MEM_WT || type == MEM_WP || type == MEM_WB || type == MEM_UC1)
        {
            if (ret != type)
            {
                memtypes[(base - 0xE0000) >> 12] = (char)type;
                wrmsr_fence(IA32_MTRR_FIX4K_E0000_MSR, *(uint32 *)memtypes, *(uint32 *)(memtypes + 4));
            }
        }
    }
    else if (base < 0xF0000)
    {
        rdmsr_fence(IA32_MTRR_FIX4K_E8000_MSR, (uint32 *)memtypes, (uint32 *)(memtypes + 4));
        ret = memtypes[(base - 0xE8000) >> 12];
        if (type == MEM_UC || type == MEM_WC || type == MEM_WT || type == MEM_WP || type == MEM_WB || type == MEM_UC1)
        {
            if (ret != type)
            {
                memtypes[(base - 0xE8000) >> 12] = (char)type;
                wrmsr_fence(IA32_MTRR_FIX4K_E8000_MSR, *(uint32 *)memtypes, *(uint32 *)(memtypes + 4));
            }
        }
    }
    else if (base < 0xF8000)
    {
        rdmsr_fence(IA32_MTRR_FIX4K_F0000_MSR, (uint32 *)memtypes, (uint32 *)(memtypes + 4));
        ret = memtypes[(base - 0xF0000) >> 12];
        if (type == MEM_UC || type == MEM_WC || type == MEM_WT || type == MEM_WP || type == MEM_WB || type == MEM_UC1)
        {
            if (ret != type)
            {
                memtypes[(base - 0xF0000) >> 12] = (char)type;
                wrmsr_fence(IA32_MTRR_FIX4K_F0000_MSR, *(uint32 *)memtypes, *(uint32 *)(memtypes + 4));
            }
        }
    }
    else
    {
        rdmsr_fence(IA32_MTRR_FIX4K_F8000_MSR, (uint32 *)memtypes, (uint32 *)(memtypes + 4));
        ret = memtypes[(base - 0xF8000) >> 12];
        if (type == MEM_UC || type == MEM_WC || type == MEM_WT || type == MEM_WP || type == MEM_WB || type == MEM_UC1)
        {
            if (ret != type)
            {
                memtypes[(base - 0xF8000) >> 12] = (char)type;
                wrmsr_fence(IA32_MTRR_FIX4K_F8000_MSR, *(uint32 *)memtypes, *(uint32 *)(memtypes + 4));
            }
        }
    }
    return ret;
}
int get_4kmem_cache_type(uint32 base)
{
    uint32_t eax = 0, edx = 0, eax1 = 0, edx1 = 0;
    rdmsrcall(IA32_MTRRCAP_MSR, &eax, &edx);
    rdmsrcall(IA32_MTRR_DEF_TYPE_MSR, &eax1, &edx1);
    int defaultType = eax1 & 0xff;
    if ((eax & (1 << 8)) && (eax1 & (1 << 10)))
    {
        if (base < 0x100000)
            return fix_range_type(base, MEM_UNKNOWN);
    }
    uint32_t vcnt = eax & 0xff;
    uint64 phybase = 0, phymask = 0, lbase = 0;
    for (uint32 i = 0; i < vcnt; i++)
    {
        eax = edx = eax1 = edx1 = 0;
        rdmsrcall(IA32_MTRR_PHYSBASE_ADDR(i), &eax, &edx);
        rdmsrcall(IA32_MTRR_PHYSMASK_ADDR(i), &eax1, &edx1);
        if (eax1 & (1 << 11) == 0)
            continue;
        phybase = edx;
        phybase <<= 32;
        phybase |= (eax & 0xFFFFF000);
        phymask = edx1;
        phymask <<= 32;
        phymask |= (eax1 & 0xFFFFF000);
        lbase = base;

        if ((lbase & phymask) == (phybase & phymask))
        {
            // edx1 = (uint32_t)&phybase;
            // edx = (uint32_t)&phymask;
            //  eax1= (uint32_t)&lbase;
            //  TRACEMTRR("index =%d basel:0x%x 0x%x maskl:0x%x 0x%x addl:0x%x 0x%x\n",i, *(uint32 *)edx1, *(uint32 *)(edx1 + 4), *(uint32 *)edx, *(uint32 *)(edx + 4),*(uint32 *)eax1, *(uint32 *)(eax1 + 4));
            return (eax & 0xff);
        }
    }
    return defaultType;
}
int mem_cache_type_get(uint32 base, uint32 size)
{
    uint32_t eax = 0, edx = 0;
    if (0xffffffff - base + 1 < size)
        size = 0xffffffff - base + 1;
    uint32 startaddr = base & 0xFFFFF000, endaddr = (base + size - 1) & 0xFFFFF000;
    if (cpufeatures[cpu_support_mtrr])
    {
        rdmsrcall(IA32_MTRR_DEF_TYPE_MSR, &eax, &edx);
        if (eax & (1 << 11) == 0)
            return MEM_UC;
        int firstType = get_4kmem_cache_type(startaddr);
        for (uint32 index = startaddr + 0x1000; index <= endaddr; index += 0x1000)
        {
            if (firstType != get_4kmem_cache_type(index))
                return MEM_MIXED;
        }
        return firstType;
    }
    return MEM_UNKNOWN;
}

int updataFixRange(uint32 base, uint32 size, int type)
{
    uint32 endaddr = (base + size - 1) & 0xFFFFF000;
    for (uint32 i = base; i <= endaddr; i += 0x1000)
    {
        fix_range_type(base, type);
    }
}
int mem_fix_type_set(uint32 base, uint32 size, int type)
{
    uint32_t eax = 0, edx = 0, eax1 = 0, edx1 = 0;
    if (cpufeatures[cpu_support_mtrr])
    {
        if ((base & 0x0fff) || (size & 0x0fff) || (size == 0))
            return FALSE;
        rdmsrcall(IA32_MTRRCAP_MSR, &eax, &edx);
        rdmsrcall(IA32_MTRR_DEF_TYPE_MSR, &eax1, &edx1);
        if ((eax & (1 << 8)) && (eax1 & (1 << 10)))
        {
            if (base < 0x100000)
            {
                asm("cli");
                uint32 cr4data = pre_mtrr_change();
                updataFixRange(base, size, type);
                post_mtrr_change(cr4data);
                asm("sti");
                return TRUE;
            }
        }
    }
    return FALSE;
}

int mem_variable_type_set(int msrId, uint32 base, uint32 size, int type)
{
    uint32_t eax = 0, edx = 0, eax1 = 0, edx1 = 0;
    if (cpufeatures[cpu_support_mtrr])
    {
        if ((base & 0x0fff) || (size & 0x0fff) || (size == 0))
            return FALSE;
        rdmsrcall(IA32_MTRRCAP_MSR, &eax, &edx);
        rdmsrcall(IA32_MTRR_DEF_TYPE_MSR, &eax1, &edx1);
        if ((eax & (1 << 8)) && (eax1 & (1 << 10)))
        {
            if (base < 0x100000)
                return FALSE;
        }
        if (base % size != 0)
            return FALSE;
        uint32_t vcnt = eax & 0xff;
        if (vcnt == 0 || msrId < 0 || msrId > (vcnt - 1))
            return FALSE;
        uint64 phybase = base, phymask = base + size;
        phybase |= (type & 0xff);

        eax = edx = eax1 = edx1 = 0;
        cpuidcall(0x80000008, &eax, &edx, &eax1, &edx1);

        uint32_t maxindex = eax & 0xff;
        uint32_t index = 12;
        eax = (uint32_t)&phybase;
        edx = (uint32_t)&phymask;
        eax1 = 0;
        // TRACEMTRR("eax:0x%x edx:0x%x eax1:0x%x edx1:0x%x\n",*(uint32*)eax,*(uint32*)(eax+4),*(uint32*)edx,*(uint32*)(edx+4));
        for (; index < maxindex; index++)
        {
            if (!eax1)
            {
                if (phymask & (((uint64)1) << index))
                    eax1 = 1;
            }
            if (eax1)
                phymask |= (((uint64)1) << index);
        }
        phymask |= (((uint64)1) << 11);
        // TRACEMTRR("eax:0x%x edx:0x%x eax1:0x%x edx1:0x%x\n", *(uint32 *)eax, *(uint32 *)(eax + 4), *(uint32 *)edx, *(uint32 *)(edx + 4));
        asm("cli");
        uint32 cr4data = pre_mtrr_change();
        wrmsr_fence(IA32_MTRR_PHYSBASE_ADDR(msrId), *(uint32 *)eax, *(uint32 *)(eax + 4));
        wrmsr_fence(IA32_MTRR_PHYSMASK_ADDR(msrId), *(uint32 *)edx, *(uint32 *)(edx + 4));
        post_mtrr_change(cr4data);
        asm("sti");
        return TRUE;
    }
    return FALSE;
}

void cacheMtrrMsrs()
{

    for (int i = 0; i < mtrrmsrinfo.variableMsrCount; i++)
    {
        rdmsrcall(IA32_MTRR_PHYSBASE_ADDR(i), &(mtrrmsrinfo.variableData[i].baselow), &(mtrrmsrinfo.variableData[i].basehigh));
        rdmsrcall(IA32_MTRR_PHYSMASK_ADDR(i), &(mtrrmsrinfo.variableData[i].masklow), &(mtrrmsrinfo.variableData[i].maskhign));
        // interruptPrintf("variable range %d:physbase = 0x%x 0x%x  physmask = 0x%x 0x%x\n", i, mtrrmsrinfo.variableData[i].basehigh,
        //        mtrrmsrinfo.variableData[i].baselow, mtrrmsrinfo.variableData[i].maskhign, mtrrmsrinfo.variableData[i].masklow);
    }

    for (int i = 0; i < 11; i++)
    {
        rdmsrcall(mtrrmsrinfo.fixMsr[i].msrid, &(mtrrmsrinfo.fixMsr[i].datalow), &(mtrrmsrinfo.fixMsr[i].datahigh));
        // interruptPrintf("%x: %x %x  ", mtrrmsrinfo.fixMsr[i].startaddr, mtrrmsrinfo.fixMsr[i].datahigh, mtrrmsrinfo.fixMsr[i].datalow);
    }
}

void refreshMtrrMsrs()
{
    uint32 cr4data = pre_mtrr_change();
    for (int i = 0; i < mtrrmsrinfo.variableMsrCount; i++)
    {
        wrmsr_fence(IA32_MTRR_PHYSBASE_ADDR(i), (mtrrmsrinfo.variableData[i].baselow), (mtrrmsrinfo.variableData[i].basehigh));
        wrmsr_fence(IA32_MTRR_PHYSMASK_ADDR(i), (mtrrmsrinfo.variableData[i].masklow), (mtrrmsrinfo.variableData[i].maskhign));
    }
    for (int i = 0; i < 11; i++)
    {
        wrmsr_fence(mtrrmsrinfo.fixMsr[i].msrid, mtrrmsrinfo.fixMsr[i].datalow, mtrrmsrinfo.fixMsr[i].datahigh);
    }
    post_mtrr_change(cr4data);
}

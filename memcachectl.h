#ifndef MEM_CACHE_CTL__H
#define MEM_CACHE_CTL__H
#include "stdint.h"
#define CPUID_SUPPORT_EDX_MTRR 0x1000
#define CPUID_SUPPORT_EDX_PAT 0x10000

#define IA32_MTRRCAP_MSR 0xFE
#define IA32_MTRR_DEF_TYPE_MSR 0x2FF

#define IA32_MTRR_FIX64K_00000_MSR 0x250 // MTRRfix64K_00000
#define IA32_MTRR_FIX16K_80000_MSR 0x258 // MTRRfix16K_80000
#define IA32_MTRR_FIX16K_A0000_MSR 0x259 // MTRRfix16K_A0000
#define IA32_MTRR_FIX4K_C0000_MSR 0x268  // MTRRfix4K_C0000
#define IA32_MTRR_FIX4K_C8000_MSR 0x269  // MTRRfix4K_C8000
#define IA32_MTRR_FIX4K_D0000_MSR 0x26A  // MTRRfix4K_D0000
#define IA32_MTRR_FIX4K_D8000_MSR 0x26B  // MTRRfix4K_D8000
#define IA32_MTRR_FIX4K_E0000_MSR 0x26C  // MTRRfix4K_E0000
#define IA32_MTRR_FIX4K_E8000_MSR 0x26D  // MTRRfix4K_E8000
#define IA32_MTRR_FIX4K_F0000_MSR 0x26E  // MTRRfix4K_F0000
#define IA32_MTRR_FIX4K_F8000_MSR 0x26F  // MTRRfix4K_F8000

#define IA32_PAT_MSR 0x277 //(R/W)

#define IA32_MTRR_PHYSBASE0_MSR 0x200
#define IA32_MTRR_PHYSMASK0_MSR 0x201

#define MEM_UC 0
#define MEM_WC 1
#define MEM_WT 4
#define MEM_WP 5
#define MEM_WB 6
#define MEM_UC1 7 // UC-

#define MEM_UNKNOWN -2
#define MEM_MIXED -1

extern uint32 mem_type_map_pat[8];//映射内存缓存类型为PAT表中相同内存缓存类型的PCD和PWD值，例如当PAT3的内存类型为UC时，则mem_type_map_pat[MEM_UC] =0x18,此时PCD=PWD=1(0x18第3和第4位为1)，2*PCD+PWD =3，PAT[3]=UC

typedef struct MTRR_FIX_MSR{
    uint32 msrid;
    uint32 datalow;
    uint32 datahigh;
    uint32 startaddr;
}MTRR_FIX_MSR;

typedef struct MTRR_Variable_MSR{
    uint32 baselow;
    uint32 basehigh;
    uint32 masklow;
    uint32 maskhign;
}MTRR_Variable_MSR;

#define MAX_VARIABLE_MTRR_MSR_COUNT  10

typedef struct MtrrMSRinfo{
    uint32 variableMsrCount;
    MTRR_Variable_MSR variableData[MAX_VARIABLE_MTRR_MSR_COUNT];
    MTRR_FIX_MSR fixMsr[11];
}MtrrMSRinfo;

extern MtrrMSRinfo mtrrmsrinfo;

void check_mtrr();
void check_pat();
#define MIXED_TYPES -1
int mem_cache_type_get(uint32 base, uint32 size);
int mem_fix_type_set(uint32 base, uint32 size, int type);
int mem_variable_type_set(int msrId,uint32 base, uint32 size, int type);

void cacheMtrrMsrs();
void refreshMtrrMsrs();
#endif
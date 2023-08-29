#include "apic.h"
#include "boot.h"

int cpuinfo()
{
   uint32_t eax=0,ebx=0,ecx=0,edx=0;
   cpuidcall(1, &eax, &ebx,&ecx,&edx);
   return eax;
}

int check_apic()
{
   uint32_t eax=0,ebx=0,ecx=0,edx=0;
   cpuidcall(1, &eax, &ebx,&ecx,&edx);
   return edx & CPUID_FEAT_EDX_APIC; 
}
int check_x2apic()
{
   uint32_t eax=0,ebx=0,ecx=0,edx=0;
   cpuidcall(1, &eax, &ebx,&ecx,&edx);
   return ecx & CPUID_SUPPORT_ECX_x2APIC;   
}

int check_apic_timer_tscdeadline()
{
   uint32_t eax=0,ebx=0,ecx=0,edx=0;
   cpuidcall(1, &eax, &ebx,&ecx,&edx);
   return ecx & CPUID_APIC_TIMER_TSCDEADLINE;       

}
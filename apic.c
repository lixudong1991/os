#include "apic.h"
#include "boot.h"

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
int enablingx2APIC()
{
   uint32 eax=0,edx=0;
   if(check_x2apic() == CPUID_SUPPORT_ECX_x2APIC)
   {
      rdmsr_fence(IA32_APIC_BASE_MSR,&eax,&edx);
      eax |= 0xC00;
      wrmsr_fence(IA32_APIC_BASE_MSR,eax,edx);
      eax=0,edx=0;   
      rdmsr_fence(IA32_X2APIC_SIVR,&eax,&edx);
      eax |= 0x100;
      wrmsr_fence(IA32_X2APIC_SIVR,eax,edx);
      return TRUE;
   }
   return FALSE;
}
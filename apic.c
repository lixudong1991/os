#include "apic.h"
#include "boot.h"
#include "cpufeature.h"
int enablingx2APIC()
{
   uint32 eax = 0, edx = 0;
   if (cpufeatures[cpu_support_x2apic])
   {
      rdmsr_fence(IA32_APIC_BASE_MSR, &eax, &edx);
      eax |= 0xC00;
      wrmsr_fence(IA32_APIC_BASE_MSR, eax, edx);
      eax = 0, edx = 0;
      rdmsr_fence(IA32_X2APIC_SIVR, &eax, &edx);
      eax |= 0x100;
      wrmsr_fence(IA32_X2APIC_SIVR, eax, edx);
      return TRUE;
   }
   return FALSE;
}
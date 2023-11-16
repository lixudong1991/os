#include "cpufeature.h"
#include "boot.h"
#include "apic.h"
#include "memcachectl.h"
#include "printf.h"
#include "vbe.h"
int cpufeatures[cpu_feature_size] = {0};
void check_cpu_features()
{
	uint32_t eax = 0, ebx = 0, ecx = 0, edx = 0;
	cpuidcall(1, &eax, &ebx, &ecx, &edx);
	printf("cpuid[1] EAX:0x%x EBX:0x%x ECX:0x%x EDX:0x%x\n", eax, ebx, ecx, edx);
	if (edx & (1 << 25))
		cpufeatures[cpu_support_sse] = 1;
	if (edx & (1 << 26))
		cpufeatures[cpu_support_sse2] = 1;
	if (ecx & 1)
		cpufeatures[cpu_support_sse3] = 1;
	if (ecx & (1 << 9))
		cpufeatures[cpu_support_ssse3] = 1;
	if (edx & (1 << 24))
		cpufeatures[cpu_support_fxsave_fxrstor] = 1;
	if (edx & (1 << 19))
		cpufeatures[cpu_support_clflush] = 1;
	if (edx & CPUID_SUPPORT_EDX_MTRR)
		cpufeatures[cpu_support_mtrr] = 1;
	if (ecx & (1 << 3))
		cpufeatures[cpu_support_monitor_mwait] = 1;
	if (edx & CPUID_FEAT_EDX_APIC)
		cpufeatures[cpu_support_apic] = 1;
	if (ecx & CPUID_SUPPORT_ECX_x2APIC)
		cpufeatures[cpu_support_x2apic] = 1;
	if (ecx & CPUID_APIC_TIMER_TSCDEADLINE)
		cpufeatures[cpu_support_tscdeadline] = 1;

	if (edx & CPUID_SUPPORT_EDX_PAT)
		cpufeatures[cpu_support_pat] = 1;
	printf("support: SSE=%d, SSE2=%d, SSE3=%d, SSSE3=%d, FXSAVE/FXRSTOR=%d, CLFLUSH=%d, apic =%d, x2apic =%d, tscdeadline =%d, mtrr =%d\n, PAT= %d\n",
		   cpufeatures[cpu_support_sse],
		   cpufeatures[cpu_support_sse2],
		   cpufeatures[cpu_support_sse3],
		   cpufeatures[cpu_support_ssse3],
		   cpufeatures[cpu_support_fxsave_fxrstor],
		   cpufeatures[cpu_support_clflush],
		   cpufeatures[cpu_support_apic],
		   cpufeatures[cpu_support_x2apic],
		   cpufeatures[cpu_support_tscdeadline],
		   cpufeatures[cpu_support_mtrr],
		   cpufeatures[cpu_support_pat]);
	printf("support:monitor/mwait = %d\n", cpufeatures[cpu_support_monitor_mwait]);
	if (cpufeatures[cpu_support_monitor_mwait])
	{
		cpuidcall(5, &eax, &ebx, &ecx, &edx);
		printf("cpuid[5] EAX:0x%x EBX:0x%x smallsize:0x%x largestsize:0x%x\n", eax, ebx, ecx & 0x0000ffff, edx & 0x0000ffff);
	}
}
void setCpuHwp()
{
	uint32_t eax = 0, ebx = 0, ecx = 0, edx = 0;
	cpuidcall(6, &eax, &ebx, &ecx, &edx);
	if (eax & 0x80)
	{
		wrmsrcall(IA32_PM_ENABLE, 1, 0);
		if (eax & 0x400)
		{
			//设置cpu为性能优先 Minimum_Performance = Maximum_Performance = 4000MHZ   Desired_Performance =4000MHZ
			wrmsrcall(IA32_HWP_REQUEST, 0x282828, edx);
		}
	}
}
void check_cpuHwp()
{
	uint32_t eax = 0, ebx = 0, ecx = 0, edx = 0;
	cpuidcall(6, &eax, &ebx, &ecx, &edx);
	consolePrintf("cpuid call 6  eax:0x%x ecx:0x%x\n",eax, ecx);
	uint32_t temp = eax;
	if (ecx & 8)
	{
		rdmsrcall(IA32_ENERGY_PERF_BIAS, &eax, &edx);
		consolePrintf("IA32_ENERGY_PERF_BIAS  eax:0x%x edx:0x%x\n", eax, edx);
	}
	 
	if (temp &0x80)
	{
		rdmsrcall(IA32_PM_ENABLE, &eax, &edx);
		consolePrintf("IA32_PM_ENABLE  eax:0x%x edx:0x%x\n", eax, edx);
		rdmsrcall(IA32_HWP_CAPABILITIES, &eax, &edx);
		consolePrintf("IA32_HWP_CAPABILITIES  eax:0x%x edx:0x%x\n", eax, edx);
		rdmsrcall(IA32_HWP_REQUEST, &eax, &edx);
		consolePrintf("IA32_HWP_REQUEST  eax:0x%x edx:0x%x\n", eax, edx);
		rdmsrcall(IA32_HWP_STATUS, &eax, &edx);
		consolePrintf("IA32_HWP_STATUS  eax:0x%x edx:0x%x\n", eax, edx);
		
		//rdmsrcall(IA32_THERM_STATUS, &eax, &edx);
		//consolePrintf("IA32_THERM_STATUS  eax:0x%x edx:0x%x\n", eax, edx);
		//rdmsrcall(MSR_PPERF, &eax, &edx);
		//consolePrintf("MSR_PPERF  eax:0x%x edx:0x%x\n", eax, edx);
	}
	if (temp &0x400000)
	{
		rdmsrcall(IA32_HWP_CTL, &eax, &edx);
		consolePrintf("IA32_HWP_CTL  eax:0x%x edx:0x%x\n", eax, edx);
	}
	if (temp & 0x10000)
	{
		rdmsrcall(IA32_HWP_PECI_REQUEST_INFO, &eax, &edx);
		consolePrintf("IA32_HWP_PECI_REQUEST_INFO  eax:0x%x edx:0x%x\n", eax, edx);
	}
	if (temp & 0x100)
	{
		rdmsrcall(IA32_HWP_INTERRUPT, &eax, &edx);
		consolePrintf("IA32_HWP_INTERRUPT  eax:0x%x edx:0x%x\n", eax, edx);
	}
	if (temp & 0x800)
	{
		rdmsrcall(IA32_HWP_REQUEST_PKG, &eax, &edx);
		consolePrintf("IA32_HWP_REQUEST_PKG  eax:0x%x edx:0x%x\n", eax, edx);
	}
}

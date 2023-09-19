#include "cpufeature.h"
#include "boot.h"
#include "apic.h"
#include "memcachectl.h"
#include "printf.h"
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
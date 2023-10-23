#ifndef MEM_TYPE_H__H
#define MEM_TYPE_H__H

enum cpu_feature_index
{
    cpu_support_sse = 0,
    cpu_support_sse2,
    cpu_support_sse3,
    cpu_support_ssse3,
    cpu_support_fxsave_fxrstor,
    cpu_support_clflush,
    cpu_support_mtrr,
    cpu_support_monitor_mwait,
    cpu_support_apic,
    cpu_support_x2apic,
    cpu_support_tscdeadline,
    cpu_support_pat,
    cpu_feature_size
};

extern int cpufeatures[cpu_feature_size];

void check_cpu_features();

#define IA32_ENERGY_PERF_BIAS 0x1b0

#define IA32_PM_ENABLE 0x770// IA32_PM_ENABLE Enable/Disable HWP.
#define IA32_HWP_CAPABILITIES 0x771// IA32_HWP_CAPABILITIES Enumerates the HWP performance range (static and dynamic).
#define IA32_HWP_REQUEST_PKG 0x772// IA32_HWP_REQUEST_PKG Conveys OSPM's control hints (Min, Max, Activity Window, Energy Performance Preference, Desired) for all logical processor in the physical package.
#define IA32_HWP_INTERRUPT 0x773// IA32_HWP_INTERRUPT Controls HWP native interrupt generation (Guaranteed Performance changes, excursions).
#define IA32_HWP_REQUEST  0x774// IA32_HWP_REQUEST Conveys OSPM's control hints (Min, Max, Activity Window, Energy Performance Preference, Desired) for a single logical processor.
#define IA32_HWP_CTL 0x776

#define IA32_HWP_PECI_REQUEST_INFO 0x775// IA32_HWP_PECI_REQUEST_INFO Conveys embedded system controller requests to override some of the OS HWP Request settings via the PECI mechanism.
#define IA32_HWP_STATUS 0x777// IA32_HWP_STATUS Status bits indicating changes to Guaranteed Performance and excursions to Minimum Performance.
#define IA32_THERM_STATUS 0x19C// IA32_THERM_STATUS[bits 15:12] Conveys reasons for performance excursions.
#define MSR_PPERF 0x64E// MSR_PPERF Productive Performance Count.
void check_cpuHwp();
#endif
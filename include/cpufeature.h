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

#endif
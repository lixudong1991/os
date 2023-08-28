#ifndef _APIC_H_HH
#define _APIC_H_HH
#include "boot.h"


typedef struct LOCAL_APIC
{
    uint32 Reserved0[4]; //Reserved
    uint32 Reserved1[4]; //Reserved
    uint32 ID[4]; //Local APIC ID Register Read/Write.
    uint32 Version[4]; //Local APIC Version Register Read Only.
    uint32 Reserved2[4]; //Reserved
    uint32 Reserved3[4]; //Reserved
    uint32 Reserved4[4]; //Reserved
    uint32 Reserved5[4]; //Reserved
    uint32 TPR[4]; //Task Priority Register (TPR) Read/Write.
    uint32 APR[4]; //Arbitration Priority Register1 (APR) Read Only.
    uint32 PPR[4]; //Processor Priority Register (PPR) Read Only.
    uint32 EOI[4]; //EOI Register Write Only.
    uint32 RRD[4]; //Remote Read Register1 (RRD) Read Only
    uint32 LDR[4]; //Logical Destination Register Read/Write.
    uint32 DFR[4]; //Destination Format Register Read/Write (see Section10.6.2.2).
    uint32 SIV[4]; //Spurious Interrupt Vector Register Read/Write (see Section 10.9.
    uint32 ISR0[4]; //In-Service Register (ISR); bits 31:0 Read Only.
    uint32 ISR1[4]; //In-Service Register (ISR); bits 63:32 Read Only.
    uint32 ISR2[4]; //In-Service Register (ISR); bits 95:64 Read Only.
    uint32 ISR3[4]; //In-Service Register (ISR); bits 127:96 Read Only.
    uint32 ISR4[4]; //In-Service Register (ISR); bits 159:128 Read Only.
    uint32 ISR5[4]; //In-Service Register (ISR); bits 191:160 Read Only.
    uint32 ISR6[4]; //In-Service Register (ISR); bits 223:192 Read Only.
    uint32 ISR7[4]; //In-Service Register (ISR); bits 255:224 Read Only.
    uint32 TMR0[4]; //Trigger Mode Register (TMR); bits 31:0 Read Only.
    uint32 TMR1[4]; //Trigger Mode Register (TMR); bits 63:32 Read Only.
    uint32 TMR2[4]; //Trigger Mode Register (TMR); bits 95:64 Read Only.
    uint32 TMR3[4]; //Trigger Mode Register (TMR); bits 127:96 Read Only.
    uint32 TMR4[4]; //Trigger Mode Register (TMR); bits 159:128 Read Only.
    uint32 TMR5[4]; //Trigger Mode Register (TMR); bits 191:160 Read Only.
    uint32 TMR6[4]; //Trigger Mode Register (TMR); bits 223:192 Read Only.
    uint32 TMR7[4]; //Trigger Mode Register (TMR); bits 255:224 Read Only.
    uint32 IRR0[4]; //Interrupt Request Register (IRR); bits 31:0 Read Only.
    uint32 IRR1[4]; //Interrupt Request Register (IRR); bits 63:32 Read Only.
    uint32 IRR2[4]; //Interrupt Request Register (IRR); bits 95:64 Read Only.
    uint32 IRR3[4]; //Interrupt Request Register (IRR); bits 127:96 Read Only.
    uint32 IRR4[4]; //Interrupt Request Register (IRR); bits 159:128 Read Only.
    uint32 IRR5[4]; //Interrupt Request Register (IRR); bits 191:160 Read Only.
    uint32 IRR6[4]; //Interrupt Request Register (IRR); bits 223:192 Read Only.
    uint32 IRR7[4]; //Interrupt Request Register (IRR); bits 255:224 Read Only.
    uint32 ErrStatus[4]; //Error Status Register Read Only.
    uint32 Reserved6[4]; //Reserved
    uint32 Reserved7[4]; //Reserved
    uint32 Reserved8[4]; //Reserved
    uint32 Reserved9[4]; //Reserved
    uint32 Reserved10[4]; //Reserved
    uint32 Reserved11[4]; //Reserved 
    uint32 LVT_CMCI[4]; //LVT CMCI Register Read/Write.
    uint32 ICR0[4]; //Interrupt Command Register (ICR); bits 0-31 Read/Write.
    uint32 ICR1[4]; //Interrupt Command Register (ICR); bits 32-63 Read/Write.
    uint32 LVT_Timer[4]; //LVT Timer Register Read/Write.
    uint32 LVT_Thermal[4]; //LVT Thermal Sensor Register2 Read/Write.
    uint32 LVT_Performance[4]; //LVT Performance Monitoring Counters Register3 Read/Write.
    uint32 LVT_LINT1[4]; //LVT LINT0 Register Read/Write.
    uint32 LVT_LINT1[4]; //LVT LINT1 Register Read/Write.
    uint32 LVT_Error[4]; //LVT Error Register Read/Write.
    uint32 InitialCount[4]; //Initial Count Register (for Timer) Read/Write.
    uint32 CurrentCount[4]; //Current Count Register (for Timer) Read Only.
    uint32 Reserved12[4]; //Reserved
    uint32 Reserved13[4]; //Reserved
    uint32 Reserved14[4]; //Reserved
    uint32 Reserved15[4]; //Reserved  
    uint32 DivideConfiguration[4]; //Divide Configuration Register (for Timer) Read/Write.
    uint32 Reserved16[4]; //Reserved
}LOCAL_APIC;


#define IA32_APIC_BASE_MSR 0x1B
#define IA32_APIC_BASE_MSR_BSP 0x100 // Processor is a BSP
#define IA32_APIC_BASE_MSR_ENABLE 0x800
#define CPUID_FEAT_EDX_APIC 0x200
#define CPUID_SUPPORT_ECX_x2APIC 0x200000

int check_apic();
int check_x2apic();

#endif
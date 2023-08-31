#ifndef _APIC_H_HH
#define _APIC_H_HH
#include "boot.h"

typedef struct LOCAL_APIC
{
    uint32 Reserved0[4]; //Reserved                                                        FEE00000H
    uint32 Reserved1[4]; //Reserved                                                        FEE00010H
    uint32 ID[4]; //Local APIC ID Register Read/Write.                                     FEE00020H
    uint32 Version[4]; //Local APIC Version Register Read Only.                            FEE00030H
    uint32 Reserved2[4]; //Reserved                                                        FEE00040H
    uint32 Reserved3[4]; //Reserved                                                        FEE00050H
    uint32 Reserved4[4]; //Reserved                                                        FEE00060H
    uint32 Reserved5[4]; //Reserved                                                        FEE00070H
    uint32 TPR[4]; //Task Priority Register (TPR) Read/Write.                              FEE00080H
    uint32 APR[4]; //Arbitration Priority Register1 (APR) Read Only.                       FEE00090H
    uint32 PPR[4]; //Processor Priority Register (PPR) Read Only.                          FEE000A0H
    uint32 EOI[4]; //EOI Register Write Only.                                              FEE000B0H
    uint32 RRD[4]; //Remote Read Register1 (RRD) Read Only                                 FEE000C0H
    uint32 LDR[4]; //Logical Destination Register Read/Write.                              FEE000D0H
    uint32 DFR[4]; //Destination Format Register Read/Write (see Section10.6.2.2).         FEE000E0H
    uint32 SIV[4]; //Spurious Interrupt Vector Register Read/Write (see Section 10.9.      FEE000F0H
    uint32 ISR0[4]; //In-Service Register (ISR); bits 31:0 Read Only.                      FEE00100H
    uint32 ISR1[4]; //In-Service Register (ISR); bits 63:32 Read Only.                     FEE00110H
    uint32 ISR2[4]; //In-Service Register (ISR); bits 95:64 Read Only.                     FEE00120H
    uint32 ISR3[4]; //In-Service Register (ISR); bits 127:96 Read Only.                    FEE00130H
    uint32 ISR4[4]; //In-Service Register (ISR); bits 159:128 Read Only.                   FEE00140H
    uint32 ISR5[4]; //In-Service Register (ISR); bits 191:160 Read Only.                   FEE00150H
    uint32 ISR6[4]; //In-Service Register (ISR); bits 223:192 Read Only.                   FEE00160H
    uint32 ISR7[4]; //In-Service Register (ISR); bits 255:224 Read Only.                   FEE00170H
    uint32 TMR0[4]; //Trigger Mode Register (TMR); bits 31:0 Read Only.                    FEE00180H
    uint32 TMR1[4]; //Trigger Mode Register (TMR); bits 63:32 Read Only.                   FEE00190H
    uint32 TMR2[4]; //Trigger Mode Register (TMR); bits 95:64 Read Only.                   FEE001A0H
    uint32 TMR3[4]; //Trigger Mode Register (TMR); bits 127:96 Read Only.                  FEE001B0H
    uint32 TMR4[4]; //Trigger Mode Register (TMR); bits 159:128 Read Only.                 FEE001C0H
    uint32 TMR5[4]; //Trigger Mode Register (TMR); bits 191:160 Read Only.                 FEE001D0H
    uint32 TMR6[4]; //Trigger Mode Register (TMR); bits 223:192 Read Only.                 FEE001E0H
    uint32 TMR7[4]; //Trigger Mode Register (TMR); bits 255:224 Read Only.                 FEE001F0H
    uint32 IRR0[4]; //Interrupt Request Register (IRR); bits 31:0 Read Only.               FEE00200H
    uint32 IRR1[4]; //Interrupt Request Register (IRR); bits 63:32 Read Only.              FEE00210H
    uint32 IRR2[4]; //Interrupt Request Register (IRR); bits 95:64 Read Only.              FEE00220H
    uint32 IRR3[4]; //Interrupt Request Register (IRR); bits 127:96 Read Only.             FEE00230H
    uint32 IRR4[4]; //Interrupt Request Register (IRR); bits 159:128 Read Only.            FEE00240H
    uint32 IRR5[4]; //Interrupt Request Register (IRR); bits 191:160 Read Only.            FEE00250H
    uint32 IRR6[4]; //Interrupt Request Register (IRR); bits 223:192 Read Only.            FEE00260H
    uint32 IRR7[4]; //Interrupt Request Register (IRR); bits 255:224 Read Only.            FEE00270H
    uint32 ErrStatus[4]; //Error Status Register Write/Read;                               FEE00280H
    uint32 Reserved6[4]; //Reserved                                                        FEE00290H
    uint32 Reserved7[4]; //Reserved                                                        FEE002A0H
    uint32 Reserved8[4]; //Reserved                                                        FEE002B0H
    uint32 Reserved9[4]; //Reserved                                                        FEE002C0H
    uint32 Reserved10[4]; //Reserved                                                       FEE002D0H
    uint32 Reserved11[4]; //Reserved                                                       FEE002E0H
    uint32 LVT_CMCI[4]; //LVT CMCI Register Read/Write.                                    FEE002F0H
    uint32 ICR0[4]; //Interrupt Command Register (ICR); bits 0-31 Read/Write.              FEE00300H
    uint32 ICR1[4]; //Interrupt Command Register (ICR); bits 32-63 Read/Write.             FEE00310H
    uint32 LVT_Timer[4]; //LVT Timer Register Read/Write.                                  FEE00320H
    uint32 LVT_Thermal[4]; //LVT Thermal Sensor Register2 Read/Write.                      FEE00330H
    uint32 LVT_Performance[4]; //LVT Performance Monitoring Counters Register3 Read/Write. FEE00340H
    uint32 LVT_LINT0[4]; //LVT LINT0 Register Read/Write.                                  FEE00350H
    uint32 LVT_LINT1[4]; //LVT LINT1 Register Read/Write.                                  FEE00360H
    uint32 LVT_Error[4]; //LVT Error Register Read/Write.                                  FEE00370H
    uint32 InitialCount[4]; //Initial Count Register (for Timer) Read/Write.               FEE00380H
    uint32 CurrentCount[4]; //Current Count Register (for Timer) Read Only.                FEE00390H
    uint32 Reserved12[4]; //Reserved                                                       FEE003A0H
    uint32 Reserved13[4]; //Reserved                                                       FEE003B0H
    uint32 Reserved14[4]; //Reserved                                                       FEE003C0H
    uint32 Reserved15[4]; //Reserved                                                       FEE003D0H
    uint32 DivideConfiguration[4]; //Divide Configuration Register (for Timer) Read/Write. FEE003E0H
    uint32 Reserved16[4]; //Reserved                                                       FEE003F0H
}LOCAL_APIC;

//MSR Address(x2APIC mode)                MMIO Offset(xAPIC mode) Register Name MSR R/W Semantics Comments
#define IA32_X2APIC_APICID        0x802 //020H Local APIC ID register Read-only1 See Section 11.12.5.1 for initial values.
#define IA32_X2APIC_VERSION       0x803 //030H Local APIC Version register Read-only Same version used in xAPIC mode and x2APIC mode.
#define IA32_X2APIC_TPR           0x808 //080H Task Priority Register (TPR) Read/write Bits 31:8 are reserved.2
#define IA32_X2APIC_PPR           0x80A //0A0H Processor Priority Register (PPR) Read-only
#define IA32_X2APIC_EOI           0x80B //0B0H EOI register Write-only3 WRMSR of a non-zero value causes #GP(0).
#define IA32_X2APIC_LDR           0x80D //0D0H Logical Destination Register(LDR) Read-only Read/write in xAPIC mode.
#define IA32_X2APIC_SIVR          0x80F //0F0H Spurious Interrupt Vector Register (SVR) Read/write See Section 11.9 for reserved bits.
#define IA32_X2APIC_ISR0          0x810 //100H In-Service Register (ISR); bits 31:0 Read-only
#define IA32_X2APIC_ISR1          0x811 //110H ISR bits 63:32 Read-only
#define IA32_X2APIC_ISR2          0x812 //120H ISR bits 95:64 Read-only
#define IA32_X2APIC_ISR3          0x813 //130H ISR bits 127:96 Read-only
#define IA32_X2APIC_ISR4          0x814 //140H ISR bits 159:128 Read-only
#define IA32_X2APIC_ISR5          0x815 //150H ISR bits 191:160 Read-only
#define IA32_X2APIC_ISR6          0x816 //160H ISR bits 223:192 Read-only
#define IA32_X2APIC_ISR7          0x817 //170H ISR bits 255:224 Read-only
#define IA32_X2APIC_TMR0          0x818 //180H Trigger Mode Register (TMR);bits 31:0 Read-only
#define IA32_X2APIC_TMR1          0x819 //190H TMR bits 63:32 Read-only
#define IA32_X2APIC_TMR2          0x81A //1A0H TMR bits 95:64 Read-only
#define IA32_X2APIC_TMR3          0x81B //1B0H TMR bits 127:96 Read-only
#define IA32_X2APIC_TMR4          0x81C //1C0H TMR bits 159:128 Read-only
#define IA32_X2APIC_TMR5          0x81D //1D0H TMR bits 191:160 Read-only
#define IA32_X2APIC_TMR6          0x81E //1E0H TMR bits 223:192 Read-only
#define IA32_X2APIC_TMR7          0x81F //1F0H TMR bits 255:224 Read-only
#define IA32_X2APIC_IRR0          0x820 //200H Interrupt Request Register (IRR); bits 31:0 Read-only
#define IA32_X2APIC_IRR1          0x821 //210H IRR bits 63:32 Read-only
#define IA32_X2APIC_IRR2          0x822 //220H IRR bits 95:64 Read-only
#define IA32_X2APIC_IRR3          0x823 //230H IRR bits 127:96 Read-only
#define IA32_X2APIC_IRR4          0x824 //240H IRR bits 159:128 Read-only
#define IA32_X2APIC_IRR5          0x825 //250H IRR bits 191:160 Read-only
#define IA32_X2APIC_IRR6          0x826 //260H IRR bits 223:192 Read-only
#define IA32_X2APIC_IRR7          0x827 //270H IRR bits 255:224 Read-only
#define IA32_X2APIC_ESR           0x828 //280H Error Status Register (ESR) Read/write WRMSR of a non-zero value causes #GP(0). See Section 11.5.3.
#define IA32_X2APIC_LVT_CMCI      0x82F //2F0H LVT CMCI register Read/write See Figure 11-8 for reserved bits.
#define IA32_X2APIC_ICR           0x830 //300H and 310H Interrupt Command Register (ICR) Read/write See Figure 11-28 for reserved bits
#define IA32_X2APIC_LVT_TIMER     0x832 //320H LVT Timer register Read/write See Figure 11-8 for reserved bits.
#define IA32_X2APIC_LVT_THERMAL   0x833 //330H LVT Thermal Sensor register Read/write See Figure 11-8 for reserved bits.
#define IA32_X2APIC_LVT_PMI       0x834 //340H LVT Performance Monitoring register Read/write See Figure 11-8 for reserved bits.
#define IA32_X2APIC_LVT_LINT0     0x835 //350H LVT LINT0 register Read/write See Figure 11-8 for reserved bits.
#define IA32_X2APIC_LVT_LINT1     0x836 //360H LVT LINT1 register Read/write See Figure 11-8 for reserved bits.
#define IA32_X2APIC_LVT_ERROR     0x837 //370H LVT Error register Read/write See Figure 11-8 for reserved bits.
#define IA32_X2APIC_INIT_COUNT    0x838 //380H Initial Count register (for Timer) Read/write
#define IA32_X2APIC_CUR_COUNT     0x839 //390H Current Count register (for Timer) Read-only
#define IA32_X2APIC_DIV_CONF      0x83E //3E0H Divide Configuration Register (DCR; for Timer) Read/write See Figure 11-10 for reserved bits.
#define IA32_X2APIC_SELF_IPI      0x83F //Not available SELF IPI5 Write-only Available only in x2APIC mode.






#define IA32_APIC_BASE_MSR 0x1B
#define IA32_APIC_BASE_MSR_BSP 0x100 // Processor is a BSP
#define IA32_APIC_BASE_MSR_ENABLE 0x800

#define CPUID_FEAT_EDX_APIC 0x200
#define CPUID_SUPPORT_ECX_x2APIC 0x200000

#define CPUID_APIC_TIMER_TSCDEADLINE 0x1000000

int check_apic();
int check_x2apic();

int check_apic_timer_tscdeadline();

int enablingx2APIC();

#endif
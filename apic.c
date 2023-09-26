#include "apic.h"
#include "boot.h"
#include "cpufeature.h"
#include "memcachectl.h"
#include "printf.h"
#include "osdataPhyAddr.h"
volatile uint32 xapicaddr = XAPIC_START_ADDR;
volatile uint32 logicalID = 1;

#ifdef TRACE_APIC
#	define TRACEAPIC(a...) printf(a...)
#else
#	define TRACEAPIC(a...)
#endif

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
int enablexApic()
{  
   uint32 eax = 0,ebx=0,ecx=0,edx = 0;
   uint64 apicbasemsrData =0,temp=0;
   uint32 pApicAddr=(uint32)&apicbasemsrData;
   if (cpufeatures[cpu_support_apic])
   {
      asm("cli");
      rdmsr_fence(IA32_APIC_BASE_MSR, &eax, &edx);
      apicbasemsrData = xapicaddr;
      xapicaddr+=0x1000;
      apicbasemsrData |=(eax & 0x1FF);
      apicbasemsrData |=(((uint64)1)<<11);
      eax = edx = ecx = edx = 0;
      cpuidcall(0x80000008, &eax, &edx, &ecx, &edx);
      eax = eax & 0xff;
      for(int i=0;i<eax;i++)
         temp |=(((uint64)1)<<i);
      apicbasemsrData &= temp;  
      wrmsr_fence(IA32_APIC_BASE_MSR, *(uint32 *)pApicAddr, *(uint32 *)(pApicAddr + 4));
      asm("sti");
      return TRUE;
   }
   return FALSE;
}
void initX2apic()
{
	uint32_t eax = 0, ebx = 0, ecx = 0, edx = 0;
	rdmsrcall(IA32_APIC_BASE_MSR, &eax, &edx);
	TRACEAPIC("before enable x2apic msr: low 32:0x%x high 32:%x\n", eax, edx);
	enablingx2APIC();
	eax = edx = 0;
	rdmsrcall(IA32_APIC_BASE_MSR, &eax, &edx);
	TRACEAPIC("after enable x2apic msr: low 32:0x%x high 32:%x\n", eax, edx);
	rdmsrcall(IA32_X2APIC_APICID, &eax, &edx);
	TRACEAPIC("Local x2APIC ID:.0x%x\n", eax);
	rdmsrcall(IA32_X2APIC_VERSION, &eax, &edx);
	TRACEAPIC("Local x2APIC Version:0x%x\n", eax);
	rdmsrcall(IA32_X2APIC_LDR, &eax, &edx); // Logical x2APIC ID = [(x2APIC ID[19:4] « 16) | (1 « x2APIC ID[3:0])]
	TRACEAPIC("Logical Destination:0x%x\n", eax);

	// rtc_8259a_enable();

	// 设置并初始化Local Apic error 中断向量为 0x81
	eax = 0x81;
	edx = 0;
	wrmsr_fence(IA32_X2APIC_LVT_ERROR, eax, edx);
	eax = edx = 0;
	wrmsr_fence(IA32_X2APIC_ESR, eax, edx);

	// 给自身处理器发送80h号中断测试
	eax = 0x80;
	edx = 0;
	wrmsr_fence(IA32_X2APIC_SELF_IPI, eax, edx);

	// IPI测试
	eax = 0xCC500; // vector =0x80  level = 1 tirgger =0  Destination Shorthand =All Excluding Self
	edx = 0xffffffff;
	wrmsr_fence(IA32_X2APIC_ICR, eax, edx);

	// Apic timer task switch
	eax = 0x82; // vector =0x82  Timer Mode =  Periodic    Not Masked
	edx = 0;
	wrmsr_fence(IA32_X2APIC_LVT_TIMER, eax, edx);
	eax = 0x9; // Divide Configuration 101: Divide by 64
	edx = 0;
	wrmsr_fence(IA32_X2APIC_DIV_CONF, eax, edx);

	// eax = 0x000fffff; //Initial Count
	// edx =0;
	// wrmsr_fence(IA32_X2APIC_INIT_COUNT,eax,edx);
}
void initxapic()
{
	enablexApic();
   
   LOCAL_APIC *xapic = (LOCAL_APIC *)getXapicAddr();
   int stat = mem4k_map((uint32)xapic, (uint32)xapic, MEM_UC,PAGE_G|PAGE_RW);
	TRACEAPIC("map xapic addr 0x%x status %d\n", (uint32)xapic,stat);
	TRACEAPIC("xapic siv =0x%x\n",xapic->SIV[0]);
	xapic->SIV[0] |=0x100; //software enable xapic
	TRACEAPIC("xapic id =0x%x\n",xapic->ID[0]);
	TRACEAPIC("xapic Version =0x%x\n",xapic->Version[0]);
	TRACEAPIC("xapic LDR =0x%x\n",logicalID);
   //设置LDR,DFR
   xapic->LDR[0] = logicalID;
   xapic->DFR[0] = 0xF0000000; //DFR设置为 平坦模式
   logicalID<<=1;

   if((xapic->Version[0]&(1<<24)))//支持当中断函数写入EOI 时候给I/O APIC广播 中断完成
      xapic->SIV[0]|=0x1000;//设置写入EOI 时不向I/O APIC广播

   // 设置并初始化Local Apic error 中断向量为 0x81
   xapic->LVT_Error[0] =0x81;
   xapic->ErrStatus[0] =0;

   // // 给自身处理器发送80h号中断测试
   // xapic->ICR1[0]=0;
   // xapic->ICR0[0]=0x44080;

   // // IPI测试
   // xapic->ICR1[0]=0;
   // xapic->ICR0[0]=0x84080;


}

void initApic()
{
#if X2APIC_ENABLE
   initX2apic();
#else
   initxapic();
#endif
}
uint32 getXapicAddr()
{
   uint32 eax = 0, edx = 0;
   rdmsr_fence(IA32_APIC_BASE_MSR, &eax, &edx);
   return (eax&0xfffff000);
}
#include "interruptGate.h"
#include "boot.h"
#include "printf.h"
#include "apic.h"
#include "memcachectl.h"
extern KernelData kernelData;
extern TaskCtrBlock **mainTask;
extern TaskCtrBlock **procCurrTask;
extern ProcessorInfo processorinfo;
extern AParg *aparg;
int general_exeption_no_code(uint32 eno,uint32 addr)
{
    printf("********Exception in 0x%x %d encounted********\r\n",addr, eno);
    return 0;
}

int general_exeption_code(uint32 eno, uint32 code,uint32 addr)
{
    printf("********Exception  %d in 0x%x  code: 0x%x********\r\n", eno,addr, code);
    return 0;
}
static void xapicwriteEOI()
{
    LOCAL_APIC *apic = (LOCAL_APIC *)getXapicAddr();
    apic->EOI[0] = 0;
}
static void x2apicwriteEOI()
{
    wrmsr_fence(IA32_X2APIC_EOI, 0, 0);
}
static void local_x2apic_error_handling()
{
    uint32 eax = 0, edx = 0, x2id = 0, empty = 0;
    wrmsr_fence(IA32_X2APIC_ESR, 0, 0);
    rdmsr_fence(IA32_X2APIC_ESR, &eax, &edx);
    rdmsr_fence(IA32_X2APIC_APICID, &x2id, &empty);
    printf("x2apic error: x2apicid =0x%x code =0x%x\r\n", x2id, eax);
    x2apicwriteEOI();
}
static void local_xapic_error_handling()
{
    LOCAL_APIC *apic = (LOCAL_APIC *)getXapicAddr();
    apic->ErrStatus[0] = 0;
    printf("xapic error: apicid =0x%x code =0x%x\r\n", apic->ID[0], apic->ErrStatus[0]);
    xapicwriteEOI();
}

static void x2ApicTimeOut()
{
    x2apicwriteEOI();
}
static void xApicTimeOut()
{
    LOCAL_APIC *apic = (LOCAL_APIC *)getXapicAddr();
    uint32 apid = apic->ID[0] >> 24;
    if(kernelData.taskList.size<2)
    {
        xapicwriteEOI();
        return;
    }
    TaskCtrBlock *nextTask = procCurrTask[apid]->next;
    while(nextTask!=procCurrTask[apid])
    {
        spinlock(lockBuff[KERNEL_LOCK].plock);
        if(nextTask->taskStats == 0)
        {
            nextTask->taskStats =1;
            unlock(lockBuff[KERNEL_LOCK].plock);
            break;
        }
        nextTask = nextTask->next;
        unlock(lockBuff[KERNEL_LOCK].plock);
    }
    printf("xApicTimeOut first:0x%x  current:0x%x  next:0x%x\r\n",kernelData.taskList.tcb_Frist,procCurrTask[apid],nextTask);
    if(nextTask!=procCurrTask[apid])
    {
        procCurrTask[apid]->taskStats =0;
        procCurrTask[apid] = nextTask;
        if(nextTask != kernelData.taskList.tcb_Frist)
            apic->InitialCount[0] = 0xfffff;
        printf("jump 0x%x\r\n",nextTask);
        xapicwriteEOI();
        callTss(&(nextTask->AllocateNextAddr));
    }
    else
    {
        if(nextTask != kernelData.taskList.tcb_Frist)
            apic->InitialCount[0] = 0xfffff;
        xapicwriteEOI();
    }
}
void systemCall()
{
#if X2APIC_ENABLE
    uint32 x2id = 0, empty = 0;
    rdmsr_fence(IA32_X2APIC_APICID, &x2id, &empty);
    printf("system call x2apicid:0x%x\r\n", x2id);
#else
    printf("system call apicid:0x%x\r\n", ((LOCAL_APIC *)getXapicAddr())->ID[0]);
#endif
}
void apicError()
{
#if X2APIC_ENABLE
    local_x2apic_error_handling();
#else
    local_xapic_error_handling();
#endif
}
void apicTimeOut()
{
#if X2APIC_ENABLE
    x2ApicTimeOut();
#else
    xApicTimeOut();
#endif
}

void updataGdt()
{
    spinlock(lockBuff[UPDATE_GDT_CR3].plock);
	aparg->logcpucount++;
	unlock(lockBuff[UPDATE_GDT_CR3].plock);
    while(aparg->logcpucount<processorinfo.count);
    resetcr3();
    setgdtr(&(kernelData.gdtInfo));

	spinlock(lockBuff[UPDATE_GDT_CR3].plock);
	aparg->logcpucount--;
	unlock(lockBuff[UPDATE_GDT_CR3].plock);
	while(aparg->logcpucount>0);
    printf("updataGdt apicid:0x%x\r\n", ((LOCAL_APIC *)getXapicAddr())->ID[0]);
    xapicwriteEOI();
}

void processorMtrrSync()
{
	spinlock(lockBuff[MTRR_LOCK].plock);
	aparg->logcpucount++;
	unlock(lockBuff[MTRR_LOCK].plock);
	while(aparg->logcpucount<processorinfo.count);
	refreshMtrrMsrs();
	spinlock(lockBuff[MTRR_LOCK].plock);
	aparg->logcpucount--;
	unlock(lockBuff[MTRR_LOCK].plock);
	while(aparg->logcpucount>0);
    printf("processorMtrrSync apicid:0x%x\r\n", ((LOCAL_APIC *)getXapicAddr())->ID[0]);
    xapicwriteEOI();
}
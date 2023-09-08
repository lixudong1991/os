#include "interruptGate.h"
#include "boot.h"
#include "printf.h"
#include "apic.h"
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
    // printf("xApicTimeOut apicid:0x%x\r\n",((LOCAL_APIC*)getXapicAddr())->ID[0]);
    LOCAL_APIC *apic = (LOCAL_APIC *)getXapicAddr();
    uint32 apid = apic->ID[0] >> 24;
    TaskCtrBlock *next = procCurrTask[apid]->next;
    while (next != procCurrTask[apid])
    {
        if (next == NULL)
            next = kernelData.taskList.tcb_Frist->next;
        spinlock(lockBuff[KERNEL_LOCK].plock);
        if (next->taskStats == 0)
        {
            next->taskStats =1; 
            unlock(lockBuff[KERNEL_LOCK].plock);
            break;
        }
        unlock(lockBuff[KERNEL_LOCK].plock);
    }
    if(next!=procCurrTask[apid])
    {
        procCurrTask[apid]->taskStats =0;
        xapicwriteEOI();
        callTss(&(next->AllocateNextAddr));
    }
    else
        xapicwriteEOI();
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
    resetcr3();
    setgdtr(&(kernelData.gdtInfo));
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

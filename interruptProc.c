#include "interruptGate.h"
#include "boot.h"
#include "printf.h"
#include "apic.h"
#include "memcachectl.h"
#include "osdataPhyAddr.h"
extern KernelData kernelData;
extern TcbList* cpuTaskList;
extern TaskCtrBlock** pEmptyTask;
extern TssPointer* cpuTssdata;
extern ProcessorInfo processorinfo;
extern AParg *aparg;
typedef int (*InterruptPrintfFun)(const char* fmt, ...);

volatile InterruptPrintfFun interrput = interruptPrintf;
int general_exeption_no_code(uint32 eno,uint32 addr)
{
    interrput("********Exception in 0x%x %d encounted********\n",addr, eno);
    return 0;
}

int general_exeption_code(uint32 eno, uint32 code,uint32 addr)
{
    interrput("********Exception  %d in 0x%x  code: 0x%x cr3:0x%x********\n", eno,addr, code,cr3_data());
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
    interrput("x2apic error: x2apicid =0x%x code =0x%x\n", x2id, eax);
    x2apicwriteEOI();
}
static void local_xapic_error_handling()
{
    LOCAL_APIC *apic = (LOCAL_APIC *)getXapicAddr();
    apic->ErrStatus[0] = 0;
    interrput("xapic error: apicid =0x%x code =0x%x\n", apic->ID[0], apic->ErrStatus[0]);
    xapicwriteEOI();
}

static void x2ApicTimeOut()
{
    x2apicwriteEOI();
}
static void xApicTimeOut()
{
    LOCAL_APIC* apic = (LOCAL_APIC*)getXapicAddr();
    uint32_t apid = apic->ID[0] >> 24;
    TaskCtrBlock* pTask = cpuTaskList[apid].pcurrTask;

    if (pTask->processdata.threads->status != SLEEPING)
        pTask->processdata.threads->status = READY;

    TaskCtrBlock* nextTask = NULL;
     
    pTask = pTask->next;
    while (1)
    {
        if (pTask == NULL)
            pTask = cpuTaskList[apid].tcb_Frist;
        if (pTask->processdata.threads->status == READY)
        {
            nextTask = pTask;
            break;
        }
    }
    if (nextTask ==NULL)
    {
        nextTask = pEmptyTask[apid];
    }
    if (nextTask == cpuTaskList[apid].pcurrTask)
    {
        apic->InitialCount[0] = 0xffffff;
        nextTask->processdata.threads->status = RUNNING;
        xapicwriteEOI();
        return;
    }
    TaskCtrBlock* temp = cpuTaskList[apid].pcurrTask;
    cpuTaskList[apid].pcurrTask = nextTask;
    cpuTssdata[apid].pTssdata->ss0 = cpuTaskList[apid].pcurrTask->processdata.context.ss0;
    cpuTssdata[apid].pTssdata->esp0 = cpuTaskList[apid].pcurrTask->processdata.threads->context.esp0;

    apic->InitialCount[0] = 0xffffff;
    nextTask->processdata.threads->status = RUNNING;
    xapicwriteEOI();
    switchStack(&(temp->processdata.context.ss),&(temp->processdata.threads->context.esp), nextTask->processdata.context.ss, nextTask->processdata.threads->context.esp, nextTask->processdata.context.cr3);
    /*
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
    interruptPrintf("xApicTimeOut first:0x%x  current:0x%x  next:0x%x\n",kernelData.taskList.tcb_Frist,procCurrTask[apid],nextTask);
    if(nextTask!=procCurrTask[apid])
    {
        procCurrTask[apid]->taskStats =0;
        procCurrTask[apid] = nextTask;
        if(nextTask != kernelData.taskList.tcb_Frist)
            apic->InitialCount[0] = 0xfffff;
        interruptPrintf("jump 0x%x\n",nextTask);
        xapicwriteEOI();
        //callTss(&(nextTask->AllocateNextAddr));
    }
    else
    {
        if(nextTask != kernelData.taskList.tcb_Frist)
            apic->InitialCount[0] = 0xfffff;
        xapicwriteEOI();
    }
    */
}
void systemCall()
{
#if X2APIC_ENABLE
    uint32 x2id = 0, empty = 0;
    rdmsr_fence(IA32_X2APIC_APICID, &x2id, &empty);
    interrput("system call x2apicid:0x%x\n", x2id);
#else
    interrput("system call apicid:0x%x\n", ((LOCAL_APIC *)getXapicAddr())->ID[0]);
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
    uint32_t *pAtomicBuff = ATOMIC_BUFF_ADDR;

    spinlock(lockBuff[UPDATE_GDT_CR3].plock);
	pAtomicBuff[UPDATE_GDT_CR3]--;
	unlock(lockBuff[UPDATE_GDT_CR3].plock);

    while(pAtomicBuff[UPDATE_GDT_CR3]>0);

    resetcr3();
    setgdtr(&(kernelData.gdtInfo));

	spinlock(lockBuff[UPDATE_GDT_CR3].plock);
	pAtomicBuff[UPDATE_GDT_CR3]++;
	unlock(lockBuff[UPDATE_GDT_CR3].plock);

	while(pAtomicBuff[UPDATE_GDT_CR3]<processorinfo.count);

   // interruptPrintf("updataGdt apicid:0x%x\n", ((LOCAL_APIC *)getXapicAddr())->ID[0]);
    xapicwriteEOI();
}

void processorMtrrSync()
{
    uint32_t *pAtomicBuff = ATOMIC_BUFF_ADDR;
	spinlock(lockBuff[MTRR_LOCK].plock);
	pAtomicBuff[MTRR_LOCK]--;
	unlock(lockBuff[MTRR_LOCK].plock);
	while(pAtomicBuff[MTRR_LOCK]>0);
	refreshMtrrMsrs();
	spinlock(lockBuff[MTRR_LOCK].plock);
	pAtomicBuff[MTRR_LOCK]++;
	unlock(lockBuff[MTRR_LOCK].plock);

	while(pAtomicBuff[MTRR_LOCK]<processorinfo.count);

  //  interruptPrintf("processorMtrrSync apicid:0x%x\n", ((LOCAL_APIC *)getXapicAddr())->ID[0]);
    xapicwriteEOI();
}

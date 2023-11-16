#include "interruptGate.h"
#include "boot.h"
#include "printf.h"
#include "apic.h"
#include "memcachectl.h"
#include "osdataPhyAddr.h"
extern void x_apicwriteEOI();
#define xapicwriteEOI x_apicwriteEOI
extern KernelData kernelData;
extern TcbList *cpuTaskList;
extern TaskCtrBlock** pCpuCurrentTask;
extern TaskCtrBlock** pEmptyTask;
extern TssPointer* cpuTssdata;
extern TssPointer* cpuTaskTssdata;
extern ProcessorInfo processorinfo;
extern AParg *aparg;
typedef int (*InterruptPrintfFun)(const char* fmt, ...);

volatile InterruptPrintfFun interrput = interruptPrintf;


int general_exeption_no_code(uint32 eno,uint32 addr)
{
    interrput("********Exception in 0x%x %d encounted********\n",addr, eno);
    return 0;
}

int general_exeption_code(uint32 eno, uint32 code, uint32 addr)
{
    interrput("********Exception  %d in 0x%x  code: 0x%x cr3:0x%x********\n", eno, addr, code, cr3_data());
    return 0;
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
void xApicTimeOut()
{
    LOCAL_APIC* apic = (LOCAL_APIC*)getXapicAddr();
    uint32_t apid = apic->ID[0] >> 24;
    TaskCtrBlock* pTargetTask = NULL;
    uint32_t isstart = FALSE;
    if (pCpuCurrentTask[apid]->processdata.threads->status == RUNNING)
        pCpuCurrentTask[apid]->processdata.threads->status = READY;

    do {

        pTargetTask = pCpuCurrentTask[apid]->next;

        while (1)
        {
            if (pTargetTask == NULL)
            {
                pTargetTask = cpuTaskList[apid].tcb_Frist;
            }
          //  interrput("c cpu:%d pid:%d stat:%d\n", apid, pTargetTask->processdata.pid, pTargetTask->processdata.threads->status);
            if (pTargetTask->processdata.threads->status == READY)
            {
                pTargetTask->processdata.threads->status = RUNNING;
                break;
            }
            if (pTargetTask->processdata.threads->status == START)
            {
                isstart = TRUE;
                pTargetTask->processdata.threads->status = RUNNING;
                break;
            }
            if (pTargetTask == pCpuCurrentTask[apid])
            {
                pTargetTask = NULL;
                break;
            }
            pTargetTask = pTargetTask->next;
        }
        if (pTargetTask == NULL)
        {
            asm("sti");
            asm("hlt");
            asm("cli");
        }
    } while (pTargetTask==NULL);

    if (pCpuCurrentTask[apid] != pTargetTask)
    {
        TaskCtrBlock* oldtask = pCpuCurrentTask[apid];
        pCpuCurrentTask[apid] = pTargetTask;
        cpuTaskTssdata[apid].pTssdata->cr3 = pTargetTask->processdata.context.cr3;
        cpuTaskTssdata[apid].pTssdata->ioPermission = pTargetTask->processdata.context.ioPermission;
        cpuTaskTssdata[apid].pTssdata->esp0 = pTargetTask->processdata.threads->context.esp0;
        cpuTaskTssdata[apid].pTssdata->eflags = pTargetTask->processdata.threads->context.eflags;
        cpuTaskTssdata[apid].pTssdata->esp = pTargetTask->processdata.threads->context.esp;
        cpuTaskTssdata[apid].pTssdata->eip = pTargetTask->processdata.threads->context.eip;
        xapicwriteEOI();
        apic->InitialCount[0] = 0xfffff;
        if (isstart)
        {
        //    interrput("a cpu:%d pid:%d esp:0x%x\n", apid, pTargetTask->processdata.pid, esp_data());
            switchNewTask(&(oldtask->processdata.threads->context.esp), pTargetTask->processdata.threads->context.esp, cpuTaskTssdata[apid].pTssdata);
        }
        else
        {
        //    interrput("b cpu:%d pid:%d esp:0x%x\n", apid, pTargetTask->processdata.pid, esp_data());
            switchStack(&(oldtask->processdata.threads->context.esp), pTargetTask->processdata.threads->context.esp, pTargetTask->processdata.context.cr3);
        }
    }
    else
    {
        interrput("pCpuCurrentTask apid:%d pid%d cr3:0x%x\n", apid, pTargetTask->processdata.pid,cr3_data());
        xapicwriteEOI();
        apic->InitialCount[0] = 0xfffff;
    }

}
void taskSleep()
{
    LOCAL_APIC* apic = (LOCAL_APIC*)getXapicAddr();
    uint32_t apid = apic->ID[0] >> 24;
    pCpuCurrentTask[apid]->processdata.threads->status = SLEEPING;
    xApicTimeOut();
}
void apicError()
{
#if X2APIC_ENABLE
    local_x2apic_error_handling();
#else
    local_xapic_error_handling();
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

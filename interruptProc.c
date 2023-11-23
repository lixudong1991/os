#include "interruptGate.h"
#include "boot.h"
#include "printf.h"
#include "apic.h"
#include "memcachectl.h"
#include "osdataPhyAddr.h"
#include "string.h"
#include "vbe.h"
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
/*
void xApicTimeOut()
{
    static uint32_t countSecond[8] ={ 0 };
    LOCAL_APIC* apic = (LOCAL_APIC*)getXapicAddr();
    uint32_t apid = apic->ID[0] >> 24;
    char buff[9] = { 0 };
    hexstr32(buff, countSecond[apid]++);
    buff[8] = 0;
    Rect rect;
    rect.top = 0;
    rect.bottom = 40;
    rect.left = apid * 100;
    rect.right = rect.left + 100;
    printText(buff, &rect);
    xapicwriteEOI();
}
*/
void xApicTimeOut()
{
    LOCAL_APIC* apic = (LOCAL_APIC*)getXapicAddr();
    uint32_t apid = apic->ID[0] >> 24;
    TaskCtrBlock* pTargetTask = NULL;
    uint32_t isstart = FALSE;
    if (pCpuCurrentTask[apid]->processdata.threads->status == RUNNING)
        pCpuCurrentTask[apid]->processdata.threads->status = READY;

    do {
        if (cpuTaskList[apid].pSleepTaskHead != &(cpuTaskList[apid].pSleepBuff[0]))
        {
          //  interrput("pCpuCurrentTask apid:%d pid:%d pSleepTaskHead:0x%x sleepcount:0x%llx basecount:0x%x\n", apid, pCpuCurrentTask[apid]->processdata.pid, cpuTaskList[apid].pSleepTaskHead, pCpuCurrentTask[apid]->processdata.threads->wake_time, cpuTaskList[apid].baseSchedCount);
            if (cpuTaskList[apid].pSleepTaskHead->pTask->processdata.threads->wake_time <= cpuTaskList[apid].baseSchedCount)
            {
                cpuTaskList[apid].pSleepTaskHead->pTask->processdata.threads->wake_time = 0;
                while (cpuTaskList[apid].pSleepTaskHead != &(cpuTaskList[apid].pSleepBuff[0]))
                {
                    cpuTaskList[apid].pSleepTaskHead->pTask->processdata.threads->status = READY;
                    SleepTaskNode* temp = cpuTaskList[apid].pSleepTaskHead;
                    cpuTaskList[apid].pSleepTaskHead = (cpuTaskList[apid].pSleepTaskHead->next);
                    memset(temp,0,sizeof(SleepTaskNode));
                    if (cpuTaskList[apid].pSleepTaskHead->pTask->processdata.threads->wake_time != 0)
                        break;
                }
                if (cpuTaskList[apid].pSleepTaskHead == &(cpuTaskList[apid].pSleepBuff[0]))
                {
                    cpuTaskList[apid].pSleepTaskHead->next = cpuTaskList[apid].pSleepTaskHead->prior =NULL;
                }
            }
            else
            {
                cpuTaskList[apid].pSleepTaskHead->pTask->processdata.threads->wake_time -= cpuTaskList[apid].baseSchedCount;
            }
            
        }

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
            apic->LVT_Timer[0] = 0x6f;//一次性模式
            apic->DivideConfiguration[0] = 0xb;
            apic->InitialCount[0] = cpuTaskList[apid].baseSchedCount;
            asm("sti");
            asm("hlt");
            asm("cli");
            apic->LVT_Timer[0] = 0x82;//一次性模式
//xapic_obj->LVT_Timer[0]=0x20082; //周期模式
            apic->DivideConfiguration[0] = 0xb;
            apic->InitialCount[0] = 0;
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
        apic->InitialCount[0] = cpuTaskList[apid].baseSchedCount;
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
     //   interrput("pCpuCurrentTask apid:%d pid%d cr3:0x%x\n", apid, pTargetTask->processdata.pid,cr3_data());
        xapicwriteEOI();
        apic->InitialCount[0] = cpuTaskList[apid].baseSchedCount;;
    }

}
void taskSleep(uint32_t tv_sec, uint32_t tv_nsec)
{
    LOCAL_APIC* apic = (LOCAL_APIC*)getXapicAddr();
    uint32_t apid = apic->ID[0] >> 24;
    apic->InitialCount[0] = 0;
    pCpuCurrentTask[apid]->processdata.threads->status = SLEEPING;
    pCpuCurrentTask[apid]->processdata.threads->wake_time = tv_sec;
    pCpuCurrentTask[apid]->processdata.threads->wake_time *= processorinfo.processcontent[apid].cpuBusFrequencyLow;
    pCpuCurrentTask[apid]->processdata.threads->wake_time += (tv_nsec / processorinfo.processcontent[apid].nsCountPerCycle);

//    interrput("pCpuCurrentTask apid:%d pid%d sleepcount:0x%llx\n", apid, pCpuCurrentTask[apid]->processdata.pid, pCpuCurrentTask[apid]->processdata.threads->wake_time);
    SleepTaskNode* sleepNode = NULL;
    for (int i=1;i< MAXTASKCOUNT;i++)
    {
        if (cpuTaskList[apid].pSleepBuff[i].pTask == NULL)
        {
            sleepNode = &(cpuTaskList[apid].pSleepBuff[i]);
            break;
        }
    }
    if (sleepNode)
    {
   //     interrput("cpuid :%d sleepNode 0x%x pSleepTaskHead:0x%x  pSleepBuff[0]:0x%x\n", apid, sleepNode, cpuTaskList[apid].pSleepTaskHead, &(cpuTaskList[apid].pSleepBuff[0]));
        sleepNode->pTask = pCpuCurrentTask[apid];
        sleepNode->next = sleepNode->prior = NULL;
        SleepTaskNode* pTargetNode = cpuTaskList[apid].pSleepTaskHead;
        uint64_t sleepCount = 0;
        while ( pTargetNode && (pTargetNode!= &(cpuTaskList[apid].pSleepBuff[0])))
        {
            sleepCount += (pTargetNode->pTask->processdata.threads->wake_time);
            if (sleepNode->pTask->processdata.threads->wake_time<= sleepCount)
            {
                break;
            }
            pTargetNode = pTargetNode->next;
        }
        if (pTargetNode == &(cpuTaskList[apid].pSleepBuff[0]))
        {

            if (pTargetNode->prior==NULL)
            {
                cpuTaskList[apid].pSleepTaskHead = sleepNode;
                sleepNode->next = pTargetNode;
                pTargetNode->prior = sleepNode;
            }
            else
            {
                sleepNode->pTask->processdata.threads->wake_time -= sleepCount;
                pTargetNode->prior->next = sleepNode;
                sleepNode->prior = pTargetNode->prior;
                sleepNode->next = pTargetNode;
                pTargetNode->prior = sleepNode;
            }
        }
        else
        {
            uint64_t tempcount = sleepCount - pTargetNode->pTask->processdata.threads->wake_time;
            pTargetNode->pTask->processdata.threads->wake_time =(sleepCount- sleepNode->pTask->processdata.threads->wake_time);
            sleepNode->pTask->processdata.threads->wake_time -= tempcount;

            if (pTargetNode->prior == NULL)
            {
                cpuTaskList[apid].pSleepTaskHead = sleepNode;
                sleepNode->next = pTargetNode;
                pTargetNode->prior = sleepNode;
            }
            else
            {
                sleepNode->pTask->processdata.threads->wake_time -= sleepCount;
                pTargetNode->prior->next = sleepNode;
                sleepNode->prior = pTargetNode->prior;
                sleepNode->next = pTargetNode;
                pTargetNode->prior = sleepNode;
            }
        }
 //       interrput("cpuid :%d sleepNode 0x%x pSleepTaskHead:0x%x  pSleepBuff[0]:0x%x\n", apid, sleepNode, cpuTaskList[apid].pSleepTaskHead, &(cpuTaskList[apid].pSleepBuff[0]));
    }
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

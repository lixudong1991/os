#include "boot.h"
#include "osdataPhyAddr.h"
#include "callgate.h"
#include "interruptGate.h"
#include "elf.h"
#include "string.h"
#include "printf.h"
#include "apic.h"
#include "cpufeature.h"
#include "memcachectl.h"
#include "acpi.h"
#include "ioapic.h"
#include "ps2device.h"
#include "pcie.h"
#include "ahci.h"
#include "stdlib.h"
#include "fat32.h"
#include "ff.h"
#include "vbe.h"

#define STACKLIMIT_G1(a) ((((uint32)(a)) - 1) >> 12) // gdt 表项粒度为1的段界限

volatile BootParam bootparam;
KernelData kernelData;
LockBlock *lockblock = NULL;
AParg *aparg = (AParg *)AP_ARG_ADDR;
ProcessorInfo processorinfo;
TcbList *cpuTaskList= NULL;
TaskCtrBlock** pCpuCurrentTask = NULL;
TaskCtrBlock** pEmptyTask = NULL;
TssPointer* cpuTssdata=NULL;
TssPointer* cpuTaskTssdata = NULL;
LockObj *lockBuff = NULL;
uint32_t* g_pidIndex=NULL;
char *hexstr32(char buff[9], uint32 val)
{
	char hexs[] = "0123456789ABCDEF";
	memset_s(buff, '0', 8);
	buff[8] = 0;
	uint32 v = val;
	for (int index = 7; index > -1 && v != 0; index--)
	{
		buff[index] = hexs[v % 16];
		v /= 16;
	}
	return buff;
}
char *hexstr64(char buff[17], uint64 val)
{
	char hexs[] = "0123456789ABCDEF";
	memset_s(buff, '0', 16);
	buff[16] = 0;
	uint64 v = val;
	for (int index = 15; index > -1 && v != 0; index--)
	{
		buff[index] = hexs[v % 16];
		v /= 16;
	}
	return buff;
}

static void createCallGate(KernelData *kdata)
{
	uint32 index = 0;
	uint16 gateSel = 0;
	TableGateItem item;
	memset_s(&item, 0, sizeof(TableGateItem));

	item.Type = CALL_GATE_TYPE;
	item.segSelect = cs_data();
	item.segAddr = puts_s;
	item.argCount = 1;
	item.GateDPL = 3;
	item.P = 1;
	gateSel = appendTableGateItem(&(kdata->gdtInfo), &item);
	strcpy_s(kdata->gateInfo[index].gateName, "puts_s");
	kdata->gateInfo[index].gateSelect = gateSel;
	kdata->gateInfo[index].gateAddr = puts_s;
	kdata->gataSize = ++index;

	item.segSelect = cs_data();
	item.segAddr = clearScreen_s;
	item.argCount = 0;
	item.GateDPL = 3;
	item.P = 1;
	gateSel = appendTableGateItem(&(kdata->gdtInfo), &item);
	strcpy_s(kdata->gateInfo[index].gateName, "clearScreen_s");
	kdata->gateInfo[index].gateSelect = gateSel;
	kdata->gateInfo[index].gateAddr = clearScreen_s;
	kdata->gataSize = ++index;

	item.segSelect = cs_data();
	item.segAddr = setcursor_s;
	item.argCount = 1;
	item.GateDPL = 3;
	item.P = 1;
	gateSel = appendTableGateItem(&(kdata->gdtInfo), &item);
	strcpy_s(kdata->gateInfo[index].gateName, "setcursor_s");
	kdata->gateInfo[index].gateSelect = gateSel;
	kdata->gateInfo[index].gateAddr = setcursor_s;
	kdata->gataSize = ++index;

	item.segSelect = cs_data();
	item.segAddr = readSectors_s;
	item.argCount = 3;
	item.GateDPL = 3;
	item.P = 1;
	gateSel = appendTableGateItem(&(kdata->gdtInfo), &item);
	strcpy_s(kdata->gateInfo[index].gateName, "readSectors_s");
	kdata->gateInfo[index].gateSelect = gateSel;
	kdata->gateInfo[index].gateAddr = readSectors_s;
	kdata->gataSize = ++index;

	item.segSelect = cs_data();
	item.segAddr = exit_s;
	item.argCount = 1;
	item.GateDPL = 3;
	item.P = 1;
	gateSel = appendTableGateItem(&(kdata->gdtInfo), &item);
	strcpy_s(kdata->gateInfo[index].gateName, "exit_s");
	kdata->gateInfo[index].gateSelect = gateSel;
	kdata->gateInfo[index].gateAddr = exit_s;
	kdata->gataSize = ++index;
}
static void createInterruptGate(KernelData *kdata)
{
	TableGateItem item;
	memset_s(&item, 0, sizeof(TableGateItem));

	item.Type = INTERRUPT_GATE_TYPE;
	item.segSelect = cs_data();
	item.argCount = 0;
	item.GateDPL = 0;
	item.P = 1;
	int i = 0;
	for (; i < 20; i++)
	{
		item.segAddr = exceptionCalls[i];
		appendTableGateItem(&(kdata->idtInfo), &item);
	}
	item.segAddr = general_interrupt_handler;
	for (; i < 0x20; i++)
		appendTableGateItem(&(kdata->idtInfo), &item);

	item.segAddr = interrupt_8259a_handler;
	for (; i < 0x27; i++)
		appendTableGateItem(&(kdata->idtInfo), &item);

	item.segAddr = interrupt_27_handler; // IRQ1 中断
	appendTableGateItem(&(kdata->idtInfo), &item);
	i++;
	item.segAddr = interrupt_28_handler; // IRQ12 中断
	appendTableGateItem(&(kdata->idtInfo), &item);
	i++;
	item.segAddr = general_interrupt_handler;
	for (; i < 0x70; i++)
		appendTableGateItem(&(kdata->idtInfo), &item);
	item.segAddr = interrupt_70_handler;
	appendTableGateItem(&(kdata->idtInfo), &item);
	i++;
	item.segAddr = interrupt_8259a_handler;
	for (; i < 0x78; i++)
		appendTableGateItem(&(kdata->idtInfo), &item);

	item.segAddr = interrupt_78_handler; // pci ahci interrupt
	item.GateDPL = 0;
	appendTableGateItem(&(kdata->idtInfo), &item);
	i++;

	item.segAddr = general_interrupt_handler;
	for (; i < 0x80; i++)
	{
		appendTableGateItem(&(kdata->idtInfo), &item);
	}
	item.segAddr = interrupt_80_handler; // syscall
	item.GateDPL = 3;
	appendTableGateItem(&(kdata->idtInfo), &item);
	i++;

	item.segAddr = interrupt_81_handler; // apic error
	item.GateDPL = 0;
	appendTableGateItem(&(kdata->idtInfo), &item);
	i++;

	item.segAddr = interrupt_82_handler; // apictimer
	appendTableGateItem(&(kdata->idtInfo), &item);
	i++;

	item.segAddr = interrupt_83_handler; // updategdt
	appendTableGateItem(&(kdata->idtInfo), &item);
	i++;

	item.segAddr = interrupt_84_handler; // updatemtrr
	appendTableGateItem(&(kdata->idtInfo), &item);
	i++;

	item.segAddr = general_interrupt_handler;
	for (; i < 0xff; i++)
	{
		appendTableGateItem(&(kdata->idtInfo), &item);
	}

	setidtr(&(kdata->idtInfo));
	sti_s();
}


/*
void createTask(TcbList *taskList, int taskStartSection, int SectionCount)
{
	cli_s();
	TaskCtrBlock *newTask = createNewTcb(taskList);
	char *elfdata = allocate_memory(taskList->tcb_Frist, SectionCount * 512, PAGE_RW);
	read_ata_sectors(elfdata, taskStartSection, SectionCount);

	ProgramaData prodata;
	prodata.vir_end = 0;
	prodata.vir_base = 0xffffffff;
	loadElf(elfdata, &prodata, PRIVILEGUSER);

	newTask->pFreeListAddr = (uint32*)allocUnCacheMem(sizeof(uint32));
	TaskFreeMemList* pFreeList = kernel_malloc(sizeof(TaskFreeMemList));
	*(newTask->pFreeListAddr)= pFreeList;
	pFreeList->memAddr =prodata.vir_end + 1;
	pFreeList->next = NULL;
	pFreeList->memSize = USERMALLOCEND - pFreeList->memAddr;
	uint32 stacksize = 1, stack0size = 1, stack1size = 1, stack2size = 1;
	char *stack = allocate_memory(newTask, stacksize * 4096, PAGE_ALL_PRIVILEG | PAGE_RW);
	char *stack0 = allocate_memory(newTask, stack0size * 4096, PAGE_RW);
	char *stack1 = allocate_memory(newTask, stack1size * 4096, PAGE_RW);
	char *stack2 = allocate_memory(newTask, stack2size * 4096, PAGE_RW);
	uint64 *ltd = (uint64 *)allocate_memory(newTask, 20 * sizeof(uint64), PAGE_ALL_PRIVILEG | PAGE_RW);
	Tableinfo ltdinfo;
	ltdinfo.base = ltd;
	ltdinfo.limit = 0xffff;
	ltdinfo.type = 1;
	TableSegmentItem tempSeg;
	memset_s((char *)&tempSeg, 0, sizeof(TableSegmentItem));

	tempSeg.segmentBaseAddr = 0;
	tempSeg.segmentLimit = STACKLIMIT_G1(stack);
	tempSeg.G = 1;
	tempSeg.D_B = 1;
	tempSeg.P = 1;
	tempSeg.DPL = 3;
	tempSeg.S = 1;
	tempSeg.Type = DATASEG_RW_E;
	newTask->TssData.ss = appendTableSegItem(&ltdinfo, &tempSeg);
	newTask->TssData.esp = (uint32)stack + stacksize * 4096;
	newTask->TssData.esp -= 8;
	*(int *)(newTask->TssData.esp) = 1;
	*(char *)(newTask->TssData.esp + 4) = NULL;

	tempSeg.segmentBaseAddr = 0;
	tempSeg.segmentLimit = 0xfffff;
	tempSeg.Type = CODESEG_X;
	newTask->TssData.cs = appendTableSegItem(&ltdinfo, &tempSeg);
	newTask->TssData.eip = prodata.proEntry;
	tempSeg.segmentBaseAddr = 0;
	tempSeg.segmentLimit = 0xfffff;
	tempSeg.Type = DATASEG_RW;
	newTask->TssData.ds = appendTableSegItem(&ltdinfo, &tempSeg);
	newTask->TssData.es = newTask->TssData.ds;
	newTask->TssData.gs = newTask->TssData.ds;
	newTask->TssData.fs = newTask->TssData.ds;

	tempSeg.segmentBaseAddr = 0;
	tempSeg.segmentLimit = STACKLIMIT_G1(stack0);
	tempSeg.G = 1;
	tempSeg.Type = DATASEG_RW_E;
	tempSeg.DPL = 0;
	newTask->TssData.ss0 = appendTableSegItem(&ltdinfo, &tempSeg);
	newTask->TssData.esp0 = (uint32)stack0 + stack0size * 4096;

	tempSeg.segmentBaseAddr = 0;
	tempSeg.segmentLimit = STACKLIMIT_G1(stack1);
	tempSeg.DPL = 1;
	newTask->TssData.ss1 = appendTableSegItem(&ltdinfo, &tempSeg);
	newTask->TssData.esp1 = (uint32)stack1 + stack1size * 4096;

	tempSeg.segmentBaseAddr = 0;
	tempSeg.segmentLimit = STACKLIMIT_G1(stack2);
	tempSeg.DPL = 2;
	newTask->TssData.ss2 = appendTableSegItem(&ltdinfo, &tempSeg);
	newTask->TssData.esp2 = (uint32)stack2 + stack2size * 4096;

	uint16 ldtSel = 0, tssSel = 0;

	tempSeg.segmentBaseAddr = ltd;
	tempSeg.segmentLimit = ltdinfo.limit;
	tempSeg.G = 0;
	tempSeg.D_B = 1;
	tempSeg.P = 1;
	tempSeg.DPL = 0;
	tempSeg.S = 0;
	tempSeg.Type = LDTSEGTYPE;
	ldtSel = appendTableSegItem(&(kernelData.gdtInfo), &tempSeg);
	tempSeg.segmentBaseAddr = (uint32) & (newTask->TssData);
	tempSeg.segmentLimit = 103;
	tempSeg.DPL = 0;
	tempSeg.Type = TSSSEGTYPE;
	tssSel = appendTableSegItem(&(kernelData.gdtInfo), &tempSeg);
	newTask->TssData.ldtsel = ldtSel;
	newTask->TssData.eflags = flags_data();
	newTask->TssData.ioPermission = sizeof(TssHead) - 1;

	newTask->tssSel = tssSel;
	newTask->taskStats = 0;

	uint32 taskPageDir = (uint32)allocatePhy4kPage(0);

	*(uint32 *)0xFFFFFFF8 = (taskPageDir | 0x7);
	newTask->TssData.cr3 = (taskPageDir);
	resetcr3();
	memcpy_s((char *)0xFFFFE000, (char *)(kernelData.pageDirectory), 4096);
	*(uint32 *)0xFFFFEFFC = (taskPageDir | 0x7);
	*(uint32 *)0xFFFFEFF8 = 0;
	memset_s(0xfffff004, 0, 0xBF8);
	*(uint32 *)0xFFFFFFF8 = 0;
	// resetcr3();
	// setgdtr(&(kernelData.gdtInfo));
	sti_s();
	LOCAL_APIC *xapic_obj = (LOCAL_APIC *)getXapicAddr();
	aparg->logcpucount = 0;
	asm("mfence");
	xapic_obj->ICR1[0] = 0;
	xapic_obj->ICR0[0] = 0x84083; // 更新gdt,cr3
}
*/
thread_t* add_thread(process_t* proc, void(*function)(void*), void* arg)
{

}
void appendNewTcb(TcbList* taskList, TaskCtrBlock* newTcb)
{
	if (taskList->size == 0)
	{
		taskList->tcb_Frist = taskList->tcb_Last = newTcb;
		newTcb->next = newTcb->prior = NULL;
		taskList->size++;
	}
	else {
		newTcb->prior = taskList->tcb_Last;
		newTcb->next = NULL;
		taskList->tcb_Last->next = newTcb;
		taskList->tcb_Last = newTcb;
		taskList->size++;
	}
}
void setCpuTask(ProgramaData *prodata, TaskCtrBlock* newTask, uint32_t pid)
{
}
/*
void testfun()
{
	char *addr = allocatePhy4kPage(16384);
	addr += 2048;
	freePhy4kPage(addr);
}
*/
void initLockBlock(BootParam *bootarg)
{
	uint32_t addr = LOCK_START, size = LOCK_SIZE;
	mem_fix_type_set(addr, size, MEM_UC);
	mem4k_map(addr, addr, MEM_UC, PAGE_G | PAGE_RW);
	lockblock = LOCK_START;
	memset_s(lockblock, 0, size);
	setBit(lockblock->lockstatus, 0);
	lockblock->lockData[0] = 0;

	mem_fix_type_set(ATOMIC_BUFF_ADDR & PAGE_ADDR_MASK, ATOMIC_BUFF_SIZE, MEM_UC);
	mem4k_map(ATOMIC_BUFF_ADDR & PAGE_ADDR_MASK, ATOMIC_BUFF_ADDR & PAGE_ADDR_MASK, MEM_UC, PAGE_G | PAGE_RW);
	memset_s(ATOMIC_BUFF_ADDR, 0, ATOMIC_BUFF_SIZE);
	lockBuff =allocateVirtual4kPage(sizeof(LockObj)*LOCK_COUNT, &(bootarg->kernelAllocateNextAddr), PAGE_G |PAGE_RW);
}
int createLock(LockObj *lobj)
{
	int ret = FALSE;
	asm("cli");
	spinlock(&(lockblock->lockData[0]));
	int i = 1;
	for (; i < MAX_LOCK; i++)
	{
		if (!testBit(lockblock->lockstatus, i))
			break;
	}
	if (i < MAX_LOCK)
	{
		setBit(lockblock->lockstatus, i);
		lobj->index = i;
		lobj->plock = (uint32_t)(&(lockblock->lockData[i]));
		lockblock->lockData[i] = 0;
		ret = TRUE;
	}
	unlock(&(lockblock->lockData[0]));
	asm("sti");
	return ret;
}
void releaseLock(LockObj *lobj)
{
	asm("cli");
	spinlock(&(lockblock->lockData[0]));
	if (lobj->index > 0 && lobj->index < MAX_LOCK)
	{
		resetBit(lockblock->lockstatus, lobj->index);
		lockblock->lockData[lobj->index] = 0;
		lobj->plock = NULL;
	}
	unlock(&(lockblock->lockData[0]));
	asm("sti");
}

void runTask()
{
	LOCAL_APIC* xapic_obj = (LOCAL_APIC*)getXapicAddr();
	uint32_t apid = xapic_obj->ID[0] >> 24;
	while (pCpuCurrentTask[apid] == pEmptyTask[apid]);

	cpuTaskTssdata[apid].pTssdata->cr3 = pCpuCurrentTask[apid]->processdata.context.cr3;
	cpuTaskTssdata[apid].pTssdata->ioPermission = pCpuCurrentTask[apid]->processdata.context.ioPermission;
	cpuTaskTssdata[apid].pTssdata->esp0 = pCpuCurrentTask[apid]->processdata.threads->context.esp0;
	cpuTaskTssdata[apid].pTssdata->eflags = pCpuCurrentTask[apid]->processdata.threads->context.eflags;
	cpuTaskTssdata[apid].pTssdata->esp = pCpuCurrentTask[apid]->processdata.threads->context.esp;
	cpuTaskTssdata[apid].pTssdata->eip = pCpuCurrentTask[apid]->processdata.threads->context.eip;
	pCpuCurrentTask[apid]->processdata.threads->status = RUNNING;
	
	char tasktss[8] = { 0 };
	*(uint32_t*)tasktss = 0;
	*(uint32_t*)(tasktss + 4) = cpuTaskTssdata[apid].tsssel;
	xapic_obj->InitialCount[0] = (processorinfo.processcontent[apid].cpuBusFrequencyLow);//每秒一次任务切换中断
	callTss(tasktss);
}
void APproc(uint32 argv)
{
	setCpuHwp();
	uint32_t eax = 0, edx = 0;
	rdmsr_fence(IA32_APIC_BASE_MSR, &eax, &edx);
	setidtr(&(kernelData.idtInfo));
	setgdtr(&(kernelData.gdtInfo));
	settr(cpuTssdata[argv].tsssel);
	pEmptyTask[argv]->processdata.threads->status = RUNNING;
	pCpuCurrentTask[argv] = pEmptyTask[argv];
	asm("sti");
	// if ((eax & 0x100) == 0) // 判读是否是AP
	{

		printf("AP log =========%d init\n", argv);
		spinlock(lockBuff[KERNEL_LOCK].plock);
		initApic();
		unlock(lockBuff[KERNEL_LOCK].plock);
		processorinfo.processcontent[argv].id = argv;
		processorinfo.processcontent[argv].apicAddr = getXapicAddr();
		// xapic_obj->InitialCount[0] = 0xfffff;
		spinlock(lockBuff[KERNEL_LOCK].plock);
		aparg->logcpucount++;
		unlock(lockBuff[KERNEL_LOCK].plock);
		while (aparg->logcpucount < processorinfo.count)
			;
		timerInit();
		//	xapic_obj->InitialCount[0] = 0xfffff;
		runTask();
		while (1)
		{
			// asm("cli");
			// printf("AP %d empty hlt\n", argv);
			// asm("sti");
			// uint32 waitap = 0xfffff;
			// while (waitap--);
			asm("sti");
			asm("hlt");
		}
	}
}
void *allocUnCacheMem(uint32_t size)
{
	void *ret =NULL;
	asm("cli");
	spinlock(lockBuff[UC_VAR_LOCK].plock);
	if(((uint32_t*)(ATOMIC_BUFF_ADDR))[UC_VAR_LOCK]+size < ATOMIC_BUFF_ADDR+ATOMIC_BUFF_SIZE)
	{
		ret = ((uint32_t*)(ATOMIC_BUFF_ADDR))[UC_VAR_LOCK];
		((uint32_t*)(ATOMIC_BUFF_ADDR))[UC_VAR_LOCK]+=size;
	}
	unlock(lockBuff[UC_VAR_LOCK].plock);
	asm("sti");
	return ret;
}
void ipiUpdateGdtCr3()
{
	LOCAL_APIC *xapic_obj = (LOCAL_APIC *)getXapicAddr();
	uint32_t *pAtomicBuff = ATOMIC_BUFF_ADDR;
	pAtomicBuff[UPDATE_GDT_CR3]=processorinfo.count;
	asm volatile("mfence");
	xapic_obj->ICR1[0] = 0;
	xapic_obj->ICR0[0] = 0x84083; // 更新gdt,cr3
}
void ipiUpdateMtrr()
{
	LOCAL_APIC *xapic_obj = (LOCAL_APIC *)getXapicAddr();
	uint32_t *pAtomicBuff = ATOMIC_BUFF_ADDR;
	pAtomicBuff[MTRR_LOCK]=processorinfo.count;
	asm volatile("mfence");
	xapic_obj->ICR1[0] = 0;
	xapic_obj->ICR0[0] = 0x84084; // 更新mtrr
}
void MPinit(uint32_t currtss)
{
	xapicaddr = XAPIC_START_ADDR;
	logicalID = 0;
#if X2APIC_ENABLE
#else
	printf("map apcode %x :%d\n", AP_CODE_ADDR, mem4k_map(AP_CODE_ADDR, AP_CODE_ADDR, MEM_WB, PAGE_G | PAGE_R));
	// read_ata_sectors(0x4b000, 144, 2);
	uint32_t addr = AP_ARG_ADDR & PAGE_ADDR_MASK, size = 0x1000, temp = 0;
	mem_fix_type_set(addr, size, MEM_UC);
	mem4k_map(addr, addr, MEM_UC, PAGE_G | PAGE_RW);

	memset_s(aparg, 0, sizeof(AParg));
	aparg->entry = APproc;
	aparg->gdt_size = kernelData.gdtInfo.limit;
	aparg->gdt_base = 0xb000;
	aparg->logcpucount = 1;

	LOCAL_APIC *xapic_obj = (LOCAL_APIC *)getXapicAddr();
	mem4k_map((uint32)xapic_obj, (uint32)xapic_obj, MEM_UC, PAGE_RW);

	xapic_obj->ICR1[0] = 0;
	xapic_obj->ICR0[0] = 0xC4500; // 发送Init

	xapic_obj->ICR1[0] = 0;
	xapic_obj->ICR0[0] = 0xC4686; // 发送SIPI AP执行0xbb000处的代码

	uint32 waitap = 0xffffffff;
	while (waitap--)
		;
	memset_s(&processorinfo, 0, sizeof(processorinfo));
	processorinfo.count = aparg->logcpucount;
	processorinfo.processcontent[0].id = 0;


	addr = xapicaddr, size = 0x1000 * (aparg->logcpucount);
	temp = mem_fix_type_set(addr, size, MEM_UC);
	printf("after 0x%x mem cache type %d temp=%d\n", addr, mem_cache_type_get(addr, size), temp);

	initApic();
	processorinfo.processcontent[0].apicAddr = getXapicAddr();

	pCpuCurrentTask = allocate_memory(kernelData.taskList.tcb_Frist, processorinfo.count * sizeof(TaskCtrBlock*), PAGE_G | PAGE_RW);
	memset_s(pCpuCurrentTask, 0, processorinfo.count * sizeof(TaskCtrBlock*));
	pEmptyTask = allocate_memory(kernelData.taskList.tcb_Frist, processorinfo.count * sizeof(TaskCtrBlock*), PAGE_G | PAGE_RW);
	cpuTssdata = allocate_memory(kernelData.taskList.tcb_Frist, processorinfo.count * sizeof(TssPointer), PAGE_G | PAGE_RW);
	memset_s(cpuTssdata, 0, processorinfo.count * sizeof(TssHead*));
	cpuTssdata[0].pTssdata = kernelData.tssdata;
	cpuTssdata[0].tsssel = currtss;
	pEmptyTask[0] = kernelData.taskList.tcb_Frist;
	pCpuCurrentTask[0] = pEmptyTask[0];
	TableSegmentItem tempSeg;
	memset_s((char *)&tempSeg, 0, sizeof(TableSegmentItem));
	for (int i = 1; i < processorinfo.count; i++)
	{
		pEmptyTask[i] = allocate_memory(pEmptyTask[0], sizeof(TaskCtrBlock), PAGE_G | PAGE_RW);
		memset_s(pEmptyTask[i], 0, sizeof(TaskCtrBlock));
		pEmptyTask[i]->processdata.threads = allocate_memory(pEmptyTask[0],sizeof(thread_t), PAGE_G | PAGE_RW);
		memset_s(pEmptyTask[i]->processdata.threads, 0, sizeof(thread_t));

		cpuTssdata[i].pTssdata = allocate_memory(pEmptyTask[0], sizeof(TssHead), PAGE_G | PAGE_RW);

		cpuTssdata[i].pTssdata->ioPermission = sizeof(TssHead) - 1;
		cpuTssdata[i].pTssdata->cr3 = cr3_data();
		tempSeg.segmentBaseAddr = cpuTssdata[i].pTssdata;
		tempSeg.segmentLimit = sizeof(TssHead) - 1;
		tempSeg.G = 0;
		tempSeg.D_B = 1;
		tempSeg.P = 1;
		tempSeg.DPL = 0;
		tempSeg.S = 0;
		tempSeg.Type = TSSSEGTYPE;
		cpuTssdata[i].tsssel = appendTableSegItem(&(kernelData.gdtInfo), &tempSeg);
	}
	uint32 *stackinfo = (uint32 *)(AP_ARG_ADDR + sizeof(AParg)+8);
	printf("processor count =%d\n", aparg->logcpucount);

	memset_s((char *)&tempSeg, 0, sizeof(TableSegmentItem));
	tempSeg.segmentBaseAddr = 0;
	tempSeg.G = 1;
	tempSeg.D_B = 1;
	tempSeg.P = 1;
	tempSeg.DPL = 0;
	tempSeg.S = 1;
	tempSeg.Type = DATASEG_RW_E;

	for (int i = 1; i < aparg->logcpucount; i++)
	{
		char *stack = allocate_memory(pEmptyTask[0], 4*4096, PAGE_RW);
		tempSeg.segmentLimit = STACKLIMIT_G1(stack);
		*stackinfo++ = (uint32)stack + 4 * 4096;
		*stackinfo++ = appendTableSegItem(&(kernelData.gdtInfo), &tempSeg);
	}
	aparg->gdt_size = kernelData.gdtInfo.limit;
	setgdtr(&(kernelData.gdtInfo));
	aparg->logcpucount = 1;
	aparg->jumpok = 1;
	while (aparg->logcpucount < processorinfo.count)
		;

#endif
}
void createUserSegmentDesc()
{
	uint32_t es;
	uint32_t cs;
	uint32_t ss;
	uint32_t ds;
	uint32_t fs;
	uint32_t gs;
	uint32_t ss0;
	TableSegmentItem tempSeg;
	memset_s((char*)&tempSeg, 0, sizeof(TableSegmentItem));

	tempSeg.segmentBaseAddr = 0;
	tempSeg.segmentLimit = STACKLIMIT_G1(USERSTACK_ADDR);
	tempSeg.G = 1;
	tempSeg.D_B = 1;
	tempSeg.P = 1;
	tempSeg.DPL = 3;
	tempSeg.S = 1;
	tempSeg.Type = DATASEG_RW_E;
	ss = appendTableSegItem(&(kernelData.gdtInfo), &tempSeg);

	tempSeg.segmentBaseAddr = 0;
	tempSeg.segmentLimit = 0xfffff;
	tempSeg.Type = CODESEG_X;
	cs = appendTableSegItem(&(kernelData.gdtInfo), &tempSeg);

	tempSeg.segmentBaseAddr = 0;
	tempSeg.segmentLimit = 0xfffff;
	tempSeg.Type = DATASEG_RW;
	ds = appendTableSegItem(&(kernelData.gdtInfo), &tempSeg);
	es = ds;
	gs = ds;
	fs = ds;

	tempSeg.segmentBaseAddr = 0;
	tempSeg.segmentLimit = STACKLIMIT_G1(USERSTACK0_ADDR);
	tempSeg.G = 1;
	tempSeg.Type = DATASEG_RW_E;
	tempSeg.DPL = 0;
	ss0 = appendTableSegItem(&(kernelData.gdtInfo), &tempSeg);

	cpuTaskList = allocate_memory(kernelData.taskList.tcb_Frist, processorinfo.count * sizeof(TcbList), PAGE_G | PAGE_RW);
	memset_s(cpuTaskList, 0, processorinfo.count * sizeof(TcbList));
	cpuTaskTssdata = allocate_memory(kernelData.taskList.tcb_Frist, processorinfo.count * sizeof(TssPointer), PAGE_G | PAGE_RW);
	memset_s(cpuTaskTssdata, 0, processorinfo.count * sizeof(TssHead*));
	memset_s((char*)&tempSeg, 0, sizeof(TableSegmentItem));

	for (int i = 0; i < processorinfo.count; i++)
	{


		cpuTaskTssdata[i].pTssdata = allocate_memory(pEmptyTask[0], sizeof(TssHead), PAGE_G | PAGE_RW);
		memset_s(cpuTaskTssdata[i].pTssdata,0, sizeof(TssHead));
		cpuTaskTssdata[i].pTssdata->es = es;
		cpuTaskTssdata[i].pTssdata->cs = cs;
		cpuTaskTssdata[i].pTssdata->ss = ss;
		cpuTaskTssdata[i].pTssdata->ds = ds;
		cpuTaskTssdata[i].pTssdata->fs = fs;
		cpuTaskTssdata[i].pTssdata->gs = gs;
		cpuTaskTssdata[i].pTssdata->ss0 = ss0;
		cpuTaskTssdata[i].pTssdata->ioPermission = sizeof(TssHead) - 1;
		tempSeg.segmentBaseAddr = cpuTaskTssdata[i].pTssdata;
		tempSeg.segmentLimit = sizeof(TssHead) - 1;
		tempSeg.G = 0;
		tempSeg.D_B = 1;
		tempSeg.P = 1;
		tempSeg.DPL = 0;
		tempSeg.S = 0;
		tempSeg.Type = TSSSEGTYPE;
		cpuTaskTssdata[i].tsssel = appendTableSegItem(&(kernelData.gdtInfo), &tempSeg);
		cpuTaskList[i].pSleepBuff = allocate_memory(pEmptyTask[0], MAXTASKCOUNT * sizeof(SleepTaskNode), PAGE_G | PAGE_RW);
		memset_s((char*)(cpuTaskList[i].pSleepBuff), 0, MAXTASKCOUNT * sizeof(SleepTaskNode));
		cpuTaskList[i].pSleepTaskHead = &(cpuTaskList[i].pSleepBuff[0]);
	}
	setgdtr(&(kernelData.gdtInfo));

}
void testAHCI()
{
	char inputbuff[1024] = {0};
	uint32_t startAddr=0,addrsize=0;
	Physical_entry *entrys=kernel_malloc(sizeof(Physical_entry)*64);
	memset_s(entrys,0,sizeof(Physical_entry)*64);
	uint32_t entryssize=0;
	char *testbuff = kernel_malloc(8192);
	memset_s(testbuff,0,8192);
	//  memset_s(0x3000,0,512);
	//  printf("getdev info:%d \n",get_dev_info(0,0x3000,512));
	//  printf("sector size:%d sector count:%d\n",((uint16_t*)0x3000)[106],*(uint32_t*)(0x3000+117*2));
	while (1)
	{
		asm("cli");
		printf("start sector:");
		asm("sti");
		int len = fgets(inputbuff, 1024);
		inputbuff[len - 1] = 0;
		startAddr =atoi(inputbuff);
		asm("cli");
		printf("sectorcount:");
		asm("sti");
		len = fgets(inputbuff, 1024);
		inputbuff[len - 1] = 0;
		addrsize =atoi(inputbuff);
		asm("cli");
		printf("start sector:0x%x sectorcount:0x%x\n",startAddr,addrsize);
		asm("sti");
		entryssize =768;
		get_memory_map_etc(testbuff,addrsize*512,entrys,&entryssize);
		asm("cli");
		printf("entrySize:%d\n",entryssize);
		asm("sti");
		for(int i=0;i<entryssize;i++)
		{
			asm("cli");
			printf("entry %d addr:0x%x size:0x%x\n",i,entrys[i].address,entrys[i].size);
			asm("sti");
		}
		uint32 srcount = ahci_read(0,startAddr,0,addrsize,testbuff);
		uint32 swcount = ahci_write(0,4,0,addrsize,testbuff);
		asm("cli");
		printf("read seccount:%d wirte seccount:%d\n", srcount,swcount);
		asm("sti");
		
		// asm("cli");
		// printf("buff:%s\n", inputbuff);
		// asm("sti");
		// uint32_t status = ahci_read(0, 0, 0, 1, 0x3000);
		
		// asm("cli");
		// printf("status %d data:%s\n",status,0x3000);
		// asm("sti");
		
		// ahci_write(0, 1, 0, 1, 0x3000);
		// printf("hba port %d is:0x%x ie:0x%x cmd:0x%x  ssts:0x%x sctl:0x%x serr:0x%x sact:0x%x tfd:0x%x ci:%x\n", sataDev[0].port,
		//                        sataDev[0].pPortMem->is, sataDev[0].pPortMem->ie, sataDev[0].pPortMem->cmd, sataDev[0].pPortMem->ssts, sataDev[0].pPortMem->sctl,
		// 					   sataDev[0].pPortMem->serr,sataDev[0].pPortMem->sact, sataDev[0].pPortMem->tfd, sataDev[0].pPortMem->ci);
	}
}
extern void initFs();
extern void testFATfs();


TaskCtrBlock* createProcess(char* filedata, uint32_t sched_priority,int argc,void *argv)
{
	TaskCtrBlock* newTask = (TaskCtrBlock*)kernel_malloc(sizeof(TaskCtrBlock));
	memset_s(newTask, 0, sizeof(TaskCtrBlock));
	newTask->processdata.threads = kernel_malloc(sizeof(thread_t));
	memset_s(newTask->processdata.threads, 0, sizeof(thread_t));

	ProgramaData prodata;
	prodata.vir_end = 0;
	prodata.vir_base = 0xffffffff;
	loadElf(filedata, &prodata, PRIVILEGUSER);

	newTask->pFreeListAddr = (uint32*)allocUnCacheMem(sizeof(uint32));
	TaskFreeMemList* pFreeList = kernel_malloc(sizeof(TaskFreeMemList));
	*(newTask->pFreeListAddr) = pFreeList;
	pFreeList->memAddr = prodata.vir_end + 1;
	pFreeList->next = NULL;
	pFreeList->memSize = USERMALLOCEND - pFreeList->memAddr;

	//uint32_t allocaddr = USERSTACK_ADDR;
	uint32_t stack = USERSTACK_ADDR;// allocateVirtual4kPage(USERSTACK_SIZE, &allocaddr, PAGE_ALL_PRIVILEG | PAGE_RW);
	//allocaddr = USERSTACK0_ADDR;
	uint32_t stack0 = USERSTACK0_ADDR;//allocateVirtual4kPage(USERSTACK0_SIZE, &allocaddr, PAGE_RW);
	for (int stacksize = 0; stacksize < USERSTACK_SIZE_4K; stacksize++)
	{

		mem4k_map(stack, (uint32)allocatePhy4kPage(START_PHY_MEM_PAGE), MEM_WB, PAGE_ALL_PRIVILEG | PAGE_RW);
		mem4k_map(stack0, (uint32)allocatePhy4kPage(START_PHY_MEM_PAGE), MEM_WB, PAGE_RW);
		stack += 0x1000;
		stack0 += 0x1000;
	}


	newTask->processdata.context.ioPermission = sizeof(TssHead) - 1;
	newTask->processdata.threads->sched_priority = 1;
	newTask->processdata.threads->tid = 0;
	newTask->processdata.threads->context.esp = (uint32)USERSTACK_ADDR + USERSTACK_SIZE;
	newTask->processdata.threads->context.esp0 = (uint32)USERSTACK0_ADDR + USERSTACK0_SIZE;
	newTask->processdata.threads->context.eip = prodata.proEntry;
	newTask->processdata.threads->context.eflags = flags_data();
	newTask->processdata.threads->context.esp -= 12;
	newTask->processdata.threads->sched_priority = sched_priority;
	*(int*)(newTask->processdata.threads->context.esp + 4) = argc;
	*(int*)(newTask->processdata.threads->context.esp + 8) = argv;
	newTask->processdata.threads->status = START;

	uint32 taskPageDir = (uint32)allocatePhy4kPage(START_PHY_MEM_PAGE);

	*(uint32*)0xFFFFFFF8 = (taskPageDir | 0x7);
	newTask->processdata.context.cr3 = (taskPageDir);
	resetcr3();
	memcpy_s((char*)0xFFFFE000, (char*)(KERNELPAGEDIR_PHYADDR), 4096);
	*(uint32*)0xFFFFEFFC = (taskPageDir | 0x7);
	*(uint32*)0xFFFFEFF8 = 0;
	*(uint32*)0xFFFFFFF8 = 0;
	memset_s(0xfffff004, 0, 0xBF8);
	resetcr3();

	asm("cli");
	spinlock(lockBuff[CREATE_TASK_LOCK].plock);
	uint32_t pid = (*g_pidIndex)++;
	unlock(lockBuff[CREATE_TASK_LOCK].plock);
	asm("sti");
	uint32_t cpuidnnum = (pid) % processorinfo.count;
	newTask->processdata.pid = pid;
	appendNewTcb(&(cpuTaskList[cpuidnnum]), newTask);
	consolePrintf("pid %d cpuidnnum %d cr3:0x%x currentCr3:0x%x virbase:0x%x virend:0x%x taskaddr:0x%x\n", pid, cpuidnnum, newTask->processdata.context.cr3, cr3_data(), prodata.vir_base, prodata.vir_end, newTask);
	return newTask;
}


void initTask()
{
	FRESULT res;
	FIL fp;
	res = f_open(&fp, "/proc/a.out", FA_OPEN_ALWAYS | FA_READ);
	if (res != FR_OK)
		return;
	uint32_t br = 0, filesize = 0;
	filesize = f_size(&fp);
	char* filedata = kernel_malloc(filesize);
	res = f_read(&fp, filedata, filesize, &br);
	f_close(&fp);
	if (res != FR_OK)
	{
		kernel_free(filedata);
		return;
	}
	for (int i=0;i< 8; i++)
	{
		createProcess(filedata,1,i,NULL);
	}
	kernel_free(filedata);
	ipiUpdateGdtCr3();

	uint32 waitap = 0xfffffff;
	while (waitap--);

	for (int i = 0; i < processorinfo.count; i++)
	{
		pCpuCurrentTask[i] = cpuTaskList[i].tcb_Frist;
		cpuTaskList[i].baseSchedCount = (processorinfo.processcontent[i].cpuBusFrequencyLow / 1000) * MINSCHEDTIME;
		consolePrintf("cpu %d bus frequency high:0x%x  low:0x%x nsPerCycle:%d\n", i, processorinfo.processcontent[i].cpuBusFrequencyHigh, processorinfo.processcontent[i].cpuBusFrequencyLow, processorinfo.processcontent[i].nsCountPerCycle);
	}
}

int _start(void *bargv,void *vbe)
{
	setCpuHwp();
	// clearscreen();
	memcpy_s((char *)&bootparam, (char *)bargv, sizeof(BootParam));
	memcpy_s(g_vbebuff, (char *)vbe, VBE_BUFF_SIZE);
	kernelData.gdtInfo.base = bootparam.gdt_base;
	kernelData.gdtInfo.limit = bootparam.gdt_size;
	kernelData.gdtInfo.type = 0;
	kernelData.pageDirectory = 0xfffff000;
	kernelData.idtInfo.base = bootparam.idtTableAddr;
	kernelData.idtInfo.limit = 0xffff;
	kernelData.idtInfo.type = 0;
	kernelData.taskList.size = 0;
	interrupt8259a_disable();
	createInterruptGate(&kernelData);

	TaskCtrBlock *tcbhead = (TaskCtrBlock *)allocateVirtual4kPage(sizeof(TaskCtrBlock), &(bootparam.kernelAllocateNextAddr), PAGE_G|PAGE_RW);
	memset_s(tcbhead, 0, sizeof(TaskCtrBlock));
	tcbhead->processdata.threads = allocateVirtual4kPage(sizeof(thread_t), &(bootparam.kernelAllocateNextAddr), PAGE_G | PAGE_RW);
	memset_s(tcbhead->processdata.threads, 0, sizeof(thread_t));

	tcbhead->processdata.threads->status = RUNNING;
	kernelData.taskList.tcb_Frist = tcbhead;
	kernelData.taskList.tcb_Last = tcbhead;
	kernelData.taskList.size = 1;

	tcbhead->processdata.context.ioPermission = sizeof(TssHead) - 1;
	tcbhead->processdata.context.cr3 = cr3_data();

	TableSegmentItem tempSeg;
	memset_s((char *)&tempSeg, 0, sizeof(TableSegmentItem));

	kernelData.tssdata = allocateVirtual4kPage(sizeof(TssHead), &(bootparam.kernelAllocateNextAddr), PAGE_G | PAGE_RW);
	memset_s((char*)kernelData.tssdata, 0, sizeof(TssHead));

	tempSeg.segmentBaseAddr = kernelData.tssdata;
	tempSeg.segmentLimit = sizeof(TssHead) - 1;
	tempSeg.G = 0;
	tempSeg.D_B = 1;
	tempSeg.P = 1;
	tempSeg.DPL = 0;
	tempSeg.S = 0;
	tempSeg.Type = TSSSEGTYPE;
	uint16_t tssSel = appendTableSegItem(&(kernelData.gdtInfo), &tempSeg);
	setgdtr(&(kernelData.gdtInfo));
	settr(tssSel);
	createCallGate(&kernelData);
	initLockBlock(&bootparam);
	((uint32_t*)(ATOMIC_BUFF_ADDR))[UC_VAR_LOCK]=ATOMIC_BUFF_ADDR+4*LOCK_COUNT;
	tcbhead->pFreeListAddr = ((uint32_t*)(ATOMIC_BUFF_ADDR))[UC_VAR_LOCK];
	((uint32_t*)(ATOMIC_BUFF_ADDR))[UC_VAR_LOCK]+=4;
	TaskFreeMemList *kernelFreeListHead = allocateVirtual4kPage(sizeof(TaskFreeMemList), &(bootparam.kernelAllocateNextAddr), PAGE_G|PAGE_RW);
	*(tcbhead->pFreeListAddr) = kernelFreeListHead;
	kernelFreeListHead->next = NULL;
	kernelFreeListHead->memAddr = bootparam.kernelAllocateNextAddr;
	kernelFreeListHead->memSize = KERNELMALLOCEND-bootparam.kernelAllocateNextAddr;
	kernelFreeListHead->status = TaskFreeMemListNodeUse;
	createLock(&(lockBuff[KERNEL_LOCK]));
	createLock(&(lockBuff[PRINT_LOCK]));
	createLock(&(lockBuff[MTRR_LOCK]));
	createLock(&(lockBuff[UPDATE_GDT_CR3]));
	createLock(&(lockBuff[AHCI_LOCK]));
	createLock(&(lockBuff[UC_VAR_LOCK]));
	createLock(&(lockBuff[CREATE_TASK_LOCK]));
	//printf("addr1:0x%x addr2:0x%x value:0x%x value2:0x%x\n",bootparam.kernelAllocateNextAddr,tcbhead->AllocateNextAddr,*(tcbhead->AllocateNextAddr),((uint32_t*)(ATOMIC_BUFF_ADDR))[UC_VAR_LOCK]);
	// initScreen();
	//  fontInit();

	// 禁用8259a所有中断

	// testfun();
	check_cpu_features();
	check_mtrr();
	check_pat();

	MPinit(tssSel);
	createUserSegmentDesc();
	cacheMtrrMsrs();

	ipiUpdateMtrr();
	// LOCAL_APIC *xapic_obj = (LOCAL_APIC *)getXapicAddr();
	// // Apic timer task switch
	// xapic_obj->LVT_Timer[0] = 0x82;
	// xapic_obj->DivideConfiguration[0] = 9;

	// spinlock(&(lockBuff[KERNEL_LOCK]));
	// createTask(&(kernelData.taskList), 200, 4);
	// unlock(&(lockBuff[KERNEL_LOCK]));
	// spinlock(&(lockBuff[KERNEL_LOCK]));
	// createTask(&(kernelData.taskList), 250, 4);
	// unlock(&(lockBuff[KERNEL_LOCK]));
	//  xapic_obj->InitialCount[0] = 0xfffff;

	// 	uint32 count = 0;
	// 	while (1)
	// 	{
	// 		asm("cli");
	// 		printf("kernel process.....................%s %d\n", "count =", count++);
	// 		asm("sti");
	// 		uint32 waittime =0xfffff;
	// 		while(waittime--);
	// 		// 给自身处理器发送82h号任务切换
	// #if X2APIC_ENABLE
	// 		eax = 0x82;
	// 		edx = 0;
	// 		wrmsr_fence(IA32_X2APIC_SELF_IPI, eax, edx);
	// #else
	// 		//xapic_obj->ICR1[0] = 0;
	// 		//xapic_obj->ICR0[0] = 0x44082;
	// #endif
	// 	}
	asm("cli");
	initAcpiTable();
	initIoApic();
	checkPciDevice();
	initAHCI();
	asm("sti");

	ipiUpdateGdtCr3(); // 更新gdt,cr3
	uint32 waitap = 0xffffff;
	while (waitap--);
	asm("cli");
	printf("ps2Deviceinit =%d\n", ps2DeviceInit());
	asm("sti");

	initVbe();
	// printf("support:monitor/mwait = %d\n", cpufeatures[cpu_support_monitor_mwait]);

	initFs();
	Rect rect;
	Pair screensize;
	getScreenPixSize(&screensize);
	rect.left = 0;
	rect.right = screensize.x-1;
	rect.top = 0;
	rect.bottom = screensize.y-1;
	fillRect(&rect, 0x707070);
	initConsole();


	g_pidIndex = (uint32*)allocUnCacheMem(sizeof(uint32));
	*g_pidIndex = 0;

	timerInit();


	initSysCall();
	//check_cpuHwp();
	//testFATfs();
	initTask();

	runTask();
	//xapic_obj->ICR1[0] = 0;
	//xapic_obj->ICR0[0] = 0x84082;
	/*
	Bitmap* bitmap = createBitmap32FromBMP24("/img/bg.bmp");
	rect.left = 400;
	rect.top = 400;
	rect.right = bitmap->width - 1 + rect.left;
	rect.bottom = bitmap->height - 1 + rect.left;
	drawBitmap(&rect, bitmap);
	//drawPngImage(&rect,"/img/imgdata");
	rect.left = 256;
	rect.top = 256;
	rect.right = 655;
	rect.bottom = 655;
	fillRect(&rect, 0xffff);

	rect.left = 800;
	rect.top = 800;
	rect.right = 999;
	rect.bottom = 999;
	drawRect(&rect, 0xff,4);

	rect.left = 0;
	rect.top = 0;
	//rect.right = 1019;
	//rect.bottom = 1019;
//	drawRect(&rect, 0xff00, 1);

	//testFATfs();
	bitmap = createBitmap32FromBMP24("/font/font32.bmp");
	rect.left = 600;
	rect.top = 100;
	rect.right = bitmap->width - 1 + rect.left;
	rect.bottom = bitmap->height - 1 + rect.left;
	drawBitmap(&rect, bitmap);
		*/
	while (1)
	{
		// printf("BSP empty\n");
		// waitap= 0xfffff;
		// while (waitap--);
		asm("sti");
		asm("hlt");
	}
}

void TerminateProgram(uint32 retval)
{
	char num[16];
	puts("\nUserProgram Exit code:");
	puts(hexstr32(num, retval));
}

void getCmosDateTime(SYSTEMTIME *datetime)
{
	uint8_t secondBcd = 0;
	uint8_t minuteBcd = 0;
	uint8_t hourBcd = 0;
	uint8_t dayBcd = 0;
	uint8_t monthBcd = 0;
	uint8_t yearBcd = 0;
	//秒
	sysOutChar(0x70,0);
	secondBcd = (uint8_t)sysInChar(0x71);

	//分
	sysOutChar(0x70,2);
	minuteBcd =  (uint8_t)sysInChar(0x71);

	//时
	sysOutChar(0x70,4);
	hourBcd =  (uint8_t)sysInChar(0x71);

	//日
	sysOutChar(0x70,7);
	dayBcd =  (uint8_t)sysInChar(0x71);

	//月
	sysOutChar(0x70,8);
	monthBcd =  (uint8_t)sysInChar(0x71);

	//年
	sysOutChar(0x70,9);
	yearBcd =  (uint8_t)sysInChar(0x71);
	datetime->wSecond =  (((uint8_t)0xf0&secondBcd)>>4)*10+((uint8_t)0xf&secondBcd);
	datetime->wMinute =  (((uint8_t)0xf0&minuteBcd)>>4)*10+((uint8_t)0xf&minuteBcd);
	datetime->wHour =  (((uint8_t)0xf0&hourBcd)>>4)*10+((uint8_t)0xf&hourBcd);
	datetime->wDay =  (((uint8_t)0xf0&dayBcd)>>4)*10+((uint8_t)0xf&dayBcd);
	datetime->wMonth =  (((uint8_t)0xf0&monthBcd)>>4)*10+((uint8_t)0xf&monthBcd);
	datetime->wYear =  (((uint8_t)0xf0&yearBcd)>>4)*10+((uint8_t)0xf&yearBcd);
	datetime->wYear+= 2000;
}

void syncKernel_TaskCr3(TaskCtrBlock* task)
{
	uint32_t taskPageDir = task->processdata.context.cr3;
	*(uint32*)0xFFFFFFF8 = (taskPageDir | 0x7);
	resetcr3();
	memcpy_s((char*)0xFFFFEC00, (char*)(KERNELPAGEDIR_PHYADDR+0xc00),1024);
	*(uint32*)0xFFFFEFFC = (taskPageDir | 0x7);
	*(uint32*)0xFFFFEFF8 = 0;
	*(uint32*)0xFFFFFFF8 = 0;
	resetcr3();
}
void syncTask_KernelCr3(TaskCtrBlock* task)
{
	uint32_t taskPageDir = task->processdata.context.cr3;
	*(uint32*)0xFFFFFFF8 = (taskPageDir | 0x7);
	resetcr3();
	uint32_t val = *(uint32_t*)(KERNELPAGEDIR_PHYADDR + 0xffc);
	memcpy_s((char*)(KERNELPAGEDIR_PHYADDR + 0xc00), (char*)0xFFFFEC00, 1024);
	*(uint32_t*)(KERNELPAGEDIR_PHYADDR + 0xffc) = val;
	*(uint32_t*)(KERNELPAGEDIR_PHYADDR + 0xff8) = 0;
	*(uint32*)0xFFFFFFF8 = 0;
	resetcr3();
}

extern uint32_t getCPUbusfrequencyLocalApicTimer(uint32_t apicaddr, uint32_t* datahigh, uint32_t* datalow);
void timerInit()
{
	uint32_t eax = 0, ebx = 0, ecx = 0, edx = 0;
	cpuidcall(6, &eax, &ebx, &ecx, &edx);
//	consolePrintf("apictimer cpuid 6 eax:0x%x\n", eax);
//	cpuidcall(1, &eax, &ebx, &ecx, &edx);
//	consolePrintf("apictimer cpuid 1 ecx:0x%x edx:0x%x\n", ecx,edx);
//	cpuidsubcall(7,0, &eax, &ebx, &ecx, &edx);
//	consolePrintf("apictimer cpuid 7 edx:0x%x\n", edx);
	LOCAL_APIC* xapic_obj = (LOCAL_APIC*)getXapicAddr();
	uint32_t apid = xapic_obj->ID[0] >> 24;
	xapic_obj->LVT_Timer[0] = 0x82;
	xapic_obj->DivideConfiguration[0] = 0xb;//不分频率
	//APIC不分频计算cpu bus frequency
	getCPUbusfrequencyLocalApicTimer(xapic_obj, &edx, &eax);

	processorinfo.processcontent[apid].cpuBusFrequencyLow = eax;
	processorinfo.processcontent[apid].cpuBusFrequencyHigh = edx;
	processorinfo.processcontent[apid].nsCountPerCycle = (((uint32_t)1000000000 / eax));
	xapic_obj->LVT_Timer[0] = 0x82;//一次性模式
	//xapic_obj->LVT_Timer[0]=0x20082; //周期模式
	xapic_obj->DivideConfiguration[0] = 0xb;
}
#include "boot.h"
#include "osdataPhyAddr.h"
#include "syscall.h"
#include "interruptGate.h"
#include "elf.h"
#include "string.h"
#include "printf.h"
#include "apic.h"
#include "cpufeature.h"
#include "memcachectl.h"
#include "screen.h"
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
TaskCtrBlock **procCurrTask = NULL;
LockObj *lockBuff = NULL;
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
TaskCtrBlock *createNewTcb(TcbList *taskList)
{
	TaskCtrBlock *newTcb = (TaskCtrBlock *)allocate_memory(taskList->tcb_Frist, sizeof(TaskCtrBlock), PAGE_G | PAGE_RW);
	memset_s(newTcb, 0, sizeof(TaskCtrBlock));
	// newTcb->prior = taskList->tcb_Last;
	taskList->tcb_Last->next = newTcb;
	newTcb->next = taskList->tcb_Frist;
	taskList->tcb_Last = newTcb;
	taskList->size++;
	return newTcb;
}

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
void APproc(uint32 argv)
{
	setCpuHwp();
	uint32_t eax = 0, edx = 0;
	rdmsr_fence(IA32_APIC_BASE_MSR, &eax, &edx);
	setidtr(&(kernelData.idtInfo));
	setgdtr(&(kernelData.gdtInfo));
	settr(procCurrTask[argv]->tssSel);
	procCurrTask[argv]->taskStats = 1;
	asm("sti");
	// if ((eax & 0x100) == 0) // 判读是否是AP
	{

		printf("AP log =========%d init\n", argv);
		spinlock(lockBuff[KERNEL_LOCK].plock);
		initApic();
		unlock(lockBuff[KERNEL_LOCK].plock);
		processorinfo.processcontent[argv].id = argv;
		processorinfo.processcontent[argv].apicAddr = getXapicAddr();
		LOCAL_APIC *xapic_obj = (LOCAL_APIC *)(processorinfo.processcontent[argv].apicAddr);
		xapic_obj->LVT_Timer[0] = 0x82;
		xapic_obj->DivideConfiguration[0] = 9;
		// xapic_obj->InitialCount[0] = 0xfffff;
		spinlock(lockBuff[KERNEL_LOCK].plock);
		aparg->logcpucount++;
		unlock(lockBuff[KERNEL_LOCK].plock);
		while (aparg->logcpucount < processorinfo.count)
			;
		//	xapic_obj->InitialCount[0] = 0xfffff;
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
void MPinit()
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
	processorinfo.processcontent[0].apicAddr = getXapicAddr();

	addr = xapicaddr, size = 0x1000 * (aparg->logcpucount);
	temp = mem_fix_type_set(addr, size, MEM_UC);
	printf("after 0x%x mem cache type %d temp=%d\n", addr, mem_cache_type_get(addr, size), temp);

	initApic();

	procCurrTask = allocate_memory(kernelData.taskList.tcb_Frist, processorinfo.count * sizeof(TaskCtrBlock *), PAGE_G | PAGE_RW);
	procCurrTask[0] = kernelData.taskList.tcb_Frist;
	procCurrTask[0]->taskStats = 1;
	TableSegmentItem tempSeg;
	memset_s((char *)&tempSeg, 0, sizeof(TableSegmentItem));
	for (int i = 1; i < processorinfo.count; i++)
	{
		procCurrTask[i] = allocate_memory(procCurrTask[0], sizeof(TaskCtrBlock), PAGE_G | PAGE_RW);
		memset_s(procCurrTask[i], 0, sizeof(TaskCtrBlock));
		procCurrTask[i]->TssData.ioPermission = sizeof(TssHead) - 1;
		procCurrTask[i]->TssData.cr3 = cr3_data();
		tempSeg.segmentBaseAddr = &(procCurrTask[i]->TssData);
		tempSeg.segmentLimit = sizeof(TssHead) - 1;
		tempSeg.G = 0;
		tempSeg.D_B = 1;
		tempSeg.P = 1;
		tempSeg.DPL = 0;
		tempSeg.S = 0;
		tempSeg.Type = TSSSEGTYPE;
		procCurrTask[i]->tssSel = appendTableSegItem(&(kernelData.gdtInfo), &tempSeg);
		kernelData.taskList.tcb_Last->next = procCurrTask[i];
		procCurrTask[i]->next = kernelData.taskList.tcb_Frist;
		kernelData.taskList.tcb_Last = procCurrTask[i];
		kernelData.taskList.size++;
	}
	uint32 *stackinfo = (uint32 *)(AP_ARG_ADDR + sizeof(AParg));
	printf("processor count =%d\n", aparg->logcpucount);

	memset_s((char *)&tempSeg, 0, sizeof(TableSegmentItem));
	tempSeg.segmentBaseAddr = 0;
	tempSeg.G = 1;
	tempSeg.D_B = 1;
	tempSeg.P = 1;
	tempSeg.DPL = 0;
	tempSeg.S = 1;
	tempSeg.Type = DATASEG_RW_E;
	for (int i = 0; i < aparg->logcpucount; i++)
	{
		char *stack = allocate_memory(procCurrTask[0], 4 * 4096, PAGE_RW);
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
	tcbhead->next = tcbhead;
	tcbhead->taskStats = 1;
	kernelData.taskList.tcb_Frist = tcbhead;
	kernelData.taskList.tcb_Last = tcbhead;
	kernelData.taskList.size = 1;
	kernelData.nextTask = NULL;
	tcbhead->TssData.ioPermission = sizeof(TssHead) - 1;
	tcbhead->TssData.cr3 = cr3_data();
	TableSegmentItem tempSeg;
	memset_s((char *)&tempSeg, 0, sizeof(TableSegmentItem));
	tempSeg.segmentBaseAddr = &(tcbhead->TssData);
	tempSeg.segmentLimit = sizeof(TssHead) - 1;
	tempSeg.G = 0;
	tempSeg.D_B = 1;
	tempSeg.P = 1;
	tempSeg.DPL = 0;
	tempSeg.S = 0;
	tempSeg.Type = TSSSEGTYPE;
	tcbhead->tssSel = appendTableSegItem(&(kernelData.gdtInfo), &tempSeg);
	setgdtr(&(kernelData.gdtInfo));
	settr(tcbhead->tssSel);
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

	//printf("addr1:0x%x addr2:0x%x value:0x%x value2:0x%x\n",bootparam.kernelAllocateNextAddr,tcbhead->AllocateNextAddr,*(tcbhead->AllocateNextAddr),((uint32_t*)(ATOMIC_BUFF_ADDR))[UC_VAR_LOCK]);
	// initScreen();
	//  fontInit();

	// 禁用8259a所有中断

	// testfun();
	check_cpu_features();
	check_mtrr();
	check_pat();

	MPinit();
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
	uint32 waitap = 0xfffffff;
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
	//check_cpuHwp();
	testFATfs();
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

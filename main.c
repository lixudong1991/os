#include "boot.h"
#include "syscall.h"
#include "interruptGate.h"
#include "elf.h"
#include "printf.h"
#include "apic.h"
#include "cpufeature.h"
#define STACKLIMIT_G1(a)   ((((uint32)(a))-1)>>12) //gdt 表项粒度为1的段界限

BootParam bootparam;
KernelData kernelData;

char* hexstr32(char buff[9],uint32 val)
{
	char hexs[]="0123456789ABCDEF";
	memset_s(buff,'0',8);
	buff[8]=0;
	uint32 v = val;
	for(int index=7;index>-1&&v!=0;index--)
	{
		buff[index]=hexs[v%16];
		v/=16;
	}
	return buff;
}
char* hexstr64(char buff[17],uint64 val)
{
	char hexs[]="0123456789ABCDEF";
	memset_s(buff,'0',16);
	buff[16]=0;
	uint64 v = val;
	for(int index=15;index>-1&&v!=0;index--)
	{
		buff[index]=hexs[v%16];
		v/=16;
	}
	return buff;
}

static void createCallGate(KernelData *kdata)
{
	uint32 index =0;
	uint16 gateSel =0;
	TableGateItem item;
	memset_s(&item,0,sizeof(TableGateItem));
	
	item.Type = CALL_GATE_TYPE;
	item.segSelect = cs_data();
	item.segAddr = puts_s;
	item.argCount = 1;
	item.GateDPL = 3;
	item.P=1;
	gateSel = appendTableGateItem(&(kdata->gdtInfo),&item);
	strcpy_s(kdata->gateInfo[index].gateName,"puts_s");
	kdata->gateInfo[index].gateSelect = gateSel;
	kdata->gateInfo[index].gateAddr = puts_s;
	kdata->gataSize=++index;
	
	item.segSelect = cs_data();
	item.segAddr = clearScreen_s;
	item.argCount = 0;
	item.GateDPL = 3;
	item.P=1;
	gateSel = appendTableGateItem(&(kdata->gdtInfo),&item);
	strcpy_s(kdata->gateInfo[index].gateName,"clearScreen_s");
	kdata->gateInfo[index].gateSelect = gateSel;
	kdata->gateInfo[index].gateAddr = clearScreen_s;
	kdata->gataSize=++index;
	
	item.segSelect = cs_data();
	item.segAddr = setcursor_s;
	item.argCount = 1;
	item.GateDPL = 3;
	item.P=1;
	gateSel = appendTableGateItem(&(kdata->gdtInfo),&item);
	strcpy_s(kdata->gateInfo[index].gateName,"setcursor_s");
	kdata->gateInfo[index].gateSelect = gateSel;
	kdata->gateInfo[index].gateAddr = setcursor_s;
	kdata->gataSize=++index;
	
	item.segSelect = cs_data();
	item.segAddr = readSectors_s;
	item.argCount = 3;
	item.GateDPL = 3;
	item.P=1;
	gateSel = appendTableGateItem(&(kdata->gdtInfo),&item);
	strcpy_s(kdata->gateInfo[index].gateName,"readSectors_s");
	kdata->gateInfo[index].gateSelect = gateSel;
	kdata->gateInfo[index].gateAddr = readSectors_s;
	kdata->gataSize=++index;
	
	item.segSelect = cs_data();
	item.segAddr = exit_s;
	item.argCount = 1;
	item.GateDPL = 3;
	item.P=1;
	gateSel = appendTableGateItem(&(kdata->gdtInfo),&item);
	strcpy_s(kdata->gateInfo[index].gateName,"exit_s");
	kdata->gateInfo[index].gateSelect = gateSel;
	kdata->gateInfo[index].gateAddr = exit_s;
	kdata->gataSize=++index;
}
static void createInterruptGate(KernelData* kdata)
{
	TableGateItem item;
	memset_s(&item, 0, sizeof(TableGateItem));

	item.Type = INTERRUPT_GATE_TYPE;
	item.segSelect = cs_data();
	item.argCount = 0;
	item.GateDPL = 0;
	item.P = 1;
	int i = 0;
	for (;i<20;i++)
	{
		item.segAddr = exceptionCalls[i];
		appendTableGateItem(&(kdata->idtInfo),&item);
	}
	for (;i<0x20;i++)
		appendTableGateItem(&(kdata->idtInfo), &item);
	i = 0x20;
	item.segAddr = interrupt_8259a_handler;
	for (;i<0x27;i++)
		appendTableGateItem(&(kdata->idtInfo), &item);
	item.segAddr = general_interrupt_handler;
	for (;i<0x70;i++)
		appendTableGateItem(&(kdata->idtInfo), &item);
	item.segAddr = interrupt_70_handler;
	appendTableGateItem(&(kdata->idtInfo), &item);
	i++;
	item.segAddr = interrupt_8259a_handler;
	for (; i < 0x78; i++)
		appendTableGateItem(&(kdata->idtInfo), &item);
	for (;i<256;i++)
	{
		if (i == 0x80)
		{
			item.segAddr = systemCall;
			item.GateDPL = 3;
		}
		else if(i == 0x81)
		{
			item.segAddr = local_x2apic_error_handling;
			item.GateDPL = 0;	
		}
		else if(i == 0x82)
		{
			item.segAddr = ApicTimeOut;		
		}else
			item.segAddr = general_interrupt_handler;

		appendTableGateItem(&(kdata->idtInfo), &item);
	}

	setidtr(&(kdata->idtInfo));
	sti_s();
}
TaskCtrBlock* createNewTcb(TcbList* taskList)
{
	TaskCtrBlock* newTcb = (TaskCtrBlock*)allocate_memory(taskList->tcb_Frist,sizeof(TaskCtrBlock),  PAGE_G|PAGE_RW);
	memset_s(newTcb, 0, sizeof(TaskCtrBlock));
	newTcb->prior = taskList->tcb_Last;
	taskList->tcb_Last->next = newTcb;
	taskList->tcb_Last = newTcb;
	taskList->size++;
	return newTcb;
}

void createTask(TcbList *taskList,int taskStartSection,int SectionCount)
{
	cli_s();
	TaskCtrBlock* newTask = createNewTcb(taskList);
	char* elfdata = allocate_memory(taskList->tcb_Frist,SectionCount*512, PAGE_RW);
	read_ata_sectors(elfdata, taskStartSection, SectionCount);

	ProgramaData prodata;
	prodata.vir_end = 0;
	prodata.vir_base = 0xffffffff;
	loadElf(elfdata,&prodata, PRIVILEGUSER);
	newTask->AllocateNextAddr = prodata.vir_end + 1;
	uint32 stacksize = 1,stack0size=1, stack1size = 1, stack2size = 1;
	char* stack  = allocate_memory(newTask, stacksize*4096, PAGE_ALL_PRIVILEG | PAGE_RW);
	char* stack0 = allocate_memory(newTask, stack0size *4096,   PAGE_RW);
	char* stack1 = allocate_memory(newTask, stack1size*4096,  PAGE_RW);
	char* stack2 = allocate_memory(newTask, stack2size*4096,  PAGE_RW);
	uint64* ltd = (uint64*)allocate_memory(newTask,20 * sizeof(uint64), PAGE_ALL_PRIVILEG  | PAGE_RW);
	Tableinfo ltdinfo;
	ltdinfo.base = ltd;
	ltdinfo.limit = 0xffff;
	ltdinfo.type = 1;
	TableSegmentItem tempSeg;
	memset_s((char*)&tempSeg, 0, sizeof(TableSegmentItem));

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
	*(int*)(newTask->TssData.esp) = 1;
	*(char*)(newTask->TssData.esp+4) = NULL;


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

	tempSeg.segmentBaseAddr =0;
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
	tempSeg.DPL =0;
	tempSeg.S = 0;
	tempSeg.Type = LDTSEGTYPE;
	ldtSel = appendTableSegItem(&(kernelData.gdtInfo), &tempSeg);
	tempSeg.segmentBaseAddr =(uint32)&(newTask->TssData);
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
	
	*(uint32*)0xFFFFFFF8 = (taskPageDir|0x7);
	newTask->TssData.cr3 = (taskPageDir);
	resetcr3();
	memcpy_s((char*)0xFFFFE000, (char*)(kernelData.pageDirectory), 4096);
	*(uint32*)0xFFFFEFFC = (taskPageDir | 0x7);
	*(uint32*)0xFFFFEFF8 = 0;
	memset_s(0xfffff004, 0, 0xBF8);
	*(uint32*)0xFFFFFFF8 = 0;
	resetcr3();
	setgdtr(&(kernelData.gdtInfo));
	sti_s();
}
/*
void testfun()
{
	char *addr = allocatePhy4kPage(16384);
	addr += 2048;
	freePhy4kPage(addr);
}
*/
int _start(BootParam *argv)
{
	clearscreen();
	printf("aaaaabbccccdddd%d %d\r\n",4444,555);
	memcpy_s((char*)&bootparam,(char*)argv,sizeof(BootParam));
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

	TaskCtrBlock* tcbhead = (TaskCtrBlock*)allocateVirtual4kPage(sizeof(TaskCtrBlock), &(bootparam.kernelAllocateNextAddr),PAGE_RW);
	memset_s(tcbhead, 0, sizeof(TaskCtrBlock));
	kernelData.taskList.tcb_Frist = tcbhead;
	kernelData.taskList.tcb_Last = tcbhead;
	kernelData.taskList.size = 1;
	kernelData.currentTask = tcbhead;
	tcbhead->AllocateNextAddr = bootparam.kernelAllocateNextAddr;
	tcbhead->taskStats = 1;
	tcbhead->TssData.ioPermission = sizeof(TssHead) - 1;
	tcbhead->TssData.cr3 = cr3_data();
	TableSegmentItem tempSeg;
	memset_s((char*)&tempSeg, 0, sizeof(TableSegmentItem));
	tempSeg.segmentBaseAddr = &(tcbhead->TssData);
	tempSeg.segmentLimit = sizeof(TssHead)-1;
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
	//禁用8259a所有中断

	// char number[32];
	// puts("\r\nbus:");
	// puts(hexstr32(number, bootparam.bus));
	// puts("\r\nslot:");
	// puts(hexstr32(number, bootparam.slot));
	// puts("\r\nvendor:");
	// puts(hexstr32(number, bootparam.vendor));
	// puts("\r\ndevice:");
	// puts(hexstr32(number, bootparam.device));
	// puts("\r\nbar5:");
	// puts(hexstr32(number, bootparam.bar5));
	// puts("\r\nReadAddress:");
	// puts(hexstr32(number, bootparam.ReadAddress));
	
	//char* buff = allocateVirtual4kPage(1,(char**)&(bootparam.kernelAllocateNextAddr),   PAGE_RW);
	/*read_ata_sectors(buff,150,1);
	buff[512] = 0;
	puts("\r\n");
	puts(buff);*/
	//callTss(kernelData.taskList.tcb_Last->tssSel);
	//testfun();
	check_cpu_features();
	check_mtrr();
    uint32_t eax=0,ebx=0,ecx=0,edx=0;
	printf("cr4: 0x%x\r\n",cr4_data());	
	eax = cr0_data();
	printf("cr0_data: 0x%x\r\n",eax);
	eax=0,edx=0;
	rdmsrcall(IA32_APIC_BASE_MSR,&eax,&edx);
	printf("before enable x2apic msr: low 32:0x%x high 32:%x\r\n",eax,edx);
	enablingx2APIC();
	eax=edx=0;
	rdmsrcall(IA32_APIC_BASE_MSR,&eax,&edx);
	printf("after enable x2apic msr: low 32:0x%x high 32:%x\r\n",eax,edx);
	rdmsrcall(IA32_X2APIC_APICID,&eax,&edx);
    printf("Local x2APIC ID:.0x%x\r\n",eax);
	rdmsrcall(IA32_X2APIC_VERSION,&eax,&edx);
	printf("Local x2APIC Version:0x%x\r\n",eax); 
	rdmsrcall(IA32_X2APIC_LDR,&eax,&edx);//Logical x2APIC ID = [(x2APIC ID[19:4] « 16) | (1 « x2APIC ID[3:0])]
	printf("Logical Destination:0x%x\r\n",eax);

	//rtc_8259a_enable();

	//设置并初始化Local Apic error 中断向量为 0x81
    eax =0x81;
	edx =0;
	wrmsr_fence(IA32_X2APIC_LVT_ERROR,eax,edx);
	eax=edx=0;
	wrmsr_fence(IA32_X2APIC_ESR,eax,edx);

	//给自身处理器发送80h号中断测试
	eax =0x80;
	edx =0;
	wrmsr_fence(IA32_X2APIC_SELF_IPI,eax,edx);
	
	//IPI测试
	eax = 0xC4080; //vector =0x80  level = 1 tirgger =0  Destination Shorthand =All Excluding Self
	edx =0xffffffff;
	wrmsr_fence(IA32_X2APIC_ICR,eax,edx);  


	//Apic timer task switch
	eax = 0x82; //vector =0x78  Timer Mode =  Periodic    Not Masked
	edx =0;
	wrmsr_fence(IA32_X2APIC_LVT_TIMER,eax,edx); 
	eax = 0x9; //Divide Configuration 101: Divide by 64
	edx =0;
	wrmsr_fence(IA32_X2APIC_DIV_CONF,eax,edx); 

	// eax = 0x000fffff; //Initial Count 
	// edx =0;
	// wrmsr_fence(IA32_X2APIC_INIT_COUNT,eax,edx); 
	
	// createTask(&(kernelData.taskList),200,4);
	// createTask(&(kernelData.taskList), 250,4);
    // uint32 count = 0;
	// while (1)
	// {
	//     printf("kernel process.....................%s %d\r\n","count =", count++);
	// 	//给自身处理器发送82h号任务切换
	// 	eax =0x82;
	// 	edx =0;
	// 	wrmsr_fence(IA32_X2APIC_SELF_IPI,eax,edx);
	// }
	while (1) 
	{
		asm("sti");
		asm("hlt");
	}	
}

void TerminateProgram(uint32 retval)
{
	char num[16];
	puts("\r\nUserProgram Exit code:");
	puts(hexstr32(num,retval));
}


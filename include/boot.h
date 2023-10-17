#ifndef BOOT_H_H_
#define BOOT_H_H_

#include "ctype.h"
#include "stdint.h"

#define _INTSIZEOF(n) ((sizeof(n) + sizeof(int) - 1) & ~(sizeof(int) - 1))
#define _ADDRESSOF(v) (&(v))

#define va_start _crt_va_start
#define va_arg _crt_va_arg
#define va_end _crt_va_end

#define _crt_va_start(ap, v) (ap = (va_list)_ADDRESSOF(v) + _INTSIZEOF(v))
#define _crt_va_arg(ap, t) (*(t *)((ap += _INTSIZEOF(t)) - _INTSIZEOF(t)))
#define _crt_va_end(ap) (ap = (va_list)0)

#pragma pack(1)

typedef struct MemInfo
{
	uint64 BaseAddr;
	uint64 Length;
	uint32 type;

} MemInfo;

typedef struct DriveParametersPacket
{
	uint16 InfoSize;		// 数据包尺寸 (26 字节)
	uint16 Flags;			// 信息标志
	uint32 Cylinders;		// 磁盘柱面数
	uint32 Heads;			// 磁盘磁头数
	uint32 SectorsPerTrack; // 每磁道扇区数
	uint64 Sectors;			// 磁盘总扇区数
	uint16 SectorSize;		// 扇区尺寸 (以字节为单位)
} DriveParametersPacket;

#define MEMINFOMAXSIZE 60

typedef struct BootParam
{
	uint32 entry;
	uint32 vir_base;
	uint32 vir_end;
	uint32 phy_cs;
	uint32 phy_ds;
	uint16 EBDAseg;
	uint16 gdt_size;
	uint32 gdt_base;
	uint32 pageStatusAddr;
	uint32 idtTableAddr;
	uint32 kernelAllocateNextAddr;
	uint16 memInfoSize;
	MemInfo meminfo[MEMINFOMAXSIZE];
	DriveParametersPacket diskParam;
} BootParam;

typedef struct AParg
{
	uint32 entry;
	uint16 jumpok;
	uint16 gdt_size;
	uint32 gdt_base;
	uint32 logcpumenutex;
	uint32 logcpucount;
}AParg;

typedef struct  ProcessorContent
{
	uint32 id;
	uint32 apicAddr;	
}ProcessorContent;

#define PROCESSOR_MAX_COUNT  8

typedef struct ProcessorInfo{
	uint32 count;
	ProcessorContent processcontent[PROCESSOR_MAX_COUNT];
}ProcessorInfo;

typedef struct Tableinfo
{
	short limit;
	uint32 base;
	uint16 type;
} Tableinfo;

typedef struct GateInfo
{
	uint16 gateSelect;
	uint32 gateAddr;
	char gateName[34];
} GateInfo;
typedef struct TssHead
{
	uint32 priorTssSel;
	uint32 esp0;
	uint32 ss0;
	uint32 esp1;
	uint32 ss1;
	uint32 esp2;
	uint32 ss2;
	uint32 cr3;
	uint32 eip;
	uint32 eflags;
	uint32 eax;
	uint32 ecx;
	uint32 edx;
	uint32 ebx;
	uint32 esp;
	uint32 ebp;
	uint32 esi;
	uint32 edi;
	uint32 es;
	uint32 cs;
	uint32 ss;
	uint32 ds;
	uint32 fs;
	uint32 gs;
	uint32 ldtsel;
	uint16 empty;
	uint16 ioPermission;
} TssHead;
#define TaskFreeMemListNodeUse     1
#define TaskFreeMemListNodeUnUse   0
typedef struct TaskFreeMemList
{
	struct TaskFreeMemList* next;
	uint32_t memAddr;
	uint32_t memSize;
	uint32_t status;
}
TaskFreeMemList;
typedef struct TaskCtrBlock
{
	struct TaskCtrBlock *next;
	struct TaskCtrBlock *prior;
	uint32 *pFreeListAddr;
	uint32 tssSel;
	uint32 taskStats;
	TssHead TssData;
} TaskCtrBlock;

typedef struct TcbList
{
	TaskCtrBlock *tcb_Frist;
	TaskCtrBlock *tcb_Last;
	uint32 size;
} TcbList;
typedef struct ProgramaData
{
	uint32 proEntry;
	uint32 vir_base;
	uint32 vir_end;
} ProgramaData;
#define MAX_GATECOUNT 16
typedef struct KernelData
{
	Tableinfo gdtInfo;
	uint32 *pageDirectory;
	Tableinfo idtInfo;
	TcbList taskList;
	TaskCtrBlock *nextTask;
	uint32 gataSize;
	GateInfo gateInfo[MAX_GATECOUNT];
} KernelData;
typedef struct ProgramHead
{
	uint32 size;
	uint32 headlen;
	uint32 stackSel;
	uint32 stackLen;
	uint32 entry;
	uint32 codeBase;
	uint32 codeLen;
	uint32 dataBase;
	uint32 dataLen;
	uint32 saltCount;
} ProgramHead;

typedef struct Physical_entry
{
	phys_addr_t	address;	/* address in physical memory */
	phys_size_t	size;		/* size of block */
}Physical_entry;
#pragma pack()

#define DATASEG_R 0
#define DATASEG_RW 2
#define DATASEG_R_E 4
#define DATASEG_RW_E 6

#define CODESEG_X 8
#define CODESEG_XR 10
#define CODESEG_X_C 12
#define CODESEG_XR_C 14

#define LDTSEGTYPE 2
#define TSSSEGTYPE 9

typedef struct TableSegmentItem
{
	uint32 segmentBaseAddr;
	uint32 segmentLimit;
	char G;
	char D_B;
	char L;
	char AVL;
	char P;
	char DPL;
	char S;
	char Type;
} TableSegmentItem;

#define CALL_GATE_TYPE 0xC0000000000
#define INTERRUPT_GATE_TYPE 0xE0000000000
#define SNARE_GATE_TYPE 0xF0000000000

typedef struct TableGateItem
{
	uint64 Type;
	uint16 segSelect;
	char argCount;
	char GateDPL;
	uint32 segAddr;
	short P;
} TableGateItem;

void setgdtr(Tableinfo *info);
void setldtr(uint32 ldtrSel);
void settr(uint32 trSel);
void setidtr(Tableinfo *info);

uint32 cs_data();
uint32 ds_data();
uint32 ss_data();
uint32 fs_data();
uint32 gs_data();
uint32 esp_data();
uint32 cr3_data();
uint32 flags_data();
void resetcr3();

extern uint32 cr0_data();
extern void set_cr0data(uint32 data);
extern uint32 cr4_data();
extern void set_cr4data(uint32 data);

extern int cpuidcall(uint32 callnum, uint32 *eax, uint32 *ebx, uint32 *ecx, uint32 *edx);
extern int rdmsrcall(uint32 msrid, uint32 *eax, uint32 *edx);
extern int wrmsrcall(uint32 msrid, uint32 eax, uint32 edx);
extern int rdmsr_fence(uint32 msrid, uint32 *eax, uint32 *edx);
extern int wrmsr_fence(uint32 msrid, uint32 eax, uint32 edx);

extern uint32_t sysInLong(uint32_t port);
extern void sysOutLong(uint32_t port, uint32_t val);
extern uint32_t sysInChar(uint32_t port);
extern void sysOutChar(uint32_t port, uint32_t val);

void setBit(uint32 *addr, uint32 nr);
void resetBit(uint32 *addr, uint32 nr);
uint32 testBit(uint32 *addr, uint32 nr);

void setds(uint32 segSel);
void setgs(uint32 segSel);
void setfs(uint32 segSel);
void callTss(uint32 *addr);
void intcall();

void invlpg_s(uint32 *tlbitem);
void cli_s();
void sti_s();

// int sprintf(char *buf, const char *fmt, ...);
// int vsprintf(char *buf, const char *fmt, va_list args);
// int printf(const char *fmt, ...);

extern void die();
extern void clearscreen();
extern void setcursor(uint32 pos);
extern uint32 getcursor();
char *allocate_memory(TaskCtrBlock *task, uint32 size, uint32 prop);//4字节对齐
void free_memory(TaskCtrBlock* task,void *addr);//4字节对齐
char *allocate_memory_align(TaskCtrBlock *task, uint32 size, uint32 prop,uint32 alignsize);
char* realloc_memory(TaskCtrBlock* task, uint32_t addr, uint32 size, uint32 prop);

extern char *allocatePhy4kPage(uint32 startPhyPage);
extern uint32 freePhy4kPage(uint32 page);
#define PAGE_R 0
#define PAGE_RW 2

#define PAGE_G 0x100

#define PAGE_ALL_PRIVILEG 4

void kassert( int expression );
char *allocateVirtual4kPage(uint32 size, uint32 *pAddr, uint32 prop);

int mem4k_map(uint32 linearaddr,uint32 phyaddr,int memcachType,uint32 prop);
int mem4k_unmap(uint32 linearaddr,int isFreePhyPage);
int get_4kpage_phyaddr(phys_addr_t linearaddr,phys_addr_t *phyaddr);
int get_memory_map_etc( phys_addr_t address, size_t numBytes,Physical_entry* table, uint32* _numEntries);


void* kernel_malloc(uint32 size);
void* kernel_malloc_align(uint32 size,uint32 alignsize);
void  kernel_free(void*);
void *kernel_realloc(void *mem_address, unsigned int newsize);

uint16 appendTableSegItem(Tableinfo *info, TableSegmentItem *item);
BOOL getTableSegItem(Tableinfo *info, TableSegmentItem *item, uint16 SegSelect);
uint16 appendTableGateItem(Tableinfo *info, TableGateItem *item);

TaskCtrBlock *createNewTcb(TcbList *taskList);

void TerminateProgram(uint32 retval);

extern void rtc_8259a_enable();
extern void interrupt8259a_disable();

// 地址范围必须是write-back type
extern uint32 _monitor(void *addr, uint32 extensions, uint32 hints);
extern uint32 _mwait(uint32 extensions, uint32 hints);

extern uint32 pre_mtrr_change();
extern void post_mtrr_change(uint32 cr4data);

void ipiUpdateGdtCr3();
void ipiUpdateMtrr();


#define MAX_LOCK 512
typedef struct LockBlock
{
	char lockstatus[MAX_LOCK/8];
	uint32 lockData[MAX_LOCK];
}LockBlock;

typedef struct LockObj
{
	uint32 index;
	uint32 plock;
}LockObj;

enum Lock_ID
{
	KERNEL_LOCK =0,
	PRINT_LOCK,
	MTRR_LOCK,
	UPDATE_GDT_CR3,
	AHCI_LOCK,
	UC_VAR_LOCK,
	LOCK_COUNT
};

void *allocUnCacheMem(uint32_t size);
void initLockBlock(BootParam *bootparam);
int createLock(LockObj *lobj);
extern int spinlock(uint32 *lobj);
extern int unlock(uint32 *lobj);
void releaseLock(LockObj *lobj);
extern LockObj *lockBuff;

typedef struct _SYSTEMTIME {
    WORD wYear;
    WORD wMonth;
    WORD wDayOfWeek;
    WORD wDay;
    WORD wHour;
    WORD wMinute;
    WORD wSecond;
    WORD wMilliseconds;
} SYSTEMTIME;

void getCmosDateTime(SYSTEMTIME *datetime);
#endif
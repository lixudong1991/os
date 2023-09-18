#include "acpi.h"
#include "boot.h"
#include "memcachectl.h"
#include "printf.h"
uint32_t *AcpiTableAddrs;
IoApicEntry **Madt_IOAPIC;
LocalApicEntry **Madt_LOCALAPIC;
uint8_t Madt_LOCALAPIC_count=0;
uint8_t Madt_IOAPIC_count=0;
extern BootParam bootparam;
static void getnext_KMP(const char *dest, int next[], int len)
{
    int i = 0, j = -1;
    next[0] = j;
    while (i < len - 1)
    {
        if (j == -1 || dest[i] == dest[j])
        {
            ++i;
            ++j;
            if (dest[i] != dest[j])
            {
                next[i] = j;
            }
            else
                next[i] = next[j];
        }
        else
            j = next[j];
    }
}
static int IndexStr_KMP(char *str, int strsize, const char *dest, int *nextbuff, int destsize)
{
    int len = destsize;
    int *next = nextbuff;
    memset_s(next, 0, len * sizeof(int));
    getnext_KMP(dest, next, len);
    int size1 = strsize;
    int i = 0, j = 0;
    while (i < size1 && j < len)
    {
        if (j == -1 || str[i] == dest[j])
        {
            ++i;
            ++j;
        }
        else
        {
            j = next[j];
        }
    }

    if (j == len)
        return i - len;
    return -1;
}

char *findRSDPAddr(uint32 startaddr, uint32 endaddr)
{
    uint32 findstart = startaddr & 0xfffff000, findend = endaddr & 0xfffff000;
    printf("findstart:%x findend:%x \r\n", startaddr, findend);
    for (int i = findstart; i < findend; i += 0x1000)
    {
        mem4k_map(i, i, MEM_UC, PAGE_G | PAGE_R);
    }
    char *ret = NULL;
    char rsdpsig[] = "RSD PTR ";
    int nextbuff[8] = {0};
    int rsdtindex = IndexStr_KMP(startaddr, endaddr - startaddr, rsdpsig, nextbuff, 8);
    if (rsdtindex != -1)
        ret = (char *)(startaddr + rsdtindex);
    return ret;
}
/*
static char madtEntryIdLen[]={8,//0 Processor Local APIC structure
                         12,//1 I/O APIC structure
                         10,//2 Interrupt Source Override
                         8,//3 NMI Source
                         6,//4 Local APIC NMI Structure
                         12,//5 Local APIC Address Override Structure
                         16,//6 I/O SAPIC Structure
                         1,// 7 Processor Local SAPIC structure
                         16,//8 Platform Interrupt Source structure
                         16,//9 Processor Local x2APIC structure
                         12,//0AH Local x2APIC NMI Structure
                         82,//0xB GICC structure
                         24,//0xC GICD structure
                         24,//0xD GIC MSI Frame structure
                         16,//0xE GICR structure
                         20,//0xF GIC ITS structure
                         16,//0x10 Multiprocessor Wakeup structure
                        };
                         //1,0x11 Core Programmable Interrupt Controller Structure
                       //  1,//0x12 Legacy I/O Programmable Interrupt Controller Structure
                      //   1,//0x13 HT Programmable Interrupt Controller Structure
                       //  1,//0x14 Extend I/O Programmable Interrupt Controller Structure
                      //   1,//0x15 Message Programmable Interrupt Controller Structure
                      //   1,//0x16 Bridge I/O Programmable Interrupt Controller Structure
*/
void readMADTInfo(uint32 madtaddr)
{
    char sign[9] = {0};
    SysDtHead *pmadt = madtaddr;
    mem4k_map(madtaddr & 0xfffff000, madtaddr & 0xfffff000, MEM_UC, PAGE_G | PAGE_R);
    mem4k_map((madtaddr + pmadt->Length - 1) & 0xfffff000, (madtaddr + pmadt->Length - 1) & 0xfffff000, MEM_UC, PAGE_G | PAGE_R);
    sign[0] = pmadt->Signature[0];
    sign[1] = pmadt->Signature[1];
    sign[2] = pmadt->Signature[2];
    sign[3] = pmadt->Signature[3];
    sign[4] = 0;
    uint32 localIntCtl = *(uint32 *)(madtaddr + 36), flags = *(uint32 *)(madtaddr + 40);
    printf("madt:length=%d sign=%s localIntCtl =0x%x  flags = 0x%x\r\n", pmadt->Length, sign, localIntCtl, flags);
    uint32 eindex = 0;
    char *pentry = madtaddr + 44;
    uint8_t entrytype;
    for (; eindex < pmadt->Length;)
    {
        entrytype = pentry[eindex];
        switch (entrytype)
        {
        case 0:
        {
            LocalApicEntry *loapic = pentry + eindex;
            printf("Loapic: ACPI Processor UID=%d APICID=%d Flags=%d\r\n", loapic->ACPI_Processor_UID, loapic->APIC_ID, loapic->Flags);
            Madt_LOCALAPIC[Madt_LOCALAPIC_count]=loapic;
            Madt_LOCALAPIC_count++;
        }
        break;
        case 1:
        {
            IoApicEntry *ioapic = pentry + eindex;
            printf("IOapic: IOAPIC_ID=%d I/O_APIC_Address=0x%x GlobalSystemInterruptBase=%d\r\n",
                   ioapic->IO_APIC_ID, ioapic->IO_APIC_Address, ioapic->Global_System_Interrupt_Base);
            Madt_IOAPIC[Madt_IOAPIC_count]=ioapic;
            Madt_IOAPIC_count++;   
        }
        case 2:
        {
            IntSourceOverride *isoverr = pentry + eindex;
             printf(": Interrupt Source Override:BUS=%d SOURCE=%d GlobalSystemInterrupt=%d Flags=%d\r\n",
                   isoverr->Bus,isoverr->Source, isoverr->G_Sys_int,isoverr->Flags);
        }
        break;
        default:
            break;
        }
        eindex += pentry[eindex + 1];
    }
}
void readAcpiTable(RSDPStruct *prsdp)
{
    AcpiTableAddrs[RSDP]=prsdp;
    char rsign[9] = {0};
    printf("RSDP addr:0x%x\r\n", prsdp);
    memcpy_s(rsign, prsdp->Signature, 8);
    printf("RSDP Signature:%s\r\n", rsign);
    // printf("RSDP Checksum:%x\r\n",prsdp->Checksum);
    memcpy_s(rsign, prsdp->OEMID, 6);
    rsign[6] = 0;
    printf("RSDP OEMID:%s\r\n", rsign);
    printf("RSDP Revision:%x\r\n", prsdp->Revision);
    // printf("RSDP RsdtAddress:%x\r\n",prsdp->RsdtAddress);
    if (prsdp->Revision == 2)
    {
        AcpiTableAddrs[XSDT]=(uint32_t)(prsdp->XsdtAddress);
        uint32_t eax = 0, mapaddr = 0;
        eax = (uint32_t) & (prsdp->XsdtAddress);
        printf("RSDP XsdtAddress:%x %x\r\n", *(uint32 *)(eax + 4), *(uint32 *)eax);
        // printf("RSDP Extended Checksum:%x\r\n",prsdp->ExtendedChecksum);
        SysDtHead *pxsdt = (uint32_t)(prsdp->XsdtAddress);
        mapaddr = pxsdt;
        mapaddr &= 0xfffff000;
        mem4k_map(mapaddr, mapaddr, MEM_UC, PAGE_G | PAGE_RW);
        mem4k_map(mapaddr + 0x1000, mapaddr + 0x1000, MEM_UC, PAGE_G | PAGE_RW);
        memcpy_s(rsign, pxsdt->Signature, 4);
        rsign[4] = 0;
        printf("XSDT Signature:%s\r\n", rsign);
        printf("XSDT Length:%x\r\n", pxsdt->Length);
        printf("XSDT Revision:%x\r\n", pxsdt->Revision);
        memcpy_s(rsign, pxsdt->OEMID, 6);
        rsign[6] = 0;
        printf("XSDT OEMID:%s\r\n", rsign);
        memcpy_s(rsign, pxsdt->OEM_TABLE_ID, 8);
        rsign[8] = 0;
        printf("XSDT OEM Table ID:%s\r\n", rsign);
        uint64 *acpitable = (uint32_t)(prsdp->XsdtAddress) + sizeof(SysDtHead);
        int tablecount = (pxsdt->Length - sizeof(SysDtHead)) / 8;
        SysDtHead *ptable = NULL;
        uint32_t fadtaddr = 0, mcfgaddr = 0, madtaddr = 0;
        for (int tableindex = 0; tableindex < tablecount; tableindex++)
        {
            ptable = (uint32_t)(acpitable[tableindex]);
            mem4k_map((uint32_t)(acpitable[tableindex]) & 0xfffff000, (uint32_t)(acpitable[tableindex]) & 0xfffff000, MEM_UC, PAGE_G | PAGE_RW);
            rsign[0] = ptable->Signature[0];
            rsign[1] = ptable->Signature[1];
            rsign[2] = ptable->Signature[2];
            rsign[3] = ptable->Signature[3];
            rsign[4] = 0;
            printf("%s ", rsign);
            if (ptable->Signature[0] == 'F' && ptable->Signature[1] == 'A' && ptable->Signature[2] == 'C' && ptable->Signature[3] == 'P')
                fadtaddr = ptable;
            else if (ptable->Signature[0] == 'M' && ptable->Signature[1] == 'C' && ptable->Signature[2] == 'F' && ptable->Signature[3] == 'G')
            {
                mcfgaddr = ptable;
            }
            else if (ptable->Signature[0] == 'A' && ptable->Signature[1] == 'P' && ptable->Signature[2] == 'I' && ptable->Signature[3] == 'C')
            {
                madtaddr = ptable;
            }
        }
        printf("\r\n");
        printf("FADT IAPC_BOOT_ARCH: %x\r\n", *(uint16_t *)(fadtaddr + 109));
        printf("MCFG addr: %x\r\n", mcfgaddr);
        printf("MADT addr: %x\r\n", madtaddr);
        AcpiTableAddrs[FADT]=(uint32_t)(fadtaddr);
        AcpiTableAddrs[MCFG]=(uint32_t)(mcfgaddr);
        AcpiTableAddrs[MADT]=(uint32_t)(madtaddr);
        readMADTInfo(madtaddr);
    }
}

void initAcpiTable()
{
	char *rsdpaddr = NULL;
    AcpiTableAddrs = kernel_malloc(ACPITYPECOUNT*sizeof(uint32));
    Madt_IOAPIC = kernel_malloc(MAX_IOAPIC_COUNT*sizeof(IoApicEntry *));
    Madt_LOCALAPIC = kernel_malloc(MAX_LOAPIC_COUNT*sizeof(LocalApicEntry *));
    memset_s(AcpiTableAddrs,0,ACPITYPECOUNT*sizeof(uint32));
    printf("Madt_IOAPIC %d\r\n",sizeof(IoApicEntry *));
    memset_s(Madt_IOAPIC,0,sizeof(IoApicEntry *)*MAX_IOAPIC_COUNT);
    printf("Madt_LOCALAPIC %d\r\n",sizeof(LocalApicEntry *));
    memset_s(Madt_LOCALAPIC,0,MAX_LOAPIC_COUNT*sizeof(LocalApicEntry *));
#if 1	
	
	for (int i = 0; i < bootparam.memInfoSize; i++)
	{
		if (bootparam.meminfo[i].type == 4)
		{
			rsdpaddr = findRSDPAddr(bootparam.meminfo[i].BaseAddr, bootparam.meminfo[i].BaseAddr + bootparam.meminfo[i].Length);
			if (rsdpaddr)
				break;
		}
	}
#else
	rsdpaddr = findRSDPAddr(0xe0000,0x100000);
#endif
	if (rsdpaddr)
	{
		readAcpiTable(rsdpaddr);
	}
	else
		printf("not find RSDP\r\n");

}
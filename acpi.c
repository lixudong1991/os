#include "acpi.h"
#include "boot.h"
#include "memcachectl.h"
#include "printf.h"
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

char *findRSDPAddr()
{
    for (int i = FIND_RSDP_START; i < FIND_RSDP_END; i += 0x1000)
    {
        mem4k_map(i, i, MEM_UC, PAGE_G | PAGE_RW);
    }
    char rsdpsig[] = "RSD PTR ";
    int nextbuff[8] = {0};
    int rsdtindex = IndexStr_KMP(FIND_RSDP_START, FIND_RSDP_END - FIND_RSDP_START, rsdpsig, nextbuff, 8);
    char *ret = NULL;
    if (rsdtindex != -1)
        ret = (char *)(FIND_RSDP_START + rsdtindex);
    return ret;
}
static madtEntryIdLen[]={8,//0 Processor Local APIC structure
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
                         1,//0x11 Core Programmable Interrupt Controller Structure
                         1,//0x12 Legacy I/O Programmable Interrupt Controller Structure
                         1,//0x13 HT Programmable Interrupt Controller Structure
                         1,//0x14 Extend I/O Programmable Interrupt Controller Structure
                         1,//0x15 Message Programmable Interrupt Controller Structure
                         1,//0x16 Bridge I/O Programmable Interrupt Controller Structure
                        };

void readMADTInfo(uint32 madtaddr)
{
    char sign[9] = {0};
    SysDtHead *pmadt = madtaddr;
    mem4k_map(madtaddr & 0xfffff000, madtaddr & 0xfffff000, MEM_UC, PAGE_G | PAGE_RW);
    mem4k_map((madtaddr + pmadt->Length-1) & 0xfffff000, (madtaddr + pmadt->Length-1) & 0xfffff000, MEM_UC, PAGE_G | PAGE_RW);
    sign[0] = pmadt->Signature[0];
    sign[1] = pmadt->Signature[1];
    sign[2] = pmadt->Signature[2];
    sign[3] = pmadt->Signature[3];
    sign[4] = 0;
    uint32 localIntCtl = *(uint32*)(madtaddr+36),flags =*(uint32*)(madtaddr+40);
    printf("madt:length=%d sign=%s localIntCtl =0x%x  flags = 0x%x\r\n",pmadt->Length,sign,localIntCtl,flags);
    uint32 eindex=0;
    char *pentry = madtaddr+44;
    // for(;eindex<pmadt->Length;)
    // {

    // }

}
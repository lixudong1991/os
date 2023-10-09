#include "fat32.h"
#include "boot.h"
#include "ahci.h"
#include "printf.h"
#define FS_START_LBA 0x800
#define FS_TOTSEC_COUNT 0x4000000-FS_START_LBA
static FAT32_BPB_Struct *pBpbinfo =NULL;
static VolumeInfo volumeInfo;
static MbrPartition partioninfo;
#define TRACE_FS
#ifdef TRACE_FS
#define TRACEFS(...) printf(__VA_ARGS__)
#else
#define TRACEFS(...)
#endif
void formatFat32()
{
    char bpbbuff[512];
    memset_s(bpbbuff,0,512);
    bpbbuff[510] =0x55;
    bpbbuff[511] =0xAA;
    FAT32_BPB_Struct *bpbstruct=bpbbuff;
    memcpy_s(bpbstruct->BS_OEMName,"MSWIN4.1",8);
    bpbstruct->BPB_BytsPerSec = 512;
    bpbstruct->BPB_SecPerClus = 8;
    bpbstruct->BPB_RsvdSecCnt =32;
    bpbstruct->BPB_NumFATs =2;
    bpbstruct->BPB_RootEntCnt = 0;
    bpbstruct->BPB_TotSec16 =0;
    bpbstruct->BPB_Media = 0xf8;
    bpbstruct->BPB_FATSz16=0;
    bpbstruct->BPB_SecPerClus = 0;
    bpbstruct->BPB_NumHeads = 0;
    bpbstruct->BPB_HiddSec = 0;
    bpbstruct->BPB_TotSec32 = FS_TOTSEC_COUNT;

    bpbstruct->BPB_FATSz32 = 65471;
    bpbstruct->BPB_ExtFlags = 0;
    bpbstruct->BPB_FSVer = 0;
    bpbstruct->BPB_RootClus =2;
    bpbstruct->BPB_FSInfo =1;
    bpbstruct->BPB_BkBootSec =6;
    bpbstruct->BS_DrvNum = 0x80;
    bpbstruct->BS_BootSig = 0x29;
    memcpy_s(bpbstruct->BS_VolLab,"NO NAME    ",11);
    memcpy_s(bpbstruct->BS_FilSysType,"FAT32   ",8);

    uint32 swcount = ahci_write(0,FS_START_LBA,0,1,bpbbuff);
    char fatbuff[512];
    memset_s(bpbbuff,0,512);
    uint32_t fat1startlba=FS_START_LBA+bpbstruct->BPB_RsvdSecCnt,fat2startlba=FS_START_LBA+bpbstruct->BPB_RsvdSecCnt+bpbstruct->BPB_FATSz32;
    for(int i=0;i<bpbstruct->BPB_FATSz32;i++)
    {
        ahci_write(0,fat1startlba++,0,1,fatbuff);
        ahci_write(0,fat2startlba++,0,1,fatbuff);
    }
    *(uint32_t*)fatbuff = 0x0FFFFFF8;
    *(uint32_t*)(fatbuff+4) = 0xFFFFFFFF;
    *(uint32_t*)(fatbuff+8) = 0x0FFFFFFF;
    fat1startlba=FS_START_LBA+bpbstruct->BPB_RsvdSecCnt,fat2startlba=FS_START_LBA+bpbstruct->BPB_RsvdSecCnt+bpbstruct->BPB_FATSz32;
    ahci_write(0,fat1startlba,0,1,fatbuff);
    ahci_write(0,fat2startlba,0,1,fatbuff);
}
void initFS()
{
    pBpbinfo  = kernel_malloc(sizeof(FAT32_BPB_Struct));
    char bpbbuff[512];
    memset_s(bpbbuff,0,512);
    uint32 swcount = ahci_read(0,0,0,1,bpbbuff);
    memcpy_s(&partioninfo,bpbbuff+446,sizeof(MbrPartition));
    swcount = ahci_read(0,partioninfo.fsStartLBA,0,1,bpbbuff);
    memcpy_s(pBpbinfo,bpbbuff,sizeof(FAT32_BPB_Struct));
    // asm("cli");
    // TRACEFS("ahci_read fs:%d\n",swcount);
    // TRACEFS("ahci_read BPB_RsvdSecCnt:0x%x\n",pBpbinfo->BPB_RsvdSecCnt);
    // TRACEFS("ahci_read BPB_FATSz32:0x%x\n",pBpbinfo->BPB_FATSz32);
    // TRACEFS("ahci_read BPB_TotSec32:0x%x\n",pBpbinfo->BPB_TotSec32);
    // asm("sti");
    volumeInfo.RootDirSectors = 0;
    volumeInfo.FirstDataSector = partioninfo.fsStartLBA+pBpbinfo->BPB_RsvdSecCnt+pBpbinfo->BPB_NumFATs*pBpbinfo->BPB_FATSz32;
    volumeInfo.DataSec =pBpbinfo->BPB_TotSec32-(pBpbinfo->BPB_RsvdSecCnt+pBpbinfo->BPB_NumFATs*pBpbinfo->BPB_FATSz32);
    volumeInfo.CountofClusters = volumeInfo.DataSec/pBpbinfo->BPB_SecPerClus;
    asm("cli");
    TRACEFS("FS FirstDataSector:0x%x DataSec:0x%x CountofClusters:0x%x\n",volumeInfo.FirstDataSector,volumeInfo.DataSec,volumeInfo.CountofClusters);
    asm("sti");
}
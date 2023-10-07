#include "fat32.h"
#include "boot.h"
#define FS_START_LBA 0x800
#define FS_TOTSEC_COUNT 0x4000000-FS_START_LBA
void formatFat32()
{
    FAT32_BPB_Struct bpbstruct;
    memset_s(&bpbstruct,0,sizeof(FAT32_BPB_Struct));
    memcpy_s(bpbstruct.BS_OEMName,"MSWIN4.1",8);
    bpbstruct.BPB_BytsPerSec = 512;
    bpbstruct.BPB_SecPerClus = 8;
    bpbstruct.BPB_RsvdSecCnt =32;
    bpbstruct.BPB_NumFATs =2;
    bpbstruct.BPB_RootEntCnt = 0;
    bpbstruct.BPB_TotSec16 =0;
    bpbstruct.BPB_Media = 0xf8;
    bpbstruct.BPB_FATSz16=0;
    bpbstruct.BPB_SecPerClus = 0;
    bpbstruct.BPB_NumHeads = 0;
    bpbstruct.BPB_HiddSec = 0;
    bpbstruct.BPB_TotSec32 = FS_TOTSEC_COUNT;

    //设1个FAT占用x个扇区,则x满足不等式  1024*x>= FS_TOTSEC_COUNT-(32+256*x) ;


}
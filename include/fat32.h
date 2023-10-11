#ifndef FAT_32_H_H
#define FAT_32_H_H

#include "stdint.h"
#pragma pack(1)
typedef struct MbrPartition
{
    uint8_t fsActive;
    uint8_t fsStartHead;
    uint16_t fsStartCylSect;
    uint8_t fsPartType;
    uint8_t fsEndHead;
    uint16_t fsEndCylSect;
    uint32_t fsStartLBA;
    uint32_t fsSize;
}MbrPartition;

typedef struct FAT32_BPB_Struct
{
    uint8_t BS_jmpBoot[3];
    uint8_t BS_OEMName[8];
    uint16_t BPB_BytsPerSec;
    uint8_t BPB_SecPerClus;
    uint16_t BPB_RsvdSecCnt;
    uint8_t BPB_NumFATs;
    uint16_t BPB_RootEntCnt;
    uint16_t BPB_TotSec16;
    uint8_t BPB_Media;
    uint16_t BPB_FATSz16;
    uint16_t BPB_SecPerTrk;
    uint16_t BPB_NumHeads;
    uint32_t BPB_HiddSec;
    uint32_t BPB_TotSec32;
    
    uint32_t BPB_FATSz32;
    uint16_t BPB_ExtFlags;
    uint16_t BPB_FSVer;
    uint32_t BPB_RootClus;
    uint16_t BPB_FSInfo ;
    uint16_t BPB_BkBootSec;
    uint8_t  BPB_Reserved[12];
    uint8_t  BS_DrvNum;
    uint8_t  BS_Reserved1;
    uint8_t  BS_BootSig;
    uint32_t BS_VolID;
    uint8_t  BS_VolLab[11];
    uint8_t  BS_FilSysType[8];
}FAT32_BPB_Struct;

typedef struct Fat32Fsinfo{
    uint32_t FSI_LeadSig;
    char FSI_Reserved1[480];
    uint32_t FSI_StrucSig;
    uint32_t FSI_Free_Count;
    uint32_t FSI_Nxt_Free;
    char FSI_Reserved2[12];
    uint32_t FSI_TrailSig; 
}Fat32Fsinfo;

typedef struct Fat32EntryInfo{
    char DIR_Name[11];
    uint8_t DIR_Attr;
    uint8_t DIR_NTRes;
    uint8_t DIR_CrtTimeTenth;
    uint16_t DIR_CrtTime;
    uint16_t DIR_CrtDate;
    uint16_t DIR_LstAccDate;
    uint16_t DIR_FstClusHI;  
    uint16_t DIR_WrtTime;
    uint16_t DIR_WrtDate;
    uint16_t DIR_FstClusLO;
    uint32_t DIR_FileSize; 
}Fat32EntryInfo;

typedef struct Fat32LongEntryInfo{
    uint8_t LDIR_Ord;
    char LDIR_Name1[10];
    uint8_t LDIR_Attr;
    uint8_t LDIR_Type; 
    uint8_t LDIR_Chksum;
    char LDIR_Name2[12];
    uint16_t LDIR_FstClusLO;
    char LDIR_Name3[4]; 
}Fat32LongEntryInfo;
#pragma pack()

void formatFat32();

void initFS();

typedef struct VolumeInfo
{
    uint32_t RootDirSectors;
    uint32_t FirstDataSector;
    uint32_t DataSec;
    uint32_t CountofClusters;
}VolumeInfo;



typedef struct FsNode{
    struct FsNode *pri;
    struct FsNode *next;
    char *descbuff;
    uint32_t descsize;
}FsNode;

#define ATTR_READ_ONLY  ((uint8_t)0x01)
#define ATTR_HIDDEN  ((uint8_t)0x02)
#define ATTR_SYSTEM  ((uint8_t)0x04)
#define ATTR_VOLUME_ID  ((uint8_t)0x08)
#define ATTR_DIRECTORY  ((uint8_t)0x10)
#define ATTR_ARCHIVE  ((uint8_t)0x20)
#define ATTR_LONG_NAME  ((uint8_t)(ATTR_READ_ONLY | ATTR_HIDDEN | ATTR_SYSTEM | ATTR_VOLUME_ID))

#define ATTR_LONG_NAME_MASK  ((uint8_t)(ATTR_READ_ONLY | ATTR_HIDDEN |ATTR_SYSTEM |ATTR_VOLUME_ID |ATTR_DIRECTORY |ATTR_ARCHIVE))


int get_dir_item_count(const char *dirpath);
int _get_dir_item_count(uint32_t firstclusternum);
int _get_dir_item_descsize(uint32_t firstclusternum,uint32_t itemIndex);
int _get_dir_item_descdata(uint32_t firstclusternum,uint32_t itemIndex,char *descbuff,uint32_t descbuffsize);

#endif
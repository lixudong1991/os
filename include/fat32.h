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

#endif
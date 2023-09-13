#ifndef ACPI_H_H_
#define ACPI_H_H_
#define FIND_RSDP_START 0xE0000
#define FIND_RSDP_END   0x100000
#include "stdint.h"
#pragma pack(1)

typedef struct RSDPStruct
{
    char Signature[8];
    uint8_t Checksum;
    char OEMID[6];
    uint8_t  Revision;
    uint32_t RsdtAddress;
    uint32_t Length;
    uint64_t XsdtAddress;
    uint8_t ExtendedChecksum;
    char Reserved[3];
}RSDPStruct;

typedef struct SysDtHead
{
    char Signature[4];
    uint32_t Length;
    uint8_t  Revision;
    uint8_t Checksum;
    char OEMID[6];
    char OEM_TABLE_ID[8];
    uint32_t OEM_Revision;
    uint32_t CreatorID;
    uint32_t CreatorRevision;
}SysDtHead;

#pragma pack(4)

char *findRSDPAddr();

void readMADTInfo(uint32 madtaddr);


#endif
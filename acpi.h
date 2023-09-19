#ifndef ACPI_H_H_
#define ACPI_H_H_
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

typedef struct LocalApicEntry
{
    uint8_t Type;
    uint8_t Length;
    uint8_t ACPI_Processor_UID;
    uint8_t APIC_ID;
    uint32_t Flags;
}LocalApicEntry;
typedef struct IoApicEntry
{
    uint8_t Type;
    uint8_t Length;
    uint8_t IO_APIC_ID;
    uint8_t Reserved;
    uint32_t IO_APIC_Address;
    uint32_t Global_System_Interrupt_Base;
}IoApicEntry;

typedef struct IntSourceOverride {
    uint8_t Type;
    uint8_t Length;
    uint8_t Bus;
    uint8_t Source;
    uint32_t G_Sys_int;
    uint16_t Flags;
}IntSourceOverride;

typedef struct pciConfigSpaceBaseAddr{
    uint64_t BaseAddr;
    uint16_t PciSegGroup;
    uint8_t StartPCIbus;
    uint8_t EndPCIbus;
    uint32_t reserved;
}pciConfigSpaceBaseAddr;

#pragma pack()

void initAcpiTable();

enum ACPI_TABLE_TYPE
{
GAS=0,
RSDP,
RSDT,
XSDT,
FADT,
FACS,
DSDT,
SSDT,
MADT,
GICC,
SBST,
ECDT,
SLIT,
SRAT,
CPEP,
MSCT,
RASF,
RAS2,
MPST,
PMTT,
BGRT,
FPDT,
GTDT,
NFIT,
HMAT,
PDTT,
PPTT,
MCFG,
ACPITYPECOUNT
};

#define MAX_IOAPIC_COUNT 32
#define MAX_LOAPIC_COUNT 32
extern uint32_t *AcpiTableAddrs;
extern IoApicEntry **Madt_IOAPIC;
extern LocalApicEntry **Madt_LOCALAPIC;
extern uint8_t Madt_LOCALAPIC_count;
extern uint8_t Madt_IOAPIC_count;
#endif
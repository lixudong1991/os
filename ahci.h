#ifndef AHCI_H_H
#define AHCI_H_H
#include "stdint.h"

#pragma pack(1)
typedef volatile struct tagHBA_PORT
{
    DWORD   clb;        // 0x00, command list base address, 1K-byte aligned
    DWORD   clbu;       // 0x04, command list base address upper 32 bits
    DWORD   fb;     // 0x08, FIS base address, 256-byte aligned
    DWORD   fbu;        // 0x0C, FIS base address upper 32 bits
    DWORD   is;     // 0x10, interrupt status
    DWORD   ie;     // 0x14, interrupt enable
    DWORD   cmd;        // 0x18, command and status
    DWORD   rsv0;       // 0x1C, Reserved
    DWORD   tfd;        // 0x20, task file data
    DWORD   sig;        // 0x24, signature
    DWORD   ssts;       // 0x28, SATA status (SCR0:SStatus)
    DWORD   sctl;       // 0x2C, SATA control (SCR2:SControl)
    DWORD   serr;       // 0x30, SATA error (SCR1:SError)
    DWORD   sact;       // 0x34, SATA active (SCR3:SActive)
    DWORD   ci;     // 0x38, command issue
    DWORD   sntf;       // 0x3C, SATA notification (SCR4:SNotification)
    DWORD   fbs;        // 0x40, FIS-based switch control
    DWORD   devslp;     //0x44
    DWORD   rsv1[10];   // 0x48 ~ 0x6F, Reserved
    DWORD   vendor[4];  // 0x70 ~ 0x7F, vendor specific
} HBA_PORT;

typedef volatile struct tagHBA_MEM
{
    // 0x00 - 0x2B, Generic Host Control
    DWORD   cap;        // 0x00, Host capability
    DWORD   ghc;        // 0x04, Global host control
    DWORD   is;     // 0x08, Interrupt status
    DWORD   pi;     // 0x0C, Port implemented
    DWORD   vs;     // 0x10, Version
    DWORD   ccc_ctl;    // 0x14, Command completion coalescing control
    DWORD   ccc_pts;    // 0x18, Command completion coalescing ports
    DWORD   em_loc;     // 0x1C, Enclosure management location
    DWORD   em_ctl;     // 0x20, Enclosure management control
    DWORD   cap2;       // 0x24, Host capabilities extended
    DWORD   bohc;       // 0x28, BIOS/OS handoff control and status
 
    // 0x2C - 0x9F, Reserved
    BYTE    rsv[0xA0-0x2C];
 
    // 0xA0 - 0xFF, Vendor specific registers
    BYTE    vendor[0x100-0xA0];
 
    // 0x100 - 0x10FF, Port control registers
    HBA_PORT    ports[1];   // 1 ~ 32
} HBA_MEM;

#pragma pack()

#define SATA_SIG_ATA    0x00000101  // SATA drive
#define SATA_SIG_ATAPI  0xEB140101  // SATAPI drive
#define SATA_SIG_SEMB   0xC33C0101  // Enclosure management bridge
#define SATA_SIG_PM 0x96690101  // Port multiplier
#define AHCI_DEV_NULL 0
#define AHCI_DEV_SATA 1
#define AHCI_DEV_SATAPI 4
#define AHCI_DEV_SEMB 2
#define AHCI_DEV_PM 3
#define HBA_PORT_DET_PRESENT 3
#define HBA_PORT_IPM_ACTIVE 1

void initAHCI();

#define HBA_PORT_COUNT 32
extern HBA_MEM *pHbaMem;
extern uint32_t portSataDev;

#endif
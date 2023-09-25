#ifndef PCIE_H_HH
#define PCIE_H_HH
#include "stdint.h"
#pragma pack(1)
typedef struct PciDeviceConfigHead
{
    uint16_t VendorID;
    uint16_t deviceID;
    uint16_t Command;
    uint16_t Status;
    uint8_t  revisonID;
    uint8_t  ProgIF;
    uint8_t  Subclass;
    uint8_t  ClassCode;
    uint8_t  CacheLineSize;
    uint8_t  LatencyTimer;
    uint8_t  HeaderType;
    uint8_t  BIST;
}PciDeviceConfigHead;
typedef struct PciCapHead
{
    uint8_t capId;
    uint8_t nextCapOffset; 
}PciCapHead;
#pragma pack()

#define MAX_PCIE_CONFIG_PAGE_COUNT 256

typedef struct PcieConfigInfo
{
    uint8_t bus;
    uint8_t device;
    uint8_t function;
    uint8_t reserve;
    PciDeviceConfigHead *pConfigPage;
}PcieConfigInfo;

void checkPciDevice();
extern PcieConfigInfo  *pcieConfigInfos;
extern uint32 pcieConfigInfoCount;
#endif
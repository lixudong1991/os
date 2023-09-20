#include "pcie.h"
#include "printf.h"
#include "acpi.h"
#include "boot.h"
#include "memcachectl.h"
void checkFunction(uint8_t bus, uint8_t device, uint8_t function);

uint8_t getHeaderType(uint8_t bus, uint8_t device, uint8_t function)
{
    uint64_t baseAddr = (mcfgPciConfigSpace[0]->BaseAddr);
    uint64_t Bus = bus - mcfgPciConfigSpace[0]->StartPCIbus, Device = device, Function = function;
    PciDeviceConfigHead *phead = (PciDeviceConfigHead *)(baseAddr + (Bus << 20 | Device << 15 | Function << 12));
    return phead->HeaderType;
}
uint16_t getVendorID(uint8_t bus, uint8_t device, uint8_t function)
{
    uint64_t baseAddr = (mcfgPciConfigSpace[0]->BaseAddr);
    uint64_t Bus = bus - mcfgPciConfigSpace[0]->StartPCIbus, Device = device, Function = function;
    PciDeviceConfigHead *phead = (PciDeviceConfigHead *)(baseAddr + (Bus << 20 | Device << 15 | Function << 12));
    return phead->VendorID;
}
uint8_t getBaseClass(uint8_t bus, uint8_t device, uint8_t function)
{
    uint64_t baseAddr = (mcfgPciConfigSpace[0]->BaseAddr);
    uint64_t Bus = bus - mcfgPciConfigSpace[0]->StartPCIbus, Device = device, Function = function;
    PciDeviceConfigHead *phead = (PciDeviceConfigHead *)(baseAddr + (Bus << 20 | Device << 15 | Function << 12));
    return phead->ClassCode;
}
uint8_t getSubClass(uint8_t bus, uint8_t device, uint8_t function)
{
    uint64_t baseAddr = (mcfgPciConfigSpace[0]->BaseAddr);
    uint64_t Bus = bus - mcfgPciConfigSpace[0]->StartPCIbus, Device = device, Function = function;
    PciDeviceConfigHead *phead = (PciDeviceConfigHead *)(baseAddr + (Bus << 20 | Device << 15 | Function << 12));
    return phead->Subclass;
}
uint8_t getSecondaryBus(uint8_t bus, uint8_t device, uint8_t function)
{
    uint64_t baseAddr = (mcfgPciConfigSpace[0]->BaseAddr);
    uint64_t Bus = bus - mcfgPciConfigSpace[0]->StartPCIbus, Device = device, Function = function;
    PciDeviceConfigHead *phead = (PciDeviceConfigHead *)(baseAddr + (Bus << 20 | Device << 15 | Function << 12));
    return phead->Subclass;
}
PciDeviceConfigHead *getDeviceHead(uint8_t bus, uint8_t device, uint8_t function)
{
    uint64_t baseAddr = (mcfgPciConfigSpace[0]->BaseAddr);
    uint64_t Bus = bus - mcfgPciConfigSpace[0]->StartPCIbus, Device = device, Function = function;
    PciDeviceConfigHead *phead = (PciDeviceConfigHead *)(baseAddr + (Bus << 20 | Device << 15 | Function << 12));
    return phead;
}

void checkDevice(uint8_t bus, uint8_t device)
{
    uint8_t function = 0;
    uint16_t vendorID=0;
    uint16_t deviceID=0;
    uint8_t headerType;
    PciDeviceConfigHead *phead = getDeviceHead(bus, device, function);
    mem4k_map(((uint32_t)phead) & 0xfffff000, ((uint32_t)phead) & 0xfffff000, MEM_UC, PAGE_R);
    vendorID = phead->VendorID;
    if (vendorID == 0xFFFF)
       return; // Device doesn't exist
    deviceID = phead->deviceID;
    asm("cli");
    printf("addr:0x%x bus:%d device:%d vendorID: 0x%x  deviceID:0x%x\n", ((uint32_t)phead),bus, device, vendorID, deviceID);
    asm("sti");
    /*
    checkFunction(bus, device, function);
    headerType = getHeaderType(bus, device, function);
    if( (headerType & 0x80) != 0) {
        // It's a multi-function device, so check remaining functions
        for (function = 1; function < 8; function++) {
            if (getVendorID(bus, device, function) != 0xFFFF) {
                checkFunction(bus, device, function);
            }
        }
    }
    */
}
void checkBus(uint8_t bus)
{
    uint8_t device;

    for (device = 0; device < 32; device++)
    {
        checkDevice(bus, device);
    }
}
void checkFunction(uint8_t bus, uint8_t device, uint8_t function)
{
    //  uint8_t baseClass;
    //  uint8_t subClass;
    //  uint8_t secondaryBus;

    //  baseClass = getBaseClass(bus, device, function);
    //  subClass = getSubClass(bus, device, function);
    //  if ((baseClass == 0x6) && (subClass == 0x4)) {
    //      secondaryBus = getSecondaryBus(bus, device, function);
    //      checkBus(secondaryBus);
    //  }
}

void checkPciDevice()
{
    uint16_t bus;
    uint8_t device;
    
    for (bus = 0; bus < 250; bus++)
    {
        for (device = 0; device < 32; device++)
        {
            checkDevice(bus, device);
        }
    }
    // 如果mcfgPciConfigSpace[0]->BaseAddr=0xf0000000,则最后一个bus的最后一个device的最后一个function不能映射到0xfffff000，因为这个地址映射到了全局页目录
    if (mcfgPciConfigSpace[0]->BaseAddr == 0xf0000000)
    {
        // for (device = 0; device < 32; device++)
        // {
        //     checkDevice(255, device);
        // }
    }
    else //不等于则可以正常映射
    {
        // for (device = 0; device < 32; device++)
        // {
        //     checkDevice(255, device);
        // }
    }
}
#include "ioapic.h"
#include "acpi.h"
#include "boot.h"
#include "printf.h"
#include "memcachectl.h"
#define IOAPICID          0x00
#define IOAPICVER         0x01
#define IOAPICARB         0x02
#define IOAPICREDTBL(n)   (0x10 + 2 * n) // lower-32bits (add +1 for upper 32-bits)
 
#ifdef TRACE_IOAPIC
#	define TRACEIOAPIC(...) printf(__VA_ARGS__)
#else
#	define TRACEIOAPIC(...)
#endif

void write_ioapic_register(uint32_t  apic_base, const uint8_t offset, const uint32_t val) 
{
    /* tell IOREGSEL where we want to write to */
    *(volatile uint32_t*)(apic_base) = offset;
    /* write the value to IOWIN */
    *(volatile uint32_t*)(apic_base + 0x10) = val; 
}
 
uint32_t read_ioapic_register(uint32_t apic_base, const uint8_t offset)
{
    /* tell IOREGSEL where we want to read from */
    *(volatile uint32_t*)(apic_base) = offset;
    /* return the data from IOWIN */
    return *(volatile uint32_t*)(apic_base + 0x10);
}

void initIoApic()
{
    for(uint8_t index =0;index<Madt_IOAPIC_count;index++)
    {
        uint32_t ioapicaddr = Madt_IOAPIC[index]->IO_APIC_Address;
        mem4k_map(ioapicaddr&PAGE_ADDR_MASK,ioapicaddr&PAGE_ADDR_MASK,MEM_UC,PAGE_RW|PAGE_G);
        TRACEIOAPIC("ioapic id=0x%x IOAPICVER=0x%x IOAPICARB=0x%x \n",read_ioapic_register(ioapicaddr,IOAPICID),
        read_ioapic_register(ioapicaddr,IOAPICVER),read_ioapic_register(ioapicaddr,IOAPICARB));
    }
    
    write_ioapic_register(Madt_IOAPIC[0]->IO_APIC_Address,0x12,0x27);//IRQ1 0x27号中断
    write_ioapic_register(Madt_IOAPIC[0]->IO_APIC_Address,0x28,0x28);//IRQ12 0x28号中断
}
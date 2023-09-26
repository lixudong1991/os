#ifndef OSDATAPHY_ADDR_H
#define OSDATAPHY_ADDR_H

#define LOCK_START 0x1000
#define LOCK_SIZE 0x1000


#define KEY_BUFF_ADDR  0x6000
#define KEY_BUFF_SIZE  0xc00
#define AP_ARG_ADDR  0x6C00



#define AP_CODE_ADDR 0x60000

#define XAPIC_START_ADDR 0x80000

#define AHCI_PORT_USEMEM_START 0x4000 //4k对齐
#define AHCI_PORT_USEMEM_4K_COUNT 2
#define AHCI_PORT_CMD_TBL_START 0x3b000  //结束地址为:AHCI_PORT_CMD_TBL_START+SUPPORT_SATA_DEVICE_MAX_COUNT*(numCmdSlot+1)*CMD_TABLE_SIZE

#define IDA_ADDR 0x7000
#define ATOMIC_BUFF_ADDR 0x6E00  //不可缓存变量区域,配合spinLock使用
#define ATOMIC_BUFF_SIZE (IDA_ADDR-ATOMIC_BUFF_ADDR)  //不可缓存变量区域大小,配合spinLock使用

//0x7000idt
// 0x8000~0x9000 全局页目录
//0x9000~0xb000 初始页表
// 0xb000~0x1B000 gdt
// 0x1b000~0x3b000 pageStatus
// 0x3b000 内核加载起始地址
#endif
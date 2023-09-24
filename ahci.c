#include "ahci.h"
#include "stdint.h"
#include "pcie.h"
#include "boot.h"
#include "memcachectl.h"
#include "osdataPhyAddr.h"
#include "printf.h"
HBA_MEM *pHbaMem = NULL;
uint32_t portSataDev = 0;
uint32_t sataDevCount = 0;
Sata_Device *sataDev = NULL;
// Check device type
static int check_type(HBA_PORT *port)
{
    DWORD ssts = port->ssts;

    BYTE ipm = (ssts >> 8) & 0x0F;
    BYTE det = ssts & 0x0F;

    // print ("ipm %d det %d sig %d\n", ipm, det, port->sig);
    if (det != HBA_PORT_DET_PRESENT) // Check drive status
        return AHCI_DEV_NULL;
    if (ipm != HBA_PORT_IPM_ACTIVE)
        return AHCI_DEV_NULL;

    switch (port->sig)
    {
    case SATA_SIG_ATAPI:
        return AHCI_DEV_SATAPI;
    case SATA_SIG_SEMB:
        return AHCI_DEV_SEMB;
    case SATA_SIG_PM:
        return AHCI_DEV_PM;
    default:
        return AHCI_DEV_SATA;
    }

    return 0;
}

void probe_port(HBA_MEM *abar_temp)
{
    // Search disk in impelemented ports
    DWORD pi = abar_temp->pi;
    int i = 0;
    while (i < HBA_PORT_COUNT)
    {
        if (pi & 1)
        {
            int dt = check_type((HBA_PORT *)&abar_temp->ports[i]);
            if (dt == AHCI_DEV_SATA)
            {
                // printf("SATA drive found at port %d\n", i);
                portSataDev |= (((uint32_t)1) << i);
                //  port_rebase(abar_temp->ports, i);
                // read(&abar_temp->ports[0], 0, 0, 2, (uint64_t)pages_for_ahci_start + (20*4096)/8);
                // print("\nafter read %d",((HBA_PORT *)&abar_temp->ports[i])->ssts);
            }
            else if (dt == AHCI_DEV_SATAPI)
            {
                // print("\nSATAPI drive found at port %d\n", i);
            }
            else if (dt == AHCI_DEV_SEMB)
            {
                // print("\nSEMB drive found at port %d\n", i);
            }
            else if (dt == AHCI_DEV_PM)
            {
                // print("\nPM drive found at port %d\n", i);
            }
            else
            {
                // print("\nNo drive found at port %d\n", i);
            }
        }

        pi >>= 1;
        i++;
    }
}
static void port_cmd_fis_MemInit()
{
    uint32_t portUseStart = AHCI_PORT_USEMEM_START;
    for (int i = 0; i < AHCI_PORT_USEMEM_4K_COUNT; i++)
    {
        mem4k_map(portUseStart, portUseStart, MEM_UC, PAGE_RW);
        memset_s(portUseStart, 0, 0x1000);
        portUseStart += 0x1000;
    }
    mem_fix_type_set(portUseStart, AHCI_PORT_USEMEM_4K_COUNT * 0x1000, MEM_UC);
}
void hbaInit()
{
    port_cmd_fis_MemInit();
    sataDev = kernel_malloc(sizeof(Sata_Device) * SUPPORT_SATA_DEVICE_MAX_COUNT);
    uint32_t bitTest = 1;

    pHbaMem->ghc |= 1; // H:Init
    while (pHbaMem->ghc & 1)
        ;
    if ((pHbaMem->cap & (bitTest << 18)) == 0) // H:WaitForAhciEnable
        pHbaMem->ghc |= (bitTest << 31);
    // // H:Idle
    while (pHbaMem->ghc & (bitTest << 31) == 0)
        ;
    pHbaMem->ghc |= 2; // enable Interrupt

    uint32 cmdMemAddr = AHCI_PORT_USEMEM_START;
    uint32 fisMemAddr = AHCI_PORT_USEMEM_START + SUPPORT_SATA_DEVICE_MAX_COUNT * 1024;
    uint32 numCmdSlot = pHbaMem->cap & 0x1F00;
    numCmdSlot >>= 8;
    // 初始化sata设备连接的端口
    for (int i = 0; i < HBA_PORT_COUNT && sataDevCount < SUPPORT_SATA_DEVICE_MAX_COUNT; i++)
    {
        if (portSataDev & (((uint32_t)1) << i))
        {
            pHbaMem->ports[i].cmd &= (~(bitTest << 4)); // clear FRE
            pHbaMem->ports[i].cmd |= (bitTest << 3);    // set clo
            while ((pHbaMem->ports[i].cmd & (bitTest << 3)) || (pHbaMem->ports[i].cmd & (bitTest << 14)))
                ;
            pHbaMem->ports[i].clb = cmdMemAddr;
            pHbaMem->ports[i].clbu = 0;
            pHbaMem->ports[i].fb = fisMemAddr;
            pHbaMem->ports[i].fbu = 0;

            HBA_CMD_HEADER *cmdhead = cmdMemAddr;
            for (int cmdslotindex = 0; cmdslotindex < numCmdSlot; cmdslotindex++)
            {
                cmdhead[cmdslotindex].ctba = kernel_malloc_align(CMD_TABLE_SIZE, 128);
                cmdhead[cmdslotindex].ctbau = 0;
            }
            cmdMemAddr += 1024;
            fisMemAddr += 256;
            pHbaMem->ports[i].cmd |= (bitTest << 4); // set FRE
            pHbaMem->ports[i].ie |= 0x7D40007F;      // 设置port启用错误中断生成

            while ((pHbaMem->ports[i].cmd & (bitTest << 15)) &&
                   ((pHbaMem->ports[i].tfd & (bitTest << 7)) || (pHbaMem->ports[i].tfd & (bitTest << 3))))
                ;
            while (1)
            {
                if (pHbaMem->ports[i].ssts & 3)
                    break;
                uint32 testssts = pHbaMem->ports[i].ssts >> 8;
                if (testssts == 2 || testssts == 6 || testssts == 8)
                    break;
            }
            pHbaMem->ports[i].cmd |= 1; // set cmd.st
            printf("sata dev: port %d fb 0x%x is:0x%x ie:0x%x cmd:0x%x sig:0x%x ssts:0x%x sctl:0x%x serr:0x%x sact:0x%x tfd:0x%x\n", i, pHbaMem->ports[i].fb,
                   pHbaMem->ports[i].is, pHbaMem->ports[i].ie, pHbaMem->ports[i].cmd, pHbaMem->ports[i].sig, pHbaMem->ports[i].ssts, pHbaMem->ports[i].sctl, pHbaMem->ports[i].serr, pHbaMem->ports[i].sact, pHbaMem->ports[i].tfd);
            sataDev[sataDevCount].port = i;
            sataDev[sataDevCount].pPortMem = &(pHbaMem->ports[i]);
            sataDevCount++;
        }
    }
}
void initAHCI()
{
    asm("cli");
    printf("pcie config space page count=%d\n", pcieConfigInfoCount);
    asm("sti");
    PcieConfigInfo *ahciconfig = NULL;
    for (int i = 0; i < pcieConfigInfoCount; i++)
    {
        asm("cli");
        printf("pcie config addr:0x%x bus:%d device:%d vendorID: 0x%x  deviceID:0x%x\n", pcieConfigInfos[i].pConfigPage, pcieConfigInfos[i].bus,
               pcieConfigInfos[i].device, pcieConfigInfos[i].pConfigPage->VendorID, pcieConfigInfos[i].pConfigPage->deviceID);
        asm("sti");
#if 0
		if(pcieConfigInfos[i].pConfigPage->VendorID==0x8086&&pcieConfigInfos[i].pConfigPage->deviceID==0xa282)
		{
			ahciconfig =&(pcieConfigInfos[i]);
		}
#else
        if (pcieConfigInfos[i].pConfigPage->ClassCode == 0x1 && pcieConfigInfos[i].pConfigPage->Subclass == 0x6)
        {
            ahciconfig = &(pcieConfigInfos[i]);
        }
#endif
    }
    if (ahciconfig)
    {
        uint32 baseAddr = ahciconfig->pConfigPage;
        uint8_t *pcieAhciIntr = baseAddr + 0x3C;
        *pcieAhciIntr = 0x78;                           // ahci 中断为0x78号中断
        ahciconfig->pConfigPage->Command &= 0xFFFFFBFF; // 启用pci中断
        ahciconfig->pConfigPage->Command |= 2;          // 启用pci内存空间访问

        printf("ahci pciaddr=0x%x ", baseAddr);
        printf("Command =0x%x  Status =0x%x IPIN=%d\n", ahciconfig->pConfigPage->Command, ahciconfig->pConfigPage->Status,*(WORD*)(baseAddr + 0x3C));
        printf("Revision ID =0x%x  Class Code =0x%x SubClass=0x%x ProgIF=0x%x\n", ahciconfig->pConfigPage->revisonID, ahciconfig->pConfigPage->ClassCode,
               ahciconfig->pConfigPage->Subclass, ahciconfig->pConfigPage->ProgIF);
        printf("CacheLineSize:0x%x LatencyTimer:0x%x HeaderType=0x%x BIST=0x%x\n", ahciconfig->pConfigPage->CacheLineSize, ahciconfig->pConfigPage->LatencyTimer,
               ahciconfig->pConfigPage->HeaderType, ahciconfig->pConfigPage->BIST);
        uint32 bar5value = (*(uint32_t *)(baseAddr + 0x24));
        printf("bar5value=0x%x Prefetchable=%d Type=0x%x Memory Space Indicator:%d ", bar5value, bar5value & 0x8, bar5value & 0x6, bar5value & 0x1);
        // 确定pci设备空间大小
        (*(uint32_t *)(baseAddr + 0x24)) = 0xFFFFFFFF;
        uint32 bar5size = (*(uint32_t *)(baseAddr + 0x24));
        bar5size = ~bar5size;
        bar5size |= 0xf;
        bar5size += 1;
        printf("bar5size =0x%x\n", bar5size);

        (*(uint32_t *)(baseAddr + 0x24)) = bar5value;
        HBA_MEM *abar_temp = (HBA_MEM *)(bar5value & 0xFFFFFFF0);
        mem4k_map(((uint32_t)abar_temp) & 0xfffff000, ((uint32_t)abar_temp) & 0xfffff000, MEM_UC, PAGE_G | PAGE_RW);
        mem4k_map(((uint32_t)abar_temp) & 0xfffff000 + 0x1000, ((uint32_t)abar_temp) & 0xfffff000 + 0x1000, MEM_UC, PAGE_G | PAGE_RW);
        printf("baseAddrReg=0x%x\n", abar_temp); // 位13-31代表bar地址
        printf("Capabilities=0x%x interrupt pin=0x%x line=0x%x\n", *(uint8_t *)(baseAddr + 0x34), *(uint8_t *)(baseAddr + 0x3c), *(uint8_t *)(baseAddr + 0x3d));

        pHbaMem = abar_temp;

        probe_port(abar_temp);
        hbaInit();
        printf("hba cap:0x%x ghc:0x%x IS:0x%x PI:0x%x VS:0x%x CCC_CTL:0x%X CCC_PORTS:0x%x EM_LOC:0x%x EM_CTL:0x%x CAP2:0x%x BOHC:0x%x\n",
               pHbaMem->cap, pHbaMem->ghc, pHbaMem->is, pHbaMem->pi, pHbaMem->vs, pHbaMem->ccc_ctl, pHbaMem->ccc_pts, pHbaMem->em_loc, pHbaMem->em_ctl, pHbaMem->cap2, pHbaMem->bohc);
    }
}

// To setup command fing a free command list slot
uint32 find_cmdslot(HBA_PORT *port)
{
    // An empty command slot has its respective bit cleared to �0� in both the PxCI and PxSACT registers.
    // If not set in SACT and CI, the slot is free // Checked
    DWORD slots = (port->sact | port->ci);
    uint32 numCmdSlot = pHbaMem->cap & 0x1F00;
    numCmdSlot >>= 8;
    uint32 i;
    for (i = 0; i < numCmdSlot; i++)
    {

        if ((slots & 1) == 0)
        {
            // print("\n[command slot is : %d]", i);
            return i;
        }
        slots >>= 1;
    }
    printf("Cannot find free command list entry\n");
    return -1;
}

int ahci_read(HBA_PORT *port, DWORD startl, DWORD starth, DWORD sectorcount, QWORD bufaddr)
{
}
int ahci_write(HBA_PORT *port, DWORD startl, DWORD starth, DWORD sectorcount, QWORD bufaddr)
{
    if (sectorcount > CMD_RW_MAX_SECTORS_COUNT)
        return 0;

    DWORD sentByteCount = sectorcount * 512;
    DWORD prdtl = sentByteCount / 0x400000; // 每个prd entry最多传输4mb;
    if (sentByteCount % 0x400000 != 0)
        prdtl++;
    if (prdtl > (CMD_TABLE_SIZE - 128) / 16)
        return 0;

    int slot = find_cmdslot(port);
    if (slot == -1)
        return 0;

    HBA_CMD_HEADER *cmdheadArr = port->clb;
    memset_s(cmdheadArr, 0, sizeof(HBA_CMD_HEADER));
    cmdheadArr[slot].prdtl = prdtl;
    cmdheadArr[slot].w = 1;
    cmdheadArr[slot].p = 1;
    cmdheadArr[slot].c = 1;
    cmdheadArr[slot].cfl = sizeof(FIS_REG_H2D) / sizeof(DWORD);
    cmdheadArr[slot].prdbc = 0;

    HBA_CMD_TBL *pcmdtb = cmdheadArr[slot].ctba;
    DWORD prdtentryByteSize = 0;
    for (int i = 0; i < prdtl; i++)
    {
        pcmdtb->prdt_entry[i].dba = (DWORD)(bufaddr & 0xFFFFFFFF);
        pcmdtb->prdt_entry[i].dbau = (DWORD)((bufaddr >> 32) & 0xFFFFFFFF);
        prdtentryByteSize = sentByteCount > 0x400000 ? 0x400000 : sentByteCount;
        sentByteCount -= prdtentryByteSize;
        pcmdtb->prdt_entry[i].DesInfo = prdtentryByteSize - 1;
        bufaddr += prdtentryByteSize;
        //    if (i == prdtl - 1)
        //       pcmdtb->prdt_entry[i].DesInfo |= 0x80000000; // 最后一个prd条目生成中断
    }
    // Setup command
    FIS_REG_H2D *cmdfis = (FIS_REG_H2D *)(&(pcmdtb->cfis));

    cmdfis->fis_type = FIS_TYPE_REG_H2D;
    cmdfis->c = 1; // Command
    cmdfis->command = ATA_CMD_WRITE_DMA_EX;

    cmdfis->lba0 = (BYTE)startl;
    cmdfis->lba1 = (BYTE)(startl >> 8);
    cmdfis->lba2 = (BYTE)(startl >> 16);
    cmdfis->device = 1 << 6; // LBA mode

    cmdfis->lba3 = (BYTE)(startl >> 24);
    cmdfis->lba4 = (BYTE)starth;
    cmdfis->lba5 = (BYTE)(starth >> 8);

    cmdfis->countl = sectorcount & 0xff;
    cmdfis->counth = sectorcount >> 8;

    port->ci |= (((uint32_t)1) << slot);

    while (1)
    {
        // In some longer duration reads, it may be helpful to spin on the DPS bit
        // in the PxIS port field as well (1 << 5)
        if ((port->ci & (1 << slot)) == 0)
            break;
    }
    return TRUE;
}

void interruptHandle_AHCI()
{
    printf("hba Interrput: GHC.IS=0x%x\n", pHbaMem->is);
    DWORD pi = pHbaMem->pi;
    DWORD is = pHbaMem->is;
    DWORD bitTest = 1;
    DWORD pie = 0;
    for (DWORD i = 0; i < HBA_PORT_COUNT; i++, bitTest <<= 1)
    {
        if (pi & bitTest)
        {
            if (is & bitTest)
            {
                printf("hba port Interrput:port =%d port.is=0x%x serr:0x%x sact:0x%x tfd:0x%x", i, pHbaMem->ports[i].is,
                       pHbaMem->ports[i].serr, pHbaMem->ports[i].sact, pHbaMem->ports[i].tfd);
                pie = pHbaMem->ports[i].ie;
                pHbaMem->ports[i].is &= (~pie);
                pHbaMem->is &= (~bitTest);
            }
        }
    }
}
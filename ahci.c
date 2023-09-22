#include "ahci.h"
#include "stdint.h"
#include "pcie.h"
#include "boot.h"
#include "memcachectl.h"

HBA_MEM *pHbaMem = NULL;
uint32_t portSataDev = 0;

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
    uint32_t supportPortCount = pHbaMem->cap & 0x1f + 1;
    int i = 0;
    while (i < supportPortCount)
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

void hbaInit()
{
    // uint32_t bitTest = 1;
    // pHbaMem->ghc |= 1; // H:Init
    // while (pHbaMem->ghc & 1);
    // if ((pHbaMem->cap & (bitTest << 18)) == 0) // H:WaitForAhciEnable
    //     pHbaMem->ghc |= (bitTest << 31);
    // // H:Idle

    // // 初始化sata设备连接的端口
    // uint32_t supportPortCount = pHbaMem->cap & 0x1f + 1;
    // for (int i = 0; i < supportPortCount; i++)
    // {
    //     if (portSataDev & (((uint32_t)1) << i))
    //     {
    //         printf("sata dev: port %d fb 0x%x is:0x%x ie:0x%x cmd:0x%x sig:0x%x ssts:0x%x sctl:0x%x serr:0x%x sact:0x%x\n", i,pHbaMem->ports[i].fb,
    //                pHbaMem->ports[i].is, pHbaMem->ports[i].ie, pHbaMem->ports[i].cmd, pHbaMem->ports[i].sig, pHbaMem->ports[i].ssts, pHbaMem->ports[i].sctl, pHbaMem->ports[i].serr, pHbaMem->ports[i].sact);

    //          pHbaMem->ports[i].cmd &=0xFFFFFFEF;
    //     }          
    // }
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
        printf("ahci pciaddr=0x%x ", baseAddr);
        printf("Command =0x%x  Status =0x%x\n", ahciconfig->pConfigPage->Command, ahciconfig->pConfigPage->Status);
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
        printf("hba cap:0x%x ghc:0x%x IS:0x%x PI:0x%x VS:0x%x CCC_CTL:0x%X CCC_PORTS:0x%x EM_LOC:0x%x EM_CTL:0x%x CAP2:0x%x BOHC:0x%x\n",
               pHbaMem->cap, pHbaMem->ghc, pHbaMem->is, pHbaMem->pi, pHbaMem->vs, pHbaMem->ccc_ctl, pHbaMem->ccc_pts, pHbaMem->em_loc, pHbaMem->em_ctl, pHbaMem->cap2, pHbaMem->bohc);
        probe_port(abar_temp);

        hbaInit();
    }
}

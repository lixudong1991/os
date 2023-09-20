#include "ahci.h"
#include "stdint.h"
#include "pcie.h"
#include "boot.h"
#include "memcachectl.h"

HBA_MEM *pHbaMem =NULL;


// Check device type
static int check_type(HBA_PORT *port)
{
    DWORD ssts = port->ssts;
 
    BYTE ipm = (ssts >> 8) & 0x0F;
    BYTE det = ssts & 0x0F;
    

   //print ("ipm %d det %d sig %d\n", ipm, det, port->sig); 
   if (det != HBA_PORT_DET_PRESENT)    // Check drive status
        return AHCI_DEV_NULL;
    if (ipm != HBA_PORT_IPM_ACTIVE)
        return AHCI_DEV_NULL;
 
    switch (port->sig)
    {
    case SATA_SIG_ATAPI:
        return AHCI_DEV_SATAPI ;
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
    while (i<32)
    {
        if (pi & 1)
        {
            int dt = check_type((HBA_PORT *)&abar_temp->ports[i]);
            if (dt == AHCI_DEV_SATA)
            {
                printf("SATA drive found at port %d\n", i);
                pHbaMem = abar_temp;
              //  port_rebase(abar_temp->ports, i);
                //read(&abar_temp->ports[0], 0, 0, 2, (uint64_t)pages_for_ahci_start + (20*4096)/8);
                //print("\nafter read %d",((HBA_PORT *)&abar_temp->ports[i])->ssts);
                return;
            }
            else if (dt == AHCI_DEV_SATAPI)
            {
                //print("\nSATAPI drive found at port %d\n", i);
            }
            else if (dt == AHCI_DEV_SEMB)
            {
                //print("\nSEMB drive found at port %d\n", i);
            }
            else if (dt == AHCI_DEV_PM)
            {
                //print("\nPM drive found at port %d\n", i);
            }
            else
            {
                //print("\nNo drive found at port %d\n", i);
            }
        }
 
        pi >>= 1;
        i ++;
    }
}
void initAHCI()
{
	asm("cli");
	printf("pcie config space page count=%d\n",pcieConfigInfoCount);
	asm("sti");
	PcieConfigInfo *ahciconfig=NULL;
	for(int i=0;i<pcieConfigInfoCount;i++)
	{
		asm("cli");
    	printf("pcie config addr:0x%x bus:%d device:%d vendorID: 0x%x  deviceID:0x%x\n",pcieConfigInfos[i].pConfigPage,pcieConfigInfos[i].bus, 
		pcieConfigInfos[i].device, pcieConfigInfos[i].pConfigPage->VendorID,pcieConfigInfos[i].pConfigPage->deviceID);
    	asm("sti");
		if(pcieConfigInfos[i].pConfigPage->VendorID==0x8086&&pcieConfigInfos[i].pConfigPage->deviceID==0xa282)
		{
			ahciconfig =&(pcieConfigInfos[i]);
		}
	}
	if(ahciconfig)
	{
        uint32 baseAddr = ahciconfig->pConfigPage;
        printf("ahci pciaddr=0x%x",baseAddr);
        printf("Command =0x%x  Status =0x%x\n",ahciconfig->pConfigPage->Command,ahciconfig->pConfigPage->Status);
        printf("Revision ID =0x%x  Class Code =0x%x SubClass=0x%x ProgIF=0x%x\n",ahciconfig->pConfigPage->revisonID,ahciconfig->pConfigPage->ClassCode,
                ahciconfig->pConfigPage->Subclass,ahciconfig->pConfigPage->ProgIF);
        printf("CacheLineSize:0x%x LatencyTimer:0x%x HeaderType=0x%x BIST=0x%x\n",ahciconfig->pConfigPage->CacheLineSize,ahciconfig->pConfigPage->LatencyTimer,
        ahciconfig->pConfigPage->HeaderType,ahciconfig->pConfigPage->BIST);
        HBA_MEM *abar_temp = (HBA_MEM *)((*(uint32_t*)(baseAddr+0x24))&0xFFFFE000);   
        mem4k_map((uint32_t)abar_temp,(uint32_t)abar_temp,MEM_UC,PAGE_G|PAGE_RW);  
        mem4k_map(((uint32_t)abar_temp)+0x1000,((uint32_t)abar_temp)+0x1000,MEM_UC,PAGE_G|PAGE_RW); 
		printf("baseAddrReg=0x%x\n",abar_temp);//位13-31代表bar地址
		printf("Capabilities=0x%x interrupt pin=0x%x line=0x%x\n",*(uint8_t*)(baseAddr+0x34),*(uint8_t*)(baseAddr+0x3c),*(uint8_t*)(baseAddr+0x3d));
        probe_port(abar_temp);
    }
}
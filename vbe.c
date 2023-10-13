#include "vbe.h"
#include "stdint.h"
#include "boot.h"
#include "stdio.h"
#include "memcachectl.h"
#include "string.h"
char g_vbebuff[VBE_BUFF_SIZE];

PMInfoBlock * findPMInfoBlock()
{
    char *ret = NULL;
    //根据vbe规范，PMInfoBlock位于BIOS image前32kb内存内
    uint32_t startaddr=0xc0000, endaddr=0xc8000;
    for(int index =startaddr;index<endaddr;index+=0x1000)
        mem4k_map(index,index,MEM_UC,PAGE_G | PAGE_RW);
    char rsdpsig[] = "PMID";
    int nextbuff[4] = {0};
    int rsdtindex=0;
    while(startaddr<endaddr)
    {
        rsdtindex= IndexStr_KMP(startaddr, endaddr-startaddr, rsdpsig, nextbuff, 4);
        if (rsdtindex != -1)
        {
            char *temp = (char *)(startaddr + rsdtindex);
            char checksum = 0;
            for(int i=0;i<sizeof(PMInfoBlock);i++)
            {   
                checksum+=temp[i];
            }
            if(checksum == 0)
            {
                ret = temp;
                break;
            }   
        }
        startaddr+=(rsdtindex+4);         
    }
    return (PMInfoBlock*)ret;
}

void dumpVbeInfo()
{
    PMInfoBlock *ppminfo = findPMInfoBlock();
    if(ppminfo)
    {
        char pmsigstr[8];
        memcpy_s(pmsigstr,ppminfo->Signature,4);
        pmsigstr[4] =0;
        printf("PMInfoBlock sig=%s\n",pmsigstr);
    } 
	asm("cli");
	char temp = g_vbebuff[4];
	g_vbebuff[4] = 0; 
	printf("VBE  signature=%s\n",g_vbebuff);
	g_vbebuff[4] = temp; 
	vbe_info_structure *pvbeinfo = g_vbebuff;
	printf("VBE  version=0x%x\n",pvbeinfo->version);
	printf("VBE  oem=0x%x\n",pvbeinfo->oem);
	printf("VBE  capabilities=0x%x\n",pvbeinfo->capabilities);
	uint32_t vbeAddr = (((pvbeinfo->video_modes>>16)&0xffff)<<4)+(pvbeinfo->video_modes&0xffff);
	mem4k_map(vbeAddr&PAGE_ADDR_MASK,vbeAddr&PAGE_ADDR_MASK,MEM_UC,PAGE_G | PAGE_RW);
	printf("VBE  video_modesAddr=0x%x modes:",vbeAddr);
	uint16_t *pmodelist = vbeAddr;
	while(*pmodelist != 0xffff)
	{
		printf("0x%x ",*pmodelist);
		pmodelist++;
	}
	printf("\nVBE  video_memory=0x%x\n",pvbeinfo->video_memory);
	printf("VBE  software_rev=0x%x\n",pvbeinfo->software_rev);
	vbeAddr= (((pvbeinfo->vendor>>16)&0xffff)<<4)+(pvbeinfo->vendor&0xffff);
	printf("VBE  vendor=0x%x\n",vbeAddr);
	vbeAddr= (((pvbeinfo->product_name>>16)&0xffff)<<4)+(pvbeinfo->product_name&0xffff);
	printf("VBE  product_name=0x%x\n",vbeAddr);
	vbeAddr= (((pvbeinfo->product_rev>>16)&0xffff)<<4)+(pvbeinfo->product_rev&0xffff);
	printf("VBE  product_rev=0x%x\n",vbeAddr);
	asm("sti");


}
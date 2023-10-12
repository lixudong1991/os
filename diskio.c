/*-----------------------------------------------------------------------*/
/* Low level disk I/O module SKELETON for FatFs     (C)ChaN, 2019        */
/*-----------------------------------------------------------------------*/
/* If a working storage control module is available, it should be        */
/* attached to the FatFs via a glue function rather than modifying it.   */
/* This is an example of glue functions to attach various exsisting      */
/* storage control modules to the FatFs module with a defined API.       */
/*-----------------------------------------------------------------------*/

#include "ff.h"		/* Obtains integer types */
#include "diskio.h" /* Declarations of disk functions */
#include "ahci.h"
#include "boot.h"
/* Definitions of physical drive number for each drive */
#define DEV_RAM 0 /* Example: Map Ramdisk to physical drive 0 */
#define DEV_MMC 1 /* Example: Map MMC/SD card to physical drive 1 */
#define DEV_USB 2 /* Example: Map USB MSD to physical drive 2 */

/*-----------------------------------------------------------------------*/
/* Get Drive Status                                                      */
/*-----------------------------------------------------------------------*/

DSTATUS disk_status(
	BYTE pdrv /* Physical drive nmuber to identify the drive */
)
{
	// DSTATUS stat;
	// int result;

	// switch (pdrv)
	// {
	// case DEV_RAM:
	// 	result = RAM_disk_status();

	// 	// translate the reslut code here

	// 	return stat;

	// case DEV_MMC:
	// 	result = MMC_disk_status();

	// 	// translate the reslut code here

	// 	return stat;

	// case DEV_USB:
	// 	result = USB_disk_status();

	// 	// translate the reslut code here

	// 	return stat;
	// }
	// return STA_NOINIT;
	return 0;
}

/*-----------------------------------------------------------------------*/
/* Inidialize a Drive                                                    */
/*-----------------------------------------------------------------------*/

DSTATUS disk_initialize(
	BYTE pdrv /* Physical drive nmuber to identify the drive */
)
{
	// DSTATUS stat;
	// int result;

	// switch (pdrv)
	// {
	// case DEV_RAM:
	// 	result = RAM_disk_initialize();

	// 	// translate the reslut code here

	// 	return stat;

	// case DEV_MMC:
	// 	result = MMC_disk_initialize();

	// 	// translate the reslut code here

	// 	return stat;

	// case DEV_USB:
	// 	result = USB_disk_initialize();

	// 	// translate the reslut code here

	// 	return stat;
	// }
	// return STA_NOINIT;
	return 0;
}

/*-----------------------------------------------------------------------*/
/* Read Sector(s)                                                        */
/*-----------------------------------------------------------------------*/

DRESULT disk_read(
	BYTE pdrv,	  /* Physical drive nmuber to identify the drive */
	BYTE *buff,	  /* Data buffer to store read data */
	LBA_t sector, /* Start sector in LBA */
	UINT count	  /* Number of sectors to read */
)
{
	DRESULT res;
	if (pdrv >= sataDevCount)
		return RES_PARERR;
	if(ahci_read(pdrv,(DWORD)(sector&0xffffffff),(DWORD)((sector>>32)&0xffffffff),count,buff) == count)
		return RES_OK;
	return RES_ERROR;
}

/*-----------------------------------------------------------------------*/
/* Write Sector(s)                                                       */
/*-----------------------------------------------------------------------*/

#if FF_FS_READONLY == 0

DRESULT disk_write(
	BYTE pdrv,		  /* Physical drive nmuber to identify the drive */
	const BYTE *buff, /* Data to be written */
	LBA_t sector,	  /* Start sector in LBA */
	UINT count		  /* Number of sectors to write */
)
{
	DRESULT res;
	if (pdrv >= sataDevCount)
		return RES_PARERR;
	if(ahci_write(pdrv,(DWORD)(sector&0xffffffff),(DWORD)((sector>>32)&0xffffffff),count,buff) == count)
		return RES_OK;
	return RES_ERROR;
}

#endif

/*-----------------------------------------------------------------------*/
/* Miscellaneous Functions                                               */
/*-----------------------------------------------------------------------*/

DRESULT disk_ioctl(
	BYTE pdrv, /* Physical drive nmuber (0..) */
	BYTE ctrl, /* Control code */
	void *buff /* Buffer to send/receive control data */
)
{
	DRESULT res = RES_PARERR;
	int result;
	if (pdrv >= sataDevCount)
		return res;

	switch (ctrl)
	{
	case CTRL_SYNC: /* Nothing to do */
		res = RES_OK;
		break;

	case GET_SECTOR_COUNT: /* Get number of sectors on the drive */
	{
		if (sataDev[pdrv].pIdentifyBuff)
		{
			char *temp = sataDev[pdrv].pIdentifyBuff;
			*(LBA_t *)buff = *(uint32_t*)(temp+60*2);
			res = RES_OK;
		}
		break;
	}
	case GET_SECTOR_SIZE: /* Get size of sector for generic read/write */
	{
		////获取1个物理扇区是多少个逻辑扇区
		// if (sataDev[pdrv].pIdentifyBuff)
		// {	
		// 	char *temp = sataDev[pdrv].pIdentifyBuff;
		// 	uint16_t phylogSectorSize = *(uint16_t*)(temp+106*2);

		// 		// Word 106: Physical sector size / logical sector size 测试ata identify命令返回的值得第106个字的内容是否有效
		// 	if(	(phylogSectorSize & (uint16_t)0x4000)&&
		// 		((phylogSectorSize & (uint16_t)0x8000) == 0))
		// 	{
		// 		//1个logical sec大小512字节，这个字得0:3位指示每个phy sec是多少个log sec,例如:0:3位是数字3，则每个phy sec是2^3个log sec，也就是512*2^3 = 512*8=4096;

		// 		uint16_t bitv = 0xf&phylogSectorSize;
		// 		uint16_t count =1;
		// 		while(bitv)
		// 		{
		// 			count*=2;
		// 			bitv--;
		// 		}
		// 		*(WORD *)buff = 512*count;
		// 	}
		// }else{
		// 	*(WORD *)buff = 512;	
		// }
		*(WORD *)buff = 512;	
		res = RES_OK;
		break;
	}
	
	case GET_BLOCK_SIZE: /* Get internal block size in unit of sector */
		*(DWORD *)buff = 512;
		res = RES_OK;
		break;
	}

	return res;
}

DWORD get_fattime (void)
{
	SYSTEMTIME tm;

	/* Get local time */
	getCmosDateTime(&tm);

	/* Pack date and time into a DWORD variable */
	return   (tm.wYear - 1980) << 25 | tm.wMonth << 21 | tm.wDay << 16 | tm.wHour << 11 | tm.wMinute << 5 | tm.wSecond >> 1;
}

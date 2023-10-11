#include "fat32.h"
#include "boot.h"
#include "ahci.h"
#include "printf.h"
#include "string.h"
#include "stdlib.h"
#define FS_START_LBA 0x800
#define FS_TOTSEC_COUNT 0x4000000 - FS_START_LBA
static FAT32_BPB_Struct *pBpbinfo = NULL;
static VolumeInfo volumeInfo;
static MbrPartition partioninfo;
static MbrPartition partioninfo;
static uint32_t g_CurrentDirFirstCluster;

#define MAX_DIR_LONGNAME_SIZE 256
#define TRACE_FS
#ifdef TRACE_FS
#define TRACEFS(...) printf(__VA_ARGS__)
#else
#define TRACEFS(...)
#endif
void formatFat32()
{
    char bpbbuff[512];
    memset_s(bpbbuff, 0, 512);
    bpbbuff[510] = 0x55;
    bpbbuff[511] = 0xAA;
    FAT32_BPB_Struct *bpbstruct = bpbbuff;
    memcpy_s(bpbstruct->BS_OEMName, "MSWIN4.1", 8);
    bpbstruct->BPB_BytsPerSec = 512;
    bpbstruct->BPB_SecPerClus = 8;
    bpbstruct->BPB_RsvdSecCnt = 32;
    bpbstruct->BPB_NumFATs = 2;
    bpbstruct->BPB_RootEntCnt = 0;
    bpbstruct->BPB_TotSec16 = 0;
    bpbstruct->BPB_Media = 0xf8;
    bpbstruct->BPB_FATSz16 = 0;
    bpbstruct->BPB_SecPerClus = 0;
    bpbstruct->BPB_NumHeads = 0;
    bpbstruct->BPB_HiddSec = 0;
    bpbstruct->BPB_TotSec32 = FS_TOTSEC_COUNT;

    bpbstruct->BPB_FATSz32 = 65471;
    bpbstruct->BPB_ExtFlags = 0;
    bpbstruct->BPB_FSVer = 0;
    bpbstruct->BPB_RootClus = 2;
    bpbstruct->BPB_FSInfo = 1;
    bpbstruct->BPB_BkBootSec = 6;
    bpbstruct->BS_DrvNum = 0x80;
    bpbstruct->BS_BootSig = 0x29;
    memcpy_s(bpbstruct->BS_VolLab, "NO NAME    ", 11);
    memcpy_s(bpbstruct->BS_FilSysType, "FAT32   ", 8);

    uint32 swcount = ahci_write(0, FS_START_LBA, 0, 1, bpbbuff);
    char fatbuff[512];
    memset_s(bpbbuff, 0, 512);
    uint32_t fat1startlba = FS_START_LBA + bpbstruct->BPB_RsvdSecCnt, fat2startlba = FS_START_LBA + bpbstruct->BPB_RsvdSecCnt + bpbstruct->BPB_FATSz32;
    for (int i = 0; i < bpbstruct->BPB_FATSz32; i++)
    {
        ahci_write(0, fat1startlba++, 0, 1, fatbuff);
        ahci_write(0, fat2startlba++, 0, 1, fatbuff);
    }
    *(uint32_t *)fatbuff = 0x0FFFFFF8;
    *(uint32_t *)(fatbuff + 4) = 0xFFFFFFFF;
    *(uint32_t *)(fatbuff + 8) = 0x0FFFFFFF;
    fat1startlba = FS_START_LBA + bpbstruct->BPB_RsvdSecCnt, fat2startlba = FS_START_LBA + bpbstruct->BPB_RsvdSecCnt + bpbstruct->BPB_FATSz32;
    ahci_write(0, fat1startlba, 0, 1, fatbuff);
    ahci_write(0, fat2startlba, 0, 1, fatbuff);
}
void initFS()
{
    pBpbinfo = kernel_malloc(sizeof(FAT32_BPB_Struct));
    char bpbbuff[512];
    memset_s(bpbbuff, 0, 512);
    uint32 swcount = ahci_read(0, 0, 0, 1, (uint32_t)bpbbuff);
    memcpy_s(&partioninfo, bpbbuff + 446, sizeof(MbrPartition));
    // TRACEFS("ahci_read fsStartLBA:0x%x\n",partioninfo.fsStartLBA);
    swcount = ahci_read(0, partioninfo.fsStartLBA, 0, 1, (uint32_t)bpbbuff);
    memcpy_s(pBpbinfo, bpbbuff, sizeof(FAT32_BPB_Struct));
    // asm("cli");
    // TRACEFS("ahci_read fs:%d\n",swcount);
    // TRACEFS("ahci_read BPB_RsvdSecCnt:0x%x\n",pBpbinfo->BPB_RsvdSecCnt);
    // TRACEFS("ahci_read BPB_FATSz32:0x%x\n",pBpbinfo->BPB_FATSz32);
    // TRACEFS("ahci_read BPB_TotSec32:0x%x\n",pBpbinfo->BPB_TotSec32);
    // asm("sti");
    volumeInfo.RootDirSectors = 0;
    volumeInfo.FirstDataSector = partioninfo.fsStartLBA + pBpbinfo->BPB_RsvdSecCnt + pBpbinfo->BPB_NumFATs * pBpbinfo->BPB_FATSz32;
    volumeInfo.DataSec = pBpbinfo->BPB_TotSec32 - (pBpbinfo->BPB_RsvdSecCnt + pBpbinfo->BPB_NumFATs * pBpbinfo->BPB_FATSz32);
    volumeInfo.CountofClusters = volumeInfo.DataSec / pBpbinfo->BPB_SecPerClus;
    asm("cli");
    TRACEFS("FS FirstDataSector:0x%x DataSec:0x%x CountofClusters:0x%x\n", volumeInfo.FirstDataSector, volumeInfo.DataSec, volumeInfo.CountofClusters);
    asm("sti");
    g_CurrentDirFirstCluster = pBpbinfo->BPB_RootClus;
}
int _get_dir_item_descdata_fromname(uint32_t firstclusternum, const char *itemname, char *descbuff, uint32_t descbufflen)
{
    uint32_t dirCluster = firstclusternum;
    uint32_t dirClusterSecIndex = (dirCluster - 2) * pBpbinfo->BPB_SecPerClus + volumeInfo.FirstDataSector;
    uint32_t dirClusterSecEnd = dirClusterSecIndex + pBpbinfo->BPB_SecPerClus;

    uint32_t thisFATSecNum = 0;
    uint32_t thisFATEntOffset = 0;
    char fatsectordata[512] = {0};

    
    int descBuffIndex = 0;
    char sectordata[512] = {0};
    int descBuffSize = 0;

    int wdescIndex = 0;
    char wdescbuffname[MAX_DIR_LONGNAME_SIZE*sizeof(wchar_t)];
    memset_s(wdescbuffname, 0, sizeof(wchar_t) * MAX_DIR_LONGNAME_SIZE);

    // wcstombs(descbuff, wdesbuffname, descbufflen);
    while (1)
    {
        if (dirClusterSecIndex >= dirClusterSecEnd)
        {
            uint32_t temp = partioninfo.fsStartLBA + pBpbinfo->BPB_RsvdSecCnt + (dirCluster * 4) / pBpbinfo->BPB_BytsPerSec;
            // TRACEFS("fs read thisFATSecNum:%d temp:%d\n",thisFATSecNum,temp);
            if (thisFATSecNum != temp)
            {
                thisFATSecNum = temp;
                if (ahci_read(0, thisFATSecNum, 0, 1, fatsectordata) != 1)
                    return -1;
            }
            thisFATEntOffset = (dirCluster * 4) % pBpbinfo->BPB_BytsPerSec;
            dirCluster = *(uint32_t *)(&(fatsectordata[thisFATEntOffset])) & 0x0FFFFFFF;
            // TRACEFS("fs read thisFATSecNum:%d thisFATEntOffset:%d dirCluster:%d\n",thisFATSecNum,thisFATEntOffset,dirCluster);
            if (dirCluster >= 0x0FFFFFF8)
                return -1;
            dirClusterSecIndex = (dirCluster - 2) * pBpbinfo->BPB_SecPerClus + volumeInfo.FirstDataSector;
            dirClusterSecEnd = dirClusterSecIndex + pBpbinfo->BPB_SecPerClus;
        }
        // TRACEFS("fs read dirCluIndex:%d dirClusterSecEnd:%d\n",dirClusterSecIndex,dirClusterSecEnd);
        if (ahci_read(0, dirClusterSecIndex++, 0, 1, sectordata) != 1)
            return -1;
        uint32_t entryindex = 0;
        while (entryindex < 512)
        {
            // TRACEFS("fs read item entry[0]:0x%x dirItemCount:%d\n",(uint8_t)(sectordata[entryindex]),dirItemCount);
            if ((uint8_t)(sectordata[entryindex]) == 0)
                return -1;

            if ((uint8_t)(sectordata[entryindex]) == (uint8_t)0xE5)
            {
                descBuffIndex = 0;
                descBuffSize = 0;
                entryindex += 0x20;
                continue;
            }
            Fat32LongEntryInfo *plongnameentry = &(sectordata[entryindex]);
            if (plongnameentry->LDIR_Attr == ATTR_LONG_NAME)
            {
                entryindex += 0x20;
                descBuffSize += 0x20;
                if (descBuffIndex <= descbufflen - 0x20)
                {
                    memcpy_s(descbuff + descBuffIndex, plongnameentry, 0x20);
                    descBuffIndex += 0x20;
                }
                continue;
            }

            if ((plongnameentry->LDIR_Attr & ATTR_LONG_NAME_MASK) != ATTR_LONG_NAME)
            {
                if ((plongnameentry->LDIR_Attr & (ATTR_DIRECTORY | ATTR_VOLUME_ID)) == 0x00)
                {
                    if (dirItemIndex == itemIndex)
                    {
                        descBuffSize += 0x20;
                        if (descBuffIndex <= descbufflen - 0x20)
                        {
                            memcpy_s(descbuff + descBuffIndex, plongnameentry, 0x20);
                            descBuffIndex += 0x20;
                        }
                        if (descBuffIndex == descBuffSize)
                            return descBuffSize;
                        else
                            return -1;
                    }
                    /* Found a file. */

                }
                else if ((plongnameentry->LDIR_Attr & (ATTR_DIRECTORY | ATTR_VOLUME_ID)) == ATTR_DIRECTORY)
                {
                    if (dirItemIndex == itemIndex)
                    {
                        descBuffSize += 0x20;
                        if (descBuffIndex <= descbufflen - 0x20)
                        {
                            memcpy_s(descbuff + descBuffIndex, plongnameentry, 0x20);
                            descBuffIndex += 0x20;
                        }
                        if (descBuffIndex == descBuffSize)
                            return descBuffSize;
                        else
                            return -1;
                    }
                    /* Found a directory. */
                }
                else if ((plongnameentry->LDIR_Attr & (ATTR_DIRECTORY | ATTR_VOLUME_ID)) == ATTR_VOLUME_ID)
                {
                    /* Found a volume label. */
                    if (dirItemIndex == itemIndex)
                    {
                        descBuffSize += 0x20;
                        if (descBuffIndex <= descbufflen - 0x20)
                        {
                            memcpy_s(descbuff + descBuffIndex, plongnameentry, 0x20);
                            descBuffIndex += 0x20;
                        }
                        if (descBuffIndex == descBuffSize)
                            return descBuffSize;
                        else
                            return -1;
                    }
                }
                /* Found an invalid directory entry. */
            }
            descBuffIndex = 0;
            descBuffSize = 0;
            entryindex += 0x20;
        }
    }
    return -1;
}
int get_dir_item_count(const char *dirpath)
{
    size_t len = strlen_s(dirpath);
    if (len < 1)
        return -1;
    char *pPathStr = dirpath;
    uint32_t startCluster = pBpbinfo->BPB_RootClus;
    size_t index = 0;
    for (; index < len; index++)
    {
    }
}
int _get_dir_item_count(uint32_t firstclusternum)
{
    uint32_t dirCluster = firstclusternum;
    uint32_t dirClusterSecIndex = (dirCluster - 2) * pBpbinfo->BPB_SecPerClus + volumeInfo.FirstDataSector;
    uint32_t dirClusterSecEnd = dirClusterSecIndex + pBpbinfo->BPB_SecPerClus;

    uint32_t thisFATSecNum = 0;
    uint32_t thisFATEntOffset = 0;
    char fatsectordata[512] = {0};

    int dirItemCount = 0;
    char sectordata[512] = {0};

    while (1)
    {
        if (dirClusterSecIndex >= dirClusterSecEnd)
        {
            uint32_t temp = partioninfo.fsStartLBA + pBpbinfo->BPB_RsvdSecCnt + (dirCluster * 4) / pBpbinfo->BPB_BytsPerSec;
            // TRACEFS("fs read thisFATSecNum:%d temp:%d\n",thisFATSecNum,temp);
            if (thisFATSecNum != temp)
            {
                thisFATSecNum = temp;
                if (ahci_read(0, thisFATSecNum, 0, 1, fatsectordata) != 1)
                    return -1;
            }
            thisFATEntOffset = (dirCluster * 4) % pBpbinfo->BPB_BytsPerSec;
            dirCluster = *(uint32_t *)(&(fatsectordata[thisFATEntOffset])) & 0x0FFFFFFF;
            // TRACEFS("fs read thisFATSecNum:%d thisFATEntOffset:%d dirCluster:%d\n",thisFATSecNum,thisFATEntOffset,dirCluster);
            if (dirCluster >= 0x0FFFFFF8)
                return dirItemCount;
            dirClusterSecIndex = (dirCluster - 2) * pBpbinfo->BPB_SecPerClus + volumeInfo.FirstDataSector;
            dirClusterSecEnd = dirClusterSecIndex + pBpbinfo->BPB_SecPerClus;
        }
        // TRACEFS("fs read dirCluIndex:%d dirClusterSecEnd:%d\n",dirClusterSecIndex,dirClusterSecEnd);
        if (ahci_read(0, dirClusterSecIndex++, 0, 1, sectordata) != 1)
            return -1;
        uint32_t entryindex = 0;
        while (entryindex < 512)
        {
            // TRACEFS("fs read item entry[0]:0x%x dirItemCount:%d\n",(uint8_t)(sectordata[entryindex]),dirItemCount);
            if ((uint8_t)(sectordata[entryindex]) == 0)
                return dirItemCount;

            if ((uint8_t)(sectordata[entryindex]) == (uint8_t)0xE5)
            {
                entryindex += 0x20;
                continue;
            }
            Fat32LongEntryInfo *plongnameentry = &(sectordata[entryindex]);
            if ((plongnameentry->LDIR_Attr & ATTR_LONG_NAME_MASK) == ATTR_LONG_NAME)
            {
                entryindex += 0x20;
                continue;
            }

            if ((plongnameentry->LDIR_Attr & ATTR_LONG_NAME_MASK) != ATTR_LONG_NAME)
            {
                if ((plongnameentry->LDIR_Attr & (ATTR_DIRECTORY | ATTR_VOLUME_ID)) == 0x00)
                {
                    /* Found a file. */
                    dirItemCount++;
                }
                else if ((plongnameentry->LDIR_Attr & (ATTR_DIRECTORY | ATTR_VOLUME_ID)) == ATTR_DIRECTORY)
                {
                    /* Found a directory. */
                    dirItemCount++;
                }
                else if ((plongnameentry->LDIR_Attr & (ATTR_DIRECTORY | ATTR_VOLUME_ID)) == ATTR_VOLUME_ID)
                {
                    /* Found a volume label. */
                    dirItemCount++;
                }
                /* Found an invalid directory entry. */
            }
            entryindex += 0x20;
        }
    }
    return dirItemCount;
}
int _get_dir_item_descsize(uint32_t firstclusternum, uint32_t itemIndex)
{
    uint32_t dirCluster = firstclusternum;
    uint32_t dirClusterSecIndex = (dirCluster - 2) * pBpbinfo->BPB_SecPerClus + volumeInfo.FirstDataSector;
    uint32_t dirClusterSecEnd = dirClusterSecIndex + pBpbinfo->BPB_SecPerClus;

    uint32_t thisFATSecNum = 0;
    uint32_t thisFATEntOffset = 0;
    char fatsectordata[512] = {0};

    int dirItemIndex = 0;
    int descBuffSize = 0;
    char sectordata[512] = {0};

    while (1)
    {
        if (dirClusterSecIndex >= dirClusterSecEnd)
        {
            uint32_t temp = partioninfo.fsStartLBA + pBpbinfo->BPB_RsvdSecCnt + (dirCluster * 4) / pBpbinfo->BPB_BytsPerSec;
            // TRACEFS("fs read thisFATSecNum:%d temp:%d\n",thisFATSecNum,temp);
            if (thisFATSecNum != temp)
            {
                thisFATSecNum = temp;
                if (ahci_read(0, thisFATSecNum, 0, 1, fatsectordata) != 1)
                    return -1;
            }
            thisFATEntOffset = (dirCluster * 4) % pBpbinfo->BPB_BytsPerSec;
            dirCluster = *(uint32_t *)(&(fatsectordata[thisFATEntOffset])) & 0x0FFFFFFF;
            // TRACEFS("fs read thisFATSecNum:%d thisFATEntOffset:%d dirCluster:%d\n",thisFATSecNum,thisFATEntOffset,dirCluster);
            if (dirCluster >= 0x0FFFFFF8)
                return -1;
            dirClusterSecIndex = (dirCluster - 2) * pBpbinfo->BPB_SecPerClus + volumeInfo.FirstDataSector;
            dirClusterSecEnd = dirClusterSecIndex + pBpbinfo->BPB_SecPerClus;
        }
        // TRACEFS("fs read dirCluIndex:%d dirClusterSecEnd:%d\n",dirClusterSecIndex,dirClusterSecEnd);
        if (ahci_read(0, dirClusterSecIndex++, 0, 1, sectordata) != 1)
            return -1;
        uint32_t entryindex = 0;
        while (entryindex < 512)
        {
            // TRACEFS("fs read item entry[0]:0x%x dirItemCount:%d\n",(uint8_t)(sectordata[entryindex]),dirItemCount);
            if ((uint8_t)(sectordata[entryindex]) == 0)
                return -1;

            if ((uint8_t)(sectordata[entryindex]) == (uint8_t)0xE5)
            {
                descBuffSize = 0;
                entryindex += 0x20;
                continue;
            }
            Fat32LongEntryInfo *plongnameentry = &(sectordata[entryindex]);
            if (plongnameentry->LDIR_Attr == ATTR_LONG_NAME)
            {
                entryindex += 0x20;
                descBuffSize += 0x20;
                continue;
            }

            if ((plongnameentry->LDIR_Attr & ATTR_LONG_NAME_MASK) != ATTR_LONG_NAME)
            {
                if ((plongnameentry->LDIR_Attr & (ATTR_DIRECTORY | ATTR_VOLUME_ID)) == 0x00)
                {
                    if (dirItemIndex == itemIndex)
                    {
                        descBuffSize += 0x20;
                        return descBuffSize;
                    }
                    /* Found a file. */
                    dirItemIndex++;
                }
                else if ((plongnameentry->LDIR_Attr & (ATTR_DIRECTORY | ATTR_VOLUME_ID)) == ATTR_DIRECTORY)
                {
                    if (dirItemIndex == itemIndex)
                    {
                        descBuffSize += 0x20;
                        return descBuffSize;
                    }
                    /* Found a directory. */
                    dirItemIndex++;
                }
                else if ((plongnameentry->LDIR_Attr & (ATTR_DIRECTORY | ATTR_VOLUME_ID)) == ATTR_VOLUME_ID)
                {
                    /* Found a volume label. */
                    if (dirItemIndex == itemIndex)
                    {
                        descBuffSize += 0x20;
                        return descBuffSize;
                    }
                    dirItemIndex++;
                }
                /* Found an invalid directory entry. */
            }
            descBuffSize = 0;
            entryindex += 0x20;
        }
    }
    return descBuffSize;
}

int _get_dir_item_descdata_fromindex(uint32_t firstclusternum, uint32_t itemIndex, char *descbuff, uint32_t descbufflen)
{
    uint32_t dirCluster = firstclusternum;
    uint32_t dirClusterSecIndex = (dirCluster - 2) * pBpbinfo->BPB_SecPerClus + volumeInfo.FirstDataSector;
    uint32_t dirClusterSecEnd = dirClusterSecIndex + pBpbinfo->BPB_SecPerClus;

    uint32_t thisFATSecNum = 0;
    uint32_t thisFATEntOffset = 0;
    char fatsectordata[512] = {0};

    int dirItemIndex = 0;
    int descBuffIndex = 0;
    char sectordata[512] = {0};
    int descBuffSize = 0;
    while (1)
    {
        if (dirClusterSecIndex >= dirClusterSecEnd)
        {
            uint32_t temp = partioninfo.fsStartLBA + pBpbinfo->BPB_RsvdSecCnt + (dirCluster * 4) / pBpbinfo->BPB_BytsPerSec;
            // TRACEFS("fs read thisFATSecNum:%d temp:%d\n",thisFATSecNum,temp);
            if (thisFATSecNum != temp)
            {
                thisFATSecNum = temp;
                if (ahci_read(0, thisFATSecNum, 0, 1, fatsectordata) != 1)
                    return -1;
            }
            thisFATEntOffset = (dirCluster * 4) % pBpbinfo->BPB_BytsPerSec;
            dirCluster = *(uint32_t *)(&(fatsectordata[thisFATEntOffset])) & 0x0FFFFFFF;
            // TRACEFS("fs read thisFATSecNum:%d thisFATEntOffset:%d dirCluster:%d\n",thisFATSecNum,thisFATEntOffset,dirCluster);
            if (dirCluster >= 0x0FFFFFF8)
                return -1;
            dirClusterSecIndex = (dirCluster - 2) * pBpbinfo->BPB_SecPerClus + volumeInfo.FirstDataSector;
            dirClusterSecEnd = dirClusterSecIndex + pBpbinfo->BPB_SecPerClus;
        }
        // TRACEFS("fs read dirCluIndex:%d dirClusterSecEnd:%d\n",dirClusterSecIndex,dirClusterSecEnd);
        if (ahci_read(0, dirClusterSecIndex++, 0, 1, sectordata) != 1)
            return -1;
        uint32_t entryindex = 0;
        while (entryindex < 512)
        {
            // TRACEFS("fs read item entry[0]:0x%x dirItemCount:%d\n",(uint8_t)(sectordata[entryindex]),dirItemCount);
            if ((uint8_t)(sectordata[entryindex]) == 0)
                return -1;

            if ((uint8_t)(sectordata[entryindex]) == (uint8_t)0xE5)
            {
                descBuffIndex = 0;
                descBuffSize = 0;
                entryindex += 0x20;
                continue;
            }
            Fat32LongEntryInfo *plongnameentry = &(sectordata[entryindex]);
            if (plongnameentry->LDIR_Attr == ATTR_LONG_NAME)
            {
                entryindex += 0x20;
                descBuffSize += 0x20;
                if (descBuffIndex <= descbufflen - 0x20)
                {
                    memcpy_s(descbuff + descBuffIndex, plongnameentry, 0x20);
                    descBuffIndex += 0x20;
                }
                continue;
            }

            if ((plongnameentry->LDIR_Attr & ATTR_LONG_NAME_MASK) != ATTR_LONG_NAME)
            {
                if ((plongnameentry->LDIR_Attr & (ATTR_DIRECTORY | ATTR_VOLUME_ID)) == 0x00)
                {
                    if (dirItemIndex == itemIndex)
                    {
                        descBuffSize += 0x20;
                        if (descBuffIndex <= descbufflen - 0x20)
                        {
                            memcpy_s(descbuff + descBuffIndex, plongnameentry, 0x20);
                            descBuffIndex += 0x20;
                        }
                        if (descBuffIndex == descBuffSize)
                            return descBuffSize;
                        else
                            return -1;
                    }
                    /* Found a file. */
                    dirItemIndex++;
                }
                else if ((plongnameentry->LDIR_Attr & (ATTR_DIRECTORY | ATTR_VOLUME_ID)) == ATTR_DIRECTORY)
                {
                    if (dirItemIndex == itemIndex)
                    {
                        descBuffSize += 0x20;
                        if (descBuffIndex <= descbufflen - 0x20)
                        {
                            memcpy_s(descbuff + descBuffIndex, plongnameentry, 0x20);
                            descBuffIndex += 0x20;
                        }
                        if (descBuffIndex == descBuffSize)
                            return descBuffSize;
                        else
                            return -1;
                    }
                    /* Found a directory. */
                    dirItemIndex++;
                }
                else if ((plongnameentry->LDIR_Attr & (ATTR_DIRECTORY | ATTR_VOLUME_ID)) == ATTR_VOLUME_ID)
                {
                    /* Found a volume label. */
                    if (dirItemIndex == itemIndex)
                    {
                        descBuffSize += 0x20;
                        if (descBuffIndex <= descbufflen - 0x20)
                        {
                            memcpy_s(descbuff + descBuffIndex, plongnameentry, 0x20);
                            descBuffIndex += 0x20;
                        }
                        if (descBuffIndex == descBuffSize)
                            return descBuffSize;
                        else
                            return -1;
                    }
                    dirItemIndex++;
                }
                /* Found an invalid directory entry. */
            }
            descBuffIndex = 0;
            descBuffSize = 0;
            entryindex += 0x20;
        }
    }
    return -1;
}
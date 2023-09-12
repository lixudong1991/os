#include "acpi.h"
#include "boot.h"
#include "memcachectl.h"

static void getnext_KMP(const char *dest, int next[], int len)
{
    int i = 0, j = -1;
    next[0] = j;
    while (i < len - 1)
    {
        if (j == -1 || dest[i] == dest[j])
        {
            ++i;
            ++j;
            if (dest[i] != dest[j])
            {
                next[i] = j;
            }
            else
                next[i] = next[j];
        }
        else
            j = next[j];
    }
}
static int IndexStr_KMP(char *str, int strsize, const char *dest, int *nextbuff, int destsize)
{
    int len = destsize;
    int *next = nextbuff;
    memset_s(next, 0, len * sizeof(int));
    getnext_KMP(dest, next, len);
    int size1 = strsize;
    int i = 0,j = 0;
    while (i < size1 && j < len)
    {
        if (j == -1 || str[i] == dest[j])
        {
            ++i;
            ++j;
        }
        else
        {
            j = next[j];
        }
    }

    if (j == len)
        return i - len;
    return -1;
}

char *findRSDPAddr()
{
    for (int i = FIND_RSDP_START; i < FIND_RSDP_END; i += 0x1000)
    {
        mem4k_map(i, i, MEM_UC, PAGE_G | PAGE_RW);
    }
    char rsdpsig[] = "RSD PTR ";
    int nextbuff[8] = {0};
    int rsdtindex = IndexStr_KMP(FIND_RSDP_START, FIND_RSDP_END - FIND_RSDP_START, rsdpsig, nextbuff, 8);
    char *ret = NULL;
    if (rsdtindex != -1)
        ret = (char *)(FIND_RSDP_START + rsdtindex);
    return ret;
}
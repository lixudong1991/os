#include "string.h"
#include "stdint.h"
int strncmp_s(const char *cs, const char *ct, uint32_t count)
{
	unsigned char c1, c2;

	while (count)
	{
		c1 = *cs++;
		c2 = *ct++;
		if (c1 != c2)
			return c1 < c2 ? -1 : 1;
		if (!c1)
			break;
		count--;
	}
	return 0;
}

uint32_t strnlen(const char *s, uint32_t maxlen)
{
	const char *es = s;
	while (*es && maxlen)
	{
		es++;
		maxlen--;
	}

	return (es - s);
}
int strcmp_s(const char *str1, const char *str2)
{
	const unsigned char *s1 = (const unsigned char *)str1;
	const unsigned char *s2 = (const unsigned char *)str2;
	int delta = 0;

	while (*s1 || *s2)
	{
		delta = *s1 - *s2;
		if (delta)
			return delta;
		s1++;
		s2++;
	}
	return 0;
}

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
int IndexStr_KMP(char *str, int strsize, const char *dest, int *nextbuff, int destsize)
{
    int len = destsize;
    int *next = nextbuff;
    memset_s(next, 0, len * sizeof(int));
    getnext_KMP(dest, next, len);
    int size1 = strsize;
    int i = 0, j = 0;
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

#include "boot.h"

int strncmp_s(const char *cs, const char *ct, uint32 count)
{
	unsigned char c1, c2;

	while (count) {
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

uint32 strnlen(const char *s, uint32 maxlen)
{
	const char *es = s;
	while (*es && maxlen) {
		es++;
		maxlen--;
	}

	return (es - s);
}
int strcmp_s(const char* str1, const char* str2)
{
	const unsigned char* s1 = (const unsigned char*)str1;
	const unsigned char* s2 = (const unsigned char*)str2;
	int delta = 0;

	while (*s1 || *s2) {
		delta = *s1 - *s2;
		if (delta)
			return delta;
		s1++;
		s2++;
	}
	return 0;
}
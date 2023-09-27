/* SPDX-License-Identifier: GPL-2.0 */
#ifndef BOOT_CTYPE_H
#define BOOT_CTYPE_H


static inline int isdigit(int ch)
{
	return (ch >= '0') && (ch <= '9');
}

static inline int isxdigit(int ch)
{
	if (isdigit(ch))
		return 1;

	if ((ch >= 'a') && (ch <= 'f'))
		return 1;

	return (ch >= 'A') && (ch <= 'F');
}
static inline int __isspace(int _c)
{
	return _c == ' ' || (unsigned)_c-'\t' < 5;
}

#define isspace(a) __isspace(a)
#endif

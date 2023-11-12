// SPDX-License-Identifier: GPL-2.0-only
/* -*- linux-c -*- ------------------------------------------------------- *
 *
 *   Copyright (C) 1991, 1992 Linus Torvalds
 *   Copyright 2007 rPath, Inc. - All Rights Reserved
 *
 * ----------------------------------------------------------------------- */

/*
 * Oh, it's a waste of space, but oh-so-yummy for debugging.  This
 * version of printf() does not include 64-bit support.  "Live with
 * it."
 *
 */

#include "boot.h"
#include "printf.h"
#include "string.h"
int printf(const char *fmt, ...)
{
	char printf_buf[1024];
	va_list args;
	int printed;

	va_start(args, fmt);
	printed = vsprintf(printf_buf, fmt, args);
	va_end(args);
	//asm("cli");
	puts(printf_buf);
	
//	asm("sti");
	return printed;
}

int interruptPrintf(const char *fmt, ...)
{
	char printf_buf[1024];
	va_list args;
	int printed;

	va_start(args, fmt);
	printed = vsprintf(printf_buf, fmt, args);
	va_end(args);
	puts(printf_buf);
	return printed;
}
void putchar(int _Character)
{
	char c = (char)_Character;
	if(c ==0x0d)
		return;
	uint32 pos = getcursor();
	if (c == 0x0a)
	{
		pos = pos / 80 * 80;
		pos += 80;
	}
	else
	{
		*(char *)(0xb8000 + pos * 2) = c;
		pos++;
	}
	if (pos > 1999)
	{
		pos = 1920;
		memcpy_s((char *)0xb8000, (char *)0xb80a0, 3840);
		short *pchar = (short *)(0xb8000 + 3840);
		memWordset_s(pchar,0x0720,80);
	}
	setcursor(pos);
}
void puts(const char *str)
{
	spinlock(lockBuff[PRINT_LOCK].plock);
	char *pstr = str;
	while (*pstr != 0)
		putchar(*pstr++);
	unlock(lockBuff[PRINT_LOCK].plock);
}
void clearChars(int count)
{
	uint32 pos = getcursor();

	for(int start=pos-count;start<pos;start++)
	{
		*(char*)(0xb8000 + start * 2) = 0x20;
	}
	setcursor(pos-count);
}
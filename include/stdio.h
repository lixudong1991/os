#ifndef _STDIO_H_H
#define _STDIO_H_H
#include "stdint.h"

/*
struct _IO_FILE {
	unsigned flags;
	unsigned char *rpos, *rend;
	int (*close)(_IO_FILE *);
	unsigned char *wend, *wpos;
	unsigned char *mustbezero_1;
	unsigned char *wbase;
	size_t (*read)(_IO_FILE *, unsigned char *, size_t);
	size_t (*write)(_IO_FILE *, const unsigned char *, size_t);
	off_t (*seek)(_IO_FILE *, off_t, int);
	unsigned char *buf;
	size_t buf_size;
	_IO_FILE *prev, *next;
	int fd;
	int pipe_pid;
	long lockcount;
	int mode;
	volatile int lock;
	int lbf;
	void *cookie;
	off_t off;
	char *getln_buf;
	void *mustbezero_2;
	unsigned char *shend;
	off_t shlim, shcnt;
	_IO_FILE *prev_locked, *next_locked;
	struct __locale_struct *locale;
};*/


typedef struct _IO_FILE { char __x; }FILE;

int printf(const char *fmt, ...);

void putchar(int _Character);
void puts(const char *str);

int fprintf(FILE *__restrict, const char *__restrict, ...);
int sprintf(char *__restrict, const char *__restrict, ...);
int snprintf(char *__restrict, size_t, const char *__restrict, ...);

int vprintf(const char *__restrict, va_list);
int vfprintf(FILE *__restrict, const char *__restrict, va_list);
int vsprintf(char *__restrict, const char *__restrict, va_list);
int vsnprintf(char *__restrict, size_t, const char *__restrict, va_list);


uint32 getchar();
int fgets(char* s, int size);

#endif
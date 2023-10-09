#ifndef STD_INT_H_
#define STD_INT_H_

typedef unsigned int uint32;
typedef unsigned int uint32_t;
typedef int int32;
typedef int int32_t;
typedef unsigned short uint16;
typedef unsigned short uint16_t;
typedef short int16;
typedef unsigned long long uint64;
typedef unsigned long long uint64_t;
typedef long long int64;
typedef long long int64_t;
typedef char *va_list;
typedef unsigned char uint8_t;
typedef unsigned int size_t;

typedef int BOOL;
#define TRUE 1
#define FALSE 0

#define NULL 0

typedef uint8_t BYTE;
typedef uint16_t WORD;
typedef uint32_t DWORD;
typedef uint64_t QWORD;


typedef uint32_t phys_addr_t;
typedef uint32_t phys_size_t;

#define PAGE_ADDR_MASK 0xFFFFF000

#endif
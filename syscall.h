#ifndef SYSCALL__H_H
#define SYSCALL__H_H


void __attribute__((__stdcall__)) puts_s(const char *str);
int __attribute__((__stdcall__)) readSectors_s(char* des, int startSector, int count);
void __attribute__((__stdcall__)) clearScreen_s();
void __attribute__((__stdcall__)) exit_s(unsigned int retval);
void __attribute__((__stdcall__)) setcursor_s(unsigned int pos);

#endif
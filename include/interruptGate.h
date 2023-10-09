#ifndef INTERRUPTGATE_H_
#define INTERRUPTGATE_H_
#include "stdint.h"

extern uint32 exceptionCalls[20]; //前20个中断向量

extern void interrupt_27_handler();
extern void interrupt_28_handler();

extern void interrupt_70_handler();
extern void interrupt_8259a_handler();  
extern void interrupt_80_handler();
extern void interrupt_81_handler();
extern void interrupt_82_handler();
extern void interrupt_83_handler();
extern void interrupt_84_handler();

extern void interrupt_78_handler();

extern void general_interrupt_handler();
#endif // !INTERRUPTGATE_H_

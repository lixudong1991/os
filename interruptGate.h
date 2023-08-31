#ifndef INTERRUPTGATE_H_
#define INTERRUPTGATE_H_

extern void general_interrupt_handler();
extern void interrupt_8259a_handler();
extern void interrupt_70_handler();
extern void local_x2apic_error_handling();
extern void systemCall();
extern uint32 exceptionCalls[20];

#endif // !INTERRUPTGATE_H_

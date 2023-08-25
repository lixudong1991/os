#ifndef INTERRUPTGATE_H_
#define INTERRUPTGATE_H_

void general_interrupt_handler();
void interrupt_8259a_handler();
void interrupt_70_handler();
void systemCall();
extern uint32 exceptionCalls[20];

#endif // !INTERRUPTGATE_H_

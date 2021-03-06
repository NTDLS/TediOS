////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#ifndef _IRQ_C
#define _IRQ_C
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/*
    Interrupt Request management
*/
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#include "System.H"

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

/*
    These are own ISRs that point to our special IRQ handler
    instead of the regular 'ISR_FaultHandler' function
*/
extern void irq0();
extern void irq1();
extern void irq2();
extern void irq3();
extern void irq4();
extern void irq5();
extern void irq6();
extern void irq7();
extern void irq8();
extern void irq9();
extern void irq10();
extern void irq11();
extern void irq12();
extern void irq13();
extern void irq14();
extern void irq15();

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void IRQ_Install_Handler(int irq, void (*handler)(struct regs *r));
void IRQ_Uninstall_Handler(int irq);
void IRQ_Remap(void);
bool IRQ_Install(void);

extern "C" void IRQ_Handler(struct regs *r);

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#endif

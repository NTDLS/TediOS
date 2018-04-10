////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#ifndef _IRQ_C
#define _IRQ_C
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/*
    Interrupt Request management
*/
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#include "../Lib/STDLib.H"

#include "System.H"
#include "IDT.H"

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

//These are our own ISRs that point to our special IRQ handler
//	instead of the regular 'ISR_FaultHandler' function.
extern "C" void irq0();
extern "C" void irq1();
extern "C" void irq2();
extern "C" void irq3();
extern "C" void irq4();
extern "C" void irq5();
extern "C" void irq6();
extern "C" void irq7();
extern "C" void irq8();
extern "C" void irq9();
extern "C" void irq10();
extern "C" void irq11();
extern "C" void irq12();
extern "C" void irq13();
extern "C" void irq14();
extern "C" void irq15();

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

/* This array is actually an array of function pointers. We use
*  this to handle custom IRQ handlers for a given IRQ */
void (*IRQ_Routines[16])(struct regs *r) =
{
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0
};

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

/* This installs a custom IRQ handler for the given IRQ */
void IRQ_Install_Handler(int irq, void (*handler)(struct regs *r))
{
    IRQ_Routines[irq] = handler;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

/* This clears the handler for a given IRQ */
void IRQ_Uninstall_Handler(int irq)
{
    IRQ_Routines[irq] = 0;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

/* Normally, IRQs 0 to 7 are mapped to entries 8 to 15. This
*  is a problem in protected mode, because IDT entry 8 is a
*  Double Fault! Without remapping, every time IRQ0 fires,
*  you get a Double Fault Exception, which is NOT actually
*  what's happening. We send commands to the Programmable
*  Interrupt Controller (PICs - also called the 8259's) in
*  order to make IRQ0 to 15 be remapped to IDT entries 32 to
*  47 */
void IRQ_Remap(void)
{
    outportb(0x20, 0x11);
    outportb(0xA0, 0x11);
    outportb(0x21, 0x20);
    outportb(0xA1, 0x28);
    outportb(0x21, 0x04);
    outportb(0xA1, 0x02);
    outportb(0x21, 0x01);
    outportb(0xA1, 0x01);
    outportb(0x21, 0x0);
    outportb(0xA1, 0x0);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

/* We first remap the interrupt controllers, and then we install
*  the appropriate ISRs to the correct entries in the IDT. This
*  is just like installing the exception handlers */
bool IRQ_Install(void)
{
    IRQ_Remap();

    IDT_Set_Gate(32, (unsigned)irq0, 0x08, 0x8E);
    IDT_Set_Gate(33, (unsigned)irq1, 0x08, 0x8E);
    IDT_Set_Gate(34, (unsigned)irq2, 0x08, 0x8E);
    IDT_Set_Gate(35, (unsigned)irq3, 0x08, 0x8E);
    IDT_Set_Gate(36, (unsigned)irq4, 0x08, 0x8E);
    IDT_Set_Gate(37, (unsigned)irq5, 0x08, 0x8E);
    IDT_Set_Gate(38, (unsigned)irq6, 0x08, 0x8E);
    IDT_Set_Gate(39, (unsigned)irq7, 0x08, 0x8E);

    IDT_Set_Gate(40, (unsigned)irq8, 0x08, 0x8E);
    IDT_Set_Gate(41, (unsigned)irq9, 0x08, 0x8E);
    IDT_Set_Gate(42, (unsigned)irq10, 0x08, 0x8E);
    IDT_Set_Gate(43, (unsigned)irq11, 0x08, 0x8E);
    IDT_Set_Gate(44, (unsigned)irq12, 0x08, 0x8E);
    IDT_Set_Gate(45, (unsigned)irq13, 0x08, 0x8E);
    IDT_Set_Gate(46, (unsigned)irq14, 0x08, 0x8E);
    IDT_Set_Gate(47, (unsigned)irq15, 0x08, 0x8E);

	return true;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

/* Each of the IRQ ISRs point to this function, rather than
*  the 'ISR_FaultHandler' in 'ISR.cpp'. The IRQ Controllers need
*  to be told when you are done servicing them, so you need
*  to send them an "End of Interrupt" command (0x20). There
*  are two 8259 chips: The first exists at 0x20, the second
*  exists at 0xA0. If the second controller (an IRQ from 8 to
*  15) gets an interrupt, you need to acknowledge the
*  interrupt at BOTH controllers, otherwise, you only send
*  an EOI command to the first controller. If you don't send
*  an EOI, you won't raise any more IRQs */
extern "C" void IRQ_Handler(struct regs *r)
{
    void (*handler)(struct regs *r); //This is a blank function pointer.

    //Find out if we have a custom handler to run for this IRQ, and then finally, run it.
    handler = IRQ_Routines[r->int_no - 32];
    if (handler)
    {
        handler(r);
    }

    /* If the IDT entry that was invoked was greater than 40
    *  (meaning IRQ8 - 15), then we need to send an EOI to
    *  the slave controller */
    if (r->int_no >= 40)
    {
        outportb(0xA0, 0x20);
    }

    //In either case, we need to send an EOI to the master interrupt controller too.
    outportb(0x20, 0x20);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#endif

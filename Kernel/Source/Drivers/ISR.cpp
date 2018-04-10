////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#ifndef _ISR_C
#define _ISR_C
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/*
    Interrupt Service Routines installer and exceptions
*/
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#include "../Lib/STDLib.H"

#include "System.H"
#include "IDT.H"
#include "ISR.H"
#include "Video.H"

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/*
    This is a simple string array. It contains the message that
        corresponds to each and every exception. We get the correct
        message by accessing like:
        exception_message[interrupt_number]
*/
unsigned char *gsISR_Msssages[] =
{
    (unsigned char *)    "Division By Zero",
    (unsigned char *)    "Debug",
    (unsigned char *)    "Non Maskable Interrupt",
    (unsigned char *)    "Breakpoint",
    (unsigned char *)    "Into Detected Overflow",
    (unsigned char *)    "Out of Bounds",
    (unsigned char *)    "Invalid Opcode",
    (unsigned char *)    "No Coprocessor",

    (unsigned char *)    "Double Fault",
    (unsigned char *)    "Coprocessor Segment Overrun",
    (unsigned char *)    "Bad TSS",
    (unsigned char *)    "Segment Not Present",
    (unsigned char *)    "Stack Fault",
    (unsigned char *)    "General Protection Fault",
    (unsigned char *)    "Page Fault",
    (unsigned char *)    "Unknown Interrupt",

    (unsigned char *)    "Coprocessor Fault",
    (unsigned char *)    "Alignment Check",
    (unsigned char *)    "Machine Check",
    (unsigned char *)    "Reserved 1",
    (unsigned char *)    "Reserved 2",
    (unsigned char *)    "Reserved 3",
    (unsigned char *)    "Reserved 4",
    (unsigned char *)    "Reserved 5",

    (unsigned char *)    "Reserved 6",
    (unsigned char *)    "Reserved 7",
    (unsigned char *)    "Reserved 8",
    (unsigned char *)    "Reserved 9",
    (unsigned char *)    "Reserved 10",
    (unsigned char *)    "Reserved 11",
    (unsigned char *)    "Reserved 12",
    (unsigned char *)    "Reserved 13"
};

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/*
    This is a very repetitive function... it's not hard, it's
        just annoying. As you can see, we set the first 32 entries
        in the IDT to the first 32 ISRs. We can't use a for loop
        for this, because there is no way to get the function names
        that correspond to that given entry. We set the access
        flags to 0x8E. This means that the entry is present, is
        running in ring 0 (kernel level), and has the lower 5 bits
        set to the required '14', which is represented by 'E' in
        hex.
*/
bool ISR_Install(void)
{
    IDT_Set_Gate(0, (unsigned)isr0, 0x08, 0x8E);
    IDT_Set_Gate(1, (unsigned)isr1, 0x08, 0x8E);
    IDT_Set_Gate(2, (unsigned)isr2, 0x08, 0x8E);
    IDT_Set_Gate(3, (unsigned)isr3, 0x08, 0x8E);
    IDT_Set_Gate(4, (unsigned)isr4, 0x08, 0x8E);
    IDT_Set_Gate(5, (unsigned)isr5, 0x08, 0x8E);
    IDT_Set_Gate(6, (unsigned)isr6, 0x08, 0x8E);
    IDT_Set_Gate(7, (unsigned)isr7, 0x08, 0x8E);

    IDT_Set_Gate(8, (unsigned)isr8, 0x08, 0x8E);
    IDT_Set_Gate(9, (unsigned)isr9, 0x08, 0x8E);
    IDT_Set_Gate(10, (unsigned)isr10, 0x08, 0x8E);
    IDT_Set_Gate(11, (unsigned)isr11, 0x08, 0x8E);
    IDT_Set_Gate(12, (unsigned)isr12, 0x08, 0x8E);
    IDT_Set_Gate(13, (unsigned)isr13, 0x08, 0x8E);
    IDT_Set_Gate(14, (unsigned)isr14, 0x08, 0x8E);
    IDT_Set_Gate(15, (unsigned)isr15, 0x08, 0x8E);

    IDT_Set_Gate(16, (unsigned)isr16, 0x08, 0x8E);
    IDT_Set_Gate(17, (unsigned)isr17, 0x08, 0x8E);
    IDT_Set_Gate(18, (unsigned)isr18, 0x08, 0x8E);
    IDT_Set_Gate(19, (unsigned)isr19, 0x08, 0x8E);
    IDT_Set_Gate(20, (unsigned)isr20, 0x08, 0x8E);
    IDT_Set_Gate(21, (unsigned)isr21, 0x08, 0x8E);
    IDT_Set_Gate(22, (unsigned)isr22, 0x08, 0x8E);
    IDT_Set_Gate(23, (unsigned)isr23, 0x08, 0x8E);

    IDT_Set_Gate(24, (unsigned)isr24, 0x08, 0x8E);
    IDT_Set_Gate(25, (unsigned)isr25, 0x08, 0x8E);
    IDT_Set_Gate(26, (unsigned)isr26, 0x08, 0x8E);
    IDT_Set_Gate(27, (unsigned)isr27, 0x08, 0x8E);
    IDT_Set_Gate(28, (unsigned)isr28, 0x08, 0x8E);
    IDT_Set_Gate(29, (unsigned)isr29, 0x08, 0x8E);
    IDT_Set_Gate(30, (unsigned)isr30, 0x08, 0x8E);
    IDT_Set_Gate(31, (unsigned)isr31, 0x08, 0x8E);

    return true;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/*
    All of our Exception Handling Interrupt Service Routines will
        point to this function. This will tell us what exception has
        happened! Right now, we simply halt the system by hitting an
        endless loop. All ISRs disable interrupts while they are being
        serviced as a 'locking' mechanism to prevent an IRQ from
        happening and messing up kernel data structures
*/

extern "C" void ISR_FaultHandler(struct regs *r)
{
    if(r->int_no < 32)
    {
		//Home-Brew Blue Screen of Death
		BSOD((const char *)gsISR_Msssages[r->int_no]);
	}
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#endif

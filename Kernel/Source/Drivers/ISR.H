////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#ifndef _ISR_H_
#define _ISR_H_
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/*
    Interrupt Service Routines installer and exceptions
*/
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

extern unsigned char *gsISR_Msssages[];

/*
    These are function prototypes for all of the exception
        handlers: The first 32 entries in the IDT are reserved
        by Intel, and are designed to service exceptions!
*/

extern "C" void isr0(void);
extern "C" void isr1(void);
extern "C" void isr2(void);
extern "C" void isr3(void);
extern "C" void isr4(void);
extern "C" void isr5(void);
extern "C" void isr6(void);
extern "C" void isr7(void);
extern "C" void isr8(void);
extern "C" void isr9(void);
extern "C" void isr10(void);
extern "C" void isr11(void);
extern "C" void isr12(void);
extern "C" void isr13(void);
extern "C" void isr14(void);
extern "C" void isr15(void);
extern "C" void isr16(void);
extern "C" void isr17(void);
extern "C" void isr18(void);
extern "C" void isr19(void);
extern "C" void isr20(void);
extern "C" void isr21(void);
extern "C" void isr22(void);
extern "C" void isr23(void);
extern "C" void isr24(void);
extern "C" void isr25(void);
extern "C" void isr26(void);
extern "C" void isr27(void);
extern "C" void isr28(void);
extern "C" void isr29(void);
extern "C" void isr30(void);
extern "C" void isr31(void);

extern "C" void ISR_FaultHandler(struct regs *r);

bool ISR_Install(void);

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#endif

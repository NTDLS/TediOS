////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#ifndef _IDT_C_
#define _IDT_C_
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/*
    Interrupt Descriptor Table management
*/
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#include "../Lib/STDLib.H"

#include "IDT.H"

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

/* Defines an IDT entry */
struct idt_entry
{
    ushort base_lo;
    ushort sel;
    unsigned char always0;
    unsigned char flags;
    ushort base_hi;
} __attribute__((packed));

struct idt_ptr
{
    ushort limit;
    uint base;
} __attribute__((packed));

/* Declare an IDT of 256 entries. Although we will only use the
*  first 32 entries in this tutorial, the rest exists as a bit
*  of a trap. If any undefined IDT entry is hit, it normally
*  will cause an "Unhandled Interrupt" exception. Any descriptor
*  for which the 'presence' bit is cleared (0) will generate an
*  "Unhandled Interrupt" exception */
struct idt_entry idt[256];
struct idt_ptr idtp;

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

/* Use this function to set an entry in the IDT. Alot simpler
*  than twiddling with the GDT ;) */
void IDT_Set_Gate(unsigned char num, uint base, ushort sel, unsigned char flags)
{
    /* The interrupt routine's base address */
    idt[num].base_lo = (base & 0xFFFF);
    idt[num].base_hi = (base >> 16) & 0xFFFF;

    /* The segment or 'selector' that this IDT entry will use
    *  is set here, along with any access flags */
    idt[num].sel = sel;
    idt[num].always0 = 0;
    idt[num].flags = flags;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

/* Installs the IDT */
bool IDT_Install(void)
{
    /* Sets the special IDT pointer up, just like in 'gdt.c' */
    idtp.limit = (sizeof (struct idt_entry) * 256) - 1;
    idtp.base = (int)&idt;

    /* Clear out the entire IDT, initializing it to zeros */
    memset(&idt, 0, sizeof(struct idt_entry) * 256);

    /* Add any new ISRs to the IDT here using IDT_Set_Gate */



    /* Points the processor's internal register to the new IDT */
    IDT_Load();

    return true;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#endif

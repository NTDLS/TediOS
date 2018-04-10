////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#ifndef _GDT_C
#define _GDT_C
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/*
    Global Descriptor Table management
*/
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#include "../Lib/STDLib.H"

#include "GDT.H"

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

/* Defines a GDT entry */
struct gdt_entry
{
    ushort limit_low;
    ushort base_low;
    unsigned char base_middle;
    unsigned char access;
    unsigned char granularity;
    unsigned char base_high;
} __attribute__((packed));

struct gdt_ptr
{
    ushort limit;
    uint base;
} __attribute__((packed));

/* Our GDT, with 3 entries, and finally our special GDT pointer */
struct gdt_entry gdt[3];
struct gdt_ptr gp;

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

/* Setup a descriptor in the Global Descriptor Table */
void GDT_Set_Gate(int num, uint base, uint limit, unsigned char access, unsigned char gran)
{
    /* Setup the descriptor base address */
    gdt[num].base_low = (base & 0xFFFF);
    gdt[num].base_middle = (base >> 16) & 0xFF;
    gdt[num].base_high = (base >> 24) & 0xFF;

    /* Setup the descriptor limits */
    gdt[num].limit_low = (limit & 0xFFFF);
    gdt[num].granularity = ((limit >> 16) & 0x0F);

    /* Finally, set up the granularity and access flags */
    gdt[num].granularity |= (gran & 0xF0);
    gdt[num].access = access;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

/* Should be called by main. This will setup the special GDT
*  pointer, set up the first 3 entries in our GDT, and then
*  finally call GDT_Flush() in our assembler file in order
*  to tell the processor where the new GDT is and update the
*  new segment registers */
bool GDT_Install()
{
    /* Setup the GDT pointer and limit */
    gp.limit = (sizeof(struct gdt_entry) * 3) - 1;
    gp.base = (int)&gdt;

    /* Our NULL descriptor */
    GDT_Set_Gate(0, 0, 0, 0, 0);

    /* The second entry is our Code Segment. The base address
    *  is 0, the limit is 4GBytes, it uses 4KByte granularity,
    *  uses 32-bit opcodes, and is a Code Segment descriptor.
    *  Please check the table above in the tutorial in order
    *  to see exactly what each value means */
    GDT_Set_Gate(1, 0, 0xFFFFFFFF, 0x9A, 0xCF);

    /* The third entry is our Data Segment. It's EXACTLY the
    *  same as our code segment, but the descriptor type in
    *  this entry's access byte says it's a Data Segment */
    GDT_Set_Gate(2, 0, 0xFFFFFFFF, 0x92, 0xCF);

    /* Flush out the old GDT and install the new changes! */
    GDT_Flush();

    return true;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#endif

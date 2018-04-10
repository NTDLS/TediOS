////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#ifndef _SYSTEM_C_
#define _SYSTEM_C_
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/*
    Low-level system functions.
*/
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#include "../Lib/STDLib.H"
#include "../Lib/Math.H"

#include "System.H"
#include "Video.H"
#include "Timer.H"

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

int CMOSRead(byte);
bool CMOSBusy(void);

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

int CMOSRead(byte iAddress)
{
	while( CMOSBusy() );
	outportb(CMOS_OUT_PORT, 0x80|iAddress ); // Use 0x80 because we need to disable NMI
	return BCD2BIN(inportb(CMOS_IN_PORT));
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool CMOSBusy(void)
{
	outportb(CMOS_OUT_PORT, 0x80|CMOS_STATUS_A );

	if( inportb(CMOS_IN_PORT) & 0x80 ) // Bit 7 is set in Status Register A if Clock is busy
    {
		return true;
    }
	else return false;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

byte CMOSReadReg(byte reg)
{
    outportb(0x70, reg);
    return inportb (0x71);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void CMOSWriteReg(byte reg, byte value)
{
    outportb(0x70, reg);
    outportb(0x71, value);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void CMOSSleep(int iSeconds)
{
    byte iValue = 0;
    while(iSeconds--)
    {
        iValue = CMOSReadReg(0);         // read seconds from RTC
        while(CMOSReadReg(0) == iValue);  // wait for the next value
    }
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/*
    Receives a byte (or word or dword) from an I/O location.
*/
/*
unsigned char inportb(ushort _port)
{
    unsigned char rv;
    __asm__ __volatile__ ("inb %1, %0" : "=a" (rv) : "dN" (_port));
    return rv;
}
*/

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/*
    sends a byte (or word or dword) on a I/O location. Traditionnal names
    are outb, outw and outl respectively. the "a" modifier enforce value to
    be placed in eax register before the asm command is issued and "Nd" allows
    for one-byte constant values to be assembled as constants and use edx
    register for other cases.
*/


/*
void outportb(ushort _port, unsigned char _data)
{
    __asm__ __volatile__ ("outb %1, %0" : : "dN" (_port), "a" (_data));
}
*/

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void Hang(void)
{
    __asm__ __volatile__ ("hlt");

    for(;;)
    {
        __asm__ __volatile__ ("nop");
    }
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void StartInterrupts(void)
{
    __asm__ __volatile__ ("sti");
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void StopInterrupts(void)
{
    __asm__ __volatile__ ("cli");
}


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void Reboot()
{
    int temp = 0;

    // Get ready for reboot...flush the keyboard controller
    do
    {
        if((temp = inportb(0x64)) & 1)
        {
            inportb( 0x60);
        }
    }
	while(temp &2);

    // Reboot the computer...
    outportb(0x64, 0xFE);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//Memory access
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/*
    FAR_PEEKx
    read a byte (or word or dword) on a given memory location using another segment than
        the default C data segment. There's unfortunately no constraint for manipulating
        directly segment registers, so issuing the 'mov <reg>,<segmentreg>' manually is required.
*/

dword farpeekl(word sel, void *off)
{
    dword ret;
    __asm__ __volatile__ ("push %%fs;mov %1,%%fs;"
        "mov %%fs:(%2),%0;"
        "pop %%fs":"=r"(ret):"g"(sel),"r"(off));
    return ret;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/*
    FAR_POKEx
    write a byte (or word or dword) to a segment:offset address too.
    Note that much like in farpeek, this version of farpoke saves and
    restore the segment register used for the access.
*/

void farpokeb(word sel, void *off, byte v)
{
    __asm__ __volatile__("push %%fs; mov %0,%%fs;"
        "movb %2,%%fs:(%1);"
        "pop %%fs": :"g"(sel),"r"(off),"r"(v));
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//I/O access
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/*
    IO_WAIT
    Forces the CPU to wait for an I/O operation to complete. only use
    this when there's nothing like a status register or an IRQ to tell
    you the info has been received.
*/
void io_wait(void)
{
    __asm__ __volatile__("jmp 1f;1:jmp 1f;1:");
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/*
    alternatively, you may use another I/O cycle on an 'unused' port
    (which has the nice property of being CPU-speed independent):
*/

/*
void io_wait(void)
{
    __asm__ __volatile__("outb %%al, $0x80" : : "a"(0));
   // port 0x80 is used for 'checkpoints' during POST.
   // linux kernel seems to think it's free for use :-/
}
*/

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//Interrupt-related functions
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/*
    Enabled?
    returns a 'true' boolean value if irq are enabled at the CPU
*/

int irqEnabled(void)
{
    int f;
    __asm__ __volatile__ ("pushf;popl %0":"=g" (f));
    return f & (1<<9);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/*
    acknowledge
    sends the PIC chip (8259a) that we're ready for more interrupts.
    as pics are cascaded for int 8-15, if no>=8, we will have to send a
    'clearance code' for both master and slave PICs.
*/
void irqUnlock(int no)
{
    //Val, Port
    if (no>7) outportb(0x20,0xa0);
    outportb(0x20,0x20);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/*
    LIDT
    define a new interrupt table
*/
/*
void lidt(void *base, uint size)
{
    uint i[2];
    
    i[0] = size << 16;
    i[1] = (uint) base;
    __asm__ __volatile__ ("lidt (%0)": :"p" (((char *) i)+2));
}
*/

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/*
    the 'char* +2' trick avoids you to wonder whether the structure
    packing/padding will work or not for that weird 6-bytes IDTR stuff.
    
    Alternatively, you can take the more straightforward approach
*/
/*
void lidt(void *base, ushort size)
{
    struct { ushort length; uint base; } __attribute__((__packed__)) IDTR;

    IDTR.length = size;
    IDTR.base = (uint) base;
    __asm__ __volatile__ ("lidt (%0)": :"p" (&IDTR));
}
*/

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//Cpu-related functions
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/*
    CPUID
    request for CPU identification. See CPUID for more information. 
    
    issue a single request to CPUID. Fits 'intel features', for instance
*/
void cpuid(int code, dword *a, dword *d)
{
    __asm__ __volatile__("cpuid":"=a"(*a),"=d"(*d):"0"(code));
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/*
    RDTSC
    read the current value of the CPU's time-stamp counter and store into EDX:EAX.
    The time-stamp counter contains the amount of clock ticks that have elapsed since
    the last CPU reset. The value is stored in a 64-bit MSR and it increments after
    each clock cycle.
    
    this can be used to find out how much time it takes to do certain functions.
    It's very useful for testing/benchmarking/etc. Note: This is only an approximation.
*/

void rdtsc(dword *upper, dword *lower)
{
    __asm__ __volatile__("rdtsc\n" : "=a"(*lower), "=d"(*upper));
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/*
    READ_CRx
    read the value in a control register
*/
/*
unsigned read_cr0(void)
{
    unsigned val;
    __asm__ __volatile__("mov %%cr0, %0":"=r"(val));
    return val;
}
*/

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/*
    PGFLUSHTLB
    invalidates the TLB (Translation Lookaside Buffer) for one specific virtual
    address (next memory reference for the page will be forced to re-read PDE
    and PTE from main memory. Must be issued every time you update one of those
    tables). m points to a logical address, not a physical or virtual one: an
    offset for your ds segment. Note *m is used, not just m: if you use m here,
    you invalidate the address of the m variable (not what you want!).
*/

void pgFlushOneTlb(void *m)
{
    __asm__ __volatile__("invlpg %0"::"m" ((uint *)m));
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool InProtectedMode(void)
{
    return ((read_msw() & 1) > 0);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/*
    Stop interupts befor calling this function!
*/
void SwitchToProtectedMode(void)
{
    write_cr0 (read_cr0() | 1); //This switches us to PMode just setting up CR0.PM bit to 1
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/*
    Stop interupts befor calling this function!
*/
void SwitchToRealMode(void)
{
    write_cr0 (read_cr0() & 0xFFFFFFFEL); //Get out of PMode clearing CR0.PM bit to 0
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void BeginInitText(INITSTATTEXT *pIST, const char *sText)
{
    SetTextColor(WHITE, BLACK);

    printf("Initializing [%s]: ", sText);

    pIST->ibX = 0;
    pIST->ibY = 0;
    pIST->iaX = 0;
    pIST->iaY = 0;

    GetXY(&pIST->ibX, &pIST->ibY); //Save the X/Y coords before we call the passed in Init() proc.
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void CriticalInitEx(const char *sFile, const char *sProc, uint ulLine, const char *sText, bool bResult)
{
	if(!bResult)
	{
		BSODEx(sFile, sProc, ulLine, sText);
		Hang();
	}
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool EndInitText(INITSTATTEXT *pIST, bool bResult)
{
	return EndInitText(pIST, bResult, false);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool EndInitText(INITSTATTEXT *pIST, bool bResult, bool bCritical)
{
    GetXY(&pIST->iaX, &pIST->iaY); //Save the X/Y coords after we call the passed in Init() proc.

    //If the coords changed, set them back so we can write the OK/FAIL.
    if(pIST->ibX != pIST->iaX || pIST->ibY != pIST->iaY)
    {
        GotoXY(pIST->ibX, pIST->ibY);
    }

    if(bResult)
    {
        SetTextColor(GREEN, BLACK);
        printf("OK\n");
    }
    else{
        SetTextColor(RED, BLACK);
        printf("Fail\n");
    }

    //If the coords changed, set them back to where
    //  they were after the call to the passed in Init() proc.
    if(pIST->ibX != pIST->iaX || pIST->ibY != pIST->iaY)
    {
        GotoXY(pIST->iaX, pIST->iaY);
    }

    SetTextColor(WHITE, BLACK);

	if(bCritical && !bResult)
	{
		Hang();
	}

    return bResult;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool VisualSpin(int iCount)
{
    const char sChars[5] = {'/', '-', '\\', '|', '\0'};

    ushort iChar = 0;

    PutCh(' ');

    for(int iDot = 0; iDot < iCount; iDot++)
    {
        if(!sChars[iChar])
        {
            iChar = 0;
        }
        PutCh(sChars[iChar++]);
        PutCh('\b');

        Sleep(1);
    }
    return true;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void BSODEx(const char *sFile, const char *sProc, uint ulLine, const char *sText)
{
    StopInterrupts();

	printf("\n");
	SetTextColor(WHITE, BLUE);
	//Cls();
	HideCursor();

	printf(" A critical kernel level error has occurred. The system has been halted.\n");
    DrawHRule();

	printf("\n\tFile: %s\n\tProc: %s\n\tLine: %u -> %s\n",
		sFile, sProc, ulLine, sText);

	Hang();
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#endif

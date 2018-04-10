////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#ifndef _SYSTEM_H_
#define _SYSTEM_H_
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/*
    Global function declarations and type definitions
*/
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#include "../Lib/Types.h"

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#define CMOS_OUT_PORT   0x70
#define CMOS_IN_PORT    0x71

#define CMOS_STATUS_A   0x0A
#define CMOS_STATUS_B   0x0B
#define CMOS_STATUS_C   0x0C
#define CMOS_STATUS_D   0x0D

#define  readb(addr)   (*(volatile unsigned char *) (addr)) 
#define  readl(addr)   (*(volatile uint *) (addr)) 
#define  readw(addr)   (*(volatile ushort *) (addr)) 
#define  writeb(b, addr)   (*(volatile unsigned char *) (addr) = (b)) 
#define  writel(b, addr)   (*(volatile uint *) (addr) = (b)) 
#define  writew(b, addr)   (*(volatile ushort *) (addr) = (b)) 

#define outportb(port, value) __asm__ ("outb %%al,%%dx"  : : "a" (value), "d" (port))
#define outportl(port, value) __asm__ ("outl %%eax,%%dx" : : "a" (value), "d" (port))
#define outportw(port, value) __asm__ ("outw %%ax,%%dx"  : : "a" (value), "d" (port))

#define inportb(port)({ unsigned char _v; __asm__ volatile ("inb %%dx,%%al" : "=a" (_v) : "d" (port)); _v;})
#define inportl(port)({uint _v; __asm__ volatile ("inl %%dx,%%eax":"=a" (_v) : "d" (port)); _v;})
#define inportw(port)({ushort _v; __asm__ volatile ("inw %%dx,%%ax":"=a" (_v) : "d" (port)); _v;})
#define Panic(x) printf("\nFile: %s\nProc: %s\nLine: %d -> %s\n", __FILE__, __FUNCTION__, __LINE__, x); Hang();
#define BSOD(x) BSODEx(__FILE__, __FUNCTION__, __LINE__, x); Hang();
#define CriticalInit(s, b) CriticalInitEx(__FILE__, __FUNCTION__, __LINE__, s, b);

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

/* This defines what the stack looks like after an ISR was running */
struct regs
{
    uint gs, fs, es, ds;
    uint edi, esi, ebp, esp, ebx, edx, ecx, eax;
    uint int_no, err_code;
    uint eip, cs, eflags, useresp, ss;    
};

extern "C" uint read_msw(void); //Protected mode functionality.
extern "C" uint read_cr0(void); //Protected mode functionality.
extern "C" void write_cr0 (uint value); //Protected mode functionality.

bool InProtectedMode(void); //Protected mode functionality.
void SwitchToProtectedMode(void); //Protected mode functionality.
void SwitchToRealMode(void); //Protected mode functionality.

//extern unsigned char inportb (ushort _port);
//extern void outportb (ushort _port, unsigned char _data);

bool VisualSpin(int iCount);

void Hang(void);

int CMOSRead(byte iAddress);

void CMOSWriteReg(byte reg, byte value);
byte CMOSReadReg(byte reg);
bool CMOSBusy(void);

dword farpeekl(word sel,void *off);
void farpokeb(word sel, void *off, byte v);
void io_wait(void);
int irqEnabled(void);
void irqUnlock(int no);
void CMOSSleep(int iSeconds);
//void lidt(void *base, uint size);
//void lidt(void *base, ushort size);
void cpuid(int code, dword *a, dword *d);
void rdtsc(dword *upper, dword *lower);
void pgFlushOneTlb(void *m);

void Reboot(void);

void StopInterrupts(void);
void StartInterrupts(void);

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

typedef struct _tag_Initialize_Status_Text {
    ushort ibX;
    ushort ibY;
    ushort iaX;
    ushort iaY;
} INITSTATTEXT, *LPINITSTATTEXT;

void BSODEx(const char *sFile, const char *sProc, uint ulLine, const char *sText);
bool EndInitText(INITSTATTEXT *pIST, bool bResult);
bool EndInitText(INITSTATTEXT *pIST, bool bResult, bool bCritical);
void BeginInitText(INITSTATTEXT *pIST, const char *sText);
void CriticalInitEx(const char *sFile, const char *sProc, uint ulLine, const char *sText, bool bResult);

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#endif

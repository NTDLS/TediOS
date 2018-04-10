////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#ifndef _CMEMORY_H_
#define _CMEMORY_H_
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/*
    Memory mapping & handing routines.
*/
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#include "../Lib/Types.h"

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//BOKS Memory Manager Defines:
#define MAX_ALLOCATION_BLOCKS		10000	//The total number of blocks that can be reserved (need to increse after debug).

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//Generic Memory Manager Defines:

#define ONEKILOBYTE                 0x400ul
#define TENMEGABYTES                0xA00000ul
#define ONEMEGABYTE                 0x100000ul
#define MAX_MEMORY                  0xfffffffful
#define TWO_MEGABYTE_BARRIER        0x200000ul
#define BIOS_AND_VIDEO_MEMORY_START 0x0a0000ul //Position of the BIOS and Video memory in physical memory on a PC
#define BIOS_AND_VIDEO_MEMORY_END   0x100000ul //Position of the BIOS and Video memory in physical memory on a PC

// Number of bits necessary to encode the size of a physical memory page.
//   This single macro defines the granularity of the physical memory manager.
#define PAGE_BIT_SHIFT 12

//Useful macros when dealing with physical memory pages.
#define PAGE_SIZE ( 1 << PAGE_BIT_SHIFT)
#define PAGE_MASK (~0 << PAGE_BIT_SHIFT)
#define PAGE_ALIGN_INF(addr) ((unsigned) (addr) & PAGE_MASK)
#define PAGE_ALIGN_SUP(addr) (((unsigned) (addr) + (PAGE_SIZE - 1)) & PAGE_MASK)

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#define BASELO 0x15
#define BASEHI 0x16
#define EXTLO  0x17
#define EXTHI  0x18

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

extern unsigned short _code; //Imported from the linker script.
extern unsigned short _bss;  //Imported from the linker script.
extern unsigned short _end;  //Imported from the linker script.

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

typedef struct _tag_MEMORYSTATUS{

    unsigned long ulBase;
    unsigned long ulExtended;
    unsigned long ulTotal;
    unsigned long ulShadow;
    unsigned long ulUnreserved;
    //unsigned long ulMaxPages;

} MEMORYSTATUS, *LPMEMORYSTATUS;

struct MEMBLOCK {
	byte *pAddress;
	MEMBLOCK *pNext;
	unsigned int iBytes;
	bool bUsed;
};

typedef struct _tag_Allocated_Blocks_of_Memory{
	MEMBLOCK *Blocks;
	unsigned int iBytes;		//The total number of bytes allocated to all combined blocks.
	unsigned int iBlocks;		//The count of *Blocks (slots) that have been created.
	unsigned int iBlocksInUse;	//The count of *Blocks (slots) that are allocated.
} MEMBLOCKS, *LPMEMBLOCKS;

extern MEMORYSTATUS gMS;

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void *malloc(unsigned int iBytes);
void *calloc(unsigned int iElementCount, unsigned int iElementSize);
bool free(void *pAddress);
void Mem_Debug_Allocation_Tests(void);

bool Mem_Initialize(void);
bool Mem_TestPhysical(void);
unsigned long Mem_SearchForUpperLimit(void);
unsigned long Mem_SearchForLowerLimit(void);
int Mem_ByteExch(int iPort);

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#endif

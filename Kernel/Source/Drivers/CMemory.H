////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#ifndef _CMEMORY_H_
#define _CMEMORY_H_
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/*
    Memory mapping & handing routines.
*/
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#include "../Lib/Types.h"
#include "../General/MultiBoot.H"

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#define malloc(uBytes) Mem->Alloc(uBytes)
#define calloc(uElementCount, uElementSize) Mem->AllocZ(uElementCount, uElementSize)
#define realloc(pAddress, uTotalBytes) Mem->ReAlloc(pAddress, uTotalBytes)
#define free(pMemory) Mem->Free(pMemory)

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#define ONEKILOBYTE                       1024ul
#define ONEMEGABYTE                    1048576ul
#define ONEGIGABYTE                 1073741824ul

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//BOKS Memory Manager Defines:
#define MEMORY_MANAGER_PAGE_SIZE		128ul    //Memory allocation granularity.

#define MEMORY_PAGE_ATTRIB_USED			0x0001ul //Pages that are allocated.
#define MEMORY_PAGE_ATTRIB_ENDOFCHAIN	0x0002ul //Pages that are the tail of a "chain" of pages.
#define MEMORY_PAGE_ATTRIB_MANAGER		0x0004ul //Pages that are used by the memory manager itsself.
#define MEMORY_PAGE_ATTRIB_HARDWARE		0x0010ul //Pages that are reserved by various peices of hardware.
#define MEMORY_PAGE_ATTRIB_POTECTED		0x0020ul
#define MEMORY_PAGE_ATTRIB_RESERVED_2	0x0040ul //Reserved
#define MEMORY_PAGE_ATTRIB_RESERVED_3	0x0080ul //Reserved
#define MEMORY_PAGE_ATTRIB_RESERVED_4	0x0100ul //Reserved

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//Generic Memory Manager Defines:

#define MAX_MEMORY                  4294967296ul // Max memory supported by the OS.
#define BASE_MEMORY_ADDRESS            2097152ul // Base of the manager memory (2 megabyte barrier).
#define SHADOW_RAM                      393216ul // 384k Shadow RAM.
#define BIOS_AND_VIDEO_MEMORY_START   0x0a0000ul //  (655,360) - BIOS & Video physical memory position on a PC.
#define BIOS_AND_VIDEO_MEMORY_END     0x100000ul //(1,048,576) - BIOS & Video physical memory position on a PC.

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#define BASELO 0x15
#define BASEHI 0x16
#define EXTLO  0x17
#define EXTHI  0x18

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

extern ushort _code; //Imported from the linker script.
extern ushort _bss;  //Imported from the linker script.
extern ushort _end;  //Imported from the linker script.

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

typedef struct _tag_Memory_Physical_Information{
    uint uBase;
    uint uExtended;
    uint uTotal;
    uint uShadow;
    uint uUnreserved;
} MEMORYPHYSICALINFO, *LPMEMORYPHYSICALINFO;

typedef struct _tag_Memory_Manager_Info {
	byte *uAttribs;		//Array of page attributes.
	uint uUsed;			//The number of pages that have been allocated (count of non-free pages).
	uint uCount;		//The total number of memory manager pages (([Total Mamory] - [Kernel Reserved]) / [Allocation Granularity]).
	uint uManagerPages;	//The number of pages reserved my the memory manager for its internal use.
	uint uSize;			//Number of bytes in each page.
	uint uBaseAddress;	//The base address for all allocations.
} MEMORYMANAGERPAGES, *LPMEMORYMANAGERPAGES;

typedef struct _tag_Memory_Status_Information {
	uint uBytesTotal;
	uint uBytesUsed;
	uint uBytesFree;
} MEMORYSTATUSINFO, *LPMEMORYSTATUSINFO;

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

class CMemory {
public:
	void *Alloc(uint uBytes);
	void *AllocZ(uint uElementCount, uint uElementSize);
	void *ReAlloc(void *pAddress, uint uTotalBytes);
	bool Free(void *pAddress);

	bool Initialize(multiboot_info_t *pMBI);
	bool TestPhysical(void);
	uint SearchForUpperLimit(void);
	uint SearchForLowerLimit(void);

	void GetStatusInfo(MEMORYSTATUSINFO *pMSI);
	void GetPhysicalInfo(MEMORYPHYSICALINFO *pMPI);
	void GetPageInfo(MEMORYMANAGERPAGES *pMMP);
	void *HardwareAlloc(uint uElementCount, uint uElementSize);

	MEMORYPHYSICALINFO MPI;
	MEMORYMANAGERPAGES Pages;

private:
	int PortByteExch(int iPort);
	void *ProtectRange(uint uStartAddress, uint uLength);

	void *Alloc(uint uBytes, uint uAttributes);
	void *AllocZ(uint uElementCount, uint uElementSize, uint uAttributes);
	void *ReAlloc(void *pAddress, uint uTotalBytes, uint uAttributes);

	byte *pBaseMemAddress;		//Base of system memory.
	byte *pKernelEndAddress;
};

extern CMemory gMem;
extern CMemory *Mem;

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#endif

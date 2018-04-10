////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#ifndef _CMEMORY_CPP_
#define _CMEMORY_CPP_
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/*
    Memory mapping & handing routines.
*/
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#include "../Lib/STDLib.H"
#include "../Lib/String.H"

#include "System.H"
#include "Video.H"
#include "CMemory.h"
#include "Timer.h"
#include "CKeyboard.H"

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

CMemory gMem;
CMemory *Mem = (CMemory *) &gMem;

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

int CMemory::PortByteExch(int iPort)
{
    outportb(0x70, iPort);
    return(inportb(0x71));
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void CMemory::GetStatusInfo(MEMORYSTATUSINFO *pMSI)
{
	pMSI->uBytesTotal = MPI.uTotal;
	pMSI->uBytesUsed = MPI.uBase + (Pages.uUsed * Pages.uSize);
	pMSI->uBytesFree = pMSI->uBytesTotal - pMSI->uBytesUsed;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void CMemory::GetPhysicalInfo(MEMORYPHYSICALINFO *pMPI)
{
	memcpy(pMPI, &MPI, sizeof(MEMORYPHYSICALINFO));
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void CMemory::GetPageInfo(MEMORYMANAGERPAGES *pMMP)
{
	memcpy(pMMP, &this->Pages, sizeof(MEMORYMANAGERPAGES));
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/*
	Initialize the memory manager.
*/
bool CMemory::Initialize(multiboot_info_t *pMBI)
{
	char sText[64];

	// Place the starting address of all usable "user mode" memory at the "2 megabyte barrier".
	pBaseMemAddress = (byte *) BASE_MEMORY_ADDRESS;
	pKernelEndAddress = (byte *) &_end;

	memset(&this->MPI, 0, sizeof(MEMORYPHYSICALINFO));

	if(pMBI && CHECK_FLAG (pMBI->flags, 6))
	{
		MPI.uBase = pMBI->mem_lower * 1024UL;
		MPI.uExtended = pMBI->mem_upper * 1024UL;
		MPI.uShadow = SHADOW_RAM; 
		MPI.uTotal = (MPI.uBase + MPI.uExtended + MPI.uShadow);
		MPI.uUnreserved = (MPI.uTotal - ((uint)pBaseMemAddress));
	}
	else{
		MPI.uBase = (this->PortByteExch(BASELO) + (this->PortByteExch(BASEHI) << 8)) * 1024UL; //the memory up to 640 kB
		MPI.uExtended = (this->PortByteExch(EXTLO) + (this->PortByteExch(EXTHI) << 8)) * 1024UL;
		MPI.uShadow = SHADOW_RAM; 
		MPI.uTotal = (MPI.uBase + MPI.uExtended + MPI.uShadow);
		MPI.uUnreserved = (MPI.uTotal - ((uint)pBaseMemAddress));

		if((MPI.uTotal / ONEMEGABYTE) < 3)
		{
			return false;
		}

		if(MPI.uTotal >= 16777216UL)
		{
			MPI.uTotal = this->SearchForUpperLimit();
			MPI.uExtended = MPI.uTotal - (MPI.uBase + MPI.uShadow);
			MPI.uUnreserved = (MPI.uTotal - ((uint)pBaseMemAddress));
		}
		else {
			INITSTATTEXT IST;
			BeginInitText(&IST, "Testing RAM");
			if(!EndInitText(&IST, this->TestPhysical()))
			{
				return false;
			}
		}
	}

	printf("     (%s)\n", FriendlySize(MPI.uTotal, sText, sizeof(sText), 0));

	//Initialize the global memory manager structure.
	memset(&this->Pages, 0, sizeof(MEMORYMANAGERPAGES));
	Pages.uSize = MEMORY_MANAGER_PAGE_SIZE;
	Pages.uCount = MPI.uUnreserved / Pages.uSize;
	Pages.uBaseAddress = (uint)pBaseMemAddress;
	Pages.uAttribs = (byte *)Pages.uBaseAddress;
	Pages.uManagerPages = ((sizeof(byte) * Pages.uCount) / Pages.uSize);
	if(((sizeof(byte) * Pages.uCount) % Pages.uSize) > 0)
	{
		Pages.uManagerPages++;
	}

	//Initialize all of the page attribs to zero.
	memset(Pages.uAttribs, 0, sizeof(byte) * Pages.uCount);

	//Reserve memory for internal use by the manager.
	for(uint uPage = 0; uPage < Pages.uManagerPages; uPage++)
	{
		Pages.uAttribs[uPage] = MEMORY_PAGE_ATTRIB_USED | MEMORY_PAGE_ATTRIB_MANAGER;
		Pages.uUsed++;
	}
	//Mark the last block in a chain of pages as the "End of Chain".
	Pages.uAttribs[Pages.uManagerPages - 1] |= MEMORY_PAGE_ATTRIB_ENDOFCHAIN;

	if(pMBI && CHECK_FLAG (pMBI->flags, 6))
	{
		memory_map_t *mmap;

		for (mmap = (memory_map_t *) pMBI->mmap_addr;
            (unsigned long) mmap < pMBI->mmap_addr + pMBI->mmap_length;
            mmap = (memory_map_t *) ((unsigned long) mmap + mmap->size + sizeof (mmap->size)))
		{
			if(mmap->base_addr_high || mmap->length_high)
			{
				BSOD("Unsupported amount of RAM. MAX 4GB.");
				return false;
			}

			if(mmap->type != 1)
			{
				ProtectRange(mmap->base_addr_low, mmap->length_low);
			}
		}
	}

    return true;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void *CMemory::ProtectRange(uint uStartAddress, uint uLength)
{
	if(
		(uStartAddress >= Pages.uBaseAddress && uStartAddress <= MPI.uTotal)
		|| (uStartAddress + uLength >= Pages.uBaseAddress && uStartAddress + uLength <= MPI.uTotal))
	{
		BSOD("Memory protection is not implemented!");
	}

	return 0;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void *CMemory::HardwareAlloc(uint uElementCount, uint uElementSize)
{
	return this->AllocZ(uElementCount, uElementSize, MEMORY_PAGE_ATTRIB_HARDWARE);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/*
	Free allocated RAM.
*/
bool CMemory::Free(void *pAddress)
{
	uint uPage = (((uint)pAddress) - Pages.uBaseAddress) / Pages.uSize;

	//Perform a few basic checks and basic memory protection.
	if(uPage < 0)
	{
		BSOD("Attempt to free sub-base memory page!");
	}
	else if(!(Pages.uAttribs[uPage] &MEMORY_PAGE_ATTRIB_USED))
	{
		BSOD("Attempt to free unused memory page!");
	}
	else if((Pages.uAttribs[uPage] &MEMORY_PAGE_ATTRIB_MANAGER))
	{
		BSOD("Attempt to free memory manager page!");
	}
	else if((Pages.uAttribs[uPage] &MEMORY_PAGE_ATTRIB_HARDWARE))
	{
		BSOD("Attempt to free reserved hardware memory page!");
	}
	else if((Pages.uAttribs[uPage] &MEMORY_PAGE_ATTRIB_POTECTED))
	{
		BSOD("Attempt to free protected memory page!");
	}
	else if(uPage > 0
		&& (Pages.uAttribs[uPage - 1] &MEMORY_PAGE_ATTRIB_USED)
		&& !(Pages.uAttribs[uPage - 1] &MEMORY_PAGE_ATTRIB_ENDOFCHAIN))
	{
		BSOD("Attempt to free non-root memory page in page chain!");
	}

	//Free the allocated pages.
	for(; uPage < Pages.uCount; uPage++)
	{
		Pages.uUsed--;
		if(Pages.uAttribs[uPage] &MEMORY_PAGE_ATTRIB_ENDOFCHAIN)
		{
			Pages.uAttribs[uPage] = 0;
			return true;
		}
		Pages.uAttribs[uPage] = 0;
	}

	return false;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/*
	Re-Allocate RAM / Resize allocated block (Will relocate the pages if necessary).
*/
void *CMemory::ReAlloc(void *pAddress, uint uTotalBytes)
{
	return this->ReAlloc(pAddress, uTotalBytes, 0);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/*
	Re-Allocate RAM / Resize allocated block (Will relocate the pages if necessary).
*/
void *CMemory::ReAlloc(void *pAddress, uint uTotalBytes, uint uAttributes)
{
	if(!pAddress)
	{
		return this->Alloc(uTotalBytes, uAttributes);
	}

	uint uFirstPage = (((uint)pAddress) - Pages.uBaseAddress) / Pages.uSize;
	uint uLastPage = uFirstPage;

	//Perform a few basic checks and basic memory protection.
	if(uFirstPage < 0)
	{
		BSOD("Attempt to realloc sub-base memory page!");
	}
	else if(!(Pages.uAttribs[uFirstPage] &MEMORY_PAGE_ATTRIB_USED))
	{
		BSOD("Attempt to realloc unused memory page!");
	}
	else if((Pages.uAttribs[uFirstPage] &MEMORY_PAGE_ATTRIB_MANAGER))
	{
		BSOD("Attempt to realloc memory manager page!");
	}
	else if((Pages.uAttribs[uFirstPage] &MEMORY_PAGE_ATTRIB_HARDWARE))
	{
		BSOD("Attempt to realloc reserved hardware memory page!");
	}
	else if(uFirstPage > 0
		&& (Pages.uAttribs[uFirstPage - 1] &MEMORY_PAGE_ATTRIB_USED)
		&& !(Pages.uAttribs[uFirstPage - 1] &MEMORY_PAGE_ATTRIB_ENDOFCHAIN))
	{
		BSOD("Attempt to realloc non-root memory page in page chain!");
	}

	//Find the end of the chain of pages.
	for(uLastPage = uFirstPage; uLastPage < Pages.uCount; uLastPage++)
	{
		if(Pages.uAttribs[uLastPage] &MEMORY_PAGE_ATTRIB_ENDOFCHAIN)
		{
			break;
		}
	}

	//Figure out how many pages we need.
	uint uCurrentPages = (uLastPage - uFirstPage) + 1;
	uint uPagesRequired = (uTotalBytes / Pages.uSize) + ((uTotalBytes % Pages.uSize) > 0);

	//If we already have enough pages, just return the old address.
	if(uPagesRequired <= uCurrentPages)
	{
		return pAddress; //Nothing to do, currently allocated page count is sufficient.
	}

	//Check to see of we can add pages to our current block of pages.
	uint uAddPages = (uPagesRequired - uCurrentPages); //How many more pages do we need?
	uint uAddPagesFound = 0;
	for(uint uOffset = 1; uOffset <= uAddPages; uOffset++)
	{
		if(Pages.uAttribs[uLastPage + uOffset] &MEMORY_PAGE_ATTRIB_USED)
		{
			break; //Not enough contiguous pages after our currently allocated block.
		}

		uAddPagesFound++;
	}

	if(uAddPagesFound == uAddPages)
	{
		//We found contiguous pages after our currently allocated pages.

		//Unflag what used to be the last page.
		Pages.uAttribs[uLastPage] &= ~MEMORY_PAGE_ATTRIB_ENDOFCHAIN;

		//Reserve the additonal pages.
		for(uint uOffset = 1; uOffset <= uAddPages; uOffset++)
		{
			Pages.uAttribs[uLastPage + uOffset] = MEMORY_PAGE_ATTRIB_USED | uAttributes;
			Pages.uUsed++;
		}
		Pages.uAttribs[uLastPage + uAddPages] |= (MEMORY_PAGE_ATTRIB_ENDOFCHAIN | uAttributes);
	}
	else{
		//We did not find the required number of contiguous pages, we have to relocate the memory.

		byte *pNewAddress = (byte *)this->Alloc(uTotalBytes, uAttributes);
		if(pNewAddress)
		{
			//Move our data from the old pages to the new pages.
			memcpy(pNewAddress, pAddress, ((uLastPage - uFirstPage) + 1) * Pages.uSize);
			this->Free(pAddress); //Free the old pages.
			return pNewAddress; //Return the new address.
		}
		else{
			return NULL; //Failed to find a large enough block of pages. The old address is still valid.
		}
	}

	return pAddress;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/*
	Allocate RAM.
*/
void *CMemory::Alloc(uint uBytes)
{
	return this->Alloc(uBytes, 0);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/*
	Allocate RAM.
*/
void *CMemory::Alloc(uint uBytes, uint uAttributes)
{
	if(uBytes <= 0)
	{
		return NULL; //Requested an invalid number or bytes.
	}

	//Figure out how many pages we need.
	uint uPagesRequired = (uBytes / Pages.uSize) + ((uBytes % Pages.uSize) > 0);
	uint uContiguousPages = 0;
	
	//Search for contiguous pages.
	for(uint uPage = Pages.uManagerPages; uPage < Pages.uCount; uPage++)
	{
		if(!(Pages.uAttribs[uPage] &MEMORY_PAGE_ATTRIB_USED))
		{
			if(++uContiguousPages == uPagesRequired)
			{
				uint uStartingPage = ((uPage - uContiguousPages) + 1);

				//Reserve the memory pages. The last page in the block of is not chained to the next page.
				for(uContiguousPages = 0; uContiguousPages < uPagesRequired; uContiguousPages++)
				{
					this->Pages.uAttribs[uPage - uContiguousPages] = MEMORY_PAGE_ATTRIB_USED | uAttributes;
					Pages.uUsed++;
				}
				Pages.uAttribs[uPage] |= (MEMORY_PAGE_ATTRIB_ENDOFCHAIN | uAttributes);

				return ((byte *)Pages.uBaseAddress) + (uStartingPage * Pages.uSize);
			}
		}
		else{
			uContiguousPages = 0; //Found a used page, reset the contiguous count.
		}
	}

	return NULL; //Insufficient memory or contiguous pages.
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/*
	Allocate RAM and clear the reserved pages.
*/
void *CMemory::AllocZ(uint uElementCount, uint uElementSize)
{
	return this->AllocZ(uElementCount, uElementSize, 0);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/*
	Allocate RAM and clear the reserved pages.
*/
void *CMemory::AllocZ(uint uElementCount, uint uElementSize, uint uAttributes)
{
	uint uBytes = uElementCount * uElementSize;
	void *pMemory = this->Alloc(uBytes, uAttributes);
	if(pMemory)
	{
		memset(pMemory, 0, uBytes);
	}
	return pMemory;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/*
	Do not call after user space memory is being used
		This function will clear data stored in RAM.
*/
uint CMemory::SearchForUpperLimit(void)
{
	char sText[64];
    const char sStatChars[5] = {'/', '-', '\\', '|', '\0'};

    ushort iStatChar = 0;
	uint uStatTick = ONEMEGABYTE * 10;
    uint uStatCount = uStatTick;

    INITSTATTEXT IST;
    BeginInitText(&IST, "Upper Memory Limit");

    uint iAddr = 0;

    for(iAddr = 0; iAddr < MAX_MEMORY; iAddr++)
    {
        (*(pBaseMemAddress + iAddr)) = (unsigned char)iAddr;
        if((*(pBaseMemAddress + iAddr)) != (unsigned char)iAddr)
        {
            break;
        }

        (*(pBaseMemAddress + iAddr)) = 0; //Rest unused RAM to Zero!

        if(uStatCount++ == uStatTick)
        {
            if(!sStatChars[iStatChar])
            {
                iStatChar = 0;
            }
            PutCh(sStatChars[iStatChar++]);
            PutCh('\b');
            uStatCount = 0;
        }
    }

    iAddr += (uint)pBaseMemAddress;

    printf("     (%s)\n", FriendlySize(iAddr, sText, sizeof(sText), 2));

    EndInitText(&IST, true);

    return iAddr;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/*
	Do not call after user space memory is being used
		This function will clear data stored in RAM.
*/
bool CMemory::TestPhysical(void)
{
    const char sStatChars[5] = {'/', '-', '\\', '|', '\0'};

    ushort iStatChar = 0;
	uint uStatTick = ONEMEGABYTE * 10;
    uint uStatCount = uStatTick;

    bool bResut = true;

    PutCh('\n');

    INITSTATTEXT IST;

    BeginInitText(&IST, "RAM Write");

    //Fill and Test RAM.
    for(uint iAddr = 0; iAddr < MPI.uUnreserved; iAddr++)
    {
        (*(pBaseMemAddress + iAddr)) = (unsigned char)iAddr;
        if((*(pBaseMemAddress + iAddr)) != (unsigned char)iAddr)
        {
            printf("\nWrite fail at %d\n", (pBaseMemAddress + iAddr));
            bResut = false;
            break;
        }

        if(uStatCount++ == uStatTick)
        {
            if(!sStatChars[iStatChar])
            {
                iStatChar = 0;
            }
            PutCh(sStatChars[iStatChar++]);
            PutCh('\b');
            uStatCount = 0;
        }
    }

    EndInitText(&IST, bResut);

    if(bResut)
    {
        BeginInitText(&IST, "RAM Read");

        //Read and Test RAM.
        for(uint iAddr = 0; iAddr < MPI.uUnreserved; iAddr++)
        {
            if((*(pBaseMemAddress + iAddr)) != (unsigned char)iAddr)
            {
                printf("\nRead fail at %d\n", (pBaseMemAddress + iAddr));
                bResut = false;
                break;
            }

			//After debugging, we will want to init ram to zero - but for bebugging purposes,
			//	Lets make it easy to see overflow and set to to the AT sign.
            (*(pBaseMemAddress + iAddr)) = (unsigned char)'@'; //Initialize RAM.
    
            if(uStatCount++ == uStatTick)
            {
                if(!sStatChars[iStatChar])
                {
                    iStatChar = 0;
                }
                PutCh(sStatChars[iStatChar++]);
                PutCh('\b');
                uStatCount = 0;
                //Sleep(1);
            }
        }
    
        EndInitText(&IST, bResut);
    }

    return bResut;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#endif

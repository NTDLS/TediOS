////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#ifndef _CMEMORY_CPP_
#define _CMEMORY_CPP_
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/*
    Memory mapping & handing routines.
*/
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#include "../Lib/STDLib.H"

#include "System.H"
#include "Video.H"
#include "Memory.H"
#include "Timer.h"
#include "CKeyboard.H"

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

MEMORYSTATUS gMS;			
byte *pBaseMemAddress;		//Base of system memory.
byte *pKernelEndAddress;

MEMBLOCKS gBlocks;

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void Mem_Debug_Allocation_Tests(void)
{
	/*
	char **sBuffers = (char **)calloc(sizeof(char), 5000);
	int iAlloc = 25;

	for(int iIndex = 0; iIndex < 5000; iIndex++)
	{
		sBuffers[iIndex] = (char *)
			calloc((iAlloc * iIndex) + 1, sizeof(char));

		memset(sBuffers[iIndex], (unsigned char)(97 + iIndex), (iAlloc * iIndex));
		sBuffers[iIndex][0] = (unsigned char)(65 + iIndex);
		sBuffers[iIndex][(iAlloc * iIndex) - 1] = (unsigned char)(65 + iIndex);
		if((iIndex % 2) == 0)
		{
			free(sBuffers[iIndex]);
			sBuffers[iIndex] = NULL;
		}
		//printf("Req: %d, Wri: %d, Address:%d, Dump:\n", (iAlloc * iIndex), strlen(sBuffers[iIndex]), sBuffers[iIndex]);
		//DumpBuf(sBuffers[iIndex], (iAlloc * iIndex));
	}

	printf("bDone!\n");
	KBD->Pause();

	for(int iIndex = 0; iIndex < 5000; iIndex++)
	{
		if(sBuffers[iIndex])
		{
			free(sBuffers[iIndex]);
			//printf("Req: %d, Wri: %d, Address:%d, Dump:\n", (iAlloc * iIndex), strlen(sBuffers[iIndex]), sBuffers[iIndex]);
			//DumpBuf(sBuffers[iIndex], (iAlloc * iIndex));
		}
	}

	free(sBuffers);
	*/
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool Mem_Initialize(void)
{
    // Place the starting address of all usable "user mode" memory at the "2 megabyte barrier".
    pBaseMemAddress = (byte *) TWO_MEGABYTE_BARRIER;
    pKernelEndAddress = (byte *) &_end;

    gMS.ulBase = (Mem_ByteExch(BASELO) + (Mem_ByteExch(BASEHI) << 8)) * 1024UL; //the memory up to 640 kB
    gMS.ulExtended = (Mem_ByteExch(EXTLO) + (Mem_ByteExch(EXTHI) << 8)) * 1024UL;
    gMS.ulShadow = 393216; //384k Shadow RAM.
    gMS.ulTotal = (gMS.ulBase + gMS.ulExtended + gMS.ulShadow);
    gMS.ulUnreserved = (gMS.ulTotal - ((unsigned long)pBaseMemAddress));

    printf("     (%uMB)\n", gMS.ulTotal / ONEMEGABYTE);

	if((gMS.ulTotal / ONEMEGABYTE) < 3)
	{
		return false;
	}

	if(gMS.ulTotal >= 67108864UL)
	{
		gMS.ulTotal = Mem_SearchForUpperLimit();
		gMS.ulExtended = gMS.ulTotal - (gMS.ulBase + gMS.ulShadow);
	    gMS.ulUnreserved = (gMS.ulTotal - ((unsigned long)pBaseMemAddress));
	}
	else {
        INITSTATTEXT IST;
        BeginInitText(&IST, "Testing RAM");
        if(!EndInitText(&IST, Mem_TestPhysical()))
		{
			return false;
		}
    }

	//The memory manager starts on our bytes zero (base). Figure out how much RAM our Memory
	//	  Manager its-self is going to use, initialize it then reserve the first managed block.
	byte *pStartingAddress = pBaseMemAddress;
	int iBytesReqForMemManager = sizeof(MEMBLOCK) * MAX_ALLOCATION_BLOCKS;
	memset(pStartingAddress, 0, iBytesReqForMemManager); //Init the memory manager's RAM.

	//Set up the global MEMBLOCKS struct:
	memset(&gBlocks, 0, sizeof(MEMBLOCKS));
	gBlocks.Blocks = (MEMBLOCK *) pBaseMemAddress;
	gBlocks.iBytes = iBytesReqForMemManager;
	gBlocks.iBlocks = 1;
	gBlocks.iBlocksInUse = 1;

	//Reserve the first allocation block for the memory manager.
	gBlocks.Blocks[0].pAddress = pStartingAddress;
	gBlocks.Blocks[0].iBytes = gBlocks.iBytes;
	gBlocks.Blocks[0].pNext = (MEMBLOCK *)NULL;
	gBlocks.Blocks[0].bUsed = true;

    return true;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool free(void *pAddress)
{
	MEMBLOCK *pBlock = &gBlocks.Blocks[0];

	//printf("Search for allocated block with address %u.\n", pAddress);
	do {
		if(!(pBlock = pBlock->pNext))
		{
			break;
		}

		if(pBlock->bUsed && pBlock->pAddress == (byte *)pAddress)
		{
			//printf("Found address, freeing %u bytes...\n", pBlock->iBytes);

			gBlocks.iBytes -= pBlock->iBytes;
			gBlocks.iBlocksInUse--; //We freed a block.

			pBlock->bUsed = false;

			printf("Free-----> Blocks: %u, Alloc: %u, Bytes: %u\n",
				gBlocks.iBlocks, gBlocks.iBlocksInUse, gBlocks.iBytes + pBaseMemAddress);

			return true;
		}

	} while(pBlock->pNext);

	Panic("Attempted to free unallocated address.");

	return false;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void *malloc(unsigned int iBytes)
{
	MEMBLOCK *pBlock = &gBlocks.Blocks[0];

	//Look for previously freed blocks, and reclaim them if possible.
	do {
		if(pBlock->pNext)
		{
			pBlock = pBlock->pNext;

			if(!pBlock->bUsed)
			{
				if(pBlock->iBytes >= iBytes) //FIXFIX: If the user needs less than we found, we will WASTE RAM.
				{
					pBlock->bUsed = true;
					gBlocks.iBytes += pBlock->iBytes;
					gBlocks.iBlocksInUse++;

					printf("Reclaim--> Blocks: %u, Alloc: %u, Bytes: %u, MM-Ram: %u\n",
						gBlocks.iBlocks, gBlocks.iBlocksInUse, gBlocks.iBytes + pBaseMemAddress,
						((byte *)pBlock->pNext) - pBaseMemAddress);

					return pBlock->pAddress;
				}
			}
		}
	} while(pBlock->pNext);

	//Didn't find any suitable blocks to reclaim - allocate a new block.

	pBlock->pNext = &gBlocks.Blocks[gBlocks.iBlocks];
	pBlock->pNext->pAddress = (byte *)(pBlock->pAddress + pBlock->iBytes);
	pBlock->pNext->iBytes   = iBytes;
	pBlock->pNext->pNext    = (MEMBLOCK *)NULL;
	pBlock->pNext->bUsed    = true;

	gBlocks.iBytes += iBytes;
	gBlocks.iBlocks++;		//We created a new block.
	gBlocks.iBlocksInUse++; //We allocated a block.

	printf("Allocate-> Blocks: %u, Alloc: %u, Bytes: %u, MM-Ram: %u\n",
		gBlocks.iBlocks, gBlocks.iBlocksInUse, gBlocks.iBytes + pBaseMemAddress,
		((byte *)pBlock->pNext) - pBaseMemAddress);

	return pBlock->pNext->pAddress;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void *calloc(unsigned int iElementCount, unsigned int iElementSize)
{
	unsigned int iBytes = iElementCount * iElementSize;
	void *pMemory = malloc(iBytes);
	if(pMemory)
	{
		memset(pMemory, 0, iBytes);
	}
	return pMemory;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

unsigned long Mem_SearchForUpperLimit(void)
{
    const char sStatChars[5] = {'/', '-', '\\', '|', '\0'};

    unsigned short iStatChar = 0;
    unsigned long ulStatCount = TENMEGABYTES;

    INITSTATTEXT IST;
    BeginInitText(&IST, "Upper Memory Limit");

    unsigned long iAddr = 0;

    for(iAddr = 0; iAddr < MAX_MEMORY; iAddr++)
    {
        (*(pBaseMemAddress + iAddr)) = (unsigned char)iAddr;
        if((*(pBaseMemAddress + iAddr)) != (unsigned char)iAddr)
        {
            break;
        }

        (*(pBaseMemAddress + iAddr)) = 0; //Rest unused RAM to Zero!

        if(ulStatCount++ == TENMEGABYTES)
        {
            if(!sStatChars[iStatChar])
            {
                iStatChar = 0;
            }
            PutCh(sStatChars[iStatChar++]);
            PutCh('\b');
            ulStatCount = 0;
        }
    }

    iAddr += (unsigned long)pBaseMemAddress;

    printf("     (%uMB)\n", iAddr / ONEMEGABYTE);


    EndInitText(&IST, true);

    return iAddr;
}


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

unsigned long Mem_SearchForLowerLimit(void)
{
    INITSTATTEXT IST;
    BeginInitText(&IST, "Lower Memory Limit");

    byte *pAddress = pBaseMemAddress;

    while(pAddress > 0)
    {
        if((*pAddress) != 0)
        {
            break;
        }
        pAddress--;
    }

    EndInitText(&IST, true);

    return (unsigned long)pAddress;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool Mem_TestPhysical(void)
{
    const char sStatChars[5] = {'/', '-', '\\', '|', '\0'};

    unsigned short iStatChar = 0;
    unsigned long ulStatCount = TENMEGABYTES;

    bool bResult = true;

    PutCh('\n');

    INITSTATTEXT IST;

    BeginInitText(&IST, "RAM Write");

    //Fill and Test RAM.
    for(unsigned long iAddr = 0; iAddr < gMS.ulUnreserved; iAddr++)
    {
        (*(pBaseMemAddress + iAddr)) = (unsigned char)iAddr;
        if((*(pBaseMemAddress + iAddr)) != (unsigned char)iAddr)
        {
            printf("\nWrite fail at %d\n", (pBaseMemAddress + iAddr));
            bResult = false;
            break;
        }

        if(ulStatCount++ == TENMEGABYTES)
        {
            if(!sStatChars[iStatChar])
            {
                iStatChar = 0;
            }
            PutCh(sStatChars[iStatChar++]);
            PutCh('\b');
            ulStatCount = 0;
            //Sleep(1);
        }
    }

    EndInitText(&IST, bResult);

    if(bResult)
    {
        BeginInitText(&IST, "RAM Read");

        //Read and Test RAM.
        for(unsigned long iAddr = 0; iAddr < gMS.ulUnreserved; iAddr++)
        {
            if((*(pBaseMemAddress + iAddr)) != (unsigned char)iAddr)
            {
                printf("\nRead fail at %d\n", (pBaseMemAddress + iAddr));
                bResult = false;
                break;
            }

			//After debugging, we will want to init ram to zero - but for bebugging purposes,
			//	Lets make it easy to see overflow and set to to the AT sign.
            (*(pBaseMemAddress + iAddr)) = (unsigned char)'@'; //Initialize RAM.
    
            if(ulStatCount++ == TENMEGABYTES)
            {
                if(!sStatChars[iStatChar])
                {
                    iStatChar = 0;
                }
                PutCh(sStatChars[iStatChar++]);
                PutCh('\b');
                ulStatCount = 0;
                //Sleep(1);
            }
        }
    
        EndInitText(&IST, bResult);
    }

    return bResult;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

int Mem_ByteExch(int iPort)
{
    outportb(0x70, iPort);
    return(inportb(0x71));
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#endif

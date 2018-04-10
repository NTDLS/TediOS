///////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#include "../Lib/STDLib.H"
#include "../Lib/String.H"
#include "../Lib/Float.H"

#include "CFS.H"
#include "CATA.H"
#include "System.H"
#include "Timer.h"

#include "CMemory.h"
#include "Video.h"
#include "CKeyboard.H"

CFS *FS = NULL;

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool CFS::Initialize(void)
{
	ATAATTACHEDINFO ADI;

	ATA->GetADI(&ADI);

	if(ADI.uDisks > 0)
	{
		for(ushort uChannel = 0; uChannel < ADI.uChannels; uChannel++)
		{
			for(ushort uDisk = 0; uDisk < ADI.Channel[uChannel].uMaxDisks; uDisk++)
			{
				if(ADI.Channel[uChannel].Disk[uDisk].bPresent)
				{
					if(!this->Initialize(uChannel, uDisk))
					{
						ATA->DestroyADI(&ADI);
						return false;
					}
				}
			}
		}
	}

	ATA->DestroyADI(&ADI);

	return true;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool CFS::Initialize(word Channel, uchar Drive)
{
	/*
	unsigned char *sSectorBuffer = (unsigned char *) calloc(sizeof(unsigned char), SECTOR_SIZE);

	if(!ATA->ReadSectors(Channel, Drive, 0, 1, sSectorBuffer))
	{
		BSOD("ATA->ReadSectors failed.");
		free(sSectorBuffer);
		return false;
	}

	struct partrecord PartInfo = *((struct partrecord *) ((struct partsector *) sSectorBuffer)->psPart);

	ATA->ReadSectors(Channel, Drive, PartInfo.prStartLBA, 1, sSectorBuffer);
	struct bpb710 *bpb = (struct bpb710 *) ((struct bootsector710 *) sSectorBuffer)->bsBPB;

	// setup global disk constants
	unsigned long FirstDataSector = PartInfo.prStartLBA;
	if(bpb->bpbFATsecs)
	{
		// bpbFATsecs is non-zero and is therefore valid
		FirstDataSector += bpb->bpbResSectors + bpb->bpbFATs * bpb->bpbFATsecs;
	}
	else {
		// bpbFATsecs is zero, real value is in bpbBigFATsecs
		FirstDataSector += bpb->bpbResSectors + bpb->bpbFATs * bpb->bpbBigFATsecs;
	}
	unsigned int SectorsPerCluster	= bpb->bpbSecPerClust;
	unsigned int BytesPerSector		= bpb->bpbBytesPerSec;
	unsigned long FirstFATSector	= bpb->bpbResSectors + PartInfo.prStartLBA;
	unsigned long FirstDirSector	= 0;
	bool Fat32Enabled				= 0;

	switch (PartInfo.prPartType)
	{
		case PART_TYPE_DOSFAT16:
		case PART_TYPE_FAT16:
		case PART_TYPE_FAT16LBA:
			// first directory cluster is 2 by default (clusters range 2->big)
			FirstDirSector  = CLUST_FIRST;
			// push data sector pointer to end of root directory area
			//FirstDataSector += (bpb->bpbRootDirEnts)/DIRENTRIES_PER_SECTOR;
			Fat32Enabled = false;
			break;
		case PART_TYPE_FAT32LBA:
		case PART_TYPE_FAT32:
			// bpbRootClust field exists in FAT32 bpb710, but not in lesser bpb's
			FirstDirSector = bpb->bpbRootClust;
			// push data sector pointer to end of root directory area
			// need this? FirstDataSector += (bpb->bpbRootDirEnts)/DIRENTRIES_PER_SECTOR;
			Fat32Enabled = true;
			break;
		default:
			//No/Unsupported Partition.
		break;
	}
	*/

	/*
	printf("\n\tChannel %d, Disk %d: ", Channel, Drive);

	switch (PartInfo.prPartType)
	{
		case PART_TYPE_DOSFAT16:
			printf("DOSFAT 16");
			break;
		case PART_TYPE_FAT16:
			printf("FAT16");
			break;
		case PART_TYPE_FAT16LBA:
			printf("FAT16 LBA");
			break;
		case PART_TYPE_FAT32LBA:
			printf("FAT32 LBA");
			break;
		case PART_TYPE_FAT32:
			printf("FAT32");
			break;
		default:
			printf("Unsupported Partition");
			break;
	}
	*/
 
	/*
	printf("First sector    : %u\n", PartInfo.prStartLBA);
	printf("Size            : %u\n", PartInfo.prSize);
	printf("bytes/sector    : %u\n", bpb->bpbBytesPerSec);
	printf("sectors/cluster : %u\n", bpb->bpbSecPerClust);
	printf("reserved sectors: %u\n", bpb->bpbResSectors);
	printf("FatSectors      : %u\n", bpb->bpbFATsecs);
	printf("BigFatSectors   : %u\n", bpb->bpbBigFATsecs);
	printf("Number of Fats  : %u\n", bpb->bpbFATs);
	printf("First Fat Sector: %u\n", FirstFATSector);
	printf("First Data Sect : %u\n", FirstDataSector);
	printf("First Dir Clust : %u\n", FirstDirSector);
	*/

	//free(sSectorBuffer);

	return true;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//Simple File System
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#include "../Lib/STDLib.H"
#include "../Lib/String.H"
#include "../Lib/Float.H"

#include "CATA.H"
#include "CSFS.H"
#include "System.H"
#include "Timer.h"
#include "Video.h"
#include "CMemory.h"
#include "CKeyboard.H"

#pragma option -a-

CSFS *SFS = NULL;

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool CSFS::Initialize(void)
{
	ATAATTACHEDINFO ADI;
	ATA->GetADI(&ADI);

	unsigned char cIdentifier = 67; //C

	memset(&this->Disks, 0, sizeof(this->Disks));

	char sDisk[5];
	char sDisks[100];
	memset(sDisks, 0, sizeof(sDisks));

	uint uValidDisks = 0;

	for(ushort uChannel = 0; uChannel < ADI.uChannels; uChannel++)
	{
		if(ADI.Channel[uChannel].bPresent)
		{
			for(ushort uDisk = 0; uDisk < ADI.Channel[uChannel].uMaxDisks; uDisk++)
			{
				if(ADI.Channel[uChannel].Disk[uDisk].bPresent)
				{
					this->Disks.Disk = (SFSENUMDISK *) realloc(this->Disks.Disk, sizeof(SFSENUMDISK) * (this->Disks.uCount + 1));

					SFSENUMDISK *pDisk = &this->Disks.Disk[this->Disks.uCount];

					memset(pDisk, 0, sizeof(SFSENUMDISK));

					pDisk->uChannel = uChannel;
					pDisk->uDisk = uDisk;
					pDisk->pATADisk = ATA->Disk(uChannel, uDisk);
					ATA->ReadSectors(uChannel, uDisk, 0, 1, &pDisk->SB);

					if(pDisk->SB.uMagicNumber == SB_MAGICNUMBER)
					{
						pDisk->cIdentifier = cIdentifier++;

						sprintf(sDisk, "%c: ", pDisk->cIdentifier);
						strcat(sDisks, sDisk);
						uValidDisks++;

						if(!this->Disks.pCurrent)
						{
							this->Disks.pCurrent = pDisk;
						}
					}
					this->Disks.uCount++;
				}
			}
		}
	}

	if(uValidDisks > 0)
	{
		Trim(sDisks);
		printf("        (%s)\n", sDisks);
	}

	return true;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void CSFS::Destroy(void)
{
	if(this->Disks.uCount > 0)
	{
		free(this->Disks.Disk);
	}

	memset(&this->Disks, 0, sizeof(this->Disks));
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////

ushort CSFS::SetSuperBlockChecksum(SUPERBLOCK *pSB)
{
	pSB->uChecksum = 255;

	pSB->uChecksum ^= pSB->uDataBlocks;
	pSB->uChecksum ^= pSB->uIndexBlocks;
	pSB->uChecksum ^= pSB->uMagicNumber;
	pSB->uChecksum ^= pSB->uVersion;
	pSB->uChecksum ^= pSB->uTotalBlocks;
	pSB->uChecksum ^= pSB->uStartingBlock;
	pSB->uChecksum ^= pSB->uBlockSize;
	pSB->uChecksum ^= pSB->uStartingBlock;
	pSB->uChecksum ^= pSB->uUsableCapacity;

	return pSB->uChecksum;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool CSFS::FormatDisk(ushort uChannel, ushort uDisk, SFSFormatProgressCallback pCallback, void *pUserData)
{
	SFSENUMDISK *pDisk = this->GetDisk(uChannel, uDisk);
	if(!pDisk)
	{
		return false;
	}
	return this->FormatDisk(pDisk, pCallback, pUserData);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool CSFS::FormatDisk(const char *sPath, SFSFormatProgressCallback pCallback, void *pUserData)
{
	SFSENUMDISK *pDisk = this->GetDisk(sPath);
	if(!pDisk)
	{
		return false;
	}
	return this->FormatDisk(pDisk, pCallback, pUserData);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool CSFS::FormatDisk(SFSENUMDISK *pDisk, SFSFormatProgressCallback pCallback, void *pUserData)
{
	if(pCallback)
	{
		pCallback(pDisk, 0, pUserData);
	}

	SUPERBLOCK SB;
	memset(&SB, 0, sizeof(SUPERBLOCK));
	SB.uVersion = 1;
	SB.uMagicNumber = 128;
	SB.uChecksum = 0;

	//Auto-Select a block size:
	if(pDisk->pATADisk->uCapacity > 1024ul*1024ul*1024ul * 3ul) {
		SB.uBlockSize = 8192; 
	}
	else if(pDisk->pATADisk->uCapacity > 1024ul*1024ul*1024ul) {
		SB.uBlockSize = 4096; 
	}
	else if(pDisk->pATADisk->uCapacity > 1024ul*1024ul * 100ul) {
		SB.uBlockSize = 4096; 
	}
	else if(pDisk->pATADisk->uCapacity > 1024ul*1024ul * 10ul) {
		SB.uBlockSize = 2048; 
	}
	else if(pDisk->pATADisk->uCapacity > 1024ul*1024ul) {
		SB.uBlockSize = 1024; 
	}
	else {
		SB.uBlockSize = 512; 
	}

	SB.uStartingBlock = 10; //Totally arbitrary.
	SB.uUsableCapacity = pDisk->pATADisk->uCapacity - (SB.uStartingBlock * SB.uBlockSize);
	SB.uTotalBlocks = (SB.uUsableCapacity / SB.uBlockSize) - SB.uStartingBlock;
	SB.uSectorsPerBlock = SB.uBlockSize / pDisk->pATADisk->uBytesPerSector;
	SB.uIndexBlocks = (SB.uTotalBlocks * sizeof(INDEXENTRY)) / SB.uBlockSize;
	SB.uDataBlocks = SB.uTotalBlocks - SB.uIndexBlocks;
	SB.uUID = 123456; //FIXFIX: Needs to be random

	this->SetSuperBlockChecksum(&SB);

	//Write superblock:
	ATA->WriteSectors(pDisk->uChannel, pDisk->uDisk, 0, 1, &SB);

	//Write blank indexes:
	byte *pIndexes = (byte *) calloc(pDisk->pATADisk->uBytesPerSector, 1);
	uint iLBA = (SB.uStartingBlock * SB.uBlockSize / pDisk->pATADisk->uBytesPerSector);
	for(uint uBlock = 0; uBlock < SB.uIndexBlocks; uBlock++)
	{
		for(uint uSector = 0; uSector < SB.uBlockSize / pDisk->pATADisk->uBytesPerSector; uSector++)
		{
			ATA->WriteSectors(pDisk->uChannel, pDisk->uDisk, iLBA++, 1, pIndexes);
		}
		if(pCallback)
		{
			pCallback(pDisk, ((float)uBlock / (float)SB.uIndexBlocks) * 100.0f, pUserData);
		}
	}

	free(pIndexes);
	return true;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////

SFSENUMDISK *CSFS::GetDisk(const char *sPath)
{
	if(sPath[1] == ':')
	{
		for(uint uIndex = 0; uIndex < this->Disks.uCount; uIndex++)
		{
			if(ToUpper(sPath[0]) == ToUpper(this->Disks.Disk[uIndex].cIdentifier))
			{
				return &this->Disks.Disk[uIndex];
			}
		}
	}
	else{
		return this->Disks.pCurrent;
	}

	return NULL;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////

SFSENUMDISK *CSFS::GetDisk(ushort uChannel, ushort uDisk)
{
	for(uint uIndex = 0; uIndex < this->Disks.uCount; uIndex++)
	{
		if(this->Disks.Disk[uIndex].uChannel == uChannel && this->Disks.Disk[uIndex].uDisk == uDisk)
		{
			return &this->Disks.Disk[uIndex];
		}
	}

	return NULL;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////

FILEHANDLE *CSFS::OpenFile(const char *sFileName)
{
	return NULL;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////

uint CSFS::ListFiles(const char *sPath)
{
	SFSENUMDISK *pDisk = this->GetDisk(sPath);
	if(!pDisk)
	{
		return 0;
	}

	uint uFiles = 0;

	INDEXENTRY *pIndexes = (INDEXENTRY *) calloc(pDisk->SB.uBlockSize, 1);

	uint uIndexesPerBlock = pDisk->SB.uBlockSize / sizeof(INDEXENTRY);

	uint uSector = (pDisk->SB.uStartingBlock * pDisk->SB.uBlockSize / pDisk->pATADisk->uBytesPerSector);
	for(uint uIndexBlock = 0; uIndexBlock < pDisk->SB.uIndexBlocks; uIndexBlock++)
	{
		if(ATA->ReadSectors(pDisk->uChannel, pDisk->uDisk, uSector,
			pDisk->SB.uBlockSize / pDisk->pATADisk->uBytesPerSector, pIndexes))
		{
			for(ushort iIndex = 0; iIndex < uIndexesPerBlock; iIndex++)
			{
				if(pIndexes[iIndex].Attributes != SFSINDEX_ATTRIB_FREE)
				{
					printf("%s\n", pIndexes[iIndex].sFileName);
					uFiles++;
				}
				else{
					return uFiles;
				}
			}

			uSector += pDisk->SB.uStartingBlock;
		}
		else{
			BSOD("Failed to read sector.");
			return uFiles;
		}
	}

	free(pIndexes);

	return uFiles;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////

FILEHANDLE *CSFS::CreateFile(const char *sFileName)
{
	SFSENUMDISK *pDisk = this->GetDisk(sFileName);
	if(!pDisk)
	{
		return NULL;
	}

	INDEXENTRY *pIndexes = (INDEXENTRY *) calloc(pDisk->SB.uBlockSize, 1);

	uint uIndexesPerBlock = pDisk->SB.uBlockSize / sizeof(INDEXENTRY);

	uint uSector = (pDisk->SB.uStartingBlock * pDisk->SB.uBlockSize / pDisk->pATADisk->uBytesPerSector);
	for(uint uIndexBlock = 0; uIndexBlock < pDisk->SB.uIndexBlocks; uIndexBlock++)
	{
		if(ATA->ReadSectors(pDisk->uChannel, pDisk->uDisk, uSector,
			pDisk->SB.uBlockSize / pDisk->pATADisk->uBytesPerSector, pIndexes))
		{
			for(ushort iIndex = 0; iIndex < uIndexesPerBlock; iIndex++)
			{
				if(pIndexes[iIndex].Attributes == SFSINDEX_ATTRIB_FREE)
				{
					pIndexes[iIndex].Attributes = SFSINDEX_ATTRIB_USED | SFSINDEX_ATTRIB_ENDOFCHAIN;

					strcpy(pIndexes[iIndex].sFileName, sFileName);

					pIndexes[iIndex].uFirstBlock = (pDisk->SB.uStartingBlock + pDisk->SB.uIndexBlocks)
						+ (uIndexBlock * pDisk->SB.uBlockSize * uIndexesPerBlock) + (iIndex * pDisk->SB.uBlockSize);

					if(ATA->WriteSectors(pDisk->uChannel, pDisk->uDisk,
						uSector, pDisk->SB.uBlockSize / pDisk->pATADisk->uBytesPerSector, pIndexes))
					{
						FILEHANDLE *pHandle = (FILEHANDLE *) calloc(sizeof(FILEHANDLE), 1);
						free(pIndexes);

						return pHandle;
					}
					else{
						BSOD("Failed to write sector.");
						return NULL;
					}
				}
			}

			uSector += pDisk->SB.uStartingBlock;
		}
		else{
			BSOD("Failed to read sector.");
			return NULL;
		}
	}

	free(pIndexes);

	return NULL;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool CSFS::CloseFile(FILEHANDLE *pHandle)
{
	if(pHandle)
	{
		free(pHandle);
	}
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////

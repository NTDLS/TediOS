#ifndef _ENTRY_CPP
#define _ENTRY_CPP
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#include <Windows.H>
#include <Stdio.H>
#include <Stdlib.H>
/*
typedef long long si64;
typedef short int sint;
typedef signed char schar;
typedef signed long slong;
typedef signed short sshort;
typedef unsigned char byte;
typedef unsigned char uchar;
typedef unsigned int dword;
typedef unsigned int uint32;
typedef unsigned int size_t;
typedef unsigned int uint;
typedef unsigned long long ui64;
typedef unsigned long ulong;
typedef unsigned short int usint;
typedef unsigned short int word;
typedef unsigned short ushort;
//typedef unsigned char bool;

typedef struct _tag_Hard_Drive_Information {
	char sModel[40];
	char sSerialNumber[24];
	char sFirmwareRev[12];

	uint uHeads;
	uint uSectorsPerTrack;
	uint uBytesPerSector;
	uint uCylinders;
	uint uSectors;
	uint uCapacity;

	bool bPresent;
} ATADISKINFO, *LPATADISKINFO;


///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

//Constants
#define SB_MAGICNUMBER        128
#define SB_CURRENTVERSION       1

//Superblock:
#define SB_SZ_RESERVED1        11  //Reserved for compatibility (boot code)
#define SB_SZ_RESERVED2        21  //Reserved for compatibility (legacy BIOS parameter block)
#define SB_SZ_RESERVED3       372  //Reserved for compatibility (boot code)
#define SB_SZ_DATAAREABLOCKS    4  //Size of data area in blocks
#define SB_SZ_INDEXAREABLOCKS   4  //Size of index area in blocks
#define SB_SZ_MAGICNUMBER       2  //Magic number (128)
#define SB_SZ_VERSION           2  //Simple File System version number.
#define SB_SZ_TOTALBLOCKS       4  //Total number of blocks
#define SB_SZ_STARTINGBLOCK     4  //Starting block of data section.
#define SB_SZ_BLOCKSIZE         2  //Block size.
#define SB_SZ_SECTORSPERBLOCK   2  //Sectors per Block.
#define SB_SZ_CHECKSUM          2  //Checksum.
#define SB_SZ_UID               4  //Unique Identifier.
#define SB_SZ_USABLECAPACITY    4  //Checksum.
#define SB_SZ_RESERVED			 8  //Reserved, but usable space in the superblock.
#define SB_SZ_RESERVED4         64 //Reserved for compatibility (partition table)
#define SB_SZ_RESERVED5         2  //Reserved for compatibility (boot signature)

//File system limits:
#define MAX_FILE_NAME		50
#define MAX_FILE_EXT		9

#define SFSINDEX_ATTRIB_FREE		0x0000ul //Indexes that are free.
#define SFSINDEX_ATTRIB_USED		0x0001ul //Indexes that are allocated.
#define SFSINDEX_ATTRIB_ENDOFCHAIN	0x0002ul //Indexes that are the tail of a "chain" of indexes.
#define SFSINDEX_ATTRIB_RESERVED_1	0x0004ul //Reserved
#define SFSINDEX_ATTRIB_RESERVED_2	0x0010ul //Reserved
#define SFSINDEX_ATTRIB_RESERVED_3	0x0020ul //Reserved
#define SFSINDEX_ATTRIB_RESERVED_4	0x0040ul //Reserved
#define SFSINDEX_ATTRIB_RESERVED_5	0x0080ul //Reserved
#define SFSINDEX_ATTRIB_RESERVED_6	0x0100ul //Reserved

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#pragma pack(push, 1)
typedef struct _tag_SuperBlock { //This structure should be exaclty 512 bytes (1 sector).
	unsigned char ActualReserve0[SB_SZ_RESERVED1 + SB_SZ_RESERVED2 + SB_SZ_RESERVED3];

	uint uDataBlocks;
	uint uIndexBlocks;
	ushort uMagicNumber;
	ushort uVersion;
	uint uTotalBlocks;
	uint uStartingBlock;
	ushort uBlockSize;
	ushort uSectorsPerBlock;
	ushort uChecksum;
	uint uUID;

	uint uUsableCapacity;

	unsigned char sReserved[SB_SZ_RESERVED];

	unsigned char ActualReserve1[SB_SZ_RESERVED4 + SB_SZ_RESERVED5];
} SUPERBLOCK, LPSUPERBLOCK;

typedef struct _tag_BlockInfo { //This structure should be exaclty 4 bytes.
	ushort Bytes;		//Number of bytes used in this block.
	ushort Reserved;
} BLOCKINFO, *LPBLOCKINFO;

typedef struct _tag_IndexEntry { //This structure should be exaclty 64 bytes (8 per sector).
	char sFileName[MAX_FILE_NAME + MAX_FILE_EXT]; //56
	byte Attributes;	//Attributes of the block.
	uint uFirstBlock;
} INDEXENTRY, *LPINDEXENTRY;
#pragma pack(pop)


#define SECTOR_SIZE 512 //Debug
FILE *hGlobalFile = fopen("C:\\FS.txt", "rb+"); //Debug
ATADISKINFO Disk; //Debug
char *sOSCache = NULL; //Debug
SUPERBLOCK SB;  //Debug

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool WriteSectors(word uChannel, uchar uDisk, uint uiLBAAddress, uchar SectorCount, const void *pInBuffer)
{
	fseek(hGlobalFile, uiLBAAddress * Disk.uBytesPerSector, SEEK_SET);
	fwrite((char *)pInBuffer, Disk.uBytesPerSector, SectorCount, hGlobalFile);
	return true;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool ReadSectors(word uChannel, uchar uDisk, uint uiLBAAddress, uchar SectorCount, void *pOutBuffer)
{
	fseek(hGlobalFile, uiLBAAddress * Disk.uBytesPerSector, SEEK_SET);
	fread((char *)pOutBuffer, Disk.uBytesPerSector, SectorCount, hGlobalFile);
	return true;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////

ushort SetSuperBlockChecksum(SUPERBLOCK *pSB)
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

void FormatDisk(ushort uChannel, ushort uDisk)
{
	memset(&SB, 0, sizeof(SUPERBLOCK));
	SB.uVersion = 1;
	SB.uMagicNumber = 128;
	SB.uChecksum = 0;

	//Auto-Select a block size:
	if(Disk.uCapacity > 1024ul*1024ul*1024ul * 3ul) {
		SB.uBlockSize = 8192; 
	}
	else if(Disk.uCapacity > 1024ul*1024ul*1024ul) {
		SB.uBlockSize = 4096; 
	}
	else if(Disk.uCapacity > 1024ul*1024ul * 100ul) {
		SB.uBlockSize = 4096; 
	}
	else if(Disk.uCapacity > 1024ul*1024ul * 10ul) {
		SB.uBlockSize = 2048; 
	}
	else if(Disk.uCapacity > 1024ul*1024ul) {
		SB.uBlockSize = 1024; 
	}
	else {
		SB.uBlockSize = 512; 
	}

	SB.uStartingBlock = 10; //Totally arbitrary.
	SB.uUsableCapacity = Disk.uCapacity - (SB.uStartingBlock * SB.uBlockSize);
	SB.uTotalBlocks = (SB.uUsableCapacity / SB.uBlockSize) - SB.uStartingBlock;
	SB.uSectorsPerBlock = SB.uBlockSize / Disk.uBytesPerSector;
	SB.uIndexBlocks = (SB.uTotalBlocks * sizeof(INDEXENTRY)) / SB.uBlockSize;
	SB.uDataBlocks = SB.uTotalBlocks - SB.uIndexBlocks;
	SB.uUID = 123456; //FIXFIX: Needs to be random

	SetSuperBlockChecksum(&SB);

	//Write superblock:
	WriteSectors(0, 0, 0, 1, &SB);

	//Write blank indexes, required to set blocks as unused.
	byte *pIndexes = (byte *) calloc(Disk.uBytesPerSector, 1);
	uint iLBA = (SB.uStartingBlock * SB.uBlockSize / Disk.uBytesPerSector);
	for(uint uBlock = 0; uBlock < SB.uIndexBlocks; uBlock++)
	{
		for(uint uSector = 0; uSector < SB.uBlockSize / Disk.uBytesPerSector; uSector++)
		{
			WriteSectors(0, 0, iLBA++, 1, pIndexes);
		}
	}
	free(pIndexes);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool CreateFile(ushort uChannel, ushort uDisk, const char *sFileName)
{
	INDEXENTRY *pIndexes = (INDEXENTRY *) calloc(SB.uBlockSize, 1);

	uint uIndexesPerBlock = SB.uBlockSize / sizeof(INDEXENTRY);

	uint uSector = (SB.uStartingBlock * SB.uBlockSize / Disk.uBytesPerSector);
	for(uint uIndexBlock = 0; uIndexBlock < SB.uIndexBlocks; uIndexBlock++)
	{
		if(ReadSectors(0, 0, uSector, SB.uBlockSize / Disk.uBytesPerSector, pIndexes))
		{
			for(ushort iIndex = 0; iIndex < uIndexesPerBlock; iIndex++)
			{
				if(pIndexes[iIndex].Attributes == SFSINDEX_ATTRIB_FREE)
				{
					pIndexes[iIndex].Attributes = SFSINDEX_ATTRIB_USED | SFSINDEX_ATTRIB_ENDOFCHAIN;

					strcpy(pIndexes[iIndex].sFileName, sFileName);
					pIndexes[iIndex].uFirstBlock = (SB.uStartingBlock + SB.uIndexBlocks)
						+ (uIndexBlock * SB.uBlockSize * uIndexesPerBlock) + (iIndex * SB.uBlockSize);

					if(WriteSectors(0, 0, uSector, SB.uBlockSize / Disk.uBytesPerSector, pIndexes))
					{
						printf("%u\n", pIndexes[iIndex].uFirstBlock);

						free(pIndexes);
						return true;
					}
					else{
						printf("Blowup!");
						system("pause");
					}
				}
			}

			uSector += SB.uStartingBlock;
		}
		else{
			printf("Blowup!");
			system("pause");
		}
	}

	free(pIndexes);

	return false;
}
*/
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#define xassert(e) ((e) ? (void) 0 : _xassert(__FILE__, __LINE__, #e))
void _xassert(const char *file, int lineno, const char *e);

void _xassert(const char *file, int lineno, const char *e)
{
    printf("assertion \"%s\" failed.\nfile: %s, line %d\n", e, file, lineno);
}

int main(int argc, char *argv[])
{

xassert(false);


/*
	Disk.uHeads = 8;
	Disk.uSectorsPerTrack = 16;
	Disk.uBytesPerSector = 512;
	Disk.uCylinders = 256;

	//Disk.uHeads = 16;
	//Disk.uSectorsPerTrack = 63;
	//Disk.uBytesPerSector = 512;
	//Disk.uCylinders = 3120;

	Disk.bPresent = true;
	Disk.uCapacity = Disk.uCylinders * Disk.uHeads * Disk.uSectorsPerTrack * Disk.uBytesPerSector;
	Disk.uSectors = Disk.uCylinders * Disk.uHeads * Disk.uSectorsPerTrack;

	char sBuf[SECTOR_SIZE + 1];
	memset(sBuf, 0, sizeof(sBuf));

	//sprintf(sBuf, "@");
	//for(uint uSector = 0; uSector < Disk.uSectors; uSector++)
	//{
		//WriteSectors(0, 0, uSector, 1, sBuf);
	//}

	FormatDisk(0, 0);

	char sFileName[255];
	for(uint uFile = 0; uFile < 10000; uFile++)
	{
		sprintf(sFileName, "Test_File_%d.txt", uFile);
		if(!CreateFile(0, 0, sFileName))
		{
			break;
		}
	}

	fclose(hGlobalFile);
	*/

	system("Pause");

	return 0;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#endif

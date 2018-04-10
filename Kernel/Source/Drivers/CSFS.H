#ifndef _CSFS_H
#define _CSFS_H
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//Simple File System
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#include "../Lib/Types.H"

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

//Index attributes:
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

typedef struct _tag_Enum_Disk {
	unsigned char cIdentifier;
	ushort uChannel;
	ushort uDisk;
	SUPERBLOCK SB;
	ATADISKINFO *pATADisk;
} SFSENUMDISK, *LPSFSENUMDISK;

typedef struct _tag_Enum_Disks {
	SFSENUMDISK *Disk;		//Array
	SFSENUMDISK *pCurrent;	//Pointer
	ushort uCount;
} SFSENUMDISKS, *LPSFSENUMDISKS;

typedef struct _tag_File_Handle {
	uint uReserved;
} FILEHANDLE, *LPFILEHANDLE;

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////

typedef void (*SFSFormatProgressCallback)(SFSENUMDISK *pDisk, float fPercentComplete, void *pUserData);

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////

class CSFS {

public:
	bool Initialize(void);
	void Destroy(void);

	bool FormatDisk(ushort uChannel, ushort uDisk, SFSFormatProgressCallback pCallback, void *pUserData);
	bool FormatDisk(SFSENUMDISK *pDisk, SFSFormatProgressCallback pCallback, void *pUserData);
	bool FormatDisk(const char *sPath, SFSFormatProgressCallback pCallback, void *pUserData);

	FILEHANDLE *CreateFile(const char *sFileName);
	FILEHANDLE *OpenFile(const char *sFileName);
	bool CloseFile(FILEHANDLE *pHandle);

    uint ListFiles(const char *sPath);

	SFSENUMDISK *GetDisk(ushort uChannel, ushort uDisk);
	SFSENUMDISK *GetDisk(const char *sPath);

private:
	ushort SetSuperBlockChecksum(SUPERBLOCK *pSB);

	SFSENUMDISKS Disks;
};

extern CSFS *SFS;

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#endif

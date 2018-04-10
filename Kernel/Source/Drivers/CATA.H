#ifndef _CATA_H
#define _CATA_H
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#include "../Lib/Types.H"

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#define SECTOR_SIZE 			512
#define KILOBYTE				1024
#define MEGABYTE				1048576
#define GIGABYTE				1073741824
#define TERABYTE				1099511627776LL
#define PETABYTE				1125899906842624LL

#define ATA_DISKS_PER_CHANNEL	2
#define ATA_MAGIC_NUMBER		0x88
#define ATA_TIMEOUT 			100 //Number of times to try to command a drive.

/* ports */
#define PORT_ATA_PRIBASE		0x1F0
#define PORT_ATA_SECBASE		0x170

/* commands */
#define PORT_ATA_DATA			0x0 //ATAREG_DR
#define PORT_ATA_ERROR			0x1 //ATAREG_ER / ATAREG_FR
#define PORT_ATA_SECCOUNT		0x2 //ATAREG_SC
#define PORT_ATA_SECSTART		0x3 //ATAREG_SN
#define PORT_ATA_CYL_LO 		0x4 //ATAREG_CL
#define PORT_ATA_CYL_HI 		0x5 //ATAREG_CH
#define PORT_ATA_DEVICE			0x6 //ATAREG_DEV
#define PORT_ATA_HEAD			0x6 //ATAREG_DH
#define PORT_ATA_COMMAND		0x7 //ATAREG_CR
#define PORT_ATA_STATUS 		0x7 //ATAREG_SR

#define PORT_ATA_LBALOW 		0x3
#define PORT_ATA_LBAMID 		0x4
#define PORT_ATA_LBAHIGH		0x5
#define PORT_ATA_DRIVE			0x6

/* read */
#define ATA_READ_WITH_RETRY 	0x20
#define ATA_READ_NO_RETRY		0x21
#define ATA_READL_WITH_RETRY	0x22
#define ATA_READL_NO_RETRY		0x23

/* write */
#define ATA_WRITE_WITH_RETRY	0x30
#define ATA_WRITE_NO_RETRY		0x31
#define ATA_WRITEL_WITH_RETRY	0x32
#define ATA_WRITEL_NO_RETRY 	0x33

//other:
#define ATA_FORMAT_TRACK		0x50

//section status:
#define ATA_STATUS_ERROR		0x01 //error -- if set, read the ERROR register to get the actual error
#define ATA_STATUS_IDX 			0x02 //vendor-specific
#define ATA_STATUS_CORR			0x04 //correctable error -- if set, a correctable error has occurred.
#define ATA_STATUS_DRQ 			0x08 //data request -- if set, the drive is ready to send data
#define ATA_STATUS_DSC 			0x10 //device seek complete -- if set, the disk is settled on a track
#define ATA_STATUS_DF			0x20 //device fault -- internal error (vendor-specific)
#define ATA_STATUS_DRDY			0x40 //drive ready -- if set, drive is ready for commands
#define ATA_STATUS_BSY 			0x80 //drive is busy -- if set, NO other bits are valid

//section errors:
#define HDE_NOERROR 			0x00 //no error occurred
#define HDE_TIMEOUT 			0x01 //timeout waiting for response
#define HDE_NOSUCHTRACK 		0x02 //no such track exists on the disk
#define HDE_ABORTED 			0x03 //the IDE controller or disk aborted the command
#define HDE_ECC 				0x04 //ECC detected error
#define HDE_BADBLOCK			0x05 //attempt to access bad block
#define HDE_INVALIDDRIVE		0x06 //no such drive exists on controller
#define HDE_UNKNOWN 			0xFF //other or unknown error occurred

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////

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

typedef struct _tag_ATA_Channel_Info {
	ATADISKINFO *Disk;
	ushort uDisks;
	ushort uMaxDisks;
	bool bPresent;
} ATACHANNELINFO, LPATACHANNELINFO;

typedef struct _tag_ATA_Attached_Info {
	ATACHANNELINFO *Channel;
	ushort uMaxChannels;
	ushort uChannels;
	ushort uDisks;
} ATAATTACHEDINFO, *LPATAATTACHEDINFO;

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////

typedef void (*ATAFormatProgressCallback)(ATADISKINFO *pDisk, float fPercentComplete, void *pUserData);

class CATA {
public:
	bool ReadSectors(word uChannel, uchar uDisk, uint uStartSector, uchar SectorCount, void *pOutBuffer);
	bool WriteSectors(word uChannel, uchar uDisk, uint uStartSector, uchar SectorCount, const void *pOutBuffer);
	bool ZeroSectors(word uChannel, uchar uDisk, uint uStartSector, uchar SectorCount);
	bool Initialize(void);
	void PrintSector(void *pBuffer);

	void GetADI(ATAATTACHEDINFO *pATAADI);
	void DestroyADI(ATAATTACHEDINFO *pATAADI);

	ATADISKINFO *Disk(ushort uChannel, ushort uDisk);
	void FormatDisk(ushort uChannel, ushort uDisk);
	void FormatDisk(ushort uChannel, ushort uDisk, ATAFormatProgressCallback pCallback, void *pUserData);
	void ZeroDisk(ushort uChannel, ushort uDisk, ATAFormatProgressCallback pCallback, void *pUserData);

	void Debug(void);

private:
	bool GetDiskInfo(word uChannel, uchar uDisk, ATADISKINFO *pHDDI);
	bool WaitOnChannel(word uChannel);
	bool StatusWait(word uChannel, word Mask);

	void WaitStatus(unsigned short command, uchar set, uchar clear);
	void SetSector(word uChannel, uchar uDisk, uint uStartSector, uchar SectorCount, ushort uCommand);

	const char *WordToText(const word *pwData, int iStart, int iEnd);
	ATAATTACHEDINFO ADI; //Information pertaining to attached disks.
};

extern CATA *ATA;

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#endif

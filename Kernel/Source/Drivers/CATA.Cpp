///////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#include "../Lib/STDLib.H"
#include "../Lib/String.H"
#include "../Lib/Float.H"

#include "CATA.H"
#include "System.H"
#include "Timer.h"
#include "Video.h"
#include "CMemory.h"
#include "CKeyboard.H"

#pragma option -a-

//http://www.osdever.net/tutorials/LBA.php
//http://forgecvs1.novell.com/viewcvs/tyros/tyros/drivers/hdd.c
//http://www.vunet.org/~vai/jtm/kernel32/irq/hd/
//http://www.programmersheaven.com/download/16132/0/ZipView.aspx

CATA *ATA = NULL;

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void CATA::Debug(void)
{
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void CATA::PrintSector(void *pBuffer)
{
	char sPad[10];
	int iRPos = 0;

	SetTextColor(RED, BLACK);
	printf("    001 002 003 004 005 006 007 008 009 010 011 012 013 014 015 016\n");

	for(int iRow = 0; iRow < 16; iRow++)
	{
		SetTextColor(RED, BLACK);
		sprintf(sPad, "%d", iRow + 1);
		printf("%s ", PadLeft(sPad, ' ', 3));

		SetTextColor(WHITE, BLACK);
		for(int iCol = 0; iCol < 16; iCol++)
		{
			if(((unsigned char *)pBuffer)[iRPos])
			{
				sprintf(sPad, "%d", ((unsigned char *)pBuffer)[iRPos++]);
				printf("%s ", PadLeft(sPad, ' ', 3));
			}
			else{
				printf("    ");
				iRPos++;
			}
		}
		printf("\n");
	}

	SetTextColor(WHITE, BLACK);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void CATA::FormatDisk(ushort uChannel, ushort uDisk)
{
	this->FormatDisk(uChannel, uDisk, NULL, NULL);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void CATA::FormatDisk(ushort uChannel, ushort uDisk, ATAFormatProgressCallback pCallback, void *pUserData)
{
	ATADISKINFO *pDisk = this->Disk(uChannel, uDisk);

	if(pCallback)
	{
		pCallback(pDisk, 0, pUserData);
	}

	for(uint uSector = 0; uSector < pDisk->uSectors; uSector += pDisk->uSectorsPerTrack)
	{
		ATA->ZeroSectors(uChannel, uDisk, uSector, pDisk->uSectorsPerTrack);
		if(pCallback)
		{
			pCallback(pDisk, ((float)uSector / (float)pDisk->uSectors) * 100.0f, pUserData);
		}
	}
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void CATA::ZeroDisk(ushort uChannel, ushort uDisk, ATAFormatProgressCallback pCallback, void *pUserData)
{
	ATADISKINFO *pDisk = this->Disk(uChannel, uDisk);

	if(pCallback)
	{
		pCallback(pDisk, 0, pUserData);
	}

	for(uint uSector = 0; uSector < pDisk->uSectors; uSector += pDisk->uSectorsPerTrack)
	{
		ATA->ZeroSectors(uChannel, uDisk, uSector, pDisk->uSectorsPerTrack);
		if(pCallback)
		{
			pCallback(pDisk, ((float)uSector / (float)pDisk->uSectors) * 100.0f, pUserData);
		}
	}
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool CATA::ZeroSectors(word uChannel, uchar uDisk, uint uiLBAAddress, uchar SectorCount)
{
	word Base = uChannel ? PORT_ATA_SECBASE : PORT_ATA_PRIBASE;
	if(!this->WaitOnChannel(Base))
	{
		BSOD("Timed-out waiting on WaitOnChannel to become ready.");
		return false;
	}

	//----( Get the drive, the Upperbits of the sector and put the drive in LBA mode ) ----
	//Send the drive indicator, some magic bits, and highest 4 bits of the block address:
	outportb(Base + PORT_ATA_HEAD, 0xE0 | (uDisk << 4) | ((uiLBAAddress >> 24) & 0x0F));
	outportb(Base + PORT_ATA_ERROR, 0x00);
	outportb(Base + PORT_ATA_SECCOUNT, SectorCount);
	outportb(Base + PORT_ATA_SECSTART, (uchar)uiLBAAddress);
	outportb(Base + PORT_ATA_CYL_LO, (uchar)((uiLBAAddress>>8)&0xFF));
	outportb(Base + PORT_ATA_CYL_HI, (uchar)((uiLBAAddress>>16)&0xFF));

	if(!this->StatusWait(Base, ATA_STATUS_DRDY))
	{
		BSOD("Timed-out waiting on StatusWait to become ready.");
		return false;
	}

	outportb(Base + PORT_ATA_COMMAND, ATA_WRITE_WITH_RETRY);

	if(!this->StatusWait(Base, ATA_STATUS_DRDY))
	{
		BSOD("Timed-out waiting on StatusWait to become ready.");
		return false;
	}

	//char *sNull = "\0\0";
	char *sNull = "~@"; //For debugging

	for(uint iByte = 0; iByte < (SectorCount * 256); iByte++)
	{
		outportw(Base + PORT_ATA_DATA, ((short *)sNull)[0]);
	}

	//Did any errors occur?
	if(inportb(Base + PORT_ATA_STATUS) & ATA_STATUS_ERROR)
	{
		//ErrorCode: inportb(base + PORT_ATA_ERROR);
		BSOD("HD/IO error!");
		return false;
	} 

	return true;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool CATA::WriteSectors(word uChannel, uchar uDisk, uint uiLBAAddress, uchar SectorCount, const void *pInBuffer)
{
	word Base = uChannel ? PORT_ATA_SECBASE : PORT_ATA_PRIBASE;
	if(!this->WaitOnChannel(Base))
	{
		BSOD("Timed-out waiting on WaitOnChannel to become ready.");
		return false;
	}

	//----( Get the drive, the Upperbits of the sector and put the drive in LBA mode ) ----
	//Send the drive indicator, some magic bits, and highest 4 bits of the block address:
	outportb(Base + PORT_ATA_HEAD, 0xE0 | (uDisk << 4) | ((uiLBAAddress >> 24) & 0x0F));
	outportb(Base + PORT_ATA_ERROR, 0x00);
	outportb(Base + PORT_ATA_SECCOUNT, SectorCount);
	outportb(Base + PORT_ATA_SECSTART, (uchar)uiLBAAddress);
	outportb(Base + PORT_ATA_CYL_LO, (uchar)((uiLBAAddress>>8)&0xFF));
	outportb(Base + PORT_ATA_CYL_HI, (uchar)((uiLBAAddress>>16)&0xFF));

	if(!this->StatusWait(Base, ATA_STATUS_DRDY))
	{
		BSOD("Timed-out waiting on StatusWait to become ready.");
		return false;
	}

	outportb(Base + PORT_ATA_COMMAND, ATA_WRITE_WITH_RETRY);

	if(!this->StatusWait(Base, ATA_STATUS_DRDY))
	{
		BSOD("Timed-out waiting on StatusWait to become ready.");
		return false;
	}

	StopInterrupts();

	for(uint iByte = 0; iByte < (SectorCount * 256); iByte++)
	{
        outportw(Base + PORT_ATA_DATA, ((short *)pInBuffer)[iByte]);
	}

	StartInterrupts();

	//Did any errors occur?
	if(inportb(Base + PORT_ATA_STATUS) & ATA_STATUS_ERROR)
	{
		//ErrorCode: inportb(base + PORT_ATA_ERROR);
		BSOD("HD/IO error!");
		return false;
	}

	return true;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool CATA::ReadSectors(word uChannel, uchar uDisk, uint uiLBAAddress, uchar SectorCount, void *pOutBuffer)
{
	word wBuf;
	memset(pOutBuffer, 0, SectorCount * SECTOR_SIZE);

	word Base = uChannel ? PORT_ATA_SECBASE : PORT_ATA_PRIBASE;
	if(!this->StatusWait(Base, ATA_STATUS_DRDY))
	{
		BSOD("Timed-out waiting on StatusWait to become ready.");
		return false;
	}

	//----( Get the drive, the Upperbits of the sector and put the drive in LBA mode ) ----
	//Send the drive indicator, some magic bits, and highest 4 bits of the block address:
	outportb(Base + PORT_ATA_HEAD, 0xE0 | (uDisk << 4) | ((uiLBAAddress >> 24) & 0x0F));
	outportb(Base + PORT_ATA_ERROR, 0x00);
	outportb(Base + PORT_ATA_SECCOUNT, SectorCount);
	outportb(Base + PORT_ATA_SECSTART, (uchar)uiLBAAddress);
	outportb(Base + PORT_ATA_CYL_LO, (uchar)((uiLBAAddress>>8)&0xFF));
	outportb(Base + PORT_ATA_CYL_HI, (uchar)((uiLBAAddress>>16)&0xFF));

	if(!this->StatusWait(Base, ATA_STATUS_DRDY))
	{
		BSOD("Timed-out waiting on StatusWait to become ready.");
		return false;
	}

	outportb(Base + PORT_ATA_COMMAND, ATA_READ_WITH_RETRY);

	if(!this->StatusWait(Base, ATA_STATUS_DRDY))
	{
		BSOD("Timed-out waiting on StatusWait to become ready.");
		return false;
	}

	StopInterrupts();

	//Read the data from the device into the buffer.
	for(uint iByte = 0; iByte < (SectorCount * 256); iByte++)
	{
        ((short*)pOutBuffer)[iByte] = inportw(Base + PORT_ATA_DATA);
	}

	StartInterrupts();

	//Did any errors occur?
	if(inportb(Base + PORT_ATA_STATUS) & ATA_STATUS_ERROR)
	{
		//ErrorCode: inportb(base + PORT_ATA_ERROR);
		BSOD("HD/IO error!");
		return false;
	}

	return true;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool CATA::StatusWait(word uChannel, word Mask)
{
	if(WaitOnChannel(uChannel))
	{
		for(int iTry = 0; !(inportb(uChannel + PORT_ATA_STATUS) &Mask); iTry++)
		{
			if(iTry == 10)
			{
				return false; //Timed-out waiting on HD to become ready (initial).
			}
			Sleep(1); //The drive is busy.
		}
	}
	return true;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool CATA::WaitOnChannel(word uChannel)
{
	for(int iTry = 0; ((inportb(uChannel + PORT_ATA_STATUS) &ATA_STATUS_BSY) == ATA_STATUS_BSY); iTry++)
	{
		if(iTry == 10)
		{
			return false; //Timed-out waiting on HD to become ready (initial).
		}
		Sleep(1); //The drive is busy.
	}
	return true;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool CATA::GetDiskInfo(word uChannel, uchar uDisk, ATADISKINFO *pATADI)
{
	word Base = uChannel ? PORT_ATA_SECBASE : PORT_ATA_PRIBASE;

	memset(pATADI, 0, sizeof(ATADISKINFO));

	word *pwData = (word *)calloc(sizeof(word), 256);

	if(!this->WaitOnChannel(Base))
	{
		return false;
	}

	// Order to get drive information
	outportb(Base + 6, uDisk ? 0xB0:0xA0);
	outportb(Base + 7, 0xEC);

	for(int iTry = 0; inportb(Base + PORT_ATA_STATUS) != 0x58; iTry++)
	{
		if(iTry == 10)
		{
			return false; //Timed-out waiting on HD to become ready.
		}

		Sleep(1); //The drive is busy.
	}

    //Read the drive information
	for(int iByte = 0; iByte < 256; iByte++)
	{
		pwData[iByte] = inportw(Base);
	}

	pATADI->bPresent = true;
	pATADI->uHeads = pwData[3];
	pATADI->uSectorsPerTrack = pwData[6];
	pATADI->uCylinders = pwData[1];
	pATADI->uBytesPerSector = pwData[5];
	pATADI->uCapacity = pATADI->uCylinders * pATADI->uHeads * pATADI->uSectorsPerTrack * SECTOR_SIZE;
	pATADI->uSectors = pATADI->uCylinders * pATADI->uHeads * pATADI->uSectorsPerTrack;

	strcpy(pATADI->sModel, this->WordToText(pwData, 27, 46));
	CleanWhiteSpaces(pATADI->sModel);

	strcpy(pATADI->sSerialNumber, this->WordToText(pwData, 10, 19));
	CleanWhiteSpaces(pATADI->sSerialNumber);

	strcpy(pATADI->sFirmwareRev, this->WordToText(pwData,23,26));
	CleanWhiteSpaces(pATADI->sFirmwareRev);

	Mem->Free(pwData);

	/*
    printf("  Bytes/Index Gap: ");
    printf("%u\n", wATADI[7]>>8);
    printf("       Cylinders: ");
    printf("%-12u", wATADI[1]);
    printf("        Bytes/PLO: ");
    printf("%u\n", wATADI[8]&0xFF);
    printf("           Heads: ");
    printf("%-12u", wATADI[3] & 0xFF);
    printf("    Buffer Scheme: ");

    switch (wATADI[20]) {
	case 1:	    printf("Single Port Single Sector\n"); break;
	case 2:	    printf("Dual Port Multisector\n"); break;
	case 3:	    printf("Dual Port Multisector Cache\n"); break;
	default:    printf("Not specified\n"); break;
    }

    printf("     Bytes/Track: ");
    printf("%-12u", wATADI[4]);
    printf("    Sector Buffer: ");
    printf("%u", wATADI[21]);
    printf(" (");
    printf("%u", wATADI[21]/2);
    printf("Kb cache)\n");
    printf("    Bytes/Sector: ");
    printf("%-12u", wATADI[5]);
    printf("        Bytes/ECC: ");
    printf("%u\n", wATADI[22]);
    printf("   Sectors/Track: ");
    printf("%-12u", wATADI[6]);
    printf(" Sectors/Transfer: ");
    printf("%u", wATADI[47]);
    printf(" (READ command)\n");
    printf("       Bytes/ISG: ");
    printf("%-12u", wATADI[7]);
    printf("         Capacity: ");
    printf("%lu", ((uint) wATADI[1])*wATADI[3]*wATADI[6]/2048);
    printf("Mb (");
    printf("%lu", ((uint) wATADI[1])*wATADI[3]*wATADI[6]*SECTOR_SIZE);
    printf(" bytes)\n");
    printf("     Double Word: ");
    printf("%-12s", wATADI[48]?"Capable":"Not Capable");
    printf("  Defect Realloc.: ");
    printf("%s\n\n", wATADI[49]?"Yes":"No");
    printf("General Configuration\n\n");
    printf("         Sectoring: ");
    switch (wATADI[0] & 0x0006) {
	case 0x0000:    printf("%-15s", "Unavailable"); break;
	case 0x0002:    printf("%-15s", "Hard"); break;
	case 0x0004:    printf("%-15s", "Soft"); break;
    }

    printf("      Transfer Rate: ");
    switch (wATADI[0] & 0x0700) {
	case 0x0000:    printf("Unavailable\n"); break;
	case 0x0100:    printf("\xF3 5Mhz\n"); break;
	case 0x0200:    printf("> 5Mhz and \xF3 10Mhz\n"); break;
	case 0x0300:    printf("Unavailable\n"); break;
	case 0x0400:    printf("> 10Mhz\n"); break;
	case 0x0500:    printf("Unavailable\n"); break;
	case 0x0600:    printf("> 5Mhz to > 10Mhz (ZBR)\n"); break;
	case 0x0700:    printf("Unavailable\n"); break;
    }

    printf("          Encoding: ");
    switch (wATADI[0] & 0x0008) {
	case 0x0000:    printf("%-15s", "MFM"); break;
	case 0x0008:    printf("%-15s", "RLL"); break;
    }

    printf("   Rotational Speed: ");
    switch (wATADI[0] & 0x0800) {
	case 0x0000:    printf("< 0.5%% Tolerance\n"); break;
	case 0x0800:    printf("> 0.5%% Tolerance\n"); break;
    }

    printf("       Head Switch: ");
    switch (wATADI[0] & 0x0010) {
	case 0x0000:    printf("%-15s", "< 15\xE6s"); break;
	case 0x0010:    printf("%-15s", "> 15\xE6s"); break;
    }

    printf(" Data Strobe Offset: ");
    switch (wATADI[0] & 0x1000) {
	case 0x0000:    printf("Not available\n"); break;
	case 0x1000:    printf("Available\n"); break;
    }

    printf("   Spindle Control: ");
    switch (wATADI[0] & 0x0020) {
	case 0x0000:    printf("%-15s", "No"); break;
	case 0x0020:    printf("%-15s", "Yes"); break;
    }

    printf("       Track Offset: ");
    switch (wATADI[0] & 0x2000) {
	case 0x0000:    printf("Not available\n"); break;
	case 0x2000:    printf("Available\n"); break;
    }

    printf("             Media: ");
    switch (wATADI[0] & 0x00C0) {
	case 0x0000:    printf("%-15s", "Unavailable"); break;
	case 0x0040:    printf("%-15s", "Fixed"); break;
	case 0x0080:    printf("%-15s", "Removable"); break;
	case 0x00C0:    printf("%-15s", "Unavailable"); break;
    }

    printf("  Frmt Spd Tol. Gap: ");
    switch (wATADI[0] & 0x4000) {
	case 0x0000:    printf("Not required\n"); break;
	case 0x4000:    printf("Required\n"); break;
    }

    printf("                                             Recording: ");
    switch (wATADI[0] & 0x8000) {
	case 0x0000:    printf("Magnetic\n"); break;
	case 0x8000:    printf("Non-Magnetic\n"); break;
    }
	*/

	return true;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////

const char *CATA::WordToText(const word *pwData, int iStart, int iEnd)
{
    static char sText[SECTOR_SIZE];
	int iTextIndex = 0;

    for(int iDatIndex = iStart; iDatIndex <= iEnd; iDatIndex++)
	{
		sText[iTextIndex++] = (char) (pwData[iDatIndex] >> 8);
		sText[iTextIndex++] = (char) (pwData[iDatIndex] & 255);
    }

    sText[iTextIndex] = '\0';

    return sText;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool CATA::Initialize(void)
{
	memset(&this->ADI, 0, sizeof(ATAATTACHEDINFO));
	this->ADI.uMaxChannels = 2;
	this->ADI.Channel = (ATACHANNELINFO *) calloc(sizeof(ATACHANNELINFO), this->ADI.uMaxChannels);
	this->ADI.Channel[0].Disk = (ATADISKINFO *) calloc(sizeof(ATADISKINFO), ATA_DISKS_PER_CHANNEL);
	this->ADI.Channel[1].Disk = (ATADISKINFO *) calloc(sizeof(ATADISKINFO), ATA_DISKS_PER_CHANNEL);

	outportb(0x1F3, ATA_MAGIC_NUMBER); //Primary Controller.
    if(inportb(0x1F3) == ATA_MAGIC_NUMBER)
    {
		this->ADI.uChannels++;

		outportb(0x1F6, 0xA0); // use 0xA0 to test the master drive on the controller
        Sleep(1); //Need to sleep 1/100th of a second.
		if(inportb(0x1F7) &0x40) // read the status port, see if the busy bit is set
		{
			this->ADI.Channel[0].uMaxDisks++;
			if(this->GetDiskInfo(0, 0, &this->ADI.Channel[0].Disk[0]))
			{
				this->ADI.uDisks++;
				this->ADI.Channel[0].uDisks++;
				this->ADI.Channel[0].bPresent = true;
			}
		}

        outportb(0x1F6, 0xB0); // use 0xB0 to test the slave drive on the controller
        Sleep(1); //Need to sleep 1/100th of a second.
        if(inportb(0x1F7) &0x40) // read the status port, see if the busy bit is set
		{			
			this->ADI.Channel[0].uMaxDisks++;
			if(this->GetDiskInfo(0, 1, &this->ADI.Channel[0].Disk[1]))
			{
				this->ADI.uDisks++;
				this->ADI.Channel[0].uDisks++;
				this->ADI.Channel[0].bPresent = true;
			}
		}
    }

    outportb(0x173, ATA_MAGIC_NUMBER); //Secondary Controller.
    if(inportb(0x173) == ATA_MAGIC_NUMBER)
    {
		outportb(0x176, 0xA0); // use 0xA0 to test the master drive on the controller
        Sleep(1); //Need to sleep 1/100th of a second.
		if(inportb(0x177) &0x40) // read the status port, see if the busy bit is set
		{
			this->ADI.Channel[1].uMaxDisks++;
			if(this->GetDiskInfo(1, 0, &this->ADI.Channel[1].Disk[0]))
			{
				this->ADI.uDisks++;
				this->ADI.Channel[1].uDisks++;
				this->ADI.Channel[1].bPresent = true;
			}
		}

        outportb(0x176, 0xB0); // use 0xB0 to test the slave drive on the controller
        Sleep(1); //Need to sleep 1/100th of a second.
		if(inportb(0x177) &0x40) // read the status port, see if the busy bit is set
		{
			this->ADI.Channel[1].uMaxDisks++;
			if(this->GetDiskInfo(1, 1, &this->ADI.Channel[1].Disk[1]))
			{
				this->ADI.uDisks++;
				this->ADI.Channel[1].uDisks++;
				this->ADI.Channel[1].bPresent = true;
			}
		}

		this->ADI.uChannels++;
	}

    return true;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////

ATADISKINFO *CATA::Disk(ushort uChannel, ushort uDisk)
{
	if(uChannel < this->ADI.uChannels)
	{
		if(this->ADI.Channel[uChannel].bPresent)
		{
			if(uDisk < this->ADI.Channel[uChannel].uMaxDisks)
			{
				if(this->ADI.Channel[uChannel].Disk[uDisk].bPresent)
				{
					return &this->ADI.Channel[uChannel].Disk[uDisk];
				}
			}
		}
	}
	return NULL;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/*
	A call to this proc must be freed with DestroyAttachedATADeviceInfo(...);
*/
void CATA::GetADI(ATAATTACHEDINFO *pATAADI)
{
	memcpy(pATAADI, &this->ADI, sizeof(ATAATTACHEDINFO));

	pATAADI->Channel = (ATACHANNELINFO *) calloc(sizeof(ATACHANNELINFO), this->ADI.uMaxChannels);

	for(ushort uChannel = 0; uChannel < this->ADI.uMaxChannels; uChannel++)
	{
		memcpy(&pATAADI->Channel[uChannel],
			&this->ADI.Channel[uChannel],
			sizeof(ATACHANNELINFO));

		pATAADI->Channel[uChannel].Disk
			= (ATADISKINFO *) calloc(sizeof(ATADISKINFO), ATA_DISKS_PER_CHANNEL);

		for(ushort uDisk = 0; uDisk < ATA_DISKS_PER_CHANNEL; uDisk++)
		{
			memcpy(&pATAADI->Channel[uChannel].Disk[uDisk],
				&this->ADI.Channel[uChannel].Disk[uDisk],
				sizeof(ATADISKINFO));
		}
	}
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void CATA::DestroyADI(ATAATTACHEDINFO *pATAADI)
{
	for(ushort uChannel = 0; uChannel < pATAADI->uMaxChannels; uChannel++)
	{
		Mem->Free(pATAADI->Channel[uChannel].Disk);
	}

	Mem->Free(pATAADI->Channel);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////

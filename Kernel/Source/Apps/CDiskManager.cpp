////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#include "../Lib/STDLib.H"
#include "../Lib/String.H"

#include "../Drivers/CATA.H"
#include "../Drivers/CSFS.H"
#include "../Drivers/System.H"
#include "../Drivers/Timer.h"
#include "../Drivers/Video.h"
#include "../Drivers/CKeyboard.H"
#include "../Drivers/CMemory.h"
#include "../Drivers/Window.h"

#include "CDiskManager.h"
#include "CShell.h"

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void CDiskManager::ClearWindow(void)
{
	SetTextColor(WHITE, BLUE);
	Cls();
	DrawParentWindow("Disk Manager");
	GotoXY(1, 4);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void CDiskManager::Alert(const char *sText)
{
	ClearLine(1, VideoProps.iMaxHeight - 2, VideoProps.iMaxWidth - 2);
	GotoXY(3, VideoProps.iMaxHeight - 2);
	SetTextColor(RED, BLUE);
	KBD->Pause(sText);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/*
	Select a controller channel.
*/
ushort CDiskManager::SelectController(void)
{
	this->ClearWindow();

	char sSize[64];
	ATAATTACHEDINFO ADI;
	ATA->GetADI(&ADI);

	ushort iY = 5;

	printf(2, iY++, "Select a Controller:");

	iY++;

	for(ushort uChannel = 0; uChannel < ADI.uChannels; uChannel++)
	{
		printf(4, iY++, "Controller(%u): [%u devices]", uChannel, ADI.Channel[uChannel].uDisks);

		for(ushort uDisk = 0; uDisk < ADI.Channel[uChannel].uMaxDisks; uDisk++)
		{
			if(ADI.Channel[uChannel].Disk[uDisk].bPresent)
			{
				FriendlySize(ADI.Channel[uChannel].Disk[uDisk].uCapacity, sSize, sizeof(sSize), 2);
				printf(8, iY++, "Disk(%u): %s", uDisk, sSize);
				printf(28, iY - 1, "%s", ADI.Channel[uChannel].Disk[uDisk].sModel);
			}
		}
		iY++;
	}

	printf(4, iY++, "X) Exit");
	printf(2, ++iY, "Option: ");

	VIDEOSNAPSHOT *pSnap = TakeVideoSnapshot();

	while(true)
    {
        if(KBD->GetText(this->sCmd, this->iMaxCmdSize, &this->iCmdSize))
		{
			if(IsNumeric(this->sCmd))
			{
				ushort uChannel = atoi(this->sCmd);
				if(uChannel < ADI.uChannels)
				{
					if(ADI.Channel[uChannel].bPresent)
					{
						DeleteVideoSnapshot(pSnap);
						return uChannel;
					}
					else {
						this->Alert("Controller is no present, press any key to continue.");
					}
				}
				else {
					this->Alert("Invalid controller, press any key to continue.");
				}
			}
			else if(strcmpi(this->sCmd, "X") == 0)
			{
				break;
			}
			else{
				this->Alert("Invalid controller, press any key to continue.");
			}

			RestoreVideoSnapshot(pSnap);
		}
	}

	DeleteVideoSnapshot(pSnap);

	return 255;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/*
	Select a disk on a given controller channel.
*/
ushort CDiskManager::SelectDisk(ushort uChannel)
{
	this->ClearWindow();

	char sSize[64];
	ATAATTACHEDINFO ADI;
	ATA->GetADI(&ADI);

	ushort iY = 5;

	printf(2, iY++, "Select a Disk:");

	iY++;

	printf(4, iY++, "Controller(%u): [%u devices]", uChannel, ADI.Channel[uChannel].uDisks);

	for(ushort uDisk = 0; uDisk < ADI.Channel[uChannel].uMaxDisks; uDisk++)
	{
		if(ADI.Channel[uChannel].Disk[uDisk].bPresent)
		{
			FriendlySize(ADI.Channel[uChannel].Disk[uDisk].uCapacity, sSize, sizeof(sSize), 2);
			printf(8, iY++, "Disk(%u): %s", uDisk, sSize);
			printf(28, iY - 1, "%s", ADI.Channel[uChannel].Disk[uDisk].sModel);
		}
	}
	iY++;

	printf(4, iY++, "X) Exit");
	printf(2, ++iY, "Option: ");

	VIDEOSNAPSHOT *pSnap = TakeVideoSnapshot();

	while(true)
    {
        if(KBD->GetText(this->sCmd, this->iMaxCmdSize, &this->iCmdSize))
		{
			if(IsNumeric(this->sCmd))
			{
				ushort uDisk = atoi(this->sCmd);
				if(uDisk < ADI.Channel[uChannel].uMaxDisks)
				{
					if(ADI.Channel[uChannel].Disk[uDisk].bPresent)
					{
						DeleteVideoSnapshot(pSnap);
						return uDisk;
					}
					else {
						this->Alert("Disk is no present, press any key to continue.");
					}
				}
				else {
					this->Alert("Invalid disk, press any key to continue.");
				}
			}
			else if(strcmpi(this->sCmd, "X") == 0)
			{
				break;
			}
			else{
				this->Alert("Invalid disk, press any key to continue.");
			}

			RestoreVideoSnapshot(pSnap);
		}
	}

	ATA->DestroyADI(&ADI);
	DeleteVideoSnapshot(pSnap);

	return 255;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

typedef struct _tag_FormatCallBackData {
	ushort uLeft;
	ushort uTop;
	ushort uWidth;
	ushort uHeight;
	ushort uLastStatus;

	ushort uLastX;
	ushort uLastY;
} DISKMANFORMATCALLBACKDATA, *LPDISKMANFORMATCALLBACKDATA;

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void DiskManSFSFormatProgressCallback(SFSENUMDISK *pDisk, float fPercentComplete, void *pUserData)
{
	DISKMANFORMATCALLBACKDATA *pCBD = (DISKMANFORMATCALLBACKDATA *)pUserData;

	uint iStatus = (int)((pCBD->uWidth - 2) * (fPercentComplete / 100.0));
	if(iStatus > pCBD->uLastStatus)
	{
		GotoXY(pCBD->uLastX, pCBD->uLastY);
		PutCh((uchar)178);
		GetXY(&pCBD->uLastX, &pCBD->uLastY);
		pCBD->uLastStatus = iStatus;
	}

	FillLine(pCBD->uLeft + 1, pCBD->uLastY + 1, ' ', pCBD->uWidth - 2);

	GotoXY((pCBD->uLeft + (pCBD->uWidth / 2)) - 3, pCBD->uLastY + 1);
	printf("%.2f%%", fPercentComplete);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void DiskManATAFormatProgressCallback(ATADISKINFO *pDisk, float fPercentComplete, void *pUserData)
{
	DISKMANFORMATCALLBACKDATA *pCBD = (DISKMANFORMATCALLBACKDATA *)pUserData;

	uint iStatus = (int)((pCBD->uWidth - 2) * (fPercentComplete / 100.0));
	if(iStatus > pCBD->uLastStatus)
	{
		GotoXY(pCBD->uLastX, pCBD->uLastY);
		PutCh((uchar)178);
		GetXY(&pCBD->uLastX, &pCBD->uLastY);
		pCBD->uLastStatus = iStatus;
	}

	FillLine(pCBD->uLeft + 1, pCBD->uLastY + 1, ' ', pCBD->uWidth - 2);

	GotoXY((pCBD->uLeft + (pCBD->uWidth / 2)) - 3, pCBD->uLastY + 1);
	printf("%.2f%%", fPercentComplete);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void CDiskManager::DisplayDiskDetail(ushort uChannel, ushort uDisk)
{
	this->ClearWindow();

	char sSize[64];
	ATAATTACHEDINFO ADI;
	ATA->GetADI(&ADI);

	ushort iY = 5;


	printf(4, iY++, "Controller: %u, Disk: %u", uChannel, uDisk);

	iY++;
	printf(8, iY++, "        Model: %s", ADI.Channel[uChannel].Disk[uDisk].sModel);
	printf(8, iY++, "Serial Number: %s", ADI.Channel[uChannel].Disk[uDisk].sSerialNumber);
	printf(8, iY++, " Firmware Rev: %s", ADI.Channel[uChannel].Disk[uDisk].sFirmwareRev);

	iY++;
	printf(8, iY++, "        Heads: %u", ADI.Channel[uChannel].Disk[uDisk].uHeads);
	printf(8, iY++, "Sectors/Track: %u", ADI.Channel[uChannel].Disk[uDisk].uSectorsPerTrack);
	printf(8, iY++, " Bytes/Sector: %u", ADI.Channel[uChannel].Disk[uDisk].uBytesPerSector);
	printf(8, iY++, "    Cylinders: %u", ADI.Channel[uChannel].Disk[uDisk].uCylinders);
	
	FriendlySize(ADI.Channel[uChannel].Disk[uDisk].uCapacity, sSize, sizeof(sSize), 2);
	printf(8, iY++, "     Capacity: %s", sSize);

	ATA->DestroyADI(&ADI);

	iY += 2;

	printf(2, iY++, "Select an operation:");
	printf(4, iY++, "1) Format disk");
	printf(4, iY++, "2) Zero disk data");
	printf(4, iY++, "X) Exit");
	printf(2, ++iY, "Option: ");

	VIDEOSNAPSHOT *pSnap = TakeVideoSnapshot();

	while(true)
    {
        if(KBD->GetText(this->sCmd, this->iMaxCmdSize, &this->iCmdSize))
		{
			if(strcmpi(this->sCmd, "1") == 0)
			{
				char sTitle[64];
				char sSize[64];
				FriendlySize(ATA->Disk(uChannel, uDisk)->uCapacity, sSize, sizeof(sSize), 2);
				sprintf(sTitle, "Formatting %s disk", sSize);

				HideCursor();

				DISKMANFORMATCALLBACKDATA CBD;
				CBD.uLeft = 10;
				CBD.uTop = 10;
				CBD.uWidth = 60;
				CBD.uHeight = 4;
				CBD.uLastStatus = 0;

				DrawWindow(sTitle, CBD.uLeft, CBD.uTop, CBD.uWidth, CBD.uHeight, (uchar)176);

				GetXY(&CBD.uLastX, &CBD.uLastY);

				SFS->FormatDisk(uChannel, uDisk, DiskManSFSFormatProgressCallback, &CBD);

				ShowCursor();

			}
			else if(strcmpi(this->sCmd, "2") == 0)
			{
				char sTitle[64];
				char sSize[64];
				FriendlySize(ATA->Disk(uChannel, uDisk)->uCapacity, sSize, sizeof(sSize), 2);
				sprintf(sTitle, "Zeroing %s disk data", sSize);

				HideCursor();

				DISKMANFORMATCALLBACKDATA CBD;
				CBD.uLeft = 10;
				CBD.uTop = 10;
				CBD.uWidth = 60;
				CBD.uHeight = 4;
				CBD.uLastStatus = 0;

				DrawWindow(sTitle, CBD.uLeft, CBD.uTop, CBD.uWidth, CBD.uHeight, (uchar)176);

				GetXY(&CBD.uLastX, &CBD.uLastY);

				ATA->ZeroDisk(uChannel, uDisk, DiskManATAFormatProgressCallback, &CBD);

				ShowCursor();

			}
			else if(strcmpi(this->sCmd, "X") == 0)
			{
				break;
			}
			else{
				this->Alert("Invalid operation, press any key to continue.");
			}

			RestoreVideoSnapshot(pSnap);
		}
	}

	DeleteVideoSnapshot(pSnap);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool CDiskManager::Run(void)
{
	this->ClearWindow();

	ushort iY = 5;
	printf(2, iY++, "Select an operation:");
	printf(4, iY++, "1) View physical disks");
	printf(4, iY++, "2) View partitions");
	printf(4, iY++, "3) Edit existing partition");
	printf(4, iY++, "4) Create new partition");
	printf(4, iY++, "5) Format disk or partition");
	printf(4, iY++, "6) Wipe disk or partition");
	printf(4, iY++, "X) Exit");
	printf(2, ++iY, "Option: ");

	VIDEOSNAPSHOT *pSnap = TakeVideoSnapshot();

	while(true)
    {
        if(KBD->GetText(this->sCmd, this->iMaxCmdSize, &this->iCmdSize))
		{
			if(strcmpi(this->sCmd, "1") == 0)
			{
				ushort uChannel = this->SelectController();
				if(uChannel != 255)
				{
					ushort uDisk = this->SelectDisk(uChannel);
					if(uDisk != 255)
					{
						this->DisplayDiskDetail(uChannel, uDisk);
					}
				}
			}
			else if(strcmpi(this->sCmd, "X") == 0)
			{
				break;
			}
			else{
				this->Alert("Invalid command, press any key to continue.");
			}

			RestoreVideoSnapshot(pSnap);
		}
	}

	SetTextColor(WHITE, BLACK);
	Cls();

	DeleteVideoSnapshot(pSnap);

	return true;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool CDiskManager::Initialize(void)
{
	this->iMaxCmdSize = (VideoProps.iMaxWidth - 12 + 1);

	if(!(this->sCmd = (char *)calloc(sizeof(char), this->iMaxCmdSize)))
	{
		return false;
	}

	return true;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool CDiskManager::Destroy(void)
{
	Mem->Free(this->sCmd);

	return true;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

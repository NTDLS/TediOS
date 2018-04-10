////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#ifndef _SHELL_C_
#define _SHELL_C_
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/*
    Console Shell Functions.
*/
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#include "../Lib/OS.H"
#include "../Lib/STDLib.H"
#include "../Lib/String.H"

#include "../Drivers/Video.H"
#include "../Drivers/GDT.H"
#include "../Drivers/IDT.H"
#include "../Drivers/IRQ.H"
#include "../Drivers/ISR.H"
#include "../Drivers/CKeyboard.H"
#include "../Drivers/CMemory.h"
#include "../Drivers/System.H"
#include "../Drivers/Time.h"
#include "../Drivers/Timer.h"
#include "../Drivers/CCPU.H"
#include "../Drivers/CATA.H"
#include "../Drivers/CSFS.H"
#include "../Drivers/Window.h"

#include "CSHA1.H"
#include "CHASHCol.h"
#include "CDiskManager.h"

#include "CShell.H"

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool CShell::Initialize(void)
{
	VisualSpin(2);
	return true;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void CShell::DisplayHelp(void)
{
    printf("\n");
    printf("Available commands:\n");
    //printf("\t Now        : Displays the current date & time.\n");
    printf("\t Date        : Displays the current date.\n");
    printf("\t Time        : Displays the current time.\n");
    printf("\t TickCount   : Reads the TickCount timer.\n");
    printf("\t Memory      : Displays basic memory information.\n");

    //printf("\t Help       : Displays this help listing.\n");
    printf("\t Ver         : Displays the OS name & version.\n");
    printf("\t Cls         : Clears the screen.\n");

    printf("\t @LastByte   : Searches for the last byte of RAM.\n");
    printf("\t @EnableFPU  : Enables the Floating Point Unit.\n");
    printf("\t @DisableFPU : Disables the Floating Point Unit.\n");

    printf("\t Hash        : Generates a valid SHA1 hash.\n");
    printf("\t Collide     : Performs reverse SHA1 hash lookup.\n");
    printf("\t DiskMan     : Launches the built in disk-manager.\n");
    printf("\t Format      : Formats a disk.\n");

    printf("\t Reboot      : Reboots the computer.\n");
    printf("\t Exit        : Safely ends your session.\n");

    printf("\n");
}


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void ShellSFSFormatProgressCallback(SFSENUMDISK *pDisk, float fPercentComplete, void *pUserData)
{
	printf("%.2f%%\t\r", fPercentComplete);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

int CShell::Run(void)
{
    char *sCommand = NULL;
    char *sLastCommand = NULL;
    int iCommandLen = 0;

	VIDEOSNAPSHOT *pVS;

	if(!(sCommand = (char *)calloc(sizeof(char), CONSOLE_CMD_SIZE + 1)))
	{
	}

	if(!(sLastCommand = (char *)calloc(sizeof(char), CONSOLE_CMD_SIZE + 1)))
	{
		Mem->Free(sCommand);
		return false;
	}

    printf("\n");

    while(true)
    {
        printf("> ");
        if(KBD->GetText(sCommand, CONSOLE_CMD_SIZE, &iCommandLen))
        {
            if((iCommandLen = Trim(sCommand)) > 0)
            {
				if(strnicmp(sCommand, "Format ", 7) == 0)
                {
					char sDisk[10];
					Trim(sCommand + 7);
					strncpy(sDisk, sCommand + 7, 9);
					if(sCommand[8] == ':')
					{
						SFSENUMDISK *pDisk = SFS->GetDisk(sDisk);
						if(pDisk)
						{
							char sSize[64];
							FriendlySize(pDisk->pATADisk->uCapacity, sSize, sizeof(sSize), 2);
							printf("\nFormatting %s disk...\n", sSize);
							SFS->FormatDisk(pDisk, &ShellSFSFormatProgressCallback, NULL);
							printf("Complete!");
						}
						else{
							printf("\nInvalid drive specification.");
						}
					}
					else{
						printf("\nInvalid drive specification.");
					}
				}
				else if(strcmpi(sCommand, "DiskMan") == 0)
                {
					pVS = TakeVideoSnapshot();
					CDiskManager DiskManager;
					if(DiskManager.Initialize())
					{
						DiskManager.Run();
					}
					DiskManager.Destroy();
					RestoreAndDeleteVideoSnapshot(pVS);
				}
				else if(strcmpi(sCommand, "Now") == 0)
                {
                    printf(" ");
                    PrintDate();
                    printf(" - ");
                    PrintTime();
                }
                else if(strcmpi(sCommand, "Date") == 0)
                {
                    PutCh(' ');
                    PrintDate();
                }
                else if(strcmpi(sCommand, "Time") == 0)
                {
                    PutCh(' ');
                    PrintTime();
                }
                else if(strcmpi(sCommand, "Memory") == 0)
                {
					char sSize[64];
					MEMORYPHYSICALINFO PMI;
					Mem->GetPhysicalInfo(&PMI);

                    printf("\nPhysical Information:\n");
                    printf("\tBase:            %s\n", FriendlySize(PMI.uBase, sSize, sizeof(sSize), 2));
                    printf("\tShadow:          %s\n", FriendlySize(PMI.uShadow, sSize, sizeof(sSize), 2));
                    printf("\tExtended:        %s\n", FriendlySize(PMI.uExtended, sSize, sizeof(sSize), 2));
                    printf("\tTotal:           %s\n", FriendlySize(PMI.uTotal, sSize, sizeof(sSize), 2));
                    printf("\tUnreserved:      %s\n", FriendlySize(PMI.uUnreserved, sSize, sizeof(sSize), 2));

					MEMORYMANAGERPAGES MMP;
					Mem->GetPageInfo(&MMP);
                    printf("\nManager Information:\n");
                    printf("\tTotal Pages:     %u\n", MMP.uCount);
                    printf("\tManager Pages:   %u\n", MMP.uManagerPages);
                    printf("\tAllocated Pages: %u\n", MMP.uUsed);
                    printf("\tPage Size:       %s\n", FriendlySize(MMP.uSize, sSize, sizeof(sSize), 2));

					MEMORYSTATUSINFO MSI;
					Mem->GetStatusInfo(&MSI);
					printf("\nStatus Information:\n");
                    printf("\tTotal:           %s\n", FriendlySize(MSI.uBytesTotal, sSize, sizeof(sSize), 2));
                    printf("\tUsed:            %s\n", FriendlySize(MSI.uBytesUsed, sSize, sizeof(sSize), 2));
                    printf("\tFree:            %s\n", FriendlySize(MSI.uBytesFree, sSize, sizeof(sSize), 2));
				}
                else if(strcmpi(sCommand, "@LastByte") == 0)
                {
                    printf("\n");
                    uint long uAddress = Mem->SearchForUpperLimit();
                    printf("Address: %u\n", uAddress);
                }
                else if(strcmpi(sCommand, "@EnableFPU") == 0)
                {
                    CPU->EnableFPU();
                }
                else if(strcmpi(sCommand, "@DisableFPU") == 0)
                {
                    CPU->DisableFPU();
                }
				else if(strcmpi(sCommand, "prints") == 0)
				{
					ATA->Debug();
                }
				else if(strcmpi(sCommand, "@List") == 0)
				{
					printf("\nFiles: %u", SFS->ListFiles("C:\\"));
                }
				else if(strcmpi(sCommand, "@CreateFiles") == 0)
				{
					printf("\n");
					char sFileName[255];
					for(uint uFile = 0; uFile < 300; uFile++)
					{
						FILEHANDLE *hFile = NULL;

						sprintf(sFileName, "C:\\Test_File_%d.txt", uFile);
						if((hFile = SFS->CreateFile(sFileName)))
						{
							printf("Files: %u\r", uFile);
							SFS->CloseFile(hFile);
						}
						else {
							BSOD("Failed to create file");
							break;
						}
					}
				}
				else if(strcmpi(sCommand, "Collide") == 0)
                {
                    CHASHCol MyHC;

                    unsigned char sUserInput[512];
                    unsigned char sHash[41];
                    int iInputSize = 0;

                    printf("\nText: ");
                    if(KBD->GetText((char *)sUserInput, sizeof(sUserInput), &iInputSize))
                    {
                        SimpleSHA1(sUserInput, iInputSize, sHash);
                        printf("\nHash: [%s], Length: %d\n", sHash, iInputSize);

                        if(MyHC.Initialize(sHash))
                        {
                            if(!MyHC.Run(5))
                            {
                                printf("No match!\n");
                            }
                        }
                    }
                }
                else if(strcmpi(sCommand, "Ver") == 0)
                {
					SetTextColor(BLUE, BLACK);
					printf(" %s", OS_NAME);
					SetTextColor(GREEN, BLACK);
					printf(" %s\n", OS_VERSION);
					SetTextColor(WHITE, BLACK);
                }
                else if(strcmpi(sCommand, "Hash") == 0)
                {
                    unsigned char sUserInput[512];
                    unsigned char sHash[41];
                    int iInputSize = 0;

                    printf("\nText: ");
                    if(KBD->GetText((char *)sUserInput, sizeof(sUserInput), &iInputSize))
                    {
                        printf(" [%d Characters]\n", iInputSize);
                        SimpleSHA1(sUserInput, iInputSize, sHash);
                        printf("Hash: [%s]\n", sHash);
                    }
                }
                else if(strcmpi(sCommand, "Cls") == 0)
                {
                    Cls();
                }
                else if(strcmpi(sCommand, "Reboot") == 0)
                {
                    printf(" - Please wait...\n");
                    Reboot();
					Hang();
                }
                else if(strcmpi(sCommand, "TickCount") == 0)
                {
                    printf(" is %d", GetTickCount());
                }
                else if(strcmpi(sCommand, "Exit") == 0)
                {
                    break;
                }
                else if(strcmpi(sCommand, "Help") == 0)
                {
                    DisplayHelp();
                }
                else{
                    printf(" - [%s] is an unknown command.", sCommand);
                }
    
                strcpy(sLastCommand, sCommand);
            }
            printf("\n");
        }
    }

	Mem->Free(sCommand);
	Mem->Free(sLastCommand);

    return true;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#endif

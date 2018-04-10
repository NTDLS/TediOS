////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#ifndef _Entry_CPP_
#define _Entry_CPP_
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/*
    C code entry.
*/
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#include "../Lib/OS.H"
#include "../Lib/STDLib.H"
#include "../Lib/String.H"

#include "../Drivers/CCPU.H"
#include "../Drivers/CATA.H"
#include "../Drivers/CSFS.H"
#include "../Drivers/CFAT.H"
#include "../Drivers/CFloppy.H"

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

#include "../Apps/CShell.H"

#include "../Apps/CDiskManager.h"

#include "MultiBoot.H"

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

int main(unsigned long ulMBIAddress, unsigned long ulMagic)
{
	StopInterrupts();

	if(InitializeVideo())
	{
		multiboot_info_t *pMBI = NULL;

		//Kernel booted by a Multiboot-compliant boot loader? 
		if(ulMagic == MULTIBOOT_BOOTLOADER_MAGIC)
		{
			pMBI = (multiboot_info_t *)ulMBIAddress;
		}

		INITSTATTEXT IST;

		SetTextColor(BLUE, BLACK);
		printf("%s", OS_NAME);
		SetTextColor(GREEN, BLACK);
		printf(" %s\n", OS_VERSION);
		SetTextColor(WHITE, BLACK);
		DrawHRule();

		BeginInitText(&IST, "CPU");
		EndInitText(&IST, CPU->Initialize(), true);

		BeginInitText(&IST, "Memory");
		EndInitText(&IST, Mem->Initialize(pMBI), true);

		BeginInitText(&IST, "Systems");
		CriticalInit("GDT", GDT_Install());
		CriticalInit("IDT", IDT_Install());
		CriticalInit("ISR", ISR_Install());
		CriticalInit("IRQ", IRQ_Install());
		CriticalInit("Timer", Timer_Install());

		KBD = (CKeyboard *) calloc(sizeof(CKeyboard), 1);
		CriticalInit("Keyboard", KBD->Initialize());
		EndInitText(&IST, true, true);

		StartInterrupts();

		Floppy = (CFloppy *) calloc(sizeof(CFloppy), 1);
		BeginInitText(&IST, "Floppy");
		EndInitText(&IST, Floppy->Initialize(), false);

		ATA = (CATA *) calloc(sizeof(CATA), 1);
		BeginInitText(&IST, "ATA");
		EndInitText(&IST, ATA->Initialize(), false);

		SFS = (CSFS *) calloc(sizeof(CSFS), 1);
		BeginInitText(&IST, "SFS");
		EndInitText(&IST, SFS->Initialize(), false);		
		
		///////////////////////////////////////////////////////////////////////////////////////////////////////////////////

        int iSectors = 1;

		unsigned char *sWriteText = (unsigned char *) calloc(sizeof(char), (SECTOR_SIZE * iSectors) + 1);
		unsigned char *sReadText = (unsigned char *) calloc(sizeof(char), (SECTOR_SIZE * iSectors) + 1);

		for(int iSector = 64; iSector < 1024 * 100; iSector += iSectors)
		{
    		memset(sWriteText, (char)(iSector & 255), (SECTOR_SIZE * iSectors));
            sWriteText[(SECTOR_SIZE * iSectors)] = '\0';

            if(!ATA->WriteSectors(0, 0, iSector, iSectors, sWriteText))
			{
    			printf("Write failed\n");
		  	}

    		memset(sReadText, 0, (SECTOR_SIZE * iSectors));
			if(ATA->ReadSectors(0, 0, iSector, iSectors, sReadText))
			{
                sReadText[(SECTOR_SIZE * iSectors)] = '\0';
                if(strcmp((char *)sReadText, (char *)sWriteText) != 0)
                {
                    for(int i = 0; i < (SECTOR_SIZE * iSectors); i++)
                    {
                        if(sReadText[i] != sWriteText[i])
                        {
                            printf("%d: %d vs %d\n", i, sReadText[i], sWriteText[i]);
                        }
                    }

    				printf("Read %d is wrong: [%d] vs [%d]\n",
                        iSector, strlen((char *)sReadText), strlen((char *)sWriteText));

        			KBD->Pause();
                }
			}
			else {
				printf("Read failed: [%s]\n", sReadText);
    			KBD->Pause();
            }

			//printf("%d: [%s]\n", iSector, sReadText);
			//KBD->Pause();
		}

		//FAT = (CFAT *) calloc(sizeof(CFAT), 1);
		//BeginInitText(&IST, "FAT");
		//EndInitText(&IST, FAT->Initialize(), false);

		///////////////////////////////////////////////////////////////////////////////////////////////////////////////////

		CShell Shell;
		BeginInitText(&IST, "Shell");
		if(EndInitText(&IST, Shell.Initialize(), true))
		{
			PutCh('\n');
			DrawHRule();
			PutCh('\n');
			Shell.Run();
		}
	}


	StopInterrupts();
	Reboot();

	SetTextColor(WHITE, BLACK);
	Cls();
	printf("It is now safe to shutdown your computer...\n");

	Hang();

	return 0;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#endif

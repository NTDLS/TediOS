///////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#include "../Lib/STDLib.H"

#include "CFloppy.H"
#include "System.H"

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////

CFloppy *Floppy = NULL;

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool CFloppy::Initialize(void)
{
    outportb(0x70, 0x10);
    byte bFloppyFlags = inportb(0x71);

    char *sFloppyDriveTypes[6] =
    {
        "Not Present",
        "360KB",  // 5.25\"",
        "1.2MB",  // 5.25\"",
        "720KB",  // 3.5\"",
        "1.44MB", // 3.5\"",
        "2.88MB", // 3.5\""
    };

	int iDrive0 = bFloppyFlags >> 4; //High nibble.
	int iDrive1 = bFloppyFlags &0xF; //Low nibble by ANDing out the High nibble.

	if(iDrive0 || iDrive1)
	{
	    printf("     (");
	}

	if(iDrive0)
	{
		printf("0:%s", sFloppyDriveTypes[iDrive0]); 
		if(iDrive1)
		{
			printf(", ");
		}
	}

	if(iDrive1)
	{
		printf("1:%s", sFloppyDriveTypes[iDrive1]); 
	}

	if(iDrive0 || iDrive1)
	{
	    printf(")");
	}

    printf("\n");

	return true;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool CFloppy::Destroy()
{
	return true;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////

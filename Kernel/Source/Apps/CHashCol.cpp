#ifndef _CHASHCOL_CPP_
#define _CHASHCOL_CPP_
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

//abc = a9993e364706816aba3e25717850c26c9cd0d89d

#include "../Lib/STDLib.H"

#include "../Drivers/System.H"
#include "../Drivers/CKeyboard.H"
#include "../Drivers/Window.h"

#include "CHASHCol.h"
#include "CSHA1.h"

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool CHASHCol::GenerateStrings(ushort iLength)
{
	unsigned char sString[65];

	unsigned char iCharacter = 0;
	unsigned char iMinCharacter = this->CI.iMinChar;
	unsigned char iMaxCharacter = this->CI.iMaxChar;
	unsigned char sDigest[20];
	uint iuPart = 0;

	bool bWorkloadComplete = false;

    memset(sString, 0, sizeof(sString));

    SHA1_CTX Context;

    KBD->KillCommand(false);

	//Process workloads (BEGIN).
	while(true)
	{
		if(iCharacter++ > iMaxCharacter) //Get the next workload.
		{
            printf("No workload remaining for %d length load.\n", iLength);
			break; //No workload remaining.
		}

		printf("Consuming workload %d->0x%X.\r", iLength, iCharacter);

		bWorkloadComplete = false;
		memset(sString, iCharacter, iLength);
		sString[iLength] = '\0';

		while(!bWorkloadComplete && !KBD->KillCommand())
		{
			//Key Usage (BEGIN).
            SHA1Init(&Context);
            SHA1Update(&Context, sString, iLength);
            SHA1Final(sDigest, &Context);

			for(iuPart = 0; iuPart < 20; iuPart++)
			{
				if(sDigest[iuPart] != this->CI.sGoalDigest[iuPart])
				{
					break;
				}
				else{
					if(iuPart == 19)
					{
						printf("\nFound perfect match: [%s]\n", sString);
                        return true;
					}
				}
			}
			//Key Usage (END).

			sString[iLength - 1]++;

			for(int iPos = (iLength - 1); sString[iPos] > iMaxCharacter && iPos != -1; iPos--)
			{
				if(iPos == 0)
				{
					bWorkloadComplete = true;
					break;
				}

				sString[iPos - 1]++;
				sString[iPos] = iMinCharacter;
			}

			for(int iPos = (iLength - 1); iPos != -1; iPos--)
			{
				if(sString[iPos] != iCharacter + 1)
				{
					break;
				}
				if(iPos == 0)
				{
					bWorkloadComplete = true;
					break;
				}
			}
		}
	}

	return false;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool CHASHCol::Run(ushort iMax)
{
    for(ushort iLen = 1; iLen < iMax; iLen++)
    {
        if(this->GenerateStrings(iLen))
        {
            return true;
        }
    }
    return false;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool CHASHCol::Initialize(const unsigned char *sHash)
{
    int iHashLength = strlen((const char *)sHash);

    if(iHashLength != 40)
    {
        printf("The hash length must be 40 characters, you supplied %d.\n", iHashLength);
        return false;
    }

	//Setup the configuration defaults.
	memset(&this->CI, 0, sizeof(CONFIGINFO));
	this->CI.iLen = 1;
	this->CI.iMaxLen = 64;
	this->CI.iMinChar = 32;
	this->CI.iMaxChar = 126;

	char sPart[9];
	int iRPos = 0;
	memset(sPart, 0, sizeof(sPart));

    //Copy the supplied hash.
	strcpy((char *)this->CI.sHash, (char *)sHash);

    //Break the hash into 5 uintegers, the goal digest.
	for(int iPart = 0; iPart < 20; iPart++)
	{
		for(int iWPos = 0; iWPos < 2; iWPos++)
		{
			sPart[iWPos] = CI.sHash[iRPos++];
		}
		this->CI.sGoalDigest[iPart] = HexToDec(sPart);
	}

    return true;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#endif


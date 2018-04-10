////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#ifndef _BEEP_C_
#define _BEEP_C_
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/*
    System speaker functions.
*/
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#include "../Lib/STDLib.H"

#include "Speaker.H"
#include "System.H"

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void Sound(uint ulFrequency)
{
	// Sending magic byte.
	outportb(0x43, SOUND_MAGIC);

	// writing frequency.
	ulFrequency = 120000L/ulFrequency;
	outportb(0x42, ulFrequency>>8);
	outportb(0x42, (ulFrequency<<8)>>8);

	// Sound on.
	outportb(0x61, inportb(0x61)|3 );
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void NoSound(void)
{
    outportb(0x61, inportb(0x61) & ~3);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void Beep(void)
{
	Sound(1500);
    //delay(100);
	NoSound();
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#endif
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#ifndef _MATH_C_
#define _MATH_C_
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/*
    System timer installer and handlers.
*/
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#include "Math.H"

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

int BCD2BIN(byte bcd)
{
	return((bcd & 0xF) + ((bcd >> 4) * 10));
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

int labs(int iValue)
{
	return iValue >= 0 ? iValue : -iValue;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#endif
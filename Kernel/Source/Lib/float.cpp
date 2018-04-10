///////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#include "Types.H"
#include "Float.H"
#include "StdLib.H"
#include "String.H"

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////

double ceil(double x)
{
	double val;
	return modf(x, &val) > 0 ? val + 1.0 : val;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////

double floor(double x)
{
	volatile short cw, cwtmp;

	__asm__ volatile ("fnstcw %0" : "=m" (cw) : );
	/* rounding down */
	cwtmp = (cw & 0xf3ff) | 0x0400;
	__asm__ volatile ("fldcw %0" : : "m" (cwtmp));
	/* x = floor of x */
	__asm__ volatile ("frndint" : "=t" (x) : "0" (x));
	__asm__ volatile ("fldcw %0" : : "m" (cw));
	return (x);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////

double modf(double value, double *iptr)
{
	register double ipart = floor(value);
	*iptr = ipart;
	return value - ipart;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

int dlen(double x)
{
	int iLength = 0;
	do {
		iLength++;
		x /= 10;
	} while(x >= 1);
	return iLength;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

int ftoa(double dValue, ushort iDeciPlaces, char *sOut)
{
	char sPart[64];
	unsigned short iRPos = 0;
	unsigned short iWPos = 0;
	double dIntPart = 0;
	double dFraction = modf(dValue, &dIntPart);

	if(iDeciPlaces > 0)
	{
		//Multiply the decimal part by (10 to the power of [iDeciPlaces]).
		for(ushort iDec = 0; iDec < iDeciPlaces; iDec++)
		{
			dFraction *= 10.0;
		}

		double dIntPart2 = 0;
		double dFraction2 =  modf(dFraction, &dIntPart2); //Split the new decimal part.

		//If the fraction part of the raised fraction is greater
		//	than 0.5, then add 1 to the faction part we use for
		//	building the final result string.
		if(dFraction2 >= 0.5)
		{
			dIntPart2++;

			//If we gained an "int-part" decimal place by adding one then add 1
			//	to the integer part we use for building the final result string.
			if(dlen(dFraction) != dlen(dIntPart2))
			{
				dIntPart++;
				dFraction = 0;
			}
			else{
				dFraction = dIntPart2;
			}
		}
	}
	else{
		//No decimal places to be used, just check
		//	the faction to see if rounding is necessary.
		if(dFraction > 0.5)
		{
			dIntPart++;
		}
	}

	//Start building the string: Add the int part.
	unsigned short iLength = itoa((int)dIntPart, sPart);
	for(iRPos = 0; iRPos < iLength; iRPos++)
	{
		sOut[iWPos++] = sPart[iRPos];
	}

	//Add the decimal part.
	if(iDeciPlaces > 0)
	{
		sOut[iWPos++] = '.';

		iLength = itoa((int)dFraction, sPart);
		for(iRPos = 0; iRPos < iDeciPlaces; iRPos++)
		{
			if(iRPos < iLength)
			{
				sOut[iWPos++] = sPart[iRPos];
			}
			else{
				sOut[iWPos++] = '0';
			}
		}
	}

	sOut[iWPos] = '\0';

	return iWPos;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

double fabs(double dValue)
{
	return dValue >= 0 ? dValue : -dValue;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////

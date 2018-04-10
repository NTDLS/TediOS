#ifndef _SPRINTF_C
#define _SPRINTF_C

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#include "STDLib.H"
#include "SPrintF.H"
#include "Float.H"

#include "../Drivers/Video.H"
#include "../Drivers/System.H"
#include "../Drivers/Timer.h"

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

int SPrintfEx(char *sBuffer, const char *sFormat, va_list args)
{
	int iWPos = 0;

	ushort iDecPlaces = 0;

	for(; *sFormat; sFormat++)
	{
		if(*sFormat != '%')
		{
			sBuffer[iWPos++] = *sFormat;
		}
		else{
			*sFormat++;

			if(*sFormat == '%')
			{
				sBuffer[iWPos++] = *sFormat;
				continue;
			}
			else{
				if(*sFormat == '.')
				{
					char sDec[16];
					int iPos = 0;

					*sFormat++; //Skip the decimal.

					while(isdigit(*sFormat))
					{
						sDec[iPos++] = *sFormat++;
					}
					sDec[iPos] = '\0';
					iDecPlaces = atoi(sDec);
				}
				else{
					iDecPlaces = 8;
				}

				if(*sFormat == 's')
				{
					for(char *sVal = va_arg(args, char *); *sVal; *sVal++)
					{
						sBuffer[iWPos++] = *sVal;
					}
				}
				else if(*sFormat == 'd' || *sFormat == 'l')
				{
					char sValue[16];
					itoa(va_arg(args, int), sValue);

					for(char *sVal = sValue; *sVal; *sVal++)
					{
						sBuffer[iWPos++] = *sVal;
					}
				}
				else if(*sFormat == 'u')
				{
					char sValue[16];
					utoa(va_arg(args, uint), sValue);

					for(char *sVal = sValue; *sVal; *sVal++)
					{
						sBuffer[iWPos++] = *sVal;
					}
				}
				else if(*sFormat == 'f')
				{
					char sValue[255];
					ftoa(va_arg(args, double), iDecPlaces, sValue);
	
					for(char *sVal = sValue; *sVal; *sVal++)
					{
						sBuffer[iWPos++] = *sVal;
					}
				}
				else if(*sFormat == 'c')
				{
					sBuffer[iWPos++] = va_arg(args, char);
				}
			}
		}
	}

	sBuffer[iWPos] = '\0';

	return iWPos;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

/*
int SPrintfEx(char *Buffer, const char *fmt, va_list args)
{
	unsigned state, flags, radix, actual_wd, count, given_wd;
    int Pos = 0;
	char *where, buf[PR_BUFLEN];
	long num;
	double dub;

	state = flags = count = given_wd = 0;
    // begin scanning format specifier list 
	for(; *fmt; fmt++)
	{
		switch(state)
		{
        // STATE 0: AWAITING % 
		case 0:
			if(*fmt != '%')	// not %... 
			{
				Buffer[Pos++] = *fmt; // ...just echo it 
				count++;
				break;
			}
            // found %, get next char and advance state to check if next char is a flag 
			state++;
			fmt++;
			// FALL THROUGH 
            // STATE 1: AWAITING FLAGS (%-0) 
		case 1:
			if(*fmt == '%')	// %% 
			{
				Buffer[Pos++] = *fmt;
				count++;
				state = flags = given_wd = 0;
				break;
			}
			if(*fmt == '-')
			{
				if(flags & PR_LJ)// %-- is illegal 
					state = flags = given_wd = 0;
				else
					flags |= PR_LJ;
				break;
			}
            // not a flag char: advance state to check if it's field width 
			state++;
            // check now for '%0...' 
			if(*fmt == '0')
			{
				flags |= PR_LZ;
				fmt++;
			}
			// FALL THROUGH 
            // STATE 2: AWAITING (NUMERIC) FIELD WIDTH 
		case 2:
			if(*fmt >= '0' && *fmt <= '9')
			{
				given_wd = 10 * given_wd +
					(*fmt - '0');
				break;
			}
            // not field width: advance state to check if it's a modifier 
			state++;
			// FALL THROUGH 
            // STATE 3: AWAITING MODIFIER CHARS (FNlh) 
		case 3:
			if(*fmt == 'F')
			{
				flags |= PR_FP;
				break;
			}
			if(*fmt == 'N')
				break;
			if(*fmt == 'l')
			{
				flags |= PR_32;
				break;
			}
			if(*fmt == 'h')
			{
				flags |= PR_16;
				break;
			}
            // not modifier: advance state to check if it's a conversion char 
			state++;
			// FALL THROUGH 
            // STATE 4: AWAITING CONVERSION CHARS (Xxpndiuocs) 
		case 4:
			where = buf + PR_BUFLEN - 1;
			*where = '\0';
			switch(*fmt)
			{
			case 'X':
				flags |= PR_CA;
				// FALL THROUGH 
                // xxx - far pointers (%Fp, %Fn) not yet supported 
			case 'x':
			case 'p':
			case 'n':
				radix = 16;
				goto DO_NUM;
			case 'd':
			case 'i':
				flags |= PR_SG;
				// FALL THROUGH 
			case 'u':
				radix = 10;
				goto DO_NUM;
			case 'o':
				radix = 8;
                // load the value to be printed. l=long=32 bits: 
                DO_NUM:
    			if(flags & PR_32)
					num = va_arg(args, uint);
                // h=short=16 bits (signed or unsigned) 
				else if(flags & PR_16)
				{
					if(flags & PR_SG)
						num = va_arg(args, short);
					else
						num = va_arg(args, ushort);
				}
                // no h nor l: sizeof(int) bits (signed or unsigned) 
				else
				{
					if(flags & PR_SG)
						num = va_arg(args, int);
					else
						num = va_arg(args, uint);
				}
                // take care of sign 
				if(flags & PR_SG)
				{
					if(num < 0)
					{
						flags |= PR_WS;
						num = -num;
					}
				}
                // convert binary to octal/decimal/hex ASCII OK, I found my mistake. The math here is _always_ unsigned 
				do
				{
					uint temp;

					temp = (uint)num % radix;
					where--;
					if(temp < 10)
						*where = temp + '0';
					else if(flags & PR_CA)
						*where = temp - 10 + 'A';
					else
						*where = temp - 10 + 'a';
					num = (uint)num / radix;
				}
				while(num != 0);
				goto EMIT;
			case 'c':
                // disallow pad-left-with-zeroes for %c 
				flags &= ~PR_LZ;
				where--;
				*where = (char)va_arg(args, char);
				actual_wd = 1;
				goto EMIT2;
			case 's':
                // disallow pad-left-with-zeroes for %s 
				flags &= ~PR_LZ;
				where = va_arg(args, char *);
                EMIT:
				actual_wd = strlen(where);
				if(flags & PR_WS)
					actual_wd++;
                // if we pad left with ZEROES, do the sign now 
				if((flags & (PR_WS | PR_LZ)) == (PR_WS | PR_LZ))
				{
					Buffer[Pos++] = '-';
					count++;
				}
                // pad on left with spaces or zeroes (for right justify) 
                EMIT2:
				if((flags & PR_LJ) == 0)
				{
					while(given_wd > actual_wd)
					{
						Buffer[Pos++] = (flags & PR_LZ ? '0' : ' ');
						count++;
						given_wd--;
					}
				}
                // if we pad left with SPACES, do the sign now 
				if((flags & (PR_WS | PR_LZ)) == PR_WS)
				{
					Buffer[Pos++] = '-';
					count++;
				}
                // emit string/char/converted number 
				while(*where != '\0')
				{
					Buffer[Pos++] = *where++;
					count++;
				}
                // pad on right with spaces (for left justify) 
				if(given_wd < actual_wd)
					given_wd = 0;
				else given_wd -= actual_wd;
				for(; given_wd; given_wd--)
				{
					Buffer[Pos++] = ' ';
					count++;
				}
				break;
			default:
				break;          
			}
		default:
			state = flags = given_wd = 0;
			break;
		}
	}

    Buffer[Pos] = '\0';

	return count;
}
*/

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void printf(ushort X, ushort Y, const char *fmt, ...)
{
	va_list args;

    char sBuffer[1024];

	va_start(args, fmt);
	(void)SPrintfEx(sBuffer, fmt, args);
	va_end(args);

	GotoXY(X, Y);
    PutS(sBuffer);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void printf(const char *fmt, ...)
{
	va_list args;

    char sBuffer[1024];

	va_start(args, fmt);
	(void)SPrintfEx(sBuffer, fmt, args);
	va_end(args);

    PutS(sBuffer);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

int sprintf(char *Buffer, const char *fmt, ...)
{
	va_list args;

	va_start(args, fmt);
	int iLength = SPrintfEx(Buffer, fmt, args);
	va_end(args);
    return iLength;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#endif

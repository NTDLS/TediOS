////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#ifndef _STDLIB_C_
#define _STDLIB_C_
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/*
    Basic C runtime functions.
*/
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#include "STDLib.H"
#include "String.H"

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

char *strcat(char *s, const char *append)
{
    char *save = s;
    for (; *s; ++s);
    {
        while((*s++ = *append++));
    }

    return save;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

char *strncat(char *dst, const char *src, uint n)
{
    if (n != 0)
    {
        char *d = dst;
        const char *s = src;

        while (*d != 0)
        {
            d++;
        }
    
        do {
            if ((*d = *s++) == 0)
            {
                break;
            }
            d++;
        } while (--n != 0);
    
        *d = 0;
    }
    return dst;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

int strcmp(const char *s1, const char *s2)
{
    while (*s1 == *s2)
    {
        if (*s1 == 0)
        {
            return 0;
        }
        s1++;
        s2++;
    }
    return *(unsigned const char *)s1 - *(unsigned const char *)(s2);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

int strcmpi(const char *s1, const char *s2)
{
    while (ToLower((unsigned char)*s1) == ToLower((unsigned char)*s2))
    {
        if (*s1 == 0)
        {
            return 0;
        }
        s1++;
        s2++;
    }
    return (int)ToLower((unsigned char)*s1) - (int)ToLower((unsigned char)*s2);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

int strnicmp(const char *s1, const char *s2, uint n)
{
    if (n == 0)
    {
        return 0;
    }

    do {
        if (ToLower((unsigned char)*s1) != ToLower((unsigned char)*s2++))
        {
            return (int)ToLower((unsigned char)*s1) - (int)ToLower((unsigned char)*--s2);
        }
        if (*s1++ == 0)
        {
            break;
        }
    } while (--n != 0);

    return 0;
}
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

int strncmp(const char *s1, const char *s2, uint n)
{
    if (n == 0)
    {
        return 0;
    }

    do {
        if (*s1 != *s2++)
        {
            return *(unsigned const char *)s1 - *(unsigned const char *)--s2;
        }
        if (*s1++ == 0)
        {
            break;
        }
    } while (--n != 0);

    return 0;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

char memcmp(void *_m1, void *_m2, int n)
{
	unsigned char *m1 = (unsigned char *)_m1, *m2 = (unsigned char *)_m2;
	int i;

	for(i=0; i<n; i++)
	{
		if(m1[i] != m2[i])
		{
			return (m1[i] > m2[i] ? 1 : -1);
		}
	}
	return 0;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

char *strncpy(char *dst, const char *src, uint n)
{
    if (n != 0)
    {
        char *d = dst;
        const char *s = src;
        do {
            if ((*d++ = *s++) == 0)
            {
                while (--n != 0)
                {
                    *d++ = 0;
                }
                break;
            }
        } while (--n != 0);
    }
    return dst;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

char *strcpy(char *to, const char *from)
{
    char *save = to;
    for (; (*to = *from); ++from, ++to);
    return save;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

ushort *memsetw(ushort *dest, ushort val, int count)
{
    ushort *temp = (ushort *)dest;
    for( ; count != 0; count--) *temp++ = val;
    return dest;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

uint strlen(const char *str)
{
    const char *s;
    if(str == 0)
    {
        return 0;
    }

    for(s = str; *s; ++s);

    return s-str;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////

int HexToDec(const char *sHexString)
{
	struct CHexMap
	{
		char cChar;
		int iValue;
	};

	const int iHexMapSz = 16;

	CHexMap HexMap[iHexMapSz] =
		{
			{'0', 0}, {'1', 1}, {'2', 2}, {'3', 3},
			{'4', 4}, {'5', 5}, {'6', 6}, {'7', 7},
			{'8', 8}, {'9', 9}, {'A',10}, {'B',11},
			{'C',12}, {'D',13}, {'E',14}, {'F',15}
		};

	char sAlloc[64];
	strcpy(sAlloc, UCase((char *)sHexString));

	char *sHex = sAlloc;
	int iResult = 0;
	bool bFirstTime = true;

	if (*sHex == '0' && *(sHex + 1) == 'X')
	{
		sHex += 2;
	}

	while (*sHex != '\0')
	{
		bool bFound = false;
		for (int i = 0; i < iHexMapSz; i++)
		{
			if(*sHex == HexMap[i].cChar)
			{
				if(!bFirstTime)
				{
					iResult <<= 4;
				}
				iResult |= HexMap[i].iValue;
				bFound = true;
				break;
			}
		}

		if (!bFound)
		{
			break;
		}

		sHex++;
		bFirstTime = false;
	}

	return iResult;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

int isalnum(int c)
{
	return isalpha(c);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

int isalpha(int c)
{
	char *AL="QWERTYUIOPASDFGHJKLÖÄÅZXCVBNM";
	char *al="qwertyuiopasdfghjklöäåzxcvbnm";
	int i,l;

	l = strlen(al);
	for(i=0; i<l; i++)
	{
		if(al[i]==c)return 1;
		if(AL[i]==c)return 1;
	}
	return 0;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

int isdigit(int c)
{
	char *nu="0123456789";
	int i,l;

	l = strlen(nu);
	for(i=0; i<l; i++)
	{
		if(nu[i]==c)return 1;
	}
	return 0;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

int isxdigit(int c)
{
	char *al="0123456789abcdefABCDEF";
	int i,l;

	l = strlen(al);
	for(i=0; i<l; i++)
	{
		if(al[i]==c)return 1;
	}
	return 0;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

int utoa(unsigned int uValue, char *sOutValue)
{
	unsigned short iWPos = 0;

	do {
		sOutValue[iWPos++] = (unsigned char )48 + (uValue % 10);
	} while((uValue /= 10) > 0);

	sOutValue[iWPos] = '\0';

	ReverseString(sOutValue, iWPos);

	return iWPos;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

int itoa(int iValue, char *sOutValue)
{
	unsigned short iWPos = 0;

	if(iValue < 0)
	{
		sOutValue[iWPos++] = '-';
	}

	do {
		sOutValue[iWPos++] = (unsigned char )48 + (iValue % 10);
	} while((iValue /= 10) > 0);

	sOutValue[iWPos] = '\0';

	ReverseString(sOutValue, iWPos);

	return iWPos;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

int atoi(const char *sInValue)
{
	int iTotal = 0;
	int iMinus = 0;

	while(IsWhiteSpace(*sInValue))
	{
		sInValue++;
	}

	if(*sInValue == '+')
	{
		sInValue++;
	}
	else if(*sInValue == '-')
	{
		iMinus = 1;
		sInValue++;
	}
	while(isdigit(*sInValue))
	{
		iTotal *= 10;
		iTotal += (*sInValue++ - '0');
	}
	return iMinus ? -iTotal : iTotal;
}


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#endif

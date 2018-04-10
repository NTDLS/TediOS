///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//  Copyright © NetworkDLS 2002, All rights reserved
//
// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF 
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO 
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A 
// PARTICULAR PURPOSE.
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#ifndef _NSWFL_STRING_CPP_
#define _NSWFL_STRING_CPP_
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#include "STDLib.H"
#include "String.H"
#include "Float.H"

#include "../Drivers/CATA.H"
#include "../Drivers/System.H"
#include "../Drivers/Timer.h"
#include "../Drivers/CMemory.h"
#include "../Drivers/CKeyboard.H"


///////////////////////////////////////////////////////////////////////////////////////////////////////////////////

char *PadRight(char *sBuffer, const char cPadWith, int iPadTo)
{
	int iWPos = strlen(sBuffer);

	for(; iWPos < iPadTo; iWPos++)
	{
		sBuffer[iWPos] = cPadWith;
	}

	sBuffer[iWPos] = '\0';

	return sBuffer;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////

char *PadLeft(char *sBuffer, const char cPadWith, int iPadTo)
{
	int iWPos = strlen(sBuffer);

	ReverseString(sBuffer, iWPos);

	for(; iWPos < iPadTo; iWPos++)
	{
		sBuffer[iWPos] = cPadWith;
	}

	sBuffer[iWPos] = '\0';

	return ReverseString(sBuffer, iWPos);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool IsUpper(char cIn)
{
	return (cIn >= 65 && cIn <= 90);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool IsLower(char cIn)
{
	return (cIn >= 97 && cIn <= 122);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////

char ToUpper(char cIn)
{
	if(IsLower(cIn))
	{
		return (cIn-32);
	}
	return cIn;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////

char ToLower(char cIn)
{
	if(IsUpper(cIn))
	{
		return (cIn+32);
	}
	return cIn;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////

int IndexOf(const char *sBuf, const char cCharacter)
{
	return IndexOf(sBuf, strlen(sBuf), cCharacter);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////

int IndexOf(const char *sBuf, int iBufSz, const char cCharacter)
{
	for(int iPos = 0; iPos < iBufSz; iPos++)
	{
		if((unsigned char)sBuf[iPos] == (unsigned char)cCharacter)
		{
			return iPos;
		}
	}
	return -1;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////

int LastIndexOf(const char *sBuf, const char cCharacter)
{
	return LastIndexOf(sBuf, strlen(sBuf), cCharacter);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////

int LastIndexOf(const char *sBuf, int iBufSz, const char cCharacter)
{
	if(iBufSz > 0)
	{
		for(int iPos = iBufSz - 1; iPos > -1; iPos--)
		{
			if((unsigned char)sBuf[iPos] == (unsigned char)cCharacter)
			{
				return iPos;
			}
		}
	}
	return -1;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////

char *UCase(char *sBuf, int iBufSz)
{
	for(int iPos = 0; iPos < iBufSz; iPos++)
	{
		sBuf[iPos] = ToUpper(sBuf[iPos]);
	}
	return sBuf;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////

char *LCase(char *sBuf, int iBufSz)
{
	for(int iPos = 0; iPos < iBufSz; iPos++)
	{
		sBuf[iPos] = ToLower(sBuf[iPos]);
	}
	return sBuf;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////

char *UCase(char *sBuf)
{
	return UCase(sBuf, strlen(sBuf));
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////

char *LCase(char *sBuf)
{
	return LCase(sBuf, strlen(sBuf));
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////

int Trim(char *sInOutBuf, int iLen)
{
	int iLength = LTrim(sInOutBuf, iLen);
	return RTrim(sInOutBuf, iLen);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////

int Trim(char *sInOutBuf)
{
	return Trim(sInOutBuf, strlen(sInOutBuf));
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////

int LTrim(char *sInOutBuf, int iLen)
{
	if(iLen > 0)
	{
		ReverseString(sInOutBuf, iLen);
		iLen = RTrim(sInOutBuf, iLen);
		ReverseString(sInOutBuf, iLen);
	}

	return iLen;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

int LTrim(char *sInOutBuf)
{
	return LTrim(sInOutBuf, strlen(sInOutBuf));
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////

int RTrim(char *sInOutBuf, int iLen)
{
    if(iLen > 0)
	{
		iLen = (GetLastCharPos(sInOutBuf, iLen) + 1);
		sInOutBuf[iLen] = '\0'; //replace the NULL terminator.
	}
	return iLen;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

int RTrim(char *sInOutBuf)
{
	return RTrim(sInOutBuf, strlen(sInOutBuf));
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

//sInOutBuf returns the string passed into it with all white spaces
//	trimmed off the beginning and end of it.
int CleanWhiteSpaces(char *sInOutBuf, int iLen)
{
	if(iLen > 0)
	{
		iLen = RTrim(sInOutBuf, iLen);
		ReverseString(sInOutBuf, iLen);
		iLen = RTrim(sInOutBuf, iLen);
		ReverseString(sInOutBuf, iLen);
	}
	return iLen;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////

int CleanWhiteSpaces(char *sInOutBuf)
{
	return CleanWhiteSpaces(sInOutBuf, strlen(sInOutBuf));
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////

int InStrI(const char *sSearchFor, int sSearchForSz, const char *sInBuf, const int iBufSz, const int iStartPos)
{
	int iControlLoop  = 0; 
	int iLookingLoop  = 0; 
	int iStringStart  = -1;

	if(iStartPos > iBufSz)
	{
		return -2;
	}

	if(sSearchForSz > iBufSz)
	{
		return -2;
	}

	for(iControlLoop = iStartPos; iControlLoop <= (iBufSz-sSearchForSz); iControlLoop++)
	{
		if(ToLower(sInBuf[iControlLoop]) == ToLower(sSearchFor[iLookingLoop]))
		{
			while(iLookingLoop <= sSearchForSz) 
			{
				if(ToLower(sSearchFor[iLookingLoop]) == ToLower(sInBuf[iLookingLoop + iControlLoop]))
				{
					iLookingLoop++;
				}
				else iLookingLoop = sSearchForSz + 1; 

				if(iLookingLoop == sSearchForSz)
				{
					iStringStart = iControlLoop; 
					iControlLoop = (iBufSz - sSearchForSz) + 1;
				}
			}
		}
		iLookingLoop = 0;
	}

	return iStringStart; 
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////

int InStr(const char *sSearchFor, int sSearchForSz, const char *sInBuf, const int iBufSz, const int iStartPos)
{
	int iControlLoop  = 0; 
	int iLookingLoop  = 0; 
	int iStringStart  = -1;

	if(iStartPos > iBufSz)
	{
		return -2;
	}

	if(sSearchForSz > iBufSz)
	{
		return -2;
	}

	for(iControlLoop = iStartPos; iControlLoop <= (iBufSz-sSearchForSz); iControlLoop++)
	{
		if(sInBuf[iControlLoop] == sSearchFor[iLookingLoop])
		{
			while(iLookingLoop <= sSearchForSz) 
			{
				if(sSearchFor[iLookingLoop] == sInBuf[iLookingLoop + iControlLoop])
				{
					iLookingLoop++;
				}
				else iLookingLoop = sSearchForSz + 1; 

				if(iLookingLoop == sSearchForSz)
				{
					iStringStart = iControlLoop; 
					iControlLoop = (iBufSz - sSearchForSz) + 1;
				}
			}
		}
		iLookingLoop = 0;
	}

	return iStringStart; 
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////

int InStr(const char *sSearchFor, const char *sInBuf, const int iBufSz, const int iStartPos)
{
	return InStr(sSearchFor, strlen(sSearchFor), sInBuf, iBufSz, iStartPos);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////

int InStr(const char *sSearchFor, const char *sInBuf)
{
	return InStr(sSearchFor, sInBuf, strlen(sInBuf), 0);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////

int InStrI(const char *sSearchFor, const char *sInBuf, const int iBufSz, const int iStartPos)
{
	return InStrI(sSearchFor, strlen(sSearchFor), sInBuf, iBufSz, iStartPos);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////

int InStrI(const char *sSearchFor, const char *sInBuf)
{
	return InStrI(sSearchFor, sInBuf, strlen(sInBuf), 0);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//Returns the count of the Occurrences of "sLookFor" that are found in "sString".

int CountOccurrences(const char *sLookFor, const char *sString, int sStringSz)
{
	int iPos = 0;
	int iOccurrences = 0;
	while( (iPos = InStr(sLookFor, sString, sStringSz, iPos)) >= 0)
	{
		iPos++;
		iOccurrences++;
	}

	return iOccurrences;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//Returns the count of the Occurrences of "sLookFor" that are found in "sString".

int CountOccurrences(const char *sLookFor, const char *sString)
{
	return CountOccurrences(sLookFor, sString, strlen(sString));
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/*
	Returns a zero-based, one-dimensional array containing a specified number of substrings.

	Example:

		char **sArray = NULL;
		char *sBuf = "Hello\tCruel\tWorld\tFrom\tC\tPlus\tPlus\t";
		int iItems = 0;
		int iItem = 0;

		sArray = SplitEx(sBuf, strlen(sBuf), sArray, &iItems, strlen("\t"), 1);

		printf("Array Items: %d\n", UBound(sArray));

		while(iItem < iItems)
		{
			printf("Item[%d]: %s\n", iItem, sArray[iItem]);
			iItem++;
		}

		FreeSplitStringEx(sArray, iItems);
*/

char **Split(const char *sBuf, int iBufSz, char **sArray, int *iOutItems, const char *sSplitOn, int iSplitOnSz)
{
	int iStartPos = 0;

	int iRPos = 0;
	int iWPos = 0;

	int iItems = 0;

	int iItemCount = 0; // The number of sub-strings that are going to be parsed from the string.
	int iLastPos = 0;

	//Get the number of items that we are going to split.
	while((iStartPos = InStr(sSplitOn, iSplitOnSz, sBuf, iBufSz, iStartPos)) >= 0)
	{
		iItemCount++;
		iStartPos = (iStartPos + iSplitOnSz);
		iLastPos = iStartPos;
	}

	//The string is not terminated by a delimiter.
	if(iBufSz > iLastPos)
	{
		iItemCount++;
	}

	iStartPos = 0;

	//Allocate enough RAM to hold all the sub-strings and the last NULL array item.
	sArray = (char **) calloc(sizeof(char *), iItemCount + 1);

	while((iStartPos = InStr(sSplitOn, iSplitOnSz, sBuf, iBufSz, iRPos)) >= 0)
	{
		//Allocate enough RAM to hold the sub-string.
		sArray[iItems] = (char *) calloc(sizeof(char), (iStartPos - iRPos) + 1);

		iWPos = 0;
		while(iRPos < iStartPos)
		{
			sArray[iItems][iWPos++] = sBuf[iRPos++];
		}

		sArray[iItems][iWPos] = '\0';

		iRPos = (iRPos + iSplitOnSz);

		iItems++;
	}

	//The string is not terminated by a delimiter.
	if(iBufSz > iRPos)
	{
		//Allocate enough RAM to hold the sub-string.
		sArray[iItems] = (char *) calloc(sizeof(char), (iBufSz - iRPos) + 1);

		iWPos = 0;

		while(iRPos < iBufSz)
		{
			sArray[iItems][iWPos++] = sBuf[iRPos++];
		}

		sArray[iItems][iWPos] = '\0';
		iRPos = (iRPos + iSplitOnSz);

		iItems++;
	}
	
	//Insert a null at the end of the array.
	sArray[iItems] = NULL;

	*iOutItems = iItems;

	return sArray;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////

char **Split(const char *sBuf, int iBufSz, char **sArray, int *iOutItems, const char *sSplitOn)
{
	return Split(sBuf, iBufSz, sArray, iOutItems, sSplitOn, strlen(sSplitOn));
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/*
	Returns a zero-based, one-dimensional array containing a specified number of substrings.

	Example:

		char **sArray = NULL;
		char *sBuf = "Hello\tCruel\tWorld\tFrom\tC\tPlus\tPlus\t";
		int iItem = 0;

		sArray = Split(sBuf, "\t");

		printf("Array Items: %d\n", UBound(sArray));

		while(sArray[iItem] != NULL)
		{
			printf("Item[%d]: %s\n", iItem, sArray[iItem]);
			iItem++;
		}

		FreeSplitString(sArray);
*/

char **Split(const char *sBuf, const char *sSplitOn)
{
	int iOutItems = 0;

	char **sArray = NULL;

	return Split(sBuf, strlen(sBuf), sArray, &iOutItems, sSplitOn, strlen(sSplitOn));
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/*
	This function will free all the RAM allocated by an array.

	All array items, and the array its self.

	Intended for use with the Split() and SplitEx functions.
*/
void FreeArray(char **sArray, int iItems)
{
	for(int iItem = 0; iItem < iItems; iItem++)
	{
		Mem->Free(sArray[iItem]);
	}

	Mem->Free(sArray);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/*
	This function will free all the RAM allocated by an array.

	All array items, and the array its self.

	Intended for use with the Split() and SplitEx functions.
*/
void FreeArray(char **sArray)
{
	FreeArray(sArray, UBound(sArray));
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/*
	This function will return the number of items in a char *Array[].
	This function will malfunction if the last item in the array is not NULL.

	Intended for use with the Split() and SplitEx functions.
*/
int UBound(char **sArray)
{
	int iItem = 0;

	while(sArray[iItem])
	{
		iItem++;
	}

	return iItem;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//Returns true if the passed character is 0-9.
bool IsNumeric(char cIn)
{
	return (cIn >= 48 && cIn <= 57);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//Returns true if the passed string is in the following formats:
	//0, -.0, 0.0, -0, -0.0
bool IsNumeric(char *sText)
{
	int iLen = strlen(sText);
	int iRPos = 0;
	bool bHasDecimal = false;

	if(iLen == 0)
	{
		return false;
	}

	if(sText[iRPos] == '-')
	{
		iRPos++;
	}

	for(; iRPos < iLen; iRPos++)
	{
		if(!IsNumeric(sText[iRPos]))
		{
			if(sText[iRPos] == '.')
			{
				if(bHasDecimal)
				{
					return false;
				}
				bHasDecimal = true;
			}
			else{
				return false;
			}
		}
	}

	return true;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////

int Mid(const char *sInBuf, int iInBufSz, int iStartPos, int iRChars, char *sOutBuf, int iMaxSz)
{
	int iRPos = iStartPos;
	int iWPos = 0;
	int iChars = 0;

	if(iRChars <= 0 || iRChars > (iInBufSz - iStartPos) )
	{
		iChars = iInBufSz;
	}
	else iChars = (iStartPos + iRChars);

	if(((iChars - iStartPos) + 1) > iMaxSz)
	{
		return -1;
	}

	while(iRPos < iChars)
	{
		sOutBuf[iWPos++] = sInBuf[iRPos++];
	}

	sOutBuf[iWPos] = '\0';

	return iWPos;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////

int Right(char *sOutBuf, int iMaxSz, const char *sInBuf, int iInLen, int iRight)
{
	int iLen = iInLen;
	int iRPos = iLen - iRight;
	int iWPos = 0;

	if((iRight + 1) > iMaxSz)
	{
		return -1;
	}

	if(iRight > iLen)
	{
		strcpy(sOutBuf, "");
		return 0;
	}

	while(iRPos < iLen)
	{
		sOutBuf[iWPos++] = sInBuf[iRPos++];
	}

	sOutBuf[iWPos] = '\0';

	return iWPos;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////

int Left(char *sOutBuf, int iMaxSz, const char *sInBuf, int iInLen, int iLeft)
{
	int iLen = iInLen;
	int iRPos = 0;

	if((iLeft + 1) > iMaxSz)
	{
		return -1;
	}

	if(iLeft > iInLen)
	{
		strcpy(sOutBuf, "");
		return 0;
	}

	while(iRPos < iLeft)
	{
		sOutBuf[iRPos] = sInBuf[iRPos];
		iRPos++;
	}

	sOutBuf[iRPos] = '\0';

	return iRPos;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////

//Returns true if the character passed into cChar is a whitespace, otherwise it returns false.
bool IsWhiteSpace(char cChar)
{
    if(cChar == ' ' || cChar == '\r' || cChar == '\n' || cChar == '\t' || cChar == '\0')
	{
        return true;
	}
    return false;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////

//Returns the position of the first valid character in the string passed into sInBuf.
//iInBufSz should be the length of the string being passed into sInBuf
int GetFirstCharPos(const char *sInBuf, int iInBufSz)
{
    for(int iRPos = 0; iRPos < iInBufSz; iRPos++)
    {
        if(!IsWhiteSpace(sInBuf[iRPos]))
		{
            return iRPos;
		}
	}
    return -1;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////

int GetFirstCharPos(const char *sInBuf)
{
	return GetFirstCharPos(sInBuf, strlen(sInBuf));
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////

//Returns the position of the last valid character in the string passed into sInBuf.
//iInBufSz should be the length of the string being passed into sInBuf
int GetLastCharPos(const char *sInBuf, int iInBufSz)
{
    int iRPos = iInBufSz;
    while(iRPos != -1)
    {
        if(!IsWhiteSpace(sInBuf[iRPos]))
		{
            return iRPos;
		}
		iRPos--;
	}
    return -1;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////

int GetLastCharPos(const char *sInBuf)
{
	return GetLastCharPos(sInBuf, strlen(sInBuf));
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

int CharIndex(const char *sBuf, const char cChar)
{
	return CharIndex(sBuf, strlen(sBuf), cChar);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

int CharIndex(const char *sBuf, int iBufSz, const char cChar)
{
	for(int iPos = 0; iPos < iBufSz; iPos++)
	{
		if(sBuf[iPos] == cChar)
		{
			return iPos;
		}
	}

	return -1;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

int SkipWhiteSpaces(const char *sBuf, int iBufSz, int *iRPos)
{
	int iiRPos = *iRPos;

	while(IsWhiteSpace(sBuf[iiRPos]) && *iRPos < iBufSz)
	{
		iiRPos++;
	}

	*iRPos = iiRPos;

	return iiRPos;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

__inline bool IsCharInArray(const unsigned char cTokChar, const unsigned char *sChars, uint iChars)
{
	for(uint iChar = 0; iChar < iChars; iChar++)
	{
		if(cTokChar == sChars[iChar])
		{
			return true;
		}
	}
	return false;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/*
	Returns the size in characters of the token.
*/
int GetToken(const char *sBuf, int iBufSz, char *sOutToken, int iMaxTokSz,
			 const char *sInvalidChars, int iInvalidCount)
{
	return GetNextToken(sBuf, iBufSz, sOutToken,
		iMaxTokSz, (int)0, sInvalidChars, iInvalidCount);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/*
	Returns the size in characters of the token.
*/
int GetToken(const char *sBuf, int iBufSz, char *sOutToken, int iMaxTokSz)
{
	return GetNextToken(sBuf, iBufSz, sOutToken, iMaxTokSz, (int)0);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/*
	Returns the size in characters of the token.
*/
int GetNextToken(const char *sBuf, int iBufSz, char *sOutToken,
				 int iMaxTokSz, int *iInOutRPos,
				 const char *sInvalidChars, int iInvalidCount)
{
	int iRPos = *iInOutRPos;
	int iWPos = 0;

	SkipWhiteSpaces(sBuf, iBufSz, &iRPos);

	if(sOutToken)
	{
		while(!IsCharInArray(sBuf[iRPos], (unsigned char *)sInvalidChars, iInvalidCount)
			&& iRPos < iBufSz && iWPos < iMaxTokSz)
		{
			sOutToken[iWPos++] = sBuf[iRPos++];
		}
		sOutToken[iWPos] = '\0';
	}

	SkipWhiteSpaces(sBuf, iBufSz, &iRPos);

	*iInOutRPos = iRPos;

	return iWPos; 
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/*
	Returns the size in characters of the token.
*/
int GetNextToken(const char *sBuf, int iBufSz, char *sOutToken,
				 int iMaxTokSz, int *iInOutRPos)
{
	int iRPos = *iInOutRPos;
	int iWPos = 0;

	SkipWhiteSpaces(sBuf, iBufSz, &iRPos);

	if(sOutToken)
	{
		while(!IsWhiteSpace(sBuf[iRPos]) && iRPos < iBufSz && iWPos < iMaxTokSz)
		{
			sOutToken[iWPos++] = sBuf[iRPos++];
		}
		sOutToken[iWPos] = '\0';
	}

	SkipWhiteSpaces(sBuf, iBufSz, &iRPos);

	*iInOutRPos = iRPos;

	return iWPos; 
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/*
	Returns the size in characters of the token.
*/
int GetNextToken(const char *sBuf, int iBufSz, char *sOutToken,
				 int iMaxTokSz, int iStartPos,
				 const char *sInvalidChars, int iInvalidCount)
{
	int iInOutRPos = iStartPos;
	return GetNextToken(sBuf, iBufSz, sOutToken, iMaxTokSz, &iInOutRPos, sInvalidChars, iInvalidCount);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/*
	Returns the size in characters of the token.
*/
int GetNextToken(const char *sBuf, int iBufSz, char *sOutToken, int iMaxTokSz, int iStartPos)
{
	int iInOutRPos = iStartPos;
	return GetNextToken(sBuf, iBufSz, sOutToken, iMaxTokSz, &iInOutRPos);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////

char *ReverseString(char *sBuf, int iBufSz)
{
	char *String1 = NULL;
	char *String2 = NULL;

	if(!sBuf || !*sBuf)
	{
		return NULL;
	}

	for(String1 = sBuf, String2 = sBuf + iBufSz - 1; String2 > String1; ++String1, --String2)
	{
		*String1 ^= *String2;
		*String2 ^= *String1;
		*String1 ^= *String2;
	}

	return sBuf;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////

char *FormatNumeric(char *sOut, int iMaxSz, double dValue, int iDecimalPlaces)
{
	char sValue[64];
	ftoa(dValue, iDecimalPlaces, sValue);
	if(iDecimalPlaces == 0)
	{

	}
	return FormatDString(sOut, iMaxSz, sValue);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////

char *FormatNumeric(char *sOut, int iMaxSz, int iValue)
{
	char sValue[64];
	itoa(iValue, sValue);
	return FormatIString(sOut, iMaxSz, sValue);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////

char *FormatIString(char *sOut, int iMaxSz, const char *sIn)
{
    int iRPos = strlen(sIn) - 1;
    int iWPos = 0;
    int iDPos = 0;

    if(iRPos < 3)
    {
        strcpy(sOut, sIn);
        return sOut;
    }

    while(iRPos != -1)
    {
        if(iDPos++ == 3)
        {
			if(iWPos >= iMaxSz) return NULL; //Buffer too small.
            sOut[iWPos++] = ',';
            iDPos = 1;
        }

		if(iWPos >= iMaxSz) return NULL; //Buffer too small.
        sOut[iWPos++] = sIn[iRPos--];
    }

    ReverseString(sOut, iWPos);

    sOut[iWPos] = '\0';

    return sOut;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////

char *FormatDString(char *sOut, int iMaxSz, const char *sIn)
{
    int iRPos = strlen(sIn) - 1;
    int iWPos = 0;
    int iDPos = 0;

	bool bFoundDecimal = false;

    if(iRPos < 3)
    {
        strcpy(sOut, sIn);
        return sOut;
    }

    while(iRPos != -1)
    {
		if(sIn[iRPos] == '.')
		{
			bFoundDecimal = true;
			iDPos = -1;
		}

        if(iDPos++ == 3 && bFoundDecimal)
        {
			return NULL;
            sOut[iWPos++] = ',';
            iDPos = 1;
        }

		if(iWPos >= iMaxSz)
		{
			return NULL;
		}
        sOut[iWPos++] = sIn[iRPos--];
    }

    ReverseString(sOut, iWPos);

    sOut[iWPos] = '\0';

    return sOut;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

char *FriendlySize(uint uFileSize, char *sOut, int iMaxOut, int iDecimalPlaces)
{
	double dDivideBy = 1;

	char *sSzStr = "";
	char sValue[64];

	if(uFileSize >= PETABYTE) {
		dDivideBy = PETABYTE;
		sSzStr = "PB";
	}
	else if(uFileSize >= TERABYTE) {
		dDivideBy = TERABYTE;
		sSzStr = "TB";
	}
	else if(uFileSize >= GIGABYTE) {
		dDivideBy = GIGABYTE;
		sSzStr = "GB";
	}
	else if(uFileSize >= MEGABYTE) {
		dDivideBy = MEGABYTE;
		sSzStr = "MB";
	}
	else if(uFileSize >= KILOBYTE) {
		dDivideBy = KILOBYTE;
		sSzStr = "KB";
	}
	else{
		dDivideBy = 1;
		sSzStr = "B";
	}

	sprintf(sOut, "%s %s", FormatNumeric(sValue,
		sizeof(sValue), uFileSize / dDivideBy, iDecimalPlaces), sSzStr);

	return sOut;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#endif

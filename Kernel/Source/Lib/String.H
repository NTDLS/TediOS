///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//  Copyright © NetworkDLS 2002, All rights reserved
//
// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF 
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO 
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A 
// PARTICULAR PURPOSE.
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#ifndef _NSWFL_STRING_H_
#define _NSWFL_STRING_H_
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#include "Types.H"

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool IsUpper(char cIn);
bool IsLower(char cIn);
char ToUpper(char cIn);
char ToLower(char cIn);
int IndexOf(const char *sBuf, const char cCharacter);
int IndexOf(const char *sBuf, int iBufSz, const char cCharacter);
int LastIndexOf(const char *sBuf, const char cCharacter);
int LastIndexOf(const char *sBuf, int iBufSz, const char cCharacter);
char *UCase(char *sBuf, int iBufSz);
char *LCase(char *sBuf, int iBufSz);
char *UCase(char *sBuf);
char *LCase(char *sBuf);
int Trim(char *sInOutBuf, int iLen);
int Trim(char *sInOutBuf);
int LTrim(char *sInOutBuf, int iLen);
int LTrim(char *sInOutBuf);
int RTrim(char *sInOutBuf, int iLen);
int RTrim(char *sInOutBuf);
int CleanWhiteSpaces(char *sInOutBuf, int iLen);
int CleanWhiteSpaces(char *sInOutBuf);
int InStrI(const char *sSearchFor, int sSearchForSz, const char *sInBuf, const int iBufSz, const int iStartPos);
int InStr(const char *sSearchFor, int sSearchForSz, const char *sInBuf, const int iBufSz, const int iStartPos);
int InStr(const char *sSearchFor, const char *sInBuf, const int iBufSz, const int iStartPos);
int InStr(const char *sSearchFor, const char *sInBuf);
int InStrI(const char *sSearchFor, const char *sInBuf, const int iBufSz, const int iStartPos);
int InStrI(const char *sSearchFor, const char *sInBuf);
int CountOccurrences(const char *sLookFor, const char *sString, int sStringSz);
int CountOccurrences(const char *sLookFor, const char *sString);
char **Split(const char *sBuf, int iBufSz, char **sArray, int *iOutItems, const char *sSplitOn, int iSplitOnSz);
char **Split(const char *sBuf, int iBufSz, char **sArray, int *iOutItems, const char *sSplitOn);
char **Split(const char *sBuf, const char *sSplitOn);
void FreeArray(char **sArray, int iItems);
int UBound(char **sArray);
bool IsNumeric(char cIn);
bool IsNumeric(char *sText);
int Mid(const char *sInBuf, int iInBufSz, int iStartPos, int iRChars, char *sOutBuf, int iMaxSz);
int Right(char *sOutBuf, int iMaxSz, const char *sInBuf, int iInLen, int iRight);
int Left(char *sOutBuf, int iMaxSz, const char *sInBuf, int iInLen, int iLeft);
bool IsWhiteSpace(char cChar);
int GetFirstCharPos(const char *sInBuf, int iInBufSz);
int GetFirstCharPos(const char *sInBuf);
int GetLastCharPos(const char *sInBuf, int iInBufSz);
int GetLastCharPos(const char *sInBuf);
int CharIndex(const char *sBuf, const char cChar);
int CharIndex(const char *sBuf, int iBufSz, const char cChar);
int SkipWhiteSpaces(const char *sBuf, int iBufSz, int *iRPos);
__inline bool IsCharInArray(const unsigned char cTokChar, const unsigned char *sChars, uint iChars);
int GetToken(const char *sBuf, int iBufSz, char *sOutToken, int iMaxTokSz, const char *sInvalidChars, int iInvalidCount);
int GetToken(const char *sBuf, int iBufSz, char *sOutToken, int iMaxTokSz);
int GetNextToken(const char *sBuf, int iBufSz, char *sOutToken, int iMaxTokSz, int *iInOutRPos, const char *sInvalidChars, int iInvalidCount);
int GetNextToken(const char *sBuf, int iBufSz, char *sOutToken, int iMaxTokSz, int *iInOutRPos);
int GetNextToken(const char *sBuf, int iBufSz, char *sOutToken, int iMaxTokSz, int iStartPos, const char *sInvalidChars, int iInvalidCount);
int GetNextToken(const char *sBuf, int iBufSz, char *sOutToken, int iMaxTokSz, int iStartPos);
char *ReverseString(char *sBuf, int iBufSz);
char *FormatIString(char *sOut, int iMaxSz, const char *sIn);
char *FormatDString(char *sOut, int iMaxSz, const char *sIn);
char *FriendlySize(uint uFileSize, char *sOut, int iMaxOut, int iDecimalPlaces);
char *FormatNumeric(char *sOut, int iMaxSz, double dValue, int iDecimalPlaces);
char *FormatNumeric(char *sOut, int iMaxSz, int iValue);
char *PadLeft(char *sBuffer, const char cPadWith, int iPadTo);
char *PadRight(char *sBuffer, const char cPadWith, int iPadTo);

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#endif

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#ifndef _STDLIB_H_
#define _STDLIB_H_
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/*
    Basic C runtime functions.
*/
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#include "Types.H"
#include "SPrintF.H"

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#define CHECK_FLAG(flags,bit) ((flags) & (1 << (bit)))

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

char *strcat(char *s, const char *append);
char *strncat(char *dst, const char *src, uint n);
char *strncpy(char *dst, const char *src, uint n);
char memcmp(void *_m1, void *_m2, int n);
int HexToDec(const char *sHexString);
int strcmp(const char *s1, const char *s2);
int strcmpi(const char *s1, const char *s2);
int strncmp(const char *s1, const char *s2, uint n);
int strnicmp(const char *s1, const char *s2, uint n);
char *strcpy(char *to, const char *from);
uint strlen(const char *str);
ushort *memsetw(ushort *dest, ushort val, int count);
int isalnum(int c);
int isalpha(int c);
int isdigit(int c);
int isxdigit(int c);
int itoa(int iValue, char *sOutValue);
int utoa(unsigned int uValue, char *sOutValue);
int atoi(const char *sInValue);
extern "C" void *memcpy(void *dest, const void *src, int count); //Declared in "memcpy.S"
extern "C" void *memmove(void *dest, const void *source, int num);
extern "C" void *memset(void *buffer, int ch, size_t num);

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#endif

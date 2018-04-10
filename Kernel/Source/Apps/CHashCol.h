#ifndef _CHASHCOL_H_
#define _CHASHCOL_H_
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

class CHASHCol {

typedef struct CONFIGINFO_TAG {
	unsigned char sHash[40 + 1];
    unsigned char sGoalDigest[20];
	int iLen;
	int iMaxLen;
	int iMinChar;
	int iMaxChar;
} CONFIGINFO, *LPCONFIGINFO;

public:
    bool Initialize(const unsigned char *sHash);
    bool Run(ushort iMax);

private:
    bool GenerateStrings(ushort iLength);
    CONFIGINFO CI;
};


///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#endif

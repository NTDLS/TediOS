////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#ifndef _CVIDEO_H_
#define _CVIDEO_H_
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/*
    Video functions.
*/
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#define	VGA_MISC_READ	0x3CC
#define	VGA_TYPE_MONO	0
#define	VGA_TYPE_COLOR	1

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#define VID_CURSOR_BLINKING	    0x00
#define VID_CURSOR_OFF		    0x20
#define VID_CURSOR_SLOWBLINK	0x60

#define VID_MODE_GRAPHICS	    0x02
#define VID_MODE_VIDEO_EN	    0x08
#define VID_MODE_BLINK_EN	    0x20
#define VID_MODE_GFX_PAGE1	    0x80

#define VID_STATUS_HSYNC    	0x01
#define VID_STATUS_VSYNC    	0x80
#define VID_STATUS_VIDEO    	0x08

#define VID_CONFIG_COL132   	0x08
#define VID_GFX_MODE_EN	    	0x01
#define VID_GFX_PAGE_EN	    	0x02

// Video modes
#define VidMode0  0x0  //   40x25         ------   B&W Text     CGA+               8
#define VidMode1  0x1  //   80x25         ------   B&W Text     MDPA+              8
#define VidMode2  0x2  //   40x25         ------   Color Text   CGA+          4 or 8
#define VidMode3  0x3  //   80x25         ------   Color Text   (MDPA?)/CGA+  4 or 8
#define VidMode4  0x4  //   40x25        320x200   4 colors     CGA+               1
#define VidMode5  0x5  //   40x25        320x200   2 colors     CGA+               1
#define VidMode6  0x6  //   80x25        640x200   2 colors     CGA+               1
#define VidMode7  0x7  //   80x25         ------   B&W          MDPA (CGA+?)       1
#define VidMode8  0x8  //   to Ch -- PCjr or other adapters; no longer used
#define VidMode9  0xD  //   40x25        320x200   16 colors    EGA+               8
#define VidMode10 0xE  //   80x25        640x200   16 colors    EGA+               4
#define VidMode11 0xF  //   80x25        640x350   2 colors     EGA+               2
#define VidMode12 0x10 //   80x25        640x350   16 colors    EGA+               2
#define VidMode13 0x11 //   80x25        640x480   2 colors     VGA+               1
#define VidMode14 0x12 //   80x25        640x480   16 colors    VGA+               1
#define VidMode15 0x13 //   40x25        320x200   256 colors   VGA+               1

// Video colors
#define BLACK		  0
#define BLUE  	      1
#define GREEN		  2
#define CYAN		  3
#define RED		      4
#define MAGENTA	      5
#define BROWN 	      6
#define LIGHTGRAY	  7
#define DARKGRAY	  8
#define LIGHTBLUE 	  9
#define LIGHTGREEN	  10
#define LIGHTCYAN	  11
#define PINK		  12
#define LIGHTMAGENTA  13
#define YELLOW	      14
#define WHITE		  15

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

typedef struct _tag_Video_Properties {
    ushort *pVidMem; //Base of video memory.
    ushort iCRTCAddress;
    ushort iAttrib;

    ushort iMaxWidth; //Number of text columns.
    ushort iMaxHeight; //Number of text lines.
    //uint	mda_vram_len;		//Size of video memory.
    uint iIndex_Port;  // Register select port.
    uint iValue_Port;  // Register value port.
    uint iMode_Port;   // Mode control port.
    uint iStatus_Port; // Status and Config port.
    uint iGFX_Port;    // Graphics control port.

    ushort iBasicType;
	uint ulVideoRAM;
	uint ulPageRAM;

    signed int X;
    signed int Y;
} VIDEOPROPERTIES, *LPVIDEOPROPERTIES;

typedef struct _tag_Video_Snapshot {
	byte *pVideoRAM;
	VIDEOPROPERTIES vpProps;
} VIDEOSNAPSHOT, *LPVIDEOSNAPSHOT;

extern VIDEOPROPERTIES VideoProps;

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void Scroll(void);
void MoveCursor(void);
void GotoXY(ushort iX, ushort iY);
void GetXY(ushort *iX, ushort *iY);
void Cls(void);
void PutCh(const unsigned char c);
void PutCh(const unsigned char c, bool bScroll);
void PutS(const char *text);
bool InitializeVideo(void);
void FindCursor (ushort *x, ushort *y);

ushort VideoAttribs(ushort iAttrib);
ushort VideoAttribs(void);
void SetTextColor(unsigned char forecolor, unsigned char backcolor);

void SetCursorSize(ushort iTop, ushort iBottom);
void ShowCursor(void);
void HideCursor(void);

void MoveCursorBack(void);
void MoveCursorForward(void);

void DrawHRule(void);

void ClearLine(ushort uX, ushort uY, ushort uLength);
void FillLine(ushort uX, ushort uY, unsigned char sChar, ushort uLength);

LPVIDEOSNAPSHOT TakeVideoSnapshot(void);
void RestoreVideoSnapshot(LPVIDEOSNAPSHOT pVS);
void RestoreAndDeleteVideoSnapshot(LPVIDEOSNAPSHOT pVS);
void DeleteVideoSnapshot(LPVIDEOSNAPSHOT pVS);

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#endif

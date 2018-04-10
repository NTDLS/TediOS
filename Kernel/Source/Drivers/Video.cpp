////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#ifndef _CVIDEO_C_
#define _CVIDEO_C_
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/*
    Video functions.
*/
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#include "../Lib/STDLib.H"

#include "System.H"
#include "Video.H"
#include "CMemory.h"
#include "CKeyboard.H"

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

VIDEOPROPERTIES VideoProps;

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void FillLine(ushort uX, ushort uY, unsigned char sChar, ushort uLength)
{
	GotoXY(uX, uY);

	for(ushort uPos = 0; uPos < uLength; uPos++)
    {
		PutCh(sChar, false);
	}
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void ClearLine(ushort uX, ushort uY, ushort uLength)
{
	GotoXY(uX, uY);

	for(ushort uPos = 0; uPos < uLength; uPos++)
    {
		PutCh(' ', false);
	}
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/*
    Scrolls the screen
*/
void ScrollScreen(void)
{
    unsigned blank, temp;

    /* A blank is defined as a space... we need to give it
    *  backcolor too */
    blank = 0x20 | (VideoProps.iAttrib << 8);

    /* Row VideoProps.iMaxHeight is the end, this means we need to Scroll up */
    if(VideoProps.Y >= VideoProps.iMaxHeight)
    {
        /* Move the current text chunk that makes up the screen
        *  back in the buffer by a line */
        temp = VideoProps.Y - VideoProps.iMaxHeight + 1;
        memcpy(VideoProps.pVidMem,
            VideoProps.pVidMem + temp * VideoProps.iMaxWidth,
            (VideoProps.iMaxHeight - temp) * VideoProps.iMaxWidth * 2);

        /* Finally, we set the chunk of memory that occupies
        *  the last line of text to our 'blank' character */
        memsetw(VideoProps.pVidMem + (VideoProps.iMaxHeight - temp)
            * VideoProps.iMaxWidth, blank, VideoProps.iMaxWidth);

        VideoProps.Y = VideoProps.iMaxHeight - 1;
    }
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

/* Reads the current hardware cursor position into (*X,*Y). */
void FindCursor(ushort *x, ushort *y)
{
  /* See [FREEVGA] under "Manipulating the Text-mode Cursor". */
  ushort cp;

  outportb (0x3d4, 0x0e);
  cp = inportb (0x3d5) << 8;

  outportb (0x3d4, 0x0f);
  cp |= inportb (0x3d5);

  *x = cp % VideoProps.iMaxWidth;
  *y = cp / VideoProps.iMaxWidth;
}


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void GetXY(ushort *iX, ushort *iY)
{
    *iX = VideoProps.X;
    *iY = VideoProps.Y;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void GotoXY(ushort iX, ushort iY)
{
    VideoProps.X = iX;
    VideoProps.Y = iY;
    MoveCursor();
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/*
    Updates the hardware cursor: the little blinking line
        on the screen under the last character pressed.
*/
void MoveCursor(void)
{
    unsigned temp;

    /* The equation for finding the index in a linear
    *  chunk of memory can be represented by:
    *  Index = [(y * width) + x] */
    temp = VideoProps.Y * VideoProps.iMaxWidth + VideoProps.X;

    /* This sends a command to indicies 14 and 15 in the
    *  CRT Control Register of the VGA controller. These
    *  are the high and low bytes of the index that show
    *  where the hardware cursor is to be 'blinking'. To
    *  learn more, you should look up some VGA specific
    *  programming documents. A great start to graphics:
    *  http://www.brackeen.com/home/vga */
    outportb(0x3D4, 14);
    outportb(0x3D5, temp >> 8);
    outportb(0x3D4, 15);
    outportb(0x3D5, temp);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/*
    Clears the screen
*/
void Cls(void)
{
    //Again, we need the 'short' that will be used to represent a space with color.
    ushort uBlank = 0x20 | (VideoProps.iAttrib << 8);

    //Sets the entire screen to spaces in our current color.
    for(ushort uPos = 0; uPos < VideoProps.iMaxHeight; uPos++)
	{
        memsetw(VideoProps.pVidMem + uPos * VideoProps.iMaxWidth, uBlank, VideoProps.iMaxWidth);
	}

    //Update our virtual cursor, and then move the hardware cursor.
    VideoProps.X = 0;
    VideoProps.Y = 0;
    MoveCursor();
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/*
    Moves the cursor one forward.
*/
void MoveCursorForward(void)
{
    VideoProps.X++;

    //If the cursor has reached the edge of the screen's width, we insert a new line in there
    if(VideoProps.X >= VideoProps.iMaxWidth)
    {
        VideoProps.X = 0;
        VideoProps.Y++;
    }

    ScrollScreen();
    MoveCursor();
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/*
    Moves the cursor one back.
*/
void MoveCursorBack(void)
{
    VideoProps.X--;

    if(VideoProps.X < 0)
    {
        VideoProps.X = VideoProps.iMaxWidth;
        VideoProps.Y--;
    }

    ScrollScreen();
    MoveCursor();
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/*
    Draws a single character on the screen and moves the cursor.
*/
void PutCh(const unsigned char c)
{
	return PutCh(c, true);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/*
    Draws a single character on the screen and moves the cursor.
*/
void PutCh(const unsigned char c, bool bScroll)
{
    ushort *where;
    unsigned att = VideoProps.iAttrib << 8;

    /* Handle a backspace, by moving the cursor back one space */
    if(c == '\b')
    {
        if(VideoProps.X != 0) VideoProps.X--;
    }
    /* Handles a tab by incrementing the cursor's x, but only
    *  to a point that will make it divisible by 4 */
    else if(c == '\t')
    {
        VideoProps.X = (VideoProps.X + 4) & ~(4 - 1);
    }
    /* Handles a 'Carriage Return', which simply brings the
    *  cursor back to the margin */
    else if(c == '\r')
    {
        VideoProps.X = 0;
    }
    /* We handle our newlines the way DOS and the BIOS do: we
    *  treat it as if a 'CR' was also there, so we bring the
    *  cursor to the margin and we increment the 'y' value */
    else if(c == '\n')
    {
        VideoProps.X = 0;
        VideoProps.Y++;
    }
    /* Any character greater than and including a space, is a
    *  printable character. The equation for finding the index
    *  in a linear chunk of memory can be represented by:
    *  Index = [(y * width) + x] */
    else if(c >= ' ')
    {
        where = VideoProps.pVidMem + (VideoProps.Y * VideoProps.iMaxWidth + VideoProps.X);
        *where = c | att;	/* Character AND attributes: color */
        VideoProps.X++;
    }

    /* If the cursor has reached the edge of the screen's width, we
    *  insert a new line in there */
    if(VideoProps.X >= VideoProps.iMaxWidth)
    {
        VideoProps.X = 0;
        VideoProps.Y++;
    }

    if(bScroll)
	{
		ScrollScreen(); //Scroll the screen if needed, and finally move the cursor
	}

    MoveCursor();
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/*
    Uses the above routine to output a string...
*/
void PutS(const char *text)
{
    int i;

    for (i = 0; i < strlen((char *)text); i++)
    {
        PutCh(text[i]);
    }
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

ushort VideoAttribs(void)
{
    return VideoProps.iAttrib;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

ushort VideoAttribs(ushort iAttrib)
{
    VideoProps.iAttrib = iAttrib;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/*
    Sets the forecolor and backcolor that we will use
*/
void SetTextColor(unsigned char uForeColor, unsigned char uBackColor)
{
    /* Top 4 bytes are the background, bottom 4 bytes are the foreground color */
    VideoProps.iAttrib = (uBackColor << 4) | (uForeColor & 0x0F);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

LPVIDEOSNAPSHOT TakeVideoSnapshot(void)
{
	LPVIDEOSNAPSHOT pVS = (LPVIDEOSNAPSHOT)calloc(sizeof(VIDEOSNAPSHOT), 1);
	pVS->pVideoRAM = (byte *) calloc(sizeof(byte), VideoProps.ulPageRAM);
	memcpy(pVS->pVideoRAM, VideoProps.pVidMem, VideoProps.ulPageRAM);
	memcpy(&pVS->vpProps, &VideoProps, sizeof(VIDEOSNAPSHOT));
	return pVS;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void RestoreVideoSnapshot(LPVIDEOSNAPSHOT pVS)
{
	memcpy(VideoProps.pVidMem, pVS->pVideoRAM, VideoProps.ulPageRAM);
	memcpy(&VideoProps, &pVS->vpProps, sizeof(VIDEOSNAPSHOT));
	MoveCursor();
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void RestoreAndDeleteVideoSnapshot(LPVIDEOSNAPSHOT pVS)
{
	memcpy(VideoProps.pVidMem, pVS->pVideoRAM, VideoProps.ulPageRAM);
	memcpy(&VideoProps, &pVS->vpProps, sizeof(VIDEOSNAPSHOT));
	MoveCursor();
	DeleteVideoSnapshot(pVS);
}


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void DeleteVideoSnapshot(LPVIDEOSNAPSHOT pVS)
{
	Mem->Free(pVS->pVideoRAM);
	Mem->Free(pVS);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void SetCursorSize(ushort iTop, ushort iBottom)
{
	outportb(VideoProps.iCRTCAddress, 10);
	outportb(VideoProps.iCRTCAddress + 1, iTop);
	outportb(VideoProps.iCRTCAddress, 11);
	outportb(VideoProps.iCRTCAddress + 1, iBottom);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void ShowCursor(void)
{
    SetCursorSize(14, 14);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void HideCursor(void)
{
    SetCursorSize(32, 0);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/*
    Sets our text-mode VGA pointer, then clears the screen for us
*/
bool InitializeVideo(void)
{
    memset(&VideoProps, 0, sizeof(VIDEOPROPERTIES));

	if((inportb(VGA_MISC_READ) & 0x01) != 0)
	{
        VideoProps.iBasicType = VGA_TYPE_COLOR;
		VideoProps.pVidMem = (ushort *)0xB8000L; //Color Video Address.
		VideoProps.iCRTCAddress = 0x3d4;
    	VideoProps.iIndex_Port  = 0x3b4;
    	VideoProps.iValue_Port  = 0x3b5;
    	VideoProps.iMode_Port   = 0x3b8;
    	VideoProps.iStatus_Port = 0x3ba;
    	VideoProps.iGFX_Port    = 0x3bf;

		VideoProps.iMaxWidth = *(ushort *)0x44A;
		VideoProps.iMaxHeight = *(unsigned char *)0x484 + 1;
	    VideoProps.iAttrib = 0x0F;

		VideoProps.ulVideoRAM = (32 * 1024);
		VideoProps.ulPageRAM = (VideoProps.iMaxWidth * VideoProps.iMaxHeight) * 2;
	}
	else { //Unsupported.
        VideoProps.iBasicType = VGA_TYPE_MONO;
		VideoProps.pVidMem = (ushort *)0xB0000L; //Monochrome Video Address.
		VideoProps.iMaxWidth = *(ushort *)0x44A;
		VideoProps.iMaxHeight = *(unsigned char *)0x484 + 1;
		//VideoProps.iCRTCAddress = 0x3B4;
		Cls();
		BSOD("Unsupported video card");
	}

    Cls();
    HideCursor();

	//printf("%s video, Resolution: %u x %u, Framebuffer: 0x%lX\n",
    //    (VideoProps.iBasicType == VGA_TYPE_COLOR) ? "Color" : "Mono",
	//	VideoProps.iMaxHeight, VideoProps.iMaxWidth, VideoProps.pVidMem);

    //CMOSSleep(5);
    //Cls();
    //HideCursor();

	return true;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#endif

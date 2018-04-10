////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#ifndef _Window_C_
#define _Window_C_
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/*
    Window functions.
*/
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#include "../Lib/STDLib.H"

#include "System.H"
#include "Video.H"
#include "CMemory.h"
#include "CKeyboard.H"
#include "Window.H"
#include "Timer.h"

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void DrawWindow(const char *sTitle, ushort uLeft, ushort uTop, ushort uWidth, ushort uBodyHeight)
{
	return DrawWindow(sTitle, uLeft, uTop, uWidth, uBodyHeight, NULL);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void DrawWindow(const char *sTitle, ushort uLeft, ushort uTop, ushort uWidth, ushort uBodyHeight, unsigned char cFill)
{
	ushort uVideoAttribs = VideoAttribs();

	int iTitleLength = strlen(sTitle);

	SetTextColor(WHITE, GREEN);
	
	//Draw body:
	DrawSingleBox(uLeft, uTop + 2, uWidth, uBodyHeight);

	if(!cFill)
	{
		cFill = ' ';
	}

	for(ushort uY = 0; uY < uBodyHeight - 2; uY++)
	{
		FillLine(uLeft + 1, uTop + 3 + uY, cFill, uWidth - 2);
	}

	//Draw top line (title bar):
	GotoXY(uLeft, uTop);
	PutCh(CHAR_SNGL_TOP_LEFT, false);
	for(ushort iX = uLeft; iX < uLeft + (uWidth - 2); iX++)
	{
		PutCh(CHAR_SNGL_HORIZONTAL, false);
	}
	GotoXY(uLeft + uWidth - 1, uTop);
	PutCh(CHAR_SNGL_TOP_RIGHT, false);

	//Draw left side of title bar:
	GotoXY(uLeft, uTop + 1);
	PutCh(CHAR_SNGL_VERTICAL, false);

	//Draw title:
	ClearLine(uLeft + 1, uTop + 1, uWidth - 2); //Set the title color.

	int uTitleStart = (uLeft + (uWidth / 2)) - (iTitleLength / 2);
	if(uTitleStart <= uLeft)
	{
		uTitleStart = uLeft+1;
	}

	GotoXY(uTitleStart, uTop + 1);
	for(ushort uPos = 0; uPos < iTitleLength && uPos < (uWidth - 2); uPos++)
	{
		if(iTitleLength > (uWidth - 2) && (uWidth - 2) - uPos <= 3)
		{
			PutCh('.');
		}
		else{
			PutCh(sTitle[uPos]);
		}
	}

	//Draw right side of title bar:
	GotoXY(uLeft + uWidth - 1, uTop + 1);
	PutCh(CHAR_SNGL_VERTICAL, false);

	//Replace upper corners of body with splits to merge into title bar.
	GotoXY(uLeft, uTop + 2);
	PutCh(CHAR_SNGL_RIGHT_H_SPLIT, false);
	GotoXY(uLeft + uWidth - 1, uTop + 2);
	PutCh(CHAR_SNGL_LEFT_H_SPLIT, false);

	//Set cursor to first line in the body and reset the text color.
	GotoXY(uLeft + 1, uTop + 3);
	VideoAttribs(uVideoAttribs);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void DrawSingleHLineAt(ushort uX, ushort uY, ushort uLength)
{
	GotoXY(uX, uY);

	for(ushort uPos = 0; uPos < uLength; uPos++)
    {
		PutCh(CHAR_SNGL_HORIZONTAL, false);
	}
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void DrawDoubleHLineAt(ushort uX, ushort uY, ushort uLength)
{
	GotoXY(uX, uY);

	for(ushort uPos = 0; uPos < uLength; uPos++)
    {
		PutCh(CHAR_DBL_HORIZONTAL, false);
	}
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void DrawHRule(void)
{
    DrawSingleHLineAt(VideoProps.X, VideoProps.Y, VideoProps.iMaxWidth);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void DrawSingleVLineAt(ushort uX, ushort uY, ushort uLength)
{
	GotoXY(uX, uY++);
	for(int uPos = 0; uPos < uLength; uPos++)
    {
		PutCh(CHAR_SNGL_VERTICAL, false);
		GotoXY(uX, uY++);
	}
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void DrawDoubleVLineAt(ushort uX, ushort uY, ushort uLength)
{
	GotoXY(uX, uY++);
	for(int uPos = 0; uPos < uLength; uPos++)
    {
		PutCh(CHAR_DBL_VERTICAL, false);
		GotoXY(uX, uY++);
	}
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void DrawParentWindow(const char *sText)
{
	DrawParentWindow(0, 0, VideoProps.iMaxWidth, VideoProps.iMaxHeight, sText);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void DrawParentWindow(ushort uX, ushort uY,
				ushort uWidth, ushort uHeight, const char *sText)
{
	DrawDoubleBox(uX, uY, uWidth, uHeight);

	GotoXY(uX, uY+2);
	PutCh(CHAR_DBL_LEFT_H_SPLIT, false);
	
	GotoXY((uX + uWidth) - 1, uY+2);
	PutCh(CHAR_DBL_RIGHT_H_SPLIT, false);

	DrawDoubleHLineAt(uX+1, uY+2, uWidth-2);

	GotoXY(uX + (uWidth / 2) - (strlen(sText) / 2), uY+1);
	printf("%s", sText);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void DrawSingleBox(ushort uX, ushort uY, ushort uWidth, ushort uHeight)
{
	DrawSingleHLineAt(uX+1, uY, uWidth-2);
	DrawSingleVLineAt(uX, uY+1, uHeight-2);
	DrawSingleHLineAt(uX+1, (uY+uHeight)-1, uWidth-2);
	DrawSingleVLineAt((uX+uWidth) - 1, uY+1, uHeight-2);

	GotoXY(uX, uY);
	PutCh(CHAR_SNGL_TOP_LEFT, false);
	GotoXY((uX + uWidth)-1, uY);
	PutCh(CHAR_SNGL_TOP_RIGHT, false);

	GotoXY(uX, (uY+uHeight)-1);
	PutCh(CHAR_SNGL_BOT_LEFT, false);
	GotoXY((uX + uWidth)-1, (uY+uHeight)-1);
	PutCh(CHAR_SNGL_BOT_RIGHT, false);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void DrawDoubleBox(ushort uX, ushort uY, ushort uWidth, ushort uHeight)
{
	DrawDoubleHLineAt(uX+1, uY, uWidth-2);
	DrawDoubleVLineAt(uX, uY+1, uHeight-2);
	DrawDoubleHLineAt(uX+1, (uY+uHeight)-1, uWidth-2);
	DrawDoubleVLineAt((uX+uWidth) - 1, uY+1, uHeight-2);

	GotoXY(uX, uY);
	PutCh(CHAR_DBL_TOP_LEFT, false);
	GotoXY((uX + uWidth)-1, uY);
	PutCh(CHAR_DBL_TOP_RIGHT, false);

	GotoXY(uX, (uY+uHeight)-1);
	PutCh(CHAR_DBL_BOT_LEFT, false);
	GotoXY((uX + uWidth)-1, (uY+uHeight)-1);
	PutCh(CHAR_DBL_BOT_RIGHT, false);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#endif

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#ifndef _Window_H_
#define _Window_H_
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/*
    Window functions.
*/
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


#define CHAR_SNGL_TOP_RIGHT		(unsigned char)191
#define CHAR_SNGL_TOP_LEFT		(unsigned char)218
#define CHAR_SNGL_BOT_RIGHT		(unsigned char)217
#define CHAR_SNGL_BOT_LEFT		(unsigned char)192
#define CHAR_SNGL_HORIZONTAL	(unsigned char)196
#define CHAR_SNGL_VERTICAL		(unsigned char)179
#define CHAR_SNGL_RIGHT_H_SPLIT	(unsigned char)195
#define CHAR_SNGL_LEFT_H_SPLIT	(unsigned char)180

#define CHAR_DBL_TOP_RIGHT		(unsigned char)187
#define CHAR_DBL_TOP_LEFT		(unsigned char)201
#define CHAR_DBL_BOT_RIGHT		(unsigned char)188
#define CHAR_DBL_BOT_LEFT		(unsigned char)200
#define CHAR_DBL_HORIZONTAL		(unsigned char)205
#define CHAR_DBL_VERTICAL		(unsigned char)186
#define CHAR_DBL_RIGHT_H_SPLIT	(unsigned char)185
#define CHAR_DBL_LEFT_H_SPLIT	(unsigned char)204

#define CHAR_PROGRESS_BLOCK		(unsigned char)177


void MessageBox(const char *sText);

void DrawDoubleBox(ushort uX, ushort uY, ushort uWidth, ushort uHeight);
void DrawSingleBox(ushort uX, ushort uY, ushort uWidth, ushort uHeight);
void DrawDoubleVLineAt(ushort uX, ushort uY, ushort uLength);
void DrawSingleVLineAt(ushort uX, ushort uY, ushort uLength);
void DrawSingleHLineAt(ushort uX, ushort uY, ushort uLength);
void DrawDoubleHLineAt(ushort uX, ushort uY, ushort uLength);

void DrawWindow(const char *sTitle, ushort uLeft, ushort uTop, ushort uWidth, ushort uBodyHeight);
void DrawWindow(const char *sTitle, ushort uLeft, ushort uTop, ushort uWidth, ushort uBodyHeight, unsigned char cFill);

void DrawParentWindow(const char *sText);
void DrawParentWindow(ushort uX, ushort uY,
				ushort uWidth, ushort uHeight, const char *sText);

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#endif

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#ifndef _CKeyboard_C_
#define _CKeyboard_C_
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/*
    Keyboard driver
*/

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//--BEGIN NEEDED INCLUDES---

#include "../Lib/STDLib.H"

#include "Video.H"
#include "CKeyboard.H"
#include "IRQ.H"

//----END NEEDED INCLUDES---

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

CKeyboard *KBD = NULL;
KEYBOARDBUFFER KBuf;

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

//KBDUS means US Keyboard Layout. Scancode table:
static char ucUSA_KB[128][2] = {
	{'\0'     },  // 0
	{27,    27},  // 1 - ESC
	{'1',   '!'},{'2',   '@'},{'3',   '#'},{'4',   '$'},
	{'5',   '%'},{'6',   '^'},{'7',   '&'},{'8',   '*'},
	{'9',   '('},{'0',   ')'},{'-',   '_'},{'=',   '+'},
	{'\b', '\n'}, // 14 - Backspace
	{'\t', '\t'},{'q',   'Q'},{'w',   'W'},{'e',   'E'},
	{'r',   'R'},{'t',   'T'},{'y',   'Y'},{'u',   'U'},
	{'i',   'I'},{'o',   'O'},{'p',   'P'},{'[',   '{'},
	{']',   '}'}, // 27
	{'\r', '\r'}, // 28 - Enter
	{'\0', '\0'}, // 29 - Ctrl
	{'a',   'A'}, // 30
	{'s',   'S'},{'d',   'D'},{'f',   'F'},{'g',   'G'},
	{'h',   'H'},{'j',   'J'},{'k',   'K'},{'l',   'L'},
	{';',   ':'},{'\'',  '"'},{'`',   '~'},
	{'\0', '\0'}, // 42 - Left Shift
	{'\\',  '|'}, // 43
	{'z',   'Z'}, // 44
	{'x',   'X'},{'c',   'C'},{'v',   'V'},{'b',   'B'},
	{'n',   'N'},{'m',   'M'},{',',   '<'},{'.',   '>'},
	{'/',   '?'}, // 53
	{'\0', '\0'}, // 54 - Right Shift
	{'\0', '\0'}, // 55 - Print Screen
	{'\0', '\0'}, // 56 - Alt
	{' ',	' '}, // 57 - Space bar
	{'\0', '\0'}, // 58 - Caps Lock
	{'\0', '\0'},{'\0', '\0'},{'\0', '\0'},{'\0', '\0'},{'\0', '\0'}, //F1 - F5
	{'\0', '\0'},{'\0', '\0'},{'\0', '\0'},{'\0', '\0'},{'\0', '\0'}, //F6 - F10
	{'\0', '\0'}, // 69 - Num Lock
	{'\0', '\0'}, // 70 - Scroll Lock
	{'7',	'7'}, // 1 - Numeric keypad 7
	{'8',	'8'}, // 2 - Numeric keypad 8
	{'9',	'9'}, // 3 - Numeric keypad 9
	{'-',	'-'}, // 4 - Numeric keypad '-'
	{'4',	'4'}, // 5 - Numeric keypad 4
	{'5',	'5'}, // 6 - Numeric keypad 5
	{'6',	'6'}, // 7 - Numeric keypad 6
	{'+',	'+'}, // 8 - Numeric keypad '+'
	{'1',	'1'}, // 9 - Numeric keypad 1
	{'2',	'2'}, // 0 - Numeric keypad 2
	{'3',	'3'}, // 1 - Numeric keypad 3
	{'0',	'0'}, // 2 - Numeric keypad 0
	{'.',	'.'}, // 3 - Numeric keypad '.'
};

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void CKeyboard::WritePort(uint uiAddress, uint uiData)
{
    uint ulTimeout = 0;

	for(ulTimeout = 500000L; ulTimeout != 0; ulTimeout--)
	{
		//uint stat = inportb(0x64);
		if((inportb(K_STATUS) & 0x02) == 0) // loop until 8042 input buffer empty.
        {
			break;
        }
	}

	if(ulTimeout != 0)
    {
		outportb(uiAddress, uiData);
    }
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

char *CKeyboard::GetText(char *sOutBuf, int iMaxSize, int *iOutSize)
{
    KEYPRESS KeyPress;

    bool bInsertMode = false;

    int iPos = 0;
    int iLength = 0;
    int iGetsCachePos = KBuf.iGetsCacheCount;

    *iOutSize = 0;
    memset(&KeyPress, 0, sizeof(KEYPRESS));
    memset(sOutBuf, 0, iMaxSize);

    ShowCursor();

    while(true)
    {
        GetNextKey(&KeyPress);

		if(iPos == iMaxSize
			&& KeyPress.iScanCode != KEY_BACKSPACE
			&& KeyPress.iScanCode != KEY_RETURN)
		{
			continue;
		}

		if(KBuf.bIsShiftDown)
        {
            if(KeyPress.iScanCode == KEY_ARROWUP)
            {
                //Previous command.
                if(iGetsCachePos > 0)
                {
                    iGetsCachePos--;
    
                    //Erase chars past our current cursor pos.
                    for(;iPos < iLength; iPos++)
                    {
                        PutCh(' ');
                    }
    
                    for(;iPos > 0; iPos--)
                    {
                        PutS("\b \b");
                    }
    
                    PutS(KBuf.sGetsCache[iGetsCachePos]);
                    //printf("%d : [%s]\n", iGetsCachePos, KBuf.sGetsCache[iGetsCachePos]);
                    iLength = strlen(KBuf.sGetsCache[iGetsCachePos]);
                    strcpy(sOutBuf, KBuf.sGetsCache[iGetsCachePos]);
                    iPos = iLength;
                }
                continue;
            }
            else if(KeyPress.iScanCode == KEY_ARROWDOWN)
            {
                //Next command.
                if(KBuf.iGetsCacheCount > iGetsCachePos)
                {
                    iGetsCachePos++;
    
                    //Erase chars past our current cursorpos.
                    for(;iPos < iLength; iPos++)
                    {
                        PutCh(' ');
                    }
    
                    for(;iPos > 0; iPos--)
                    {
                        PutS("\b \b");
                    }
    
                    if(KBuf.iGetsCacheCount != iGetsCachePos)
                    {
                        PutS(KBuf.sGetsCache[iGetsCachePos]);
                        //printf("%d : [%s]\n", iGetsCachePos, KBuf.sGetsCache[iGetsCachePos]);
                        iLength = strlen(KBuf.sGetsCache[iGetsCachePos]);
                        strcpy(sOutBuf, KBuf.sGetsCache[iGetsCachePos]);
                        iPos = iLength;
                    }
                }
                continue;
            }
            else if(KeyPress.iScanCode == KEY_ARROWLEFT)
            {
                if(iPos > 0)
                {
                    MoveCursorBack();
                    iPos--;
                }
                continue;
            }
            else if(KeyPress.iScanCode == KEY_ARROWRIGHT)
            {
                if(iPos < iLength)
                {
                    MoveCursorForward();
                    iPos++;
                }
                continue;
            }
            else if(KeyPress.iScanCode == KEY_INSERT)
            {
                if((bInsertMode = !bInsertMode))
                {
                    SetCursorSize(12, 14);
                }
                else{
                    ShowCursor();
                }
                continue;
            }
            else if(KeyPress.iScanCode == KEY_DELETE)
            {
                if(iPos < iLength)
                {
                    for(int iRep = iPos + 1; iRep < iLength; iRep++)
                    {
                        PutCh(sOutBuf[iRep]);
                        sOutBuf[iRep-1] = sOutBuf[iRep];
                    }
                    PutCh(' ');
                    for(int iRep = iPos; iRep < iLength; iRep++)
                    {
                        PutCh('\b');
                    }

                    iLength--;

                    sOutBuf[iLength] = '\0';
                }
                continue;
            }
        }

        if(KeyPress.iScanCode == KEY_BACKSPACE)
        {
            if(iPos > 0)
            {
                if(iPos == iLength)
                {
                    iPos--;
                    iLength--;

                    sOutBuf[iPos] = '\0';
                    PutS("\b \b");
                }
                else if(iPos < iLength)
                {
                    iPos--;
                    PutS("\b");

                    for(int iRep = iPos + 1; iRep < iLength; iRep++)
                    {
                        PutCh(sOutBuf[iRep]);
                        sOutBuf[iRep-1] = sOutBuf[iRep];
                    }
                    PutCh(' ');
                    for(int iRep = iPos; iRep < iLength; iRep++)
                    {
                        PutCh('\b');
                    }

                    iLength--;

                    sOutBuf[iLength] = '\0';
                }
            }
        }
        else if(KeyPress.iScanCode == KEY_RETURN)
        {
            sOutBuf[iLength] = '\0';
            if(iLength > 0)
            {
                if(KBuf.iGetsCacheCount < GETS_CACHE_SIZE )
                {
                    strcpy(KBuf.sGetsCache[KBuf.iGetsCacheCount++], sOutBuf);
                }
                else{
                    for(int iCache = 0; iCache < KBuf.iGetsCacheCount - 1; iCache++)
                    {
                        memset(KBuf.sGetsCache[iCache], 0, sizeof(KBuf.sGetsCache[iCache]));
                        strcpy(KBuf.sGetsCache[iCache], KBuf.sGetsCache[iCache + 1]);
                    }
                    strcpy(KBuf.sGetsCache[KBuf.iGetsCacheCount - 1], sOutBuf);
                }
            }
            break;
        }
        else if(KeyPress.cCharacter != 0)
        {
            if(iPos == iLength)
            {
                iLength++;
            }

            PutCh(KeyPress.cCharacter);

            if(bInsertMode && iPos < iLength)
            {
                for(int iRep = iPos; iRep < iLength; iRep++)
                {
                    PutCh(sOutBuf[iRep]);
                }

                for(int iRep = iLength + 1; iRep > iPos; iRep--)
                {
                    sOutBuf[iRep] = sOutBuf[iRep - 1];
                    PutCh('\b');
                }

                sOutBuf[iPos] = KeyPress.cCharacter;
                iLength++;
                sOutBuf[iLength] = '\0';
            }
            else{
                sOutBuf[iPos] = KeyPress.cCharacter;
                iPos++;
            }
        }
        else{
            //Non-printable character.
        }
        __asm__ __volatile__ ("nop");
    }

    *iOutSize = iLength;

    return sOutBuf;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

/*
char *CKeyboard::gets(char *sOutBuf, int iMaxSize, int *iOutSize)
{
    KEYPRESS KeyPress;

    for(int iPos = 0; iPos < iMaxSize; iPos++)
    {
        GetNextKey(&KeyPress);

        if(KeyPress.iScanCode == KEY_BACKSPACE)
        {
            if(iPos > 0)
            {
                iPos--;
                sOutBuf[iPos] = '\0';
                iPos--;

                *iOutSize = iPos;

                PutCh(KeyPress.cCharacter); //Backspace.
                PutCh(' '); //Replace with space.
                PutCh(KeyPress.cCharacter); //Backspace again.
            }
            else{
                iPos--;
            }
        }
        else if(KeyPress.iScanCode == KEY_RETURN)
        {
            sOutBuf[iPos] = '\0';
            break;
        }
        else if(KeyPress.cCharacter != 0)
        {
            sOutBuf[iPos] = KeyPress.cCharacter;
            *iOutSize = iPos;

            PutCh(KeyPress.cCharacter);
        }
        else{
            iPos--; //Non-printable character.
        }
        __asm__ __volatile__ ("nop");
    }

    return sOutBuf;
}
*/

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void CKeyboard::GetNextKey(KEYPRESS *pKeyPress)
{   
    while(!GetLastKey(pKeyPress))
    {
        __asm__ __volatile__ ("nop");
    }
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void CKeyboard::Pause(const char *sText)
{
	printf(sText);
	KEYPRESS kp;
	GetNextKey(&kp);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void CKeyboard::Pause(void)
{
	Pause("Press any key to continue...\n");
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool CKeyboard::GetLastKey(KEYPRESS *pKeyPress)
{
    if(KBuf.bBusy == false)
    {
        if(KBuf.iCurrentChars > 0)
        {
            KBuf.iCurrentChars--;
            memcpy(pKeyPress, &KBuf.KeyPress[0], sizeof(KEYPRESS));

            for(int iPos = 0; iPos < (KBuf.iMaxBuffer - 1); iPos++)
            {
                memcpy(&KBuf.KeyPress[iPos], &KBuf.KeyPress[iPos+1], sizeof(KEYPRESS));
            }
            return true;
        }
        else{
            return false;
        }
    }
    else{
        return false;
    }
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool CKeyboard::UpdateLEDs(void)
{
	this->WritePort(K_RDWR, K_CMD_LEDS);	// "set LEDs" command
	uint uiStatus = 0; //All LEDs off.

	if(KBuf.bScrollLock)
    {
		uiStatus |= 1;
    }
	if(KBuf.bNumLock)
    {
		uiStatus |= 2;
    }
	if(KBuf.bCapsLock)
    {
		uiStatus |= 4;
    }

	this->WritePort(K_RDWR, uiStatus); //Bottom 3 bits set LEDs
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/*
    Handles the keyboard interrupt
*/
void /*CKeyboard::*/Keyboard_Handler(struct regs *r)
{
    while(KBuf.bBusy)
    {
        __asm__ __volatile__ ("nop");
    }

    KBuf.bBusy = true;

    //Read from the keyboard's data buffer */
    unsigned char iScanCode = inportb(K_RDWR);

    // If the top bit of the byte we read from the keyboard is
    //  set, that means that a key has just been released
    if (iScanCode & 0x80)
    {
        if(iScanCode == (KEY_CTRL + 128))
        {
            KBuf.bIsCtrlDown = false;
        }
        else if(iScanCode == (KEY_LSHIFT + 128) || iScanCode == (KEY_RSHIFT + 128))
        {
            KBuf.bIsShiftDown = false;
        }
    }
    else {

    	if(iScanCode == KEY_CAPSLOCK || iScanCode == KEY_NUMLOCK || iScanCode == KEY_SCROLLLOCK)
    	{
        	if(iScanCode == KEY_CAPSLOCK)
        	{
                KBuf.bCapsLock = !KBuf.bCapsLock;
            }
        	else if(iScanCode == KEY_NUMLOCK)
        	{
                KBuf.bNumLock = !KBuf.bNumLock;
            }
        	else if(iScanCode == KEY_SCROLLLOCK)
        	{
                KBuf.bScrollLock = !KBuf.bScrollLock;
            }

            KBD->UpdateLEDs();
    	}
		else if(iScanCode == KEY_F5)
        {
            KBuf.bInDebugMode = !KBuf.bInDebugMode; //Toggle Keyboard driver debug mode.
            if(KBuf.bInDebugMode)
            {
                printf("\n**Entering");
            }
            else{
                printf("\n**Leaving");
            }
            printf(" keyboard debug mode**\n");
        }
        else if(iScanCode == KEY_F6)
        {
			//uint long ulAddress = 2147483648u;
			//printf("%u\n", ulAddress);
        }
        else if(iScanCode == KEY_F7)
        {
            ShowCursor();
        }
        else if(iScanCode == KEY_LSHIFT || iScanCode == KEY_RSHIFT)
        {
            KBuf.bIsShiftDown = true;
        }
        else if(iScanCode == KEY_CTRL)
        {
            KBuf.bIsCtrlDown = true;
        }

        unsigned char ucChar = 0;

        if(KBuf.bIsShiftDown)
        {
            ucChar = ucUSA_KB[iScanCode][1];
        }
        else{
            ucChar = ucUSA_KB[iScanCode][0];
        }

        if(KBuf.bIsCtrlDown && (ucChar == 'C' || ucChar == 'c'))
        {
            KBuf.bSoftTerm = true;
        }

        if(KBuf.bCapsLock && ((ucChar >= 'a' && ucChar <= 'z') || (ucChar >= 'A' && ucChar <= 'Z')))
        {
            if(KBuf.bIsShiftDown)
            {
                ucChar = ucUSA_KB[iScanCode][0];
            }
            else{
                ucChar = ucUSA_KB[iScanCode][1];
            }
        }

        if(KBuf.iCurrentChars == KBuf.iMaxBuffer)
        {
            for(int iPos = 0; iPos < (KBuf.iMaxBuffer - 1); iPos++)
            {
                memcpy(&KBuf.KeyPress[iPos], &KBuf.KeyPress[iPos+1], sizeof(KEYPRESS));
            }
            KBuf.iCurrentChars--;
        }

        KBuf.KeyPress[KBuf.iCurrentChars].cCharacter = ucChar;
        KBuf.KeyPress[KBuf.iCurrentChars].iScanCode = iScanCode;

        if(KBuf.bInDebugMode)
        {
            printf("Char: [%d], ScanCode: [%d]\n", ucChar, iScanCode);
        }

        KBuf.iCurrentChars++;
    }

    KBuf.bBusy = false;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool CKeyboard::KillCommand(void)
{
    return KBuf.bSoftTerm;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool CKeyboard::KillCommand(bool bValue)
{
	bool bOldValue = KBuf.bSoftTerm;
    KBuf.bSoftTerm = bValue;
	return bOldValue;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

/*
    Installs the keyboard handler into IRQ1
*/
bool CKeyboard::Initialize(void)
{
    memset(&KBuf, 0, sizeof(KBuf));
    KBuf.iMaxBuffer = SOFT_BUFFER_SIZE;
    KBuf.iCurrentChars = 0;

    KBuf.bNumLock = true;
    this->UpdateLEDs();

    IRQ_Install_Handler(1, Keyboard_Handler);

    return true;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#endif

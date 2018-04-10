////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#ifndef _CKeyboard_H_
#define _CKeyboard_H_
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/*
    Keyboard driver
*/
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#include "../Lib/Types.h"

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#define SOFT_BUFFER_SIZE 16
#define GETS_CACHE_SIZE  10

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#define KEY_RETURN       28
#define KEY_ESCAPE        1
#define KEY_BACKSPACE    14
#define KEY_TAB          15
#define KEY_SPACE        57

#define KEY_ARROWDOWN    80 // Will simulate a SHIFT press.
#define KEY_ARROWLEFT    75 // Will simulate a SHIFT press.
#define KEY_ARROWRIGHT   77 // Will simulate a SHIFT press.
#define KEY_ARROWUP      72 // Will simulate a SHIFT press.

#define KEY_DELETE       83 // Will simulate a SHIFT press.
#define KEY_INSERT       82 // Will simulate a SHIFT press.

#define KEY_LSHIFT       42
#define KEY_RSHIFT       54
#define KEY_CTRL         29

#define KEY_CAPSLOCK     58
#define KEY_NUMLOCK      69
#define KEY_SCROLLLOCK   70

#define KEY_F1           59
#define KEY_F2           60
#define KEY_F3           61
#define KEY_F4           62
#define KEY_F5           63
#define KEY_F6           64
#define KEY_F7           65
#define KEY_F8           66
#define KEY_F9           67
#define KEY_F10          68
#define KEY_F11          87
#define KEY_F12          88

#define KEY_A            30
#define KEY_B            48
#define KEY_C            46
#define KEY_D            32
#define KEY_E            18
#define KEY_F            33
#define KEY_G            34
#define KEY_H            35
#define KEY_I            23
#define KEY_J            36
#define KEY_K            37
#define KEY_L            38
#define KEY_M            50
#define KEY_N            49
#define KEY_O            24
#define KEY_P            25
#define KEY_Q            16
#define KEY_R            19
#define KEY_S            31
#define KEY_T            20
#define KEY_U            22
#define KEY_V            47
#define KEY_W            17
#define KEY_X            45
#define KEY_Y            21
#define KEY_Z            44

/*
 * Keyboard I/O ports.
 */
#define K_RDWR 		     0x60		/* keyboard data & cmds (read/write) */
#define K_STATUS 	     0x64		/* keybd status (read-only) */
#define K_CMD	 	     0x64		/* keybd ctlr command (write-only) */

/*
 * Bit definitions for K_STATUS port.
 */
#define K_OBUF_FUL 	     0x01		/* output (from keybd) buffer full */
#define K_IBUF_FUL 	     0x02		/* input (to keybd) buffer full */
#define K_SYSFLAG	     0x04		/* "System Flag" */
#define K_CMD_DATA	     0x08		/* 1 = input buf has cmd, 0 = data */
#define K_KBD_INHIBIT	 0x10		/* 0 if keyboard inhibited */
#define K_AUX_OBUF_FUL	 0x20		/* 1 = obuf holds aux device data */
#define K_TIMEOUT	     0x40		/* timout error flag */
#define K_PARITY_ERROR	 0x80		/* parity error flag */

/*
 * Keyboard controller commands (sent to K_CMD port).
 */
#define KC_CMD_READ	     0x20		/* read controller command byte */
#define KC_CMD_WRITE	 0x60		/* write controller command byte */
#define KC_CMD_DIS_AUX	 0xa7		/* disable auxiliary device */
#define KC_CMD_ENB_AUX	 0xa8		/* enable auxiliary device */
#define KC_CMD_TEST_AUX	 0xa9		/* test auxiliary device interface */
#define KC_CMD_SELFTEST	 0xaa		/* keyboard controller self-test */
#define KC_CMD_TEST	     0xab		/* test keyboard interface */
#define KC_CMD_DUMP	     0xac		/* diagnostic dump */
#define KC_CMD_DISABLE	 0xad		/* disable keyboard */
#define KC_CMD_ENABLE	 0xae		/* enable keyboard */
#define KC_CMD_RDKBD	 0xc4		/* read keyboard ID */
#define KC_CMD_WIN	     0xd0		/* read  output port */
#define KC_CMD_WOUT	     0xd1		/* write output port */
#define KC_CMD_ECHO	     0xee		/* used for diagnostic testing */
#define KC_CMD_PULSE	 0xff		/* pulse bits 3-0 based on low nybble */

/*
 * Keyboard commands (send to K_RDWR).
 */
#define K_CMD_LEDS	     0xed		/* set status LEDs (caps lock, etc.) */
#define K_CMD_TYPEMATIC	 0xf3		/* set key repeat and delay */

/*
 * Bit definitions for controller command byte (sent following
 * KC_CMD_WRITE command).
 *
 * Bits 0x02 and 0x80 unused, always set to 0.
 */
#define K_CB_ENBLIRQ	 0x01		/* enable data-ready intrpt */
#define K_CB_SETSYSF	 0x04		/* Set System Flag */
#define K_CB_INHBOVR	 0x08		/* Inhibit Override */
#define K_CB_DISBLE	     0x10		/* disable keyboard */
#define K_CB_IGNPARITY	 0x20		/* ignore parity from keyboard */
#define K_CB_SCAN	     0x40		/* standard scan conversion */

/*
 * Bit definitions for "Indicator Status Byte" (sent after a
 * K_CMD_LEDS command).  If the bit is on, the LED is on.  Undefined
 * bit positions must be 0.
 */
#define K_LED_SCRLLK	 0x1		/* scroll lock */
#define K_LED_NUMLK	     0x2		/* num lock */
#define K_LED_CAPSLK	 0x4		/* caps lock */

/*
 * Bit definitions for "Miscellaneous port B" (K_PORTB).
 */
/* read/write */
#define K_ENABLETMR2	 0x01		/* enable output from timer 2 */
#define K_SPKRDATA	     0x02		/* direct input to speaker */
#define K_ENABLEPRTB	 0x04		/* "enable" port B */
#define K_EIOPRTB	     0x08		/* enable NMI on parity error */
/* read-only */
#define K_REFRESHB	     0x10		/* refresh flag from INLTCONT PAL */
#define K_OUT2B		     0x20		/* timer 2 output */
#define K_ICKB		     0x40		/* I/O channel check (parity error) */

/*
 * Bit definitions for the keyboard controller's output port.
 */
#define KO_SYSRESET	     0x01		/* processor reset */
#define KO_GATE20	     0x02		/* A20 address line enable */
#define KO_AUX_DATA_OUT	 0x04		/* output data to auxiliary device */
#define KO_AUX_CLOCK	 0x08		/* auxiliary device clock */
#define KO_OBUF_FUL      0x10		/* keyboard output buffer full */
#define KO_AUX_OBUF_FUL	 0x20		/* aux device output buffer full */
#define KO_CLOCK	     0x40		/* keyboard clock */
#define KO_DATA_OUT      0x80		/* output data to keyboard */

/*
 * Keyboard return codes.
 */
#define K_RET_RESET_DONE 0xaa		/* BAT complete */
#define K_RET_ECHO		 0xee		/* echo after echo command */
#define K_RET_ACK		 0xfa		/* ack */
#define K_RET_RESET_FAIL 0xfc		/* BAT error */
#define K_RET_RESEND	 0xfe		/* resend request */

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

typedef struct _tag_Key_Press {
    unsigned char cCharacter;
    ushort iScanCode;
} KEYPRESS, *LPKEYPRESS;

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

typedef struct _tag_Keyboard_Buffer{
    KEYPRESS KeyPress[SOFT_BUFFER_SIZE];

    char sGetsCache[GETS_CACHE_SIZE][255];
    ushort iGetsCacheCount;

    ushort iMaxBuffer;
    ushort iCurrentChars;

    bool bIsShiftDown;
    bool bIsCtrlDown;
    bool bSoftTerm; //CTRL+C has been pressed.
    bool bBusy;

    bool bCapsLock;
    bool bScrollLock;
    bool bNumLock;

    bool bInDebugMode;

} KEYBOARDBUFFER, *LPKEYBOARDBUFFER;

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

class CKeyboard {
public:
	bool UpdateLEDs(void);
	void WritePort(uint uiAddress, uint uiData);
	friend void Keyboard_Handler(struct regs *r);
	bool Initialize(void);

	char *GetText(char *sOutBuf, int iMaxSize, int *iOutSize);
	void GetNextKey(KEYPRESS *pKeyPress);
	bool GetLastKey(KEYPRESS *pKeyPress);
	bool KillCommand(void);
	bool KillCommand(bool bValue);
	void Pause(void);
	void Pause(const char *sText);

private:
};

extern CKeyboard *KBD;

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#endif

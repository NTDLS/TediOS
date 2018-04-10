/*  IDE-INFO.C:
**  ===========
**
**  This is the C-source of the IDE-INFO program, which can show some info
**  of the IDE drives in your system. This code code is written for the
**  Borland C++ v3.1 compiler, but it may work with other compilers as well.
**
**  Compilation:    bcc -d -O ide-info.c
**
**  History:
**  --------
**	v1.00	(930901/RdK)	Initial version
**
**  However the program is free, the source-code isn't. Read the
**  documentation, which should be in the archive too. If you don't have
**  the documentation contact the author.
**
**  (C) 1993 Ramon de Klein
*/

#include <bios.h>
#include <conio.h>
#include <dos.h>
#include <limits.h>
#include <stddef.h>


/*
** This is a structure, which can hold an entry from the BIOS harddisk
** paramter table. Make sure word-alignment is OFF, because otherwise the
** BIOS reports will be incorrect.
*/

#pragma option -a-

struct hdparam {
    unsigned cylinders   : 16;	/* Number of cylinders                     */
    unsigned heads	 :  8;	/* Number of heads                         */
    unsigned reducedwr   : 16;	/* Starting reduced write current cylinder */
    unsigned precomp     : 16;	/* Starting write precomp cyl. number (XT) */
    unsigned maxECCburst :  8;	/* Maximum ECC burst (XT)                  */
    unsigned driveoption :  3;	/* Drive option (XT)                       */
    unsigned more8heads  :  1;	/* Set if more than 8 heads (AT+)          */
    unsigned zero	 :  1;	/* Always zero                             */
    unsigned hidefectmap :  1;	/* Manufact. defect map on maxcyl+1 (AT+)  */
    unsigned disECCretry :  1;	/* Disable ECC retries                     */
    unsigned disaccretry :  1;	/* Disable access retries                  */
    unsigned stdtimeout  :  8;	/* Standard timeout (XT)                   */
    unsigned fmttimeout  :  8;	/* Formatting timeout (XT)                 */
    unsigned chktimeout  :  8;	/* Timeout for checking drive (XT)         */
    unsigned landingzone : 16;	/* Cylinder number of landing zone (AT+)   */
    unsigned sectors     :  8;	/* Number of sectors per track (AT+)       */
    unsigned reserved    :  8;	/* Reserved				   */
};

struct partrec {
    unsigned char boot;		/* Boot indicator (80h = active partition) */
    unsigned char shead;  	/* Partition start head			   */
    unsigned char ssect;  	/* Partition start sector		   */
    unsigned char scyl;  	/* Partition start cylinder		   */
    unsigned char osind;	/* Operating System indicator		   */
    unsigned char ehead;  	/* Partition end head			   */
    unsigned char esect;  	/* Partition end sector			   */
    unsigned char ecyl;  	/* Partition end cylinder		   */
    unsigned long presect;	/* Sectors preceding partition		   */
    unsigned long nsect;	/* Length of partition in sector	   */
};

void showintro (void)
{
    /* Show copyright notice */
    clrscr();
    textcolor(MAGENTA);
    cprintf("IDE-INFO v1.00\r\n\r\n");
    textcolor(LIGHTGRAY);
    cprintf("        This utility can be used to diagnose your harddisk(s) and is put\r\n");
    cprintf("        into the public domain. However, the author is not responsible\r\n");
    cprintf("        for any damage that may occur by using this program.\r\n\r\n");
    cprintf("        It can be very useful, when you do not have specifications of\r\n");
    cprintf("        your harddisk and want to find the optimum BIOS parameters. It\r\n");
    cprintf("        provides also some information about the caching functions and\r\n");
    cprintf("        the way data is stored on the disk.\r\n\r\n");
    cprintf("        This is low-level software and it might give weird results if you\r\n");
    cprintf("        use programs as QEMM-stealth, OS/2, Windows (32-bit HD-access).\r\n\r\n");
    textcolor(MAGENTA);
    cprintf("Contacting the author\r\n\r\n");
    textcolor(LIGHTGRAY);
    cprintf("        If you want some more information you can contact me on the\r\n");
    cprintf("        address:\r\n\r\n");
    textcolor(CYAN);
    cprintf("                Ramon de Klein              2:283/7.21@fidonet.org\r\n");
    cprintf("                Th. de Keyserstraat 298     27:4331/107.21@signet.fnt\r\n");
    cprintf("                7545 AJ  Enschede           81:431/16.21@os2net.fnt\r\n");
    cprintf("                The Netherlands\r\n\r\n");
    textcolor(MAGENTA);
    cprintf("                        -=Press any key to start IDE-INFO=-");
    getch();
}


void ideerror (int drive)
{
    /* Clear the screen */
    clrscr();

    /* Display error message */
    textcolor(RED);
    cprintf("IDE-ERROR on drive %d --- Incompatible IDE hardware/software detected\r\n\r\n", drive);
    textcolor(LIGHTGRAY);
    cprintf("The HD-controller did not respond in time and IDE-INFO cannot\r\n");
    cprintf("detect the information it requires. Try to boot a \"clean\" DOS disk,\r\n");
    cprintf("without loading any device-drivers. If this doesn't work you have\r\n");
    cprintf("probably incompatible hardware.\r\n\r\n\r\n");

    /* Wait until the user hits a key */
    textcolor(MAGENTA);
    cprintf("                           -=Press any key to quit=-");
    getch();

    /* Restore video */
    textcolor(LIGHTGRAY);
    _setcursortype(_NORMALCURSOR);
    clrscr();
}

const char *totext (const unsigned int *data, int start, int end)
{
    static char dest[512];
    int ctr1, ctr2;

    for (ctr1 = start, ctr2 = 0; ctr1 <= end; ctr1++, ctr2 += 2) {
	dest[ctr2+0] = (char) (data[ctr1] >> 8);
	dest[ctr2+1] = (char) (data[ctr1] & 255);
    }
    dest[ctr2] = '\0';
    return dest;
}


int gethdtype(int drive)
{
    int hdtype, mask;

    /* Find the harddisk-type */
    outportb(0x70, 0x92);
    hdtype = inportb(0x71);
    mask   = 0xF0 >> (drive<<2);
    if ((hdtype & mask) == mask) {
	outportb(0x70, 0x99+drive);
	return inportb(0x71);
    } else
	return (hdtype & mask) >> (drive<<2);
}


int getideinfo (int drive, unsigned int *dinfo)
{
    volatile unsigned int retry;
    int wordno;

    /* Wait until controller is not busy */
    retry = UINT_MAX;
    while ((inp(0x1F7) != 0x50) && --retry);
    if (!retry) return 1;

    /* Get drive information */
    outp(0x1F6, drive ? 0xB0:0xA0);
    outp(0x1F7, 0xEC);

    /* Wait until data is available */
    retry = UINT_MAX;
    while ((inp(0x1F7) != 0x58) && --retry);
    if (!retry) return 1;

    /* Read the drive information */
    for (wordno=0; wordno<256; wordno++)
	dinfo[wordno] = inpw(0x1F0);

    /* Return succesful */
    return 0;
}


void showideinfo (int drive, const unsigned int *dinfo)
{
    clrscr();
    textcolor(MAGENTA);
    cprintf("IDE/ATA Interface ID Information of drive %u:\r\n\r\n", drive);
    textcolor(LIGHTGRAY);
    cprintf("              Model: ");
    textcolor(CYAN);
    cprintf("%s\r\n",   totext(dinfo, 27, 46));
    textcolor(LIGHTGRAY);
    cprintf("      Serial Number: ");
    textcolor(CYAN);
    cprintf("%s\r\n\r\n", totext(dinfo, 10, 19));

    textcolor(MAGENTA);
    cprintf("Drive Parameter Information\r\n\r\n");
    textcolor(LIGHTGRAY);
    cprintf("    Firmware Rev: ");
    textcolor(CYAN);
    cprintf("%-12s", totext(dinfo,23,26));
    textcolor(LIGHTGRAY);
    cprintf("  Bytes/Index Gap: ");
    textcolor(CYAN);
    cprintf("%u\r\n", dinfo[7]>>8);
    textcolor(LIGHTGRAY);
    cprintf("       Cylinders: ");
    textcolor(CYAN);
    cprintf("%-12u", dinfo[1]);
    textcolor(LIGHTGRAY);
    cprintf("        Bytes/PLO: ");
    textcolor(CYAN);
    cprintf("%u\r\n", dinfo[8]&0xFF);
    textcolor(LIGHTGRAY);
    cprintf("           Heads: ");
    textcolor(CYAN);
    cprintf("%-12u", dinfo[3] & 0xFF);
    textcolor(LIGHTGRAY);
    cprintf("    Buffer Scheme: ");
    textcolor(CYAN);
    switch (dinfo[20]) {
	case 1:	    cprintf("Single Port Single Sector\r\n"); break;
	case 2:	    cprintf("Dual Port Multisector\r\n"); break;
	case 3:	    cprintf("Dual Port Multisector Cache\r\n"); break;
	default:    cprintf("Not specified\r\n"); break;
    }
    textcolor(LIGHTGRAY);
    cprintf("     Bytes/Track: ");
    textcolor(CYAN);
    cprintf("%-12u", dinfo[4]);
    textcolor(LIGHTGRAY);
    cprintf("    Sector Buffer: ");
    textcolor(CYAN);
    cprintf("%u", dinfo[21]);
    textcolor(LIGHTGRAY);
    cprintf(" (");
    textcolor(CYAN);
    cprintf("%u", dinfo[21]/2);
    textcolor(LIGHTGRAY);
    cprintf("Kb cache)\r\n");
    cprintf("    Bytes/Sector: ");
    textcolor(CYAN);
    cprintf("%-12u", dinfo[5]);
    textcolor(LIGHTGRAY);
    cprintf("        Bytes/ECC: ");
    textcolor(CYAN);
    cprintf("%u\r\n", dinfo[22]);
    textcolor(LIGHTGRAY);
    cprintf("   Sectors/Track: ");
    textcolor(CYAN);
    cprintf("%-12u", dinfo[6]);
    textcolor(LIGHTGRAY);
    cprintf(" Sectors/Transfer: ");
    textcolor(CYAN);
    cprintf("%u", dinfo[47]);
    textcolor(LIGHTGRAY);
    cprintf(" (READ command)\r\n");
    cprintf("       Bytes/ISG: ");
    textcolor(CYAN);
    cprintf("%-12u", dinfo[7]);
    textcolor(LIGHTGRAY);
    cprintf("         Capacity: ");
    textcolor(CYAN);
    cprintf("%lu", ((unsigned long) dinfo[1])*dinfo[3]*dinfo[6]/2048);
    textcolor(LIGHTGRAY);
    cprintf("Mb (");
    textcolor(CYAN);
    cprintf("%lu", ((unsigned long) dinfo[1])*dinfo[3]*dinfo[6]*512);
    textcolor(LIGHTGRAY);
    cprintf(" bytes)\r\n");
    cprintf("     Double Word: ");
    textcolor(CYAN);
    cprintf("%-12s", dinfo[48]?"Capable":"Not Capable");
    textcolor(LIGHTGRAY);
    cprintf("  Defect Realloc.: ");
    textcolor(CYAN);
    cprintf("%s\r\n\r\n", dinfo[49]?"Yes":"No");

    textcolor(MAGENTA);
    cprintf("General Configuration\r\n\r\n");
    textcolor(LIGHTGRAY);
    cprintf("         Sectoring: ");
    textcolor(CYAN);
    switch (dinfo[0] & 0x0006) {
	case 0x0000:    cprintf("%-15s", "Unavailable"); break;
	case 0x0002:    cprintf("%-15s", "Hard"); break;
	case 0x0004:    cprintf("%-15s", "Soft"); break;
    }
    textcolor(LIGHTGRAY);
    cprintf("      Transfer Rate: ");
    textcolor(CYAN);
    switch (dinfo[0] & 0x0700) {
	case 0x0000:    cprintf("Unavailable\r\n"); break;
	case 0x0100:    cprintf("\xF3 5Mhz\r\n"); break;
	case 0x0200:    cprintf("> 5Mhz and \xF3 10Mhz\r\n"); break;
	case 0x0300:    cprintf("Unavailable\r\n"); break;
	case 0x0400:    cprintf("> 10Mhz\r\n"); break;
	case 0x0500:    cprintf("Unavailable\r\n"); break;
	case 0x0600:    cprintf("> 5Mhz to > 10Mhz (ZBR)\r\n"); break;
	case 0x0700:    cprintf("Unavailable\r\n"); break;
    }
    textcolor(LIGHTGRAY);
    cprintf("          Encoding: ");
    textcolor(CYAN);
    switch (dinfo[0] & 0x0008) {
	case 0x0000:    cprintf("%-15s", "MFM"); break;
	case 0x0008:    cprintf("%-15s", "RLL"); break;
    }
    textcolor(LIGHTGRAY);
    cprintf("   Rotational Speed: ");
    textcolor(CYAN);
    switch (dinfo[0] & 0x0800) {
	case 0x0000:    cprintf("< 0.5%% Tolerance\r\n"); break;
	case 0x0800:    cprintf("> 0.5%% Tolerance\r\n"); break;
    }
    textcolor(LIGHTGRAY);
    cprintf("       Head Switch: ");
    textcolor(CYAN);
    switch (dinfo[0] & 0x0010) {
	case 0x0000:    cprintf("%-15s", "< 15\xE6s"); break;
	case 0x0010:    cprintf("%-15s", "> 15\xE6s"); break;
    }
    textcolor(LIGHTGRAY);
    cprintf(" Data Strobe Offset: ");
    textcolor(CYAN);
    switch (dinfo[0] & 0x1000) {
	case 0x0000:    cprintf("Not available\r\n"); break;
	case 0x1000:    cprintf("Available\r\n"); break;
    }
    textcolor(LIGHTGRAY);
    cprintf("   Spindle Control: ");
    textcolor(CYAN);
    switch (dinfo[0] & 0x0020) {
	case 0x0000:    cprintf("%-15s", "No"); break;
	case 0x0020:    cprintf("%-15s", "Yes"); break;
    }
    textcolor(LIGHTGRAY);
    cprintf("       Track Offset: ");
    textcolor(CYAN);
    switch (dinfo[0] & 0x2000) {
	case 0x0000:    cprintf("Not available\r\n"); break;
	case 0x2000:    cprintf("Available\r\n"); break;
    }
    textcolor(LIGHTGRAY);
    cprintf("             Media: ");
    textcolor(CYAN);
    switch (dinfo[0] & 0x00C0) {
	case 0x0000:    cprintf("%-15s", "Unavailable"); break;
	case 0x0040:    cprintf("%-15s", "Fixed"); break;
	case 0x0080:    cprintf("%-15s", "Removable"); break;
	case 0x00C0:    cprintf("%-15s", "Unavailable"); break;
    }
    textcolor(LIGHTGRAY);
    cprintf("  Frmt Spd Tol. Gap: ");
    textcolor(CYAN);
    switch (dinfo[0] & 0x4000) {
	case 0x0000:    cprintf("Not required\r\n"); break;
	case 0x4000:    cprintf("Required\r\n"); break;
    }
    textcolor(LIGHTGRAY);
    cprintf("                                             Recording: ");
    textcolor(CYAN);
    switch (dinfo[0] & 0x8000) {
	case 0x0000:    cprintf("Magnetic\r\n"); break;
	case 0x8000:    cprintf("Non-Magnetic\r\n"); break;
    }

    /* Wait until the user presses a key */
    textcolor(MAGENTA);
    cprintf("        -=Press a key to continue=-");
    getch();
}


const struct hdparam far *gethdparam (int drive)
{
    switch (drive) {
	case 0: return (struct hdparam far *) getvect(0x41);
	case 1: return (struct hdparam far *) getvect(0x46);
    }
    return NULL;
}


void showhdparam (int drive, int hdtype, const struct hdparam far *hdp)
{
    clrscr();
    textcolor(MAGENTA);
    cprintf("BIOS HD parameter table of drive %u:\r\n\r\n", drive);
    textcolor(LIGHTGRAY);
    cprintf("                             Harddisk type: ");
    textcolor(CYAN);
    cprintf("%u\r\n", hdtype);
    textcolor(LIGHTGRAY);
    cprintf("                 Location of HD parameters: ");
    textcolor(CYAN);
    cprintf("%Fp\r\n\r\n", hdp);
    textcolor(LIGHTGRAY);
    cprintf("                       Number of Cylinders: ");
    textcolor(CYAN);
    cprintf("%-5u\r\n", hdp->cylinders);
    textcolor(LIGHTGRAY);
    cprintf("                           Number of heads: ");
    textcolor(CYAN);
    cprintf("%-5u\r\n", hdp->heads);
    textcolor(LIGHTGRAY);
    cprintf("   Starting Reduced Write Current Cylinder: ");
    textcolor(CYAN);
    cprintf("%-5u", hdp->reducedwr);
    textcolor(LIGHTGRAY);
    cprintf("   (XT-only)\r\n");
    cprintf("   Starting Write Precomp. Cylinder Number: ");
    textcolor(CYAN);
    cprintf("%-5u\r\n", hdp->precomp);
    textcolor(LIGHTGRAY);
    cprintf("                  Maximum ECC burst length: ");
    textcolor(CYAN);
    cprintf("%-5u", hdp->maxECCburst);
    textcolor(LIGHTGRAY);
    cprintf("   (XT-only)\r\n");
    cprintf("                              Drive Option: ");
    textcolor(CYAN);
    cprintf("%-5u", hdp->driveoption);
    textcolor(LIGHTGRAY);
    cprintf("   (XT-only)\r\n");
    cprintf("                         More than 8 Heads: ");
    textcolor(CYAN);
    cprintf("%-5s", (hdp->more8heads)?"Yes":"No");
    textcolor(LIGHTGRAY);
    cprintf("   (AT-only)\r\n");
    cprintf("   Manufactorer's Defect Map on 'maxcyl+1': ");
    textcolor(CYAN);
    cprintf("%-5s", (hdp->hidefectmap)?"Yes":"No");
    textcolor(LIGHTGRAY);
    cprintf("   (AT-only)\r\n");
    cprintf("                       Disable ECC retries: ");
    textcolor(CYAN);
    cprintf("%-5s\r\n", (hdp->disECCretry)?"On":"Off");
    textcolor(LIGHTGRAY);
    cprintf("                    Disable access retries: ");
    textcolor(CYAN);
    cprintf("%-5s\r\n", (hdp->disaccretry)?"On":"Off");
    textcolor(LIGHTGRAY);
    cprintf("                          Standard Timeout: ");
    textcolor(CYAN);
    cprintf("%-5u", hdp->stdtimeout);
    textcolor(LIGHTGRAY);
    cprintf("   (XT-only)\r\n");
    cprintf("                        Formatting Timeout: ");
    textcolor(CYAN);
    cprintf("%-5u", hdp->fmttimeout);
    textcolor(LIGHTGRAY);
    cprintf("   (XT-only)\r\n");
    cprintf("                Timeout for Checking Drive: ");
    textcolor(CYAN);
    cprintf("%-5u", hdp->chktimeout);
    textcolor(LIGHTGRAY);
    cprintf("   (XT-only)\r\n");
    cprintf("           Cylinder number of Landing Zone: ");
    textcolor(CYAN);
    cprintf("%-5u", hdp->landingzone);
    textcolor(LIGHTGRAY);
    cprintf("   (AT-only)\r\n");
    cprintf("               Number of Sectors per Track: ");
    textcolor(CYAN);
    cprintf("%-5u", hdp->sectors);
    textcolor(LIGHTGRAY);
    cprintf("   (AT-only)\r\n\r\n");
    cprintf("   Total Capacity (calculated by BIOS par): ");
    textcolor(CYAN);
    cprintf("%lu", ((unsigned long) hdp->cylinders)*hdp->heads*hdp->sectors/2048);
    textcolor(LIGHTGRAY);
    cprintf("Mb (");
    textcolor(CYAN);
    cprintf("%lu", ((unsigned long) hdp->cylinders)*hdp->heads*hdp->sectors*512);
    textcolor(LIGHTGRAY);
    cprintf(" bytes)\r\n\r\n");

    /* Wait until the user presses a key */
    textcolor(MAGENTA);
    cprintf("                           -=Press a key to continue=-");
    getch();
}


void getbootsec (int drive, unsigned char *bootsec)
{
    biosdisk(_DISK_READ, 0x80+drive, 0, 0, 1, 1, bootsec);
}


void showbootsec (int drive, unsigned char *bootsec)
{
    const struct partrec *ptable;
    int partno;

    /* Find start of partition table */
    ptable = (const struct partrec *) (bootsec+0x1BE);

    /* Display header */
    clrscr();
    textcolor(MAGENTA);
    cprintf("Partition table of drive %u:\r\n\r\n", drive);

    /* Display all partitions */
    textcolor(LIGHTGRAY);
    cprintf("  \xC9\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xD1\xCD\xCD\xCD\xCD\xCD\xCD\xD1\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xD1\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xD1\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xD1\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xBB\r\n");
    cprintf("  \xBA  Operating  \xB3      \xB3    Start     \xB3     End      \xB3 Relative \xB3 Number of \xBA\r\n");
    cprintf("  \xBA    System   \xB3 Boot \xB3 Hd  Cyl Sect \xB3 Hd  Cyl Sect \xB3  sector  \xB3  sectors  \xBA\r\n");
    cprintf("  \xC7\xC4\xC4\xC4\xC4\xC4\xC4\xC4\xC4\xC4\xC4\xC4\xC4\xC4\xC5\xC4\xC4\xC4\xC4\xC4\xC4\xC5\xC4\xC4\xC4\xC4\xC4\xC4\xC4\xC4\xC4\xC4\xC4\xC4\xC4\xC4\xC5\xC4\xC4\xC4\xC4\xC4\xC4\xC4\xC4\xC4\xC4\xC4\xC4\xC4\xC4\xC5\xC4\xC4\xC4\xC4\xC4\xC4\xC4\xC4\xC4\xC4\xC5\xC4\xC4\xC4\xC4\xC4\xC4\xC4\xC4\xC4\xC4\xC4\xB6\r\n");
    for (partno=0; partno<4; partno++) {
	cprintf("  \xBA ");
	textcolor(CYAN);
	switch (ptable[partno].osind) {
	    case 0x00: cprintf("   empty   "); break;
	    case 0x01: cprintf("   DOS-12  "); break;
	    case 0x02: cprintf(" XENIX root"); break;
	    case 0x03: cprintf(" XENIX /usr"); break;
	    case 0x04: cprintf("   DOS-16  "); break;
	    case 0x05: cprintf("DOS-extend."); break;
	    case 0x06: cprintf("  BIG-DOS  "); break;
	    case 0x07: cprintf(" OS/2 HPFS "); break;
	    case 0x08: cprintf("  AIX boot "); break;
	    case 0x09: cprintf("  AIX data "); break;
	    case 0x0a: cprintf("Coher/Bmgr."); break;
	    case 0x10: cprintf("    OPUS   "); break;
	    case 0x24: cprintf(" NEC MS-DOS"); break;
	    case 0x40: cprintf("VENIX 80286"); break;
	    case 0x50: cprintf("Diskman R/O"); break;
	    case 0x51: cprintf("Diskman R/W"); break;
	    case 0x52: cprintf("    CP/M   "); break;
	    case 0x56: cprintf(" GoldenBow "); break;
	    case 0x61: cprintf(" SpeedStor "); break;
	    case 0x63: cprintf("UNIX / HURD"); break;
	    case 0x64: cprintf("   Novell  "); break;
	    case 0x65: cprintf("Novell 3.11"); break;
	    case 0x75: cprintf("   PC/IX   "); break;
	    case 0x80: cprintf("Minix -1.4a"); break;
	    case 0x81: cprintf("Minix/Linux"); break;
	    case 0x82: cprintf(" Linux swap"); break;
	    case 0x93: cprintf(" Amoeba FS "); break;
	    case 0x94: cprintf(" Amoeba BBT"); break;
	    case 0xB7: cprintf("  BSDI FS  "); break;
	    case 0xB8: cprintf(" BSDI swap "); break;
	    case 0xC6: cprintf("DR-DOS prot"); break;
	    case 0xDB: cprintf(" Conc. DOS "); break;
            case 0xE1: cprintf("SStor FAT12"); break;
	    case 0xE4: cprintf("SStor FAT16"); break;
	    case 0xF2: cprintf("DOS 3.3 sec"); break;
	    case 0xFE: cprintf("  LANstep  "); break;
	    case 0xFF: cprintf(" Xenix BBT "); break;
	    default:   cprintf("  unknown  "); break;
	}
	textcolor(LIGHTGRAY);
	cprintf(" \xB3 ");
	textcolor(CYAN);
	cprintf("%3s ", ptable[partno].boot?"Yes":"No");
	textcolor(LIGHTGRAY);
	cprintf(" \xB3 ");
	textcolor(CYAN);
	cprintf("%2u %4u  %2u ", ptable[partno].shead, ptable[partno].scyl | ((unsigned) ptable[partno].ssect & 0xC0) << 2, ptable[partno].ssect & 0x3F);
	textcolor(LIGHTGRAY);
	cprintf(" \xB3 ");
	textcolor(CYAN);
	cprintf("%2u %4u  %2u ", ptable[partno].ehead, ptable[partno].ecyl | ((unsigned) ptable[partno].esect & 0xC0) << 2, ptable[partno].esect & 0x3F);
	textcolor(LIGHTGRAY);
	cprintf(" \xB3 ");
	textcolor(CYAN);
	cprintf("%8lu", ptable[partno].presect);
	textcolor(LIGHTGRAY);
	cprintf(" \xB3 ");
	textcolor(CYAN);
	cprintf("%9lu", ptable[partno].nsect);
	textcolor(LIGHTGRAY);
	cprintf(" \xBA\r\n");
    }
    cprintf("  \xC8\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCF\xCD\xCD\xCD\xCD\xCD\xCD\xCF\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCF\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCF\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCF\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xBC\r\n\r\n");

    /* Show remark */
    cprintf("   Because several operating systems use the same identifier for the\r\n");
    cprintf("   identification of the partition, you may see the wrong operating system.\r\n");
    cprintf("   This could be confusing, but it isn't dangerous. So don't worry...\r\n\r\n\r\n");

    /* Wait until the user presses a key */
    textcolor(MAGENTA);
    cprintf("                           -=Press a key to continue=-");
    getch();
}


unsigned int findhdtype (unsigned int cyl, unsigned int heads, unsigned int sectors, unsigned long *space)
{
    const struct hdparam far *hdtab;
    unsigned int hdtype;
    unsigned int ctr;
    long hdcap;

    /* Initialize variables */
    hdtab = MK_FP(0xF000,0xE401);
    hdcap = (long) cyl * (long) heads * (long) sectors;
    *space = hdcap+1;

    /* Check every type in the HD table */
    for (ctr=0; ctr<47; ctr++) {
	long diff;

	diff = hdcap -
	       (long) hdtab[ctr].cylinders *
	       (long) hdtab[ctr].heads *
	       (long) hdtab[ctr].sectors;

	if ((diff >= 0L) && (diff < *space)) {
	    *space = diff;
	    hdtype = ctr+1;
	}
    }

    /* Return the HD type */
    *space >>= 1; /* convert sectors to Kb */
    return hdtype;
}


void showhints (const struct hdparam far *hdp, const unsigned int *dinfo)
{
    unsigned long biospar, hdpar;
    int suggest;

    clrscr();
    textcolor(MAGENTA);
    cprintf("Diagnostics\r\n\r\n");

    suggest = 0;

    hdpar = (unsigned long) dinfo[1] *
	    (unsigned long) (dinfo[3] & 0xFF) *
	    (unsigned long) dinfo[6];

    biospar = (unsigned long) hdp->cylinders *
	      (unsigned long) hdp->heads *
	      (unsigned long) hdp->sectors;

    textcolor(LIGHTGRAY);
    if (biospar < hdpar) {
	cprintf("   You don't use the full capacity of your harddisk. Choose another\r\n");
	cprintf("   HD-type in your BIOS (or adjust user-definable type if available).\r\n");
	cprintf("   There are ");
        textcolor(CYAN);
	cprintf("%lu", (hdpar-biospar)/2);
	textcolor(LIGHTGRAY);
	cprintf("Kb unused on your HD.\r\n\r\n");
	suggest = 1;
    }

    if (biospar == hdpar) {
	cprintf("   You're using the full capacity of your harddisk and you do not need\r\n");
	cprintf("   to change the parameters.\r\n\r\n");
    }

    if (biospar > hdpar) {
	cprintf("   The BIOS parameters specify a capacity which is bigger than the\r\n");
	cprintf("   physical capacity. You'll need to choose another type in your BIOS\r\n");
	cprintf("   setup (or adjust the user definable type if available). If you\r\n");
	cprintf("   continue to use these parameters you can lose data!\r\n\r\n");
	suggest = 1;
    }

    if (hdp->cylinders >= 1024) {
	cprintf("   You're using more than 1024 cylinders, which is supported by your\r\n");
	cprintf("   BIOS. However some software may refuse to work with this. Some\r\n");
	cprintf("   low-level disk programs may have some problems with it. If you\r\n");
	cprintf("   don't experience any problems you can leave it the way it is, but\r\n");
	cprintf("   you may consider to modify the harddisk paramers.\r\n\r\n");
	suggest = 1;
    }

    if (suggest) {
	unsigned int  cyl, heads, sectors;
	unsigned int  hdtype;
	unsigned long space;
	unsigned int  prime;
	const struct hdparam far *hdtab;

	/* Initialize proposed parameters with physical characteristics */
	cyl     = dinfo[1];
	heads   = dinfo[3] & 0xFF;
	sectors = dinfo[6];

	/* Make sure that cylinders don't exceed 1024. Some BIOS support this */
	/* but not all operating systems work well with it, so it should be   */
	/* avoided as much as possible					      */
	prime = 2;
	while ((cyl >= 1024) && (prime < cyl)) {

	    /* Find smallest value which is a divider of 'cyl' */
	    while ((cyl % prime) && (prime < cyl))
		prime++;

	    /* Adjust 'cyl' and 'heads' with calculated prime number */
	    if (prime < cyl) {
		cyl /= prime;
		heads *= prime;
	    }
	}

	/* Make sure that the heads don't exceed 256, because the BIOS can't */
	/* handle it.							     */
	prime = 2;
	while (heads >= 256) {

	    /* Find smallest value which is a divider of 'head' */
	    while ((heads % prime) && (prime < heads))
		prime++;

	    /* Adjust 'heads' and 'sectors' with calculated prime number */
	    if (prime < heads) {
		heads /= prime;
		sectors *= prime;
	    }
	}

	/* Make sure that the sectors don't exceed 64, because the BIOS */
	/* can't handle it.						*/
	prime = 2;
	while (sectors >= 64) {

	    /* Find smallest value which is a divider of 'head' */
	    while ((sectors % prime) && (prime < sectors))
		prime++;

	    /* Adjust 'sectors' with calculated prime number and if 'cyl' */
	    /* don't exceed 1024 then adjust the 'cyl', otherwise adjust  */
	    /* 'heads'.  						  */
	    if (prime < sectors) {
		sectors /= prime;
		if ((heads * prime) < 256)
		    heads *= prime;
		else
		    cyl *= prime;
	    }
	}

	/* Find the corresponding HD-type from the BIOS table */
	hdtype = findhdtype(cyl,heads,sectors,&space);

	/* Get pointer to the HD parameter table */
	hdtab = MK_FP(0xF000,0xE401);
	hdtab += hdtype-1;


	/* Show the proposed values */
	cprintf("   The following parameters are suggested for your harddisk if you\r\n");
	cprintf("   have a user definable type:\r\n\r\n");
	cprintf("            Cylinders : ");
	textcolor(CYAN);
	cprintf("%-5u", cyl);
	textcolor(LIGHTGRAY);
	cprintf("   Heads : ");
	textcolor(CYAN);
	cprintf("%-5u", heads);
	textcolor(LIGHTGRAY);
	cprintf("   Sectors per Track : ");
	textcolor(CYAN);
	cprintf("%u\r\n\r\n", sectors);
	textcolor(LIGHTGRAY);
	cprintf("   If you don't have a user definable type, choose HD-type ");
	textcolor(CYAN);
	cprintf("%u", hdtype);
	textcolor(LIGHTGRAY);
	cprintf(". This\r\n");
	cprintf("   entry is the closest match to the proposed parameters. It leaves\r\n");
	cprintf("   you with ");
	textcolor(CYAN);
	cprintf("%lu", space);
	textcolor(LIGHTGRAY);
	cprintf("Kb of unused diskspace.\r\n\r\n");
	textcolor(CYAN);
	cprintf("       %2u", hdtype);
	textcolor(LIGHTGRAY);
	cprintf("   Cylinders : ");
	textcolor(CYAN);
	cprintf("%-5u", hdtab->cylinders);
	textcolor(LIGHTGRAY);
	cprintf("   Heads : ");
	textcolor(CYAN);
	cprintf("%-5u", hdtab->heads);
	textcolor(LIGHTGRAY);
	cprintf("   Sectors per Track : ");
	textcolor(CYAN);
	cprintf("%u\r\n\r\n", hdtab->sectors);
    }

    /* Wait until the user presses a key */
    textcolor(MAGENTA);
    cprintf("                           -=Press a key to continue=-");
    getch();
}


void showhdtable (void)
{
    const struct hdparam far *hdtab;
    int ctr;

    /* Get pointer to the HD parameter table */
    hdtab = MK_FP(0xF000,0xE401);

    /* Print the table */
    clrscr();
    textcolor(MAGENTA);
    cprintf("   Cyls  Hd  PrCmp  ECC  Ctl  Park  Sec ");
    textcolor(LIGHTGRAY);
    cprintf("\xB3");
    textcolor(MAGENTA);
    cprintf("   Cyls  Hd  PrCmp  ECC  Ctl  Park  Sec");
    for (ctr=0; ctr<23; ctr++) {
	textcolor(LIGHTGRAY);
	cprintf("%2u", ctr+1);
	textcolor(CYAN);
	cprintf(" %4u  %2u  %5u  %3u  %3u  %4u  %3u ",
	    hdtab[ctr].cylinders, hdtab[ctr].heads, hdtab[ctr].precomp, hdtab[ctr].maxECCburst,
	    hdtab[ctr].driveoption, hdtab[ctr].landingzone, hdtab[ctr].sectors);
	textcolor(LIGHTGRAY);
	cprintf("\xB3%2u", ctr+25);
	textcolor(CYAN);
	cprintf(" %4u  %2u  %5u  %3u  %3u  %4u  %3u",
	    hdtab[ctr+24].cylinders, hdtab[ctr+24].heads, hdtab[ctr+24].precomp, hdtab[ctr+24].maxECCburst,
	    hdtab[ctr+24].driveoption, hdtab[ctr+24].landingzone, hdtab[ctr+24].sectors);
    }
    textcolor(LIGHTGRAY);
    cprintf("24");
    textcolor(CYAN);
    cprintf(" %4u  %2u  %5u  %3u  %3u  %4u  %3u ",
	hdtab[23].cylinders, hdtab[23].heads, hdtab[23].precomp, hdtab[23].maxECCburst,
	hdtab[23].driveoption, hdtab[23].landingzone, hdtab[23].sectors);
    textcolor(LIGHTGRAY);
    cprintf("\xB3");
    textcolor(MAGENTA);
    cprintf("      -=Press a key to continue=-");

    getch();
}



int main (void)
{
    unsigned int  drive, ndrives;

    /* Turn cursor off */
    _setcursortype(_NOCURSOR);

    /* Show introduction */
    showintro();

    /* Number of HD reported by the BIOS */
    ndrives = peekb(0x40,0x75);

    for (drive=0; drive<ndrives; drive++) {
	int error;
	unsigned int dinfo[256];
	unsigned char bootsec[512];
	int hdtype;
	const struct hdparam far *hdp;

	/* Get and show IDE drive information */
	if (!(error = getideinfo(drive, dinfo)))
	    showideinfo(drive, dinfo);
	else
	    ideerror(drive);

	/* Get and show BIOS information of the drive */
	hdtype = gethdtype(drive);
	hdp = gethdparam(drive);
	showhdparam(drive, hdtype, hdp);

	/* Show hints */
	if (!error)
	    showhints(hdp, dinfo);

	/* Show partition table */
	getbootsec(drive, bootsec);
	showbootsec(drive, bootsec);
    }

    /* Show BIOS HD parameter table */
    showhdtable();

End:
    /* Restore video */
    textcolor(LIGHTGRAY);
    _setcursortype(_NORMALCURSOR);
    clrscr();

    return 0;
}

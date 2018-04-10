# ifndef _SPRINTF_H
# define _SPRINTF_H

/*///////////////////////////////////////////////////////////////////////////////////////////////////////////////////*/

#include "Types.H"
#include "VAList.H"

/*///////////////////////////////////////////////////////////////////////////////////////////////////////////////////*/

/* Flags used in processing format string */
#define PR_LJ 0x01 /* left justify */
#define PR_CA 0x02 /* use A-F instead of a-f for hex */
#define PR_SG 0x04 /* signed numeric conversion (%d vs. %u) */
#define PR_32 0x08 /* long (32-bit) numeric conversion */
#define PR_16 0x10 /* short (16-bit) numeric conversion */
#define PR_WS 0x20 /* PR_SG set and num was < 0 */
#define PR_LZ 0x40 /* pad left with '0' instead of ' ' */
#define PR_FP 0x80 /* pointers are far */

/* Largest number handled is 2^32-1, lowest radix handled is 8. 2^32-1 in base 8 has 11 digits
(add 5 for trailing NUL and for slop) */

#define	    PR_BUFLEN 16

/*///////////////////////////////////////////////////////////////////////////////////////////////////////////////////*/

int sprintf(char *Buffer, const char *fmt, ...);
void printf(const char *fmt, ...);
void printf(ushort X, ushort Y, const char *fmt, ...);

/*///////////////////////////////////////////////////////////////////////////////////////////////////////////////////*/

# endif


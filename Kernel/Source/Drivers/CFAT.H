#ifndef _CFAT_H
#define _CFAT_H
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#include "../Lib/Types.H"

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////

// Useful cluster numbers
#define MSDOSFSROOT     0               // cluster 0 means the root dir
#define CLUST_FREE      0               // cluster 0 also means a free cluster
#define MSDOSFSFREE     CLUST_FREE
#define CLUST_FIRST     2               // first legal cluster number
#define CLUST_RSRVD     0xfffffff6      // reserved cluster range
#define CLUST_BAD       0xfffffff7      // a cluster with a defect
#define CLUST_EOFS      0xfffffff8      // start of eof cluster range
#define CLUST_EOFE      0xffffffff      // end of eof cluster range

#define FAT12_MASK      0x00000fff      // mask for 12 bit cluster numbers
#define FAT16_MASK      0x0000ffff      // mask for 16 bit cluster numbers
#define FAT32_MASK      0x0fffffff      // mask for FAT32 cluster numbers

// Partition Type used in the partition record
#define PART_TYPE_UNKNOWN       0x00
#define PART_TYPE_FAT12         0x01
#define PART_TYPE_XENIX         0x02
#define PART_TYPE_DOSFAT16      0x04
#define PART_TYPE_EXTDOS        0x05
#define PART_TYPE_FAT16         0x06
#define PART_TYPE_NTFS          0x07
#define PART_TYPE_FAT32         0x0B
#define PART_TYPE_FAT32LBA      0x0C
#define PART_TYPE_FAT16LBA      0x0E
#define PART_TYPE_EXTDOSLBA     0x0F
#define PART_TYPE_ONTRACK       0x33
#define PART_TYPE_NOVELL        0x40
#define PART_TYPE_PCIX          0x4B
#define PART_TYPE_PHOENIXSAVE   0xA0
#define PART_TYPE_CPM           0xDB
#define PART_TYPE_DBFS          0xE0
#define PART_TYPE_BBT           0xFF

// BPB for DOS 7.10 (FAT32)
// This one has a few extensions to bpb50.
struct bpb710 {
        word    bpbBytesPerSec; // bytes per sector
        byte    bpbSecPerClust; // sectors per cluster
        word    bpbResSectors;  // number of reserved sectors
        byte    bpbFATs;        // number of FATs
        word    bpbRootDirEnts; // number of root directory entries
        word    bpbSectors;     // total number of sectors
        byte    bpbMedia;       // media descriptor
        word    bpbFATsecs;     // number of sectors per FAT
        word    bpbSecPerTrack; // sectors per track
        word    bpbHeads;       // number of heads
        dword   bpbHiddenSecs;  // # of hidden sectors
// 3.3 compat ends here
        dword   bpbHugeSectors; // # of sectors if bpbSectors == 0
// 5.0 compat ends here
        dword     bpbBigFATsecs;// like bpbFATsecs for FAT32
        word      bpbExtFlags;  // extended flags:
#define FATNUM    0xf           // mask for numbering active FAT
#define FATMIRROR 0x80          // FAT is mirrored (like it always was)
        word      bpbFSVers;    // filesystem version
#define FSVERS    0             // currently only 0 is understood
        dword     bpbRootClust; // start cluster for root directory
        word      bpbFSInfo;    // filesystem info structure sector
        word      bpbBackup;    // backup boot sector
        // There is a 12 byte filler here, but we ignore it
};


struct bootsector710 {
    byte    bsJump[3];                  // jump inst E9xxxx or EBxx90
    char    bsOEMName[8];               // OEM name and version
    char    bsBPB[53];                  // BIOS parameter block
    char    bsExt[26];                  // Bootsector Extension
    char    bsBootCode[418];            // pad so structure is 512b
    byte    bsBootSectSig2;             // 2 & 3 are only defined for FAT32?
    byte    bsBootSectSig3;
    byte    bsBootSectSig0;             // boot sector signature byte 0x55
    byte    bsBootSectSig1;             // boot sector signature byte 0xAA
#define BOOTSIG0        0x55
#define BOOTSIG1        0xaa
#define BOOTSIG2        0
#define BOOTSIG3        0
};

struct partrecord // length 16 bytes
{           
    byte    prIsActive;                 // 0x80 indicates active partition
    byte    prStartHead;                // starting head for partition
    word    prStartCylSect;             // starting cylinder and sector
    byte    prPartType;                 // partition type (see above)
    byte    prEndHead;                  // ending head for this partition
    word    prEndCylSect;               // ending cylinder and sector
    dword   prStartLBA;                 // first LBA sector for this partition
    dword   prSize;                     // size of this partition (bytes or sectors ?)
};
        
struct partsector
{
    char    psPartCode[512-64-2];       // pad so struct is 512b
    byte    psPart[64];                 // four partition records (64 bytes)
    byte    psBootSectSig0;             // two signature bytes (2 bytes)
    byte    psBootSectSig1;
#define BOOTSIG0        0x55
#define BOOTSIG1        0xaa
};

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////

class CFAT {
public:
	bool Initialize(void);

private:
	bool Initialize(word Channel, uchar Drive);
};

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////

extern CFAT *FAT;

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#endif

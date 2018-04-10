#ifndef __FAT_H
#define __FAT_H
/*///////////////////////////////////////////////////////////////////////////////////////////////////////////////////*/

#include "Types.H"
//#include "Globals.H"

/*///////////////////////////////////////////////////////////////////////////////////////////////////////////////////*/

extern "C" void lmul(ulong *m1, ulong *m2);
extern "C" void ldiv(ulong *num, ulong *den, ulong *rem);
extern "C" void llsh(ulong *l, uchar t);
extern "C" void lrsh(ulong *l, uchar t);

/******* extended (LBA) addressing *******/

typedef struct
{
	uchar packet_size;	/* 16 */
	uchar reserved;		/* set to 0 */
	uchar n;		/* number of blocks to transfer */
	ushort dest_seg;
	ushort dest_offset;
	ulong lba[2];	/* 64-bit LBA sec address */
} bios_ext_disk_xfer_t;

extern uchar bios_read_disk_ext(uchar drive, bios_ext_disk_xfer_t *p);

typedef struct
{
	ushort size;	/* 26 */
	ushort flags;
	ulong num_cyls;
	ulong num_heads;
	ulong num_spt;
	ulong num_total_sectors[2];
	ushort sector_size;
} bios_ext_drive_parm_buf_t;

extern uchar bios_get_drive_parms_ext(uchar drive, bios_ext_drive_parm_buf_t *b);

extern ushort data_seg;


/******* file system types *******/
#define FS_FAT12 0
#define FS_FAT16 1
#define FS_FAT32 2
#define FS_UNKNOWN 255

/* special values for FAT entries */
#define FAT12_RESERVED		0xFF0
#define FAT12_BAD_CLUS		0xFF7
#define FAT12_LAST_CLUS		0xFF8
#define FAT16_RESERVED		0xFFF0
#define FAT16_BAD_CLUS		0xFFF7
#define FAT16_LAST_CLUS		0xFFF8
#define FAT32_RESERVED		0x0FFFFFF0
#define FAT32_BAD_CLUS		0x0FFFFFF7
#define FAT32_LAST_CLUS		0x0FFFFFF8

/* directory entry attributes */
#define FAT_ATTR_READ_ONLY	0x01
#define FAT_ATTR_HIDDEN		0x02
#define FAT_ATTR_SYSTEM		0x04
#define FAT_ATTR_VOLUME_ID	0x08
#define FAT_ATTR_DIRECTORY	0x10
#define FAT_ATTR_ARCHIVE	0x20
#define FAT_ATTR_LONG_NAME (FAT_ATTR_READ_ONLY | FAT_ATTR_HIDDEN | FAT_ATTR_SYSTEM | FAT_ATTR_VOLUME_ID)
#define FAT_ATTR_LONG_NAME_MASK (FAT_ATTR_READ_ONLY | FAT_ATTR_HIDDEN | FAT_ATTR_SYSTEM | FAT_ATTR_VOLUME_ID | FAT_ATTR_DIRECTORY | FAT_ATTR_ARCHIVE)

/* special values of first character of directory entry name */
#define FAT_UNUSED		0x00
#define FAT_DELETED1	0xE5
#define FAT_DELETED2	0x05	/* Japanese character support, except in LFN string */

/* FAT short directory entry */
typedef struct
{
	uchar name[11];
	uchar attribute;		/* attribute */
	uchar nt_reserved;		/* set to 0 */
	uchar ct_ms;			/* creation time millisecond */
	ushort ct_hour_min;		/* creation time hour and minute */
	ushort ct_date;			/* creation time date */
	ushort last_accessed;	/* last accessed date */
	ushort start_clus_hi;	/* high word of start cluster (0 if not FAT32) */
	ushort time;			/* last modification time */
	ushort date;			/* last modification date */
	ushort start_clus_lo;	/* low word of start cluster */
	ulong size;				/* file size in bytes */
} fat_short_de_t;

/* FAT long (LFN) directory entry */
typedef struct
{
	uchar ordinal;
	ushort unicode0[5];
	uchar attribute;
	uchar reserved0;
	uchar checksum;
	ushort unicode1[6];
	ushort reserved1;
	ushort unicode2[2];
} fat_long_de_t;

/* FAT max data clusters */
#define FAT12_MAX_CLUSTERS	4084	/* anything above this is not a FAT12 */
#define FAT16_MAX_CLUSTERS	65525	/* anything above this is a FAT32 */

/* FAT boot signature */
#define FAT_BOOT_SIG	0x29

/* FAT compatible boot sector data (first 36 bytes) */
typedef struct
{
	uchar jmp[3];
	uchar oem_name[8];
	ushort sec_size;
	uchar spc;
	ushort res_sec;
	uchar num_fats;
	ushort tot_rd_ents;
	ushort tot_sec16;
	uchar media;
	ushort fat_size16;
	ushort int13_spt;
	ushort int13_heads;
	ulong hidd_sec;
	ulong tot_sec32;
} fat_compat_boot_t;

/* FAT12/FAT16 boot sector structure */
typedef struct
{
	fat_compat_boot_t cb;
	uchar int13_drive;
	uchar reserved;
	uchar boot_sig;
	ulong volume_id;
	uchar volume_label[11];
	uchar fs_type[8];
	uchar boot_code[448];
	ushort bios_sig;
} fat16_boot_t;

typedef fat16_boot_t fat12_boot_t;

/* FAT32 boot sector structure */
typedef struct
{
	fat_compat_boot_t cb;
	ulong fat_size32;
	ushort fat_flags;
	uchar fs_ver[2];
	ulong root_clus;
	ushort fsinfo_sec;
	ushort bak_boot_sec;
	uchar reserved0[12];
	uchar int13_drive;
	uchar reserved1;
	uchar boot_sig;
	ulong volume_id;
	uchar volume_label[11];
	uchar fs_type[8];
	uchar boot_code[420];
	ushort bios_sig;
} fat32_boot_t;

/* FAT file structure */
typedef struct
{
	ulong bytes_left;		/* number of bytes left to read in the file */
	ulong curr_clus;		/* current cluster */
	ushort clus_offs;		/* number of sectors we are into the cluster */
	ushort sec_offs;		/* number of bytes we are into the sector */
	uchar sec_buf[4096];	/* buffer one data sector at a time */
	ulong fat_sec;			/* number of first FAT sector in the buffer */
	uchar fat_buf[8192];	/* buffer 2 contiguous FAT sectors at a time */
} fat_file_t;

/******* function prototypes *******/
void fat_init(void *bs);
uchar find_fat_root_file(char *name, fat_short_de_t *de);
uchar read_fat_byte(fat_file_t *file, uchar *b);
uchar read_fat_sector(fat_file_t *file, void *buf);
ulong read_fat_entry(fat_file_t *file, ulong ent);
void fat_name_to_shortname(char *name, char *shortname);
uchar ReadDisk(ulong start, void *buf, uchar n);

/*///////////////////////////////////////////////////////////////////////////////////////////////////////////////////*/
#endif

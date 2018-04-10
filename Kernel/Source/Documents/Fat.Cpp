# ifndef _FAT_H
# define _FAT_H

/*///////////////////////////////////////////////////////////////////////////////////////////////////////////////////*/

#include "Types.H"
#include "System.H"
#include "StdLib.H"

//#include "Entry.h"
#include "Fat.h"
//#include "String.H"
//#include "Globals.H"


byte use_ext = 0;

/*///////////////////////////////////////////////////////////////////////////////////////////////////////////////////*/

/******* pointers to file system's boot sector *******/
fat16_boot_t *fat16_bs;
fat32_boot_t *fat32_bs;

extern uchar boot_drive;
extern uchar fs_type;

/*///////////////////////////////////////////////////////////////////////////////////////////////////////////////////*/

void fat_init(void *bs)
{
	ulong tot_clust, m1, m2, den, rem;

	/* save pointers to boot sector */
	fat16_bs = (fat16_boot_t *)bs;
	fat32_bs = (fat32_boot_t *)bs;

	/* determine what kind of FAT file system we are booting from */
	fs_type = FS_UNKNOWN;
	if(fat16_bs->boot_sig == FAT_BOOT_SIG)
	{
		/* calculate number of clusters */
		tot_clust = fat16_bs->cb.tot_sec16 == 0 ? fat16_bs->cb.tot_sec32 : fat16_bs->cb.tot_sec16;
		tot_clust = tot_clust - fat16_bs->cb.res_sec - (fat16_bs->cb.num_fats * fat16_bs->cb.fat_size16) - (fat16_bs->cb.tot_rd_ents / 32);
		den = fat16_bs->cb.spc;
		ldiv(&tot_clust, &den, &rem);
		/* verify cluster count */
		if(tot_clust <= FAT12_MAX_CLUSTERS)
		{
			fs_type = FS_FAT12;
		}
		else if(tot_clust <= FAT16_MAX_CLUSTERS)
		{
			fs_type = FS_FAT16;
		}
	}
	if(fat32_bs->boot_sig == FAT_BOOT_SIG)
	{
		tot_clust = fat32_bs->cb.tot_sec32 - fat32_bs->cb.res_sec;
		m1 = fat32_bs->cb.num_fats;
		m2 = fat32_bs->fat_size32;
		lmul(&m1, &m2);
		tot_clust -= m1;
		den = fat32_bs->cb.spc;
		ldiv(&tot_clust, &den, &rem);
		if(fat32_bs->fs_ver[0] == 0 && fat32_bs->fs_ver[1] == 0)
		{
			fs_type = FS_FAT32;
		}
	}

	if(!(fat16_bs->cb.sec_size == 512 || fat16_bs->cb.sec_size == 1024 || fat16_bs->cb.sec_size == 2048 || fat16_bs->cb.sec_size == 4096))
	{
		fs_type = FS_UNKNOWN;
	}
}

/*///////////////////////////////////////////////////////////////////////////////////////////////////////////////////*/

uchar find_fat_root_file(char *name, fat_short_de_t *de)
{
	/***************************************************************************
	Description:
	Finds the specified file by matching its name. Makes a copy of the directory
	entry for the caller.
	***************************************************************************/
	ushort rd_sec;
	uchar b, *s;
	fat_short_de_t de2, root[4096/32];
	fat_file_t file;
	size_t i, j;

	if(fs_type == FS_FAT32)
	{
		file.bytes_left = 0xFFFFFFFF;
		file.curr_clus = fat32_bs->root_clus;
		file.clus_offs = 0;
		file.sec_offs = 4096;
		file.fat_sec = 0;
		/* read root directory, sector by sector till it's empty */
		for(;;)
		{
			s = (uchar *)&de2;
			for(i=0; i<32; i++)
			{
				if(read_fat_byte(&file, &b) == 255)
				{
					return 255;
				}
				memcpy(s + i, &b, 1);
			}
			if(memcmp(de2.name, name, 11) == 0)
			{
				memcpy(de, &de2, 32);
				return 0;
			}
		}
	}
	else
	{
		/* calculate number of root directory sectors */
		rd_sec = (fat16_bs->cb.tot_rd_ents * 32) / fat16_bs->cb.sec_size;
		/* read them in one at a time */
		for(i=0; i<rd_sec; i++)
		{
			if( ReadDisk(fat16_bs->cb.hidd_sec + fat16_bs->cb.res_sec + (fat16_bs->cb.num_fats * fat16_bs->cb.fat_size16) + i, (void *)root, 1))
            {
                printf("Error reading root directory.");
                Hang();
            }

			/* search each directory entry in the sector */
			for(j=0; j<fat16_bs->cb.sec_size / 32; j++)
			{
				if(memcmp(root[j].name, name, 11) == 0)
				{
					memcpy(de, &root[j], 32);
					return 0;
				}
			}
		}
		return 255;
	}
}

/*///////////////////////////////////////////////////////////////////////////////////////////////////////////////////*/

uchar read_fat_byte(fat_file_t *file, uchar *b)
{
	/***************************************************************************
	Description:
	Reads the next byte in the file into the buffer.
	***************************************************************************/

	if(file->bytes_left == 0)	/* check for EOF */
	{
		return 255;
	}
	if(file->sec_offs >= fat16_bs->cb.sec_size)	/* need to read next sector */
	{
		if(read_fat_sector(file, file->sec_buf) == 255)
		{
			return 255;
		}
		file->sec_offs = 0;
	}
	*b = file->sec_buf[file->sec_offs];
	file->bytes_left--;
	file->sec_offs++;
	return 0;
}

/*///////////////////////////////////////////////////////////////////////////////////////////////////////////////////*/

uchar read_fat_sector(fat_file_t *file, void *buf)
{
	/***************************************************************************
	Description:
	Reads the next sector in the file into the buffer.
	***************************************************************************/
	ulong sec, m1, m2;
	char str[20];

	/* switch to next cluster if necessary */
	if(file->clus_offs == fat16_bs->cb.spc)
	{
		file->curr_clus = read_fat_entry(file, file->curr_clus);
		file->clus_offs = 0;
	}

	/*  check for EOF - this is only used to check EOF for the FAT32 rootdir,
	but is still somewhat useful at catching errors in the file system */
	if((fs_type == FS_FAT12 && file->curr_clus >= FAT12_LAST_CLUS) || (fs_type == FS_FAT16 && file->curr_clus >= FAT16_LAST_CLUS) || (fs_type == FS_FAT32 && file->curr_clus >= FAT32_LAST_CLUS))
	{
		return 255;
	}

	/* calculate sector to read */
	m1 = file->curr_clus;
	m2 = fat16_bs->cb.spc;	/* cb is same for all FATs, so this is OK */
	lmul(&m1, &m2);
	if(fs_type == FS_FAT32)
	{
		sec = fat32_bs->cb.hidd_sec + fat32_bs->cb.res_sec + m1;
		m1 = fat32_bs->cb.num_fats;
		m2 = fat32_bs->fat_size32;
		lmul(&m1, &m2);
		sec = sec + m1 - (2 * fat32_bs->cb.spc);
	}
	else
	{
		sec = (fat16_bs->cb.hidd_sec + fat16_bs->cb.res_sec + (fat16_bs->cb.num_fats * fat16_bs->cb.fat_size16) + ((fat16_bs->cb.tot_rd_ents * 32) / fat16_bs->cb.sec_size) + m1 + file->clus_offs) - (2 * fat16_bs->cb.spc);
	}

	/* read the sector */
	if( ReadDisk(sec, buf, 1) )
    {
        printf("Error reading from disk");
        Hang();
    }

	/* increment cluster offset */
	file->clus_offs++;
	return 0;
}

/*///////////////////////////////////////////////////////////////////////////////////////////////////////////////////*/

ulong read_fat_entry(fat_file_t *file, ulong ent)
{
	/***************************************************************************
	Description:
	Reads the specified FAT entry and returns it. Utilizes backup copies of the
	FAT, if necessary and when available.
	***************************************************************************/
	ulong sec, m1, m2, num, den, rem;
	ushort i, offs;

	if(fs_type == FS_FAT32)
	{
		for(i=0; i<(fat32_bs->fat_flags & 0x80 ? 1 : fat32_bs->cb.num_fats); i++)
		{
			if(fat32_bs->fat_flags & 0x80)
			{
				i = fat32_bs->fat_flags & 0x0F;
			}
			m1 = (fat32_bs->fat_flags & 0x80 ? fat32_bs->fat_flags & 0x0F : i);
			m2 = fat32_bs->fat_size32;
			lmul(&m1, &m2);
			/* calculate FAT sector and read it in */
			num = ent;
			llsh(&num, 2);
			den = fat32_bs->cb.sec_size;
			ldiv(&num, &den, &rem);
			sec = fat32_bs->cb.hidd_sec + fat32_bs->cb.res_sec + m1 + num;
			if(file->fat_sec != sec)
			{
				if(ReadDisk(sec, file->fat_buf, 1) != 0)
				{
					continue;
				}
			}
			/* calc offset into FAT sector and read the entry */
			offs = (((ushort)ent << 2) & (fat32_bs->cb.sec_size - 1));
			return file->fat_buf[offs] | (file->fat_buf[offs+1] << 8) | (file->fat_buf[offs+2] << 16) | (file->fat_buf[offs+3] << 24);
		}
	}
	else
	{
		for(i=0; i<fat16_bs->cb.num_fats; i++)
		{
			/* calculate FAT sector and read it in */
			sec = fat16_bs->cb.hidd_sec + fat16_bs->cb.res_sec + (i * fat16_bs->cb.fat_size16);
			m1 = ent;
			m2 = (fs_type == FS_FAT12 ? 3 : 1);
			lmul(&m1, &m2);
			num = m1;
			if(fs_type == FS_FAT12)
			{
				lrsh(&num, 1);
			}
			else
			{
				llsh(&num, 1);
			}
			den = fat16_bs->cb.sec_size;
			ldiv(&num, &den, &rem);
			sec += num;
			if(file->fat_sec != sec)
			{
				if(ReadDisk(sec, file->fat_buf, (fs_type == FS_FAT12 ? 2 : 1))	!= 0)
				{
					continue;
				}
			}
			offs = ((fs_type == FS_FAT12 ? ((ushort)m1 >> 1) : ((ushort)m1 << 1)) & (fat16_bs->cb.sec_size - 1));
			if(fs_type == FS_FAT12)
			{
				if(m1 & 0x1)	/* odd */
				{
					return (file->fat_buf[offs] >> 4) | (file->fat_buf[offs+1] << 4);
				}
				else	/* even */
				{
					return ((file->fat_buf[offs+1] & 0x0F) << 8) | 	file->fat_buf[offs];
				}
			}
			else	/* FAT16 */
			{
				return file->fat_buf[offs] | (file->fat_buf[offs+1] << 8);
			}
		}
	}

    printf("Couldn't read from FAT");
    Hang();
}

/*///////////////////////////////////////////////////////////////////////////////////////////////////////////////////*/

void fat_name_to_shortname(char *name, char *shortname)
{
	/* note: no error checking */

	size_t i, j;

	j = 0;
	for(i=0; i<strlen(name); i++)
	{
		if(name[i] == '.')	/* if dot */
		{
			while(j < 8)	/* pad with spaces */
			{
				shortname[j] = ' ';
				j++;
			}
		}
		else
		{
			if(name[i] >= 'a' && name[i] <= 'z')
			{
				shortname[j] = name[i] + 'A' - 'a';
			}
			else
			{
				shortname[j] = name[i];
			}
			j++;
		}
	}
	while(j < 11)	/* pad with spaces */
	{
		shortname[j] = ' ';
		j++;
	}
	shortname[11] = '\0';
}

/*///////////////////////////////////////////////////////////////////////////////////////////////////////////////////*/

uchar ReadDisk(ulong _sec, void *buf, uchar n)
{
	/***************************************************************************
	Description:
	Uses the correct CHS or LBA parameters to read the specified number of
	sectors into the buffer. This function performs all necessary translation
	of the start logical sector address.
	***************************************************************************/
	ulong num, den, rem1, rem2;
	ushort cyl;
	uchar head, sec, error;
	bios_ext_disk_xfer_t p;
	size_t i;
	char str[20];

	if(use_ext)	/* use extended int 13h */
	{
		p.packet_size = 16;
		p.reserved = 0;
		p.n = n;
		p.dest_seg = data_seg;
		p.dest_offset = (ushort)buf;
		p.lba[0] = _sec;
		p.lba[1] = 0;
		return bios_read_disk_ext(boot_drive, &p);
	}
	else /* use old int 13h */
	{
		/* 3 tries for floppy, one otherwise */
		for(i=0; i<(boot_drive >= 0x80 ? 1 : 3); i++)
		{
			num = _sec;
			den = old_heads * old_spt;

			ldiv(&num, &den, &rem1);
			cyl = (ushort)num;
			den = old_spt;
			ldiv(&rem1, &den, &rem2);
			head = (uchar)rem1;
			sec = (uchar)rem2 + 1;

			error = bios_read_disk_old_chs(boot_drive, cyl, head, sec, n, data_seg, (ushort)buf);
			if(error == 0)
			{
				return 0;
			}
			if( bios_reset_disk_system(boot_drive) )
            {
                printf("failed to reset drive.");
                Hang();
            }

		}
		return error;
	}
}

/*///////////////////////////////////////////////////////////////////////////////////////////////////////////////////*/

#endif


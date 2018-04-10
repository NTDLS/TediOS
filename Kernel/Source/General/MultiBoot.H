#ifndef _MultiBoot_H
#define _MultiBoot_H
/* multiboot.h - the header for Multiboot */
/* Copyright (C) 1999, 2001  Free Software Foundation, Inc.

This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA. */

/* Macros. */

/* The magic number for the Multiboot header. */
#define MULTIBOOT_HEADER_MAGIC          0x1BADB002

/* The flags for the Multiboot header. */
#ifdef __ELF__
	# define MULTIBOOT_HEADER_FLAGS         0x00000003
#else
	# define MULTIBOOT_HEADER_FLAGS         0x00010003
#endif

/* The magic number passed by a Multiboot-compliant boot loader. */
#define MULTIBOOT_BOOTLOADER_MAGIC      0x2BADB002

/* The size of our stack (16KB). */
#define STACK_SIZE                      0x4000

/* C symbol format. HAVE_ASM_USCORE is defined by configure. */
#ifdef HAVE_ASM_USCORE
	# define EXT_C(sym)                     _ ## sym
#else
	# define EXT_C(sym)                     sym
#endif

#ifndef ASM
/* Do not include here in boot.S. */

/* Types. */

/* The Multiboot header. */
typedef struct multiboot_header
{
	unsigned long magic;
	unsigned long flags;
	unsigned long checksum;
	unsigned long header_addr;
	unsigned long load_addr;
	unsigned long load_end_addr;
	unsigned long bss_end_addr;
	unsigned long entry_addr;
} multiboot_header_t;

/* The symbol table for a.out. */
typedef struct aout_symbol_table
{
	unsigned long tabsize;
	unsigned long strsize;
	unsigned long addr;
	unsigned long reserved;
} aout_symbol_table_t;

/* The section header table for ELF. */
typedef struct elf_section_header_table
{
	unsigned long num;
	unsigned long size;
	unsigned long addr;
	unsigned long shndx;
} elf_section_header_table_t;

/* The Multiboot information. */
typedef struct multiboot_info
{
	unsigned long flags;
	unsigned long mem_lower;
	unsigned long mem_upper;
	unsigned long boot_device;
	unsigned long cmdline;
	unsigned long mods_count;
	unsigned long mods_addr;
union
{
	aout_symbol_table_t aout_sym;
	elf_section_header_table_t elf_sec;
} u;
	unsigned long mmap_length;
	unsigned long mmap_addr;
} multiboot_info_t;

/* The module structure. */
typedef struct module
{
	unsigned long mod_start;
	unsigned long mod_end;
	unsigned long string;
	unsigned long reserved;
} module_t;

/* The memory map. Be careful that the offset 0 is base_addr_low
but no size. */
typedef struct memory_map
{
	unsigned long size;
	unsigned long base_addr_low;
	unsigned long base_addr_high;
	unsigned long length_low;
	unsigned long length_high;
	unsigned long type;
} memory_map_t;

#endif /* ! ASM */
#endif //_MultiBoot_H
/*
			//// Print out the flags. 
			printf ("flags = %d\n", (unsigned) pMBI->flags);

			//// Are mem_* valid? 
			if(CHECK_FLAG (pMBI->flags, 0))
			 printf ("mem_lower = %uKB, mem_upper = %uKB\n",
					 (unsigned) pMBI->mem_lower, (unsigned) pMBI->mem_upper);

			//// Is boot_device valid? 
			if(CHECK_FLAG (pMBI->flags, 1))
			 printf ("boot_device = %d\n", (unsigned) pMBI->boot_device);

			//// Is the command line passed? 
			if(CHECK_FLAG (pMBI->flags, 2))
			 printf ("cmdline = %s\n", (char *) pMBI->cmdline);

			//// Are mods_* valid? 
			if(CHECK_FLAG(pMBI->flags, 3))
			{
				module_t *mod;
				int i;

				printf ("mods_count = %d, mods_addr = %d\n",
					(int) pMBI->mods_count, (int) pMBI->mods_addr);

				for (i = 0, mod = (module_t *) pMBI->mods_addr; i < pMBI->mods_count; i++, mod++)
				{
					printf (" mod_start = %d, mod_end = %d, string = %s\n",
						(unsigned) mod->mod_start,
						(unsigned) mod->mod_end,
						(char *) mod->string);
				}
			}

			//// Bits 4 and 5 are mutually exclusive! 
			if(CHECK_FLAG (pMBI->flags, 4) && CHECK_FLAG (pMBI->flags, 5))
			{
				printf ("Both bits 4 and 5 are set.\n");
				return 0;
			}

			//// Is the symbol table of a.out valid? 
			if(CHECK_FLAG (pMBI->flags, 4))
			{
				aout_symbol_table_t *aout_sym = &(pMBI->u.aout_sym);

				printf ("aout_symbol_table: tabsize = 0x%0x, "
					"strsize = %d, addr = %d\n",
					(unsigned) aout_sym->tabsize,
					(unsigned) aout_sym->strsize,
					(unsigned) aout_sym->addr);
			}

		}

		//// Is the section header table of ELF valid? 
		if(CHECK_FLAG (pMBI->flags, 5))
		{
			elf_section_header_table_t *elf_sec = &(pMBI->u.elf_sec);

			printf ("elf_sec: num = %u, size = %d,"
				" addr = %d, shndx = %d\n",
				(unsigned) elf_sec->num, (unsigned) elf_sec->size,
				(unsigned) elf_sec->addr, (unsigned) elf_sec->shndx);
		}

		//// Are mmap_* valid? 
		if (CHECK_FLAG (pMBI->flags, 6))
		{
			memory_map_t *mmap;

			printf ("mmap_addr = %d, mmap_length = %d\n",
				(unsigned) pMBI->mmap_addr, (unsigned) pMBI->mmap_length);

			for (mmap = (memory_map_t *) pMBI->mmap_addr;
                (unsigned long) mmap < pMBI->mmap_addr + pMBI->mmap_length;
                mmap = (memory_map_t *) ((unsigned long) mmap + mmap->size + sizeof (mmap->size)))
			{
				printf (" size = %d, base_addr = %d&%d,"
					" length = %d&%d, type = %d\n",
                    (unsigned) mmap->size,
                    (unsigned) mmap->base_addr_high,
                    (unsigned) mmap->base_addr_low,
                    (unsigned) mmap->length_high,
                    (unsigned) mmap->length_low,
                    (unsigned) mmap->type); //Type=1 means avaliable RAM.
			}
	   }

*/

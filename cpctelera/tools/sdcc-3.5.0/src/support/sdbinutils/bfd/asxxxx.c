/* BFD back-end for asxxxx .rel objects.
   Copyright 2011
   Borut Razem (Free Software Foundation, Inc.)
   Written by Borut Razem <borut.razem@gmail.com>.

   This file is part of BFD, the Binary File Descriptor library.

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 3 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 51 Franklin Street - Fifth Floor, Boston,
   MA 02110-1301, USA.  */


/* SUBSECTION
        The object module contains the following designators:

                [XDQ][HL][234]
                        X       Hexadecimal radix
                        D       Decimal radix
                        Q       Octal radix

                        H       Most significant byte first
                        L       Least significant byte first

                        2       16-Bit Addressing
                        3       24-Bit Addressing
                        4       32-Bit Addressing

                H       Header
                M       Module
                G       Merge Mode
                B       Bank
                A       Area
                S       Symbol
                T       Object code
                R       Relocation information
                P       Paging information

        3.5.1  Object Module Format
                [XDQ][HL][234]

        3.5.2  Header Line
                H aa areas gg global symbols

        3.5.3  Module Line
                M name

        3.5.4  Merge Mode Line
                G nn ii 00 01 02 03 04 05 06 07 08 09 0A 0B 0C 0D 0E 0F

        3.5.5  Bank Line
                B name base nn size nn map nn flags nn fsfx string

        3.5.6  Area Line
                A label size ss flags ff

        3.5.7  Symbol Line
                S name Defnnnn
                        or
                S name Refnnnn

        3.5.8  T Line
                T xx xx nn nn nn nn nn ...

        3.5.9  R Line
                R 0 0 nn nn n1 n2 xx xx ...

        3.5.10  P Line
                P 0 0 nn nn n1 n2 xx xx
 */

#include "sysdep.h"
#include "bfd.h"
#include "libbfd.h"
#include "libiberty.h"
#include "safe-ctype.h"

#define NELEM(x) (sizeof (x) / sizeof (x)[0])

/* ******************* from asld aslink.h ******************* */
/*
 *      ASLINK - Version 3 Definitions
 */

/*
 *      The "A3_" area constants define values used in
 *      generating the assembler area output data.
 *
 * Area flags
 *
 *         8      7     6     5     4     3     2     1     0
 *         A_     A_    A_    A_   A3_   A3_   A3_
 *      +-----++-----+-----+-----+-----+-----+-----+-----+-----+
 *      |LOAD || BIT |XDATA|CODE | PAG | ABS | OVR |     |     |
 *      +-----++-----+-----+-----+-----+-----+-----+-----+-----+
 */

#define A3_CON    000           /* concatenate */
#define A3_OVR    004           /* overlay */
#define A3_REL    000           /* relocatable */
#define A3_ABS    010           /* absolute */
#define A3_NOPAG  000           /* non-paged */
#define A3_PAG    020           /* paged */

/* sdld specific */
/* Additional flags for 8051 address spaces */
#define A_DATA    0000          /* data space (default)*/
#define A_CODE    0040          /* code space */
#define A_XDATA   0100          /* external data space */
#define A_BIT     0200          /* bit addressable space */

/* Additional flags for hc08 */
#define A_NOLOAD  0400          /* nonloadable */
#define A_LOAD    0000          /* loadable (default) */
/* end sdld specific */

/*
 *      ASLINK - Version 4 Definitions
 */

/*
 *      The "A4_" area constants define values used in
 *      generating the assembler area output data.
 *
 * Area flags
 *
 *         7     6     5     4     3     2     1     0
 *      +-----+-----+-----+-----+-----+-----+-----+-----+
 *      | BNK | SEG |     | PAG | ABS | OVR | WL1 | WL0 |
 *      +-----+-----+-----+-----+-----+-----+-----+-----+
 */

#define A4_BYTE         0x0000          /*  8 bit */
#define A4_WORD         0x0001          /* 16 bit */

#define A4_1BYTE        0x0000          /* 1 Byte Word Length */
#define A4_2BYTE        0x0001          /* 2 Byte Word Length */
#define A4_3BYTE        0x0002          /* 3 Byte Word Length */
#define A4_4BYTE        0x0003          /* 4 Byte Word Length */
#define A4_WLMSK        0x0003          /* Word Length Mask */

#define A4_CON          0x0400          /* Concatenating */
#define A4_OVR          0x0404          /* Overlaying */
#define A4_REL          0x0800          /* Relocatable */
#define A4_ABS          0x0808          /* absolute */
#define A4_NOPAG        0x1000          /* Non-Paged */
#define A4_PAG          0x1010          /* Paged */

#define A4_CSEG         0x4000          /* CSEG */
#define A4_DSEG         0x4040          /* DSEG */
#define A4_NOBNK        0x8000          /* Non-Banked */
#define A4_BNK          0x8080          /* Banked */

#define A4_OUT          0x0100          /* Output Code Flag */
/* ********************************************************** */

/* Macros for converting between hex and binary.  */

static const char digs[] = "0123456789ABCDEF";

#define NIBBLE(x)    hex_value(x)
#define HEX(buffer) ((NIBBLE ((buffer)[0])<<4) + NIBBLE ((buffer)[1]))
#define TOHEX(d, x, ch) \
        d[1] = digs[(x) & 0xf]; \
        d[0] = digs[((x)>>4)&0xf]; \
        ch += ((x) & 0xff);
#define ISHEX(x)    hex_p(x)

/* When scanning the asxxxx .rel file, a linked list of asxxxx_symbol
   structures is built to represent the symbol table (if there is
   one).  */

struct asxxxx_symbol
{
  struct asxxxx_symbol *next;
  const char *name;
  symvalue val;
  flagword flags;
  struct bfd_section *section;
};

/* The asxxxx .rel tdata information.  */

enum asxxxx_cpu_type_e
  {
    CPU_UNKNOWN = 0,
    CPU_MCS51,
    CPU_DS390,
    CPU_DS400,
    CPU_HC08,
    CPU_Z80,
    CPU_GBZ80,
    CPU_R2K,
  };

enum asxxxx_rel_version_e
  {
    REL_VER_3 = 0,
    REL_VER_4,
  };

enum asxxxx_radix_e
  {
    RADIX_OCT = 8,
    RADIX_DEC = 10,
    RADIX_HEX = 16,
  };

enum asxxxx_endian_e
  {
    ENDIAN_LITTLE = 0,
    ENDIAN_BIG,
  };

enum asxxxx_address_size_e
  {
    ADDR_SIZE_2 = 2,
    ADDR_SIZE_3 = 3,
    ADDR_SIZE_4 = 4,
  };

typedef struct asxxxx_data_struct
  {
    struct asxxxx_symbol *symbols;
    struct asxxxx_symbol *symtail;
    asymbol *csymbols;
    asection *sections;
    asection *secttail;
#define CURRENT_SECT(abfd) ((abfd)->tdata.asxxxx_data->secttail)

    unsigned int sect_id;
#define NEXT_SECT_ID(abfd) (++(abfd)->tdata.asxxxx_data->sect_id)

    enum asxxxx_cpu_type_e cpu_type;
#define SET_CPU_TYPE(abfd, type) ((abfd)->tdata.asxxxx_data->cpu_type = (type))
#define GET_CPU_TYPE(abfd) ((abfd)->tdata.asxxxx_data->cpu_type)

    enum asxxxx_rel_version_e rel_version;
#define SET_REL_VERSION(abfd, version) ((abfd)->tdata.asxxxx_data->rel_version = (version))
#define GET_REL_VERSION(abfd) ((abfd)->tdata.asxxxx_data->rel_version)

    enum asxxxx_radix_e radix;
#define SET_RADIX(abfd, r) ((abfd)->tdata.asxxxx_data->radix = (r))
#define GET_RADIX(abfd) ((abfd)->tdata.asxxxx_data->radix)

    enum asxxxx_endian_e endian;
#define SET_ENDIAN(abfd, e) ((abfd)->tdata.asxxxx_data->endian = (e))
#define GET_ENDIAN(abfd) ((abfd)->tdata.asxxxx_data->endian)

    enum asxxxx_address_size_e address_size;
#define SET_ADDRESS_SIZE(abfd, as) ((abfd)->tdata.asxxxx_data->address_size = (as))
#define GET_ADDRESS_SIZE(abfd) ((abfd)->tdata.asxxxx_data->address_size)
  }
tdata_type;

/* Initialize by filling in the hex conversion array.  */

static void
asxxxx_init (void)
{
  static bfd_boolean inited = FALSE;

  if (! inited)
    {
      inited = TRUE;
      hex_init ();
    }
}

/* Set up the asxxxx .rel tdata information.  */

static bfd_boolean
asxxxx_mkobject (bfd *abfd)
{
  tdata_type *tdata;

  asxxxx_init ();

  tdata = (tdata_type *) bfd_zalloc (abfd, sizeof (tdata_type));
  if (tdata == NULL)
    return FALSE;

  abfd->tdata.asxxxx_data = tdata;

  return TRUE;
}

/* Read a byte from an asxxxx .rel file.  Set *ERRORPTR if an error
   occurred.  Return EOF on error or end of file.  */

static int
asxxxx_get_byte (bfd *abfd, bfd_boolean *errorptr)
{
  bfd_byte c;

  if (bfd_bread (&c, (bfd_size_type) 1, abfd) != 1)
    {
      if (bfd_get_error () != bfd_error_file_truncated)
        *errorptr = TRUE;
      return EOF;
    }

  return (int) (c & 0xff);
}

/* Report a problem in an asxxxx .rel file.  FIXME: This probably should
   not call fprintf, but we really do need some mechanism for printing
   error messages.  */

static void
asxxxx_bad_byte (bfd *abfd,
               int c,
               unsigned int lineno,
               bfd_boolean error)
{
  if (c == EOF)
    {
      if (! error)
        bfd_set_error (bfd_error_file_truncated);
    }
  else
    {
      char buf[10];

      if (! ISPRINT (c))
        sprintf (buf, "\\%03o", (unsigned int) c);
      else
        {
          buf[0] = c;
          buf[1] = '\0';
        }
      (*_bfd_error_handler)
        (_("%B:%d: Unexpected character `%s' in asxxxx .rel file\n"),
         abfd, lineno, buf);
      bfd_set_error (bfd_error_bad_value);
    }
}

/* Add a new symbol found in an asxxxx .rel file.  */

static bfd_boolean
asxxxx_new_symbol (bfd *abfd, const char *name, symvalue val, flagword flags, struct bfd_section *section)
{
  struct asxxxx_symbol *n;

  n = (struct asxxxx_symbol *) bfd_alloc (abfd, sizeof (* n));
  if (n == NULL)
    return FALSE;

  n->name = name;
  n->val = val;
  n->flags = flags;
  n->section = section;

  if (abfd->tdata.asxxxx_data->symbols == NULL)
    abfd->tdata.asxxxx_data->symbols = n;
  else
    abfd->tdata.asxxxx_data->symtail->next = n;
  abfd->tdata.asxxxx_data->symtail = n;
  n->next = NULL;

  ++abfd->symcount;

  return TRUE;
}

/* Add a new section found in an asxxxx .rel file.  */

static flagword
asxxxx_to_asection_flags(bfd *abfd, unsigned int flags, unsigned int sect_size)
{
  flagword sect_flags = SEC_NO_FLAGS;

  if (sect_size)
     sect_flags |= SEC_HAS_CONTENTS;

  if (GET_REL_VERSION (abfd) == REL_VER_3)
    {
      /*
       *         8      7     6     5     4     3     2     1     0
       *         A_     A_    A_    A_   A3_   A3_   A3_
       *      +-----++-----+-----+-----+-----+-----+-----+-----+-----+
       *      |LOAD || BIT |XDATA|CODE | PAG | ABS | OVR |     |     |
       *      +-----++-----+-----+-----+-----+-----+-----+-----+-----+
       */

      if (!(flags & A3_ABS))
        sect_flags |= SEC_RELOC;

      switch (GET_CPU_TYPE (abfd))
        {
        case CPU_MCS51:
          sect_flags |= SEC_LOAD;
          if (flags & A_CODE)
            sect_flags |= (SEC_CODE | SEC_ROM | SEC_READONLY);
          if (flags & (A_XDATA | A_BIT) || !(flags & (A_CODE | A_XDATA | A_BIT)))
            sect_flags |= SEC_DATA;
          break;

        case CPU_HC08:
          sect_flags |= SEC_CODE;
          sect_flags |= (flags & A_LOAD) ? SEC_LOAD : SEC_NEVER_LOAD;
          break;

        default:
          sect_flags |= SEC_CODE;
          sect_flags |= SEC_LOAD;
          break;
        }
    }
  else
    {
      /*
       *         7     6     5     4     3     2     1     0
       *      +-----+-----+-----+-----+-----+-----+-----+-----+
       *      | BNK | SEG |     | PAG | ABS | OVR | WL1 | WL0 |
       *      +-----+-----+-----+-----+-----+-----+-----+-----+
       */

      if (!(flags & A4_ABS))
        sect_flags |= SEC_RELOC;

      sect_flags |= (flags & A4_DSEG) ? SEC_DATA : SEC_CODE;
      sect_flags |= SEC_LOAD;
    }

  return sect_flags;
}

static bfd_boolean
asxxxx_new_section (bfd *abfd, const char *sect_name, unsigned int sect_size, unsigned int sect_flags, unsigned int sect_addr)
{
  asection *sect;

  if ((sect = (asection *) bfd_zalloc (abfd, sizeof (*sect))) == NULL)
    return FALSE;

  sect->name = sect_name;
  sect->id = sect->index = NEXT_SECT_ID(abfd);
  sect->flags = asxxxx_to_asection_flags(abfd, sect_flags, sect_size);
  sect->size = sect->rawsize = sect_size;
  sect->vma = sect->lma = sect_addr;

  if (abfd->tdata.asxxxx_data->sections == NULL)
    abfd->tdata.asxxxx_data->sections = sect;
  else
    {
      sect->prev = abfd->tdata.asxxxx_data->secttail;
      abfd->tdata.asxxxx_data->secttail->next = sect;
    }
  abfd->tdata.asxxxx_data->secttail = sect;

  return TRUE;
}

/* Read the asxxxx .rel file and turn it into sections.  We create a new
   section for each contiguous set of bytes.  */

static bfd_boolean
asxxxx_skip_spaces (bfd *abfd, int *p_c, unsigned int lineno, bfd_boolean *errorptr)
{
  if (! ISSPACE (*p_c))
    {
      asxxxx_bad_byte (abfd, *p_c, lineno, *errorptr);
      return FALSE;
    }

  while (ISSPACE (*p_c = asxxxx_get_byte (abfd, errorptr)))
    ;

  return TRUE;
}

static bfd_boolean
asxxxx_skip_word (bfd *abfd, char *word, int *p_c, unsigned int lineno, bfd_boolean *errorptr)
{
  char *p;

  for (p = word; *p != '\0'; ++p)
    {
      if (*p_c == *p)
        {
          *p_c = asxxxx_get_byte (abfd, errorptr);
        }
      else
        {
          asxxxx_bad_byte (abfd, *p_c, lineno, *errorptr);
          return FALSE;
        }
    }

  return TRUE;
}

static bfd_boolean
asxxxx_get_word (bfd *abfd, char **p_word, int *p_c, unsigned int lineno, bfd_boolean *errorptr)
{
  bfd_size_type alc;
  char *p, *symname;
  char *symbuf;

  alc = 10;
  symbuf = (char *) bfd_malloc (alc + 1);
  if (symbuf == NULL)
    goto error_return;

  p = symbuf;

  *p++ = *p_c;
  while (! ISSPACE (*p_c = asxxxx_get_byte (abfd, errorptr)) && *p_c != EOF)
    {
      if ((bfd_size_type) (p - symbuf) >= alc)
        {
          char *n;

          alc *= 2;
          n = (char *) bfd_realloc (symbuf, alc + 1);
          if (n == NULL)
            goto error_return;
          p = n + (p - symbuf);
          symbuf = n;
        }

      *p++ = *p_c;
    }

  if (*p_c == EOF)
   {
     asxxxx_bad_byte (abfd, *p_c, lineno, *errorptr);
     goto error_return;
   }

   *p++ = '\0';

  symname = (char *) bfd_alloc (abfd, (bfd_size_type) (p - symbuf));
  if (symname == NULL)
    goto error_return;

  strcpy (symname, symbuf);
  free (symbuf);

  *p_word = symname;

  return TRUE;

error_return:
  if (symbuf != NULL)
    free (symbuf);

  *p_word = NULL;

  return FALSE;
}

static bfd_boolean
asxxxx_get_hex (bfd *abfd, unsigned int *p_val, int *p_c, unsigned int lineno, bfd_boolean *errorptr)
{
  *p_val = 0;
  while (ISHEX (*p_c))
    {
      *p_val = (*p_val << 4) + NIBBLE (*p_c);
      *p_c = asxxxx_get_byte (abfd, errorptr);
    }

  if (*p_c == EOF)
   {
     asxxxx_bad_byte (abfd, *p_c, lineno, *errorptr);
     return FALSE;
   }

  return TRUE;
}

static bfd_boolean
asxxxx_get_eol (bfd *abfd, int *p_c, unsigned int *p_lineno, bfd_boolean *errorptr)
{
  if (*p_c == '\n')
    {
      ++*p_lineno;
      return TRUE;
    }
  else if (*p_c != '\r')
    {
      asxxxx_bad_byte (abfd, *p_c, *p_lineno, *errorptr);
      return FALSE;
    }

  return TRUE;
}

static bfd_boolean
asxxxx_skip_line (bfd *abfd, int *p_c, unsigned int *p_lineno, bfd_boolean *errorptr)
{
  while ((*p_c = asxxxx_get_byte (abfd, errorptr)) != EOF && *p_c != '\n' && *p_c != '\r')
    ;

  return asxxxx_get_eol (abfd, p_c, p_lineno, errorptr);
}

static void
asxxxx_set_cpu_type(bfd *abfd, const char *cpu_type)
{
  struct cpu
    {
      const char *name;
      enum asxxxx_cpu_type_e type;
    }
  cpus[] =
    {
      { "-mmcs51", CPU_MCS51 },
      { "-mds390", CPU_DS390 },
      { "-mds400", CPU_DS400 },
      { "-mhc08",  CPU_HC08  },
      { "-mz80",   CPU_Z80   },
      { "-mgbz80", CPU_GBZ80 },
      { "-mr2K",   CPU_R2K   },
    };
  size_t i;

  for (i = 0; i < NELEM (cpus); ++i)
    {
      if (! strcmp (cpu_type, cpus[i].name))
        {
          SET_CPU_TYPE (abfd, cpus[i].type);
          return;
        }
    }

  SET_CPU_TYPE (abfd, CPU_UNKNOWN);
}

static bfd_boolean
asxxxx_scan (bfd *abfd, unsigned int *p_lineno)
{
  int c;
  bfd_boolean error = FALSE;

  bfd_set_error (bfd_error_file_truncated);

  while ((c = asxxxx_get_byte (abfd, &error)) != EOF)
    {
      switch (c)
        {
        default:
          /* unknown line */
          asxxxx_bad_byte (abfd, c, *p_lineno, error);
          goto error_return;
          break;

        case '\n':
          ++p_lineno;
          break;

        case '\r':
          break;

        case 'A':
          /* A CSEG size 12E2 flags 20 addr 0 */
          {
            char *sect_name;
            unsigned int sect_size, sect_flags, sect_addr;

            /* Starting a symbol definition.  */
            c = asxxxx_get_byte (abfd, &error);

            if (! asxxxx_skip_spaces (abfd, &c, *p_lineno, &error))
              goto error_return;

            if (! asxxxx_get_word (abfd, &sect_name, &c, *p_lineno, &error))
              goto error_return;

            if (! asxxxx_skip_spaces (abfd, &c, *p_lineno, &error))
              goto error_return;

            if (! asxxxx_skip_word (abfd, "size", &c, *p_lineno, &error))
              goto error_return;

            if (! asxxxx_skip_spaces (abfd, &c, *p_lineno, &error))
              goto error_return;

            if (! asxxxx_get_hex (abfd, &sect_size, &c, *p_lineno, &error))
              goto error_return;

            if (! asxxxx_skip_spaces (abfd, &c, *p_lineno, &error))
              goto error_return;

            if (! asxxxx_skip_word (abfd, "flags", &c, *p_lineno, &error))
              goto error_return;

            if (! asxxxx_skip_spaces (abfd, &c, *p_lineno, &error))
              goto error_return;

            if (! asxxxx_get_hex (abfd, &sect_flags, &c, *p_lineno, &error))
              goto error_return;

            if (! asxxxx_skip_spaces (abfd, &c, *p_lineno, &error))
              goto error_return;

            if (! asxxxx_skip_word (abfd, "addr", &c, *p_lineno, &error))
              goto error_return;

            if (! asxxxx_skip_spaces (abfd, &c, *p_lineno, &error))
              goto error_return;

            if (! asxxxx_get_hex (abfd, &sect_addr, &c, *p_lineno, &error))
              goto error_return;

            if (! asxxxx_get_eol (abfd, &c, p_lineno, &error))
              goto error_return;

            if (! asxxxx_new_section (abfd, sect_name, sect_size, sect_flags, sect_addr))
              goto error_return;
          }
          break;
        case 'O':
          /*
           * O -mds390 --model-flat24
           * O -mds400 --model-flat24
           * O -mhc08
           * O -mmcs51 --model-huge
           * O -mmcs51 --model-large
           * O -mmcs51 --model-medium
           * O -mmcs51 --model-small
           * O -mmcs51 --model-small --xstack
           */
          {
            char *cpu_type;

            /* check if the next character is space */
            c = asxxxx_get_byte (abfd, &error);

            if (! asxxxx_skip_spaces (abfd, &c, *p_lineno, &error))
              goto error_return;

            if (! asxxxx_get_word (abfd, &cpu_type, &c, *p_lineno, &error))
              goto error_return;

            /* eat the rest of line */
            if (! asxxxx_skip_line (abfd, &c, p_lineno, &error))
              goto error_return;

            asxxxx_set_cpu_type(abfd, cpu_type);
          }
          break;
        case 'G': /* V 4.XX+ */
        case 'B': /* V 4.XX+ */
          SET_REL_VERSION (abfd, REL_VER_4);
          /* fall through */
        case 'H':
        case 'M':
        case 'T':
        case 'R':
        case 'P':
          /* check if the next character is space */
          c = asxxxx_get_byte (abfd, &error);

          if (! asxxxx_skip_spaces (abfd, &c, *p_lineno, &error))
            goto error_return;

          /* eat the rest of line */
          if (! asxxxx_skip_line (abfd, &c, p_lineno, &error))
            goto error_return;
          break;

        case 'S':
          /* S __ret3 Def0001 */
          {
            char *symname = NULL;
            unsigned int symval;
            bfd_boolean is_def;

            /* Starting a symbol definition.  */
            c = asxxxx_get_byte (abfd, &error);

            if (! asxxxx_skip_spaces (abfd, &c, *p_lineno, &error))
              goto error_return;

            if (! asxxxx_get_word (abfd, &symname, &c, *p_lineno, &error))
              goto error_return;

            if (! asxxxx_skip_spaces (abfd, &c, *p_lineno, &error))
              goto error_return;

            if (c != 'D' && c != 'R')
              {
                asxxxx_bad_byte (abfd, c, *p_lineno, error);
                goto error_return;
              }

            is_def = (c == 'D');

            c = asxxxx_get_byte (abfd, &error);
            if (! asxxxx_skip_word (abfd, "ef", &c, *p_lineno, &error))
              goto error_return;

            if (! asxxxx_get_hex(abfd, &symval, &c, *p_lineno, &error))
              goto error_return;

            if (! asxxxx_get_eol (abfd, &c, p_lineno, &error))
              goto error_return;

            if (! asxxxx_new_symbol (abfd, symname, symval, BSF_GLOBAL, CURRENT_SECT(abfd) ? CURRENT_SECT(abfd) : (is_def ? bfd_abs_section_ptr : bfd_und_section_ptr)))
              goto error_return;
          }
          break;
        }
    }

  if (error)
    goto error_return;

  return TRUE;

error_return:
  return FALSE;
}

/* Check whether an existing file is an asxxxx .rel file.  */

static bfd_boolean
asxxxx_is_rel (bfd *abfd, unsigned int *p_lineno)
{
  int c;
  bfd_boolean error = FALSE;

__begin_check:
  error = FALSE;

  /* [XDQ][HL][234] */
  switch (c = asxxxx_get_byte (abfd, &error))
    {
    default:
      /* unknown line */
      asxxxx_bad_byte (abfd, c, *p_lineno, error);
      return FALSE;

    case ';':
      c = asxxxx_get_byte (abfd, &error);

      if (! asxxxx_skip_word (abfd, "!FILE ", &c, *p_lineno, &error))
        {
          return FALSE;
        }
      /* eat the rest of line */
      if (! asxxxx_skip_line (abfd, &c, p_lineno, &error))
        return FALSE;
      goto __begin_check;

    case 'X':
      SET_RADIX (abfd, RADIX_HEX);
      goto get_endian;
    case 'D':
      SET_RADIX (abfd, RADIX_DEC);
      goto get_endian;
    case 'Q':
      SET_RADIX (abfd, RADIX_OCT);
    get_endian:
      switch (c = asxxxx_get_byte (abfd, &error))
        {
        default:
          asxxxx_bad_byte (abfd, c, *p_lineno, error);
          return FALSE;

        case 'H':
          SET_ENDIAN (abfd, ENDIAN_BIG);
          goto get_addr_size;
        case 'L':
          SET_ENDIAN (abfd, ENDIAN_LITTLE);
        get_addr_size:
          switch (c = asxxxx_get_byte (abfd, &error))
            {
            default:
              asxxxx_bad_byte (abfd, c, *p_lineno, error);
              return FALSE;

            case '\n':
              ++*p_lineno;
              /* fall through */
            case '\r':
              SET_ADDRESS_SIZE (abfd, ADDR_SIZE_2);
              break;

            case '2':
              SET_ADDRESS_SIZE (abfd, ADDR_SIZE_2);
              goto get_eol;
            case '3':
              SET_ADDRESS_SIZE (abfd, ADDR_SIZE_3);
              goto get_eol;
            case '4':
              SET_ADDRESS_SIZE (abfd, ADDR_SIZE_4);
            get_eol:
              c = asxxxx_get_byte (abfd, &error);
              if (! asxxxx_get_eol (abfd, &c, p_lineno, &error))
                return FALSE;
              break;
            }
          break;
        }
      break;
    }

  return error ? FALSE : TRUE;
}

static const bfd_target *
asxxxx_object_p (bfd *abfd)
{
  void * tdata_save;
  unsigned int lineno = 1;

  asxxxx_init ();

  if (! asxxxx_mkobject (abfd) || ! asxxxx_is_rel (abfd, &lineno))
    {
      bfd_set_error (bfd_error_wrong_format);
      return NULL;
    }

  tdata_save = abfd->tdata.any;
  if (! asxxxx_scan (abfd, &lineno))
    {
      if (abfd->tdata.any != tdata_save && abfd->tdata.any != NULL)
        bfd_release (abfd, abfd->tdata.any);
      abfd->tdata.any = tdata_save;
      return NULL;
    }

  if (abfd->symcount > 0)
    abfd->flags |= HAS_SYMS;

  return abfd->xvec;
}

/* Get the contents of a section.  */

static bfd_boolean
asxxxx_get_section_contents (bfd *abfd ATTRIBUTE_UNUSED,
                           asection *section ATTRIBUTE_UNUSED,
                           void * location ATTRIBUTE_UNUSED,
                           file_ptr offset ATTRIBUTE_UNUSED,
                           bfd_size_type count ATTRIBUTE_UNUSED)
{
  return FALSE;
}

/* Set the architecture.  We accept an unknown architecture here.  */

static bfd_boolean
asxxxx_set_arch_mach (bfd *abfd, enum bfd_architecture arch, unsigned long mach)
{
  if (arch != bfd_arch_unknown)
    return bfd_default_set_arch_mach (abfd, arch, mach);

  abfd->arch_info = & bfd_default_arch_struct;
  return TRUE;
}

/* Set the contents of a section.  */

static bfd_boolean
asxxxx_set_section_contents (bfd *abfd ATTRIBUTE_UNUSED,
                           sec_ptr section ATTRIBUTE_UNUSED,
                           const void * location ATTRIBUTE_UNUSED,
                           file_ptr offset ATTRIBUTE_UNUSED,
                           bfd_size_type bytes_to_do ATTRIBUTE_UNUSED)
{
  return FALSE;
}

static int
asxxxx_sizeof_headers (bfd *abfd ATTRIBUTE_UNUSED,
                     struct bfd_link_info *info ATTRIBUTE_UNUSED)
{
  return 0;
}

/* Return the amount of memory needed to read the symbol table.  */

static long
asxxxx_get_symtab_upper_bound (bfd *abfd)
{
  return (bfd_get_symcount (abfd) + 1) * sizeof (asymbol *);
}

/* Return the symbol table.  */

static long
asxxxx_canonicalize_symtab (bfd *abfd, asymbol **alocation)
{
  bfd_size_type symcount = bfd_get_symcount (abfd);
  asymbol *csymbols;
  unsigned int i;

  csymbols = abfd->tdata.asxxxx_data->csymbols;
  if (csymbols == NULL && symcount != 0)
    {
      asymbol *c;
      struct asxxxx_symbol *s;

      csymbols = (asymbol *) bfd_alloc (abfd, symcount * sizeof (asymbol));
      if (csymbols == NULL)
        return -1;
      abfd->tdata.asxxxx_data->csymbols = csymbols;

      for (s = abfd->tdata.asxxxx_data->symbols, c = csymbols;
           s != NULL;
           s = s->next, ++c)
        {
          c->the_bfd = abfd;
          c->name = s->name;
          c->value = s->val;
          c->flags = s->flags;
          c->section = s->section;
          c->udata.p = NULL;
        }
    }

  for (i = 0; i < symcount; i++)
    *alocation++ = csymbols++;
  *alocation = NULL;

  return symcount;
}

static void
asxxxx_get_symbol_info (bfd *ignore_abfd ATTRIBUTE_UNUSED,
                      asymbol *symbol,
                      symbol_info *ret)
{
  bfd_symbol_info (symbol, ret);
}

static void
asxxxx_print_symbol (bfd *abfd,
                   void * afile,
                   asymbol *symbol,
                   bfd_print_symbol_type how)
{
  FILE *file = (FILE *) afile;

  switch (how)
    {
    case bfd_print_symbol_name:
      fprintf (file, "%s", symbol->name);
      break;
    default:
      bfd_print_symbol_vandf (abfd, (void *) file, symbol);
      fprintf (file, " %-5s %s",
               symbol->section->name,
               symbol->name);
    }
}

#define asxxxx_find_line                            _bfd_nosymbols_find_line
#define asxxxx_close_and_cleanup                    _bfd_generic_close_and_cleanup
#define asxxxx_bfd_free_cached_info                 _bfd_generic_bfd_free_cached_info
#define asxxxx_new_section_hook                     _bfd_generic_new_section_hook
#define asxxxx_bfd_is_target_special_symbol         ((bfd_boolean (*) (bfd *, asymbol *)) bfd_false)
#define asxxxx_bfd_is_local_label_name              bfd_generic_is_local_label_name
#define asxxxx_get_lineno                           _bfd_nosymbols_get_lineno
#define asxxxx_find_nearest_line                    _bfd_nosymbols_find_nearest_line
#define asxxxx_find_inliner_info                    _bfd_nosymbols_find_inliner_info
#define asxxxx_make_empty_symbol                    _bfd_generic_make_empty_symbol
#define asxxxx_bfd_make_debug_symbol                _bfd_nosymbols_bfd_make_debug_symbol
#define asxxxx_read_minisymbols                     _bfd_generic_read_minisymbols
#define asxxxx_minisymbol_to_symbol                 _bfd_generic_minisymbol_to_symbol
#define asxxxx_get_section_contents_in_window       _bfd_generic_get_section_contents_in_window
#define asxxxx_bfd_get_relocated_section_contents   bfd_generic_get_relocated_section_contents
#define asxxxx_bfd_relax_section                    bfd_generic_relax_section
#define asxxxx_bfd_gc_sections                      bfd_generic_gc_sections
#define asxxxx_bfd_lookup_section_flags             bfd_generic_lookup_section_flags
#define asxxxx_bfd_merge_sections                   bfd_generic_merge_sections
#define asxxxx_bfd_is_group_section                 bfd_generic_is_group_section
#define asxxxx_bfd_discard_group                    bfd_generic_discard_group
#define asxxxx_section_already_linked               _bfd_generic_section_already_linked
#define asxxxx_bfd_define_common_symbol             bfd_generic_define_common_symbol
#define asxxxx_bfd_link_hash_table_create           _bfd_generic_link_hash_table_create
#define asxxxx_bfd_link_hash_table_free             _bfd_generic_link_hash_table_free
#define asxxxx_bfd_link_add_symbols                 _bfd_generic_link_add_symbols
#define asxxxx_bfd_link_just_syms                   _bfd_generic_link_just_syms
#define asxxxx_bfd_copy_link_hash_symbol_type       _bfd_generic_copy_link_hash_symbol_type
#define asxxxx_bfd_final_link                       _bfd_generic_final_link
#define asxxxx_bfd_link_split_section               _bfd_generic_link_split_section

const bfd_target asxxxx_vec =
{
  "asxxxx",                     /* Name.  */
  bfd_target_asxxxx_flavour,
  BFD_ENDIAN_UNKNOWN,           /* Target byte order.  */
  BFD_ENDIAN_UNKNOWN,           /* Target headers byte order.  */
  (HAS_RELOC | EXEC_P |         /* Object flags.  */
   HAS_LINENO | HAS_DEBUG |
   HAS_SYMS | HAS_LOCALS | WP_TEXT | D_PAGED),
  (SEC_CODE | SEC_DATA | SEC_ROM | SEC_HAS_CONTENTS
   | SEC_ALLOC | SEC_LOAD | SEC_RELOC), /* Section flags.  */
  0,                            /* Leading underscore.  */
  '/',                          /* AR_pad_char.  */
  15,                           /* AR_max_namelen.  */
  1,                            /* match priority.  */
  bfd_getb64, bfd_getb_signed_64, bfd_putb64,
  bfd_getb32, bfd_getb_signed_32, bfd_putb32,
  bfd_getb16, bfd_getb_signed_16, bfd_putb16,   /* Data.  */
  bfd_getb64, bfd_getb_signed_64, bfd_putb64,
  bfd_getb32, bfd_getb_signed_32, bfd_putb32,
  bfd_getb16, bfd_getb_signed_16, bfd_putb16,   /* Hdrs.  */

  {     /* Check the format of a file being read.  Return a <<bfd_target *>> or zero.  */

    _bfd_dummy_target,
    asxxxx_object_p,            /* object */
    bfd_generic_archive_p,      /* archive */
    _bfd_dummy_target,          /* core */
  },
  {     /* Set the format of a file being written.  */
    bfd_false,
    asxxxx_mkobject,            /* object */
    _bfd_generic_mkarchive,     /* archive */
    bfd_false,                  /* core */
  },
  {     /* Write cached information into a file being written, at <<bfd_close>>.  */
    bfd_false,
    bfd_false, //    asxxxx_write_object_contents, /* object */
    _bfd_write_archive_contents,  /* archive */
    bfd_false,                    /* core */
  },

  BFD_JUMP_TABLE_GENERIC (asxxxx),
  BFD_JUMP_TABLE_COPY (_bfd_generic),
  BFD_JUMP_TABLE_CORE (_bfd_nocore),
  BFD_JUMP_TABLE_ARCHIVE (_bfd_archive_coff),
  BFD_JUMP_TABLE_SYMBOLS (asxxxx),
  BFD_JUMP_TABLE_RELOCS (_bfd_norelocs),
  BFD_JUMP_TABLE_WRITE (asxxxx),
  BFD_JUMP_TABLE_LINK (asxxxx),
  BFD_JUMP_TABLE_DYNAMIC (_bfd_nodynamic),

  NULL,

  NULL
};

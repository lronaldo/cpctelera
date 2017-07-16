/*
  makebin - turn a .ihx file into a binary image or GameBoy format binaryimage

  Copyright (c) 2000 Michael Hope
  Copyright (c) 2010 Borut Razem
  Copyright (c) 2012 Noel Lemouel

  This software is provided 'as-is', without any express or implied
  warranty.  In no event will the authors be held liable for any damages
  arising from the use of this software.

  Permission is granted to anyone to use this software for any purpose,
  including commercial applications, and to alter it and redistribute it
  freely, subject to the following restrictions:

  1. The origin of this software must not be misrepresented; you must not
     claim that you wrote the original software. If you use this software
     in a product, an acknowledgment in the product documentation would be
     appreciated but is not required.
  2. Altered source versions must be plainly marked as such, and must not be
     misrepresented as being the original software.
  3. This notice may not be removed or altered from any source distribution.
*/


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#if defined(_WIN32)
#include <fcntl.h>
#include <io.h>
#else
#include <unistd.h>
#endif


typedef unsigned char BYTE;

#define FILL_BYTE 0xff

int
getnibble (FILE *fin)
{
  int ret;
  int c = getc (fin);

  if (feof (fin) || ferror (fin))
    {
      fprintf (stderr, "error: unexpected end of file.\n");
      exit (6);
    }

  ret = c - '0';
  if (ret > 9)
    {
      ret -= 'A' - '9' - 1;
    }

  if (ret > 0xf)
    {
       ret -= 'a' - 'A';
    }

  if (ret < 0 || ret > 0xf)
    {
      fprintf (stderr, "error: character %02x.\n", ret);
      exit (7);
    }
  return ret;
}

int
getbyte (FILE *fin, int *sum)
{
  int b = (getnibble (fin) << 4) | getnibble (fin);
  *sum += b;
  return b;
}

void
usage (void)
{
  fprintf (stderr,
           "makebin: convert a Intel IHX file to binary or GameBoy format binary.\n"
           "Usage: makebin [options] [<in_file> [<out_file>]]\n"
           "Options:\n"
           "  -p             pack mode: the binary file size will be truncated to the last occupied byte\n"
           "  -s romsize     size of the binary file (default: 32768)\n"
           "  -Z             genarate GameBoy format binary file\n"
           "GameBoy format options (applicable only with -Z option):\n"
           "  -yo n          number of rom banks (default: 2)\n"
           "  -ya n          number of ram banks (default: 0)\n"
           "  -yt n          MBC type (default: no MBC)\n"
           "  -yn name       cartridge name (default: none)\n"
           "Arguments:\n"
           "  <in_file>      optional IHX input file, '-' means stdin. (default: stdin)\n"
           "  <out_file>     optional output file, '-' means stdout. (default: stdout)\n");
}

#define CART_NAME_LEN 16

struct gb_opt_s
{
  char cart_name[CART_NAME_LEN];  /* cartridge name buffer */
  BYTE mbc_type;                  /* MBC type (default: no MBC) */
  short nb_rom_banks;             /* Number of rom banks (default: 2) */
  BYTE nb_ram_banks;              /* Number of ram banks (default: 0) */
};

void
gb_postproc (BYTE * rom, int size, int *real_size, struct gb_opt_s *o)
{
  int i, chk;
  static const BYTE gb_logo[] =
    {
      0xce, 0xed, 0x66, 0x66, 0xcc, 0x0d, 0x00, 0x0b,
      0x03, 0x73, 0x00, 0x83, 0x00, 0x0c, 0x00, 0x0d,
      0x00, 0x08, 0x11, 0x1f, 0x88, 0x89, 0x00, 0x0e,
      0xdc, 0xcc, 0x6e, 0xe6, 0xdd, 0xdd, 0xd9, 0x99,
      0xbb, 0xbb, 0x67, 0x63, 0x6e, 0x0e, 0xec, 0xcc,
      0xdd, 0xdc, 0x99, 0x9f, 0xbb, 0xb9, 0x33, 0x3e
    };

  /* $0104-$0133: Nintendo logo
   * If missing, an actual Game Boy won't run the ROM.
   */

  memcpy (&rom[0x104], gb_logo, sizeof (gb_logo));

  /*
   * 0134-0142: Title of the game in UPPER CASE ASCII. If it
   * is less than 16 characters then the
   * remaining bytes are filled with 00's.
   */

  /* capitalize cartridge name */
  for (i = 0; i < CART_NAME_LEN; ++i)
    {
      rom[0x134 + i] = toupper (o->cart_name[i]);
    }

  /*
   * 0147: Cartridge type:
   * 0-ROM ONLY            12-ROM+MBC3+RAM
   * 1-ROM+MBC1            13-ROM+MBC3+RAM+BATT
   * 2-ROM+MBC1+RAM        19-ROM+MBC5
   * 3-ROM+MBC1+RAM+BATT   1A-ROM+MBC5+RAM
   * 5-ROM+MBC2            1B-ROM+MBC5+RAM+BATT
   * 6-ROM+MBC2+BATTERY    1C-ROM+MBC5+RUMBLE
   * 8-ROM+RAM             1D-ROM+MBC5+RUMBLE+SRAM
   * 9-ROM+RAM+BATTERY     1E-ROM+MBC5+RUMBLE+SRAM+BATT
   * B-ROM+MMM01           1F-Pocket Camera
   * C-ROM+MMM01+SRAM      FD-Bandai TAMA5
   * D-ROM+MMM01+SRAM+BATT FE - Hudson HuC-3
   * F-ROM+MBC3+TIMER+BATT FF - Hudson HuC-1
   * 10-ROM+MBC3+TIMER+RAM+BATT
   * 11-ROM+MBC3
   */
  rom[0x147] = o->mbc_type;

  /*
   * 0148 ROM size:
   * 0 - 256Kbit = 32KByte = 2 banks
   * 1 - 512Kbit = 64KByte = 4 banks
   * 2 - 1Mbit = 128KByte = 8 banks
   * 3 - 2Mbit = 256KByte = 16 banks
   * 4 - 4Mbit = 512KByte = 32 banks
   * 5 - 8Mbit = 1MByte = 64 banks
   * 6 - 16Mbit = 2MByte = 128 banks
   * $52 - 9Mbit = 1.1MByte = 72 banks
   * $53 - 10Mbit = 1.2MByte = 80 banks
   * $54 - 12Mbit = 1.5MByte = 96 banks
   */
  switch (o->nb_rom_banks)
    {
    case 2:
      rom[0x148] = 0;
      break;

    case 4:
      rom[0x148] = 1;
      break;

    case 8:
      rom[0x148] = 2;
      break;

    case 16:
      rom[0x148] = 3;
      break;

    case 32:
      rom[0x148] = 4;
      break;

    case 64:
      rom[0x148] = 5;
      break;

    case 128:
      rom[0x148] = 6;
      break;

    case 256:
      rom[0x148] = 7;
      break;

    case 512:
      rom[0x148] = 8;
      break;

    default:
      fprintf (stderr, "warning: unsupported number of ROM banks (%d)\n", o->nb_rom_banks);
      rom[0x148] = 0;
      break;
    }

  /*
   * 0149 RAM size:
   * 0 - None
   * 1 - 16kBit = 2kB = 1 bank
   * 2 - 64kBit = 8kB = 1 bank
   * 3 - 256kBit = 32kB = 4 banks
   * 4 - 1MBit =128kB =16 banks
   */
  switch (o->nb_ram_banks)
    {
    case 0:
      rom[0x149] = 0;
      break;

    case 1:
      rom[0x149] = 2;
      break;

    case 4:
      rom[0x149] = 3;
      break;

    case 16:
      rom[0x149] = 4;
      break;

    default:
      fprintf (stderr, "warning: unsupported number of RAM banks (%d)\n", o->nb_ram_banks);
      rom[0x149] = 0;
      break;
    }

  /* Update complement checksum */
  chk = 0;
  for (i = 0x134; i < 0x14d; ++i)
    chk += rom[i];
  rom[0x014d] = (unsigned char) (0xe7 - (chk & 0xff));

  /* Update checksum */
  chk = 0;
  rom[0x14e] = 0;
  rom[0x14f] = 0;
  for (i = 0; i < size; ++i)
    chk += rom[i];
  rom[0x14e] = (unsigned char) ((chk >> 8) & 0xff);
  rom[0x14f] = (unsigned char) (chk & 0xff);

  if (*real_size < 0x150)
    *real_size = 0x150;
}

int
read_ihx (FILE *fin, BYTE *rom, int size, int *real_size)
{
  int record_type;

  do
    {
      int nbytes;
      int addr;
      int checksum, sum = 0;

      if (getc (fin) != ':')
        {
          fprintf (stderr, "error: invalid IHX line.\n");
          return 0;
        }
      nbytes = getbyte (fin, &sum);
      addr = getbyte (fin, &sum) << 8 | getbyte (fin, &sum);
      record_type = getbyte (fin, &sum);
      if (record_type > 1)
        {
          fprintf (stderr, "error: unsupported record type: %02x.\n", record_type);
          return 0;
        }

      if (addr + nbytes > size)
        {
          fprintf (stderr, "error: size of the buffer is too small.\n");
          return 0;
        }

      while (nbytes--)
        {
          if (addr < size)
            rom[addr++] = getbyte (fin, &sum);
        }

      if (addr > *real_size)
        *real_size = addr;

      checksum = getbyte (fin, &sum);
      if (0 != (sum & 0xff))
        {
          fprintf (stderr, "error: bad checksum: %02x.\n", checksum);
          return 0;
        }

      while (isspace (sum = getc (fin)))  /* skip all kind of speces */
        ;
      ungetc (sum, fin);
    }
  while (1 != record_type); /* EOF record */

  return 1;
}

int
main (int argc, char **argv)
{
  int size = 32768, pack = 0, real_size = 0;
  BYTE *rom;
  FILE *fin, *fout;
  int ret;
  int gb = 0;
  struct gb_opt_s gb_opt = { "", 0, 2, 0 };

#if defined(_WIN32)
  setmode (fileno (stdout), O_BINARY);
#endif

  while (*++argv && '-' == argv[0][0] && '\0' != argv[0][1])
    {
      switch (argv[0][1])
        {
        case 's':
          if (!*++argv)
            {
              usage ();
              return 1;
            }
          size = atoi (*argv);
          break;

        case 'h':
          usage ();
          return 0;

        case 'p':
          pack = 1;
          break;

        case 'Z':
          /* generate GameBoy binary file */
          gb = 1;
          break;

        case 'y':
          /* GameBoy options:
           * -yo  Number of rom banks (default: 2)
           * -ya  Number of ram banks (default: 0)
           * -yt  MBC type (default: no MBC)
           * -yn  Name of program (default: name of output file)
           */
          switch (argv[0][2])
            {
            case 'o':
              if (!*++argv)
                {
                  usage ();
                  return 1;
                }
              gb_opt.nb_rom_banks = atoi (*argv);
              break;

            case 'a':
              if (!++argv)
                {
                  usage ();
                  return 1;
                }
              gb_opt.nb_ram_banks = atoi (*argv);
              break;

            case 't':
              if (!*++argv)
                {
                  usage ();
                  return 1;
                }
              gb_opt.mbc_type = atoi (*argv);
              break;

            case 'n':
              if (!*++argv)
                {
                  usage ();
                  return 1;
                }
              strncpy (gb_opt.cart_name, *argv, CART_NAME_LEN);
              break;

            default:
              usage ();
              return 1;
            }
          break;

        default:
          usage ();
          return 1;
        }
    }

  fin = stdin;
  fout = stdout;
  if (*argv)
    {
      if ('-' != argv[0][0] || '\0' != argv[0][1])
        {
          if (NULL == (fin = fopen (*argv, "r")))
            {
              fprintf (stderr, "error: can't open %s: ", *argv);
              perror(NULL);
              return 1;
            }
        }
      ++argv;
    }

  if (NULL != argv[0] && NULL != argv[1])
    {
      usage ();
      return 1;
    }

  if (gb && size != 32768)
    {
      fprintf (stderr, "error: only length of 32768 bytes supported for GameBoy binary.\n");
      return 1;
    }

  rom = malloc (size);
  if (rom == NULL)
    {
      fclose (fin);
      fprintf (stderr, "error: couldn't allocate room for the image.\n");
      return 1;
    }
  memset (rom, FILL_BYTE, size);

  ret = read_ihx (fin, rom, size, &real_size);

  fclose (fin);

  if (ret)
    {
      if (gb)
        gb_postproc (rom, size, &real_size, &gb_opt);

      if (*argv)
        {
          if ('-' != argv[0][0] || '\0' != argv[0][1])
            {
              if (NULL == (fout = fopen (*argv, "wb")))
                {
                  fprintf (stderr, "error: can't create %s: ", *argv);
                  perror(NULL);
                  return 1;
                }
            }
        }

      fwrite (rom, 1, (pack ? real_size : size), fout);

      fclose (fout);

      return 0;
    }
  else
    return 1;
}

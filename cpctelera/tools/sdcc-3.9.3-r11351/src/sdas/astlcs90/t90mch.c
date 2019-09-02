/* t90mch.c */

/*
 *  Copyright (C) 2013 Rainer Keuchel
 *
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <limits.h>
#include "asxxxx.h"
#include "t90.h"

#define IS_DIRECT_MEM(V) (((V) & 0xFF00) == 0xFF00)

// these fail on negative numbers..
//#define IS_8BIT_IMMED(V) (((V) & 0xFF00) == 0x0)
//#define IS_16BIT_IMMED(V) (((V) & 0xFFFF0000) == 0x0)

#define IS_8BIT_IMMED(V) 1
#define IS_16BIT_IMMED(V) 1
#define IS_REG_INDIRECT(T) ((T) >= S_IDBC && (T) <= S_IDSP)
#define GET_IND_REG(T) ((T) - S_IDBC)
#define GET_COND_FROM_MODE(V) ((V) & 0xFF)

char *cpu = "Toshiba TLCS90";
char *dsft = "asm";

/*
 * Opcode Cycle Definitions (not yet)
 */
#define OPCY_SDP        ((char) (0xFF))
#define OPCY_ERR        ((char) (0xFE))

/*      OPCY_NONE       ((char) (0x80)) */
/*      OPCY_MASK       ((char) (0x7F)) */

#define OPCY_CPU        ((char) (0xFD))

#define UN      ((char) (OPCY_NONE | 0x00))
#define P2      ((char) (OPCY_NONE | 0x01))
#define P3      ((char) (OPCY_NONE | 0x02))
#define P4      ((char) (OPCY_NONE | 0x03))
#define P5      ((char) (OPCY_NONE | 0x04))
#define P6      ((char) (OPCY_NONE | 0x05))
#define P7      ((char) (OPCY_NONE | 0x06))

/*
 * TLCS-90 Opcode Cycle Pages (NOT CHANGED FROM Z80 YET!)
 */

static char  t90pg1[256] = {
/*--*--* 0  1  2  3  4  5  6  7  8  9  A  B  C  D  E  F */
/*--*--* -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  - */
/*00*/   4,10, 7, 6, 4, 4, 7, 4, 4,11, 7, 6, 4, 4, 7, 4,
/*10*/  13,10, 7, 6, 4, 4, 7, 4,12,11, 7, 6, 4, 4, 7, 4,
/*20*/  12,10,16, 6, 4, 4, 7, 4,12,11,16, 6, 4, 4, 7, 4,
/*30*/  12,10,13, 6,11,11,10, 4,12,11,13, 6, 4, 4, 7, 4,
/*40*/   4, 4, 4, 4, 4, 4, 7, 4, 4, 4, 4, 4, 4, 4, 7, 4,
/*50*/   4, 4, 4, 4, 4, 4, 7, 4, 4, 4, 4, 4, 4, 4, 7, 4,
/*60*/   4, 4, 4, 4, 4, 4, 7, 4, 4, 4, 4, 4, 4, 4, 7, 4,
/*70*/   7, 7, 7, 7, 7, 7, 4, 7, 4, 4, 4, 4, 4, 4, 7, 4,
/*80*/   4, 4, 4, 4, 4, 4, 7, 4, 4, 4, 4, 4, 4, 4, 7, 4,
/*90*/   4, 4, 4, 4, 4, 4, 7, 4, 4, 4, 4, 4, 4, 4, 7, 4,
/*A0*/   4, 4, 4, 4, 4, 4, 7, 4, 4, 4, 4, 4, 4, 4, 7, 4,
/*B0*/   4, 4, 4, 4, 4, 4, 7, 4, 4, 4, 4, 4, 4, 4, 7, 4,
/*C0*/  11,10,10,10,17,11, 7,11,11,10,10,P2,17,17, 7,11,
/*D0*/  11,10,10,11,17,11, 7,11,11, 4,10,11,17,P3, 7,11,
/*E0*/  11,10,10,19,17,11, 7,11,11, 4,10, 4,17,P4, 7,11,
/*F0*/  11,10,10, 4,17,11, 7,11,11, 6,10, 4,17,P5, 7,11
};

static char  t90pg2[256] = {  /* P2 == CB */
/*--*--* 0  1  2  3  4  5  6  7  8  9  A  B  C  D  E  F */
/*--*--* -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  - */
/*00*/   8, 8, 8, 8, 8, 8,15, 8, 8, 8, 8, 8, 8, 8,15, 8,
/*10*/   8, 8, 8, 8, 8, 8,15, 8, 8, 8, 8, 8, 8, 8,15, 8,
/*20*/   8, 8, 8, 8, 8, 8,15, 8, 8, 8, 8, 8, 8, 8,15, 8,
/*30*/  UN,UN,UN,UN,UN,UN,UN,UN, 8, 8, 8, 8, 8, 8,15, 8,
/*40*/   8, 8, 8, 8, 8, 8,12, 8, 8, 8, 8, 8, 8, 8,12, 8,
/*50*/   8, 8, 8, 8, 8, 8,12, 8, 8, 8, 8, 8, 8, 8,12, 8,
/*60*/   8, 8, 8, 8, 8, 8,12, 8, 8, 8, 8, 8, 8, 8,12, 8,
/*70*/   8, 8, 8, 8, 8, 8,12, 8, 8, 8, 8, 8, 8, 8,12, 8,
/*80*/   8, 8, 8, 8, 8, 8,15, 8, 8, 8, 8, 8, 8, 8,15, 8,
/*90*/   8, 8, 8, 8, 8, 8,15, 8, 8, 8, 8, 8, 8, 8,15, 8,
/*A0*/   8, 8, 8, 8, 8, 8,15, 8, 8, 8, 8, 8, 8, 8,15, 8,
/*B0*/   8, 8, 8, 8, 8, 8,15, 8, 8, 8, 8, 8, 8, 8,15, 8,
/*C0*/   8, 8, 8, 8, 8, 8,15, 8, 8, 8, 8, 8, 8, 8,15, 8,
/*D0*/   8, 8, 8, 8, 8, 8,15, 8, 8, 8, 8, 8, 8, 8,15, 8,
/*E0*/   8, 8, 8, 8, 8, 8,15, 8, 8, 8, 8, 8, 8, 8,15, 8,
/*F0*/   8, 8, 8, 8, 8, 8,15, 8, 8, 8, 8, 8, 8, 8,15, 8
};

static char  t90pg3[256] = {  /* P3 == DD */
/*--*--* 0  1  2  3  4  5  6  7  8  9  A  B  C  D  E  F */
/*--*--* -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  - */
/*00*/  UN,UN,UN,UN,UN,UN,UN,UN,UN,15,UN,UN,UN,UN,UN,UN,
/*10*/  UN,UN,UN,UN,UN,UN,UN,UN,UN,15,UN,UN,UN,UN,UN,UN,
/*20*/  UN,14,20,10,UN,UN,UN,UN,UN,15,20,10,UN,UN,UN,UN,
/*30*/  UN,UN,UN,UN,23,23,19,UN,UN,15,UN,UN,UN,UN,UN,UN,
/*40*/  UN,UN,UN,UN,UN,UN,19,UN,UN,UN,UN,UN,UN,UN,19,UN,
/*50*/  UN,UN,UN,UN,UN,UN,19,UN,UN,UN,UN,UN,UN,UN,19,UN,
/*60*/  UN,UN,UN,UN,UN,UN,19,UN,UN,UN,UN,UN,UN,UN,19,UN,
/*70*/  19,19,19,19,19,19,UN,19,UN,UN,UN,UN,UN,UN,19,UN,
/*80*/  UN,UN,UN,UN,UN,UN,19,UN,UN,UN,UN,UN,UN,UN,19,UN,
/*90*/  UN,UN,UN,UN,UN,UN,19,UN,UN,UN,UN,UN,UN,UN,19,UN,
/*A0*/  UN,UN,UN,UN,UN,UN,19,UN,UN,UN,UN,UN,UN,UN,19,UN,
/*B0*/  UN,UN,UN,UN,UN,UN,19,UN,UN,UN,UN,UN,UN,UN,19,UN,
/*C0*/  UN,UN,UN,UN,UN,UN,UN,UN,UN,UN,UN,P6,UN,UN,UN,UN,
/*D0*/  UN,UN,UN,UN,UN,UN,UN,UN,UN,UN,UN,UN,UN,UN,UN,UN,
/*E0*/  UN,14,UN,23,UN,15,UN,UN,UN, 8,UN,UN,UN,UN,UN,UN,
/*F0*/  UN,UN,UN,UN,UN,UN,UN,UN,UN,10,UN,UN,UN,UN,UN,UN
};

static char  t90pg4[256] = {  /* P4 == ED */
/*--*--* 0  1  2  3  4  5  6  7  8  9  A  B  C  D  E  F */
/*--*--* -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  - */
/*00*/  UN,UN,UN,UN,UN,UN,UN,UN,UN,UN,UN,UN,UN,UN,UN,UN,
/*10*/  UN,UN,UN,UN,UN,UN,UN,UN,UN,UN,UN,UN,UN,UN,UN,UN,
/*20*/  UN,UN,UN,UN,UN,UN,UN,UN,UN,UN,UN,UN,UN,UN,UN,UN,
/*30*/  UN,UN,UN,UN,UN,UN,UN,UN,UN,UN,UN,UN,UN,UN,UN,UN,
/*40*/  12,12,15,20, 8,14, 8, 9,12,12,15,20,UN,14,UN, 9,
/*50*/  12,12,15,20,UN,UN, 8, 9,12,12,15,20,UN,UN, 8, 9,
/*60*/  12,12,15,20,UN,UN,UN,18,12,12,15,20,UN,UN,UN,18,
/*70*/  UN,UN,15,20,UN,UN,UN,UN,12,12,15,20,UN,UN,UN,UN,
/*80*/  UN,UN,UN,UN,UN,UN,UN,UN,UN,UN,UN,UN,UN,UN,UN,UN,
/*90*/  UN,UN,UN,UN,UN,UN,UN,UN,UN,UN,UN,UN,UN,UN,UN,UN,
/*A0*/  16,16,16,16,UN,UN,UN,UN,16,16,16,16,UN,UN,UN,UN,
/*B0*/  21,21,21,21,UN,UN,UN,UN,21,21,21,21,UN,UN,UN,UN,
/*C0*/  UN,UN,UN,UN,UN,UN,UN,UN,UN,UN,UN,UN,UN,UN,UN,UN,
/*D0*/  UN,UN,UN,UN,UN,UN,UN,UN,UN,UN,UN,UN,UN,UN,UN,UN,
/*E0*/  UN,UN,UN,UN,UN,UN,UN,UN,UN,UN,UN,UN,UN,UN,UN,UN,
/*F0*/  UN,UN,UN,UN,UN,UN,UN,UN,UN,UN,UN,UN,UN,UN,UN,UN
};

static char  t90pg5[256] = {  /* P5 == FD */
/*--*--* 0  1  2  3  4  5  6  7  8  9  A  B  C  D  E  F */
/*--*--* -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  - */
/*00*/  UN,UN,UN,UN,UN,UN,UN,UN,UN,15,UN,UN,UN,UN,UN,UN,
/*10*/  UN,UN,UN,UN,UN,UN,UN,UN,UN,15,UN,UN,UN,UN,UN,UN,
/*20*/  UN,14,20,10,UN,UN,UN,UN,UN,15,20,10,UN,UN,UN,UN,
/*30*/  UN,UN,UN,UN,23,23,19,UN,UN,15,UN,UN,UN,UN,UN,UN,
/*40*/  UN,UN,UN,UN,UN,UN,19,UN,UN,UN,UN,UN,UN,UN,19,UN,
/*50*/  UN,UN,UN,UN,UN,UN,19,UN,UN,UN,UN,UN,UN,UN,19,UN,
/*60*/  UN,UN,UN,UN,UN,UN,19,UN,UN,UN,UN,UN,UN,UN,19,UN,
/*70*/  19,19,19,19,19,19,UN,19,UN,UN,UN,UN,UN,UN,19,UN,
/*80*/  UN,UN,UN,UN,UN,UN,19,UN,UN,UN,UN,UN,UN,UN,19,UN,
/*90*/  UN,UN,UN,UN,UN,UN,19,UN,UN,UN,UN,UN,UN,UN,19,UN,
/*A0*/  UN,UN,UN,UN,UN,UN,19,UN,UN,UN,UN,UN,UN,UN,19,UN,
/*B0*/  UN,UN,UN,UN,UN,UN,19,UN,UN,UN,UN,UN,UN,UN,19,UN,
/*C0*/  UN,UN,UN,UN,UN,UN,UN,UN,UN,UN,UN,P7,UN,UN,UN,UN,
/*D0*/  UN,UN,UN,UN,UN,UN,UN,UN,UN,UN,UN,UN,UN,UN,UN,UN,
/*E0*/  UN,14,UN,23,UN,15,UN,UN,UN, 8,UN,UN,UN,UN,UN,UN,
/*F0*/  UN,UN,UN,UN,UN,UN,UN,UN,UN,10,UN,UN,UN,UN,UN,UN
};

static char  t90pg6[256] = {  /* P6 == DD CB */
/*--*--* 0  1  2  3  4  5  6  7  8  9  A  B  C  D  E  F */
/*--*--* -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  - */
/*00*/  UN,UN,UN,UN,UN,UN,23,UN,UN,UN,UN,UN,UN,UN,23,UN,
/*10*/  UN,UN,UN,UN,UN,UN,23,UN,UN,UN,UN,UN,UN,UN,23,UN,
/*20*/  UN,UN,UN,UN,UN,UN,23,UN,UN,UN,UN,UN,UN,UN,23,UN,
/*30*/  UN,UN,UN,UN,UN,UN,UN,UN,UN,UN,UN,UN,UN,UN,23,UN,
/*40*/  UN,UN,UN,UN,UN,UN,20,UN,UN,UN,UN,UN,UN,UN,20,UN,
/*50*/  UN,UN,UN,UN,UN,UN,20,UN,UN,UN,UN,UN,UN,UN,20,UN,
/*60*/  UN,UN,UN,UN,UN,UN,20,UN,UN,UN,UN,UN,UN,UN,20,UN,
/*70*/  UN,UN,UN,UN,UN,UN,20,UN,UN,UN,UN,UN,UN,UN,20,UN,
/*80*/  UN,UN,UN,UN,UN,UN,23,UN,UN,UN,UN,UN,UN,UN,23,UN,
/*90*/  UN,UN,UN,UN,UN,UN,23,UN,UN,UN,UN,UN,UN,UN,23,UN,
/*A0*/  UN,UN,UN,UN,UN,UN,23,UN,UN,UN,UN,UN,UN,UN,23,UN,
/*B0*/  UN,UN,UN,UN,UN,UN,23,UN,UN,UN,UN,UN,UN,UN,23,UN,
/*C0*/  UN,UN,UN,UN,UN,UN,23,UN,UN,UN,UN,UN,UN,UN,23,UN,
/*D0*/  UN,UN,UN,UN,UN,UN,23,UN,UN,UN,UN,UN,UN,UN,23,UN,
/*E0*/  UN,UN,UN,UN,UN,UN,23,UN,UN,UN,UN,UN,UN,UN,23,UN,
/*F0*/  UN,UN,UN,UN,UN,UN,23,UN,UN,UN,UN,UN,UN,UN,23,UN
};

static char  t90pg7[256] = {  /* P7 == FD CB */
/*--*--* 0  1  2  3  4  5  6  7  8  9  A  B  C  D  E  F */
/*--*--* -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  - */
/*00*/  UN,UN,UN,UN,UN,UN,23,UN,UN,UN,UN,UN,UN,UN,23,UN,
/*10*/  UN,UN,UN,UN,UN,UN,23,UN,UN,UN,UN,UN,UN,UN,23,UN,
/*20*/  UN,UN,UN,UN,UN,UN,23,UN,UN,UN,UN,UN,UN,UN,23,UN,
/*30*/  UN,UN,UN,UN,UN,UN,UN,UN,UN,UN,UN,UN,UN,UN,23,UN,
/*40*/  UN,UN,UN,UN,UN,UN,20,UN,UN,UN,UN,UN,UN,UN,20,UN,
/*50*/  UN,UN,UN,UN,UN,UN,20,UN,UN,UN,UN,UN,UN,UN,20,UN,
/*60*/  UN,UN,UN,UN,UN,UN,20,UN,UN,UN,UN,UN,UN,UN,20,UN,
/*70*/  UN,UN,UN,UN,UN,UN,20,UN,UN,UN,UN,UN,UN,UN,20,UN,
/*80*/  UN,UN,UN,UN,UN,UN,23,UN,UN,UN,UN,UN,UN,UN,23,UN,
/*90*/  UN,UN,UN,UN,UN,UN,23,UN,UN,UN,UN,UN,UN,UN,23,UN,
/*A0*/  UN,UN,UN,UN,UN,UN,23,UN,UN,UN,UN,UN,UN,UN,23,UN,
/*B0*/  UN,UN,UN,UN,UN,UN,23,UN,UN,UN,UN,UN,UN,UN,23,UN,
/*C0*/  UN,UN,UN,UN,UN,UN,23,UN,UN,UN,UN,UN,UN,UN,23,UN,
/*D0*/  UN,UN,UN,UN,UN,UN,23,UN,UN,UN,UN,UN,UN,UN,23,UN,
/*E0*/  UN,UN,UN,UN,UN,UN,23,UN,UN,UN,UN,UN,UN,UN,23,UN,
/*F0*/  UN,UN,UN,UN,UN,UN,23,UN,UN,UN,UN,UN,UN,UN,23,UN
};

static char *t90Page[7] = {
  t90pg1, t90pg2, t90pg3, t90pg4,
  t90pg5, t90pg6, t90pg7
};

int
gen_xop_arith(int rf, int op, int t1, int t2, int v1, int v2, struct expr *e1, struct expr *e2)
{
  if(IS_REG_INDIRECT(t1) && t2 == S_IMMED)
    {
      int r = GET_IND_REG(t1);

      if(e1->e_addr == 0)
        {
          outab(0xe8 + r);
          outab(op);
          outab(v2);
        }
      else
        {
          if(r == HL) // HL+A
            {
              outab(0xf0 + 7);
              outab(op);
              outab(v2);
            }
          else
            {
              outab(0xf0 + r);
              outab(e1->e_addr & 0xFF);
              outab(op);
              outab(v2);
            }
        }

      return 1;
    }

  if(IS_REG_INDIRECT(t1) && t2 == S_R8)
    {
      // not allowed
      aerr();

      return 1;
    }

  if(IS_REG_INDIRECT(t1) && t2 == S_R16)
    {
      // TODO!
      aerr();
      return 1;
    }

  if(t1 == S_R8 && IS_REG_INDIRECT(t2))
    {
      // TODO!
      aerr();
      return 1;
    }

  if(t1 == S_R16 && IS_REG_INDIRECT(t2))
    {
      int r;

      r = GET_IND_REG(t2);

      if(e2->e_addr == 0)
        {
          outab(0xe0 + r);

          if(v1 == HL)
            outab(op);
          else
            outab(op + v1);
        }
      else
        {
          if(r == HL)
            {
              if(v1 == HL)
                {
                  if(e2->e_addr == 0)
                    {
                      outab(0xf3);
                      outab(op);
                    }
                  else
                    {
                      // HL+A
                      outab(0xf3);
                      outab(op);
                    }
                }
              else
                {
                  aerr();
                }
            }
          else
            {
              outab((0xf0 - IX) + r);
              outab(e2->e_addr);

              if(v1 == HL)
                outab(op);
              else
                outab(op + v1);
            }
        }
      return 1;
    }

  if(t1 == S_R16 && t2 == S_R16)
    {
      outab(0xf8 + v2);

      if(v1 == HL)
        outab(op + 0x10);
      else if(v1 == IX)
        outab(0x14);
      else if(v1 == IY)
        outab(0x15);
      else
        goto err;

      return 1;
    }

  if(t1 == S_R8 && t2 == S_INDM)
    {
      if(v1 == A)
        {
          if(IS_DIRECT_MEM(v2))
            {
              outab(op);
              outab(e2->e_addr & 0xFF);
              return 1;
            }
          else
            {
              outab(0xe3);
              outrw(e2, 0); // reloc..
              outab(op);
              return 1;
            }

          goto err;
        }

      return 1;
    }

  if(t1 == S_R8 && t2 == S_IMMED)
    {
      outab(0xf8 + v1);
      outab(op);
      outab(v2);
      return 1;
    }

  if(t1 == S_R16 && t2 == S_INDM)
    {
      if(IS_DIRECT_MEM(v2))
        {
          if(v1 == HL)
            {
              outab(op);
              outab(v2 & 0xFF);
              return 1;
            }
          else
            {
              outab(0xE7);
              outab(v2 & 0xFF);
              outab(op + v1);
              return 1;
            }
        }
      else
        {
          outab(0xE3);
          outrw(e2, 0); // reloc..

          if(v1 == HL)
            outab(op);
          else
            outab(op + v1);
          return 1;
        }

      goto err;
    }

  if(t1 == S_R8 && t2 == S_R8)
    {
      outab(0xf8 + v2);

      if(v1 == A)
        outab(op);
      else
        outab(op + v1);
      return 1;
    }

  if(t1 == S_INDM && t2 == S_IMMED)
    {
      if(IS_DIRECT_MEM(v1))
        {
          outab(0xef);
          outab(v1 & 0xFF);
          outab(op);
          outab(v2);
          return 1;
        }
      else
        {
          outab(0xeb);
          outrw(e1, 0);
          outab(op);
          outab(v2);
          return 1;
        }
    }

 err:
  aerr();
  return 0;
}

/*
 * Process an insn..
 */

void
machine(struct mne *mp)
{
  int op, t1, t2;
  struct expr e1, e2;
  int rf, v1, v2;

  clrexpr(&e1);
  clrexpr(&e2);

  op = (int) mp->m_valu;
  rf = mp->m_type;

  // NOTE: S_CPU must be max!?

  if(rf > S_CPU)
    rf = 0;

  switch (rf)
    {
    case S_PUSH:
      t1 = addr(&e1);
      v1 = (int) e1.e_addr;

      if ((t1 == S_R16))
        {
          outab(op | (v1 & 0x0F));
          break;
        }

      aerr();
      break;

      //////////////////////////////////////////////////////////////////////

    case S_INCX:
    case S_DECX:
      t1 = addr(&e1);
      v1 = (unsigned short) e1.e_addr;

      // incx (0xff20)
      // decx (0xff20)
      if ((t1 == S_INDM) && (IS_DIRECT_MEM(v1)))
        {
          outab(op);
          outab(v1 & 0xff);
          break;
        }

      aerr();
      break;

      //////////////////////////////////////////////////////////////////////

    case S_RET:
      if (more())
        {
          // ret cc
          if ((v1 = admode(CND)) != 0)
            {
              outab(0xFE);

              outab(0xD0 | GET_COND_FROM_MODE(v1));
              break;
            }
          else
            {
              qerr();
            }
        }
      else
        {
          // ret
          outab(0x1E);
          break;
        }

      aerr();
      break;

      //////////////////////////////////////////////////////////////////////

    case S_CALL:
      if ((v1 = admode(CND)) != 0)
        {
          // call cc

          comma(1);

          t1 = addr(&e1);

          if (t1 == S_USER)
            {
              outab(0xEB);
              outrw(&e1, 0);
              outab(0xD0 + GET_COND_FROM_MODE(v1));
              break;
            }
        }
      else
        {
          t1 = addr(&e1);

          op = 0x1C;

          if (t1 == S_USER)
            {
              outab(op);
              outrw(&e1, 0);
              break;
            }
          else if (IS_REG_INDIRECT(t1))
            {
              int r = GET_IND_REG(t1);

              outab(0xE8 + r);
              outab(0xD8);

              break;
            }
        }

      aerr();
      break;

      //////////////////////////////////////////////////////////////////////

    case S_INH1:
      // singe byte op
      outab(op);
      break;

      //////////////////////////////////////////////////////////////////////

    case S_JP:
      if ((v1 = admode(CND)) != 0)
        {
          comma(1);

          t1 = addr(&e1);

          if(t1 == S_R16)
            {
              op = 0xC0;

              // jp cc
              op |= GET_COND_FROM_MODE(v1);

              aerr();
              break;
            }
          else if(t1 == S_USER)
            {
              outab(0xEB);
              outrw(&e1, 0); // jmp, reloc
              outab(0xC0 | GET_COND_FROM_MODE(v1));
              break;
            }
          else
            {
              aerr();
            }
        }
      else
        {
          t1 = addr(&e1);

          if (t1 == S_USER)
            {
              outab(0x1A);
              outrw(&e1, 0); // jmp, reloc..
              break;
            }
          else if (IS_REG_INDIRECT(t1)) // also S_R16?
            {
              int r = GET_IND_REG(t1);

              outab(0xE8 + r);
              outab(0xC0 | 0x8);
              break;
            }
        }

      aerr();
      break;

      //////////////////////////////////////////////////////////////////////

    case S_JR:
    case S_DJNZ:
    case S_CALLR:
      if (rf == S_JR)
        {
          if((v1 = admode(CND)) != 0)
            {
              op = 0xC0 | GET_COND_FROM_MODE(v1);

              comma(1);
            }
          else
            {
              op = 0xC8;
            }
        }
      else if(rf == S_CALLR)
        {
          t1 = addr(&e2);

          op = 0x1D;
          outab(op);

          e2.e_addr += 1;

          // signed 16-bit pcrel
          outrw(&e2, R_PCR);
          break;
        }
      else if(rf == S_DJNZ)
        {
          op = 0x18;
        }

      //expr(&e2, 0);
      t2 = addr(&e2);

      if(rf == S_DJNZ)
        {
          // djnz bc, xxxx
          if(t2 == S_R16)
            {
              if(e2.e_addr != BC)
                aerr();

              op = 0x19;

              comma(1);

              expr(&e2, 0);
            }
        }

      if (e2.e_mode != S_USER)
        rerr();

      outab(op);

      if (mchpcr(&e2))
        {
          v2 = (int) (e2.e_addr - dot.s_addr - 1);

          // SDCC currently generates jr cc, xxxx calls that exceeds the pc rel limit, i.e. 132..
          if (pass == 2 && ((v2 < -128) || (v2 > 127)))
            aerr();

          outab(v2);
          break;
        }
      else
        {
          outrb(&e2, R_PCR);
          break;
        }

      aerr();
      break;

      //////////////////////////////////////////////////////////////////////

    case S_EX:
      t1 = addr(&e1);
      comma(1);
      t2 = addr(&e2);

      v1 = (int) e1.e_addr;
      v2 = (int) e2.e_addr;

      if (t1 == S_R16 && t2 == S_R16)
        {
          if ((v1 == DE) && (v2 == HL))
            {
              outab(0x08);
              break;
            }
          if ((v2 == DE) && (v1 == HL))
            {
              outab(0x08);
              break;
            }

          if((v1 & 0xF) == AF && (v2 & 0xF) == AF)
            {
              // ex af, af'
              outab(0x09);
              break;
            }

          aerr();
        }

      if(IS_REG_INDIRECT(t1) && t2 == S_R16)
        {
          int r = GET_IND_REG(t1);

          if(e1.e_addr == 0)
            {
              outab(0xE0 + r);
              outab(op + v2);
              break;
            }
          else
            {
              if(r == HL)
                {
                  outab(0xF3);
                  outab(op + v2);
                  break;
                }
              else
                {
                  outab(0xF0 + (r - IX));
                  outab(e1.e_addr);
                  outab(op + v2);
                  break;
                }
            }
        }

      if(t1 == S_R16 && IS_REG_INDIRECT(t2))
        {
          int r = GET_IND_REG(t2);

          if(e2.e_addr == 0)
            {
              outab(0xE0 + r);
              outab(op + v1);
              break;
            }
          else
            {
              if(r == HL)
                {
                  outab(0xF3);
                  outab(op + v1);
                  break;
                }
              else
                {
                  outab(0xF0 + (r - IX));
                  outab(e2.e_addr);
                  outab(op + v1);
                  break;
                }
            }
        }

      aerr();
      break;

      //////////////////////////////////////////////////////////////////////

    case S_DEC:
    case S_INC:
      t1 = addr(&e1);
      v1 = (int) e1.e_addr;

      if(IS_REG_INDIRECT(t1))
        {
          int r;

          r = GET_IND_REG(t1);

          if(e1.e_addr == 0)
            {
              outab(0xe0 + r);
              outab(op + 7);
              break;
            }
          else
            {
              if(r == HL)
                {
                  outab(0xF3);
                  outab(op + 7);
                  break;
                }
              else
                {
                  outab(0xec + r);
                  outab(e1.e_addr);
                  outab(op + 7);
                  break;
                }
            }
        }

      if(t1 == S_R8)
        {
          outab(op + v1);
          break;
        }

      if(t1 == S_R16)
        {
          outab(op + 0x10 + v1);
          break;
        }

      if ((t1 == S_INDM))
        {
          if(IS_DIRECT_MEM(v1))
            {
              outab(op + 7);
              outab(v1 & 0xff);
              break;
            }
          else
            {
              outab(0xE3);
              outrw(&e1, 0); // reloc
              outab(op + 7);
              break;
            }
        }

      aerr();
      break;

      //////////////////////////////////////////////////////////////////////

    case S_INCW:
    case S_DECW:
      t1 = addr(&e1);
      v1 = (int) e1.e_addr;

      if ((t1 == S_INDM))
        {
          if((IS_DIRECT_MEM(v1)))
            {
              outab(op);
              outab(v1 & 0xff);
              break;
            }
          else
            {
              outab(0xE3);
              outrw(&e1, 0);
              outab(op);
              break;
            }
        }
      else if(IS_REG_INDIRECT(t1))
        {
          int r = GET_IND_REG(t1);

          if(e1.e_addr == 0)
            {
              outab(0xe0 + r);
              outab(op);
              break;
            }
          else
            {
              if(r == HL) // HL+A
                {
                  outab(0xf0 + 3);
                  outab(op);
                  break;
                }
              else
                {
                  outab(0xf0 + (r - IX));
                  outab(v1);
                  outab(op);
                  break;
                }
            }
        }

      aerr();
      break;

      //////////////////////////////////////////////////////////////////////

    case S_MUL:
    case S_DIV:
      t1 = addr(&e1);
      v1 = (int) e1.e_addr;
      comma(1);
      t2 = addr(&e2);
      v2 = (int) e2.e_addr;

      if(t1 == S_R16 && t2 == S_R8)
        {
          if(v1 != HL)
            aerr();

          outab(0xF8 + v2);
          outab(op);
          break;
        }

      if(t1 == S_R16 && t2 == S_INDM)
        {
          if(v1 == HL && (IS_DIRECT_MEM(v2)))
            {
              outab(0xE7);
              outab(v2 & 0xFF);
              outab(op);
              break;
            }

          aerr();
        }

      if(t1 == S_R16 && t2 == S_IMMED)
        {
          if(v1 == HL && (IS_8BIT_IMMED(v2)))
            {
              outab(op);
              outab(v2);
              break;
            }

          aerr();
        }

      if(t1 == S_R16 && IS_REG_INDIRECT(t2))
        {
          int r = GET_IND_REG(t2);

          if(v1 == HL)
            {
              if(e2.e_addr == 0)
                {
                  outab(0xE0 | r);
                  outab(op);
                }
              else
                {
                  if(r == HL) // HL+A
                    {
                      outab(0xF3);
                      outab(op);
                    }
                  else
                    {
                      outab(0xF0 + (r - IX));
                      outab(e2.e_addr & 0xFF);
                      outab(op);
                    }
                }
              break;
            }
        }

      aerr();
      break;

      //////////////////////////////////////////////////////////////////////

    case S_BIT:
    case S_RES:
    case S_SET:
      t1 = addr(&e1);
      v1 = (int) e1.e_addr;
      comma(1);
      t2 = addr(&e2);
      v2 = (int) e2.e_addr;

      if(t1 == S_USER && t2 == S_R8)
        {
          outab(0xF8 + v2);
          outab(op + v1);
          break;
        }

      if ((t1 == S_USER && t2 == S_INDM))
        {
          if(v1 < 0 || v1 > 7)
            aerr();

          if((IS_DIRECT_MEM(v2)))
            {
              outab(op + v1);
              outab(v2 & 0xff);
              break;
            }
          else
            {
              outab(0xe3);
              outrw(&e2, 0);
              outab(op + v1);
              outab(v2 & 0xff);
              break;
            }
        }

      if ((t1 == S_USER && IS_REG_INDIRECT(t2)))
        {
          int r = GET_IND_REG(t2);

          if(v1 < 0 || v1 > 7)
            aerr();

          if(e2.e_addr == 0)
            {
              outab(0xe0 + r);
              outab(op + v1);
              break;
            }
          else
            {
              if(r == HL) // HL+A
                {
                  outab(0xf3);
                  outab(op + v1);
                  break;
                }

              if(r == IX)
                outab(0xF0);
              else if(r == IY)
                outab(0xF1);
              else if(r == SP)
                outab(0xF2);
              else
                aerr();

              outab(e2.e_addr & 0xFF);
              outab(op + v1);
              break;
            }
        }

      aerr();
      break;

      //////////////////////////////////////////////////////////////////////

    case S_CP:
    case S_OR:
    case S_XOR:
    case S_AND:
    case S_ADD:
    case S_ADC:
    case S_SUB:
    case S_SBC:
      t1 = addr(&e1);
      v1 = (int) e1.e_addr;
      comma(1);
      t2 = addr(&e2);
      v2 = (int) e2.e_addr;

      // add r8, r8
      if(t1 == S_R8 && t2 == S_R8)
        {
          gen_xop_arith(rf, op, t1, t2, v1, v2, &e1, &e2);
          break;
        }

      // add (mm), imd
      if(t1 == S_INDM && t2 == S_IMMED)
        {
          gen_xop_arith(rf, op + 0x8, t1, t2, v1, v2, &e1, &e2);
          break;
        }

      // add r, (mm)
      if(t1 == S_R8 && t2 == S_INDM)
        {
          if(v1 == A)
            {
              if(IS_DIRECT_MEM(v2))
                {
                  outab(op);
                  outab(v2 & 0xff);
                  break;
                }
              else
                {
                  gen_xop_arith(rf, op, t1, t2, v1, v2, &e1, &e2);
                  break;
                }
            }
          else
            {
              // only a allowed!?
              aerr();
            }
        }

      // add r, (rr)
      if(t1 == S_R8 && IS_REG_INDIRECT(t2))
        {
          int r = GET_IND_REG(t2);

          if(v1 != A)
            {
              aerr();
              break;
            }

          if(e2.e_addr == 0)
            {
              outab(0xE0 + r);
              outab((op - 6) + v1);
              break;
            }
          else
            {
              if(r == HL) // HL+A
                {
                  outab(0xF3);
                  outab(op);
                  break;
                }

              outab(0xEC + r);
              outab(e2.e_addr & 0xFF);
              outab((op - 6) + v1);
              break;
            }
        }

      // add rr, (rr)
      if(t1 == S_R16 && IS_REG_INDIRECT(t2))
        {
          // not allowed
          if(v1 == BC || v1 == DE)
            {
              aerr();
              break;
            }

          if(rf != S_ADD && v1 != HL)
            {
              aerr();
              break;
            }

          if(v1 == HL)
            gen_xop_arith(rf, op + 0x10, t1, t2, v1, v2, &e1, &e2);
          else
            gen_xop_arith(rf, 0x10, t1, t2, v1, v2, &e1, &e2);

          break;
        }

      // add (rr), r
      if(IS_REG_INDIRECT(t1) && t2 == S_R8)
        {
          gen_xop_arith(rf, op, t1, t2, v1, v2, &e1, &e2);
          break;
        }

      // add (rr), imd
      if(IS_REG_INDIRECT(t1) && t2 == S_IMMED)
        {
          gen_xop_arith(rf, op + 0x8, t1, t2, v1, v2, &e1, &e2);
          break;
        }

      // add r, imm
      if(t1 == S_R8 && t2 == S_IMMED)
        {
          if((v1 == A) && (IS_8BIT_IMMED(v2)))
            {
              outab(op + 8);
              outrb(&e2,0);
              break;
            }
          else
            {
              gen_xop_arith(rf, op + 8, t1, t2, v1, v2, &e1, &e2);
              break;
            }
        }

      // add rr, (mm)
      if(t1 == S_R16 && t2 == S_INDM)
        {
          // bc, de not allowed..
          if(v1 == BC || v1 == DE)
            aerr();

          if(v1 == HL)
            {
              if(IS_DIRECT_MEM(v2))
                {
                  outab(op + 0x10);
                  outab(v2 & 0xff);
                  break;
                }
              else
                {
                  gen_xop_arith(rf, op + 0x10, t1, t2, v1, v2, &e1, &e2);
                  break;
                }
            }
          else
            {
              gen_xop_arith(rf, 0x10, t1, t2, v1, v2, &e1, &e2);
              break;
            }
        }

      // add rr, rr
      if(t1 == S_R16 && t2 == S_R16)
        {
          gen_xop_arith(rf, op, t1, t2, v1, v2, &e1, &e2);
          break;
        }

      // add rr, imd
      if(t1 == S_R16 && t2 == S_IMMED)
        {
          // sub not allowed on sp, ix, iy
          // de, bc not allowed at all!?

          if(v1 == IX)
            {
              if(rf == S_ADD)
                {
                  outab(0x14);
                  outrw(&e2, 0);
                  break;
                }
              aerr();
            }
          else if(v1 == IY)
            {
              if(rf == S_ADD)
                {
                  outab(0x15);
                  outrw(&e2, 0);
                  break;
                }
              aerr();
            }
          else if(v1 == SP)
            {
              if(rf == S_ADD)
                {
                  outab(0x16);
                  outrw(&e2, 0);
                  break;
                }
              aerr();
            }
          else
            {
              // only hl allowed?
              if(v1 != HL)
                aerr();

              outab(op + 0x16 + v1);
              outrw(&e2, 0);
              break;
            }
        }

      aerr();
      break;

      //////////////////////////////////////////////////////////////////////

    case S_RLC:
    case S_RRC:
    case S_RL:
    case S_RR:
    case S_SLA:
    case S_SRA:
    case S_SLL:
    case S_SRL:
      t1 = addr(&e1);
      v1 = (int) e1.e_addr;
      // can have a as first op or not..
      if(more())
        {
          if(t1 == S_R8 && v1 == A)
            {
              clrexpr(&e1);

              comma(1);

              t1 = addr(&e1);
              v1 = (int) e1.e_addr;
            }
          else
            {
              aerr();
              break;
            }
        }

      if(IS_DIRECT_MEM(v1) && v1 > 0)
        {
          outab(0xE7);
          outab(v1 & 0xFF);
          outab(0xA0 + (rf - S_RLC));
          break;
        }

      if(IS_REG_INDIRECT(t1))
        {
          int r = GET_IND_REG(t1);

          if(v1 == 0)
            {
              outab(0xE0 + r);
              outab(0xA0 + (rf - S_RLC));
              break;
            }
          else
            {
              if(r == HL) // HL+A
                {
                  outab(0xF3);
                  outab(0xA0 + (rf - S_RLC));
                  break;
                }

              outab(0xF0 + (r - IX));
              outab(v1 & 0xFF);
              outab(0xA0 + (rf - S_RLC));
              break;
            }
        }

      if(t1 == S_R8)
        {
#if 0
          // TODO: not used by asl assembler..
          if(v1 == A)
            {
              outab(0xA0 + (rf - S_RLC));
              break;
            }
          else
#endif
            {
              outab(0xF8 + v1);
              outab(0xA0 + (rf - S_RLC));
              break;
            }
        }

      aerr();
      break;

      //////////////////////////////////////////////////////////////////////

    case S_LDI:
      outab(0xFE);
      outab(0x58);
      break;

    case S_LDIR:
      outab(0xFE);
      outab(0x59);
      break;

    case S_LDD:
      outab(0xFE);
      outab(0x5A);
      break;

    case S_LDDR:
      outab(0xFE);
      outab(0x5B);
      break;

    case S_CPI:
      outab(0xFE);
      outab(0x5C);
      break;

    case S_CPIR:
      outab(0xFE);
      outab(0x5D);
      break;

    case S_CPD:
      outab(0xFE);
      outab(0x5E);
      break;

    case S_CPDR:
      outab(0xFE);
      outab(0x5F);
      break;

      //////////////////////////////////////////////////////////////////////

    case S_LDAR:
      // load relative
      t1 = addr(&e1);
      v1 = (int) e1.e_addr;
      comma(1);
      t2 = addr(&e2);
      v2 = (int) e2.e_addr;

      if(t1 == S_R16 && t2 == S_USER)
        {
          if(v1 == HL)
            {
              outab(op);

              e2.e_addr += 1;

              // signed 16-bit pcrel
              outrw(&e2, R_PCR);

              break;
            }
        }

      aerr();
      break;

      //////////////////////////////////////////////////////////////////////

    case S_LDW:
      t1 = addr(&e1);
      v1 = (int) e1.e_addr;
      comma(1);
      t2 = addr(&e2);
      v2 = (int) e2.e_addr;

      if(t1 == S_INDM && t2 == S_IMMED)
        {
          if(IS_DIRECT_MEM(v1))
            {
              outab(0x3F);
              outab(v1 & 0xff);
              outrw(&e2, 0);
              break;
            }
          else
            {
              outab(0xEB);
              outrw(&e2, 0); // addr
              outab(0x3F);
              outrw(&e2, 0); // immd
              break;
            }

          aerr();
        }

      if(IS_REG_INDIRECT(t1) && t2 == S_IMMED)
        {
          int r;

          r = GET_IND_REG(t1);

          if(e1.e_addr == 0)
            {
              outab(0xe8 + r);
              outab(0x3F);
              outrw(&e2, 0);
              break;
            }
          else
            {
              if(r == HL)
                {
                  outab(0xF7);
                  outab(0x3F);
                  outrw(&e2, 0);
                  break;
                }

              outab(0xF0 + r);
              outab(e1.e_addr & 0xFF);
              outab(0x3F);
              outrw(&e2, 0);
              break;
            }
        }

      aerr();
      break;

      //////////////////////////////////////////////////////////////////////

    case S_LD:
      t1 = addr(&e1);
      v1 = (int) e1.e_addr;
      comma(1);
      t2 = addr(&e2);
      v2 = (int) e2.e_addr;

      if(t1 == S_R8 && t2 == S_R8)
        {
          if(v1 == A)
            {
              outab(0x20 + v2);
              break;
            }
          if(v2 == A)
            {
              outab(0x28 + v1);
              break;
            }

          outab(0xF8 + v2);
          outab(0x30 + v1);
          break;

          aerr();
        }

      if(IS_REG_INDIRECT(t1) && t2 == S_R8)
        {
          int r;

          r = GET_IND_REG(t1);

          if(e1.e_addr == 0)
            {
              outab(0xe8 + r);
              outab(0x20 + v2);
              break;
            }
          else
            {
              if(r == IX)
                outab(0xf4);
              else if(r == IY)
                outab(0xf5);
              else if(r == SP)
                outab(0xf6);
              else if(r == HL)
                {
                  outab(0xf7); // HL+A
                  outab(0x20 + v2);
                  break;
                }
              else
                aerr();

              outab(e1.e_addr);
              outab(0x20 + v2);
              break;
            }
        }

      if(t1 == S_R8 && IS_REG_INDIRECT(t2))
        {
          int r;

          r = GET_IND_REG(t2);

          if(e2.e_addr == 0)
            {
              outab(0xe0 + r);
              outab(0x28 + v1);
            }
          else
            {
              if(r == IX)
                outab(0xf0);
              else if(r == IY)
                outab(0xf1);
              else if(r == SP)
                outab(0xf2);
              else if(r == HL) // HL+A
                {
                  outab(0xf3);
                  outab(0x28 + v1);
                  break;
                }
              else
                aerr();

              outab(e2.e_addr);
              outab(0x28 + v1);
            }

          break;
        }

      if(IS_REG_INDIRECT(t1) && t2 == S_R16)
        {
          int r;

          r = GET_IND_REG(t1);

          if(e1.e_addr == 0)
            {
              outab(0xe8 + r);
              outab(0x40 + v2);
              break;
            }
          else
            {
              if(r == HL) // HL+A
                {
                  outab(0xF7);
                  outab(0x40 + v2);
                  break;
                }

              outab(0xF0 + r);
              outab(e1.e_addr);
              outab(0x40 + v2);
              break;
            }
        }

      if(IS_REG_INDIRECT(t1) && t2 == S_IMMED)
        {
          int r;

          r = GET_IND_REG(t1);

          if(e1.e_addr == 0)
            {
              outab(0xe8 + r);
              outab(0x37);
              outrb(&e2,0);
              break;
            }
          else
            {
              if(r == HL) // HL+A
                {
                  outab(0xF7);
                  outab(0x37);
                  outrb(&e2,0);
                  break;
                }

              outab(0xF0 + r);
              outab(e1.e_addr);
              outab(0x37);
              outrb(&e2,0);
              break;
            }
        }

      if(t1 == S_R16 && IS_REG_INDIRECT(t2))
        {
          int r;

          r = GET_IND_REG(t2);

          if(e2.e_addr == 0)
            {
              outab(0xe0 + r);
              outab(0x48 + v1);
            }
          else
            {
              if(r == IX)
                outab(0xf0);
              else if(r == IY)
                outab(0xf1);
              else if(r == SP)
                outab(0xf2);
              else if(r == HL) // HL+A
                {
                  outab(0xf3);
                  outab(0x48 + v1);
                  break;
                }
              else
                aerr();

              outab(e2.e_addr);
              outab(0x48 + v1);
            }

          break;
        }

      if(t1 == S_R8 && t2 == S_IMMED)
        {
          outab(0x30 + v1);
          outrb(&e2,0);
          break;
        }

      if(t1 == S_R8 && t2 == S_INDM)
        {
          if(IS_DIRECT_MEM(v2) && v1 == A)
            {
              outab(0x27);
              outab(v2 & 0xff);
              break;
            }

          if(IS_DIRECT_MEM(v2))
            {
              outab(0xE7);
              outab(v2 & 0xff);
              outab(0x28 + v1);
              break;
            }
          else
            {
              // normal access
              outab(0xE3);
              outrw(&e2, 0); // reloc!?
              outab(0x28 + v1);
              break;
            }

          aerr();
        }

#if 0
      // alternate encodings or disassmbler bug?
      if(t1 == S_R8 && t2 == S_INDM)
        {
          if(v1 == B)
            {
              outab(0xE3);
              outrw(&e2, 0);
              outab(0x28);
              break;
            }
          if(v1 == C)
            {
              outab(0xE3);
              outrw(&e2, 0);
              outab(0x29);
              break;
            }
          if(v1 == D)
            {
              outab(0xE3);
              outrw(&e2, 0);
              outab(0x2A);
              break;
            }
          if(v1 == A)
            {
              outab(0xE3);
              outrw(&e2, 0);
              outab(0x2E);
              break;
            }
        }
#endif

      if(t1 == S_R16 && t2 == S_R16)
        {
          if(v1 == HL)
            {
              outab(0x40 + v2);
              break;
            }
          if(v2 == HL)
            {
              outab(0x48 + v1);
              break;
            }

          outab(0xF8 + v2);
          outab(0x38 + v1);

          break;
        }

      if(t1 == S_R16 && t2 == S_IMMED)
        {
          if(IS_16BIT_IMMED(v2))
            {
              outab(0x38 + v1);
              outrw(&e2, 0);
              break;
            }
        }

      if(t1 == S_R16 && t2 == S_INDM)
        {
          if(IS_DIRECT_MEM(v2))
            {
              if(v1 == HL)
                {
                  outab(0x47);
                  outab(v2 & 0xFF);
                  break;
                }
              else
                {
                  outab(0xE7);
                  outab(v2 & 0xFF);
                  outab(0x48 + v1);
                  break;
                }
            }
          else
            {
              outab(0xE3);
              outrw(&e2, 0); // reloc!?
              outab(0x48 + v1);
              break;
            }
        }

      if(t1 == S_INDM && t2 == S_R8)
        {
          if(IS_DIRECT_MEM(v1) && v2 == A)
            {
              outab(0x2F);
              outab(v1 & 0xff);
              break;
            }
          else if(IS_DIRECT_MEM(v1))
            {
              outab(0xEF);
              outab(v1 & 0xff);
              outab(0x20 + v2);
              break;
            }
          else
            {
              outab(0xEB);
              outrw(&e1, 0);
              outab(0x20 + v2);
              break;
            }

          aerr();
        }

      if(t1 == S_INDM && t2 == S_R16)
        {
          if(IS_DIRECT_MEM(v1))
            {
              if(v2 == HL)
                {
                  outab(0x4F);
                  outab(v1 & 0xFF);
                  break;
                }
              else
                {
                  outab(0xEF);
                  outab(v1 & 0xFF);
                  outab(0x40 + v2);
                  break;
                }
            }
          else
            {
              outab(0xEB);
              outrw(&e1, 0);
              outab(0x40 + v2);
              break;
            }
        }
      if(t1 == S_INDM && t2 == S_IMMED)
        {
          if(IS_DIRECT_MEM(v1))
            {
              outab(0x37);
              outab(v1 & 0xff);
              outab(v2 & 0xff);
              break;
            }
          else
            {
              outab(0xeb);
              outrw(&e1, 0);
              outab(0x37);
              outab(v2 & 0xff);
              break;
            }

          aerr();
        }

      aerr();
      break;

      //////////////////////////////////////////////////////////////////////

    case S_RLD:
    case S_RRD:

      // implicit (hl) operand
      outab(0xE2);
      outab(op);
      break;

      //////////////////////////////////////////////////////////////////////

    case S_CPU:
      opcycles = OPCY_CPU;
      //mchtyp = op;
      sym[2].s_addr = op;
      lmode = SLIST;
      break;

      //////////////////////////////////////////////////////////////////////

    default:
      opcycles = OPCY_ERR;
      err('o');
      break;
    }

  if (opcycles == OPCY_NONE)
    {
      opcycles = t90pg1[cb[0] & 0xFF];
      while ((opcycles & OPCY_NONE) && (opcycles & OPCY_MASK))
        {
          switch (opcycles) {
          case P2:        /* CB xx        */
          case P3:        /* DD xx        */
          case P4:        /* ED xx        */
          case P5:        /* FD xx        */
            opcycles = t90Page[opcycles & OPCY_MASK][cb[1] & 0xFF];
            break;
          case P6:        /* DD CB -- xx  */
          case P7:        /* FD CB -- xx  */
            opcycles = t90Page[opcycles & OPCY_MASK][cb[3] & 0xFF];
            break;
          default:
            opcycles = OPCY_NONE;
            break;
          }
        }
    }
}

/*
 * Branch/Jump PCR Mode Check
 */

int
mchpcr(struct expr *esp)
{
  if (esp->e_base.e_ap == dot.s_area)
    {
      return(1);
    }

  if (esp->e_flag==0 && esp->e_base.e_ap==NULL)
    {
      /*
       * Absolute Destination
       *
       * Use the global symbol '.__.ABS.'
       * of value zero and force the assembler
       * to use this absolute constant as the
       * base value for the relocation.
       */
      esp->e_flag = 1;
      esp->e_base.e_sp = &sym[1];
    }

  return(0);
}

/*
 * Machine dependent initialization
 */

VOID
minit()
{
  /*
   * Byte Order
   */
  hilo = 0;

  if (pass == 0)
    {
      sym[2].s_addr = X_T90;
    }
}

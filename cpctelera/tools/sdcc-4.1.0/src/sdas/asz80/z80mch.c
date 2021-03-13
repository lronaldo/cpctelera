/* z80mch.c */

/*
 *  Copyright (C) 1993-2006  Alan R. Baldwin
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
 *
 *
 * Alan R. Baldwin
 * 721 Berkeley St.
 * Kent, Ohio  44240
 */

/*
 * Extensions: P. Felber
 * eZ80 port (based on ASxxxx sources): H. Sladky
 */

#include "asxxxx.h"
#include "z80.h"

char    *cpu    = "Zilog Z80 / Hitachi HD64180 / ZX-Next / eZ80";
char    *dsft   = "asm";

char    imtab[3] = { 0x46, 0x56, 0x5E };
int     mchtyp;
int     allow_undoc;

static int ez80_adl = 0;

/*
 * Opcode Cycle Definitions
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

#define PF      ((char) (OPCY_NONE | 0x10))

/*
 * Z80 Opcode Cycle Pages
 */

static const char  z80pg1[256] = {
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

static const char  z80pg2[256] = {  /* P2 == CB */
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

static const char  z80pg3[256] = {  /* P3 == DD */
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

static const char  z80pg4[256] = {  /* P4 == ED */
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

static const char  z80pg5[256] = {  /* P5 == FD */
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

static const char  z80pg6[256] = {  /* P6 == DD CB */
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

static const char  z80pg7[256] = {  /* P7 == FD CB */
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

static const char *z80Page[7] = {
    z80pg1, z80pg2, z80pg3, z80pg4,
    z80pg5, z80pg6, z80pg7
};

/*
 * HD64180 / Z180  Opcode Cycle Pages
 */

static const char  hd64pg1[256] = {
/*--*--* 0  1  2  3  4  5  6  7  8  9  A  B  C  D  E  F */
/*--*--* -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  - */
/*00*/   3, 9, 7, 4, 4, 4, 6, 3, 4, 7, 6, 4, 4, 4, 6, 3,
/*10*/   9, 9, 7, 4, 4, 4, 6, 3, 8, 7, 6, 4, 4, 4, 6, 3,
/*20*/   8, 9,16, 4, 4, 4, 6, 4, 8, 7,15, 4, 4, 4, 6, 3,
/*30*/   8, 9,13, 4,10,10, 9, 3, 8, 7,12, 4, 4, 4, 6, 3,
/*40*/   4, 4, 4, 4, 4, 4, 6, 4, 4, 4, 4, 4, 4, 4, 6, 4,
/*50*/   4, 4, 4, 4, 4, 4, 6, 4, 4, 4, 4, 4, 4, 4, 6, 4,
/*60*/   4, 4, 4, 4, 4, 4, 6, 4, 4, 4, 4, 4, 4, 4, 6, 4,
/*70*/   7, 7, 7, 7, 7, 7, 3, 7, 4, 4, 4, 4, 4, 4, 6, 4,
/*80*/   4, 4, 4, 4, 4, 4, 6, 4, 4, 4, 4, 4, 4, 4, 6, 4,
/*90*/   4, 4, 4, 4, 4, 4, 6, 4, 4, 4, 4, 4, 4, 4, 6, 4,
/*A0*/   4, 4, 4, 4, 4, 4, 6, 4, 4, 4, 4, 4, 4, 4, 6, 4,
/*B0*/   4, 4, 4, 4, 4, 4, 6, 4, 4, 4, 4, 4, 4, 4, 6, 4,
/*C0*/  10, 9, 9, 9,16,11, 6,11,10, 9, 9,P2,16,16, 6,11,
/*D0*/  10, 9, 9,10,16,11, 6,11,10, 3, 9, 9,16,P3, 6,11,
/*E0*/  10, 9, 9,16,16,11, 6,11,10, 3, 9, 3,16,P4, 6,11,
/*F0*/  10, 9, 9, 3,16,11, 6,11,10, 4, 9, 3,16,P5, 6,11
};

static const char  hd64pg2[256] = {  /* P2 == CB */
/*--*--* 0  1  2  3  4  5  6  7  8  9  A  B  C  D  E  F */
/*--*--* -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  - */
/*00*/   7, 7, 7, 7, 7, 7,13, 7, 7, 7, 7, 7, 7, 7,13, 7,
/*10*/   7, 7, 7, 7, 7, 7,13, 7, 7, 7, 7, 7, 7, 7,13, 7,
/*20*/   7, 7, 7, 7, 7, 7,13, 7, 7, 7, 7, 7, 7, 7,13, 7,
/*30*/  UN,UN,UN,UN,UN,UN,UN,UN, 7, 7, 7, 7, 7, 7,13, 7,
/*40*/   7, 7, 7, 7, 7, 7,13, 7, 7, 7, 7, 7, 7, 7,13, 7,
/*50*/   7, 7, 7, 7, 7, 7,13, 7, 7, 7, 7, 7, 7, 7,13, 7,
/*60*/   7, 7, 7, 7, 7, 7,13, 7, 7, 7, 7, 7, 7, 7,13, 7,
/*70*/   7, 7, 7, 7, 7, 7,13, 7, 7, 7, 7, 7, 7, 7,13, 7,
/*80*/   7, 7, 7, 7, 7, 7,13, 7, 7, 7, 7, 7, 7, 7,13, 7,
/*90*/   7, 7, 7, 7, 7, 7,13, 7, 7, 7, 7, 7, 7, 7,13, 7,
/*A0*/   7, 7, 7, 7, 7, 7,13, 7, 7, 7, 7, 7, 7, 7,13, 7,
/*B0*/   7, 7, 7, 7, 7, 7,13, 7, 7, 7, 7, 7, 7, 7,13, 7,
/*C0*/   7, 7, 7, 7, 7, 7,13, 7, 7, 7, 7, 7, 7, 7,13, 7,
/*D0*/   7, 7, 7, 7, 7, 7,13, 7, 7, 7, 7, 7, 7, 7,13, 7,
/*E0*/   7, 7, 7, 7, 7, 7,13, 7, 7, 7, 7, 7, 7, 7,13, 7,
/*F0*/   7, 7, 7, 7, 7, 7,13, 7, 7, 7, 7, 7, 7, 7,13, 7
};

static const char  hd64pg3[256] = {  /* P3 == DD */
/*--*--* 0  1  2  3  4  5  6  7  8  9  A  B  C  D  E  F */
/*--*--* -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  - */
/*00*/  UN,UN,UN,UN,UN,UN,UN,UN,UN,10,UN,UN,UN,UN,UN,UN,
/*10*/  UN,UN,UN,UN,UN,UN,UN,UN,UN,10,UN,UN,UN,UN,UN,UN,
/*20*/  UN,12,19, 7,UN,UN,UN,UN,UN,10,18, 7,UN,UN,UN,UN,
/*30*/  UN,UN,UN,UN,18,18,15,UN,UN,10,UN,UN,UN,UN,UN,UN,
/*40*/  UN,UN,UN,UN,UN,UN,14,UN,UN,UN,UN,UN,UN,UN,14,UN,
/*50*/  UN,UN,UN,UN,UN,UN,14,UN,UN,UN,UN,UN,UN,UN,14,UN,
/*60*/  UN,UN,UN,UN,UN,UN,14,UN,UN,UN,UN,UN,UN,UN,14,UN,
/*70*/  15,15,15,15,15,15,UN,15,UN,UN,UN,UN,UN,UN,14,UN,
/*80*/  UN,UN,UN,UN,UN,UN,14,UN,UN,UN,UN,UN,UN,UN,14,UN,
/*90*/  UN,UN,UN,UN,UN,UN,14,UN,UN,UN,UN,UN,UN,UN,14,UN,
/*A0*/  UN,UN,UN,UN,UN,UN,14,UN,UN,UN,UN,UN,UN,UN,14,UN,
/*B0*/  UN,UN,UN,UN,UN,UN,14,UN,UN,UN,UN,UN,UN,UN,14,UN,
/*C0*/  UN,UN,UN,UN,UN,UN,UN,UN,UN,UN,UN,P6,UN,UN,UN,UN,
/*D0*/  UN,UN,UN,UN,UN,UN,UN,UN,UN,UN,UN,UN,UN,UN,UN,UN,
/*E0*/  UN,12,UN,19,UN,14,UN,UN,UN, 6,UN,UN,UN,UN,UN,UN,
/*F0*/  UN,UN,UN,UN,UN,UN,UN,UN,UN, 7,UN,UN,UN,UN,UN,UN
};

static const char  hd64pg4[256] = {  /* P4 == ED */
/*--*--* 0  1  2  3  4  5  6  7  8  9  A  B  C  D  E  F */
/*--*--* -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  - */
/*00*/  12,13,UN,UN, 7,UN,UN,UN,12,13,UN,UN, 7,UN,UN,UN,
/*10*/  12,13,UN,UN, 7,UN,UN,UN,12,13,UN,UN, 7,UN,UN,UN,
/*20*/  12,13,UN,UN, 7,UN,UN,UN,12,13,UN,UN, 7,UN,UN,UN,
/*30*/  UN,UN,UN,UN,10,UN,UN,UN,12,13,UN,UN, 7,UN,UN,UN,
/*40*/   9,10,10,19, 6,12, 6, 6, 9,10,10,18,17,12,UN, 6,
/*50*/   9,10,10,19,UN,UN, 6, 6, 9,10,10,18,17,UN, 6, 6,
/*60*/   9,10,10,19, 9,UN,UN,16, 9,10,10,18,17,UN,UN,16,
/*70*/  UN,10,10,19,12,UN, 8,UN, 9,10,10,18,17,UN,UN,UN,
/*80*/  UN,UN,UN,14,UN,UN,UN,UN,UN,UN,UN,14,UN,UN,UN,UN,
/*90*/  UN,UN,UN,16,UN,UN,UN,UN,UN,UN,UN,16,UN,UN,UN,UN,
/*A0*/  12,12,12,12,UN,UN,UN,UN,12,12,12,12,UN,UN,UN,UN,
/*B0*/  14,14,14,14,UN,UN,UN,UN,14,14,14,14,UN,UN,UN,UN,
/*C0*/  UN,UN,UN,UN,UN,UN,UN,UN,UN,UN,UN,UN,UN,UN,UN,UN,
/*D0*/  UN,UN,UN,UN,UN,UN,UN,UN,UN,UN,UN,UN,UN,UN,UN,UN,
/*E0*/  UN,UN,UN,UN,UN,UN,UN,UN,UN,UN,UN,UN,UN,UN,UN,UN,
/*F0*/  UN,UN,UN,UN,UN,UN,UN,UN,UN,UN,UN,UN,UN,UN,UN,UN
};

static const char  hd64pg5[256] = {  /* P5 == FD */
/*--*--* 0  1  2  3  4  5  6  7  8  9  A  B  C  D  E  F */
/*--*--* -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  - */
/*00*/  UN,UN,UN,UN,UN,UN,UN,UN,UN,10,UN,UN,UN,UN,UN,UN,
/*10*/  UN,UN,UN,UN,UN,UN,UN,UN,UN,10,UN,UN,UN,UN,UN,UN,
/*20*/  UN,12,19, 7,UN,UN,UN,UN,UN,10,18, 7,UN,UN,UN,UN,
/*30*/  UN,UN,UN,UN,18,18,15,UN,UN,10,UN,UN,UN,UN,UN,UN,
/*40*/  UN,UN,UN,UN,UN,UN,14,UN,UN,UN,UN,UN,UN,UN,14,UN,
/*50*/  UN,UN,UN,UN,UN,UN,14,UN,UN,UN,UN,UN,UN,UN,14,UN,
/*60*/  UN,UN,UN,UN,UN,UN,14,UN,UN,UN,UN,UN,UN,UN,14,UN,
/*70*/  15,15,15,15,15,15,UN,15,UN,UN,UN,UN,UN,UN,14,UN,
/*80*/  UN,UN,UN,UN,UN,UN,14,UN,UN,UN,UN,UN,UN,UN,14,UN,
/*90*/  UN,UN,UN,UN,UN,UN,14,UN,UN,UN,UN,UN,UN,UN,14,UN,
/*A0*/  UN,UN,UN,UN,UN,UN,14,UN,UN,UN,UN,UN,UN,UN,14,UN,
/*B0*/  UN,UN,UN,UN,UN,UN,14,UN,UN,UN,UN,UN,UN,UN,14,UN,
/*C0*/  UN,UN,UN,UN,UN,UN,UN,UN,UN,UN,UN,P7,UN,UN,UN,UN,
/*D0*/  UN,UN,UN,UN,UN,UN,UN,UN,UN,UN,UN,UN,UN,UN,UN,UN,
/*E0*/  UN,12,UN,19,UN,14,UN,UN,UN, 6,UN,UN,UN,UN,UN,UN,
/*F0*/  UN,UN,UN,UN,UN,UN,UN,UN,UN, 7,UN,UN,UN,UN,UN,UN
};

static const char  hd64pg6[256] = {  /* P6 == DD CB */
/*--*--* 0  1  2  3  4  5  6  7  8  9  A  B  C  D  E  F */
/*--*--* -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  - */
/*00*/  UN,UN,UN,UN,UN,UN,19,UN,UN,UN,UN,UN,UN,UN,19,UN,
/*10*/  UN,UN,UN,UN,UN,UN,19,UN,UN,UN,UN,UN,UN,UN,19,UN,
/*20*/  UN,UN,UN,UN,UN,UN,19,UN,UN,UN,UN,UN,UN,UN,19,UN,
/*30*/  UN,UN,UN,UN,UN,UN,UN,UN,UN,UN,UN,UN,UN,UN,19,UN,
/*40*/  UN,UN,UN,UN,UN,UN,19,UN,UN,UN,UN,UN,UN,UN,19,UN,
/*50*/  UN,UN,UN,UN,UN,UN,19,UN,UN,UN,UN,UN,UN,UN,19,UN,
/*60*/  UN,UN,UN,UN,UN,UN,19,UN,UN,UN,UN,UN,UN,UN,19,UN,
/*70*/  UN,UN,UN,UN,UN,UN,19,UN,UN,UN,UN,UN,UN,UN,19,UN,
/*80*/  UN,UN,UN,UN,UN,UN,19,UN,UN,UN,UN,UN,UN,UN,19,UN,
/*90*/  UN,UN,UN,UN,UN,UN,19,UN,UN,UN,UN,UN,UN,UN,19,UN,
/*A0*/  UN,UN,UN,UN,UN,UN,19,UN,UN,UN,UN,UN,UN,UN,19,UN,
/*B0*/  UN,UN,UN,UN,UN,UN,19,UN,UN,UN,UN,UN,UN,UN,19,UN,
/*C0*/  UN,UN,UN,UN,UN,UN,19,UN,UN,UN,UN,UN,UN,UN,19,UN,
/*D0*/  UN,UN,UN,UN,UN,UN,19,UN,UN,UN,UN,UN,UN,UN,19,UN,
/*E0*/  UN,UN,UN,UN,UN,UN,19,UN,UN,UN,UN,UN,UN,UN,19,UN,
/*F0*/  UN,UN,UN,UN,UN,UN,19,UN,UN,UN,UN,UN,UN,UN,19,UN
};

static const char  hd64pg7[256] = {  /* P7 == FD CB */
/*--*--* 0  1  2  3  4  5  6  7  8  9  A  B  C  D  E  F */
/*--*--* -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  - */
/*00*/  UN,UN,UN,UN,UN,UN,19,UN,UN,UN,UN,UN,UN,UN,19,UN,
/*10*/  UN,UN,UN,UN,UN,UN,19,UN,UN,UN,UN,UN,UN,UN,19,UN,
/*20*/  UN,UN,UN,UN,UN,UN,19,UN,UN,UN,UN,UN,UN,UN,19,UN,
/*30*/  UN,UN,UN,UN,UN,UN,UN,UN,UN,UN,UN,UN,UN,UN,19,UN,
/*40*/  UN,UN,UN,UN,UN,UN,19,UN,UN,UN,UN,UN,UN,UN,19,UN,
/*50*/  UN,UN,UN,UN,UN,UN,19,UN,UN,UN,UN,UN,UN,UN,19,UN,
/*60*/  UN,UN,UN,UN,UN,UN,19,UN,UN,UN,UN,UN,UN,UN,19,UN,
/*70*/  UN,UN,UN,UN,UN,UN,19,UN,UN,UN,UN,UN,UN,UN,19,UN,
/*80*/  UN,UN,UN,UN,UN,UN,19,UN,UN,UN,UN,UN,UN,UN,19,UN,
/*90*/  UN,UN,UN,UN,UN,UN,19,UN,UN,UN,UN,UN,UN,UN,19,UN,
/*A0*/  UN,UN,UN,UN,UN,UN,19,UN,UN,UN,UN,UN,UN,UN,19,UN,
/*B0*/  UN,UN,UN,UN,UN,UN,19,UN,UN,UN,UN,UN,UN,UN,19,UN,
/*C0*/  UN,UN,UN,UN,UN,UN,19,UN,UN,UN,UN,UN,UN,UN,19,UN,
/*D0*/  UN,UN,UN,UN,UN,UN,19,UN,UN,UN,UN,UN,UN,UN,19,UN,
/*E0*/  UN,UN,UN,UN,UN,UN,19,UN,UN,UN,UN,UN,UN,UN,19,UN,
/*F0*/  UN,UN,UN,UN,UN,UN,19,UN,UN,UN,UN,UN,UN,UN,19,UN
};

static const char *hd64Page[7] = {
    hd64pg1, hd64pg2, hd64pg3, hd64pg4,
    hd64pg5, hd64pg6, hd64pg7
};

/*
 * Z80-ZXN Opcode Cycle Pages
 */

static const char  zxnpg1[256] = {
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

static const char  zxnpg2[256] = {  /* P2 == CB */
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

static const char  zxnpg3[256] = {  /* P3 == DD */
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

static const char  zxnpg4[256] = {  /* P4 == ED */
/*--*--* 0  1  2  3  4  5  6  7  8  9  A  B  C  D  E  F */
/*--*--* -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  - */
/*00*/  UN,UN,UN,UN,UN,UN,UN,UN,UN,UN,UN,UN,UN,UN,UN,UN,
/*10*/  UN,UN,UN,UN,UN,UN,UN,UN,UN,UN,UN,UN,UN,UN,UN,UN,
/*20*/  UN,UN,UN, 8, 8,UN, 8,11,UN,UN,UN,UN,UN,UN,UN,UN,
/*30*/   8, 4, 4, 4,12,12,12,UN,UN,UN,UN,UN,UN,UN,UN,UN,
/*40*/  12,12,15,20, 8,14, 8, 9,12,12,15,20,UN,14,UN, 9,
/*50*/  12,12,15,20,UN,UN, 8, 9,12,12,15,20,UN,UN, 8, 9,
/*60*/  12,12,15,20,UN,UN,UN,18,12,12,15,20,UN,UN,UN,18,
/*70*/  UN,UN,15,20,UN,UN,UN,UN,12,12,15,20,UN,UN,UN,UN,
/*80*/  UN,UN,UN,UN,UN,UN,UN,UN,UN,UN,22, 8,UN,UN,UN,UN,
/*90*/  16,16,12, 8, 8, 8,UN,UN,UN,UN,UN,UN,UN,UN,UN,UN,
/*A0*/  16,16,16,16,16,UN,UN,UN,16,16,16,16,16,UN,UN,UN,
/*B0*/  21,21,21,21,21,UN,21,12,21,21,21,21,21,UN,UN,UN,
/*C0*/  UN,UN,UN,UN,UN,UN,UN,UN,UN,UN,UN,UN,UN,UN,UN,UN,
/*D0*/  UN,UN,UN,UN,UN,UN,UN,UN,UN,UN,UN,UN,UN,UN,UN,UN,
/*E0*/  UN,UN,UN,UN,UN,UN,UN,UN,UN,UN,UN,UN,UN,UN,UN,UN,
/*F0*/  UN,UN,UN,UN,UN,UN,UN,UN,UN,UN,UN,UN,UN,UN,UN,UN
};

static const char  zxnpg5[256] = {  /* P5 == FD */
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

static const char  zxnpg6[256] = {  /* P6 == DD CB */
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

static const char  zxnpg7[256] = {  /* P7 == FD CB */
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

static const char *zxnPage[7] = {
    zxnpg1, zxnpg2, zxnpg3, zxnpg4,
    zxnpg5, zxnpg6, zxnpg7
};

/*
 * EZ80 Opcode Cycle Pages
 */

static const char  ez80pg1[256] = {
/*--*--* 0  1  2  3  4  5  6  7  8  9  A  B  C  D  E  F */
/*--*--* -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  - */
/*00*/   1, 3, 2, 1, 1, 1, 2, 1, 1, 1, 2, 1, 1, 1, 2, 1,
/*10*/   4, 3, 2, 1, 1, 1, 2, 1, 3, 1, 2, 1, 1, 1, 2, 1,
/*20*/   3, 3, 7, 1, 1, 1, 2, 1, 3, 1, 5, 1, 1, 1, 2, 1,
/*30*/   3, 3, 4, 1, 4, 4, 3, 1, 3, 1, 4, 1, 1, 1, 2, 1,
/*40*/  PF, 1, 1, 1, 1, 1, 2, 1, 1,PF, 1, 1, 1, 1, 2, 1,
/*50*/   1, 1,PF, 1, 1, 1, 2, 1, 1, 1, 1,PF, 1, 1, 2, 1,
/*60*/   1, 1, 1, 1, 1, 1, 2, 1, 1, 1, 1, 1, 1, 1, 2, 1,
/*70*/   2, 2, 2, 2, 2, 2, 1, 2, 1, 1, 1, 1, 1, 1, 2, 1,
/*80*/   1, 1, 1, 1, 1, 1, 2, 1, 1, 1, 1, 1, 1, 1, 2, 1,
/*90*/   1, 1, 1, 1, 1, 1, 2, 1, 1, 1, 1, 1, 1, 1, 2, 1,
/*A0*/   1, 1, 1, 1, 1, 1, 2, 1, 1, 1, 1, 1, 1, 1, 2, 1,
/*B0*/   1, 1, 1, 1, 1, 1, 2, 1, 1, 1, 1, 1, 1, 1, 2, 1,
/*C0*/   6, 3, 4, 4, 6, 3, 2, 5, 6, 5, 4,P2, 6, 5, 2, 5,
/*D0*/   6, 3, 4, 3, 6, 3, 2, 5, 6, 1, 4, 3, 6,P3, 2, 5,
/*E0*/   6, 3, 4, 5, 6, 3, 2, 5, 6, 3, 4, 1, 6,P4, 2, 5,
/*F0*/   6, 3, 4, 1, 6, 3, 2, 5, 6, 1, 4, 1, 6,P5, 2, 5
};

static const char  ez80pg2[256] = {  /* P2 == CB */
/*--*--* 0  1  2  3  4  5  6  7  8  9  A  B  C  D  E  F */
/*--*--* -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  - */
/*00*/   2, 2, 2, 2, 2, 2, 5, 2, 2, 2, 2, 2, 2, 2, 5, 2,
/*10*/   2, 2, 2, 2, 2, 2, 5, 2, 2, 2, 2, 2, 2, 2, 5, 2,
/*20*/   2, 2, 2, 2, 2, 2, 5, 2, 2, 2, 2, 2, 2, 2, 5, 2,
/*30*/  UN,UN,UN,UN,UN,UN,UN,UN, 2, 2, 2, 2, 2, 2, 5, 2,
/*40*/   2, 2, 2, 2, 2, 2, 3, 2, 2, 2, 2, 2, 2, 2, 3, 2,
/*50*/   2, 2, 2, 2, 2, 2, 3, 2, 2, 2, 2, 2, 2, 2, 3, 2,
/*60*/   2, 2, 2, 2, 2, 2, 3, 2, 2, 2, 2, 2, 2, 2, 3, 2,
/*70*/   2, 2, 2, 2, 2, 2, 3, 2, 2, 2, 2, 2, 2, 2, 3, 2,
/*80*/   2, 2, 2, 2, 2, 2, 3, 2, 2, 2, 2, 2, 2, 2, 3, 2,
/*90*/   2, 2, 2, 2, 2, 2, 3, 2, 2, 2, 2, 2, 2, 2, 3, 2,
/*A0*/   2, 2, 2, 2, 2, 2, 3, 2, 2, 2, 2, 2, 2, 2, 3, 2,
/*B0*/   2, 2, 2, 2, 2, 2, 3, 2, 2, 2, 2, 2, 2, 2, 3, 2,
/*C0*/   2, 2, 2, 2, 2, 2, 3, 2, 2, 2, 2, 2, 2, 2, 3, 2,
/*D0*/   2, 2, 2, 2, 2, 2, 3, 2, 2, 2, 2, 2, 2, 2, 3, 2,
/*E0*/   2, 2, 2, 2, 2, 2, 3, 2, 2, 2, 2, 2, 2, 2, 3, 2,
/*F0*/   2, 2, 2, 2, 2, 2, 3, 2, 2, 2, 2, 2, 2, 2, 3, 2
};

static const char  ez80pg3[256] = {  /* P3 == DD */
/*--*--* 0  1  2  3  4  5  6  7  8  9  A  B  C  D  E  F */
/*--*--* -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  - */
/*00*/  UN,UN,UN,UN,UN,UN,UN, 5,UN, 2,UN,UN,UN,UN,UN, 5,
/*10*/  UN,UN,UN,UN,UN,UN,UN, 5,UN, 2,UN,UN,UN,UN,UN, 5,
/*20*/  UN, 4, 6, 2, 2, 2, 2, 5,UN, 2, 6, 2, 2, 2, 2, 5,
/*30*/  UN, 5,UN,UN, 6, 6, 5, 5,UN, 2,UN,UN,UN,UN, 5, 5,
/*40*/  UN,UN,UN,UN, 2, 2, 4,UN,UN,UN,UN,UN, 2, 2, 4,UN,
/*50*/  UN,UN,UN,UN, 2, 2, 4,UN,UN,UN,UN,UN, 2, 2, 4,UN,
/*60*/   2, 2, 2, 2, 2, 2, 4, 2, 2, 2, 2, 2, 2, 2, 4, 2,
/*70*/   4, 4, 4, 4, 4, 4,UN, 4,UN,UN,UN,UN, 2, 2, 4,UN,
/*80*/  UN,UN,UN,UN, 2, 2, 4,UN,UN,UN,UN,UN, 2, 2, 4,UN,
/*90*/  UN,UN,UN,UN, 2, 2, 4,UN,UN,UN,UN,UN, 2, 2, 4,UN,
/*A0*/  UN,UN,UN,UN, 2, 2, 4,UN,UN,UN,UN,UN, 2, 2, 4,UN,
/*B0*/  UN,UN,UN,UN, 2, 2, 4,UN,UN,UN,UN,UN, 2, 2, 4,UN,
/*C0*/  UN,UN,UN,UN,UN,UN,UN,UN,UN,UN,UN,P6,UN,UN,UN,UN,
/*D0*/  UN,UN,UN,UN,UN,UN,UN,UN,UN,UN,UN,UN,UN,UN,UN,UN,
/*E0*/  UN, 4,UN, 6,UN, 4,UN,UN,UN, 4,UN,UN,UN,UN,UN,UN,
/*F0*/  UN,UN,UN,UN,UN,UN,UN,UN,UN, 2,UN,UN,UN,UN,UN,UN
};

static const char  ez80pg4[256] = {  /* P4 == ED */
/*--*--* 0  1  2  3  4  5  6  7  8  9  A  B  C  D  E  F */
/*--*--* -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  - */
/*00*/   4, 4, 3, 3, 2,UN,UN, 4, 4, 4,UN,UN, 2,UN,UN, 4,
/*10*/   4, 4, 3, 3, 2,UN,UN, 4, 4, 4,UN,UN, 2,UN,UN, 4,
/*20*/   4, 4, 3, 3, 2,UN,UN, 4, 4, 4,UN,UN, 2,UN,UN, 4,
/*30*/  UN, 4, 3, 3, 3,UN,UN, 4, 4, 4,UN,UN, 2,UN, 4, 4,
/*40*/   3, 3, 2, 6, 2, 6, 2, 2, 3, 3, 2, 6, 6, 6,UN, 2,
/*50*/   3, 3, 2, 6, 3, 3, 2, 2, 3, 3, 2, 6, 6,UN, 2, 2,
/*60*/   3, 3, 2,UN, 3, 5, 5, 5, 3, 3, 2,UN, 6, 2, 2, 5,
/*70*/  UN,UN, 2, 6, 4,UN, 2,UN, 3, 3, 2, 5, 6, 2, 2,UN,
/*80*/  UN,UN, 5, 5, 5,UN,UN,UN,UN,UN, 5, 5, 5,UN,UN,UN,
/*90*/  UN,UN, 2, 2, 2,UN,UN,UN,UN,UN, 2, 2, 2,UN,UN,UN,
/*A0*/   5, 3, 5, 5, 5,UN,UN,UN, 5, 3, 5, 5, 5,UN,UN,UN,
/*B0*/   2, 1, 2, 2, 2,UN,UN,UN, 2, 1, 2, 2, 2,UN,UN,UN,
/*C0*/  UN,UN, 2, 3,UN,UN,UN, 2,UN,UN, 2, 2,UN,UN,UN,UN,
/*D0*/  UN,UN,UN,UN,UN,UN,UN, 2,UN,UN,UN,UN,UN,UN,UN,UN,
/*E0*/  UN,UN,UN,UN,UN,UN,UN,UN,UN,UN,UN,UN,UN,UN,UN,UN,
/*F0*/  UN,UN,UN,UN,UN,UN,UN,UN,UN,UN,UN,UN,UN,UN,UN,UN
};

static const char  ez80pg5[256] = {  /* P5 == FD */
/*--*--* 0  1  2  3  4  5  6  7  8  9  A  B  C  D  E  F */
/*--*--* -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  - */
/*00*/  UN,UN,UN,UN,UN,UN,UN, 5,UN, 2,UN,UN,UN,UN,UN, 5,
/*10*/  UN,UN,UN,UN,UN,UN,UN, 5,UN, 2,UN,UN,UN,UN,UN, 5,
/*20*/  UN, 4, 6, 2, 2, 2, 2, 5,UN, 2, 6, 2, 2, 2, 2, 5,
/*30*/  UN, 5,UN,UN, 6, 6, 5, 5,UN, 2,UN,UN,UN,UN, 5, 5,
/*40*/  UN,UN,UN,UN, 2, 2, 4,UN,UN,UN,UN,UN, 2, 2, 4,UN,
/*50*/  UN,UN,UN,UN, 2, 2, 4,UN,UN,UN,UN,UN, 2, 2, 4,UN,
/*60*/   2, 2, 2, 2, 2, 2, 4, 2, 2, 2, 2, 2, 2, 2, 4, 2,
/*70*/   4, 4, 4, 4, 4, 4,UN, 4,UN,UN,UN,UN, 2, 2, 4,UN,
/*80*/  UN,UN,UN,UN, 2, 2, 4,UN,UN,UN,UN,UN, 2, 2, 4,UN,
/*90*/  UN,UN,UN,UN, 2, 2, 4,UN,UN,UN,UN,UN, 2, 2, 4,UN,
/*A0*/  UN,UN,UN,UN, 2, 2, 4,UN,UN,UN,UN,UN, 2, 2, 4,UN,
/*B0*/  UN,UN,UN,UN, 2, 2, 4,UN,UN,UN,UN,UN, 2, 2, 4,UN,
/*C0*/  UN,UN,UN,UN,UN,UN,UN,UN,UN,UN,UN,P7,UN,UN,UN,UN,
/*D0*/  UN,UN,UN,UN,UN,UN,UN,UN,UN,UN,UN,UN,UN,UN,UN,UN,
/*E0*/  UN, 4,UN, 6,UN, 4,UN,UN,UN, 4,UN,UN,UN,UN,UN,UN,
/*F0*/  UN,UN,UN,UN,UN,UN,UN,UN,UN, 2,UN,UN,UN,UN,UN,UN
};

static const char  ez80pg6[256] = {  /* P6 == DD CB */
/*--*--* 0  1  2  3  4  5  6  7  8  9  A  B  C  D  E  F */
/*--*--* -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  - */
/*00*/  UN,UN,UN,UN,UN,UN, 7,UN,UN,UN,UN,UN,UN,UN, 7,UN,
/*10*/  UN,UN,UN,UN,UN,UN, 7,UN,UN,UN,UN,UN,UN,UN, 7,UN,
/*20*/  UN,UN,UN,UN,UN,UN, 7,UN,UN,UN,UN,UN,UN,UN, 7,UN,
/*30*/  UN,UN,UN,UN,UN,UN,UN,UN,UN,UN,UN,UN,UN,UN, 7,UN,
/*40*/  UN,UN,UN,UN,UN,UN, 5,UN,UN,UN,UN,UN,UN,UN, 5,UN,
/*50*/  UN,UN,UN,UN,UN,UN, 5,UN,UN,UN,UN,UN,UN,UN, 5,UN,
/*60*/  UN,UN,UN,UN,UN,UN, 5,UN,UN,UN,UN,UN,UN,UN, 5,UN,
/*70*/  UN,UN,UN,UN,UN,UN, 5,UN,UN,UN,UN,UN,UN,UN, 5,UN,
/*80*/  UN,UN,UN,UN,UN,UN, 5,UN,UN,UN,UN,UN,UN,UN, 5,UN,
/*90*/  UN,UN,UN,UN,UN,UN, 5,UN,UN,UN,UN,UN,UN,UN, 5,UN,
/*A0*/  UN,UN,UN,UN,UN,UN, 5,UN,UN,UN,UN,UN,UN,UN, 5,UN,
/*B0*/  UN,UN,UN,UN,UN,UN, 5,UN,UN,UN,UN,UN,UN,UN, 5,UN,
/*C0*/  UN,UN,UN,UN,UN,UN, 5,UN,UN,UN,UN,UN,UN,UN, 5,UN,
/*D0*/  UN,UN,UN,UN,UN,UN, 5,UN,UN,UN,UN,UN,UN,UN, 5,UN,
/*E0*/  UN,UN,UN,UN,UN,UN, 5,UN,UN,UN,UN,UN,UN,UN, 5,UN,
/*F0*/  UN,UN,UN,UN,UN,UN, 5,UN,UN,UN,UN,UN,UN,UN, 5,UN
};

static const char  ez80pg7[256] = {  /* P7 == FD CB */
/*--*--* 0  1  2  3  4  5  6  7  8  9  A  B  C  D  E  F */
/*--*--* -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  - */
/*00*/  UN,UN,UN,UN,UN,UN, 7,UN,UN,UN,UN,UN,UN,UN, 7,UN,
/*10*/  UN,UN,UN,UN,UN,UN, 7,UN,UN,UN,UN,UN,UN,UN, 7,UN,
/*20*/  UN,UN,UN,UN,UN,UN, 7,UN,UN,UN,UN,UN,UN,UN, 7,UN,
/*30*/  UN,UN,UN,UN,UN,UN,UN,UN,UN,UN,UN,UN,UN,UN, 7,UN,
/*40*/  UN,UN,UN,UN,UN,UN, 5,UN,UN,UN,UN,UN,UN,UN, 5,UN,
/*50*/  UN,UN,UN,UN,UN,UN, 5,UN,UN,UN,UN,UN,UN,UN, 5,UN,
/*60*/  UN,UN,UN,UN,UN,UN, 5,UN,UN,UN,UN,UN,UN,UN, 5,UN,
/*70*/  UN,UN,UN,UN,UN,UN, 5,UN,UN,UN,UN,UN,UN,UN, 5,UN,
/*80*/  UN,UN,UN,UN,UN,UN, 5,UN,UN,UN,UN,UN,UN,UN, 5,UN,
/*90*/  UN,UN,UN,UN,UN,UN, 5,UN,UN,UN,UN,UN,UN,UN, 5,UN,
/*A0*/  UN,UN,UN,UN,UN,UN, 5,UN,UN,UN,UN,UN,UN,UN, 5,UN,
/*B0*/  UN,UN,UN,UN,UN,UN, 5,UN,UN,UN,UN,UN,UN,UN, 5,UN,
/*C0*/  UN,UN,UN,UN,UN,UN, 5,UN,UN,UN,UN,UN,UN,UN, 5,UN,
/*D0*/  UN,UN,UN,UN,UN,UN, 5,UN,UN,UN,UN,UN,UN,UN, 5,UN,
/*E0*/  UN,UN,UN,UN,UN,UN, 5,UN,UN,UN,UN,UN,UN,UN, 5,UN,
/*F0*/  UN,UN,UN,UN,UN,UN, 5,UN,UN,UN,UN,UN,UN,UN, 5,UN
};

static const char *ez80Page[7] = {
    ez80pg1, ez80pg2, ez80pg3, ez80pg4,
    ez80pg5, ez80pg6, ez80pg7
};

/*
 * Process a machine op.
 */
VOID
machine(mp)
struct mne *mp;
{
        int op, t1, t2, t3;
        struct expr e1, e2;
        int rf, sf, v1, v2;

        clrexpr(&e1);
        clrexpr(&e2);
        op = (int) mp->m_valu;
        rf = mp->m_type;
        sf = mp->m_flag;

        /* check eZ80 flags in non-eZ80 mode and block such instruction */
        if ((sf & (M_LIL | M_SIS)) != 0 && mchtyp != X_EZ80)
                rf = 0;

        switch (mchtyp) {
        case X_Z80:
                if (rf > S_CPU)
                        rf = 0;
                break;
        case X_HD64:
                if (rf > X_TSTIO)
                        rf = 0;
                break;
        case X_ZXN:
                if (rf > S_CPU && rf < X_ZXN_INH2 && rf != X_TST && rf != X_MLT)
                        rf = 0;
                break;
        case X_EZ80:
                if (rf >= X_ZXN_INH2 && rf < X_EZ_ADL) {
                        rf = 0;
                        break;
                }
                /* handle eZ80 suffix and write it as prefix byte */
                if (ez80_adl) {
                        switch (sf) {
                        case M_S:
                                sf = M_SIL;
                        case M_SIL:
                                outab (0x52);
                                break;
                        case M_IS:
                                sf = M_LIS;
                        case M_LIS:
                                outab (0x49);
                                break;
                        case M_L:
                        case M_IL:
                                sf = M_LIL;
                        case M_LIL:
                                outab (0x5B);
                                break;
                        case M_SIS:
                                outab (0x40);
                                break;
                        }
                } else {
                        switch (sf) {
                        case M_S:
                        case M_IS:
                                sf = M_SIS;
                        case M_SIS:
                                outab (0x40);
                                break;
                        case M_L:
                                sf = M_LIS;
                        case M_LIS:
                                outab (0x49);
                                break;
                        case M_IL:
                                sf = M_SIL;
                        case M_SIL:
                                outab (0x52);
                                break;
                        case M_LIL:
                                outab (0x5B);
                                break;
                        }
                }
                break;

        default:
                rf = 0;
        }

        switch (rf) {

        case S_INH1:
                outab(op);
                break;

        case S_INH2:
                outab(0xED);
                outab(op);
                break;

        case S_RET:
                if (more()) {
                        /*
                         * ret  cc
                         */
                        if ((v1 = admode(CND)) != 0) {
                                outab(op | (v1<<3));
                        } else {
                                qerr();
                        }
                } else {
                        /*
                         * ret
                         */
                        outab(0xC9);
                }
                break;

        case S_PUSH:
                /*
                 * push/pop af
                 */
                if (admode(R16X)) {
                        outab(op+0x30);
                        break;
                } else
                /*
                 * push/pop bc/de/hl/ix/iy      (not sp)
                 */
                if ((v1 = admode(R16)) != 0 && (v1 &= 0xFF) != SP) {
                        if (v1 != gixiy(v1)) {
                                outab(op+0x20);
                                break;
                        }
                        outab(op | (v1<<4));
                        break;
                }
                if (mchtyp == X_ZXN && op == 0xC5 && (t1 = addr(&e1)) == S_IMMED) {
                        // ZXN push is big-endian
                        outab(0xED);
                        outab(0x8A);
                        // ASXXXX do not check for R_MSB/R_LSB for constants!!!
                        if (e1.e_flag==0 && e1.e_base.e_ap==NULL) {
                                outab(hibyte(e1.e_addr));
                                outab(lobyte(e1.e_addr));
                        } else {
                                outrb(&e1, R_MSB);
                                outrb(&e1, R_LSB);
                        }
                        break;
                }
                if (mchtyp == X_ZXN && op == 0xC1 && (v1 = admode(RX)) != 0 && (v1 &= 0xFF) == X) {
                        outab(0xED);
                        outab(0x8B);
                        break;
                }
                aerr();
                break;

        case S_RST:
                v1 = (int) absexpr();
                if (v1 & ~0x38) {
                        aerr();
                        v1 = 0;
                }
                outab(op|v1);
                break;

        case S_IM:
                expr(&e1, 0);
                abscheck(&e1);
                if (e1.e_addr > 2) {
                        aerr();
                        e1.e_addr = 0;
                }
                outab(op);
                outab(imtab[(int) e1.e_addr]);
                break;

        case S_BIT:
                expr(&e1, 0);
                t1 = 0;
                v1 = (int) e1.e_addr;
                if (v1 > 7) {
                        ++t1;
                        v1 &= 0x07;
                }
                op |= (v1<<3);
                comma(1);
                addr(&e2);
                abscheck(&e1);
                /*
                 * bit  b,(hl)
                 * bit  b,(ix+d)
                 * bit  b,(iy+d)
                 * bit  b,r
                 */
                if (genop(0xCB, op, &e2, 0) || t1)
                        aerr();
                break;

        case S_RL_UNDOCD:
          if (!allow_undoc)
            aerr( );

        case S_RL:
                t1 = 0;
                t2 = addr(&e2);
                /*
                 * rl  (hl)
                 * rl  (ix+d)
                 * rl  (iy+d)
                 * rl  r
                 */
                if (more()) {
                        if ((t2 != S_R8) || (e2.e_addr != A))
                                ++t1;
                        comma(1);
                        clrexpr(&e2);
                        t2 = addr(&e2);
                }
                if (genop(0xCB, op, &e2, 0) || t1)
                        aerr();
                break;

        case S_AND:
        case S_SUB:
                /*
                 * op  (hl)
                 * op  (ix+d)
                 * op  (iy+d)
                 * op  r
                 * op  n        [#n]
                 * op    ixl
                 * op    ixh
                 * op    iyl
                 * op    iyh
                 */
                t1 = 0;
                t2 = addr(&e2);
                if (more()) {
                        /*
                         * op  a,(hl)
                         * op  a,(ix+d)
                         * op  a,(iy+d)
                         * op  a,r
                         * op  a,n      [a,#n]
                         * op  a,ixl
                         * op  a,ixh
                         * op  a,iyl
                         * op  a,iyh
                         */
                        if ((t2 != S_R8) || (e2.e_addr != A))
                                ++t1;
                        comma(1);
                        clrexpr(&e2);
                        t2 = addr(&e2);
                }

                if ((!t1) && allow_undoc && ((t2 == S_R8U1) || (t2 == S_R8U2))) {
                        /* undocumented instruction: and/sub  a,ixh|ixl|iyh|iyl */
                        outab( ((t2 == S_R8U1) ? 0xDD : 0xFD ) );
                        outab( op + e2.e_addr );
                        break;
                }

                /*
                 * op  (hl)
                 * op  (ix+d)
                 * op  (iy+d)
                 * op  r
                 * op  n        [#n]
                 *
                 * op  a,(hl)
                 * op  a,(ix+d)
                 * op  a,(iy+d)
                 * op  a,r
                 * op  a,n      [a,#n]
                 */
                if (genop(0, op, &e2, 1) || t1)
                        aerr();
                break;

        case S_ADD:
        case S_ADC:
        case S_SBC:
                t1 = addr(&e1);
                t2 = 0;
                if (more()) {
                        comma(1);
                        t2 = addr(&e2);
                }
                if (t2 == 0) {
                        if (allow_undoc && ((t1 == S_R8U1) || (t1 == S_R8U2))) {
                                /* undocumented instruction: add/adc/sbc  a,ixh|ixl|iyh|iyl */
                                outab( ((t1 == S_R8U1) ? 0xDD : 0xFD ) );
                                outab( op + e1.e_addr );
                                break;
                        }

                        if (genop(0, op, &e1, 1))
                                aerr();
                        break;
                }
                if ((t1 == S_R8) && (e1.e_addr == A)) {

                        if (allow_undoc && ((t2 == S_R8U1) || (t2 == S_R8U2))) {
                                /* undocumented instruction: add/adc/sbc  a,ixh|ixl|iyh|iyl */
                                outab( ((t2 == S_R8U1) ? 0xDD : 0xFD ) );
                                outab( op + e2.e_addr );
                                break;
                        }

                        if (genop(0, op, &e2, 1))
                                aerr();
                        break;
                }
                if ((t1 == S_R16) && (t2 == S_R16)) {
                        if (rf == S_ADD)
                                op = 0x09;
                        if (rf == S_ADC)
                                op = 0x4A;
                        if (rf == S_SBC)
                                op = 0x42;
                        v1 = (int) e1.e_addr;
                        v2 = (int) e2.e_addr;
                        /*
                         * op  hl,bc
                         * op  hl,de
                         * op  hl,hl
                         * op  hl,sp
                         */
                        if ((v1 == HL) && (v2 <= SP)) {
                                if (rf != S_ADD)
                                        outab(0xED);
                                outab(op | (v2<<4));
                                break;
                        }
                        if (rf != S_ADD) {
                                aerr();
                                break;
                        }
                        /*
                         * add  ix,bc
                         * add  ix,de
                         * add  ix,ix
                         * add  ix,sp
                         */
                        if ((v1 == IX) && (v2 != HL) && (v2 != IY)) {
                                if (v2 == IX)
                                        v2 = HL;
                                outab(0xDD);
                                outab(op | (v2<<4));
                                break;
                        }
                        /*
                         * add  iy,bc
                         * add  iy,de
                         * add  iy,iy
                         * add  iy,sp
                         */
                        if ((v1 == IY) && (v2 != HL) && (v2 != IX)) {
                                if (v2 == IY)
                                        v2 = HL;
                                outab(0xFD);
                                outab(op | (v2<<4));
                                break;
                        }
                }
                if (mchtyp == X_ZXN && rf == S_ADD && t1 == S_R16) {
                        if (e1.e_addr == HL && t2 == S_R8 && e2.e_addr == A) {
                                outab(0xED);
                                outab(0x31);
                                break;
                        }
                        if (e1.e_addr == DE && t2 == S_R8 && e2.e_addr == A) {
                                outab(0xED);
                                outab(0x32);
                                break;
                        }
                        if (e1.e_addr == BC && t2 == S_R8 && e2.e_addr == A) {
                                outab(0xED);
                                outab(0x33);
                                break;
                        }
                        if (e1.e_addr == HL && t2 == S_IMMED) {
                                outab(0xED);
                                outab(0x34);
                                outrw(&e2, 0);
                                break;
                        }
                        if (e1.e_addr == DE && t2 == S_IMMED) {
                                outab(0xED);
                                outab(0x35);
                                outrw(&e2, 0);
                                break;
                        }
                        if (e1.e_addr == BC && t2 == S_IMMED) {
                                outab(0xED);
                                outab(0x36);
                                outrw(&e2, 0);
                                break;
                        }
                }
                aerr();
                break;

        case S_LD:
                /*
                 * Enumerated ld instructions:
                 *
                 * ld
                 * ld.l         ld.il           ld.lil
                 * ld.s         ld.is           ld.sis
                 */
                t1 = addr(&e1);
                comma(1);
                t2 = addr(&e2);
                /*
                 * ld  r,g
                 * ld  r,n      (r,#n)
                 * ld  r,(hl)
                 * ld  r,(ix+d)
                 * ld  r,(iy+d)
                 */
                if (t1 == S_R8) {
                        /*
                         * ld  r,g
                         */
                        v1 = op | e1.e_addr<<3;
                        if (genop(0, v1, &e2, 0) == 0)
                                break;
                        /*
                         * ld  r,n      (r,#n)
                         */
                        if (t2 == S_IMMED) {
                                outab((e1.e_addr<<3) | 0x06);
                                outrb(&e2,0);
                                break;
                        }
                }

                if (allow_undoc &&
                    ((t1 == S_R8U1) || (t1 == S_R8U2)) &&
                    (t2 == S_IMMED))
                  {
                    outab( ((t1 == S_R8U1) ? 0xDD : 0xFD ) );
                    outab((e1.e_addr<<3) | 0x06);
                    outrb(&e2,0);
                    break;
                  }

                v1 = (int) e1.e_addr;
                v2 = (int) e2.e_addr;
                /*
                 * ld  be,mn    [be,#mn]
                 * ld  de,mn    [de,#mn]
                 * ld  hl,mn    [hl,#mn]
                 * ld  sp,mn    [sp,#mn]
                 * ld  ix,mn    [ix,#mn]
                 * ld  iy,mn    [iy,#mn]
                 */
                if ((t1 == S_R16) && (t2 == S_IMMED)) {
                        v1 = gixiy(v1);
                        outab(0x01|(v1<<4));
                        glilsis(sf, &e2);
                        break;
                }

                /*
                 * ld  bc,(hl)
                 * ld  de,(hl)
                 * ld  hl,(hl)
                 * ld  ix,(hl)
                 * ld  iy,(hl)
                 */
                if (mchtyp == X_EZ80 && (t1 == S_R16) && (t2 == S_IDHL)) {
                        if (v1 == SP) {
                                aerr();
                                break;
                        }
                        outab(0xED);
                        if (v1 == IX) {
                                outab(0x37);
                        } else
                        if (v1 == IY) {
                                outab(0x31);
                        } else {
                                outab(7 + (v1 << 4));
                        }
                        break;
                }

                /*
                 * ld  (hl),bc
                 * ld  (hl),de
                 * ld  (hl),hl
                 * ld  (hl),ix
                 * ld  (hl),iy
                 */
                if (mchtyp == X_EZ80 && (t2 == S_R16) && (t1 == S_IDHL)) {
                        if (v2 == SP) {
                                aerr();
                                break;
                        }
                        outab(0xED);
                        if (v2 == IX) {
                                outab(0x3f);
                        } else
                        if (v2 == IY) {
                                outab(0x3e);
                        } else {
                                outab(0xf + (v2 << 4));
                        }
                        break;
                }

                /*
                 * ld  bc,(ix+d)        ld  bc,(iy+d)
                 * ld  de,(ix+d)        ld  de,(iy+d)
                 * ld  hl,(ix+d)        ld  hl,(iy+d)
                 * ld  ix,(ix+d)        ld  ix,(iy+d)
                 * ld  iy,(ix+d)        ld  iy,(iy+d)
                 */
                if (mchtyp == X_EZ80 && (t1 == S_R16) && ((t2 == S_IDIX) || (t2 == S_IDIY))) {
                        if (v1 == SP) {
                                aerr();
                                break;
                        }
                        if (t2 == S_IDIX) {
                                outab(0xDD);
                                if (v1 == IX) {
                                        outab(0x37);
                                } else
                                if (v1 == IY) {
                                        outab(0x31);
                                }
                        } else {
                                outab(0xFD);
                                if (v1 == IX) {
                                        outab(0x31);
                                }else
                                if (v1 == IY) {
                                        outab(0x37);
                                }
                        }
                        if ((v1 == BC) || (v1 == DE) || (v1 == HL))
                                outab((v1 << 4) + 7);
                        outrb(&e2, R_SGND);
                        break;
                }

                /*
                 * ld  (ix+d),bc        ld  (iy+d),bc
                 * ld  (ix+d),de        ld  (iy+d),de
                 * ld  (ix+d),hl        ld  (iy+d),hl
                 * ld  (ix+d),ix        ld  (iy+d),ix
                 * ld  (ix+d),iy        ld  (iy+d),iy
                 */
                if (mchtyp == X_EZ80 && (t2 == S_R16) && ((t1 == S_IDIX) || (t1 == S_IDIY))) {
                        if (v2 == SP) {
                                aerr();
                                break;
                        }
                        if (t1 == S_IDIX) {
                                outab(0xDD);
                                if (v2 == IX) {
                                        outab(0x3F);
                                } else
                                if (v2 == IY) {
                                        outab(0x3E);
                                }
                        } else {
                                outab(0xFD);
                                if (v2 == IX) {
                                        outab(0x3E);
                                } else
                                if (v2 == IY) {
                                        outab(0x3F);
                                }
                        }
                        if ((v2 == BC) || (v2 == DE) || (v2 == HL))
                                outab((v2 << 4) + 0xf);
                        outrb(&e1, R_SGND);
                        break;
                }

                /*
                 * ld  be,(mn)  [be,(#mn)]
                 * ld  de,(mn)  [de,(#mn)]
                 * ld  hl,(mn)  [hl,(#mn)]
                 * ld  sp,(mn)  [sp,(#mn)]
                 * ld  ix,(mn)  [ix,(#mn)]
                 * ld  iy,(mn)  [iy,(#mn)]
                 */
                if ((t1 == S_R16) && (t2 == S_INDM)) {
                        if (gixiy(v1) == HL) {
                                outab(0x2A);
                        } else {
                                outab(0xED);
                                outab(0x4B | (v1<<4));
                        }
                        glilsis(sf, &e2);
                        break;
                }
                /*
                 * ld  (mn),bc  [(#mn),bc]
                 * ld  (mn),de  [(#mn),de]
                 * ld  (mn),hl  [(#mn),hl]
                 * ld  (mn),sp  [(#mn),sp]
                 * ld  (mn),ix  [(#mn),ix]
                 * ld  (mn),iy  [(#mn),iy]
                 */
                if ((t1 == S_INDM) && (t2 == S_R16)) {
                        if (gixiy(v2) == HL) {
                                outab(0x22);
                        } else {
                                outab(0xED);
                                outab(0x43 | (v2<<4));
                        }
                        glilsis(sf, &e1);
                        break;
                }
                /*
                 * ld  a,(mn)   [a,(#mn)]
                 */
                if ((t1 == S_R8) && (v1 == A) && (t2 == S_INDM)) {
                        outab(0x3A);
                        outrw(&e2, 0);
                        break;
                }
                /*
                 * ld  (mn),a   [(#mn),a]
                 */
                if ((t1 == S_INDM) && (t2 == S_R8) && (v2 == A)) {
                        outab(0x32);
                        glilsis(sf, &e1);
                        break;
                }
                /*
                 * ld  (hl),r
                 * ld  (ix+d),r
                 * ld  (iy+d),r
                 */
                if ((t2 == S_R8) && (gixiy(t1) == S_IDHL)) {
                        outab(0x70|v2);
                        if (t1 != S_IDHL)
                                outrb(&e1, 0);
                        break;
                }
                /*
                 * ld  (hl),n           [(hl),#n]
                 * ld  (ix+d),n         [(ix+d),#n]
                 * ld  (iy+d),n         [(iy+d),#n]
                 */
                if ((t2 == S_IMMED) && (gixiy(t1) == S_IDHL)) {
                        outab(0x36);
                        if (t1 != S_IDHL)
                                outrb(&e1, 0);
                        outrb(&e2, 0);
                        break;
                }
                /*
                 * ld  R,a
                 * ld  I,a
                 */
                if ((t1 == S_R8X) && (t2 == S_R8) && (v2 == A)) {
                        outab(0xED);
                        outab(v1);
                        break;
                }
                /*
                 * ld  MB,a
                 */
                if ((t1 == S_R8MB) && (t2 == S_R8) && (v2 == A)) {
                        outab(0xED);
                        outab(0x6D);
                        break;
                }
                /*
                 * ld  a,R
                 * ld  a,I
                 */
                if ((t1 == S_R8) && (v1 == A) && (t2 == S_R8X)) {
                        outab(0xED);
                        outab(v2|0x10);
                        break;
                }
                /*
                 * ld  a,R
                 * ld  a,I
                 */
                if ((t1 == S_R8) && (v1 == A) && (t2 == S_R8MB)) {
                        outab(0xED);
                        outab(0x6E);
                        break;
                }
                /*
                 * ld  sp,hl
                 * ld  sp,ix
                 * ld  sp,iy
                 */
                if ((t1 == S_R16) && (v1 == SP)) {
                        if ((t2 == S_R16) && (gixiy(v2) == HL)) {
                                outab(0xF9);
                                break;
                        }
                }
                /*
                 * ld  a,(bc)
                 * ld  a,(de)
                 */
                if ((t1 == S_R8) && (v1 == A)) {
                        if ((t2 == S_IDBC) || (t2 == S_IDDE)) {
                                outab(0x0A | ((t2-S_INDR)<<4));
                                break;
                        }
                }
                /*
                 * ld  (bc),a
                 * ld  (de),a
                 */
                if ((t2 == S_R8) && (v2 == A)) {
                        if ((t1 == S_IDBC) || (t1 == S_IDDE)) {
                                outab(0x02 | ((t1-S_INDR)<<4));
                                break;
                        }
                }

                /*
                 * ld  hl,i
                 */
                if ((t1 == S_R16) && (v1 == HL) && (t2 == S_R8X) && (v2 == I)) {
                        outab(0xED);
                        outab(0xD7);
                        break;
                }
                /*
                 * ld  i,hl
                 */
                if ((t2 == S_R16) && (v2 == HL) && (t1 == S_R8X) && (v1 == I)) {
                        outab(0xED);
                        outab(0xC7);
                        break;
                }

                /*
                 * ld  r,ixl
                 * ld  r,ixh
                 * ld  r,iyl
                 * ld  r,iyh
                 */
                if ( (t1 == S_R8) &&
                     allow_undoc &&
                     ((t2 == S_R8U1) || (t2 == S_R8U2)) )
                  {
                    outab( ((t2 == S_R8U1) ? 0xDD : 0xFD ) );
                    outab( (e1.e_addr << 3) | (0x40 + e2.e_addr) );
                    break;
                  }
                /*
                 * ld  ixl,r
                 * ld  ixh,r
                 * ld  iyl,r
                 * ld  iyh,r
                 */
                if ( allow_undoc &&
                     ((t1 == S_R8U1) || (t1 == S_R8U2)) &&
                     (t2 == S_R8) )
                  {
                    if ( (e2.e_addr == H) || (e2.e_addr == L) )
                      aerr();

                    outab( ((t1 == S_R8U1) ? 0xDD : 0xFD ) );
                    outab( (e1.e_addr << 3) | (0x40 + e2.e_addr) );
                    break;
                  }
                /*
                 * ld  ixh,ihx
                 * ld  ixh,ixl
                 * ld  ixl,ixh
                 * ld  ixl,ixl
                 * ld  iyh,iyh
                 * ld  iyh,iyl
                 * ld  iyl,iyh
                 * ld  iyl,iyl
                 */
                if ( allow_undoc &&
                     ((t1 == S_R8U1) &&  (t2 == S_R8U1)) || ((t1 == S_R8U2) &&  (t2 == S_R8U2)) )
                  {
                    outab( ((t1 == S_R8U1) ? 0xDD : 0xFD ) );
                    outab( (e1.e_addr << 3) | (0x40 + e2.e_addr) );
                    break;
                  }
                if (mchtyp == X_EZ80 && ((t1 == S_R8MB && v2 == A) || (v1 == A && t2 == S_R8MB))) {
                  outab (0xED);
                  outab ((v1 == A) ? 0x6E : 0x6D);
                  break;
                }
                aerr();
                break;

        case S_EX:
                t1 = addr(&e1);
                comma(1);
                t2 = addr(&e2);
                if (t2 == S_R16) {
                        v1 = (int) e1.e_addr;
                        v2 = (int) e2.e_addr;
                        /*
                         * ex  (sp),hl
                         * ex  (sp),ix
                         * ex  (sp),iy
                         */
                        if ((t1 == S_IDSP) && (v1 == 0)) {
                                if (gixiy(v2) == HL) {
                                        outab(op);
                                        break;
                                }
                        }
                        /*
                         * ex  de,hl
                         */
                        if (t1 == S_R16) {
                                if ((v1 == DE) && (v2 == HL)) {
                                        outab(0xEB);
                                        break;
                                }
                        }
                }
                /*
                 * ex  af,af'
                 */
                if ((t1 == S_R16X) && (t2 == S_R16X)) {
                        outab(0x08);
                        break;
                }
                aerr();
                break;

        case S_IN:
        case S_OUT:
                if (rf == S_IN) {
                        t1 = addr(&e1);
                        comma(1);
                        t2 = addr(&e2);
                } else {
                        t2 = addr(&e2);
                        comma(1);
                        t1 = addr(&e1);
                }
                v1 = (int) e1.e_addr;
                v2 = (int) e2.e_addr;
                if (t1 == S_R8) {
                        /*
                         * in   a,(n)   [in   a,(#n)]
                         * out  (n),a   [out  (#n),a]
                         */
                        if ((v1 == A) && (t2 == S_INDM)) {
                                outab(op);
                                outab(v2);
                                break;
                        }
                        /*
                         * in   r,(c)   [in   r,(bc)]
                         * out  (c),r   [out  (bc),r]
                         */
                        if (t2 == S_IDC) {
                                outab(0xED);
                                outab(((rf == S_IN) ? 0x40 : 0x41) + (v1<<3));
                                break;
                        }
                }
                aerr();
                break;

        case S_DEC:
        case S_INC:
                t1 = addr(&e1);
                v1 = (int) e1.e_addr;
                /*
                 * op  r
                 */
                if (t1 == S_R8) {
                        outab(op|(v1<<3));
                        break;
                }
                /*
                 * op  (hl)
                 */
                if (t1 == S_IDHL) {
                        outab(op|0x30);
                        break;
                }
                /*
                 * op  (ix+d)
                 * op  (iy+d)
                 */
                if (t1 != gixiy(t1)) {
                        outab(op|0x30);
                        outrb(&e1,0);
                        break;
                }
                /*
                 * op  bc
                 * op  de
                 * op  hl
                 * op  sp
                 * op  ix
                 * op  iy
                 */
                if (t1 == S_R16) {
                        v1 = gixiy(v1);
                        if (rf == S_INC) {
                                outab(0x03|(v1<<4));
                                break;
                        }
                        if (rf == S_DEC) {
                                outab(0x0B|(v1<<4));
                                break;
                        }
                }
                /*
                 * op  IXL
                 * op  IXH
                 * op  IYL
                 * op  IYH
                 */
                if ((t1 == S_R8U1)||(t1 == S_R8U2)) {
                        outab( ((t1 == S_R8U1) ? 0xDD : 0xFD ) );
                        outab(op|(v1<<3));
                        break;
                }
                aerr();
                break;

        case S_DJNZ:
        case S_JR:
                /*
                 * jr  cc,e
                 */
                if (rf == S_JR && (v1 = admode(CND)) != 0) {
                        if ((v1 &= 0xFF) <= 0x03) {
                                op += (v1+1)<<3;
                        } else {
                                aerr();
                        }
                        comma(1);
                }
                /*
                 * jr  e
                 */
                expr(&e2, 0);
                outab(op);
                if (mchpcr(&e2)) {
                        v2 = (int) (e2.e_addr - dot.s_addr - 1);
                        if (pass == 2 && ((v2 < -128) || (v2 > 127)))
                                aerr();
                        outab(v2);
                } else {
                        outrb(&e2, R_PCR);
                }
                if (e2.e_mode != S_USER)
                        rerr();
                break;

        case S_CALL:
                if ((v1 = admode(CND)) != 0) {
                        /*
                         * call  cc,n
                         */
                        op |= (v1&0xFF)<<3;
                        comma(1);
                } else {
                        /*
                         * call  n
                         */
                        op = 0xCD;
                }
                expr(&e1, 0);
                outab(op);
                glilsis(sf, &e1);
                break;

        case S_JP:
                /*
                 * jp  cc,mn
                 */
                if ((v1 = admode(CND)) != 0) {
                        op |= (v1&0xFF)<<3;
                        comma(1);
                        expr(&e1, 0);
                        outab(op);
                        glilsis(sf, &e1);
                        break;
                }
                /*
                 * jp  mn
                 */
                t1 = addr(&e1);
                if (t1 == S_USER) {
                        outab(0xC3);
                        glilsis(sf, &e1);
                        break;
                }
                /*
                 * jp  (hl)
                 * jp  (ix)
                 * jp  (iy)
                 */
                if ((e1.e_addr == 0) && (gixiy(t1) == S_IDHL)) {
                        outab(0xE9);
                        break;
                }
                /*
                 * jp  (c)
                 */
                if (mchtyp == X_ZXN && t1 == S_IDC) {
                        outab(0xED);
                        outab(0x98);
                        break;
                }
                aerr();
                break;

        case X_UNDOCD:
                ++allow_undoc;
                break;

        case S_CPU:
                opcycles = OPCY_CPU;
                mchtyp = op;
                sym[2].s_addr = op;
                lmode = SLIST;
		if (mchtyp == X_EZ80)
		  allow_undoc = 1;
                break;

        case X_INH2:
                outab(0xED);
                outab(op);
                break;

        case X_IN:
        case X_OUT:
                if (rf == X_IN) {
                        t1 = addr(&e1);
                        comma(1);
                        t2 = addr(&e2);
                } else {
                        t2 = addr(&e2);
                        comma(1);
                        t1 = addr(&e1);
                }
                if ((t1 == S_R8) && (t2 == S_INDM)) {
                        outab(0xED);
                        outab(op | (e1.e_addr<<3));
                        outrb(&e2, 0);
                        break;
                }
                aerr();
                break;

        case X_MLT:
                /*
                 * mlt  bc/de/hl/sp
                 */
                t1 = addr(&e1);
                if (mchtyp == X_ZXN && (t1 == S_R16) && (int) e1.e_addr == DE) {
                        outab(0xED);
                        outab(0x30);
                        break;
                }
                else if ((t1 == S_R16) && ((v1 = (int) e1.e_addr) <= SP)) {
                        outab(0xED);
                        outab(op | (v1<<4));
                        break;
                }
                aerr();
                break;

        case X_TST:
                /*
                 * op  (hl)
                 * op  r
                 * op  n        [#n]
                 */
                t2 = addr(&e2);
                if (more()) {
                        /*
                         * op  a,(hl)
                         * op  a,r
                         * op  a,n      [a,#n]
                         */
                        if ((t2 != S_R8) || (e2.e_addr != A))
                                aerr();
                        comma(1);
                        clrexpr(&e2);
                        t2 = addr(&e2);
                }
                /*
                 * tst  (hl)
                 */
                if ((mchtyp == X_HD64 || mchtyp == X_EZ80) && t2 == S_IDHL) {
                        outab(0xED);
                        outab(0x34);
                        break;
                }
                /*
                 * tst  r
                 */
                if ((mchtyp == X_HD64 || mchtyp == X_EZ80) && t2 == S_R8) {
                        outab(0xED);
                        outab(op | (e2.e_addr << 3));
                        break;
                }
                /*
                 * tst  n       [tst  #n]
                 */
                if (t2 == S_IMMED) {
                        outab(0xED);
                        if (mchtyp == X_HD64 || mchtyp == X_EZ80)
                                outab(0x64);
                        else if (mchtyp == X_ZXN)
                                outab(0x27);
                        else
                                aerr();
                        outrb(&e2, 0);
                        break;
                }
                aerr();
                break;

        case X_TSTIO:
                t1 = addr(&e1);
                /*
                 * tstio  n             [tstio  #n]
                 */
                if (t1 == S_IMMED) {
                        outab(0xED);
                        outab(op);
                        outrb(&e1, 0);
                        break;
                }
                aerr();
                break;

        case X_ZXN_INH2:
                switch (op) {
                case 0x23: //swap
                        if (more()) { // Optional argument a on swap
                                t1 = addr(&e1);
                                if (t1 != S_R8 || e1.e_addr != A)
                                        aerr();
                         }
                         break;
                case 0x28: // BSLA DE,B
                case 0x29: // BSRA DE,B
                case 0x2a: // BSRL DE,B
                case 0x2b: // BSRF DE,B
                case 0x2c: // BRLC DE,B
                         t1 = addr(&e1);
                         comma(1);
                         t2 = addr(&e2);
                         if (t1 != S_R16 || e1.e_addr != DE || t2 != S_R8 || e2.e_addr != B)
                                aerr();
                         break;
                }
                outab(0xED);
                outab(op);
                break;

        case X_ZXN_MUL:
                if ((t1 = addr(&e1)) == S_R8 && e1.e_addr == D &&
                        more() && comma(1) &&
                        (t2 = addr(&e2)) == S_R8 && e2.e_addr == E
                        ) {
                        outab(0xED);
                        outab(op);
                        break;
                }
                aerr();
                break;

        case X_ZXN_MIRROR:
                t1 = addr(&e1);
                if (t1 == S_R8 && e1.e_addr == A) {
                        outab(0xED);
                        outab(0x24);
                        break;
                }
                if (t1 == S_R16 && e1.e_addr == DE) {
                        outab(0xED);
                        outab(0x26);
                        break;
                }
                aerr();
                break;

        case X_ZXN_NEXTREG:
                t1 = addr(&e1);
                t2 = 0;
                if (more()) {
                        comma(1);
                        t2 = addr(&e2);
                }
                if (t1 == S_IMMED && t2 == S_IMMED) {
                        outab(0xED);
                        outab(0x91);
                        outrb(&e1, 0);
                        outrb(&e2, 0);
                        break;
                }
                if (t1 == S_IMMED && t2 == S_R8 && e2.e_addr == A) {
                        outab(0xED);
                        outab(0x92);
                        outrb(&e1, 0);
                        break;
                }
                aerr();
                break;

        case X_ZXN_CU_WAIT:
                t1 = addr(&e1);
                t2 = 0;
                if (more()) {
                        comma(1);
                        t2 = addr(&e2);
                }
                if (t1 == S_IMMED && t2 == S_IMMED) {
                        if (e1.e_addr > 311 || e2.e_addr > 55) {
                                aerr();
                                break;
                        }
                        v1 = 0x8000 + (e2.e_addr << 9) + (e1.e_addr);
                        outab(v1 >> 8);
                        outab(v1 & 0xFF);
                        opcycles = OPCY_ERR;
                        break;
                }
                aerr();
                break;

        case X_ZXN_CU_MOVE:
                t1 = addr(&e1);
                t2 = 0;
                if (more()) {
                        comma(1);
                        t2 = addr(&e2);
                }
                if (t1 == S_IMMED && t2 == S_IMMED) {
                        if (e1.e_addr > 127 || e2.e_addr > 255) {
                                aerr();
                                break;
                        }
                        v1 = (e1.e_addr << 8) + (e2.e_addr);
                        outab(v1 >> 8);
                        outab(v1 & 0xFF);
                        opcycles = OPCY_ERR;
                        break;
                }
                aerr();
                break;

        case X_ZXN_CU_STOP:
                outab(0xFF);
                outab(0xFF);
                opcycles = OPCY_ERR;
                break;

        case X_ZXN_CU_NOP:
                outab(0x00);
                outab(0x00);
                opcycles = OPCY_ERR;
                break;

        case X_EZ_ADL:
                expr(&e1,0);
                abscheck(&e1);
                ez80_adl = e1.e_addr;
                break;

        case X_EZ_INH2:
                outab(0xED);
                outab(op);
                break;

        case X_EZ_LEA:
                t1 = addr(&e1);
                v1 = (int) e1.e_addr;
                comma(1);
                t2 = addr(&e2);
                v2 = (int) e2.e_addr;
                comma(1);
                t3 = addr(&e2);
                if ((t1 == S_R16) && (v1 != SP) && (t2 == S_R16) && (v2==IX || v2==IY) && t3 == S_IMMED) {
                        t2 = e2.e_mode = S_INDR + v2;
                        outab(0xED);
                        /*
                         * op  ix,ix,#d
                         * op  ix,iy,#d
                         */
                        if (v1 == IX) {
                                if (t2 == S_IDIX) {
                                        outab(0x32);
                                } else {
                                        outab(0x54);
                                }
                        } else
                        /*
                         * op  iy,ix,#d
                         * op  iy,iy,#d
                         */
                        if (v1 == IY) {
                                if (t2 == S_IDIX) {
                                        outab(0x55);
                                } else {
                                        outab(0x33);
                                }
                        } else {
                                if (t2 == S_IDIY) {
                                        /*
                                         * op  bc,iy,#d
                                         * op  de,iy,#d
                                         * op  hl,iy,#d
                                         */
                                        outab((v1 << 4) + 3);
                                } else {
                                        /*
                                         * op  bc,ix,#d
                                         * op  de,ix,#d
                                         * op  hl,ix,#d
                                         */
                                        outab((v1 << 4) + 2);
                                }
                        }
                        outrb(&e2, R_SGND);
                        break;
                }
                aerr();
                break;

        case X_EZ_PEA:
                t1 = addr(&e1);
                v1 = (int) e1.e_addr;
                comma(1);
                t2 = addr (&e1);
                /*
                 * pea  ix,#d
                 * pea  iy,#d
                 */
                if ((t1 == S_R16) && (v1==IX || v1==IY) && t2 == S_IMMED) {
                        t1 = e1.e_mode = S_INDR + v1;
                        outab(0xED);
                        if (t1 == S_IDIY)
                                ++op;
                        outab(op);
                        outrb(&e1, R_SGND);
                        break;
                }
                aerr();
                break;


        default:
                opcycles = OPCY_ERR;
                err('o');
                break;
        }

        if (opcycles == OPCY_NONE) {
                switch (mchtyp) {
                case X_HD64:
                        opcycles = hd64pg1[cb[0] & 0xFF];
                        while ((opcycles & OPCY_NONE) && (opcycles & OPCY_MASK)) {
                                switch (opcycles) {
                                case P2:        /* CB xx        */
                                case P3:        /* DD xx        */
                                case P4:        /* ED xx        */
                                case P5:        /* FD xx        */
                                        opcycles = hd64Page[opcycles & OPCY_MASK][cb[1] & 0xFF];
                                        break;
                                case P6:        /* DD CB -- xx  */
                                case P7:        /* FD CB -- xx  */
                                        opcycles = hd64Page[opcycles & OPCY_MASK][cb[3] & 0xFF];
                                        break;
                                default:
                                        opcycles = OPCY_NONE;
                                        break;
                                }
                        }
                        break;
                case X_Z80:
                        opcycles = z80pg1[cb[0] & 0xFF];
                        while ((opcycles & OPCY_NONE) && (opcycles & OPCY_MASK)) {
                                switch (opcycles) {
                                case P2:        /* CB xx        */
                                case P3:        /* DD xx        */
                                case P4:        /* ED xx        */
                                case P5:        /* FD xx        */
                                        opcycles = z80Page[opcycles & OPCY_MASK][cb[1] & 0xFF];
                                        break;
                                case P6:        /* DD CB -- xx  */
                                case P7:        /* FD CB -- xx  */
                                        opcycles = z80Page[opcycles & OPCY_MASK][cb[3] & 0xFF];
                                        break;
                                default:
                                        opcycles = OPCY_NONE;
                                        break;
                                }
                        }
                        break;
                case X_ZXN:
                        opcycles = zxnpg1[cb[0] & 0xFF];
                        while ((opcycles & OPCY_NONE) && (opcycles & OPCY_MASK)) {
                                switch (opcycles) {
                                case P2:        /* CB xx        */
                                case P3:        /* DD xx        */
                                case P4:        /* ED xx        */
                                case P5:        /* FD xx        */
                                        opcycles = zxnPage[opcycles & OPCY_MASK][cb[1] & 0xFF];
                                        break;
                                case P6:        /* DD CB -- xx  */
                                case P7:        /* FD CB -- xx  */
                                        opcycles = zxnPage[opcycles & OPCY_MASK][cb[3] & 0xFF];
                                        break;
                                default:
                                        opcycles = OPCY_NONE;
                                        break;
                                }
                        }
                        break;
                case X_EZ80:
                        {
                                int of = 0;
                                opcycles = ez80pg1[cb[0] & 0xFF];
                                while ((opcycles & OPCY_NONE) && (opcycles & OPCY_MASK)) {
                                        switch (opcycles) {
                                        case PF:      /* 40 / 49 / 52 / 5B */
                                                of = 1;
                                                opcycles = ez80pg1[cb[1] & 0xFF];
                                                break;
                                        case P2:        /* CB xx        */
                                        case P3:        /* DD xx        */
                                        case P4:        /* ED xx        */
                                        case P5:        /* FD xx        */
                                                opcycles = ez80Page[opcycles & OPCY_MASK][cb[of + 1] & 0xFF];
                                                break;
                                        case P6:        /* DD CB -- xx  */
                                        case P7:        /* FD CB -- xx  */
                                                opcycles = ez80Page[opcycles & OPCY_MASK][cb[of + 3] & 0xFF];
                                                break;
                                        default:
                                                opcycles = OPCY_NONE;
                                                break;
                                        }
                                }
                        }
// ToDo: ASXX has 'Cycle Adjustment' here...
                        break;
                default:
                        break;
                }
        }
}

/*
 * general addressing evaluation
 * return(0) if general addressing mode output, else
 * return(esp->e_mode)
 */
int
genop(pop, op, esp, f)
int pop, op;
struct expr *esp;
int f;
{
        int t1;

        if ((t1 = esp->e_mode) == S_R8) {
                if (pop)
                        outab(pop);
                outab(op|esp->e_addr);
                return(0);
        }
        if (t1 == S_IDHL) {
                if (pop)
                        outab(pop);
                outab(op|0x06);
                return(0);
        }
        if (gixiy(t1) == S_IDHL) {
                if (pop) {
                        outab(pop);
                        outrb(esp,0);
                        outab(op|0x06);
                } else {
                        outab(op|0x06);
                        outrb(esp,0);
                }
                return(0);
        }
        if ((t1 == S_IMMED) && (f)) {
                if (pop)
                        outab(pop);
                outab(op|0x46);
                outrb(esp,0);
                return(0);
        }
        return(t1);
}

/*
 * IX and IY prebyte check
 */
int
gixiy(v)
int v;
{
        if (v == IX) {
                v = HL;
                outab(0xDD);
        } else if (v == IY) {
                v = HL;
                outab(0xFD);
        } else if (v == S_IDIX) {
                v = S_IDHL;
                outab(0xDD);
        } else if (v == S_IDIY) {
                v = S_IDHL;
                outab(0xFD);
        }
        return(v);
}

/*
 * .IL/.LIL and .IS/.SIS checks
 */
VOID
glilsis(sfx, esp)
int sfx;
struct expr *esp;
{

// Pokus napsat zapis parametru 16/24-bit 'jinak'...
        if (ez80_adl && !(sfx & M_IS)) {
                outr3b(esp, R_ADL);
        } else {
                outrw(esp, 0);
// warning "Proc je pro Z80 defaultni funkce outrwm()...?
// funkce - viz asxxsrc/asout.c
// zrejme se musim podivat do ASxx zdrojaku, jak se zpracovavaji priznaky R_ADL, R_Z80, R_PAGX1...
//              outrwm(esp, R_Z80|R_PAGX1, 0);
        }
}

/*
 * Branch/Jump PCR Mode Check
 */
int
mchpcr(esp)
struct expr *esp;
{
        if (esp->e_base.e_ap == dot.s_area) {
                return(1);
        }
        if (esp->e_flag==0 && esp->e_base.e_ap==NULL) {
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

        /*
         * Address Space
         */
        exprmasks(3);

        if (pass == 0) {
                mchtyp = X_Z80;
                sym[2].s_addr = X_Z80;
                ez80_adl = 0;
        }
}

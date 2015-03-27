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
 */

#include "asxxxx.h"
#include "z80.h"

char    *cpu    = "Zilog Z80 / Hitachi HD64180";
char    *dsft   = "asm";

char    imtab[3] = { 0x46, 0x56, 0x5E };
int     mchtyp;
int     allow_undoc;

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

/*
 * Z80 Opcode Cycle Pages
 */

static char  z80pg1[256] = {
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

static char  z80pg2[256] = {  /* P2 == CB */
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

static char  z80pg3[256] = {  /* P3 == DD */
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

static char  z80pg4[256] = {  /* P4 == ED */
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

static char  z80pg5[256] = {  /* P5 == FD */
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

static char  z80pg6[256] = {  /* P6 == DD CB */
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

static char  z80pg7[256] = {  /* P7 == FD CB */
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

static char *z80Page[7] = {
    z80pg1, z80pg2, z80pg3, z80pg4,
    z80pg5, z80pg6, z80pg7
};

/*
 * HD64180 / Z180  Opcode Cycle Pages
 */

static char  hd64pg1[256] = {
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

static char  hd64pg2[256] = {  /* P2 == CB */
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

static char  hd64pg3[256] = {  /* P3 == DD */
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

static char  hd64pg4[256] = {  /* P4 == ED */
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

static char  hd64pg5[256] = {  /* P5 == FD */
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

static char  hd64pg6[256] = {  /* P6 == DD CB */
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

static char  hd64pg7[256] = {  /* P7 == FD CB */
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

static char *hd64Page[7] = {
    hd64pg1, hd64pg2, hd64pg3, hd64pg4,
    hd64pg5, hd64pg6, hd64pg7
};

/*
 * Process a machine op.
 */
VOID
machine(mp)
struct mne *mp;
{
        int op, t1, t2;
        struct expr e1, e2;
        int rf, v1, v2;

        clrexpr(&e1);
        clrexpr(&e2);
        op = (int) mp->m_valu;
        rf = mp->m_type;
        if (!mchtyp && rf>S_CPU)
                rf = 0;
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
                        if ((v1 = admode(CND)) != 0) {
                                outab(op | (v1<<3));
                        } else {
                                qerr();
                        }
                } else {
                        outab(0xC9);
                }
                break;

        case S_PUSH:
                if (admode(R16X)) {
                        outab(op+0x30);
                        break;
                } else
                if ((v1 = admode(R16)) != 0 && (v1 &= 0xFF) != SP) {
                        if (v1 != gixiy(v1)) {
                                outab(op+0x20);
                                break;
                        }
                        outab(op | (v1<<4));
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
                if (genop(0xCB, op, &e2, 0) || t1)
                        aerr();
                break;

        case S_RL_UNDOCD:
          if (!allow_undoc)
            aerr( );
          
        case S_RL:
                t1 = 0;
                t2 = addr(&e2);
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
                t1 = 0;
                t2 = addr(&e2);
                if (more()) {
                        if ((t2 != S_R8) || (e2.e_addr != A))
                                ++t1;
                        comma(1);
                        clrexpr(&e2);
                        t2 = addr(&e2);
                }
                
                if ((!t1) && allow_undoc && ((t2 == S_R8U1) || (t2 == S_R8U2)))
                  {
                    /* undocumented instruction: and/sub  a,ixh|ixl|iyh|iyl */
                    outab( ((t2 == S_R8U1) ? 0xDD : 0xFD ) );
                    outab( op + e2.e_addr );
                    break;
                  }
                
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
                  if (allow_undoc && ((t1 == S_R8U1) || (t1 == S_R8U2)))
                    {
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
                  
                  if (allow_undoc && ((t2 == S_R8U1) || (t2 == S_R8U2)))
                    {
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
                        if ((v1 == IX) && (v2 != HL) && (v2 != IY)) {
                                if (v2 == IX)
                                        v2 = HL;
                                outab(0xDD);
                                outab(op | (v2<<4));
                                break;
                        }
                        if ((v1 == IY) && (v2 != HL) && (v2 != IX)) {
                                if (v2 == IY)
                                        v2 = HL;
                                outab(0xFD);
                                outab(op | (v2<<4));
                                break;
                        }
                }
                aerr();
                break;

        case S_LD:
                t1 = addr(&e1);
                comma(1);
                t2 = addr(&e2);
                if (t1 == S_R8) {
                        v1 = op | e1.e_addr<<3;
                        if (genop(0, v1, &e2, 0) == 0)
                                break;
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
                if ((t1 == S_R16) && (t2 == S_IMMED)) {
                        v1 = gixiy(v1);
                        outab(0x01|(v1<<4));
                        outrw(&e2, 0);
                        break;
                }
                if ((t1 == S_R16) && (t2 == S_INDM)) {
                        if (gixiy(v1) == HL) {
                                outab(0x2A);
                        } else {
                                outab(0xED);
                                outab(0x4B | (v1<<4));
                        }
                        outrw(&e2, 0);
                        break;
                }
                if ((t1 == S_INDM) && (t2 == S_R16)) {
                        if (gixiy(v2) == HL) {
                                outab(0x22);
                        } else {
                                outab(0xED);
                                outab(0x43 | (v2<<4));
                        }
                        outrw(&e1, 0);
                        break;
                }
                if ((t1 == S_R8) && (v1 == A) && (t2 == S_INDM)) {
                        outab(0x3A);
                        outrw(&e2, 0);
                        break;
                }
                if ((t1 == S_INDM) && (t2 == S_R8) && (v2 == A)) {
                        outab(0x32);
                        outrw(&e1, 0);
                        break;
                }
                if ((t2 == S_R8) && (gixiy(t1) == S_IDHL)) {
                        outab(0x70|v2);
                        if (t1 != S_IDHL)
                                outrb(&e1, 0);
                        break;
                }
                if ((t2 == S_IMMED) && (gixiy(t1) == S_IDHL)) {
                        outab(0x36);
                        if (t1 != S_IDHL)
                                outrb(&e1, 0);
                        outrb(&e2, 0);
                        break;
                }
                if ((t1 == S_R8X) && (t2 == S_R8) && (v2 == A)) {
                        outab(0xED);
                        outab(v1);
                        break;
                }
                if ((t1 == S_R8) && (v1 == A) && (t2 == S_R8X)) {
                        outab(0xED);
                        outab(v2|0x10);
                        break;
                }
                if ((t1 == S_R16) && (v1 == SP)) {
                        if ((t2 == S_R16) && (gixiy(v2) == HL)) {
                                outab(0xF9);
                                break;
                        }
                }
                if ((t1 == S_R8) && (v1 == A)) {
                        if ((t2 == S_IDBC) || (t2 == S_IDDE)) {
                                outab(0x0A | ((t2-S_INDR)<<4));
                                break;
                        }
                }
                if ((t2 == S_R8) && (v2 == A)) {
                        if ((t1 == S_IDBC) || (t1 == S_IDDE)) {
                                outab(0x02 | ((t1-S_INDR)<<4));
                                break;
                        }
                }

                if ( (t1 == S_R8) &&
                     allow_undoc &&
                     ((t2 == S_R8U1) || (t2 == S_R8U2)) )
                  {
                    outab( ((t2 == S_R8U1) ? 0xDD : 0xFD ) );
                    outab( (e1.e_addr << 3) | (0x40 + e2.e_addr) );
                    break;                    
                  }
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
                if ( allow_undoc &&
                     ((t1 == S_R8U1) &&  (t2 == S_R8U1)) || ((t1 == S_R8U2) &&  (t2 == S_R8U2)) )
                  {
                    outab( ((t1 == S_R8U1) ? 0xDD : 0xFD ) );
                    outab( (e1.e_addr << 3) | (0x40 + e2.e_addr) );
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
                        if ((t1 == S_IDSP) && (v1 == 0)) {
                                if (gixiy(v2) == HL) {
                                        outab(op);
                                        break;
                                }
                        }
                        if (t1 == S_R16) {
                                if ((v1 == DE) && (v2 == HL)) {
                                        outab(0xEB);
                                        break;
                                }
                        }
                }
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
                        if ((v1 == A) && (t2 == S_INDM)) {
                                outab(op);
                                outab(v2);
                                break;
                        }
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
                if (t1 == S_R8) {
                        outab(op|(v1<<3));
                        break;
                }
                if (t1 == S_IDHL) {
                        outab(op|0x30);
                        break;
                }
                if (t1 != gixiy(t1)) {
                        outab(op|0x30);
                        outrb(&e1,0);
                        break;
                }
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
                if ((t1 == S_R8U1)||(t1 == S_R8U2)) {
                        outab( ((t1 == S_R8U1) ? 0xDD : 0xFD ) );
                        outab(op|(v1<<3));
                        break;
                }
                aerr();
                break;

        case S_DJNZ:
        case S_JR:
                if (rf == S_JR && (v1 = admode(CND)) != 0) {
                        if ((v1 &= 0xFF) <= 0x03) {
                                op += (v1+1)<<3;
                        } else {
                                aerr();
                        }
                        comma(1);
                }
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
                        op |= (v1&0xFF)<<3;
                        comma(1);
                } else {
                        op = 0xCD;
                }
                expr(&e1, 0);
                outab(op);
                outrw(&e1, 0);
                break;

        case S_JP:
                if ((v1 = admode(CND)) != 0) {
                        op |= (v1&0xFF)<<3;
                        comma(1);
                        expr(&e1, 0);
                        outab(op);
                        outrw(&e1, 0);
                        break;
                }
                t1 = addr(&e1);
                if (t1 == S_USER) {
                        outab(0xC3);
                        outrw(&e1, 0);
                        break;
                }
                if ((e1.e_addr == 0) && (gixiy(t1) == S_IDHL)) {
                        outab(0xE9);
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
                t1 = addr(&e1);
                if ((t1 == S_R16) && ((v1 = (int) e1.e_addr) <= SP)) {
                        outab(0xED);
                        outab(op | (v1<<4));
                        break;
                }
                aerr();
                break;

        case X_TST:
                t2 = addr(&e2);
                if (more())
                  {
                    if ((t2 != S_R8) || (e2.e_addr != A))
                      aerr();
                    comma(1);
                    clrexpr(&e2);
                    t2 = addr(&e2);
                  }
                if (t2 == S_R8) {
                        outab(0xED);
                        outab(op | (e2.e_addr<<3));
                        break;
                }
                if (t2 == S_IDHL) {
                        outab(0xED);
                        outab(0x34);
                        break;
                }
                if (t2 == S_IMMED) {
                        outab(0xED);
                        outab(0x64);
                        outrb(&e2, 0);
                        break;
                }
                aerr();
                break;

        case X_TSTIO:
                t1 = addr(&e1);
                if (t1 == S_IMMED) {
                        outab(0xED);
                        outab(op);
                        outrb(&e1, 0);
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
                if (mchtyp) {
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
                } else {
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

        if (pass == 0) {
                mchtyp = X_Z80;
                sym[2].s_addr = X_Z80;
        }
}

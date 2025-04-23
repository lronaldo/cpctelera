/* r65mch.c */

/*
 *  Copyright (C) 1995-2023  Alan R. Baldwin
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
 * With Contributions from
 *
 * Marko Makela
 * Sillitie 10 A
 * 01480 Vantaa
 * Finland
 * Internet: Marko dot Makela at Helsinki dot Fi
 * EARN/BitNet: msmakela at finuh
 */

#include "asxxxx.h"
#include "r6500.h"

char *cpu  = "Rockwell 6502/6510/65C02";
char *dsft = "asm";

int r6500;
int r65f11;
int r65c00;
int r65c02;

/*
 * Opcode Cycle Definitions
 */
#define	OPCY_SDP	((char) (0xFF))
#define	OPCY_ERR	((char) (0xFE))

#define	OPCY_6500	((char) (0xFD))
#define	OPCY_65F11	((char) (0xFC))
#define	OPCY_65C00	((char) (0xFB))
#define	OPCY_65C02	((char) (0xFA))

/*	OPCY_NONE	((char) (0x80))	*/
/*	OPCY_MASK	((char) (0x7F))	*/

#define	UN	((char) (OPCY_NONE | 0x00))

/*
 * R65 Cycle Count
 *
 *	opcycles = r65pg1[opcode]
 */
static char r65pg1[256] = {
/*--*--* 0  1  2  3  4  5  6  7  8  9  A  B  C  D  E  F */
/*--*--* -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  - */
/*00*/   7, 6,UN,UN,UN, 3, 5,UN, 3, 2, 2,UN,UN, 4, 6,UN,
/*10*/   4, 6,UN,UN,UN, 4, 6,UN, 2, 5,UN,UN,UN, 5, 7,UN,
/*20*/   6, 6,UN,UN, 3, 3, 5,UN, 4, 2, 2,UN, 4, 4, 6,UN,
/*30*/   4, 6,UN,UN,UN, 4, 6,UN, 2, 5,UN,UN,UN, 5, 7,UN,
/*40*/   6, 6,UN,UN,UN, 3, 5,UN, 3, 2, 2,UN, 3, 4, 6,UN,
/*50*/   4, 6,UN,UN,UN, 4, 6,UN, 2, 5,UN,UN,UN, 5, 7,UN,
/*60*/   6, 6,UN,UN,UN, 3, 5,UN, 4, 2, 2,UN, 5, 4, 6,UN,
/*70*/   4, 6,UN,UN,UN, 4, 6,UN, 2, 5,UN,UN,UN, 5, 7,UN,
/*80*/  UN, 6,UN,UN, 3, 3, 3,UN, 2,UN, 2,UN, 4, 4, 4,UN,
/*90*/   4, 6,UN,UN, 4, 4, 4,UN, 2, 5, 2,UN,UN, 5,UN,UN,
/*A0*/   2, 6, 2,UN, 3, 3, 3,UN, 2, 2, 2,UN, 4, 4, 4,UN,
/*B0*/   4, 6,UN,UN, 4, 4, 4,UN, 2, 5, 2,UN, 5, 5, 5,UN,
/*C0*/   2, 6,UN,UN, 3, 3, 5,UN, 2, 2, 2,UN, 4, 4, 6,UN,
/*D0*/   4, 6,UN,UN,UN, 4, 6,UN, 2, 5,UN,UN,UN, 5, 7,UN,
/*E0*/   2, 6,UN,UN, 3, 3, 5,UN, 2, 2, 2,UN, 4, 4, 6,UN,
/*F0*/   4, 6,UN,UN,UN, 4, 6,UN, 2, 5,UN,UN,UN, 5, 7,UN
};

static char f11pg1[256] = {
/*--*--* 0  1  2  3  4  5  6  7  8  9  A  B  C  D  E  F */
/*--*--* -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  - */
/*00*/   7, 6,UN,UN,UN, 3, 5, 5, 3, 2, 2,UN,UN, 4, 6, 7,
/*10*/   4, 6,UN,UN,UN, 4, 6, 5, 2, 5,UN,UN,UN, 5, 7, 7,
/*20*/   6, 6,UN,UN, 3, 3, 5, 5, 4, 2, 2,UN, 4, 4, 6, 7,
/*30*/   4, 6,UN,UN,UN, 4, 6, 5, 2, 5,UN,UN,UN, 5, 7, 7,
/*40*/   6, 6,UN,UN,UN, 3, 5, 5, 3, 2, 2,UN, 3, 4, 6, 7,
/*50*/   4, 7,UN,UN,UN, 4, 6, 5, 2, 5,UN,UN,UN, 5, 7, 7,
/*60*/   6, 6,UN,UN,UN, 3, 5, 5, 4, 2, 2,UN, 5, 4, 6, 7,
/*70*/   4, 6,UN,UN,UN, 4, 6, 5, 2, 5,UN,UN,UN, 5, 7, 7,
/*80*/  UN, 6,UN,UN, 3, 3, 3, 5, 2,UN, 2,UN, 4, 4, 4, 7,
/*90*/   4, 6,UN,UN, 4, 4, 4, 5, 2, 5, 2,UN,UN, 5,UN, 7,
/*A0*/   2, 6, 2,UN, 3, 3, 3, 5, 2, 2, 2,UN, 4, 4, 4, 7,
/*B0*/   4, 6,UN,UN, 4, 4, 4, 5, 2, 5, 2,UN, 5, 5, 5, 7,
/*C0*/   2, 6,UN,UN, 3, 3, 5, 5, 2, 2, 2,UN, 4, 4, 6, 7,
/*D0*/   4, 6,UN,UN,UN, 4, 6, 5, 2, 5,UN,UN,UN, 5, 7, 7,
/*E0*/   2, 6,UN,UN, 3, 3, 5, 5, 2, 2, 2,UN, 4, 4, 6, 7,
/*F0*/   2, 6,UN,UN,UN, 4, 6, 5, 2, 5,UN,UN,UN, 5, 7, 7
};

static char c00pg1[256] = {
/*--*--* 0  1  2  3  4  5  6  7  8  9  A  B  C  D  E  F */
/*--*--* -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  - */
/*00*/   7, 6,10,UN,UN, 3, 5, 5, 3, 2, 2,UN,UN, 4, 6, 7,
/*10*/   4, 6,UN,UN,UN, 4, 6, 5, 2, 5,UN,UN,UN, 5, 7, 7,
/*20*/   6, 6,UN,UN, 3, 3, 5, 5, 4, 2, 2,UN, 4, 4, 6, 7,
/*30*/   4, 6,UN,UN,UN, 4, 6, 5, 2, 5,UN,UN,UN, 5, 7, 7,
/*40*/   6, 6,UN,UN,UN, 3, 5, 5, 3, 2, 2,UN, 3, 4, 6, 7,
/*50*/   4, 6,UN,UN,UN, 4, 6, 5, 2, 5, 3,UN,UN, 5, 7, 7,
/*60*/   6, 6,UN,UN,UN, 3, 5, 5, 4, 2, 2,UN, 5, 4, 6, 7,
/*70*/   4, 6,UN,UN,UN, 4, 6, 5, 2, 5, 4,UN,UN, 5, 7, 7,
/*80*/   4, 6,UN,UN, 3, 3, 3, 5, 2,UN, 5,UN, 4, 4, 4, 7,
/*90*/   4, 6,UN,UN, 4, 4, 4, 5, 2, 5, 2,UN,UN, 5,UN, 7,
/*A0*/   2, 6, 2,UN, 3, 3, 3, 5, 2, 2, 2,UN, 4, 4, 4, 7,
/*B0*/   4, 6,UN,UN, 4, 4, 4, 5, 2, 5, 2,UN, 5, 5, 5, 7,
/*C0*/   2, 6,UN,UN, 3, 3, 5, 5, 2, 2, 2,UN, 4, 4, 6, 7,
/*D0*/   4, 6,UN,UN,UN, 4, 6, 5, 2, 5, 3,UN,UN, 5, 7, 7,
/*E0*/   2, 6,UN,UN, 3, 3, 5, 5, 2, 2, 2,UN, 4, 4, 6, 7,
/*F0*/   4, 6,UN,UN,UN, 4, 6, 5, 2, 5, 4,UN,UN, 5, 7, 7
};

static char c02pg1[256] = {
/*--*--* 0  1  2  3  4  5  6  7  8  9  A  B  C  D  E  F */
/*--*--* -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  - */
/*00*/   7, 6,UN,UN, 5, 3, 5, 5, 3, 2, 2,UN, 6, 4, 6, 7,
/*10*/   4, 6, 5,UN, 5, 4, 6, 5, 2, 5, 2,UN, 6, 5, 7, 7,
/*20*/   6, 6,UN,UN, 3, 3, 5, 5, 4, 2, 2,UN, 4, 4, 6, 7,
/*30*/   4, 6, 5,UN, 4, 4, 6, 5, 2, 5, 2,UN, 5, 5, 7, 7,
/*40*/   6, 6,UN,UN,UN, 3, 5, 5, 3, 2, 2,UN, 3, 4, 6, 7,
/*50*/   4, 6, 5,UN,UN, 4, 6, 5, 2, 5, 3,UN,UN, 5, 7, 7,
/*60*/   6, 7,UN,UN, 3, 4, 5, 5, 4, 3, 2,UN, 6, 5, 6, 7,
/*70*/   4, 7, 6,UN, 4, 5, 6, 5, 2, 6, 4,UN, 6, 6, 7, 7,
/*80*/   4, 6,UN,UN, 3, 3, 3, 5, 2, 2, 2,UN, 4, 4, 4, 7,
/*90*/   4, 6, 5,UN, 4, 4, 4, 5, 2, 5, 2,UN, 4, 5, 5, 7,
/*A0*/   2, 6, 2,UN, 3, 3, 3, 5, 2, 2, 2,UN, 4, 4, 4, 7,
/*B0*/   4, 6, 5,UN, 4, 4, 4, 5, 2, 5, 2,UN, 5, 5, 5, 7,
/*C0*/   2, 6,UN,UN, 3, 3, 5, 5, 2, 2, 2,UN, 4, 4, 6, 7,
/*D0*/   4, 6, 5,UN,UN, 4, 6, 5, 2, 5, 3,UN,UN, 5, 7, 7,
/*E0*/   2, 7,UN,UN, 3, 4, 5, 5, 2, 3, 2,UN, 4, 5, 6, 7,
/*F0*/   4, 7, 6,UN,UN, 5, 6, 5, 2, 6, 4,UN,UN, 6, 7, 7
};

int mchtyp;
struct area *zpg;

/*
 * Process a machine op.
 */
VOID
machine(mp)
struct mne *mp;
{
	int op, t1;
	struct expr e1,e2;
	char id[NCPS];
	int c, v1, v2;

	clrexpr(&e1);
	clrexpr(&e2);
	op = (int) mp->m_valu;
	switch (mp->m_type) {

	case S_SDP:
		opcycles = OPCY_SDP;
		zpg = dot.s_area;
		if (more()) {
			expr(&e1, 0);
			if (e1.e_flag == 0 && e1.e_base.e_ap == NULL) {
				if (e1.e_addr) {
					xerr('b', "Only Page 0 Allowed.");
				}
			}
			if ((c = getnb()) == ',') {
				getid(id, -1);
				zpg = alookup(id);
				if (zpg == NULL) {
					xerr('u', "Undefined Area.");
				}
			} else {
				unget(c);
			}
		}
		outdp(zpg, &e1, 0);
		lmode = SLIST;
		break;
	case S_CPU:
		mchtyp = op;
		switch(mchtyp) {
		case X_R6500:
			opcycles = OPCY_6500;
			r65f11 = 0;
			r65c00 = 0;
			r65c02 = 0;
			break;

		case X_R65F11:
			opcycles = OPCY_65F11;
			r65f11 = 1;
			r65c00 = 0;
			r65c02 = 0;
			break;

		case X_R65C00:
			opcycles = OPCY_65C00;
			r65f11 = 1;
			r65c00 = 1;
			r65c02 = 0;
			break;

		case X_R65C02:
			opcycles = OPCY_65C02;
			r65f11 = 1;
			r65c00 = 1;
			r65c02 = 1;
			break;
		}
		break;

	case S_INH3:
		if (r65c02) {
			while(more()) getnb();
			xerr('o', "Invalid 65C02 Instruction");
			break;
		}
		
	case S_INH2:
		if (!r65c00) {
			while(more()) getnb();
			xerr('o', "Invalid 6500/65F11 Instruction");
			break;
		}
		
	case S_INH1:
		outab(op);
		break;

	case S_BRA2:
		if (!r65c00) {
			while(more()) getnb();
			xerr('o', "Invalid 6500/65F11 Instruction");
			break;
		}
		
	case S_BRA1:
		expr(&e1, 0);
		outab(op);
		if (mchpcr(&e1)) {
			v1 = (int) (e1.e_addr - dot.s_addr - 1);
			if ((v1 < -128) || (v1 > 127))
				aerr();
			outab(v1);
		} else {
			outrb(&e1, R_PCR);
		}
		if (e1.e_mode != S_USER)
			rerr();
		break;

	case S_JSR:
		t1 = addr(&e1);
		outab(op);
		outrw(&e1, 0);
		if (t1 != S_DIR && t1 != S_EXT)
			aerr();
		break;

	case S_JMP:
		t1 = addr(&e1);
		switch (t1) {
		case S_DIR:
		case S_EXT:
	                outab(op);
			outrw(&e1, 0);
			break;
		case S_IND:
			outab(op + 0x20);
			outrw(&e1, 0);
			break;
		default:
			if (r65c02) {	/* Check 65C02 Extensions */
				switch(t1) {
				case S_IPREX:
					outab(op + 0x30);
					outrw(&e1, 0);
					break;
				default:
					outab(op);
					outaw(0);
					aerr();
					break;
				}
			} else {
				outab(op);
				outaw(0);
				aerr();
			}
			break;
		}
		break;

	case S_DOP:
		t1 = addr(&e1);
		switch (t1) {
		case S_IPREX:
			outab(op + 0x01);
			outrb(&e1, R_PAG0);
			break;
		case S_DIR:
			outab(op + 0x05);
			outrb(&e1, R_PAG0);
			break;
		case S_IMMED:
			outab(op + 0x09);
			outrb(&e1, 0);
			if (op == 0x80)
				aerr();
			break;
		case S_EXT:
			outab(op + 0x0D);
			outrw(&e1, 0);
			break;
		case S_IPSTY:
			outab(op + 0x11);
			outrb(&e1, R_PAG0);
			break;
		case S_DINDX:
			outab(op + 0x15);
			outrb(&e1, R_PAG0);
			break;
		case S_DINDY:
		case S_INDY:
			outab(op + 0x19);
			outrw(&e1, 0);
			break;
		case S_INDX:
			outab(op + 0x1D);
			outrw(&e1, 0);
			break;
		default:
			if (r65c02) {	/* Check 65C02 Extensions */
				switch(t1) {
				case S_IND:
					outab(op + 0x12);
					outrb(&e1, R_PAG0);
					break;
				default:
					outab(op + 0x05);
					outab(0);
					aerr();
					break;
				}
			} else {
				outab(op + 0x05);
				outab(0);
				aerr();
			}
			break;
		}
		break;

	case S_SOP:
		t1 = addr(&e1);
		switch (t1) {
		case S_DIR:
			outab(op + 0x06);
			outrb(&e1, R_PAG0);
			break;
		case S_EXT:
			outab(op + 0x0E);
			outrw(&e1, 0);
			break;
		case S_ACC:
			if (op == 0xC0) {	/* 65C02 Extension */
				outab(0x3A);
				if (!r65c02)
					xerr('o', "Valid Only For The 65C02");
			} else
			if (op == 0xE0) {	/* 65C02 Extension */
				outab(0x1A);
				if (!r65c02)
					xerr('o', "Valid Only For The 65C02");
			} else {
				outab(op + 0x0A);
			}
			break;
		case S_DINDX:
			outab(op + 0x16);
			outrb(&e1, R_PAG0);
			break;
		case S_INDX:
			outab(op + 0x1E);
			outrw(&e1, 0);
			break;
		default:
			outab(op + 0x06);
			outab(0);
			aerr();
			break;
		}
		break;

	case S_BIT:
		t1 = addr(&e1);
		switch (t1) {
		case S_DIR:
			outab(op + 0x04);
			outrb(&e1, R_PAG0);
			break;
		case S_EXT:
			outab(op + 0x0C);
			outrw(&e1, 0);
			break;
		default:
			if (r65c02) {	/* Check 65C02 Extensions */
				switch(t1) {
				case S_DINDX:
					outab(op + 0x14);
					outrb(&e1, R_PAG0);
					break;
				case S_INDX:
					outab(op + 0x1C);
					outrw(&e1, 0);
					break;
				case S_IMMED:
					outab(0x89);
					outrb(&e1, R_USGN);
					break;
				default:
					outab(op + 0x04);
					outab(0);
					aerr();
					break;
				}
			} else {
				outab(op + 0x04);
				outab(0);
				aerr();
			}
			break;
		}
		break;

	case S_CP:
		t1 = addr(&e1);
		switch (t1) {
		case S_DIR:
			outab(op + 0x04);
			outrb(&e1, R_PAG0);
			break;
		case S_EXT:
			outab(op+0x0C);
			outrw(&e1, 0);
			break;
		case S_IMMED:
			outab(op);
			outrb(&e1, 0);
			break;
		default:
			outab(op);
			outab(0);
			aerr();
			break;
		}
		break;

	case S_LDSTX:
		t1 = addr(&e1);
		switch (t1) {
		case S_IMMED:
			outab(op + 0x02);
			outrb(&e1, 0);
			if (op == 0x80)
				aerr();
			break;
		case S_DIR:
			outab(op + 0x06);
			outrb(&e1, R_PAG0);
			break;
		case S_EXT:
			outab(op + 0x0E);
			outrw(&e1, 0);
			break;
		case S_DINDY:
			outab(op + 0x16);
			outrb(&e1, R_PAG0);
			break;
		case S_INDY:
			outab(op + 0x1E);
			outrw(&e1, 0);
			break;
		default:
			outab(op + 0x06);
			outab(0);
			aerr();
			break;
		}
		break;

	case S_LDSTY:
		t1 = addr(&e1);
		switch (t1) {
		case S_IMMED:
			outab(op);
			outrb(&e1, 0);
			if (op == 0x80)
				aerr();
			break;
		case S_DIR:
			outab(op + 0x04);
			outrb(&e1, R_PAG0);
			break;
		case S_EXT:
			outab(op + 0x0C);
			outrw(&e1, 0);
			break;
		case S_DINDX:
			outab(op + 0x14);
			outrb(&e1, R_PAG0);
			break;
		case S_INDX:
			outab(op + 0x1C);
			outrw(&e1, 0);
			break;
		default:
			outab(op + 0x04);
			outab(0);
			aerr();
			break;
		}
		break;

	case S_BB:
		if (!r65f11) {
			while(more()) getnb();
			xerr('o', "Invalid For The 6500");
			break;
		}
		if ((c = getnb()) != '*')
			unget(c);
		expr(&e1, 0);
		comma(1);
		expr(&e2, 0);
		outab(op);
		outrb(&e1, R_PAG0);
		if (mchpcr(&e2)) {
			v2 = (int) (e2.e_addr - dot.s_addr - 1);
			if ((v2 < -128) || (v2 > 127))
				aerr();
			outab(v2);
		} else {
			outrb(&e2, R_PCR);
		}
		if (e2.e_mode != S_USER)
			rerr();
		break;

	case S_MB:
		if (!r65f11) {
			while(more()) getnb();
			xerr('o', "Invalid For The 6500");
			break;
		}
		t1 = addr(&e1);
		outab(op);
		outrb(&e1, R_PAG0);
		if (t1 != S_DIR && t1 != S_EXT)
			aerr();
		break;

	case S_STZ:
		if (!r65c02) {
			while(more()) getnb();
			xerr('o', "Valid Only For The 65C02");
			break;
		}
		switch (addr(&e1)) {
		case S_DIR:
			outab(op + 0x04);
			outrb(&e1, R_PAG0);
			break;
		case S_EXT:
			outab(op + 0x3C);
			outrw(&e1, 0);
			break;
		case S_DINDX:
			outab(op + 0x14);
			outrb(&e1, R_PAG0);
			break;
		case S_INDX:
			outab(op + 0x3E);
			outrw(&e1, 0);
			break;
		default:
			outab(op + 0x04);
			outab(0);
			aerr();
			break;
	        }
		break;

	case S_TB:
		if (!r65c02) {
			while(more()) getnb();
			xerr('o', "Valid Only For The 65C02");
			break;
		}
		switch (addr(&e1)) {
		case S_DIR:
			outab(op + 0x04);
			outrb(&e1, R_PAG0);
			break;
		case S_EXT:
			outab(op+0x0C);
			outrw(&e1, 0);
			break;
		default:
			outab(op);
			outab(0);
			aerr();
			break;
		}
		break;

	default:
		opcycles = OPCY_ERR;
		err('o');
		break;
	}
	if (opcycles == OPCY_NONE) {
		/*
		 * Donot Change Selection Order
		 */
		if (r65c02) {
			opcycles = c02pg1[cb[0] & 0xFF];
		} else
		if (r65c00) {
			opcycles = c00pg1[cb[0] & 0xFF];
		} else
		if (r65f11) {
			opcycles = f11pg1[cb[0] & 0xFF];
		} else
		if (r6500) {
			opcycles = r65pg1[cb[0] & 0xFF];
		}
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
	
	/*
	 * Zero Page Area Pointer
	 */
	zpg = NULL;

	/*
	 * Default Machine
	 */
	mchtyp = X_R6500;
	r6500  = 1;
	r65f11 = 0;
	r65c00 = 0;
	r65c02 = 0;
}

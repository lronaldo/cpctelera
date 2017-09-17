/* stm8mch.c */

/*
 *  Copyright (C) 2010  Alan R. Baldwin
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
 * 
 */

#include "asxxxx.h"
#include "stm8.h"

char	*cpu	= "STMicroelectronics STM8";
char	*dsft	= "asm";

#define	NB	512

int	*bp;
int	bm;
int	bb[NB];

/*
 * Opcode Cycle Definitions
 */
#define	OPCY_SDP	((char) (0xFF))
#define	OPCY_ERR	((char) (0xFE))
#define	OPCY_SKP	((char)	(0xFD))

/*	OPCY_NONE	((char) (0x80))	*/
/*	OPCY_MASK	((char) (0x7F))	*/

#define	UN	((char) (OPCY_NONE | 0x00))
#define	P1	((char) (OPCY_NONE | 0x01))
#define	P2	((char) (OPCY_NONE | 0x02))
#define	P3	((char) (OPCY_NONE | 0x03))
#define	P4	((char) (OPCY_NONE | 0x04))

/*
 * stm8 Opcode Cycle Pages
 */

static char  stm8pg[256] = {
/*--*--* 0  1  2  3  4  5  6  7  8  9  A  B  C  D  E  F */
/*--*--* -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  - */
/*00*/   1, 1, 1, 1, 1,UN, 1, 1, 1, 1, 1,UN, 1, 1, 1, 1,
/*10*/   1, 1, 1, 2, 1, 1, 2, 2, 1, 1, 1, 1, 2, 2, 2, 2,
/*20*/   2, 1, 1, 2, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
/*30*/   1, 3, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
/*40*/   1, 1, 4, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
/*50*/   2, 1, 2, 2, 2, 1, 2, 2, 2, 2, 2, 2, 1, 2, 1, 1,
/*60*/   1, 1, 2, 1, 1, 2, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
/*70*/   1,UN,P1, 1, 1,UN, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
/*80*/  11, 4,UN, 9, 1, 2, 1, 5, 1, 2, 1,UN, 1, 5,10,10,
/*90*/  P2,P3,P4, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
/*A0*/   1, 1, 1, 2, 1, 1, 1, 1, 1, 1, 1, 1, 2, 4, 2, 1,
/*B0*/   1, 1, 1, 2, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 2, 2,
/*C0*/   1, 1, 1, 2, 1, 1, 1, 1, 1, 1, 1, 1, 2, 4, 2, 2,
/*D0*/   1, 1, 1, 2, 1, 1, 1, 1, 1, 1, 1, 1, 2, 4, 2, 2,
/*E0*/   1, 1, 1, 2, 1, 1, 1, 1, 1, 1, 1, 1, 2, 4, 2, 2,
/*F0*/   1, 1, 1, 2, 1, 1, 1, 1, 1, 1, 1, 1, 2, 4, 2, 2
};

static char  pg72[256] = {  /* P1: PreByte == 72 */
/*--*--* 0  1  2  3  4  5  6  7  8  9  A  B  C  D  E  F */
/*--*--* -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  - */
/*00*/   2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2,
/*10*/   1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
/*20*/  UN,UN,UN,UN,UN,UN,UN,UN,UN,UN,UN,UN,UN,UN,UN,UN,
/*30*/   4,UN,UN, 4, 4,UN, 4, 4, 4, 4, 4,UN, 4, 4, 4, 4,
/*40*/   1,UN,UN, 1, 1,UN, 1, 1, 1, 1, 1,UN, 1, 1, 1, 1,
/*50*/   1,UN,UN, 1, 1,UN, 1, 1, 1, 1, 1,UN, 1, 1, 1, 1,
/*60*/   4,UN,UN, 4, 4,UN, 4, 4, 4, 4, 4,UN, 4, 4, 4, 4,
/*70*/  UN,UN,UN,UN,UN,UN,UN,UN,UN,UN,UN,UN,UN,UN,UN,UN,
/*80*/  UN,UN,UN,UN,UN,UN,UN,UN,UN,UN,UN,UN,UN,UN,UN, 1,
/*90*/  UN,UN,UN,UN,UN,UN,UN,UN,UN,UN,UN,UN,UN,UN,UN,UN,
/*A0*/  UN,UN, 2,UN,UN,UN,UN,UN,UN, 2,UN,UN,UN,UN,UN,UN,
/*B0*/   2,UN, 2,UN,UN,UN,UN,UN,UN, 2,UN, 2,UN,UN,UN,UN,
/*C0*/   4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 5, 6, 5, 5,
/*D0*/   4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 5, 6, 5, 5,
/*E0*/  UN,UN,UN,UN,UN,UN,UN,UN,UN,UN,UN,UN,UN,UN,UN,UN,
/*F0*/   2,UN, 2,UN,UN,UN,UN,UN,UN, 2,UN, 2,UN,UN,UN,UN,
};

static char  pg90[256] = {  /* P2: PreByte == 90 */
/*--*--* 0  1  2  3  4  5  6  7  8  9  A  B  C  D  E  F */
/*--*--* -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  - */
/*00*/  UN, 1, 1,UN,UN,UN,UN,UN,UN,UN,UN,UN,UN,UN,UN,UN,
/*10*/   1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
/*20*/  UN,UN,UN,UN,UN,UN,UN,UN, 1, 1,UN,UN, 1, 1, 1, 1,
/*30*/  UN,UN,UN,UN,UN,UN,UN,UN,UN,UN,UN,UN,UN,UN,UN,UN,
/*40*/   1,UN, 4, 1, 1,UN, 1, 1, 1, 1, 1,UN, 1, 1, 1, 1,
/*50*/   2,UN,UN, 2, 2,UN, 2, 2, 2, 2, 2,UN, 1, 2, 1, 1,
/*60*/   1,UN, 2, 1, 1,UN, 1, 1, 1, 1, 1,UN, 1, 1, 1, 1,
/*70*/   1,UN,UN, 1, 1,UN, 1, 1, 1, 1, 1,UN, 1, 1, 1, 1,
/*80*/  UN,UN,UN,UN,UN, 2,UN,UN,UN, 2,UN,UN,UN,UN,UN,UN,
/*90*/  UN,UN,UN, 1, 1, 1, 1, 1,UN,UN,UN,UN,UN,UN, 1, 1,
/*A0*/  UN,UN,UN, 2,UN,UN,UN, 1,UN,UN,UN,UN,UN,UN, 2, 1,
/*B0*/  UN,UN,UN, 2,UN,UN,UN,UN,UN,UN,UN,UN,UN,UN, 2, 2,
/*C0*/  UN,UN,UN, 2,UN,UN,UN,UN,UN,UN,UN,UN,UN,UN, 2, 2,
/*D0*/   1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 2, 4, 2, 2,
/*E0*/   1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 2, 4, 2, 2,
/*F0*/   1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 2, 4, 2, 2
};

static char  pg91[256] = {  /* P3: PreByte == 91 */
/*--*--* 0  1  2  3  4  5  6  7  8  9  A  B  C  D  E  F */
/*--*--* -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  - */
/*00*/  UN,UN,UN,UN,UN,UN,UN,UN,UN,UN,UN,UN,UN,UN,UN,UN,
/*10*/  UN,UN,UN,UN,UN,UN,UN,UN,UN,UN,UN,UN,UN,UN,UN,UN,
/*20*/  UN,UN,UN,UN,UN,UN,UN,UN,UN,UN,UN,UN,UN,UN,UN,UN,
/*30*/  UN,UN,UN,UN,UN,UN,UN,UN,UN,UN,UN,UN,UN,UN,UN,UN,
/*40*/  UN,UN,UN,UN,UN,UN,UN,UN,UN,UN,UN,UN,UN,UN,UN,UN,
/*50*/  UN,UN,UN,UN,UN,UN,UN,UN,UN,UN,UN,UN,UN,UN,UN,UN,
/*60*/   4,UN,UN, 4, 4,UN, 4, 4, 4, 4, 4,UN, 4, 4, 4, 4,
/*70*/  UN,UN,UN,UN,UN,UN,UN,UN,UN,UN,UN,UN,UN,UN,UN,UN,
/*80*/  UN,UN,UN,UN,UN,UN,UN,UN,UN,UN,UN,UN,UN,UN,UN,UN,
/*90*/  UN,UN,UN,UN,UN,UN,UN,UN,UN,UN,UN,UN,UN,UN,UN,UN,
/*A0*/  10,UN,UN,UN,UN,UN,UN, 1,UN,UN,UN,UN,UN,UN,UN, 1,
/*B0*/  UN,UN,UN,UN,UN,UN,UN,UN,UN,UN,UN,UN,UN,UN,UN,UN,
/*C0*/  UN,UN,UN, 5,UN,UN,UN,UN,UN,UN,UN,UN,UN,UN, 5, 5,
/*D0*/   4, 4, 4, 5, 4, 5, 4, 4, 4, 4, 4, 4, 5, 6, 5, 5,
/*E0*/  UN,UN,UN,UN,UN,UN,UN,UN,UN,UN,UN,UN,UN,UN,UN,UN,
/*F0*/  UN,UN,UN,UN,UN,UN,UN,UN,UN,UN,UN,UN,UN,UN,UN,UN
};

static char  pg92[256] = {  /* P4: PreByte == 92 */
/*--*--* 0  1  2  3  4  5  6  7  8  9  A  B  C  D  E  F */
/*--*--* -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  - */
/*00*/  UN,UN,UN,UN,UN,UN,UN,UN,UN,UN,UN,UN,UN,UN,UN,UN,
/*10*/  UN,UN,UN,UN,UN,UN,UN,UN,UN,UN,UN,UN,UN,UN,UN,UN,
/*20*/  UN,UN,UN,UN,UN,UN,UN,UN,UN,UN,UN,UN,UN,UN,UN,UN,
/*30*/   4,UN,UN, 4, 4,UN, 4, 4, 4, 4, 4,UN, 4, 4, 4, 4,
/*40*/  UN,UN,UN,UN,UN,UN,UN,UN,UN,UN,UN,UN,UN,UN,UN,UN,
/*50*/  UN,UN,UN,UN,UN,UN,UN,UN,UN,UN,UN,UN,UN,UN,UN,UN,
/*60*/   4,UN,UN, 4, 4,UN, 4, 4, 4, 4, 4,UN, 4, 4, 4, 4,
/*70*/  UN,UN,UN,UN,UN,UN,UN,UN,UN,UN,UN,UN,UN,UN,UN,UN,
/*80*/  UN,UN,UN,UN,UN,UN,UN,UN,UN,UN,UN,UN,UN, 8,UN,UN,
/*90*/  UN,UN,UN,UN,UN,UN,UN,UN,UN,UN,UN,UN,UN,UN,UN,UN,
/*A0*/  UN,UN,UN,UN,UN,UN,UN, 4,UN,UN,UN,UN, 6,UN,UN, 5,
/*B0*/  UN,UN,UN,UN,UN,UN,UN,UN,UN,UN,UN,UN, 5, 4,UN,UN,
/*C0*/   4, 4, 4, 5, 4, 5, 4, 4, 4, 4, 4, 4, 5, 6, 5, 5,
/*D0*/   4, 4, 4, 5, 4, 5, 4, 4, 4, 4, 4, 4, 5, 6, 5, 5,
/*E0*/  UN,UN,UN,UN,UN,UN,UN,UN,UN,UN,UN,UN,UN,UN,UN,UN,
/*F0*/  UN,UN,UN,UN,UN,UN,UN,UN,UN,UN,UN,UN,UN,UN,UN,UN
};

static char *Page[5] = {
    stm8pg, pg72, pg90, pg91, pg92
};

/*
 * Process a machine op.
 */
VOID
machine(mp)
struct mne *mp;
{
	struct expr e1, e2, e3;
	char *p1, *p2;
	int t1, t2, t3;
	int v1, v2, v3;
	int op, rf;

	clrexpr(&e1);
	clrexpr(&e2);
	clrexpr(&e3);
	op = (int) mp->m_valu;
	rf = mp->m_type;

	switch (rf) {
	/*
	 * S_AOP:
	 *	SUB,  CP, SBC, AND,
	 *	BCP, XOR, ADC,  OR,
	 *	ADD
	 */
	case S_AOP:
		t2 = addr(&e2);
		v2 = rcode;
		comma(1);
		t1 = addr(&e1);
		v1 = rcode;
		if ((t2 == S_REG) && (v2 == SP) &&
		    (t1 == S_IMM)) {
			if (op == 0x00) {	/* SUB  SP,# */
				outab(0x52);
				outrb(&e1, R_USGN);
				break;
			} else
			if (op == 0x0B) {	/* ADD  SP,# */
				outab(0x5B);
				outrb(&e1, R_USGN);
				break;
			}
		}
		if ((t2 != S_REG) || (v2 != A)) {
			opcy_aerr();
			break;
		}
		switch(t1) {
		case S_REG:	/* A, X, XL, XH, Y, YL, YH, CC, SP */
			opcy_aerr();
			break;
		case S_LONG:	/*  arg */
			if (ls_mode(&e1)) {
				outab(op | 0xC0);
				outrw(&e1, R_USGN);
			} else {
		case S_SHORT:	/* *arg */
				outab(op | 0xB0);
				outrb(&e1, R_USGN);
			}
			break;
		case S_IMM:	/* #arg */
			outab(op | 0xA0);
			outrb(&e1, R_NORM);
			break;
		case S_IXO:	/* (offset,R), R = X, Y, SP */
			if (ls_mode(&e1)) {
		case S_IXE:	/* (offset,R).e, R = X, Y, SP */
				if (t1 == S_IXE) { aerr(); }
		case S_IXW:	/* (offset,R).w, R = X, Y, SP */
				switch(v1) {
				case Y:		outab(0x90);
				case X:		outab(op | 0xD0);
						outrw(&e1, R_USGN);	break;
				case SP:	if (t1 == S_IXW) { aerr(); }
						if (e1.e_addr & ~0xFF) { aerr(); }
						outab(op | 0x10);
						outrb(&e1, R_USGN);	break;
				default:	opcy_aerr();		break;
				}
			} else {
		case S_IXB:	/* (offset,R).b, R = X, Y, SP */
				switch(v1) {
				case Y:		outab(0x90);
				case X:		outab(op | 0xE0);
						outrb(&e1, R_USGN);	break;
				case SP:	outab(op | 0x10);
						outrb(&e1, R_USGN);	break;
				default:	opcy_aerr();		break;
				}
			}
			break;
		case S_IX:	/* (R), R = X, Y */
			switch(v1) {
			case Y:		outab(0x90);
			case X:		outab(op | 0xF0);	break;
			default:	opcy_aerr();		break;
			}
			break;
		case S_IN:	/* [offset] */
			if (ls_mode(&e1)) {
		case S_INE:	/* [offset].e */
				if (t1 == S_INE) { aerr(); }
		case S_INW:	/* [offset].w */
				outab(0x72);
				outab(op | 0xC0);
				outrw(&e1, R_USGN);
			} else {
		case S_INB:	/* [offset].b */
				outab(0x92);
				outab(op | 0xC0);
				outrb(&e1, R_USGN);
			}
			break;
		case S_INIX:	/* ([offset],R), R = X, Y */
			if (ls_mode(&e1)) {
		case S_INIXE:	/* ([offset],R).e, R = X, Y */
				if (t1 == S_INIXE) { aerr(); }
		case S_INIXW:	/* ([offset],R).w, R = X, Y */
				switch(v1) {
				case X:		outab(0x72);
						outab(op | 0xD0);
						outrw(&e1, R_USGN);	break;
				case Y:	        if (t1 == S_INIXW) { aerr(); }
						outab(0x91);
						outab(op | 0xD0);
						outrb(&e1, R_USGN);	break;
				default:	opcy_aerr();		break;
				}
			} else {
		case S_INIXB:	/* ([offset],R).b, R = X, Y */
				switch(v1) {
				case X:
				case Y:		switch(v1) {
						case X:		outab(0x92);	break;
						case Y:		outab(0x91);	break;
						default:			break;
						}
						outab(op | 0xD0);
						outrb(&e1, R_USGN);	break;
				default:	opcy_aerr();		break;
				}
			}
			break;
		default:
			opcy_aerr();
			break;
		}
		break;

	/*
	 * S_ADDW:
	 *	ADDW
	 */
	case S_ADDW:
		t2 = addr(&e2);
		v2 = rcode;
		comma(1);
		t1 = addr(&e1);
		v1 = rcode;
		if ((t2 != S_REG) || ((v2 != X) && (v2 != Y) && (v2 != SP))) {
			opcy_aerr();
			break;
		}
		switch(t1) {
		case S_LONG:	/*  arg */
		case S_SHORT:	/* *arg */
			outab(0x72);
			switch(v2) {
			case X:		outab(0xBB);	break;
			case Y:		outab(0xB9);	break;
			case SP:	opcy_aerr();	break;
			default:			break;
			}
			outrw(&e1, R_USGN);
			break;
		case S_IMM:	/* #arg */
			switch(v2) {
			case X:		outab(0x1C);	break;
			case Y:		outab(0x72);
					outab(0xA9);	break;
			case SP:	outab(0x5B);	break;
			default:			break;
			}
			if(v2 == SP)	outab(e1.e_addr); // byte
			else		outrw(&e1, R_NORM); // word
			break;
		case S_IXE:	/* (offset,R).e, R = SP */
		case S_IXW:	/* (offset,R).w, R = SP */
			aerr();
		case S_IXB:	/* (offset,R).b, R = SP */
		case S_IXO:	/* (offset,R), R = SP */
			if (v1 == SP) {
				outab(0x72);
				switch(v2) {
				case X:		outab(0xFB);	break;
				case Y:		outab(0xF9);	break;
				default:			break;
				}
				outrb(&e1, R_USGN);	break;
			} else {
				opcy_aerr();
			}
			break;
		default:
			opcy_aerr();
			break;
		}
		break;

	/*
	 * S_SUBW:
	 *	SUBW
	 */
	case S_SUBW:
		t2 = addr(&e2);
		v2 = rcode;
		comma(1);
		t1 = addr(&e1);
		v1 = rcode;
		if ((t2 != S_REG) || ((v2 != X) && (v2 != Y))) {
			opcy_aerr();
			break;
		}
		switch(t1) {
		case S_LONG:	/*  arg */
		case S_SHORT:	/* *arg */
			outab(0x72);
			switch(v2) {
			case X:		outab(0xB0);	break;
			case Y:		outab(0xB2);	break;
			default:			break;
			}
			outrw(&e1, R_USGN);
			break;
		case S_IMM:	/* #arg */
			switch(v2) {
			case X:		outab(0x1D);	break;
			case Y:		outab(0x72);
					outab(0xA2);	break;
			default:			break;
			}
			outrw(&e1, R_NORM);
			break;
		case S_IXE:	/* (offset,R).e, R = SP */
		case S_IXW:	/* (offset,R).w, R = SP */
			aerr();
		case S_IXB:	/* (offset,R).b, R = SP */
		case S_IXO:	/* (offset,R), R = SP */
			if (v1 == SP) {
				outab(0x72);
				switch(v2) {
				case X:		outab(0xF0);	break;
				case Y:		outab(0xF2);	break;
				default:			break;
				}
				outrb(&e1, R_USGN);	break;
			} else {
				opcy_aerr();
			}
			break;
		default:
			opcy_aerr();
			break;
		}
		break;

	/*
	 * S_CPW:
	 *	CPW
	 */
	case S_CPW:
		t2 = addr(&e2);
		v2 = rcode;
		comma(1);
		t1 = addr(&e1);
		v1 = rcode;
		if ((t2 != S_REG) || ((v2 != X) && (v2 != Y))) {
			opcy_aerr();
			break;
		}
		if (v2 == v1) {
			opcy_aerr();
			break;
		}
		switch(t1) {
		case S_LONG:	/*  arg */
			if (ls_mode(&e1)) {
				switch(v2) {
				case Y:		outab(0x90);
				case X:		outab(0xC3);	break;
				default:			break;
				}
				outrw(&e1, R_USGN);
			} else {
		case S_SHORT:	/* *arg */
				switch(v2) {
				case Y:		outab(0x90);
				case X:		outab(0xB3);	break;
				default:			break;
				}
				outrb(&e1, R_USGN);
			}
			break;
		case S_IMM:	/* #arg */
			switch(v2) {
			case Y:		outab(0x90);
			case X:		outab(0xA3);	break;
			default:			break;
			}
			outrw(&e1, R_NORM);
			break;
		case S_IXE:	/* (offset,R).e, R = X, Y, SP */
			aerr();
		case S_IXW:	/* (offset,R).w, R = X, Y, SP */
		case S_IXB:	/* (offset,R).b, R = X, Y, SP */
		case S_IXO:	/* (offset,R), R = X, Y, SP */
			if ((v2 == Y) && (v1 == SP)) {
				opcy_aerr();
				break;
			}
			switch(t1) {
			case S_IXO:	/* (offset,R), R = X, Y, SP */
				if (ls_mode(&e1)) {
			case S_IXE:	/* (offset,R).e, R = X, Y, SP */
					if (t1 == S_IXE) { aerr(); }
			case S_IXW:	/* (offset,R).w, R = X, Y, SP */
					switch(v1) {
					case Y:		outab(0x90);
					case X:		outab(op | 0xD0);
							outrw(&e1, R_USGN);	break;
					case SP:	if (t1 == S_IXW) { aerr(); }
							outab(op | 0x10);
							outrb(&e1, R_USGN);	break;
					default:	opcy_aerr();		break;
					}
				} else {
			case S_IXB:	/* (offset,R).b, R = X, Y, SP */
					switch(v1) {
					case Y:		outab(0x90);
					case X:		outab(op | 0xE0);
							outrb(&e1, R_USGN);	break;
					case SP:	outab(op | 0x10);
							outrb(&e1, R_USGN);	break;
					default:	opcy_aerr();		break;
					}
				}
			}
			break;
		case S_IX:	/* (R), R = X, Y */
			switch(v1) {
			case Y:		outab(0x90);
			case X:		outab(op | 0xF0);	break;
			default:	opcy_aerr();		break;
			}
			break;
		case S_IN:	/* [offset] */
			if (ls_mode(&e1)) {
		case S_INE:	/* [offset].e */
				if (t1 == S_INE) { aerr(); }
		case S_INW:	/* [offset].w */
				switch(v2) {
				case X:		outab(0x72);
						outab(0xC3);
						outrw(&e1, R_USGN);	break;
				case Y:	        if (t1 == S_INW) { aerr(); }
						outab(0x91);
						outab(0xC3);
						outrb(&e1, R_USGN);	break;
				default:				break;
				}
			} else {
		case S_INB:	/* [offset].b */
				switch(v2) {
				case X:		outab(0x92);	break;
				case Y:		outab(0x91);	break;
				default:			break;
				}
				outab(0xC3);
				outrb(&e1, R_USGN);
			}
			break;
		case S_INIX:	/* ([offset],R), R = X, Y */
			if (ls_mode(&e1)) {
		case S_INIXE:	/* ([offset],R).e, R = X, Y */
				if (t1 == S_INIXE) { aerr(); }
		case S_INIXW:	/* ([offset],R).w, R = X, Y */
				switch(v1) {
				case X:		outab(0x72);
						outab(op | 0xD0);
						outrw(&e1, R_USGN);	break;
				case Y:	        if (t1 == S_INIXW) { aerr(); }
						outab(0x91);
						outab(op | 0xD0);
						outrb(&e1, R_USGN);	break;
				default:	opcy_aerr();		break;
				}
			} else {
		case S_INIXB:	/* ([offset],R).b, R = X, Y */
				switch(v1) {
				case X:
				case Y:		switch(v1) {
						case X:		outab(0x92);	break;
						case Y:		outab(0x91);	break;
						default:			break;
						}
						outab(op | 0xD0);
						outrb(&e1, R_USGN);	break;
				default:	opcy_aerr();		break;
				}
			}
			break;
		default:
			opcy_aerr();
			break;
		}
		break;

	/*
	 * S_BOP:
	 *	NEG, CPL, SRL, RRC,
	 *	SRA, SLA, SLL, RLC,
	 *	DEC, INC, TNZ, SWAP,
	 *	CLR
	 */
	case S_BOP:
		t1 = addr(&e1);
		v1 = rcode;
		switch(t1) {
		case S_REG:	/* A, X, XL, XH, Y, YL, YH, CC, SP */
			if (v1 == A) {
				outab(op | 0x40);
			} else {
				opcy_aerr();
			}
			break;
		case S_LONG:	/*  arg */
			if (ls_mode(&e1)) {
				outab(0x72);
				outab(op | 0x50);
				outrw(&e1, R_USGN);
			} else {
		case S_SHORT:	/* *arg */
				outab(op | 0x30);
				outrb(&e1, R_USGN);
			}
			break;
		case S_IMM:	/* #arg */
			opcy_aerr();
			break;
		case S_IXO:	/* (offset,R), R = X, Y, SP */
			if (ls_mode(&e1)) {
		case S_IXE:	/* (offset,R).e, R = X, Y, SP */
				if (t1 == S_IXE) { aerr(); }
		case S_IXW:	/* (offset,R).w, R = X, Y, SP */
				switch(v1) {
				case Y:
				case X:		switch(v1) {
						case X:		outab(0x72);	break;
						case Y:		outab(0x90);	break;
						default:			break;
						}
						outab(op | 0x40);
						outrw(&e1, R_USGN);	break;
				case SP:	if (t1 == S_IXW) { aerr(); }
						outab(op | 0x00);
						outrb(&e1, R_USGN);	break;
				default:	opcy_aerr();		break;
				}
			} else {
		case S_IXB:	/* (offset,R).b, R = X, Y, SP */
				switch(v1) {
				case Y:		outab(0x90);
				case X:		outab(op | 0x60);
						outrb(&e1, R_USGN);	break;
				case SP:	outab(op | 0x00);
						outrb(&e1, R_USGN);	break;
				default:	opcy_aerr();		break;
				}
			}
			break;
		case S_IX:	/* (R), R = X, Y */
			switch(v1) {
			case Y:		outab(0x90);
			case X:		outab(op | 0x70);	break;
			default:	opcy_aerr();		break;
			}
			break;
		case S_IN:	/* [offset] */
			if (ls_mode(&e1)) {
		case S_INE:	/* [offset] */
				if (t1 == S_INE) { aerr(); }
		case S_INW:	/* [offset].w */
				outab(0x72);
				outab(op | 0x30);
				outrw(&e1, R_USGN);
			} else {
		case S_INB:	/* [offset].b */
				outab(0x92);
				outab(op | 0x30);
				outrb(&e1, R_USGN);
			}
			break;
		case S_INIX:	/* ([offset],R), R = X, Y */
			if (ls_mode(&e1)) {
		case S_INIXE:	/* ([offset],R).e, R = X, Y */
				if (t1 == S_INIXE) { aerr(); }
		case S_INIXW:	/* ([offset],R).w, R = X, Y */
				switch(v1) {
				case X:		outab(0x72);
						outab(op | 0x60);
						outrw(&e1, R_USGN);	break;
				case Y:		if (t1 == S_INIXW) { aerr(); }
						outab(0x91);
						outab(op | 0x60);
						outrb(&e1, R_USGN);	break;
				default:	opcy_aerr();		break;
				}
			} else {
		case S_INIXB:	/* ([offset],R).b, R = X, Y */
				switch(v1) {
				case X:
				case Y:		switch(v1) {
						case X:		outab(0x92);	break;
						case Y:		outab(0x91);	break;
						default:			break;
						}
						outab(op | 0x60);
						outrb(&e1, R_USGN);	break;
				default:	opcy_aerr();		break;
				}
			}
			break;
		default:
			opcy_aerr();
			break;
		}
		break;

	/*
	 * S_LD:
	 *	LD  A,---
	 *	LD  ---,A
	 */
	case S_LD:
		t2 = addr(&e2);
		v2 = rcode;
		comma(1);
		t1 = addr(&e1);
		v1 = rcode;
		/*
		 * LD  A,---
		 */
		if ((t2 == S_REG) && (v2 == A)) {
			op = 0x06;
		} else
		/*
		 * LD  ---,A
		 */
		if ((t1 == S_REG) && (v1 == A)) {
			op = 0x07;
			p1 = (char *) &e1;
			p2 = (char *) &e2;
			for (v3=0; v3<sizeof(e1); p1++,p2++,v3++) {
				 t3 = (int) *p2;
				*p2 = *p1;
				*p1 = (char) t3;
			}
			t1 = t2;
			v1 = v2;
		} else {
			opcy_aerr();
			break;
		}
		switch(t1) {
		case S_REG:	/* A, X, XL, XH, Y, YL, YH, CC, SP */
			switch(op) {
			case 0x06:	/* A,--- */
				switch(v1) {
				case YL:	outab(0x90);
				case XL:	outab(0x9F);	break;
				case YH:	outab(0x90);
				case XH:	outab(0x9E);	break;
				default:	opcy_aerr();	break;
				}
				break;
			case 0x07:	/* ---,A */
				switch(v1) {
				case YL:	outab(0x90);
				case XL:	outab(0x97);	break;
				case YH:	outab(0x90);
				case XH:	outab(0x95);	break;
				default:	opcy_aerr();	break;
				}
				break;
			default:	break;
			}
			break;
		case S_LONG:	/*  arg */
			if (ls_mode(&e1)) {
				outab(op | 0xC0);
				outrw(&e1, R_USGN);
			} else {
		case S_SHORT:	/* *arg */
				outab(op | 0xB0);
				outrb(&e1, R_USGN);
			}
			break;
		case S_IMM:	/* #arg */
			switch(op) {
			case 0x06:	outab(op | 0xA0);
					outrb(&e1, R_NORM);	break;
			case 0x07:	opcy_aerr();		break;
			default:				break;
			}
			break;
		case S_IXO:	/* (offset,R), R = X, Y, SP */
			if (ls_mode(&e1)) {
		case S_IXE:	/* (offset,R).e, R = X, Y, SP */
				if (t1 == S_IXE) { aerr(); }
		case S_IXW:	/* (offset,R).w, R = X, Y, SP */
				switch(v1) {
				case Y:		outab(0x90);
				case X:		outab(op | 0xD0);
						outrw(&e1, R_USGN);	break;
				case SP:
					switch(op) {
					case 0x06:	outab(0x7B);
							outrb(&e1, R_USGN);	break;
					case 0x07:	outab(0x6B);
							outrb(&e1, R_USGN);	break;
					default:				break;
					}
					if (t1 == S_IXW) { aerr(); }		break;
				default:	opcy_aerr();		break;
				}
			} else {
		case S_IXB:	/* (offset,R).b, R = X, Y, SP */
				switch(v1) {
				case Y:		outab(0x90);
				case X:		outab(op | 0xE0);
						outrb(&e1, R_USGN);	break;
				case SP:
					switch(op) {
					case 0x06:	outab(0x7B);
							outrb(&e1, R_USGN);	break;
					case 0x07:	outab(0x6B);
							outrb(&e1, R_USGN);	break;
					default:				break;
					}
					break;
				default:	opcy_aerr();		break;
				}
			}
			break;
		case S_IX:	/* (R), R = X, Y */
			switch(v1) {
			case Y:		outab(0x90);
			case X:		outab(op | 0xF0);	break;
			default:	opcy_aerr();		break;
			}
			break;
		case S_IN:	/* [offset] */
			if (ls_mode(&e1)) {
		case S_INE:	/* [offset].e */
				if (t1 == S_INE) { aerr(); }
		case S_INW:	/* [offset].w */
				outab(0x72);
				outab(op | 0xC0);
				outrw(&e1, R_USGN);
			} else {
		case S_INB:	/* [offset].b */
				outab(0x92);
				outab(op | 0xC0);
				outrb(&e1, R_USGN);
			}
			break;
		case S_INIX:	/* ([offset],R), R = X, Y */
			if (ls_mode(&e1)) {
		case S_INIXE:	/* ([offset],R).e, R = X, Y */
				if (t1 == S_INIXE) { aerr(); }
		case S_INIXW:	/* ([offset],R).w, R = X, Y */
				switch(v1) {
				case X:		outab(0x72);
						outab(op | 0xD0);
						outrw(&e1, R_USGN);	break;
				case Y:		if (t1 == S_INIXW) { aerr(); }
						outab(0x91);
						outab(op | 0xD0);
						outrb(&e1, R_USGN);	break;
				default:	opcy_aerr();		break;
				}
			} else {
		case S_INIXB:	/* ([offset],R).b, R = X, Y */
				switch(v1) {
				case X:
				case Y:		switch(v1) {
						case X:		outab(0x92);	break;
						case Y:		outab(0x91);	break;
						default:			break;
						}
						outab(op | 0xD0);
						outrb(&e1, R_USGN);	break;
				default:	opcy_aerr();			break;
				}
			}
			break;
		default:
			opcy_aerr();
			break;
		}
		break;

	/*
	 * S_LDF:
	 *	LDF  A,---
	 *	LDF  ---,A
	 */
	case S_LDF:
		t2 = addr(&e2);
		v2 = rcode;
		comma(1);
		t1 = addr(&e1);
		v1 = rcode;
		/*
		 * LD  A,---
		 */
		if ((t2 == S_REG) && (v2 == A)) {
			op = 0x0C;
		} else
		/*
		 * LD  ---,A
		 */
		if ((t1 == S_REG) && (v1 == A)) {
			op = 0x0D;
			p1 = (char *) &e1;
			p2 = (char *) &e2;
			for (v3=0; v3<sizeof(e1); p1++,p2++,v3++) {
				 t3 = (int) *p2;
				*p2 = *p1;
				*p1 = (char) t3;
			}
			v1 = v2;
			t1 = t2;
		} else {
			opcy_aerr();
			break;
		}
		switch(t1) {
		case S_LONG:	/*  arg */
		case S_SHORT:	/* *arg */
			outab(op | 0xB0);
			outr3b(&e1, R_NORM);
			break;
		case S_IXB:	/* (offset,R).b, R = X, Y */
		case S_IXW:	/* (offset,R).w, R = X, Y */
			aerr();
		case S_IXE:	/* (offset,R).e, R = X, Y */
		case S_IXO:	/* (offset,R), R = X, Y */
			switch(op) {
			case 0x0C:	op = 0xAF;	break;
			case 0x0D:	op = 0xA7;	break;
			default:			break;
			}
			switch(v1) {
			case Y:		outab(0x90);
			case X:		outab(op);
					outr3b(&e1, R_NORM);	break;
			default:	opcy_aerr();		break;
			}
			break;
		case S_INB:	/* [offset].b */
		case S_INW:	/* [offset].w */
			aerr();
		case S_INE:	/* [offset].e */
		case S_IN:	/* [offset] */
			outab(0x92);
			outab(op | 0xB0);
			outrw(&e1, R_USGN);
			break;
		case S_INIXB:	/* ([offset],R).b, R = X, Y */
		case S_INIXW:	/* ([offset],R).w, R = X, Y */
			aerr();
		case S_INIXE:	/* ([offset],R).e, R = X, Y */
		case S_INIX:	/* ([offset],R), R = X, Y */
			switch(op) {
			case 0x0C:	op = 0xAF;	break;
			case 0x0D:	op = 0xA7;	break;
			default:			break;
			}
			switch(v1) {
			case X:		outab(0x92);
					outab(op);
					outrw(&e1, R_USGN);	break;
			case Y:		outab(0x91);
					outab(op);
					outrw(&e1, R_USGN);	break;
			default:	opcy_aerr();		break;
			}
			break;
		default:
			opcy_aerr();
			break;
		}
		break;

	/*
	 * S_LDW:
	 *	LDW
	 */
	case S_LDW:
		t1 = addr(&e1);
		v1 = rcode;
		comma(1);
		t2 = addr(&e2);
		v2 = rcode;
		if ((t1 == S_REG) && (t2 == S_REG)) {
			switch(v1) {	/* x,--- or y,--- or sp,--- */
			case X:
				switch(v2) {	/* ---,x or ---,y or ---,sp */
				case Y:		outab(0x93);	break;
				case SP:	outab(0x96);	break;
				default:	opcy_aerr();	break;
				}
				break;
			case Y:
				outab(0x90);
				switch(v2) {	/* ---,x or ---,y or ---,sp */
				case X:		outab(0x93);	break;
				case SP:	outab(0x96);	break;
				default:	opcy_aerr();	break;
				}
				break;
			case SP:
				switch(v2) {	/* ---,x or ---,y or ---,sp */
				case Y:		outab(0x90);
				case X:		outab(0x94);	break;
				default:	opcy_aerr();	break;
				}
				break;
			default:	opcy_aerr();	break;
			}
			break;
		}
		if ((t1 == S_REG) && (v1 == X)) {
			switch(t2) {
			case S_LONG:	/*  arg */
				if (ls_mode(&e2)) {
					outab(0xCE);
					outrw(&e2, R_USGN);	break;
				} else {
			case S_SHORT:	/* *arg */
					outab(0xBE);
					outrb(&e2, R_USGN);	break;
				}
			case S_IMM:	/* #arg */
				outab(0xAE);
				outrw(&e2, R_NORM);	break;
			case S_IXO:	/* (offset,R), R = X, SP */
				if (ls_mode(&e2)) {
			case S_IXE:	/* (offset,R).e, R = X, SP */
					if (t2 == S_IXE) { aerr(); }
			case S_IXW:	/* (offset,R).w, R = X, SP */
					switch(v2) {
					case X:		outab(0xDE);
							outrw(&e2, R_USGN);	break;
					case SP:	if (t2 == S_IXW) { aerr(); }
							outab(0x1E);
							outrb(&e2, R_USGN);	break;
					default:	opcy_aerr();		break;
					}
				} else {
			case S_IXB:	/* (offset,R).b, R = X, SP */
					switch(v2) {
					case X:		outab(0xEE);
							outrb(&e2, R_USGN);	break;
					case SP:	outab(0x1E);
							outrb(&e2, R_USGN);	break;
					default:	opcy_aerr();		break;
					}
				}
				break;
			case S_IX:	/* (R), R = X */
				switch(v2) {
				case X:		outab(0xFE);	break;
				default:	opcy_aerr();	break;
				}
				break;
			case S_IN:	/* [offset] */
				if (ls_mode(&e2)) {
			case S_INE:	/* [offset].e */
					if (t2 == S_INE) { aerr(); }
			case S_INW:	/* [offset].w */
					outab(0x72);
					outab(0xCE);
					outrw(&e2, R_USGN);
				} else {
			case S_INB:	/* [offset].b */
					outab(0x92);
					outab(0xCE);
					outrb(&e2, R_USGN);
				}
				break;
			case S_INIX:	/* ([offset],R), R = X */
				if (ls_mode(&e2)) {
			case S_INIXE:	/* ([offset],R).e, R = X */
					if (t2 == S_INIXE) { aerr(); }
			case S_INIXW:	/* ([offset],R).w, R = X */
					switch(v2) {
					case X:		outab(0x72);
							outab(0xDE);
							outrw(&e2, R_USGN);	break;
					default:	opcy_aerr();		break;
					}
				} else {
			case S_INIXB:	/* ([offset],R).b, R = X */
					switch(v2) {
					case X:		outab(0x92);
							outab(0xDE);
							outrb(&e2, R_USGN);	break;
					default:	opcy_aerr();		break;
					}
				}
				break;
			default:
				opcy_aerr();
				break;
			}
			break;
		} else
		if ((t1 == S_REG) && (v1 == Y)) {
			switch(t2) {
			case S_LONG:	/*  arg */
				if (ls_mode(&e2)) {
					outab(0x90);
					outab(0xCE);
					outrw(&e2, R_USGN);	break;
				} else {
			case S_SHORT:	/* *arg */
					outab(0x90);
					outab(0xBE);
					outrb(&e2, R_USGN);	break;
				}
			case S_IMM:	/* #arg */
				outab(0x90);
				outab(0xAE);
				outrw(&e2, R_NORM);	break;
			case S_IXO:	/* (offset,R), R = Y, SP */
				if (ls_mode(&e2)) {
			case S_IXE:	/* (offset,R).e, R = Y, SP */
					if (t2 == S_IXE) { aerr(); }
			case S_IXW:	/* (offset,R).w, R = Y, SP */
					switch(v2) {
					case Y:	        outab(0x90);
							outab(0xDE);
							outrw(&e2, R_USGN);	break;
					case SP:	if (t2 == S_IXW) { aerr(); }
							outab(0x16);
							outrb(&e2, R_USGN);	break;
					default:	opcy_aerr();		break;
					}
				} else {
			case S_IXB:	/* (offset,R).b, R = Y, SP */
					switch(v2) {
					case Y:	        outab(0x90);
							outab(0xEE);
							outrb(&e2, R_USGN);	break;
					case SP:	outab(0x16);
							outrb(&e2, R_USGN);	break;
					default:	opcy_aerr();		break;
					}
				}
				break;
			case S_IX:	/* (R), R = Y */
				switch(v2) {
				case Y:	        outab(0x90);
						outab(0xFE);	break;
				default:	opcy_aerr();	break;
				}
				break;
			case S_IN:	/* [offset] */
				if (ls_mode(&e2)) {
			case S_INE:	/* [offset].e */
					if (t2 == S_INE) { aerr(); }
			case S_INW:	/* [offset].w */
					if (t2 == S_INW) { aerr(); }
					outab(0x91);
					outab(0xCE);
					outrb(&e2, R_USGN);
				} else {
			case S_INB:	/* [offset].b */
					outab(0x91);
					outab(0xCE);
					outrb(&e2, R_USGN);
				}
				break;
			case S_INIX:	/* ([offset],R), R = Y */
				if (ls_mode(&e2)) {
			case S_INIXE:	/* ([offset],R).e, R = Y */
					if (t2 == S_INIXE) { aerr(); }
			case S_INIXW:	/* ([offset],R).w, R = Y */
					switch(v2) {
					case Y:	        if (t2 == S_INIXW) { aerr(); }
							outab(0x91);
							outab(0xDE);
							outrb(&e2, R_USGN);	break;
					default:	opcy_aerr();		break;
					}
				} else {
			case S_INIXB:	/* ([offset],R).b, R = Y */
					switch(v2) {
					case Y:		outab(0x91);
							outab(0xDE);
							outrb(&e2, R_USGN);	break;
					default:	opcy_aerr();		break;
					}
				}
				break;
			default:
				opcy_aerr();
				break;
			}
			break;
		} else
		if ((t2 == S_REG) && (v2 == X)) {
			switch(t1) {
			case S_LONG:	/*  arg */
				if (ls_mode(&e1)) {
					outab(0xCF);
					outrw(&e1, R_USGN);	break;
				} else {
			case S_SHORT:	/* *arg */
					outab(0xBF);
					outrb(&e1, R_USGN);	break;
				}
			case S_IXO:	/* (offset,R), R = Y, SP */
				if (ls_mode(&e1)) {
			case S_IXE:	/* (offset,R).e, R = Y, SP */
					if (t1 == S_IXE) { aerr(); }
			case S_IXW:	/* (offset,R).w, R = Y, SP */
					switch(v1) {
					case Y:	        outab(0x90);
							outab(0xDF);
							outrw(&e1, R_USGN);	break;
					case SP:	if (t1 == S_IXW) { aerr(); }
							outab(0x1F);
							outrb(&e1, R_USGN);	break;
					default:	opcy_aerr();		break;
					}
				} else {
			case S_IXB:	/* (offset,R).b, R = Y, SP */
					switch(v1) {
					case Y:	        outab(0x90);
							outab(0xEF);
							outrb(&e1, R_USGN);	break;
					case SP:	outab(0x1F);
							outrb(&e1, R_USGN);	break;
					default:	opcy_aerr();		break;
					}
				}
				break;
			case S_IX:	/* (R), R = Y */
				switch(v1) {
				case Y:	        outab(0x90);
						outab(0xFF);	break;
				default:	opcy_aerr();	break;
				}
				break;
			case S_IN:	/* [offset] */
				if (ls_mode(&e1)) {
			case S_INE:	/* [offset].e */
					if (t1 == S_INE) { aerr(); }
			case S_INW:	/* [offset].w */
					outab(0x72);
					outab(0xCF);
					outrw(&e1, R_USGN);
				} else {
			case S_INB:	/* [offset].b */
					outab(0x92);
					outab(0xCF);
					outrb(&e1, R_USGN);
				}
				break;
			case S_INIX:	/* ([offset],R), R = Y */
				if (ls_mode(&e1)) {
			case S_INIXE:	/* ([offset],R).e, R = Y */
					if (t1 == S_INIXE) { aerr(); }
			case S_INIXW:	/* ([offset],R).w, R = Y */
					switch(v1) {
					case Y:	        if (t1 == S_INIXW) { aerr(); }
							outab(0x91);
							outab(0xDF);
							outrb(&e1, R_USGN);	break;
					default:	opcy_aerr();		break;
					}
				} else {
			case S_INIXB:	/* ([offset],R).b, R = Y */
					switch(v1) {
					case Y:		outab(0x91);
							outab(0xDF);
							outrb(&e1, R_USGN);	break;
					default:	opcy_aerr();		break;
					}
				}
				break;
			default:
				opcy_aerr();
				break;
			}
			break;
		} else
		if ((t2 == S_REG) && (v2 == Y)) {
			switch(t1) {
			case S_LONG:	/*  arg */
				if (ls_mode(&e1)) {
					outab(0x90);
					outab(0xCF);
					outrw(&e1, R_USGN);	break;
				} else {
			case S_SHORT:	/* *arg */
					outab(0x90);
					outab(0xBF);
					outrb(&e1, R_USGN);	break;
				}
			case S_IXO:	/* (offset,R), R = X, SP */
				if (ls_mode(&e1)) {
			case S_IXE:	/* (offset,R).e, R = X, SP */
					if (t1 == S_IXE) { aerr(); }
			case S_IXW:	/* (offset,R).w, R = X, SP */
					switch(v1) {
					case X:		outab(0xDF);
							outrw(&e1, R_USGN);	break;
					case SP:	if (t1 == S_IXW) { aerr(); }
							outab(0x17);
							outrb(&e1, R_USGN);	break;
					default:	opcy_aerr();		break;
					}
				} else {
			case S_IXB:	/* (offset,R).b, R = X, SP */
					switch(v1) {
					case X:		outab(0xEF);
							outrb(&e1, R_USGN);	break;
					case SP:	outab(0x17);
							outrb(&e1, R_USGN);	break;
					default:	opcy_aerr();		break;
					}
				}
				break;
			case S_IX:	/* (R), R = X */
				switch(v1) {
				case X:		outab(0xFF);	break;
				default:	opcy_aerr();	break;
				}
				break;
			case S_IN:	/* [offset] */
				if (ls_mode(&e1)) {
			case S_INE:	/* [offset].e */
					if (t1 == S_INE) { aerr(); }
			case S_INW:	/* [offset].w */
					if (t1 == S_INW) { aerr(); }
					outab(0x91);
					outab(0xCF);
					outrb(&e1, R_USGN);
				} else {
			case S_INB:	/* [offset].b */
					outab(0x91);
					outab(0xCF);
					outrb(&e1, R_USGN);
				}
				break;
			case S_INIX:	/* ([offset],R), R = X */
				if (ls_mode(&e1)) {
			case S_INIXE:	/* ([offset],R).e, R = X */
					if (t1 == S_INIXE) { aerr(); }
			case S_INIXW:	/* ([offset],R).w, R = X */
					switch(v1) {
					case X:		outab(0x72);
							outab(0xDF);
							outrw(&e1, R_USGN);	break;
					default:	opcy_aerr();		break;
					}
				} else {
			case S_INIXB:	/* ([offset],R).b, R = X */
					switch(v1) {
					case X:		outab(0x92);
							outab(0xDF);
							outrb(&e1, R_USGN);	break;
					default:	opcy_aerr();		break;
					}
				}
				break;
			default:
				opcy_aerr();
				break;
			}
			break;
		}
		opcy_aerr();
		break;

	/*
	 * S_MOV:
	 *	MOV
	 */
	case S_MOV:
		t1 = addr(&e1);
		v1 = rcode;
		comma(1);
		t2 = addr(&e2);
		v2 = rcode;
		if (t1 == S_LONG) {
			t1 = ls_mode(&e1) ? S_LONG : S_SHORT;
		}
		if (t2 == S_LONG) {
			t2 = ls_mode(&e2) ? S_LONG : S_SHORT;
		}
		switch(t1) {
		case S_LONG:
			switch(t2) {
			case S_IMM:	outab(0x35);
					outrb(&e2, R_NORM);     valu_aerr(&e2, 1);
					outrw(&e1, R_USGN);	break;
			case S_SHORT:
			case S_LONG:	outab(0x55);
					outrw(&e2, R_USGN);
					outrw(&e1, R_USGN);	break;
			default:	opcy_aerr();		break;
			}
			break;
		case S_SHORT:
			switch(t2) {
			case S_IMM:	outab(0x35);
					outrb(&e2, R_NORM);     valu_aerr(&e2, 1);
					outrw(&e1, R_USGN);	break;
			case S_SHORT:	outab(0x45);
					outrb(&e2, R_USGN);
					outrb(&e1, R_USGN);	break;
			case S_LONG:	outab(0x55);
					outrw(&e2, R_USGN);
					outrw(&e1, R_USGN);	break;
			default:	opcy_aerr();		break;
			}
			break;
		default:	opcy_aerr();	break;
		}
		break;

	case S_WOP:
		t1 = addr(&e1);
		v1 = rcode;
		if (t1 == S_REG) {
			switch(v1) {
			case Y:		outab(0x90);
			case X:		outab(op);	break;
			default:	opcy_aerr();	break;
			}
		} else {
			opcy_aerr();
		}
		break;

	case S_RWA:
		t1 = addr(&e1);
		v1 = rcode;
		if(t1 != S_REG) {
			opcy_aerr();
			break;
		}
		switch(v1) {
		case Y:		outab(0x90);
		case X:		outab(op);	break;
		default:	opcy_aerr();	break;
		}
		break;

	case S_EXG:
		t1 = addr(&e1);
		v1 = rcode;
		comma(1);
		t2 = addr(&e2);
		v2 = rcode;
		if ((t1 == S_REG) && (v1 == A)) {
			switch(t2) {
			case S_SHORT:
			case S_LONG:	outab(op | 0x30);
					outrw(&e2, R_USGN);	break;
			case S_REG:
				switch(v2) {
				case XL:	outab(op | 0x40);	break;
				case YL:	outab(op | 0x60);	break;
				default:	opcy_aerr();		break;
				}
				break;
			default:	opcy_aerr();		break;
			}
		} else {
			opcy_aerr();
		}
		break;

	case S_MLDV:
		t1 = addr(&e1);
		v1 = rcode;
		comma(1);
		t2 = addr(&e2);
		v2 = rcode;
		if ((t2 != S_REG) && (v2 != A)) {
			opcy_aerr();
			break;
		}
		if (t1 == S_REG) {
			switch(v1) {
			case Y:		outab(0x90);
			case X:		outab(op);	break;
			default:	opcy_aerr();	break;
			}
		} else {
			opcy_aerr();
		}
		break;

	case S_DIVW:
		t1 = addr(&e1);
		v1 = rcode;
		comma(1);
		t2 = addr(&e2);
		v2 = rcode;
		if (((t1 == S_REG) && (v1 == X)) &&
		    ((t2 == S_REG) && (v2 == Y))) {
			outab(0x65);
		} else {
			opcy_aerr();
		}
		break;

	case S_EXGW:
		t1 = addr(&e1);
		v1 = rcode;
		comma(1);
		t2 = addr(&e2);
		v2 = rcode;
		if ((t1 == S_REG) && (t2 == S_REG)) {
			if (((v1 == X) && (v2 == Y)) ||
			    ((v1 == Y) && (v2 == X))) {
				outab(op);
				break;
			}
		}
		opcy_aerr();
		break;

	case S_POP:
		t1 = addr(&e1);
		v1 = rcode;
		switch(t1) {
		case S_REG:
			switch(v1) {
			case A:		outab(0x84);		break;
			case CC:	outab(0x86);		break;
			default:	opcy_aerr();		break;
			}
			break;
		case S_SHORT:
		case S_LONG:		outab(op);
					outrw(&e1, R_USGN);	break;
		default:		opcy_aerr();		break;
		}
		break;

	case S_PUSH:
		t1 = addr(&e1);
		v1 = rcode;
		switch(t1) {
		case S_REG:
			switch(v1) {
			case A:		outab(0x88);		break;
			case CC:	outab(0x8A);		break;
			default:	opcy_aerr();		break;
			}
			break;
		case S_SHORT:
		case S_LONG:		outab(op);
					outrw(&e1, R_USGN);	break;
		case S_IMM:		outab(0x4B);
					outrb(&e1, R_NORM);	break;
		default:		opcy_aerr();		break;
		}
		break;

	case S_PW:
		t1 = addr(&e1);
		v1 = rcode;
		switch(t1) {
		case S_REG:
			switch(v1) {
			case Y:		outab(0x90);
			case X:		outab(op);		break;
			default:	opcy_aerr();		break;
			}
			break;
		default:		opcy_aerr();		break;
		}
		break;

	/*
	 * S_CLJP:
	 *	CALL, JP
	 */
	case S_CLJP:
		t1 = addr(&e1);
		v1 = rcode;
		switch(t1) {
		case S_LONG:	/*  arg */
		case S_SHORT:	/* *arg */
			outab(op | 0xC0);
			outrw(&e1, R_NORM);
			break;
		case S_IXO:	/* (offset,R), R = X, Y */
			if (ls_mode(&e1)) {
		case S_IXE:	/* (offset,R).e, R = X, Y */
				if (t1 == S_IXE) { aerr(); }
		case S_IXW:	/* (offset,R).w, R = X, Y */
				switch(v1) {
				case Y:		outab(0x90);
				case X:		outab(op | 0xD0);
						outrw(&e1, R_USGN);	break;
				default:	opcy_aerr();		break;
				}
			} else {
		case S_IXB:	/* (offset,R).b, R = X, Y */
				switch(v1) {
				case Y:		outab(0x90);
				case X:		outab(op | 0xE0);
						outrb(&e1, R_USGN);	break;
				default:	opcy_aerr();		break;
				}
			}
			break;
		case S_IX:	/* (R), R = X, Y */
			switch(v1) {
			case Y:		outab(0x90);
			case X:		outab(op | 0xF0);	break;
			default:	opcy_aerr();		break;
			}
			break;
		case S_IN:	/* [offset] */
			if (ls_mode(&e1)) {
		case S_INE:	/* [offset].e */
				if (t1 == S_INE) { aerr(); }
		case S_INW:	/* [offset].w */
				outab(0x72);
				outab(op | 0xC0);
				outrw(&e1, R_USGN);
			} else {
		case S_INB:	/* [offset].b */
				outab(0x92);
				outab(op | 0xC0);
				outrb(&e1, R_USGN);
			}
			break;
		case S_INIX:	/* ([offset],R), R = X, Y */
			if (ls_mode(&e1)) {
		case S_INIXE:	/* ([offset],R).e, R = X, Y */
				if (t1 == S_INIXE) { aerr(); }
		case S_INIXW:	/* ([offset],R).w, R = X, Y */
				switch(v1) {
				case X:		outab(0x72);
						outab(op | 0xD0);
						outrw(&e1, R_USGN);	break;
				case Y:		if (t1 == S_INIXW) { aerr(); }
						outab(0x91);
						outab(op | 0xD0);
						outrb(&e1, R_USGN);	break;
				default:	opcy_aerr();		break;
				}
			} else {
		case S_INIXB:	/* ([offset],R).b, R = X, Y */
				switch(v1) {
				case X:
				case Y:		switch(v1) {
						case X:		outab(0x92);	break;
						case Y:		outab(0x91);	break;
						default:			break;
						}
						outab(op | 0xD0);
						outrb(&e1, R_USGN);	break;
				default:	opcy_aerr();		break;
				}
			}
			break;
		default:
			opcy_aerr();
			break;
		}
		break;


	/*
	 * S_CLJPF:
	 *	CALLF, JPF
	 */
	case S_CLJPF:
		t1 = addr(&e1);
		v1 = rcode;
		switch(t1) {
		case S_SHORT:
		case S_LONG:	outab(op);
				outr3b(&e1, R_NORM);	break;
		case S_INB:	/* [offset].b */
		case S_INW:	/* [offset].w */
			aerr();
		case S_INE:	/* [offset].e */
		case S_IN:	/* [offset] */
				outab(0x92);
				outab(op);
				outrw(&e1, R_USGN);	break;
		default:	opcy_aerr();		break;
		}
		break;

	case S_JRPG:
		outab(0x90);

	case S_JR:
	case S_CALLR:
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
		if (e1.e_mode != S_USER) {
			rerr();
		}
		break;

	case S_JRBT:
		t1 = addr(&e1);
		v1 = (int) e1.e_addr;
		comma(1);
		t2 = addr(&e2);
		v2 = (int) e2.e_addr;
		comma(1);
		expr(&e3, 0);
		if (((t1 != S_SHORT) && (t1 != S_LONG)) ||
		     (t2 != S_IMM)) {
			opcy_aerr();
			break;
		}
		if (is_abs(&e2) && (v2 & ~0x07)) {
			aerr();
		}
		outab(0x72);
		//outrbm(&e2, R_BITS, op);
		outab(op|(v2 << 1));  /* TODO: maybe fix outrb vs. outrbm */
		outrw(&e1, R_USGN);
		if (mchpcr(&e3)) {
			v3 = (int) (e3.e_addr - dot.s_addr - 1);
			if ((v3 < -128) || (v3 > 127))
				aerr();
			outab(v3);
		} else {
			outrb(&e3, R_PCR);
		}
		if (e3.e_mode != S_USER) {
			rerr();
		}
		break;

	case S_BT72:
	case S_BT90:
		t1 = addr(&e1);
		v1 = (int) e1.e_addr;
		comma(1);
		t2 = addr(&e2);
		v2 = (int) e2.e_addr;
		if (((t1 != S_SHORT) && (t1 != S_LONG)) ||
		     (t2 != S_IMM)) {
			opcy_aerr();
			break;
		}
		if (is_abs(&e2) && (v2 & ~0x07)) {
			aerr();
		}
		switch(rf) {
		case S_BT72:	outab(0x72);	break;
		case S_BT90:	outab(0x90);	break;
		default:			break;
		}
		//outrbm(&e2, R_BITS, op);
                outab(op|v2*2); /* TODO: maybe fix outrb vs. outrbm */
		outrw(&e1, R_USGN);
		break;

	case S_INH72:
		outab(0x72);

	case S_INH:
		outab(op);
		break;

	case S_INT:
		t1 = addr(&e1);
		if(t1 != S_LONG)
			opcy_aerr();
		outab(0x82);
		outr3b(&e1, R_USGN);
		break;

	default:
		opcycles = OPCY_ERR;
		err('o');
		break;
	}

	if (opcycles == OPCY_NONE) {
		opcycles = stm8pg[cb[0] & 0xFF];
		if ((opcycles & OPCY_NONE) && (opcycles & OPCY_MASK)) {
			opcycles = Page[opcycles & OPCY_MASK][cb[1] & 0xFF];
		}
	}
}

/*
 * Disable Opcode Cycles with aerr()
 */
VOID
opcy_aerr()
{
	opcycles = OPCY_SKP;
	aerr();
}

/*
 * Select the long or short addressing mode
 * based upon the expression type and value.
 */
int
ls_mode(e)
struct expr *e;
{
	int flag, v;

	v = (int) e->e_addr;
	/*
	 * 1) area based arguments (e_base.e_ap != 0) use longer mode
	 * 2) constant arguments (e_base.e_ap == 0) use
	 * 	shorter mode if (arg & ~0xFF) == 0
	 *	longer  mode if (arg & ~0xFF) != 0
	 */
	if (pass == 0) {
		;
	} else
	if (e->e_base.e_ap) {
		;
	} else
	if (pass == 1) {
		if (e->e_addr >= dot.s_addr) {
			e->e_addr -= fuzz;
		}
		flag = (v & ~0xFF) ? 1 : 0;
		return(setbit(flag) ? 1 : 0);
	} else {
		return(getbit() ? 1 : 0);
	}
	return(1);
}

/*
 * Generate an 'a' error if the absolute
 * value is not a valid unsigned or signed value.
 */
VOID
valu_aerr(e, n)
struct expr *e;
int n;
{
	int v;

	if (is_abs(e)) {
		v = e->e_addr;
		switch(n) {
		default:
#ifdef	LONGINT
		case 1:	if ((v & ~0x000000FFl) && ((v & ~0x000000FFl) != ~0x000000FFl)) aerr();	break;
		case 2:	if ((v & ~0x0000FFFFl) && ((v & ~0x0000FFFFl) != ~0x0000FFFFl)) aerr();	break;
		case 3:	if ((v & ~0x00FFFFFFl) && ((v & ~0x00FFFFFFl) != ~0x00FFFFFFl)) aerr();	break;
		case 4:	if ((v & ~0xFFFFFFFFl) && ((v & ~0xFFFFFFFFl) != ~0xFFFFFFFFl)) aerr();	break;
#else
		case 1:	if ((v & ~0x000000FF) && ((v & ~0x000000FF) != ~0x000000FF)) aerr();	break;
		case 2:	if ((v & ~0x0000FFFF) && ((v & ~0x0000FFFF) != ~0x0000FFFF)) aerr();	break;
		case 3:	if ((v & ~0x00FFFFFF) && ((v & ~0x00FFFFFF) != ~0x00FFFFFF)) aerr();	break;
		case 4:	if ((v & ~0xFFFFFFFF) && ((v & ~0xFFFFFFFF) != ~0xFFFFFFFF)) aerr();	break;
#endif
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
 * Machine specific initialization.
 */
VOID
minit()
{
	/*
	 * 24-Bit Machine
	 */
	exprmasks(3);

	/*
	 * Byte Order
	 */
	hilo = 1;

	/*
	 * Reset Bit Table
	 */
	bp = bb;
	bm = 1;
}

/*
 * Store `b' in the next slot of the bit table.
 * If no room, force the longer form of the offset.
 */
int
setbit(b)
int b;
{
	if (bp >= &bb[NB])
		return(1);
	if (b)
		*bp |= bm;
	bm <<= 1;
	if (bm == 0) {
		bm = 1;
		++bp;
	}
	return(b);
}

/*
 * Get the next bit from the bit table.
 * If none left, return a `1'.
 * This will force the longer form of the offset.
 */
int
getbit()
{
	int f;

	if (bp >= &bb[NB])
		return (1);
	f = *bp & bm;
	bm <<= 1;
	if (bm == 0) {
		bm = 1;
		++bp;
	}
	return (f);
}


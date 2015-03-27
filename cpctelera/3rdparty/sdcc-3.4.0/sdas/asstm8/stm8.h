/* ST8.h */

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

/*)BUILD
	$(PROGRAM) =	ASSTM8
	$(INCLUDE) = {
		ASXXXX.H
		ST8.H
	}
	$(FILES) = {
		ST8MCH.C
		ST8ADR.C
		ST8PST.C
		ASMAIN.C
		ASMCRO.C
		ASDBG.C
		ASLEX.C
		ASSYM.C
		ASSUBR.C
		ASEXPR.C
		ASDATA.C
		ASLIST.C
		ASOUT.C
	}
	$(STACK) = 3000
*/

/*
 * Registers
 */
#define A		 0
#define X		 1
#define XL		 2
#define XH		 3
#define Y		 4
#define YL		 5
#define YH		 6

#define SP		 7
#define CC		 8

/*
 * Addressing Modes
 */
#define	S_REG		0x00
#define	S_SHORT		0x01
#define	S_LONG		0x02
#define	S_EXT		0x04
/*	Illegal		0x03, 0x05, 0x06, 0x07	*/

#define	S_IXO		0x08
#define	S_IXB		0x09
#define	S_IXW		0x0A
#define	S_IXE		0x0C
/*	Illegal		0x0B, 0x0D, 0x0E, 0x0F	*/

#define	S_IN		0x10
#define	S_INB		0x11
#define	S_INW		0x12
#define	S_INE		0x14
/*	Illegal		0x13, 0x15, 0x16, 0x17	*/

#define	S_INIX		0x18
#define	S_INIXB		0x19
#define	S_INIXW		0x1A
#define	S_INIXE		0x1C
/*	Illegal		0x1B, 0x1D, 0x1E, 0x1F	*/

#define	S_IMM		0x20
#define	S_IX		0x21

/*
 * Instruction types
 */
#define	S_JR		60
#define	S_JRPG		61
#define	S_JRBT		62
#define	S_BT72		63
#define	S_BT90		64
#define	S_LD		65
#define	S_LDF		66
#define	S_LDW		67
#define	S_MOV		68
#define	S_AOP		69
#define	S_BOP		70
#define	S_WOP		71
#define	S_ADDW		72
#define	S_CPW		73
#define	S_SUBW		74
#define	S_RWA		75
#define	S_EXG		76
#define	S_EXGW		77
#define	S_POP		78
#define	S_PUSH		79
#define	S_PW		80
#define	S_CLJP		81
#define	S_CLJPF		82
#define	S_CALLR		83
#define	S_INH		84
#define	S_INH72		85
#define	S_MLDV		86
#define	S_DIVW		87
#define	S_INT		88

/*
 * Extended Addressing Modes
 */
#define	R_BITS	0x0100		/* Bit Test Addressing Mode */


struct adsym
{
	char	a_str[4];	/* addressing string */
	int	a_val;		/* addressing mode value */
};

extern	struct	adsym	REG[];

extern	int	rcode;

	/* machine dependent functions */

#ifdef	OTHERSYSTEM
	
        /* ST8adr.c */
extern	int		addr(struct expr *esp);
extern	int		addr1(struct expr *esp);
extern	int		addrsl(struct expr *esp);
extern	int		admode(struct adsym *sp);
extern	int		any(int c, char *str);
extern	int		srch(char *str);

	/* ST8mch.c */
extern	VOID		machine(struct mne *mp);
extern	int		mchpcr(struct expr *esp);
extern	VOID		minit(void);
extern	VOID		opcy_aerr(void);
extern	VOID		valu_aerr(struct expr *e, int n);
extern	int		ls_mode(struct expr *e);
extern	int		setbit(int b);
extern	int		getbit(void);

#else

	/* ST8adr.c */
extern	int		addr();
extern	int		addr1();
extern	int		addrsl();
extern	int		admode();
extern	int		any();
extern	int		srch();

	/* ST8mch.c */
extern	VOID		machine();
extern	int		mchpcr();
extern	VOID		minit();
extern	VOID		opcy_aerr();
extern	VOID		valu_aerr();
extern	int		ls_mode();
extern	int		setbit();
extern	int		getbit();

#endif


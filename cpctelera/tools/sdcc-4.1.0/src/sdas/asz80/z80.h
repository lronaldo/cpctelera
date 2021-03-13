/* z80.h */

/*
 *  Copyright (C) 1989-2009  Alan R. Baldwin
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

/*)BUILD
	$(PROGRAM) =	ASZ80
	$(INCLUDE) = {
		ASXXXX.H
		Z80.H
	}
	$(FILES) = {
		Z80MCH.C
		Z80ADR.C
		Z80PST.C
		ASMAIN.C
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
 * Indirect Addressing delimeters
 */
#define	LFIND	'('
#define RTIND	')'

/*
 * Registers
 */
#define B	0
#define C	1
#define D	2
#define E	3
#define H	4
#define L	5
#define A	7

#define I	0107
#define R	0117

#define X       8       /* for ZXN pop x */
#define	MB	9	/* for eZ80 */

#define BC	0
#define DE	1
#define HL	2
#define SP	3
#define AF	4
#define IX	5
#define IY	6

/*
 * Conditional definitions
 */
#define	NZ	0
#define	Z	1
#define	NC	2
#define	CS	3
#define	PO	4
#define	PE	5
#define	P	6
#define	M	7

/*
 * Symbol types
 */
#define	S_IMMED	30
#define	S_R8	31
#define	S_R8X	32
#define	S_R16	33
#define	S_R16X	34
#define	S_CND	35
#define	S_FLAG	36
#define S_R8U1  37
#define S_R8U2  38
#define	S_R8MB	39

/*
 * Indexing modes
 */
#define	S_INDB	40
#define	S_IDC	41
#define	S_INDR	50
#define	S_IDBC	50
#define	S_IDDE	51
#define	S_IDHL	52
#define	S_IDSP	53
#define	S_IDIX	55
#define	S_IDIY	56
#define	S_INDM	57

/*
 * Instruction types
 */
#define	S_LD	60
#define	S_CALL	61
#define	S_JP	62
#define	S_JR	63
#define	S_RET	64
#define	S_BIT	65
#define	S_INC	66
#define	S_DEC	67
#define	S_ADD	68
#define	S_ADC	69
#define	S_AND	70
#define	S_EX	71
#define	S_PUSH	72
#define	S_IN	73
#define	S_OUT	74
#define	S_RL	75
#define	S_RST	76
#define	S_IM	77
#define	S_INH1	78
#define	S_INH2	79
#define	S_DJNZ	80
#define	S_SUB	81
#define	S_SBC	82

/*
 * CPU Types
 */
#define	S_CPU	83

#define S_RL_UNDOCD  85
#define X_UNDOCD 89

/*
 * Processor Types (S_CPU)
 */
#define	X_Z80	0
#define	X_HD64	1
#define	X_ZXN	2
#define	X_EZ80	3

/*
 * HD64180 Instructions
 */
#define	X_INH2	90
#define	X_IN	91
#define	X_OUT	92
#define	X_MLT	93
#define	X_TST	94
#define	X_TSTIO	95

/*
 * Z80-ZX Next Instructions
 */
#define X_ZXN_INH2	100
#define X_ZXN_MUL	101
#define X_ZXN_MIRROR	102
#define X_ZXN_NEXTREG   103
#define X_ZXN_CU_WAIT   104
#define X_ZXN_CU_MOVE   105
#define X_ZXN_CU_STOP   106
#define X_ZXN_CU_NOP    107

/*
 * eZ80 Instructions
 */
#define	X_EZ_ADL	110
#define	X_EZ_INH2	111
#define	X_EZ_LEA	112
#define	X_EZ_PEA	113

/*
 * eZ80 specific addressing extensions (used in mne m_flag)
 */
#define	M_L		0x01
#define	M_S		0x02
#define	M_IL		0x04
#define	M_IS		0x08
#define M_LIL		(M_L | M_IL)
#define M_LIS		(M_L | M_IS)
#define	M_SIL		(M_S | M_IL)
#define	M_SIS		(M_S | M_IS)

/*
 * Extended Addressing Modes
 */
#define	R_ADL	0x0000		/* 24-Bit Addressing Mode */
#define	R_Z80	0x0100		/* 16-Bit Addressing Mode */
#define	R_3BIT	0x0200		/*  3-Bit Addressing Mode */

struct adsym
{
	char	a_str[4];	/* addressing string */
	int	a_val;		/* addressing mode value */
};

extern	struct	adsym	R8[];
extern	struct	adsym	R8X[];
extern	struct	adsym	R8U1[];
extern	struct	adsym	R8U2[];

extern	struct	adsym	R16[];
extern	struct	adsym	R16X[];
extern  struct  adsym   R8MB[];
extern	struct	adsym	RX[];
extern	struct	adsym	CND[];

	/* machine dependent functions */

#ifdef	OTHERSYSTEM
	
	/* z80adr.c */
extern	int		addr(struct expr *esp);
extern	int		admode(struct adsym *sp);
extern	int		srch(char *str);

	/* z80mch.c */
extern	int		genop(int pop, int op, struct expr *esp, int f);
extern	int		gixiy(int v);
extern	VOID		glilsis(int sfx, struct expr *esp);
extern	VOID		machine(struct mne *mp);
extern	int		mchpcr(struct expr *esp);
extern	VOID		minit(void);

#else

	/* z80adr.c */
extern	int		addr();
extern	int		admode();
extern	int		srch();

	/* z80mch.c */
extern	int		genop();
extern	int		gixiy();
extern	VOID		glilsis();
extern	VOID		machine();
extern	int		mchpcr();
extern	VOID		minit();

#endif

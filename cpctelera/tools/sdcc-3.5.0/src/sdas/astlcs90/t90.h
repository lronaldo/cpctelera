/* t90.h */

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

/*
 * TLCS90 Port: R. Keuchel
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
#define A	6

#define I	0107
#define R	0117

#define BC	0
#define DE	1
#define HL	2
#define IX	4
#define IY	5
#define SP	6

#define AF	6

/*
 * Conditional definitions
 */

/*
F - always False
T - always True
Z Z = 1 Result zero
NZ Z = 0 Result not zero
C C = 1 Carry is Set
NC C = 0 Carry is not Set
PL or P S = 0 Result positive; >= 0
MI or M S = 1 Result negative; < 0
NE Z = 0 Result not zero
EQ Z = 1 Result zero
OV P/V = 1 Overflow ocurred
NOV P/V = 0 Overflow didn't occur
PE P/V = 1 Result has even Parity
PO P/V = 0 Result has odd Parity
GE (S xor P/V) = 0 Result (signed) is >= 0
LT (S xor P/V) = 1 Result (signed) is < 0
GT [Z or (S xor P/V)] = 0 Result (signed) is > 0
LE [Z or (S xor P/V)] = 1 Result (signed) is <= 0
UGE C = 0 Result (unsigned) is >= 0
ULT C = 1 Result (unsigned) is < 0
UGT (C or Z) = 0 Result (unsigned) is > 0
ULE (C or Z) = 1 Result (unsigned) is <= 0
*/

#define CS	0x7
#define EQ	0x6
#define F       0x0
#define GE      0x9
#define GT      0xA
#define LE      0x2
#define LT      0x1
#define M	0x5
#define NC	0xF
#define NE	0xE
#define NOV	0xC
#define NZ	0xE
#define OV      0x4
#define P       0xD
#define PE      0x4
#define PO	0xC
#define T       0x8
#define UGE     0xF
#define UGT     0xB
#define ULE     0x3
#define ULT	0x7
#define Z	0x6

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
//#define S_R8U1  37
#define S_R8U2  38

/*
 * Indexing modes
 */
#define	S_INDB	40
#define	S_IDC	41

#define	S_INDR	50

#define	S_IDBC	50
#define	S_IDDE	51
#define	S_IDHL	52
#define	S_IDIX	54
#define	S_IDIY	55
#define	S_IDSP	56

#define	S_INDM	57

/*
 * Instruction types
 */
#define	S_LD	60
#define	S_CALL	61
#define	S_JP	62
#define	S_JR	63
#define	S_RET	64
#define	S_INC	66
#define	S_DEC	67
#define	S_ADD	68
#define	S_ADC	69
#define	S_EX	71
#define	S_PUSH	72
#define	S_INH1	78
#define	S_DJNZ	80
#define	S_SUB	81
#define	S_SBC	82

/*
 * CPU Types
 */

#define S_INCX  84
#define S_DECX  85

#define	S_IRET	86

#define	S_MUL	87
#define	S_DIV	88

#define S_INCW  89
#define S_DECW  90

#define	S_BIT	91
#define	S_SET	92
#define	S_RES	93

#define	S_AND	94
#define	S_CP	95
#define	S_OR	96
#define	S_XOR	97

// must be ordered!
#define	S_RLC	98
#define	S_RRC	99
#define	S_RL	100
#define	S_RR	101
#define	S_SLA	102
#define	S_SRA	103
#define	S_SLL	104
#define	S_SRL	105

#define	S_LDI	106
#define	S_LDIR	107
#define	S_LDD	108
#define	S_LDDR	109
#define	S_CPI	110
#define	S_CPIR	111
#define	S_CPD	112
#define	S_CPDR	113

#define	S_LDAR	114

#define	S_LDW	115

#define	S_RLD	116
#define	S_RRD	117

#define S_CALLR 118

// .t90
#define	S_CPU	123

/*
 * Processor Types (S_CPU)
 */
#define	X_T90	0

struct adsym
{
	char a_str[4];	/* addressing string */
	int	a_val;		/* addressing mode value */
};

extern	struct	adsym	R8[];
//extern	struct	adsym	R8X[];
//extern	struct	adsym	R8U1[];
//extern	struct	adsym	R8U2[];

extern	struct	adsym	R16[];
extern	struct	adsym	R16X[];
extern	struct	adsym	CND[];

	/* machine dependent functions */

#ifdef	OTHERSYSTEM
	
	/* t90adr.c */
extern	int		addr(struct expr *esp);
extern	int		admode(struct adsym *sp);
extern	int		srch(char *str);

	/* t90mch.c */
extern	int		genop(int pop, int op, struct expr *esp, int f);
extern	VOID		machine(struct mne *mp);
extern	int		mchpcr(struct expr *esp);
extern	VOID		minit(void);

#else

	/* t90adr.c */
extern	int		addr();
extern	int		admode();
extern	int		srch();

	/* t90mch.c */
extern	VOID		machine();
extern	int		mchpcr();
extern	VOID		minit();

#endif

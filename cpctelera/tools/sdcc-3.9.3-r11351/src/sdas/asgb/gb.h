/* gb.h */

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

/* Gameboy mods by Roger Ivie (ivie at cc dot usu dot edu) 
 *
 * The Gameboy mods are based on 
 * http://www.komkon.org/fms/GameBoy/Tech/Software.html
 * by Marat Fayzullin
 *
 * The Gameboy is allegedly a Z80 with the following mods:
 *
 * - No index registers
 * - No alternate registers
 * - No I and R
 * - No I/O instructions (all I/O is memory mapped)
 * - No parity flag
 * - No minus flag
 * - No instructions with ED prefix (RETI is moved)
 * - HALT is always interruptible, even if interrupts have been disabled.
 * - The following instructions are different:
 * - No ADC or SBC of r16s.
 *
 * ----------------------------------------------------------------------------
 * Code       Z80 operation  GameBoy operation
 * ----------------------------------------------------------------------------
 * 08 xx xx   EX AF,AF'      LD (word),SP     Save SP at given address
 * 10 xx      DJNZ offset    STOP             Meaning unknown
 * 22         LD (word),HL   LD (HLI),A       Save A at (HL) and increment HL
 * 2A         LD HL,(word)   LD A,(HLI)       Load A from (HL) and increment HL
 * 32         LD (word),A    LD (HLD),A       Save A at (HL) and decrement HL
 * 3A         LD A,(word)    LD A,(HLD)       Load A from (HL) and decrement HL
 * D3         OUTA (byte)    No operation
 * D9         EXX            RETI             Enable interrupts and return
 * DB         INA (byte)     No operation
 * DD         Prefix DD      No operation
 * E0 xx      RET PO         LD (byte),A      Save A at (FF00+byte)
 * E2         JP PO,word     LD (C),A         Save A at (FF00+C)
 * E3         EX HL,(SP)     No operation
 * E4         CALL PO,word   No operation
 * E8 xx      RET PE         ADD SP,offset    Add signed offset to SP
 * EA xx xx   JP PE,word     LD (word),A      Save A at given address
 * EB         EX DE,HL       No operation
 * EC         CALL PE,word   No operation
 * F0 xx      RET P          LD A,(byte)      Load A from (FF00+byte)
 * F2         JP P,word      No operation
 * F4         CALL P,word    No operation
 * F8 xx      RET M          LDHL SP,offset   Load HL with SP + signed offset
 * FA xx xx   JP M,word      LD A,(word)      Load A from given address
 * FC         CALL M,word    No operation
 * FD         Prefix FD      No operation
 * ----------------------------------------------------------------------------
 */

/*
 * Extensions: P. Felber
 */

/*)BUILD
	$(PROGRAM) =	ASGB
	$(INCLUDE) = {
		ASXXXX.H
		GB.H
	}
	$(FILES) = {
		GBMCH.C
		GBADR.C
		GBPST.C
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

#define BC	0
#define DE	1
#define HL	2
#define SP	3
#define AF	4
#define HLD	5
#define HLI	6

/*
 * Conditional definitions
 */
#define	NZ	0
#define	Z	1
#define	NC	2
#define	CS	3

/*
 * Symbol types
 */
#define	S_IMMED	30
#define	S_R8	31
#define	S_R16	33
#define	S_R16X	34   /* AF */
#define	S_CND	35
#define	S_FLAG	36

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
#define	S_IDHLD	55
#define	S_IDHLI	56
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
#define	S_PUSH	72
#define	S_LDH	73

#define	S_RL	76
#define	S_RST	77
#define	S_IM	78
#define	S_INH1	79
#define	S_SUB	80
#define	S_SBC	81
#define S_LDHL	90	/* LDHL SP,offset */
#define	S_LDA	91
#define	S_STOP	83

struct adsym
{
	char	a_str[4];	/* addressing string */
	int	a_val;		/* addressing mode value */
};

extern	struct	adsym	R8[];
extern	struct	adsym	R16[];
extern	struct	adsym	R16X[];
extern	struct	adsym	CND[];

	/* machine dependent functions */

#ifdef	OTHERSYSTEM
	
	/* gbadr.c */
extern	int		addr(struct expr *esp);
extern	int		admode(struct adsym *sp);
extern	int		any(int c, char *str);
extern	int		srch(char *str);

	/* gbmch.c */
extern	int		genop(int pop, int op, struct expr *esp, int f);
extern	VOID		machine(struct mne *mp);
extern	int		mchpcr(struct expr *esp);
extern	VOID		minit(void);

#else

	/* gbadr.c */
extern	int		addr();
extern	int		admode();
extern	int		any();
extern	int		srch();

	/* gbmch.c */
extern	int		genop();
extern	VOID		machine();
extern	int		mchpcr();
extern	VOID		minit();

#endif

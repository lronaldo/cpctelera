/* f8.h */

/*
 *  Copyright (C) 2010  Alan R. Baldwin
 *  Copyright (C) 2022  Philipp K. Krause
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

// Opcodes
#define OPCODE_SWAPOP 0x9c
#define OPCODE_ALTACC1 0x9d
#define OPCODE_ALTACC2 0x9e
#define OPCODE_ALTACC3 0x9f
#define OPCODE_ALTACC4 0x94
#define OPCODE_ALTACC5 0xd8

/*
 * Registers
 */
#define XL		 0
#define XH		 1
#define YL		 2
#define YH		 3
#define ZL		 4
#define ZH		 5
#define F		 7
#define SP		 8
#define X		 9
#define Y		10
#define Z		11

/*
 * Addressing Modes
 */
#define	S_IMM		0x00
#define	S_DIR		0x01
#define	S_SPREL		0x02
#define	S_ZREL		0x03
#define	S_REG		0x04
#define	S_IX		0x05
#define	S_YREL		0x06
#define	S_ISPREL	0x07

#define S_SHORT		0x08
#define S_LONG		0x09

/*
 * Instruction types
 */
enum insttype {
S_2OP = 60,
S_2OPSUB,
S_1OP,
S_1OPPUSH,
S_2OPW,
S_2OPWSUB,
S_2OPWSBC,
S_2OPWADD,
S_2OPWADC,
S_1OPW,
S_1OPWPUSH,
S_LD,
S_LDI,
S_LDW,
S_0OP,
S_0OPXCH,
S_0OPMAD,
S_0OPROT,
S_0OPMSK,
S_0OPCAX,
S_0OPW,
S_0OPWXCH,
S_0OPWSLL,
S_0OPWRLC,
S_0OPWCP,
S_0OPWDEC,
S_0OPWSEX,
S_0OPWCAX,
S_BIT,
S_JR,
S_JR2,
S_DNJNZ,
S_JP,
S_RET,
S_NOP,
S_TRAP
};

struct adsym
{
	char	a_str[4];	/* addressing string */
	int	a_val;		/* addressing mode value */
};

extern	struct	adsym	REG[];

extern	int	rcode;

	/* machine dependent functions */
	
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
extern	int		d_mode(struct expr *e);
extern	int		setbit(int b);
extern	int		getbit(void);
extern	void	altacc(int reg);
extern	void	altaccw(int reg);
extern	void	altaccw2(int reg0, int reg1);


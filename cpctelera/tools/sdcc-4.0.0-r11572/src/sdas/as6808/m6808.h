/* m6808.h */

/*
 *  Copyright (C) 1993-2009  Alan R. Baldwin
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

/*)BUILD
	$(PROGRAM) =	AS6808
	$(INCLUDE) = {
		ASXXXX.H
		M6808.H
	}
	$(FILES) = {
		M08MCH.C
		M08ADR.C
		M08PST.C
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

struct adsym
{
	char	a_str[4];	/* addressing string */
	int	a_val;		/* addressing mode value */
};

/*
 * Addressing types
 */
#define	S_IMMED	30
#define	S_DIR	31
#define	S_EXT	32
#define	S_IX	33
#define	S_IXP	34
#define	S_IX1	35
#define	S_IX1P	36
#define	S_IX2	37
#define	S_IX2P	38
#define	S_IS	39
#define	S_SP1	40
#define	S_SP2	41
#define	S_A	42
#define	S_X	43
#define	S_S	44
#define	S_XP	45

/*
 * Instruction types
 */
#define	S_INH	60
#define	S_BRA	61
#define	S_TYP1	62
#define	S_TYP2	63
#define	S_TYP3	64
#define	S_TYP4	65
#define	S_TYPAI	66
#define	S_TYPHX	67
#define	S_CBEQ	68
#define	S_CQAX	69
#define	S_DBNZ	70
#define	S_DZAX	71
#define	S_MOV	72

#define	S_INH8	73
#define	S_INH8S	74
#define	S_BRA8	75

/*
 * Special Types
 */
#define	S_SDP	80
#define	S_CPU	81

/*
 * Processor Types (S_CPU)
 */
#define	X_HC08	0
#define X_HCS08	1
#define	X_6805	2
#define	X_HC05	3




	/* machine dependent functions */

#ifdef	OTHERSYSTEM
	
	/* m08adr.c */
extern	struct	adsym	axs[];
extern	int		addr(struct expr *esp);
extern	int		admode(struct adsym *sp);
extern	int		any(int c, char *str);
extern	int		srch(char *str);

	/* m08mch.c */
extern	VOID		machine(struct mne *mp);
extern	int		mchpcr(struct expr *esp);
extern	VOID		minit(void);

#else

	/* m08adr.c */
extern	struct	adsym	axs[];
extern	int		addr();
extern	int		admode();
extern	int		any();
extern	int		srch();

	/* m08mch.c */
extern	VOID		machine();
extern	int		mchpcr();
extern	VOID		minit();

#endif

/* r6500.h */

/*
 *  Copyright (C) 1995-2022  Alan R. Baldwin
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

/*)BUILD
	$(PROGRAM) =	AS6500
	$(INCLUDE) = {
		ASXXXX.H
		R6500.H
	}
	$(FILES) = {
		R65MCH.C
		R65ADR.C
		R65PST.C
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
	char	a_str[2];	/* addressing string */
	int	a_val;		/* addressing mode value */
};

#define	S_A	1
#define	S_X	2
#define	S_Y	3

/*
 * Addressing types
 */
#define S_IMMED	40
#define S_ACC	41
#define S_DIR	42
#define S_EXT	43
#define S_IND	44
#define S_DINDX	45
#define S_DINDY	46
#define S_INDX	47
#define S_INDY	48
#define S_IPREX	49
#define S_IPSTY	50

/*
 * 650X and 651X Instructions
 */
#define S_INH1	60
#define S_BRA1	61
#define S_JSR	62
#define S_JMP	63
#define S_DOP	64
#define S_SOP	65
#define S_BIT	66
#define S_CP	67
#define S_LDSTX 68
#define S_LDSTY 69

/*
 * 65F11 and 65F12 Extensions
 */
#define S_BB	70
#define S_MB	71

/*
 * 65C00/21 and 6529 Extensions
 */
#define	S_BRA2	72
#define S_INH2	73
#define	S_INH3	74

/*
 * 65C02, 65C102, and 65C112 Extensions
 */
#define S_STZ	75
#define	S_TB	76

/*
 * machine dependent functions
 */
#define	S_SDP		80
#define	S_CPU		82

#define	X_R6500		31
#define	X_R65F11	32
#define X_R65C00	33
#define	X_R65C02	34

#ifdef	OTHERSYSTEM

	/* r65adr.c */
extern	struct	adsym	axy[];
extern	int		addr(struct expr *esp);
extern	int		admode(struct adsym *sp);
extern	int		srch(char *str);

	/* r65mch.c */
extern	struct  area	*zpg;
extern	VOID		machine(struct mne *mp);
extern	int		mchpcr(struct expr *esp);
extern	VOID		mcherr(int c, char *str);
extern	VOID		mchwrn(char *str);
extern	VOID		minit(void);

#else

	/* r65adr.c */
extern	struct	adsym	axy[];
extern	int		addr();
extern	int		admode();
extern	int		srch();

	/* r65mch.c */
extern	struct  area	*zpg;
extern	VOID		machine();
extern	int		mchpcr();
extern	VOID		mcherr();
extern	VOID		mchwrn();
extern	VOID		minit();

#endif


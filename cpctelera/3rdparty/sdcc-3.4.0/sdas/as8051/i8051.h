/* i8051.h */

/*
 *  Copyright (C) 1998-2009  Alan R. Baldwin
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
 *   This Assember Ported by
 *      John L. Hartman (JLH)
 *      jhartman at compuserve dot com
 *      noice at noicedebugger dot com
 *
 */

/*)BUILD
        $(PROGRAM) =    AS8051
        $(INCLUDE) = {
                ASXXXX.H
                I8051.H
        }
        $(FILES) = {
                I51MCH.C
                I51ADR.C
                I51PST.C
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
 * Symbol types.
 */
#define S_INH   50              /* One byte inherent */
#define S_JMP11 51              /* Jump and call 11 bit. */
#define S_JMP16 52              /* Jump and call 16 bit */
#define S_ACC   53              /* Accumulator */
#define S_TYP1  54              /* Type 1 (inc and dec) */
#define S_TYP2  55              /* Type 2 (arith ops) */
#define S_TYP3  56              /* Type 3 (logic ops) */
#define S_TYP4  57              /* Type 4 (XCH) */
#define S_MOV   58              /* MOV */
#define S_BITBR 59              /* bit branch */
#define S_BR    60              /* branch */
#define S_ACBIT 61              /* CLR, CPL */
#define S_CJNE  62              /* CJNE */
#define S_DJNZ  63              /* DJNZ */
#define S_JMP   64              /* JMP */
#define S_MOVC  65              /* MOVC */
#define S_MOVX  66              /* MOVX */
#define S_AB    67              /* AB (div and mul) */
#define S_CPL   68              /* CPL */
#define S_SETB  69              /* SETB */
#define S_DIRECT 70             /* DIRECT (pusha and pop) */
#define S_XCHD  71              /* XCHD */

/* Addressing modes */
#define S_A      30             /* A */
/* #define S_B   31 */          /* B */
#define S_C      32             /* C (carry) */
#define S_RAB    33             /* AB */
#define S_DPTR   34             /* DPTR */
#define S_REG    35             /* Register R0-R7 */
#define S_IMMED  36             /* immediate */
#define S_DIR    37             /* direct */
#define S_EXT    38             /* extended */
#define S_PC     39             /* PC (for addressing mode) */

#define S_AT_R   40             /* @R0 or @R1 */
#define S_AT_DP  41             /* @DPTR */
#define S_AT_APC 42             /* @A+PC */
#define S_AT_ADP 43             /* @A+DPTR */
#define S_NOT_BIT 44             /* /BIT (/DIR) */

/*
 * Registers.  Value  == address in RAM, except for PC
 */
#define R0      0
#define R1      1
#define R2      2
#define R3      3
#define R4      4
#define R5      5
#define R6      6
#define R7      7
#define A       0xE0
#define DPTR    0x82
#define PC      0xFF            /* dummy number for register ID only */
#define AB      0xFE            /* dummy number for register ID only */
#define C       0xFD            /* dummy number for register ID only */

struct adsym
{
        char    a_str[5];       /* addressing string (length for DPTR+null)*/
        int     a_val;          /* addressing mode value */
};

/* pre-defined symbol structure: name and value */
struct PreDef
{
   char *id;            /* ARB */
   int  value;
};
extern struct PreDef preDef[];

        /* machine dependent functions */

#ifdef  OTHERSYSTEM
        
        /* i51adr.c */
extern  struct  adsym   reg51[];
extern  int             addr(struct expr *esp);
extern  int             admode(struct adsym *sp);
extern  int             any(int c, char *str);
extern  int             srch(char *str);
extern  int             reg(void);

        /* i51mch.c */
extern  VOID            machine(struct mne *mp);
extern  int             mchpcr(struct expr *esp);
extern  VOID            minit(void);

#else

        /* i51adr.c */
extern  struct  adsym   reg51[];
extern  int             addr();
extern  int             admode();
extern  int             any();
extern  int             srch();
extern  int             reg();

        /* i51mch.c */
extern  VOID            machine();
extern  int             mchpcr();
extern  VOID            minit();

#endif

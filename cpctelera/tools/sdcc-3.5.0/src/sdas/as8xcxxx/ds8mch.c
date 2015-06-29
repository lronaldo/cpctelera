/* ds8mch.c */

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
 *      jhartman at compuserve dot com
 *      noice at noicedebugger dot com
 *
 *   Modified from i51pst.c
 *      Bill McKinnon
 *      w_mckinnon at conknet dot com
 *
 */

#include "asxxxx.h"
#include "ds8.h"

char    *cpu    = "Dallas Semiconductor [User Defined]";
char    *dsft   = "asm";

static int amode;
static char buff[80];
static int ds8_bytes;
static int mchtyp;

/*
 * Opcode Cycle Definitions
 */
#define OPCY_SDP        ((char) (0xFF))
#define OPCY_ERR        ((char) (0xFE))

/*      OPCY_NONE       ((char) (0x80)) */
/*      OPCY_MASK       ((char) (0x7F)) */

#define OPCY_CPU        ((char) (0xFD))
#define OPCY_AMODE      ((char) (0xFC))
#define OPCY_BITS       ((char) (0xFB))


#define UN      ((char) (OPCY_NONE | 0x00))

/*
 * ds8xcxxx Cycle Count
 *
 *      opcycles = ds8pg1[opcode]
 */
static char ds8pg1[256] = {
/*--*--* 0  1  2  3  4  5  6  7  8  9  A  B  C  D  E  F */
/*--*--* -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  - */
/*00*/   4,12,16, 4, 4, 8, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4,
/*10*/  12,12,16, 4, 4, 8, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4,
/*20*/  12,12,16, 4, 8, 8, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4,
/*30*/  12,12,16, 4, 8, 8, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4,
/*40*/  12,12, 8,12, 8, 8, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4,
/*50*/  12,12, 8,12, 8, 8, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4,
/*60*/  12,12, 8,12, 8, 8, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4,
/*70*/  12,12, 8,12, 8,12, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8,
/*80*/  12,12, 8,12,20,12, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8,
/*90*/  12,12, 8,12, 8, 8, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4,
/*A0*/   8,12, 8,12,20,UN, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8,
/*B0*/   8,12, 8, 4,16,16,16,16,16,16,16,16,16,16,16,16,
/*C0*/   8,12, 8, 4, 4, 8, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4,
/*D0*/   8,12, 8, 4, 4,16, 4, 4,12,12,12,12,12,12,12,12,
/*E0*/   8,12, 8, 8, 4, 8, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4,
/*F0*/   8,12, 8, 8, 4, 8, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4
};

/*
 * Process machine ops.
 */
VOID
machine(struct mne *mp)
{
        char *p, *str;
        char pid[NINPUT], id[NINPUT];
        int c, d, t, t1, v1;
        a_uint op;
        struct sym *sp;
        struct expr e, e1, e2;

        clrexpr(&e);
        clrexpr(&e1);
        clrexpr(&e2);

        op = mp->m_valu;
        switch (mp->m_type) {

        case S_CPU:
                opcycles = OPCY_CPU;
                lmode = SLIST;
                switch(op) {
                default: op = DS8XCXXX;
                case DS8XCXXX: v1 = 2; str = "DS8XCXXX"; sym[2].s_addr = X_DS8XCXXX; break;
                case DS80C310: v1 = 2; str = "DS80C310"; sym[2].s_addr = X_DS80C310; break;
                case DS80C320: v1 = 2; str = "DS80C320"; sym[2].s_addr = X_DS80C320; break;
                case DS80C323: v1 = 2; str = "DS80C323"; sym[2].s_addr = X_DS80C323; break;
                case DS80C390: v1 = 3; str = "DS80C390"; sym[2].s_addr = X_DS80C390; break;
                case DS83C520: v1 = 2; str = "DS83C520"; sym[2].s_addr = X_DS83C520; break;
                case DS83C530: v1 = 2; str = "DS83C530"; sym[2].s_addr = X_DS83C530; break;
                case DS83C550: v1 = 2; str = "DS83C550"; sym[2].s_addr = X_DS83C550; break;
                case DS87C520: v1 = 2; str = "DS87C520"; sym[2].s_addr = X_DS87C520; break;
                case DS87C530: v1 = 2; str = "DS87C530"; sym[2].s_addr = X_DS87C530; break;
                case DS87C550: v1 = 2; str = "DS87C550"; sym[2].s_addr = X_DS87C550; break;
                case DS______: v1 = 2; str = "DS______"; sym[2].s_addr = X_DS______;
                        if (more()) {
                                str = p = pid;
                                d = getnb();
                                while ((c = get()) != d) {
                                        if (c == '\0') {
                                                qerr();
                                        }
                                        if (p < &pid[sizeof(pid)-3]) {
                                                *p++ = c;
                                        } else {
                                                break;
                                        }
                                }
                                *p = 0;
                        }
                        break;
                }
                if (op != 0) {
                        ds8_bytes = v1;
                        exprmasks(v1);
                }
                mchtyp = (int) op;

                sprintf(id, "__%s", str);
                sp = lookup(id);
                if (sp->s_type != S_NEW && (sp->s_flag & S_ASG) == 0) {
                        err('m');
                }
                sp->s_type = S_USER;
                sp->s_addr = 1;
                sp->s_flag |= S_ASG;

                sprintf(buff, "%s %s", DS_CPU, str);
                cpu = buff;

                sp = lookup("__SFR_BITS");
                if (sp->s_type != S_NEW && (sp->s_flag & S_ASG) == 0) {
                        err('m');
                }
                sp->s_type = S_USER;
                sp->s_flag |= S_ASG;

                if (more()) {
                        expr(&e, 0);
                        abscheck(&e);
                        sp->s_addr = e.e_addr;
                } else {
                        sp->s_addr = 1;
                }
                break;

        case S_AMODE:
                opcycles = OPCY_AMODE;
                if ((mchtyp != 0) && (mchtyp != DS80C390)) {
                        err('o');
                        break;
                } else
                if ((mchtyp == 0) && ((a_bytes < 2) || (a_bytes > 3))) {
                        err('o');
                        break;
                }
                expr(&e, 0);
                abscheck(&e);
                amode = (int) e.e_addr;
                if ((amode < 0) || (amode > 2)) {
                        amode = 0;
                        err('o');
                }
                if ((c = getnb()) == ',') {
                        expr(&e1, 0);
                        abscheck(&e1);
                        if (e1.e_addr != 0) {
/* mov  ta,#0aah */             outab(0x075);   outab(0x0C7);   outab(0x0AA);
/* mov  ta,#055h */             outab(0x075);   outab(0x0C7);   outab(0x055);
/* mov  acon,#amode */          outab(0x075);   outab(0x09D);   outab(amode);
                        } else {
                                lmode = SLIST;
                        }
                } else {
                        unget(c);
                        lmode = SLIST;
                }
                break;

        case S_BITS:
                if (ds8_bytes == 0) {
                        ds8_bytes = (int) op;
                        exprmasks(ds8_bytes);
                } else
                if (ds8_bytes != (int) op) {
                        err('m');
                }
                opcycles = OPCY_BITS;
                lmode = SLIST;
                break;

        case S_INH:
                outab(op);
                break;

        case S_JMP11:
                expr(&e, 0);
                if (amode == 2) {
                        outr3bm(&e, R_J19, op);
                } else {
                        outrwm(&e, R_J11, op);
                }
                break;

        case S_JMP16:
                expr(&e, 0);
                outab(op);
                if (amode == 2) {
                        outr3b(&e, R_NORM);
                } else {
                        outrw(&e, R_NORM);
                }
                break;

        case S_ACC:
                t = addr(&e);
                if (t != S_A)
                        aerr();
                outab(op);
                break;

        case S_TYP1:
                /* A, direct, @R0, @R1, R0 to R7.  "INC" also allows DPTR */
                t = addr(&e);

                switch (t) {
                case S_A:
                        outab(op + 4);
                        break;

                case S_DIR:
                case S_EXT:
                        /* Direct is also legal */
                        outab(op + 5);
                        outrb(&e, R_PAG0);
                        break;

                case S_AT_R:
                        outab(op + 6 + e.e_addr);
                        break;

                case S_REG:
                        outab(op + 8 + e.e_addr);
                        break;

                case S_DPTR:
                        if (op != 0)
                                /* only INC (op=0) has DPTR mode */
                                aerr();
                        else
                                outab( 0xA3);
                        break;

                default:
                        aerr();
                }
                break;

        case S_TYP2:
                /* A,#imm; A,direct; A,@R0; A,@R1; A,R0 to A,R7 */
                t = addr(&e);
                if (t != S_A)
                        aerr();
                comma(1);
                t1 = addr(&e1);

                switch (t1) {
                case S_IMMED:
                        outab(op + 4);
                        outrb(&e1, R_NORM);
                        break;

                case S_DIR:
                case S_EXT:
                        outab(op + 5);
                        outrb(&e1, R_PAG0);
                        break;

                case S_AT_R:
                        outab(op + 6 + e1.e_addr);
                        break;

                case S_REG:
                        outab(op + 8 + (e1.e_addr));
                        break;

                default:
                        aerr();
                }
                break;

        case S_TYP3:
                /* dir,A; dir,#imm;
                 * A,#imm; A,direct; A,@R0; A,@R1; A,R0 to A,R7
                 * C,direct;  C,/direct
                 */
                t = addr(&e);
                comma(1);
                t1 = addr(&e1);

                switch (t) {
                case S_DIR:
                case S_EXT:
                        switch (t1) {
                        case S_A:
                                outab(op + 2);
                                outrb(&e, R_PAG0);
                                break;

                        case S_IMMED:
                                outab(op + 3);
                                outrb(&e, R_PAG0);
                                outrb(&e1, R_NORM);
                                break;

                        default:
                                aerr();
                        }
                        break;

                case S_A:
                        switch (t1) {
                        case S_IMMED:
                                outab(op + 4);
                                outrb(&e1, R_NORM);
                                break;

                        case S_DIR:
                        case S_EXT:
                                outab(op + 5);
                                outrb(&e1, R_PAG0);
                                break;

                        case S_AT_R:
                                outab(op + 6 + e1.e_addr);
                                break;

                        case S_REG:
                                outab(op + 8 + e1.e_addr);
                                break;

                        default:
                                aerr();
                        }
                        break;

                case S_C:
                        /* XRL has no boolean version.  Trap it */
                        if (op == 0x60)
                                aerr();

                        switch (t1) {
                        case S_DIR:
                        case S_EXT:
                                outab(op + 0x32);
                                outrb(&e1, R_PAG0);
                                break;

                        case S_NOT_BIT:
                                outab(op + 0x60);
                                outrb(&e1, R_PAG0);
                                break;

                        default:
                                aerr();
                        }
                        break;

                default:
                        aerr();
                }
                break;

        case S_TYP4:
                /* A,direct; A,@R0; A,@R1; A,R0 to A,R7 */
                t = addr(&e);
                if (t != S_A)
                        aerr();
                comma(1);
                t1 = addr(&e1);

                switch (t1) {
                case S_DIR:
                case S_EXT:
                        outab(op + 5);
                        outrb(&e1, R_PAG0);
                        break;

                case S_AT_R:
                        outab(op + 6 + e1.e_addr);
                        break;

                case S_REG:
                        outab(op + 8 + e1.e_addr);
                        break;

                default:
                        aerr();
                }
                break;

        /* MOV instruction, all modes */
        case S_MOV:
                t = addr(&e);
                comma(1);
                t1 = addr(&e1);

                switch (t) {
                case S_A:
                        switch (t1) {
                        case S_IMMED:
                                outab(0x74);
                                outrb(&e1, R_NORM);
                                break;

                        case S_DIR:
                        case S_EXT:
                                outab(0xE5);
                                outrb(&e1, R_PAG0);
                                break;

                        case S_AT_R:
                                outab(0xE6 + e1.e_addr);
                                break;

                        case S_REG:
                                outab(0xE8 + e1.e_addr);
                                break;

                        default:
                                aerr();
                        }
                        break;

                case S_REG:
                        switch (t1) {
                        case S_A:
                                outab(0xF8 + e.e_addr);
                                break;

                        case S_IMMED:
                                outab(0x78 + e.e_addr);
                                outrb(&e1, R_NORM);
                                break;

                        case S_DIR:
                        case S_EXT:
                                outab(0xA8 + e.e_addr);
                                outrb(&e1, R_PAG0);
                                break;

                        default:
                                aerr();
                        }
                        break;

                case S_DIR:
                case S_EXT:
                        switch (t1) {
                        case S_A:
                                outab(0xF5);
                                outrb(&e, R_PAG0);
                                break;

                        case S_IMMED:
                                outab(0x75);
                                outrb(&e, R_PAG0);
                                outrb(&e1, R_NORM);
                                break;

                        case S_DIR:
                        case S_EXT:
                                outab(0x85);
                                outrb(&e1, R_PAG0);
                                outrb(&e, R_PAG0);
                                break;

                        case S_AT_R:
                                outab(0x86 + e1.e_addr);
                                outrb(&e, R_PAG0);
                                break;

                        case S_REG:
                                outab(0x88 + e1.e_addr);
                                outrb(&e, R_PAG0);
                                break;

                        case S_C:
                                outab(0x92);
                                outrb(&e, R_PAG0);
                                break;

                        default:
                                aerr();
                        }
                        break;

                case S_AT_R:
                        switch (t1) {
                        case S_IMMED:
                                outab(0x76 + e.e_addr);
                                outrb(&e1, R_NORM);
                                break;

                        case S_DIR:
                        case S_EXT:
                                outab(0xA6 + e.e_addr);
                                outrb(&e1, R_PAG0);
                                break;

                        case S_A:
                                outab(0xF6 + e.e_addr);
                                break;

                        default:
                                aerr();
                        }
                        break;

                case S_C:
                        if ((t1 != S_DIR) && (t1 != S_EXT))
                                aerr();
                        outab(0xA2);
                        outrb(&e1, R_PAG0);
                        break;

                case S_DPTR:
                        if (t1 != S_IMMED)
                                aerr();
                        outab(0x90);
                        if (amode == 2)
                                outr3b(&e1, R_NORM);
                        else
                                outrw(&e1, R_NORM);
                        break;

                default:
                        aerr();
                }
                break;

        case S_BITBR:   /* JB, JBC, JNB bit,rel */
                /* Branch on bit set/clear */
                t = addr(&e);
                if ((t != S_DIR) && (t != S_EXT))
                        aerr();
                /* sdcc svn rev #4994: fixed bug 1865114 */
                comma(1);
                expr(&e1, 0);

                outab(op);
                outrb(&e, R_PAG0);

                if (mchpcr(&e1)) {
                        v1 = (int) (e1.e_addr - dot.s_addr - 1);
                        /* sdcc svn rev #602: Fix some path problems */
                        if (pass == 2 && ((v1 < -128) || (v1 > 127)))
                                aerr();
                        outab(v1);
                } else {
                        outrb(&e1, R_PCR);
                }
                if (e1.e_mode != S_USER)
                        rerr();
                break;

        case S_BR:  /* JC, JNC, JZ, JNZ */
                /* Relative branch */
                /* sdcc svn rev #4994: fixed bug 1865114 */
                expr(&e1, 0);
                outab(op);

                if (mchpcr(&e1)) {
                        v1 = (int) (e1.e_addr - dot.s_addr - 1);
                        /* sdcc svn rev #602: Fix some path problems */
                        if (pass == 2 && ((v1 < -128) || (v1 > 127)))
                                aerr();
                        outab(v1);
                } else {
                        outrb(&e1, R_PCR);
                }
                if (e1.e_mode != S_USER)
                        rerr();
                break;

        case S_CJNE:
                /* A,#;  A,dir;  @R0,#;  @R1,#;  Rn,# */
                t = addr(&e);
                comma(1);
                t1 = addr(&e1);

                /* Benny */
                comma(1);
                expr(&e2, 0);

                switch (t) {
                case S_A:
                        if (t1 == S_IMMED) {
                                outab(op + 4);
                                outrb(&e1, R_NORM);
                        }
                        else if ((t1 == S_DIR) || (t1 == S_EXT)) {
                                outab(op + 5);
                                outrb(&e1, R_PAG0);
                        }
                        else
                                aerr();
                        break;

                case S_AT_R:
                        outab(op + 6 + e.e_addr);
                        if (t1 != S_IMMED)
                                aerr();
                        outrb(&e1, R_NORM);
                        break;

                case S_REG:
                        outab(op + 8 + e.e_addr);
                        if (t1 != S_IMMED)
                                aerr();
                        outrb(&e1, R_NORM);
                        break;

                default:
                        aerr();
                        break;
                }

                /* branch destination */
                if (mchpcr(&e2)) {
                        v1 = (int) (e2.e_addr - dot.s_addr - 1);
                        /* sdcc svn rev #602: Fix some path problems */
                        if (pass == 2 && ((v1 < -128) || (v1 > 127)))
                                aerr();
                        outab(v1);
                } else {
                        outrb(&e2, R_PCR);
                }
                if (e2.e_mode != S_USER)
                        rerr();
                break;

        case S_DJNZ:
                /* Dir,dest;  Reg,dest */
                t = addr(&e);
                /* sdcc svn rev #4994: fixed bug 1865114 */
                comma(1);
                expr(&e1, 0);

                switch (t) {
                case S_DIR:
                case S_EXT:
                        outab(op + 5);
                        outrb(&e, R_PAG0);
                        break;

                case S_REG:
                        outab(op + 8 + e.e_addr);
                        break;

                default:
                        aerr();
                }

                /* branch destination */
                /* sdcc svn rev #4994: fixed bug 1865114 */
                if (mchpcr(&e1)) {
                        v1 = (int) (e1.e_addr - dot.s_addr - 1);
                        /* sdcc svn rev #602: Fix some path problems */
                        if (pass == 2 && ((v1 < -128) || (v1 > 127)))
                                aerr();
                        outab(v1);
                } else {
                        outrb(&e1, R_PCR);
                }
                if (e1.e_mode != S_USER)
                        rerr();
                break;

        case S_JMP:
                /* @A+DPTR */
                t = addr(&e);
                if (t != S_AT_ADP)
                        aerr();
                outab(op);
                break;

        case S_MOVC:
                /* A,@A+DPTR  A,@A+PC */
                t = addr(&e);
                if (t != S_A)
                        aerr();
                comma(1);
                t1 = addr(&e1);
                if (t1 == S_AT_ADP)
                        outab(0x93);
                else if (t1 == S_AT_APC)
                        outab(0x83);
                else
                        aerr();
                break;

        case S_MOVX:
                /* A,@DPTR  A,@R0  A,@R1  @DPTR,A  @R0,A  @R1,A */
                t = addr(&e);
                comma(1);
                t1 = addr(&e1);

                switch (t) {
                case S_A:
                        switch (t1) {
                        case S_AT_DP:
                                outab(0xE0);
                                break;

                        case S_AT_R:
                                outab(0xE2 + e1.e_addr);
                                break;

                        default:
                                aerr();
                        }
                        break;

                case S_AT_DP:
                        if (t1 == S_A)
                                outab(0xF0);
                        else
                                aerr();
                        break;

                case S_AT_R:
                        if (t1 == S_A)
                                outab(0xF2 + e.e_addr);
                        else
                                aerr();
                        break;

                default:
                        aerr();
                }
                break;

        /* MUL/DIV A,B */
        case S_AB:
                t = addr(&e);
                if (t != S_RAB)
                        aerr();
                outab(op);
                break;

        /* CLR or CPL:  A, C, or bit */
        case S_ACBIT:
                t = addr(&e);
                switch (t) {
                case S_A:
                        if (op == 0xB2)
                                outab(0xF4);
                        else
                                outab(0xE4);
                        break;

                case S_C:
                        outab(op+1);
                        break;

                case S_DIR:
                case S_EXT:
                        outab(op);
                        outrb(&e, R_PAG0);
                        break;

                default:
                        aerr();
                }
                break;

        /* SETB C or bit */
        case S_SETB:
                t = addr(&e);
                switch (t) {
                case S_C:
                        outab(op+1);
                        break;

                case S_DIR:
                case S_EXT:
                        outab(op);
                        outrb(&e, R_PAG0);
                        break;

                default:
                        aerr();
                }
                break;

        /* direct */
        case S_DIRECT:
                t = addr(&e);
                if (t == S_A) {
                        e.e_addr = 0xE0;
                        e.e_mode = S_DIR;
                } else
                if ((t != S_DIR) && (t != S_EXT)) {
                        aerr();
                        break;
                }
                outab(op);
                outrb(&e, R_PAG0);
                break;

        /* XCHD A,@Rn */
        case S_XCHD:
                t = addr(&e);
                if (t != S_A)
                        aerr();
                comma(1);
                t1 = addr(&e1);
                switch (t1) {
                case S_AT_R:
                        outab(op + e1.e_addr);
                        break;

                default:
                        aerr();
                }
                break;

        default:
                opcycles = OPCY_ERR;
                err('o');
                break;
        }
        if (opcycles == OPCY_NONE) {
                opcycles = ds8pg1[cb[0] & 0xFF];
        }
}

/*
 * Branch/Jump PCR Mode Check
 */
int
mchpcr(struct expr *esp)
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
 * Machine specific initialization
 */

VOID
minit(void)
{
        struct sym      *sp;
        struct PreDef   *pd;
        int i;
        char pid[8];
        char *p;

        /*
         * Byte Order
         */
        hilo = 1;

        amode = 0;
        /*
         * First time only:
         *      add the pre-defined symbols to the table
         *      as local symbols.
         */
        if (pass == 0) {
                ds8_bytes = 0;
                mchtyp = X_DS8XCXXX;
                sym[2].s_addr = X_DS8XCXXX;

                pd = preDef;
                while (pd->id) {
                        strcpy(pid, pd->id);
                        for (i=0; i<2; i++) {
                                /*
                                 * i == 0,  Create Upper Case Symbols
                                 * i == 1,  Create Lower Case Symbols
                                 */
                                if (i == 1) {
                                        p = pid;
                                        while (*p) {
                                                *p = ccase[*p & 0x007F];
                                                p++;
                                        }
                                }
                                sp = lookup(pid);
                                if (sp->s_type == S_NEW) {
                                        sp->s_addr = pd->value;
                                        sp->s_type = S_USER;
                                        sp->s_flag = S_LCL | S_ASG;
                                }
                        }
                        pd++;
                }
        }
}

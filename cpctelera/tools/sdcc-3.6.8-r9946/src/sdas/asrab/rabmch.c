/* rabmch.c */

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
 * 
 * ported to the Rabbit2000 by
 * Ulrich Raich and Razaq Ijoduola
 * PS Division
 * CERN
 * CH-1211 Geneva-23
 * email: Ulrich dot Raich at cern dot ch
 */

/*
 * Extensions: P. Felber
 *
 * Altered by Leland Morrison to support rabbit 2000 
 *   and rabbit 4000 instruction sets (2011)
 */

#include "asxxxx.h"
#include "rab.h"

char    *cpu    = "Rabbit 2000/4000";
char    *dsft   = "asm";

char    imtab[3] = { 0x46, 0x56, 0x5E };

int     r3k_mode;
int     r4k_mode;
/* when set, generate op-code for Rabbit-4000 instead of Rabbit 2000/3000 */

int     mchtyp;

/*
 * Opcode Cycle Definitions
 */
#define OPCY_SDP        ((char) (0xFF))
#define OPCY_ERR        ((char) (0xFE))

/*      OPCY_NONE       ((char) (0x80)) */
/*      OPCY_MASK       ((char) (0x7F)) */

#define OPCY_CPU        ((char) (0xFD))

#define UN      ((char) (OPCY_NONE | 0x00))
#define P2      ((char) (OPCY_NONE | 0x01))
#define P3      ((char) (OPCY_NONE | 0x02))
#define P4      ((char) (OPCY_NONE | 0x03))
#define P5      ((char) (OPCY_NONE | 0x04))
#define P6      ((char) (OPCY_NONE | 0x05))
#define P7      ((char) (OPCY_NONE | 0x06))

/*
 * Process a machine op.
 */
VOID  machine(struct mne * mp)
{
        int op, t1, t2;
        struct expr e1, e2;
        int rf, v1, v2;

        clrexpr(&e1);
        clrexpr(&e2);
        op = (int) mp->m_valu;
        rf = mp->m_type;

        if (!r4k_mode && rf > X_R4K_MODE)
                rf = 0;

        switch (rf) {
        case S_CPU:
                if (op == X_R2K)
                        r3k_mode=1;
                if (op == X_R4K)
                        r4k_mode=1;
                mchtyp = op;
                sym[2].s_addr = op;
                opcycles = OPCY_CPU;
                lmode = SLIST;
                break;

        case S_INH1:
                outab(op);
                break;

        case S_INH2:
                outab(0xED);
                outab(op);
                break;

        case S_RET:
                if (more()) {
                        if ((v1 = admode(CND)) != 0) {
                                outab(op | (v1<<3));
                        } else {
                                qerr();
                        }
                } else {
                        outab(0xC9);
                }
                break;

        case S_PUSH:
                if (admode(R16X)) {
                        outab(op+0x30);
                        break;
                } else if ((v1 = admode(R8IP)) != 0) {
                        outab(0xED);
                        if (op == 0xC5)
                                outab(0x76);  /* push */
                        else
                                outab(0x7E);  /* pop  */
                        break;
                } else
                if ((v1 = admode(R16)) != 0 && (v1 &= 0xFF) != SP) {
                        if (v1 != gixiy(v1)) {
                                outab(op+0x20);
                                break;
                        }
                        outab(op | (v1<<4));
                        break;
                } else if (r4k_mode) {
                        if ( (v1 = admode(R32_JKHL)) != 0 ) {
                                outab(JKHL_PG);
                                outab(op+0x30);
                                break;
                        } else if ( (v1 = admode(R32_BCDE)) != 0 ) {
                                outab(BCDE_PG);
                                outab(op+0x30);
                                break;
                        }
                }
                aerr();
                break;

        case S_RST:
                v1 = (int) absexpr();
                /* ljm comment -
                 *   block RST 00, 08, and 30 b/c those opcodes
                 *   are assigned to different instructions in the
                 *   rabbit processor
                 */
                if ((v1 == 0x00) || (v1 == 0x08) || (v1 == 0x30)) {
                        aerr( );
                        v1 = 0;
                }
                if (v1 & ~0x38) {
                        aerr();
                        v1 = 0;
                }
                outab(op|v1);
                break;

#if 0
        /* IM x set interrupt mode on Z-80 */
        /* Rabbit processor use the opcode to set interrupt level */
        case S_IM:
                expr(&e1, 0);
                abscheck(&e1);
                if (e1.e_addr > 2) {
                        aerr();
                        e1.e_addr = 0;
                }
                outab(op);
                outab(imtab[(int) e1.e_addr]);
                break;
#endif

        case S_BIT:
                expr(&e1, 0);
                t1 = 0;
                v1 = (int) e1.e_addr;
                if (v1 > 7) {
                        v1 &= 0x07;
                        ++t1;
                }
                op |= (v1<<3);
                comma(1);
                addr(&e2);
                abscheck(&e1);
                if (genop(0xCB, op, &e2, 0) || t1)
                        aerr();
                break;

        case S_RL:
                t1 = 0;
                t2 = addr(&e2);
                if ((t2 == S_IMMED) && r4k_mode)
                {
                        v1 = (int) e2.e_addr;
                        /* v1 should be shift count of 1,2,4, or 8 */
                        comma(1);
                        clrexpr(&e2);
                        t2 = addr(&e2);

                        if ((t2 != S_R32_BCDE) && (t2 != S_R32_JKHL))
                                aerr( );

                        if (v1 == 1)
                                v1 = 0x48;
                        else if (v1 == 2)
                                v1 = 0x49;
                        else if (v1 == 4)
                                v1 = 0x4B;
                        else if ((v1 == 8) && (op < 0x20 /* op is rlc|rrc|rl|rr */))
                                v1 = 0x4F;
                        else {
                                err('o');
                                break;
                        }

                        /* 00 rlc, 08 rrc, 10 rl , 18 rr                    *
                         * 20 sla, 28 sra,         38 srl,  [30 sll == sla] */
                        outab( ((t2 == S_R32_JKHL)?JKHL_PG:BCDE_PG) );
                        outab(v1 + (op << 1));
                        break;
                }
                else if (more()) {
                        if ((t2 != S_R8) || (e2.e_addr != A))
                                ++t1;
                        comma(1);
                        clrexpr(&e2);
                        t2 = addr(&e2);
                } else if (t2 == S_R16) {
                        v2 = (int) e2.e_addr;
                        if ((v2 == DE) && 
                            ((op == 0x10 /* rl */) || (op == 0x18 /* rr */))) {
                                outab( 0xF3 - 0x10 + op );
                                break;
                        }

                        if ((v2 == HL) && (op == 0x18 /* rr */)) {
                                outab( 0xFC );
                                break;
                        }

                        if (r4k_mode) {
                                if ((v2 == HL) && (op == 0x10 /* rl */)) {
                                        outab( 0x42 );
                                        break;
                                }
                                if (((v2 == BC)||(v2 == DE)) &&
                                    (op < 0x20 /* 00 rlc, 08 rrc, 10 rl, 18 rr */)) {
                                        outab( 0x50 + (op >> 3) + ((v2==BC)?0x10:0x00) );
                                        break;
                                }
                        }

                        aerr( );
                }
                if (genop(0xCB, op, &e2, 0) || t1)
                        aerr();
                break;

        case S_AND:  /* and, xor, or, cp */
        case S_SUB:  /* sub */
        case S_SBC:  /* sbc */
                t1 = addr(&e1);
#if 0
                if (!(more())) {
                        /* handle case for implicit target of 'A' register */
                        /* TODO */
                        aerr( );
                }
#endif
                comma(1);
                t2 = addr(&e2);

                v1 = (int) e1.e_addr;
                v2 = (int) e2.e_addr;

                if ((t1 == S_R8) && (v1 == A)) {
                        if ( ((t2 == S_R8) && (v2 == A)) &&
                             ((op == 0xA8) || (op == 0xB0)) ) {
                                /* AF: "xor a,a"  ||  B7: "or a,a" */
                                outab( op | 0x07 );
                                break;
                        }

                        if ((t2 == S_R8) || (t2 == S_IDHL)) {
                                /* ljm - rabbit 4000 support
                                 * (sub,sbc,and,xor,or,cp) A,R  or A,(HL)
                                 * needs a 0x7F prefix byte when
                                 * operating in rabbit 4000 mode
                                 */
                                if (r4k_mode)
                                        outab(0x7F);
                        }

#if 0
                        if (t2 == S_IMMED) {  /* AND,XOR,OR,CP,SUB A, #n */
                                /* opcode for (sub,sbc,and,xor,or,cp) A,#immediate 
                                 * do not need 0x7F prefix byte
                                 */
                                outab(op|0x46);  /* 0xA0 | 0x46 => 0xE6, etc */
                                outrb(&e2, 0);
                        } else
#endif

                        if (genop(0, op, &e2, 1))
                                aerr();
                        break;
                }

                if ((t1 == S_R16) && (v1 == HL) &&
                    (t2 == S_R16) && (rf == S_SBC)) {
                        /* sbc  hl, [bc|de|hl|sp] */
                        if ( v2 != gixiy(v2) )
                                /* sorry, sbc hl, [ix|iy] do not exist */
                                aerr( );

                        outab(0xED);
                        outab(0x42 | v2 << 4);
                        break;
                }
                if ((t1 == S_R16) && (v1 == HL) &&
                    (t2 == S_R16) && (v2 == DE)) {
                        /*  55     sub  hl, de */
                        /*         sbc  hl, de  does not exist */
                        /*  DC     and  hl, de */
                        /*  54     xor  hl, de */
                        /*  EC     or   hl, de */
                        /*  ED 48  cp   hl, de */
                        if (rf == S_SBC) /* op == 0x98 */
                                aerr( );

                        switch( op ) {
                        case 0x90:  /* sub */ outab(0x55); break;
                        case 0xA0:  /* and */ outab(0xDC); break;
                        case 0xA8:  /* xor */ outab(0x54); break;
                        case 0xB0:  /* or  */ outab(0xEC); break;
                        case 0xB8:  /* cp  */
                                outab( 0xED );
                                outab( 0x48 );
                                break;
                        }
                        break;
                }

                if ((t1 == S_R16) && ((v1 == IX) || (v1 == IY)) &&
                    (t2 == S_R16) && (v2 == DE) &&
                    ((op == 0xA0 /* and */) || (op == 0xB0 /* or */))) {
                        v1 = gixiy(v1);
                        outab(op + 0x3C);
                        break;
                }

                if ((t1 == S_R32_JKHL) && (t2 == S_R32_BCDE)) {
                        /* ED D6   sub  jkhl, bcde  */
                        /*         sbc  jkhl, bcde does not exist */
                        /* ED E6   and  jkhl, bcde  */
                        /* ED EE   xor  jkhl, bcde  */
                        /* ED F6   or   jkhl, bcde  */
                        /* ED 58   cp   jkhl, bcde  */
                        if (rf == S_SBC) /* op == 0x98 */
                                aerr( );

                        outab(0xED);
                        switch( op ) {
                        case 0x90:  /* sub */ outab(0xD6); break;
                        case 0xA0:  /* and */ outab(0xE6); break;
                        case 0xA8:  /* xor */ outab(0xEE); break;
                        case 0xB0:  /* or  */ outab(0xF6); break;
                        case 0xB8:  /* cp  */ outab(0x58); break;
                        }
                        break;
                }

                if ((t1 == S_R16) && (v1 == HL) && (t2 == S_IMMED)) {
                        /* cp  hl, #signed displacement */
                        outab(0x48);
                        outrb(&e2, 0);
                        break;
                }

                aerr( );
                break;

        case S_ADD:
        case S_ADC:
                t1 = addr(&e1);
                t2 = 0;
                if (more()) {
                        comma(1);
                        t2 = addr(&e2);
                }
                if (t2 == 0) {
                        /* implied destination of the 8-bit 'a' register */
                        if ((t1 == S_R8) || (t1 == S_IDHL)) {
                                /* ljm - rabbit 4000 support
                                 * (add,adc,sub,sbc,and,xor,or,cp) A,R  or A,(HL)
                                 * needs a 0x7F prefix byte when
                                 * operating in rabbit 4000 mode
                                 */
                                if (r4k_mode)
                                        outab(0x7F);
                        }

                        if (genop(0, op, &e1, 1))
                                aerr();
                        break;
                }
                if ((t1 == S_R8) && (e1.e_addr == A)) {
                        if ( ((t2 == S_R8) || (t2 == S_IDHL)) && r4k_mode )
                                /* ljm - rabbit 4000 support, see note in t2==0 */
                                outab(0x7F);

                        if (genop(0, op, &e2, 1))
                                aerr();
                        break;
                }

                if ((t1 == S_R32_JKHL) && (t2 == S_R32_BCDE) &&
                    (rf == S_ADD)) {
                        /* rabbit 4000 - ED C6   "add  jkhl, bcde"  */
                        outab(0xED);
                        outab(0xC6);
                        break;
                }

                v1 = (int) e1.e_addr;
                v2 = (int) e2.e_addr;

                if ((t1 == S_R16) && (v1 == SP) && (t2 == S_IMMED)) {
                        /* rabbit 4000 - add sp,#n  n=signed displacement */
                        outab(0x27);
                        outrb(&e2, 0);
                        break;
                }

                if ((t1 == S_R16) && (v1 == HL) && (t2 == S_R16)) {
                        if (v2 > SP)
                                aerr( );

                        if (rf == S_ADC)
                                outab(0xED);

                        op = (rf == S_ADD) ? 0x09 : 0x4A;
                        outab(op | (v2 << 4) );
                        break;
                }

                if ((t1 == S_R16) && ((v1 == IX) || (v1 == IY)) &&
                    (t2 == S_R16))
                {
                        if ((v2 == HL) ||
                            (((v2 == IX) || (v2 == IY)) && (v2 != v1)))
                                aerr( );

                        if ((v2 == IX) || (v2 == IY))
                                v2 = HL;

                        gixiy(v1);
                        outab(0x09 | (v2 << 4));
                        break;
                }
                aerr();
                break;

        case S_LD:
                t1 = addr(&e1);
                v1 = (int) e1.e_addr;
                comma(1);
                t2 = addr(&e2);
                v2 = (int) e2.e_addr;
                if (t1 == S_R8)
                {
                        if (t2 == S_IMMED) {
                                outab((e1.e_addr<<3) | 0x06);
                                outrb(&e2, 0);
                                break;
                        }

                        if (r4k_mode && (v1 == A) && (t2 == S_R8) && (v2 == A)) {
                                /* exception for "ld a,a" 
                                 * on rabbit 4000 0x7F is a prefix instead of "ld a,a"
                                 */
                                aerr( );
                        }

                        if ((v1 == A) && (t2 == S_R8)) {
                                /* "ld  a,r", (except "ld a,a") */
                                v1 = op | e1.e_addr<<3;
                                if (genop(0, v1, &e2, 0))
                                        aerr( );
                                break;
                        }

                        /* ld [b,c,d,e,h,l,a], _ */
                        if ((t2 == S_R8) && (v2 != A)) {
                                /* 8-bit register to 8-bit register */
                                /* use 0x7F prefix when in rabbit 4000 mode */
                                v1 = op | e1.e_addr<<3;
                                if (r4k_mode)
                                        outab(0x7F);
                                if (genop(0, v1, &e2, 0) == 0)
                                        break;

                                aerr( );
                        }

                        if ((t2 == S_R8) && (v2 == A) &&
                            ((v1 != A) || (!r4k_mode))) {
                                /* "ld  r,a", but except "ld a,a" 
                                 * on rabbit 4000 0x7F is a prefix instead of "ld a,a" */
                                v1 = op | e1.e_addr<<3;
                                if (genop(0, v1, &e2, 0))
                                        aerr( );
                                break;
                        }

                        if ((t2 == S_IDHL) || (t2 == S_IDIX) || (t2 == S_IDIY)) {
                                /* "ld r,(hl)" or "ld r,disp (ix|iy)" */
                                v1 = op | e1.e_addr<<3;
                                if (genop(0, v1, &e2, 0))
                                        aerr( );
                                break;
                        }
                }
      
                if ((t1 == S_R16) && (t2 == S_IMMED)) {
                        v1 = gixiy(v1);  /* generayes prefix when ix or iy */
                        outab(0x01|(v1<<4));
                        outrw(&e2, 0);
                        break;
                }
                if ((t1 == S_R16) && (t2 == S_INDM)) {
                        if (gixiy(v1) == HL) {
                                outab(0x2A);
                        } else {
                                outab(0xED);
                                outab(0x4B | (v1<<4));
                        }
                        outrw(&e2, 0);
                        break;
                }
                if ((t1 == S_R16) && (v1 == HL))
                {
                        if ((t2 == S_IDIX) || (t2 == S_IDIY) ||
                            (t2 == S_IDHL) || (t2 == S_IDHL_OFFSET))
                        {
                                /* ljm - added rabbit instruction LD HL,n(IX|HL|IY) */
                                if (t2 == S_IDIY)
                                        outab(0xFD);
                                else if ((t2 == S_IDHL) || (t2 == S_IDHL_OFFSET))
                                        /* ljm - added rabbit instruction LD HL,n(IY)
                                         * normally 0xFD generated by "gixiy(v1)", but
                                         * 0xDD results in n(HL) instead of n(IX)
                                         */
                                        outab(0xDD);

                                outab(0xE4);
                                outrb(&e2, 0);
                                break;
                        }
                        if ((t2 == S_R16) && ((v2 == IX) || (v2 == IY))) {
                                outab( ((v2==IX)?0xDD:0xFD) );
                                outab(0x7C);
                                break;
                        }
                        if (r4k_mode) {
                                if ((t2 == S_R16) && ((v2 == BC) || (v2 == DE))) {
                                        outab( 0x81 + ((v2 == DE) ? 0x20 : 0) );
                                        break;
                                }
                        }
                }
                if ((t2 == S_R16) && (v2 == HL)) /* ld n(IX|IY|HL), HL */
                {
                        if ((t1 == S_IDIY) || (t1 == S_IDHL) ||
                            (t1 == S_IDHL_OFFSET))
                                outab( ((t1==S_IDIY) ? 0xFD : 0xDD) );

                        if ((t1 == S_IDIY) || (t1 == S_IDIX) ||
                            (t1 == S_IDHL) || (t1 == S_IDHL_OFFSET)) {
                                outab(0xF4);
                                outrb(&e2, 0);
                                break;
                        }

                        if ((t1 == S_R16) && ((v1 == IX) || (v1 == IY))) {
                                outab( ((v1==IX)?0xDD:0xFD) );
                                outab(0x7D);
                                break;
                        }
                }
                if ((t1 == S_INDM) && (t2 == S_R16)) {
                        if (gixiy(v2) == HL) {
                                outab(0x22);
                        } else {
                                outab(0xED);
                                outab(0x43 | (v2<<4));
                        }
                        outrw(&e1, 0);
                        break;
                }
                if ((t1 == S_R8) && (v1 == A) && (t2 == S_INDM)) {
                        outab(0x3A);
                        outrw(&e2, 0);
                        break;
                }
                if ((t1 == S_INDM) && (t2 == S_R8) && (v2 == A)) {
                        outab(0x32);
                        outrw(&e1, 0);
                        break;
                }
                if ((t2 == S_R8) && (gixiy(t1) == S_IDHL)) {
                        outab(0x70|v2);
                        if (t1 != S_IDHL)
                                outrb(&e1, 0);
                        break;
                }
                if ((t2 == S_IMMED) && (gixiy(t1) == S_IDHL)) {
                        outab(0x36);
                        if (t1 != S_IDHL)
                                outrb(&e1, 0);
                        outrb(&e2, 0);
                        break;
                }
                if ((t1 == S_R8X) && (t2 == S_R8) && (v2 == A)) {
                        outab(0xED);
                        outab(v1);
                        break;
                }
                if ((t1 == S_R8) && (v1 == A) && (t2 == S_R8X)) {
                        outab(0xED);
                        outab(v2|0x10);
                        break;
                }
                if ((t1 == S_R16) && (v1 == SP)) {
                        if ((t2 == S_R16) && (gixiy(v2) == HL)) {
                                outab(0xF9);
                                break;
                        }
                }
                if ((t1 == S_R16) && (t2 == S_IDSP))
                {
                        if ( (v1=gixiy(v1)) == HL ) {
                                /* ljm - added rabbit instruction:
                                 * LD HL|IX|IY, n(SP)
                                 */
                                outab(0xC4);
                                outrb(&e2, 0);
                                break;
                        }
                }
      
                if ((t1 == S_IDSP) && (t2 == S_R16))
                {
                        //printf( "at %s: %d, t1=%d, v1=%d, t2=%d, v2=%d\n",
                        //  __FILE__, __LINE__, t1, v1, t2, v2 );
                        if ( (v2=gixiy(v2)) == HL ) {
                                /* ljm - added rabbit instruction:
                                 * LD HL|IX|IY, n(SP)
                                 */
                                outab(0xD4);
                                outrb(&e1, 0);
                                break;
                        }
                }
                if ((t1 == S_R8) && (v1 == A)) {
                        if ((t2 == S_IDBC) || (t2 == S_IDDE)) {
                                outab(0x0A | ((t2-S_INDR)<<4));
                                break;
                        }
                }
                if ((t2 == S_R8) && (v2 == A)) {
                        if ((t1 == S_IDBC) || (t1 == S_IDDE)) {
                                outab(0x02 | ((t1-S_INDR)<<4));
                                break;
                        }
                }
      
                /* load/save code bank register "xpc" */
                if ((t1 == S_RXPC) && (t2 == S_R8) && (v2 == A)) {
                        outab(0xED);
                        outab(0x67);
                        break;
                }
      
                if ((t1 == S_RXPC) && r4k_mode &&
                    (t2 == S_R16) && (v2 == HL)) {
                        outab(0x97);
                        break;
                }
      
                if ((t2 == S_RXPC) && (t1 == S_R8) && (v1 == A)) {
                        outab(0xED);
                        outab(0x77);
                        break;
                }
      
                if ((t2 == S_RXPC) && r4k_mode &&
                    (t1 == S_R16) && (v1 == HL)) {
                        outab(0x9F);
                        break;
                }
      
                /* 16-bit operations valid only in rabbit 4000 mode */
                if (r4k_mode && (t1 == S_R16) && (t2 == S_R16)) {
                        if ((v1 == HL) && ((v2 == BC) || (v2 == DE))) {
                                outab( 0x81 + ((v2==DE)?0x20:0x00) );
                                break;
                        }
                        if ((v2 == HL) && ((v1 == BC) || (v1 == DE))) {
                                outab( 0x91 + ((v1==DE)?0x20:0x00) );
                                break;
                        }
                }
      
                /* 32-bit operations valid in rabbit 4000 mode */
                if (r4k_mode && ((t1 == S_R32_JKHL) || (t1 == S_R32_BCDE))) {
                        if (t2 == S_IDHL) {
                                outab( ((t1 == S_R32_JKHL)?JKHL_PG:BCDE_PG) );
                                outab( 0x1A );
                                break;
                        }
                        if ((t2 == S_IDIX) || (t2 == S_IDIY) || (t2 == S_IDSP)) {
                                outab( ((t1 == S_R32_JKHL)?JKHL_PG:BCDE_PG) );
                                if (t2 == S_IDSP)
                                        v2 = 0x20;
                                else 
                                        v2 = ((t2 == S_IDIY) ? 0x10 : 0x00);
          
                                outab( 0xCE + v2 );
                                outrb(&e2, 0);
                                break;
                        }
                        if (t2 == S_INDM) {
                                outab( 0x93 + ((t1 == S_R32_JKHL) ? 1 : 0) );
                                outrw(&e2, 0);
                                break;
                        }
                        if (t2 == S_IMMED) {
                                outab( 0xA3 + ((t1 == S_R32_JKHL) ? 1 : 0) );
                                outrb(&e2, 0);
                                break;
                        }
                }

                if (r4k_mode && ((t2 == S_R32_JKHL) || (t2 == S_R32_BCDE))) {
                        if (t1 == S_IDHL) {
                                outab( ((t2 == S_R32_JKHL)?JKHL_PG:BCDE_PG) );
                                outab( 0x1B );
                                break;
                        }
                        if ((t1 == S_IDIX) || (t1 == S_IDIY) || (t1 == S_IDSP)) {
                                outab( ((t2 == S_R32_JKHL)?JKHL_PG:BCDE_PG) );
                                if (t1 == S_IDSP)
                                        v1 = 0x20;
                                else
                                        v1 = ((t1 == S_IDIY) ? 0x10 : 0x00);

                                outab( 0xCF + v1 );
                                outrb(&e1, 0);
                                break;
                        }
                        if (t1 == S_INDM) {
                                outab( 0x83 + ((t2 == S_R32_JKHL) ? 1 : 0) );
                                outrw(&e1, 0);
                                break;
                        }
                }
                aerr();
                break;
      
        case S_EX:
                t1 = addr(&e1);
                comma(1);
                t2 = addr(&e2);
                if (t2 == S_R16) {
                        v1 = (int) e1.e_addr;
                        v2 = (int) e2.e_addr;
                        if (t1 == S_R16) {
                                if ((v1 == DE) && (v2 == HL)) {
                                        outab(0xEB);
                                        break;
                                }
                                if (r4k_mode && (v1==BC) && (v2==HL)) {
                                        outab(0xB3);
                                        break;
                                }
                        }
        
                        if ((t1 == S_IDSP) && (v1 == 0) && (v2!=HL)) {
                                /* 0xE3 is EX DE',HL on rabbit 2000 
                                 * but DD/FD E3 "ex (sp),ix|iy" is valid
                                 */
                                if (gixiy(v2) == HL) {
                                        outab(op);
                                        break;
                                }
                        }
                }
                if ((t1 == S_R16X) && (t2 == S_R16X)) {
                        outab(0x08);
                        break;
                }
                if ((t1==S_R32_JKHL) && (t2==S_R32_BCDE)) {
                        outab(0xB4);
                        break;
                }
                aerr();
                break;
      
        case S_IN:
        case S_OUT:
                outab(op);
                break;
      
        case S_DEC:
        case S_INC:
                t1 = addr(&e1);
                v1 = (int) e1.e_addr;
                if (t1 == S_R8) {
                        outab(op|(v1<<3));
                        break;
                }
                if (t1 == S_IDHL) {
                        outab(op|0x30);
                        break;
                }
                if (t1 != gixiy(t1)) {
                        outab(op|0x30);
                        outrb(&e1, 0);
                        break;
                }
                if (t1 == S_R16) {
                        v1 = gixiy(v1);
                        if (rf == S_INC) {
                                outab(0x03|(v1<<4));
                                break;
                        }
                        if (rf == S_DEC) {
                                outab(0x0B|(v1<<4));
                                break;
                        }
                }
                aerr();
                break;
      
        case S_NEG:
                if (!more()) {
                        /* "neg" implies "neg a" */
                        outab(0xED);
                        outab(op);
                        break;
                }
                t1 = addr(&e1);
                v1 = (int) e1.e_addr;
                if ((t1 == S_R8) && (v1 == A)) { /* "neg a" */
                        outab(0xED);
                        outab(op);
                        break;
                }
      
                if ((t1 == S_R16) && (v1 == HL) && r4k_mode) { /* "neg hl" */
                        outab(0x4D);
                        break;
                }
      
                if (r4k_mode &&
                    ((t1 == S_R32_JKHL) || (t1 == S_R32_BCDE))) {
                        /* neg jkhl|bcde */
                        outab( ( (t1 == S_R32_BCDE) ? 0xDD : 0xFD ) );
                        outab(0x4D);
                        break;
                }
                break;
      
        case S_DJNZ:
        case S_JR:
                if ((v1 = admode(CND)) != 0 && rf != S_DJNZ) {
                        if ((v1 &= 0xFF) <= 0x03) {
                                op += (v1+1)<<3;
                        } else {
                                aerr();
                        }
                        comma(1);
                }
                expr(&e2, 0);
                outab(op);
                if (mchpcr(&e2)) {
                        v2 = (int) (e2.e_addr - dot.s_addr - 1);
                        if (pass == 2 && ((v2 < -128) || (v2 > 127)))
                                aerr();
                        outab(v2);
                } else {
                        outrb(&e2, R_PCR);
                }
                if (e2.e_mode != S_USER)
                        rerr();
                break;
      
        case S_CALL:
                op = 0xCD;
                expr(&e1, 0);
                outab(op);
                outrw(&e1, 0);
                break;
      
        case S_JP:
                if ((v1 = admode(CND)) != 0) {
                        op |= (v1&0xFF)<<3;
                        comma(1);
                        expr(&e1, 0);
                        outab(op);
                        outrw(&e1, 0);
                        break;
                }
                t1 = addr(&e1);
                if (t1 == S_USER) {
                        outab(0xC3);
                        outrw(&e1, 0);
                        break;
                }
                if ((e1.e_addr == 0) && (gixiy(t1) == S_IDHL)) {
                        outab(0xE9);
                        break;
                }
                aerr();
                break;
/*
    case X_HD64:
      ++hd64;
      break;
*/
        case HD_INH2:
                outab(0xED);
                outab(op);
                break;
      
        case HD_IN:
        case HD_OUT:
                if (rf == HD_IN) {
                        t1 = addr(&e1);
                        comma(1);
                        t2 = addr(&e2);
                } else {
                        t2 = addr(&e2);
                        comma(1);
                        t1 = addr(&e1);
                }
                if ((t1 == S_R8) && (t2 == S_INDM)) {
                        outab(0xED);
                        outab(op | (e1.e_addr<<3));
                        outrb(&e2, 0);
                        break;
                }
                aerr();
                break;
      
        case HD_MLT:
                t1 = addr(&e1);
                if ((t1 == S_R16) && ((v1 = (int) e1.e_addr) <= SP)) {
                        outab(0xED);
                        outab(op | (v1<<4));
                        break;
                }
                aerr();
                break;
      
        case HD_TST:
                t1 = addr(&e1);
                if (t1 == S_R8) {
                        outab(0xED);
                        outab(op | (e1.e_addr<<3));
                        break;
                }
                if (t1 == S_IDHL) {
                        outab(0xED);
                        outab(0x34);
                        break;
                }
                if (t1 == S_IMMED) {
                        outab(0xED);
                        outab(0x64);
                        outrb(&e1, 0);
                        break;
                }
                aerr();
                break;
      
        case HD_TSTIO:
                t1 = addr(&e1);
                if (t1 == S_IMMED) {
                        outab(0xED);
                        outab(op);
                        outrb(&e1, 0);
                        break;
                }
                aerr();
                break;
      
        case X_LJP:
        case X_LCALL:
                /* bank jump or call for rabbit processor */
                t1 = addr(&e1);
                comma(1);
                t2 = addr(&e2);
                v1 = (int) e1.e_addr;
                if ((t1 == S_USER) && (t2 == S_IMMED)) {
                        outab(op);
                        outrw(&e1, 0);
                        outrb(&e2, 0);
                        break;
                }
                break;
      
        case X_BOOL:
                t1 = addr(&e1);
                v1 = (int) e1.e_addr;
                if ((t1 == S_R16) && ((v1 == HL) || (v1 == IX) || (v1 == IY))) {
                        v1 = gixiy(v1);
                        outab(op);
                        break;
                }
                aerr( );
                break;

        case R3K_INH1:
                if (!(r3k_mode || r4k_mode))
                        err('o');
      
                outab(op);
                break;
      
        case R3K_INH2:
                if (!(r3k_mode || r4k_mode))
                        err('o');
      
                outab(0xED);
                outab(op);
                break;

        case R4K_INH2:
                if (!r4k_mode)
                        err('o');
      
                outab(0xED);
                outab(op);
                break;
      
        case X_R4K_MULU:
                if (!r4k_mode)
                        err('o');
      
                outab(op);
                break;
      
        case X_JRE:
                if (!r4k_mode)
                        err('o');
      
                if ((v1 = admode(ALT_CND)) != 0) {
                        op += v1<<3;
                        comma(1);
                } else {
                        op = 0x98;
                }
                expr(&e2, 0);
                outab(op);
                if (mchpcr(&e2)) {
                        v2 = (int) (e2.e_addr - dot.s_addr - 1);
                        if (pass == 2 && ((v2 < -32768) || (v2 > 32767)))
                                aerr();
                        outab( (v2 & 0xFF) );
                        outab( (v2 >> 8) );
                } else {
                        outrb(&e2, R_PCR);
                }
                if (e2.e_mode != S_USER)
                        rerr();
                break;
      
        case X_CLR:
                if (!r4k_mode)
                        err('o');
                t1 = addr(&e1);
                v1 = (int) e1.e_addr;
                if ((t1 == S_R16) && (v1 == HL)) {
                        outab(op);
                        break;
                }
                aerr( );
                break;

        default:
                err('o');
        }
}

/*
 * general addressing evaluation
 * return(0) if general addressing mode output, else
 * return(esp->e_mode)
 */
int
genop(int pop, int op, struct expr *esp, int f)
{
        int t1;
        /*
         * r
         */
        if ((t1 = esp->e_mode) == S_R8) {
                if (pop)
                        outab(pop);
                outab(op|esp->e_addr);
                return(0);
        }
        /*
         * (hl)
         */
        if (t1 == S_IDHL) {
                if ((esp->e_base.e_ap != NULL) || (esp->e_addr != 0))
                        aerr();
                if (pop)
                        outab(pop);
                outab(op|0x06);
                return(0);
        }
        /*
         * (ix) / (ix+d)
         * (iy) / (iy+d)
         */
        if (gixiy(t1) == S_IDHL) {
                if (pop) {
                        outab(pop);
                        outrb(esp, 0);
                        outab(op|0x06);
                } else {
                        outab(op|0x06);
                        outrb(esp, 0);
                }
                return(0);
        }
        /*
         *  n
         * #n
         */
        if ((t1 == S_IMMED) && (f)) {
                if (pop)
                        outab(pop);
                outab(op|0x46);
                outrb(esp,0);
                return(0);
        }
        return(t1);
}

/*
 * IX and IY prebyte check
 */
int
gixiy(int v)
{
        if (v == IX) {
                v = HL;
                outab(0xDD);
        } else if (v == IY) {
                v = HL;
                outab(0xFD);
        } else if (v == S_IDIX) {
                v = S_IDHL;
                outab(0xDD);
        } else if (v == S_IDIY) {
                v = S_IDHL;
                outab(0xFD);
        }
        return(v);
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
 * Machine dependent initialization
 */
VOID
minit(void)
{
        /*
         * Byte Order
         */
        hilo = 0;

        if (pass == 0) {
                mchtyp = X_R2K;
                sym[2].s_addr = X_R2K;
        }
}

/* i51adr.c */

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
 *      This Assember Ported by
 *      John L. Hartman (JLH)
 *      jhartman at compuserve dot com
 *      noice at noicedebugger dot com
 *
 */

#include "asxxxx.h"
#include "i8051.h"


struct adsym reg51[] = {        /* R0 thru R7 registers */
    {   "R0",   R0      },
    {   "R1",   R1      },
    {   "R2",   R2      },
    {   "R3",   R3      },
    {   "R4",   R4      },
    {   "R5",   R5      },
    {   "R6",   R6      },
    {   "R7",   R7      },
    {   "A",    A       },
    {   "DPTR", DPTR    },
    {   "PC",   PC      },
    {   "C",    C       },
    {   "AB",   AB      },
    {   "",     0x00    }
};

/*  Classify argument as to address mode */
int
addr(struct expr *esp)
{
        int c;
        unsigned rd;

        if ((c = getnb()) == '#') {
                /*  Immediate mode */
                expr(esp, 0);
                esp->e_mode = S_IMMED;
        }
        else if (c == '@') {
                /* choices are @R0, @R1, @DPTR, @A+PC, @A+DPTR */
                switch (reg()) {
                case R0:
                        esp->e_mode = S_AT_R;
                        esp->e_addr = R0;
                        break;
                case R1:
                        esp->e_mode = S_AT_R;
                        esp->e_addr = R1;
                        break;
                case DPTR:
                        esp->e_mode = S_AT_DP;
                        esp->e_addr = DPTR;
                        break;
                case A:
                        if (getnb() == '+') {
                                rd = reg();
                                if (rd == PC) {
                                        esp->e_mode = S_AT_APC;
                                        esp->e_addr = 0;
                                } else if (rd == DPTR) {
                                        esp->e_mode = S_AT_ADP;
                                        esp->e_addr = 0;
                                } else {
                                        aerr();
                                }
                        } else
                                aerr();
                        break;
                }

                esp->e_flag = 0;
                esp->e_base.e_ap = NULL;
        }
        else if (c == '*') {
                if ((c = getnb()) == '/') {
                        /* Force inverted bit */
                        expr(esp, 0);
                        esp->e_mode = S_NOT_BIT;
                } else {
                        unget(c);
                        /* Force direct page */
                        expr(esp, 0);
                        esp->e_mode = S_DIR;
                }
                if (esp->e_addr & ~0xFF)
                        err('d');
        }
        else if (c == '/') {
                /* Force inverted bit  */
                expr(esp, 0);
                esp->e_mode = S_NOT_BIT;
        }
        else {
                unget(c);

                /* try for register: A, AB, R0-R7, DPTR, PC, Cy */
                if ((esp->e_addr = admode(reg51)) != -1) {
                        switch (esp->e_addr) {
                        case A:
                                esp->e_mode = S_A;
                                break;
                        case AB:
                                esp->e_mode = S_RAB;
                                break;
                        case DPTR:
                                esp->e_mode = S_DPTR;
                                break;
                        case PC:
                                esp->e_mode = S_PC;
                                break;
                        case C:
                                esp->e_mode = S_C;
                                break;
                        default:
                                /* R0-R7 */
                                esp->e_mode = S_REG;
                        }
                } else {
                        /* Must be an expression */
                        esp->e_addr = 0;
                        expr(esp, 0);
                        if ((!esp->e_flag)
                                && (esp->e_base.e_ap==NULL)
                                && !(esp->e_addr & ~0xFF)) {
                                esp->e_mode = S_DIR;
                        } else {
                                esp->e_mode = S_EXT;
                        }
                }
        }
        return (esp->e_mode);
}

/*
 * Enter admode() to search a specific addressing mode table
 * for a match. Return the addressing value on a match or
 * -1 for no match.
 */
int
admode(struct adsym *sp)
{
        char *ptr;
        int i;

        unget(getnb());
        i = 0;
        while ( *(ptr = &sp[i].a_str[0]) ) {
                if (srch(ptr)) {
                        return(sp[i].a_val);
                }
                i++;
        }
        return(-1);
}

/*
 *      srch --- does string match ?
 */
int
srch(char *str)
{
        char *ptr;
        ptr = ip;

        while (*ptr && *str) {
                if(ccase[*ptr & 0x007F] != ccase[*str & 0x007F])
                        break;
                ptr++;
                str++;
        }
        if (ccase[*ptr & 0x007F] == ccase[*str & 0x007F]) {
                ip = ptr;
                return(1);
        }

        if (!*str)
                if (any(*ptr," \t\n,];")) {
                        ip = ptr;
                        return(1);
                }
        return(0);
}

/*
 *      any --- does str contain c?
 */
int
any(int c, char *str)
{
        while (*str)
                if(*str++ == c)
                        return(1);
        return(0);
}

/*
 * Read a register name.  Return register value, -1 if no register found
 */
int
reg(void)
{
        struct mne *mp;
        char id[NCPS];

        getid(id, -1);
        if ((mp = mlookup(id))==NULL) {
                aerr();
                return (-1);
        }
        switch (mp->m_type) {
        case S_A:
        case S_AB:
        case S_DPTR:
        case S_PC:
        case S_REG:
                return ((int) mp->m_valu);

        default:
                return (-1);
        }
}

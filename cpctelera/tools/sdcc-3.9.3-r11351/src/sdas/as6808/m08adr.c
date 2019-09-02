/* m08adr.c */

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

#include "asxxxx.h"
#include "m6808.h"

int
addr(esp)
struct expr *esp;
{
        int c;
        struct area *espa;
        a_uint espv;
        char *tcp;

        if ((c = getnb()) == '#') {
                expr(esp, 0);
                esp->e_mode = S_IMMED;
        } else if (c == ',') {
                switch(admode(axs)) {
                default:
                        aerr();

                case S_X:
                        c = S_IX;
                        break;

                case S_S:
                        c = S_IS;
                        break;

                case S_XP:
                        c = S_IXP;
                        break;
                }
                esp->e_mode = c;
        } else if (c == '*') {
                expr(esp, 0);
                esp->e_mode = S_DIR;
                if (more()) {
                        comma(1);
                        tcp = ip;
                        switch(admode(axs)) {
                        case S_X:
                                esp->e_mode = S_IX1;
                                break;

                        case S_S:
                                esp->e_mode = S_SP1;
                                break;

                        case S_XP:
                                esp->e_mode = S_IX1P;
                                break;

                        default:
                                ip = --tcp;
                        }
                }
        } else {
                unget(c);
                if ((esp->e_mode = admode(axs)) != 0) {
                        ;
                } else {
                        expr(esp, 0);
                        espa = esp->e_base.e_ap;
                        espv = esp->e_addr;
                        if (more()) {
                                comma(1);
                                c = admode(axs);
                                if (esp->e_flag == 0 &&
                                    espa == NULL &&
                                    (espv & ~0xFF) == 0) {
                                        switch(c) {
                                        default:
                                                aerr();

                                        case S_X:
                                                c = S_IX1;
                                                break;

                                        case S_S:
                                                c = S_SP1;
                                                break;

                                        case S_XP:
                                                c = S_IX1P;
                                                break;
                                        }
                                } else {
                                        switch(c) {
                                        default:
                                                aerr();

                                        case S_X:
                                                c = S_IX2;
                                                break;

                                        case S_S:
                                                c = S_SP2;
                                                break;

                                        case S_XP:
                                                c = S_IX2P;
                                                break;
                                        }
                                }
                                esp->e_mode = c;
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
 * zero for no match.
 */
int
admode(sp)
struct adsym *sp;
{
        char *ptr;
        int i;
        char *ips;

        ips = ip;
        unget(getnb());

        i = 0;
        while ( *(ptr = &sp[i].a_str[0]) ) {
                if (srch(ptr)) {
                        return(sp[i].a_val);
                }
                i++;
        }
        ip = ips;
        return(0);
}

/*
 *      srch --- does string match ?
 */
int
srch(str)
char *str;
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
any(c,str)
int c;
char*str;
{
        while (*str)
                if(*str++ == c)
                        return(1);
        return(0);
}

struct adsym    axs[] = {       /* a, x, or s registers */
    {   "a",    S_A     },
    {   "x",    S_X     },
    {   "s",    S_S     },
    {   "x+",   S_XP    },
    {   "",     0x00    }
};

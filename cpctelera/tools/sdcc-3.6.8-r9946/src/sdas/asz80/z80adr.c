/* z80adr.c */

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

#include "asxxxx.h"
#include "z80.h"

/*
 * Read an address specifier. Pack the
 * address information into the supplied
 * `expr' structure. Return the mode of
 * the address.
 *
 * This addr(esp) routine performs the following addressing decoding:
 *
 *      address         mode            flag            addr            base
 *      #n              S_IMMED         0               n               NULL
 *      label           s_type          ----            s_addr          s_area
 *      [REG]           S_IND+icode     0               0               NULL
 *      [label]         S_INDM          ----            s_addr          s_area
 *      offset[REG]     S_IND+icode     ----            offset          ----
 */
int
addr(esp)
struct expr *esp;
{
        int c, mode, indx;

        if ((c = getnb()) == '#') {
                expr(esp, 0);
                esp->e_mode = S_IMMED;
        } else
        if (c == LFIND) {
                if ((indx = admode(R8)) != 0) {
                        mode = S_INDB;
                } else
                if ((indx = admode(R16)) != 0) {
                        mode = S_INDR;
                } else
                if ((indx = admode(R8X)) != 0) {
                        mode = S_R8X;
                        aerr();
                } else
                if ((indx = admode(R16X)) != 0) {
                        mode = S_R16X;
                        aerr();
                } else {
                        mode = S_INDM;
                        expr(esp, 0);
                        esp->e_mode = mode;
                }
                if (indx) {
                        esp->e_mode = (mode + indx)&0xFF;
                        esp->e_base.e_ap = NULL;
                }
                if ((c = getnb()) != RTIND) {
                        qerr();
                }
        } else {
                unget(c);
                if ((indx = admode(R8)) != 0) {
                        mode = S_R8;
                } else
                if ((indx = admode(R16)) != 0) {
                        mode = S_R16;
                } else
                if ((indx = admode(R8X)) != 0) {
                        mode = S_R8X;
                } else
                if ((indx = admode(R8U1)) != 0) {
                        mode = S_R8U1;
                } else
                if ((indx = admode(R8U2)) != 0) {
                        mode = S_R8U2;
                } else
                if ((indx = admode(R16X)) != 0) {
                        mode = S_R16X;
                } else {
                        mode = S_USER;
                        expr(esp, 0);
                        esp->e_mode = mode;
                }
                if (indx) {
                        esp->e_addr = indx&0xFF;
                        esp->e_mode = mode;
                        esp->e_base.e_ap = NULL;
                }
                if ((c = getnb()) == LFIND) {
                        if ((indx=admode(R16))!=0
                                && ((indx&0xFF)==IX || (indx&0xFF)==IY)) {
                                esp->e_mode = S_INDR + (indx&0xFF);
                        } else {
                                aerr();
                        }
                        if ((c = getnb()) != RTIND)
                                qerr();
                } else {
                        unget(c);
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
                if (ccase[*ptr & 0x007F] != ccase[*str & 0x007F])
                        break;
                ptr++;
                str++;
        }
        if (ccase[*ptr & 0x007F] == ccase[*str & 0x007F]) {
                ip = ptr;
                return(1);
        }

        if (!*str)
                if (!(ctype[*ptr & 0x007F] & LTR16)) {
                        ip = ptr;
                        return(1);
                }
        return(0);
}

/*
 * Registers
 */

struct  adsym   R8[] = {
    {   "b",    B|0400  },
    {   "c",    C|0400  },
    {   "d",    D|0400  },
    {   "e",    E|0400  },
    {   "h",    H|0400  },
    {   "l",    L|0400  },
    {   "a",    A|0400  },
    {   "",     0000    }
};

struct  adsym   R8X[] = {
    {   "i",    I|0400  },
    {   "r",    R|0400  },
    {   "",     0000    }
};

/* Undocumented instructions for 0xDD prefix H->IX high, L->IX low */
struct  adsym   R8U1[] = {
  {   "ixh",  H|0400  },
  {   "ixl",  L|0400  },
  {   "",     0000    }
};

/* Undocumented instructions for 0xFD prefix H->IY high, L->IY low */
struct  adsym   R8U2[] = {
  {   "iyh",  H|0400  },
  {   "iyl",  L|0400  },
  {   "",     0000    }
};

struct  adsym   R16[] = {
    {   "bc",   BC|0400 },
    {   "de",   DE|0400 },
    {   "hl",   HL|0400 },
    {   "sp",   SP|0400 },
    {   "ix",   IX|0400 },
    {   "iy",   IY|0400 },
    {   "",     0000    }
};

struct  adsym   R16X[] = {
    {   "af'",  AF|0400 },      /* af' must be first !!! */
    {   "af",   AF|0400 },
    {   "",     0000    }
};

/*
 * Conditional definitions
 */

struct  adsym   CND[] = {
    {   "NZ",   NZ|0400 },
    {   "Z",    Z |0400 },
    {   "NC",   NC|0400 },
    {   "C",    CS|0400 },
    {   "PO",   PO|0400 },
    {   "PE",   PE|0400 },
    {   "P",    P |0400 },
    {   "M",    M |0400 },
    {   "",     0000    }
};

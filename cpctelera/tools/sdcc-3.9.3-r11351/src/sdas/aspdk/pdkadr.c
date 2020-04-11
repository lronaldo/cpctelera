/* pdkadr.c */

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
#include "pdk.h"


/*  Classify argument as to address mode */
int
addr(struct expr *esp)
{
        int c = getnb(), c1;

        switch (c) {
        case '#':
                /* Immediate mode */
                expr(esp, 0);
                esp->e_mode = S_K;
                break;

        case 'a':
                /* Accumulator */
                esp->e_mode = S_A;
                break;

        case 's':
                if ((c1 = getnb()) == 'p') {
                        /* Stack (SP) */
                        esp->e_mode = S_IO;
                        esp->e_addr = 2;
                        break;
                }
                unget(c1);
                goto fallback;

        case 'f':
                /* ACC flag */
                esp->e_mode = S_IO;
                esp->e_addr = 0;
                break;

        default:
        fallback:
                unget(c);

                /* Memory address */
                expr(esp, 0);
                esp->e_mode = S_M;

                /* If there is no area information, assume that we have in
                fact parsed an IO register variable - since any other constant
                would have been prefixed by a '#'. */
                if (!esp->e_flag && !esp->e_base.e_ap)
                        esp->e_mode = S_IO;
        }

        return (esp->e_mode);
}

int
pdkbit(struct expr *esp)
{
        int c = getnb(), c1;

        switch (c) {
        case '#':
                /* Bit number */
                expr(esp, 0);
                esp->e_mode = S_K;
                break;

        case 'a':
                if ((c1 = getnb()) == 'c') {
                        /* AC bit of ACC flag */
                        esp->e_mode = S_K;
                        esp->e_addr = 2;
                        break;
                }
                unget(c1);
                goto fallback;
 
        case 'o':
                if ((c1 = getnb()) == 'v') {
                        /* OV bit of ACC flag */
                        esp->e_mode = S_K;
                        esp->e_addr = 3;
                        break;
                }
                unget(c1);
                goto fallback;

        case 'c':
                /* C bit of ACC flag */
                esp->e_mode = S_K;
                esp->e_addr = 1;
                break;

        case 'z':
                /* Z bit of ACC flag */
                esp->e_mode = S_K;
                esp->e_addr = 0;
                break;

        case 'f':
                /* ACC status flag */
                esp->e_mode = S_IO;
                esp->e_addr = -2;
                break;

        default:
        fallback:
                unget(c);

                /* Error */
                esp->e_mode = 0;
                esp->e_addr = 0;
        }

        return (esp->e_mode);
}

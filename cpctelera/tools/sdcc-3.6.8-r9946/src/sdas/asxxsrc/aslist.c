/* aslist.c */

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
 *   With enhancements from
 *
 *      John L. Hartman (JLH)
 *      jhartman at compuserve dot com
*/

#include "asxxxx.h"

/*)Module       aslist.c
 *
 *      The module aslist.c contains all the functions used
 *      to generate the assembler list and symbol output files.
 *
 *      aslist.c contains the following functions:
 *              VOID    list()
 *              VOID    list1()
 *              VOID    list2()
 *              VOID    slew()
 *              VOID    lstsym()
 *
 *      The module aslist.c contains no local/static variables
 */

/*)Function     VOID    list()
 *
 *      The function list() generates the listing output
 *      which includes the input source, line numbers,
 *      and generated code.  Numerical output may be selected
 *      as hexadecimal, decimal, or octal.
 * 
 *      local variables:
 *              char *  il              pointer to assembler-source listing line
 *              int     l_addr          laddr (int) truncated to 2-bytes
 *              int     n               number of bytes listed per line
 *              int     nb              computed number of assembled bytes
 *              int     nl              number of bytes listed on this line
 *              int     listing         listing enable flags
 *              int     paging          computed paging enable flag
 *              char *  wp              pointer to the assembled data bytes
 *              int *   wpt             pointer to the data byte mode
 *              char *  frmt            pointer to format string
 *
 *      global variables:
 *              int     a_bytes         T line addressing size
 *              int     bflag           -b(b), listing mode flag
 *              int     cb[]            array of assembler output values
 *              int     cbt[]           array of assembler relocation types
 *                                      describing the data in cb[]
 *              int *   cp              pointer to assembler output array cb[]
 *              int *   cpt             pointer to assembler relocation type
 *                                      output array cbt[]
 *              char    eb[]            array of generated error codes
 *              char *  ep              pointer into error list
 *                                      array eb[]
 *              char  * ib              string buffer containing
 *                                      assembler-source text line for processing
 *              char  * ic              string buffer containing
 *                                      assembler-source text line for listing
 *              a_uint  laddr           address of current assembler line,
 *                                      equate, or value of .if argument
 *              FILE *  lfp             list output file handle
 *              int     line            current assembler source line number
 *              int     lmode           listing mode
 *              int     lnlist          LIST-NLIST state
 *              int     srcline         source file line number
 *              int     uflag           -u, disable .list/.nlist processing
 *              int     xflag           -x, listing radix flag
 *
 *      functions called:
 *              int     fprintf()       c_library
 *              VOID    list1()         aslist.c
 *              int     putc()          c_library
 *              VOID    slew()          asslist.c
 *
 *      side effects:
 *              Listing or symbol output updated.
 */

/* The Output Formats, No Cycle Count
| Tabs- |       |       |       |       |       |
          11111111112222222222333333333344444-----
012345678901234567890123456789012345678901234-----
   |    |               |     | |
ee XXXX xx xx xx xx xx xx LLLLL *************   HEX(16)
ee 000000 ooo ooo ooo ooo LLLLL *************   OCTAL(16)
ee  DDDDD ddd ddd ddd ddd LLLLL *************   DECIMAL(16)
                     XXXX
                   OOOOOO
                    DDDDD

| Tabs- |       |       |       |       |       |
          11111111112222222222333333333344444-----
012345678901234567890123456789012345678901234-----
     |       |                  |     | |
ee    XXXXXX xx xx xx xx xx xx xx LLLLL *********       HEX(24)
ee   OO000000 ooo ooo ooo ooo ooo LLLLL *********       OCTAL(24)
ee   DDDDDDDD ddd ddd ddd ddd ddd LLLLL *********       DECIMAL(24)
                           XXXXXX
                         OOOOOOOO
                         DDDDDDDD

| Tabs- |       |       |       |       |       |
          11111111112222222222333333333344444-----
012345678901234567890123456789012345678901234-----
  |          |                  |     | |
ee  XXXXXXXX xx xx xx xx xx xx xx LLLLL *********       HEX(32)
eeOOOOO000000 ooo ooo ooo ooo ooo LLLLL *********       OCTAL(32)
ee DDDDDDDDDD ddd ddd ddd ddd ddd LLLLL *********       DECIMAL(32)
                         XXXXXXXX
                      OOOOOOOOOOO
                       DDDDDDDDDD
*/

/* The Output Formats,  With Cycle Count [nn]
| Tabs- |       |       |       |       |       |
          11111111112222222222333333333344444-----
012345678901234567890123456789012345678901234-----
   |    |               |     | |
ee XXXX xx xx xx xx xx[nn]LLLLL *************   HEX(16)
ee 000000 ooo ooo ooo [nn]LLLLL *************   OCTAL(16)
ee  DDDDD ddd ddd ddd [nn]LLLLL *************   DECIMAL(16)
                     XXXX
                   OOOOOO
                    DDDDD

| Tabs- |       |       |       |       |       |
          11111111112222222222333333333344444-----
012345678901234567890123456789012345678901234-----
     |       |                  |     | |
ee    XXXXXX xx xx xx xx xx xx[nn]LLLLL *********       HEX(24)
ee   OO000000 ooo ooo ooo ooo [nn]LLLLL *********       OCTAL(24)
ee   DDDDDDDD ddd ddd ddd ddd [nn]LLLLL *********       DECIMAL(24)
                           XXXXXX
                         OOOOOOOO
                         DDDDDDDD

| Tabs- |       |       |       |       |       |
          11111111112222222222333333333344444-----
012345678901234567890123456789012345678901234-----
  |          |                  |     | |
ee  XXXXXXXX xx xx xx xx xx xx[nn]LLLLL *********       HEX(32)
eeOOOOO000000 ooo ooo ooo ooo [nn]LLLLL *********       OCTAL(32)
ee DDDDDDDDDD ddd ddd ddd ddd [nn]LLLLL *********       DECIMAL(32)
                         XXXXXXXX
                      OOOOOOOOOOO
                       DDDDDDDDDD
*/

VOID
list(void)
{
        char *frmt, *wp;
        int *wpt;
        int n, nb, nl;
        a_uint l_addr;
        int listing, paging;
        const char *il;

        /* ib/ic are dynamically allocated */
        if (bflag != 0) {
                il = ib;
        } else {
                il = ic;
        }

        /*
         * Internal Listing
         */
        listing = lnlist;

        /*
         * Listing Control Override
         */
        if (uflag) {
                listing = LIST_BITS;
                if (lmode == NLIST) {
                        lmode = SLIST;
                }
        }

        /*
         * Paging Control
         */
        paging = !pflag && ((lnlist & LIST_PAG) || (uflag == 1)) ? 1 : 0;

        /*
         * ALIST/BLIST Output Processing
         */
        if (lmode == ALIST) {
                outchk(ASXHUGE,ASXHUGE);
        }
        if (lmode == ALIST || lmode == BLIST) {
                outdot();
        }

        /*
         * Check NO-LIST Conditions
         */
        if ((lfp == NULL) || (lmode == NLIST)) {
                return;
        }

        /*
         * ALIST/BLIST Output Processing
         */
        if (lmode == ALIST) {
                outchk(ASXHUGE,ASXHUGE);
        }

        /*
         * Get Correct Line Number
         */
        line = srcline;

        /*
         * Move to next line.
         */
        slew(lfp, paging);

        /*
         * LIST_ERR - Output a maximum of NERR error codes with listing.
         */
        if (listing & LIST_ERR) {
                while (ep < &eb[NERR])
                        *ep++ = ' ';
                fprintf(lfp, "%.2s", eb);
        } else {
                fprintf(lfp, "  ");
        }

        /*
         * SLIST
         * Source listing Option.
         */
        if (lmode == SLIST) {
                if (listing & LIST_LOC) {
                        switch(a_bytes) {
                        default:
                        case 2: frmt = "%24s%5u %s\n"; break;
                        case 3:
                        case 4: frmt = "%32s%5u %s\n"; break;
                        }
                        fprintf(lfp, frmt, "", line, il);
                } else {
                        switch(a_bytes) {
                        default:
                        case 2: frmt = "%29s %s\n"; break;
                        case 3:
                        case 4: frmt = "%37s %s\n"; break;
                        }
                        fprintf(lfp, frmt, "", il);
                }
                return;
        }

        /*
         * Truncate (int) to N-Bytes
         */
        l_addr = laddr;

        /*
         * ELIST
         * Equate Listing Option
         */
        if (lmode == ELIST) {
                if (listing & LIST_EQT) {
                        switch (xflag) {
                        default:
                        case 0:         /* HEX */
#ifdef  LONGINT
                                switch(a_bytes) {
                                default:
                                case 2: frmt = "%19s%04lX"; break;
                                case 3: frmt = "%25s%06lX"; break;
                                case 4: frmt = "%23s%08lX"; break;
                                }
#else
                                switch(a_bytes) {
                                default:
                                case 2: frmt = "%19s%04X"; break;
                                case 3: frmt = "%25s%06X"; break;
                                case 4: frmt = "%23s%08X"; break;
                                }
#endif
                                break;

                        case 1:         /* OCTAL */
#ifdef  LONGINT
                                switch(a_bytes) {
                                default:
                                case 2: frmt = "%17s%06lo"; break;
                                case 3: frmt = "%23s%08lo"; break;
                                case 4: frmt = "%20s%011lo"; break;
                                }
#else
                                switch(a_bytes) {
                                default:
                                case 2: frmt = "%17s%06o"; break;
                                case 3: frmt = "%23s%08o"; break;
                                case 4: frmt = "%20s%011o"; break;
                                }
#endif
                                break;

                        case 2:         /* DECIMAL */
#ifdef  LONGINT
                                switch(a_bytes) {
                                default:
                                case 2: frmt = "%18s%05lu"; break;
                                case 3: frmt = "%23s%08lu"; break;
                                case 4: frmt = "%21s%010lu"; break;
                                }
#else
                                switch(a_bytes) {
                                default:
                                case 2: frmt = "%18s%05u"; break;
                                case 3: frmt = "%23s%08u"; break;
                                case 4: frmt = "%21s%010u"; break;
                                }
#endif
                                break;
                        }
                        fprintf(lfp, frmt, "", l_addr);
                } else {
                        switch(a_bytes) {
                        default:
                        case 2: frmt = "%23s"; break;
                        case 3:
                        case 4: frmt = "%31s"; break;
                        }
                        fprintf(lfp, frmt, "");
                }
                if ((listing & LIST_LIN) && (listing & LIST_SRC)) {
                        fprintf(lfp, " %5u %s\n", line, il);
                } else
                if (listing & LIST_LIN) {
                        fprintf(lfp, " %5u\n", line);
                } else
                if (listing & LIST_SRC) {
                        fprintf(lfp, " %5s %s\n", "", il);
                } else {
                        fprintf(lfp, "\n");
                }
                return;
        }

        /*
         * LIST_LOC - Location Address
         */
        if (listing & LIST_LOC) {
                switch (xflag) {
                default:
                case 0:         /* HEX */
#ifdef  LONGINT
                        switch(a_bytes) {
                        default:
                        case 2: frmt = " %04lX"; break;
                        case 3: frmt = "    %06lX"; break;
                        case 4: frmt = "  %08lX"; break;
                        }
#else
                        switch(a_bytes) {
                        default:
                        case 2: frmt = " %04X"; break;
                        case 3: frmt = "    %06X"; break;
                        case 4: frmt = "  %08X"; break;
                        }
#endif
                        break;

                case 1:         /* OCTAL */
#ifdef  LONGINT
                        switch(a_bytes) {
                        default:
                        case 2: frmt = " %06lo"; break;
                        case 3: frmt = "   %08lo"; break;
                        case 4: frmt = "%011lo"; break;
                        }
#else
                        switch(a_bytes) {
                        default:
                        case 2: frmt = " %06o"; break;
                        case 3: frmt = "   %08o"; break;
                        case 4: frmt = "%011o"; break;
                        }
#endif
                        break;

                case 2:         /* DECIMAL */
#ifdef  LONGINT
                        switch(a_bytes) {
                        default:
                        case 2: frmt = "  %05lu"; break;
                        case 3: frmt = "   %08lu"; break;
                        case 4: frmt = " %010lu"; break;
                        }
#else
                        switch(a_bytes) {
                        default:
                        case 2: frmt = "  %05u"; break;
                        case 3: frmt = "   %08u"; break;
                        case 4: frmt = " %010u"; break;
                        }
#endif
                        break;
                }
                fprintf(lfp, frmt, l_addr);
        } else {
                switch(a_bytes) {
                default:
                case 2: frmt = "%5s"; break;
                case 3:
                case 4: frmt = "%10s"; break;
                }
                fprintf(lfp, frmt, "");
        }

        /*
         * ALIST/BLIST Listing Options
         */
        if (lmode == ALIST || lmode == BLIST) {
                if (listing & LIST_LIN) {
                        switch (xflag) {
                        default:
                        case 0:         /* HEX */
                                switch(a_bytes) {
                                default:
                                case 2: frmt = "%19s%5u %s\n"; break;
                                case 3:
                                case 4: frmt = "%22s%5u %s\n"; break;
                                }
                                break;

                        case 1:         /* OCTAL */
                                switch(a_bytes) {
                                default:
                                case 2: frmt = "%17s%5u %s\n"; break;
                                case 3:
                                case 4: frmt = "%21s%5u %s\n"; break;
                                }
                                break;

                        case 2:         /* DECIMAL */
                                switch(a_bytes) {
                                default:
                                case 2: frmt = "%17s%5u %s\n"; break;
                                case 3:
                                case 4: frmt = "%21s%5u %s\n"; break;
                                }
                                break;
                        }
                        fprintf(lfp, frmt, "", line, il);
                } else {
                        switch (xflag) {
                        default:
                        case 0:         /* HEX */
                                switch(a_bytes) {
                                default:
                                case 2: frmt = "%19s%5s %s\n"; break;
                                case 3:
                                case 4: frmt = "%22s%5s %s\n"; break;
                                }
                                break;

                        case 1:         /* OCTAL */
                                switch(a_bytes) {
                                default:
                                case 2: frmt = "%17s%5s %s\n"; break;
                                case 3:
                                case 4: frmt = "%21s%5s %s\n"; break;
                                }
                                break;

                        case 2:         /* DECIMAL */
                                switch(a_bytes) {
                                default:
                                case 2: frmt = "%17s%5s %s\n"; break;
                                case 3:
                                case 4: frmt = "%21s%5s %s\n"; break;
                                }
                                break;
                        }
                        fprintf(lfp, frmt, "", "", il);
                }
                return;
        }

        /*
         * LIST_BIN - Binary Listing Option
         * LIST_CYC - Opcode Cycles Option
         * LIST_LIN - Line Number Option
         * LIST_SRC - Source Listing Option
         */
        if (!(listing & (LIST_BIN | LIST_CYC | LIST_LIN | LIST_SRC))) {
                fprintf(lfp, "\n");
                return;
        }

        /*
         * Format
         */
        switch (xflag) {
        default:
        case 0:         /* HEX */
                switch(a_bytes) {
                default:
                case 2: n = 6; frmt = "%7s"; break;
                case 3:
                case 4: n = 7; frmt = "%12s"; break;
                }
                break;

        case 1:         /* OCTAL */
                switch(a_bytes) {
                default:
                case 2: n = 4; frmt = "%9s"; break;
                case 3:
                case 4: n = 5; frmt = "%13s"; break;
                }
                break;

        case 2:         /* DECIMAL */
                switch(a_bytes) {
                default:
                case 2: n = 4; frmt = "%9s"; break;
                case 3:
                case 4: n = 5; frmt = "%13s"; break;
                }
                break;
        }

        wp = cb;
        wpt = cbt;
        nb = (int) (cp - cb);

        /*
         * If we list cycles, decrease max. bytes on first line.
         */
        nl = (!cflag && !(opcycles & OPCY_NONE) && (listing & LIST_CYC)) ? (n-1) : n;

        /*
         * First line of output for this source line with data.
         */
        if (listing & (LIST_LIN | LIST_SRC)) {
                list1(wp, wpt, nb, nl, 1, listing);
                if ((listing & LIST_LIN) && (listing & LIST_SRC)) {
                        fprintf(lfp, "%5u %s", line, il);
                } else
                if (listing & LIST_LIN) {
                        fprintf(lfp, "%5u", line);
                } else
                if (listing & LIST_SRC) {
                        fprintf(lfp, "%5s %s", "", il);
                }
        } else {
                list1(wp, wpt, nb, nl, listing & LIST_CYC, listing);
        }
        fprintf(lfp, "\n");

        /*
         * Subsequent lines of output if more data.
         */
        if (listing & LIST_BIN) {
                while ((nb - nl) > 0) {
                        nb -= nl;
                        wp += nl;
                        wpt += nl;
                        nl = n;
                        slew(lfp, paging);
                        fprintf(lfp, frmt, "");
                        list1(wp, wpt, nb, nl, 0, listing);
                        putc('\n', lfp);
                }
        }
}

/*)Function     VOID    list1(wp, wpt, nw, n, f, g)
 *
 *              int     g               listing enable flags
 *              int     f               fill blank fields (1)
 *              int     n               number of bytes listed per line
 *              int     nb              number of data bytes
 *              int *   wp              pointer to data bytes
 *              int *   wpt             pointer to data byte mode
 *
 *      local variables:
 *              int     i               loop counter
 *
 *      global variables:
 *              int     xflag           -x, listing radix flag
 *
 *      functions called:
 *              VOID    list2()         asslist.c
 *              int     fprintf()       c_library
 *
 *      side effects:
 *              Data formatted and output to listing.
 */

VOID
list1(char *wp, int *wpt, int nb, int n, int f, int g)
{
        int i;
        char *frmt1, *frmt2;

        switch (xflag) {
        default:
        case 0:         /* HEX */
                frmt1 = "%02X";
                frmt2 = "   ";
                break;

        case 1:         /* OCTAL */
                frmt1 = "%03o";
                frmt2 = "    ";
                break;

        case 2:         /* DECIMAL */
                frmt1 = "%03u";
                frmt2 = "    ";
                break;
        }

        if (nb > n)
                nb = n;

        /*
         * Output bytes.
         */
        for (i=0; i<nb; ++i) {
                if (g & LIST_BIN) {
                        list2(*wpt++);
                        fprintf(lfp, frmt1, (*wp++)&0377);
                } else {
                        fprintf(lfp, "%s", frmt2);
                }
        }

        /*
         * Output blanks if required.
         */
        if (f) {
                while (i++ < n) {
                        fprintf(lfp, "%s", frmt2);
                }
        }

        /*
         * If we list cycles, put them out, first line only
         */
        if (f && (g & LIST_CYC) && !cflag && !(opcycles & OPCY_NONE)) {
                fprintf(lfp, "%s%c%2d%c",
                        (xflag != 0) ? " " : "", CYCNT_BGN, opcycles, CYCNT_END);
        } else
        if (f) {
                fprintf(lfp, " ");
        }
}

/*)Function     VOID    list2(wpt)
 *
 *              int *   wpt             pointer to relocation mode
 *
 *      The function list2() outputs the selected
 *      relocation flag as specified by fflag.
 *
 *      local variables:
 *              int     c               relocation flag character
 *              int     t               relocation mode
 *
 *      global variables:
 *              int     fflag           -f(f), relocations flagged flag
 *
 *      functions called:
 *              int     putc()          c_library
 *
 *      side effects:
 *              Relocation flag output to listing file.
 */

VOID
list2(int t)
{
        int c;

        c = ' ';

        /*
         * Designate a relocatable word by `.
         */
        if (fflag == 1) {
                if (t & R_RELOC) {
                        c = '`';
                }
        } else
        /*
         * Designate a relocatable word by its mode:
         *      page0 or paged          *
         *      unsigned                u (v)
         *      operand offset          p (q)
         *      relocatable symbol      r (s)
         */
        if (fflag >= 2) {
                if (t & R_RELOC) {
                        if (t & R_PCR) {
                                c = 'p';
                        } else
                        if (t & (R_PAG0|R_PAGN)) {
                                c = '*';
                        } else
                        if (t & R_USGN) {
                                c = 'u';
                        } else {
                                c = 'r';
                        }
                        if (t & R_HIGH) c += 1;
                }
        }

        /*
         * Output the selected mode.
         */
        putc(c, lfp);
}

/*)Function     VOID    slew(fp, flag)
 *
 *              FILE *  fp              file handle for listing
 *              int     flag            enable pagination
 *
 *      The function slew() increments the page line count.
 *      If the page overflows and pagination is enabled:
 *              1)      put out a page skip,
 *              2)      a title,
 *              3)      a subtitle,
 *              4)      and reset the line count.
 *
 *      local variables:
 *              none
 *
 *      global variables:
 *              int     a_bytes         T line addressing size
 *              char    cpu[]           cpu type string
 *              int     lop             current line number on page
 *              int     page            current page number
 *              char    stb[]           Subtitle string buffer
 *              char    tb[]            Title string buffer
 *
 *      functions called:
 *              int     fprintf()       c_library
 *
 *      side effects:
 *              Increments page line counter, on overflow
 *              a new page header is output to the listing file.
 */

VOID
slew(FILE *fp, int flag)
{
        char *frmt;

        if (lop++ >= NLPP) {
                if (flag) {
                        fprintf(fp, "\fASxxxx Assembler %s  (%s), page %u.\n",
                                VERSION, cpu, ++page);
                        switch(xflag) {
                        default:
                        case 0: frmt = "Hexadecimal [%d-Bits]\n"; break;
                        case 1: frmt = "Octal [%d-Bits]\n"; break;
                        case 2: frmt = "Decimal [%d-Bits]\n"; break;
                        }
                        fprintf(fp, frmt, 8 * a_bytes);
                        fprintf(fp, "%s\n", tb);
                        fprintf(fp, "%s\n\n", stb);
                        lop = 6;
                } else {
                        lop = 1;
                }
        }
}

/* sdas specific */
/* Used for qsort call in lstsym */
static int _cmpSym(const void *p1, const void *p2)
{
    struct sym **s1 = (struct sym **)(p1);
    struct sym **s2 = (struct sym **)(p2);
    return strcmp((*s1)->s_id,(*s2)->s_id);
}
/* end sdas specific */

/*)Function     VOID    lstsym(fp)
 *
 *              FILE *  fp              file handle for output
 *
 *      The function lstsym() outputs alphabetically
 *      sorted symbol and area tables.
 *
 *      local variables:
 *              int     c               temporary
 *              int     i               loop counter
 *              int     j               temporary
 *              int     k               temporary
 *              int     n               temporary
 *              int     nmsym           number of symbols
 *              int     narea           number of areas
 *              int     nbank           number of banks
 *              sym **  p               pointer to an array of
 *                                      pointers to symbol structures
 *              int     paging          computed paging enable flag
 *              char *  ptr             pointer to an id string
 *              a_uint  sa              temporary
 *              sym *   sp              pointer to symbol structure
 *              area *  ap              pointer to an area structure
 *
 *      global variables:
 *              int     a_bytes         T line addressing size
 *              area *  areap           pointer to an area structure
 *              char    aretbl[]        string "Area Table"
 *              sym     dot             defined as sym[0]
 *              int     lnlist          LIST-NLIST state
 *              char    stb[]           Subtitle string buffer
 *              sym * symhash[]         array of pointers to NHASH
 *                                      linked symbol lists
 *              char    symtbl[]        string "Symbol Table"
 *              FILE *  tfp             symbol table output file handle
 *              int     uflag           LIST-NLIST override flag
 *              int     wflag           -w, wide listing flag
 *              int     xflag           -x, listing radix flag
 *
 *      functions called:
 *              int     fprintf()       c_library
 *              int     putc()          c_library
 *              VOID    slew()          aslist.c
 *              int     strcmp()        c_library
 *              char *  strcpy()        c_library
 *              char *  new()           assym.c
 *
 *      side effects:
 *              Symbol and area tables output.
 */

VOID
lstsym(FILE *fp)
{
        int i, j, k, n, paging;
        int nmsym, narea, nbank;
        a_uint sa;
        char *frmt, *ptr;
        struct sym *sp;
        struct sym **p;
        struct area *ap;

        /*
         * Symbol Table Header
         */
        strcpy(stb, &symtbl[0]);
        lop = NLPP;
        if (fp == tfp) {
                page = 0;
                paging = 1;
        } else {
                paging = !pflag && ((lnlist & LIST_PAG) || (uflag == 1)) ? 1 : 0;
        }
        slew(fp, 1);

        /*
         * Find number of symbols
         */
        nmsym = 0;
        for (i=0; i<NHASH; i++) {
                sp = symhash[i];
                while (sp) {
                        if (sp != &dot)
                                ++nmsym;
                        sp = sp->s_sp;
                }
        }
        if (nmsym == 0)
                goto atable;

        /*
         * Allocate space for an array of pointers to symbols
         * and load array.
         */
        p = (struct sym **) new (sizeof((struct sym *) sp)*nmsym);
        if (p == NULL) {
                fprintf(fp, "Insufficient space to build Symbol Table.\n");
                return;
        }
        nmsym = 0;
        for (i=0; i<NHASH; i++) {
                sp = symhash[i];
                while (sp) {
                        if (sp != &dot)
                                p[nmsym++] = sp;
                        sp = sp->s_sp;
                }
        }

/* sdas specific */
#if 0
        /* BUBBLE SORT?? WTF??? */
        /*
         * Bubble Sort on Symbol Table Array
         */
        j = 1;
        c = nmsym - 1;
        while (j) {
                j = 0;
                for (i=0; i<c; ++i) {
                        if (strcmp(&p[i]->s_id[0],&p[i+1]->s_id[0]) > 0) {
                                j = 1;
                                sp = p[i+1];
                                p[i+1] = p[i];
                                p[i] = sp;
                        }
                }
        }
#else

        qsort(p, nmsym, sizeof(struct sym *), _cmpSym);
#endif  
/* end sdas specific */

        /*
         * Symbol Table Output
         */
        for (i=0; i<nmsym;) {
                sp = p[i];
                if (sp->s_area) {
                        j = sp->s_area->a_ref;
                        switch(xflag) {
                        default:
                        case 0: frmt = " %2X "; break;
                        case 1: frmt = "%3o "; break;
                        case 2: frmt = "%3u "; break;
                        }
                        fprintf(fp, frmt, j);
                } else {
                        fprintf(fp, "    ");
                }

                ptr = &sp->s_id[0];
                if (wflag) {
                        fprintf(fp, "%-60.60s", ptr );  /* JLH */
                } else {
                        fprintf(fp, "%-8.8s", ptr);
                }
                if (sp->s_flag & S_ASG) {
                        fprintf(fp, "=");
                } else {
                        fprintf(fp, " ");
                }
                if (sp->s_type == S_NEW) {
                        switch(a_bytes) {
                        default:
                        case 2:
                                switch(xflag) {
                                default:
                                case 0: frmt = "  **** "; break;
                                case 1: frmt = "****** "; break;
                                case 2: frmt = " ***** "; break;
                                }
                                break;

                        case 3:
                                switch(xflag) {
                                default:
                                case 0: frmt = "  ****** "; break;
                                case 1: frmt = "******** "; break;
                                case 2: frmt = "******** "; break;
                                }
                                break;

                        case 4:
                                switch(xflag) {
                                default:
                                case 0: frmt = "   ******** "; break;
                                case 1: frmt = "*********** "; break;
                                case 2: frmt = " ********** "; break;
                                }
                                break;

                        }
                        fprintf(fp, "%s", frmt);
                } else {
                        sa = sp->s_addr;
#ifdef  LONGINT
                        switch(a_bytes) {
                        default:
                        case 2:
                                switch(xflag) {
                                default:
                                case 0: frmt = "  %04lX "; break;
                                case 1: frmt = "%06lo "; break;
                                case 2: frmt = " %05lu "; break;
                                }
                                break;

                        case 3:
                                switch(xflag) {
                                default:
                                case 0: frmt = "  %06lX "; break;
                                case 1: frmt = "%08lo "; break;
                                case 2: frmt = "%08lu "; break;
                                }
                                break;

                        case 4:
                                switch(xflag) {
                                default:
                                case 0: frmt = "   %08lX "; break;
                                case 1: frmt = "%011lo "; break;
                                case 2: frmt = " %010lu "; break;
                                }
                                break;
                        }
#else
                        switch(a_bytes) {
                        default:
                        case 2:
                                switch(xflag) {
                                default:
                                case 0: frmt = "  %04X "; break;
                                case 1: frmt = "%06o "; break;
                                case 2: frmt = " %05u "; break;
                                }
                                break;

                        case 3:
                                switch(xflag) {
                                default:
                                case 0: frmt = "  %06X "; break;
                                case 1: frmt = "%08o "; break;
                                case 2: frmt = "%08u "; break;
                                }
                                break;

                        case 4:
                                switch(xflag) {
                                default:
                                case 0: frmt = "   %08X "; break;
                                case 1: frmt = "%011o "; break;
                                case 2: frmt = " %010u "; break;
                                }
                                break;
                        }
#endif
                        fprintf(fp, frmt, sa);
                }

                j = 0;
                if (sp->s_flag & S_GBL) {
                        putc('G', fp);
                        ++j;
                }
                if (sp->s_flag & S_LCL) {
                        putc('L', fp);
                        ++j;
                }
                if (sp->s_area != NULL) {
                        putc('R', fp);
                        ++j;
                }
                if (sp->s_type == S_NEW) {
                        putc('X', fp);
                        ++j;
                }
                if (wflag) {
                        putc('\n', fp);         /* JLH */
                        slew(fp, paging);
                        ++i;
                } else {
                        if (++i % 3 == 0) {
                                putc('\n', fp);
                                slew(fp, paging);
                        } else
                        if (i < nmsym) {
                                while (j++ < 4)
                                        putc(' ', fp);
                                fprintf(fp, "| ");
                        }
                }
        }
        if (nmsym % 3) {
                putc('\n', fp);
        }
        putc('\n', fp);

        /*
         * Area Table Header
         */

atable:
        strcpy(stb, &aretbl[0]);
        lop = NLPP;
        slew(fp, 1);

        /*
         * Area Table Output
         */
        narea = 0;
        nbank = 1;
        ap = areap;
        while (ap) {
                ++narea;
                ap = ap->a_ap;
        }

        for (n=0; n<nbank; ++n) {
                for (i=0; i<narea; ++i) {
                        ap = areap;
                        for (j=i+1; j<narea; ++j)
                                ap = ap->a_ap;
                        j = ap->a_ref;
                        switch(xflag) {
                        default:
                        case 0: frmt = "  %2X "; break;
                        case 1: frmt = " %3o "; break;
                        case 2: frmt = " %3u "; break;
                        }
                        fprintf(fp, frmt, j);

                        ptr = &ap->a_id[0];
                        if (wflag) {
                                fprintf(fp, "%-40.40s", ptr );
                        } else {
                                fprintf(fp, "%-8.8s", ptr);
                        }

                        sa = ap->a_size;
                        k = ap->a_flag;
#ifdef  LONGINT
                        switch(a_bytes) {
                        default:
                        case 2:
                                switch(xflag) {
                                default:
                                case 0: frmt = "   size %4lX   flags %4X\n"; break;
                                case 1: frmt = "   size %6lo   flags %6o\n"; break;
                                case 2: frmt = "   size %5lu   flags %6u\n"; break;
                                }
                                break;

                        case 3:
                                switch(xflag) {
                                default:
                                case 0: frmt = "   size %6lX   flags %4X\n"; break;
                                case 1: frmt = "   size %8lo   flags %6o\n"; break;
                                case 2: frmt = "   size %8lu   flags %6u\n"; break;
                                }
                                break;

                        case 4:
                                switch(xflag) {
                                default:
                                case 0: frmt = "   size %8lX   flags %4X\n"; break;
                                case 1: frmt = "   size %11lo   flags %6o\n"; break;
                                case 2: frmt = "   size %10lu   flags %6u\n"; break;
                                }
                                break;
                        }
#else
                        switch(a_bytes) {
                        default:
                        case 2:
                                switch(xflag) {
                                default:
                                case 0: frmt = "   size %4X   flags %4X\n"; break;
                                case 1: frmt = "   size %6o   flags %6o\n"; break;
                                case 2: frmt = "   size %5u   flags %6u\n"; break;
                                }
                                break;

                        case 3:
                                switch(xflag) {
                                default:
                                case 0: frmt = "   size %6X   flags %4X\n"; break;
                                case 1: frmt = "   size %8o   flags %6o\n"; break;
                                case 2: frmt = "   size %8u   flags %6u\n"; break;
                                }
                                break;

                        case 4:
                                switch(xflag) {
                                default:
                                case 0: frmt = "   size %8X   flags %4X\n"; break;
                                case 1: frmt = "   size %11o   flags %6o\n"; break;
                                case 2: frmt = "   size %10u   flags %6u\n"; break;
                                }
                                break;
                        }
#endif
                        fprintf(fp, frmt, sa, k);
                        slew(fp, paging);
                }
        }
        putc('\n', fp);
}

/* assubr.c */

/*
 *  Copyright (C) 1989-2021  Alan R. Baldwin
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

#include "sdas.h"
#include "asxxxx.h"

/*)Module       assubr.c
 *
 *      The module assubr.c contains the error
 *      processing routines.
 *
 *      assubr.c contains the following functions:
 *              VOID    aerr()
 *              VOID    diag()
 *              VOID    err()
 *              VOID    qerr()
 *              VOID    rerr()
 *              char *  geterr()
 *		VOID	xerr()
 *
 *      assubr.c contains the local array of *error[]
 */

/*)Function     VOID    err(c)
 *
 *              int     c               error type character
 *
 *	The legacy function err() reports errors using
 *	the default error descriptions by calling xerr()
 *	with a NULL string.
  *
 *	functions called:
 *		VOID	xerr()		assubr.c
 *
 *	side effects:
 *		The error code may be inserted into the
 *		error code array eb[].
 */

VOID
err(int c)
{
	xerr(c, NULL);
}

/*)Function	VOID	xerr(c, str)
 *
 *		int	c		error type character
 *		char *	str		the error message string
 *
 *	The function xerr() logs the error code character
 *      suppressing duplicate errors.  If the error code
 *      is 'q' then the parse of the current assembler-source
 *      text line is terminated.
 *
 *      local variables:
 *              char *  p               pointer to the error array
 *
 *      global variables:
 *              int     aserr           error counter
 *              char    eb[]            array of generated error codes
 *		char *	ex[]		array of error string pointers
 *
 *      functions called:
 *              VOID    longjmp()       c_library
 *
 *      side effects:
 *              The error code may be inserted into the
 *		error code array eb[], a pointer to the
 *		optional error message inserted into the
 *		array ex[], or the parse terminated.
 */

VOID
xerr(int c, char *str)
{
        char *p;

        aserr++;
        p = eb;
        while (p < ep)
                if (*p++ == c)
                        return;
        if (p < &eb[NERR]) {
		ex[(int) (p-eb)] = str;
                *p++ = c;
                ep = p;
        }
        if (c == 'q')
                longjmp(jump_env, -1);
}

/*)Function     VOID    diag()
 *
 *      The function diag() prints any error codes and
 *      the source line number to the stderr output device.
 *
 *      local variables:
 *              char *  p               pointer to error code array eb[]
 *
 *      global variables:
 *              char    eb[]            array of generated error codes
 *              char *  ep              pointer into error list
 *              int     incline         include file line number
 *              char    afn[]           afile() constructed filespec
 *              FILE *  stderr          c_library
 *
 *      functions called:
 *              int     fprintf()       c_library
 *              char *  geterr()        assubr.c
 *              int     getlnm()        assubr.c
 *
 *      side effects:
 *		Error strings output to stderr.
 */

VOID
diag(void)
{
        char *p,*errstr;
	FILE *fp;

	fp = stderr;
        if (eb != ep) {
                p = eb;
                if (!is_sdas()) {
			fprintf(fp, "?ASxxxx-Error-<");
                        while (p < ep) {
				fprintf(fp, "%c", *p);
				p++;
                        }
			fprintf(fp, "> in line ");
			fprintf(fp, "%d", getlnm());
			fprintf(fp, " of %s\n", afn);
                }
                p = eb;
                while (p < ep) {
                        if ((ex[(int) (p-eb)] != NULL) && (*ex[(int) (p-eb)] != 0)) {
                                if (!is_sdas()) {
                                        fprintf(fp, "              <%c> %s\n", *p, ex[(int) (p-eb)]);
                                } else {
                                        /* Modified to conform to gcc error standard, basxto, 24 Mar '22. */
                                        fprintf(stderr, "%s:", afn);
                                        fprintf(stderr, "%d: Error:", getlnm());
                                        fprintf(stderr, " <%c> %s\n", *p, ex[(int) (p-eb)]);
                                }
                        } else
                        if ((errstr = geterr(*p)) != NULL) {
                                if (!is_sdas()) {
                                        fprintf(fp, "              %s\n", errstr);
                                } else {
                                        /* Modified to conform to gcc error standard, M. Hope, 7 Feb 98. */
                                        fprintf(stderr, "%s:", afn);
                                        fprintf(stderr, "%d: Error:", getlnm());
                                        fprintf(stderr, " %s\n", errstr);
                                }
                        }
			p++;
                }
        }
}


/* sdas specific */
/*)Function     VOID    warnBanner()
 *
 *      The function warnBanner() prints a generic warning message
 *      header (including the current source file/line) and positions
 *      the output for a more specific warning message.
 *
 *      It is assumed that the call to warnBanner will be followed with
 *      a fprintf to stderr (or equivalent) with the specific warning
 *      text.
 *
 *      local variables:
 *              none
 *
 *      global variables:
 *              char    afn[]           afile() constructed filespec
 *              FILE *  stderr          c_library
 *
 *      functions called:
 *              int     fprintf()       c_library
 *              int     getlnm()        assubr.c
 *
 *      side effects:
 *              none
 */
VOID
warnBanner(void)
{
        fprintf(stderr, "?ASxxxx-Warning in line ");
        fprintf(stderr, "%d", getlnm());
        fprintf(stderr, " of %s\n", afn);
        fprintf(stderr, "               ");
}       
/* end sdas specific */

/*)Functions:   VOID    aerr()
 *              VOID    qerr()
 *              VOID    rerr()
 *
 *      The functions aerr(), qerr(), and rerr() report their
 *      respective error type.  These are included only for
 *      convenience.
 *
 *      local variables:
 *              none
 *
 *      global variables:
 *              none
 *
 *      functions called:
 *              VOID    err()           assubr.c
 *
 *      side effects:
 *              The appropriate error code is inserted into the
 *              error array and the parse may be terminated.
 */

/*
 * Note an 'r' error.
 */
VOID
rerr(void)
{
        err('r');
}

/*
 * Note an 'a' error.
 */
VOID
aerr(void)
{
        err('a');
}

/*
 * Note a 'q' error.
 */
VOID
qerr(void)
{
        err('q');
}

/*
 * Default ASxxxx assembler errors
 */
char *errors[] = {
        "<.> use \". = . + <arg>\" not \". = <arg>\"",
        "<a> machine specific addressing or addressing mode error",
	"<b> address / direct page boundary error",
	"<c> .bndry offset error",
        "<d> direct page addressing error",
	"<e> .error/.assume programmed error",
	"<i> .include/.incbin file error or an .if/.endif mismatch",
	"<k> numerical conversion error",
	"<m> multiple definitions error or macro recursion error",
	"<n> .endm, .mexit, or .narg outside of a macro",
        "<o> .org in REL area or directive / mnemonic error",
        "<p> phase error: label location changing between passes 2 and 3",
        "<q> missing or improper operators, terminators, or delimiters",
        "<r> relocation error",
	"<s> string substitution / recursion error",
        "<u> undefined symbol encountered during assembly",
	"<v> out of range signed / unsigned value",
	"<z> divide by zero or mod of zero error",
        NULL
};

/*)Function:    char    *geterr(c)
 *
 *              int     c               the error code character
 *
 *      The function geterr() scans the list of errors returning the
 *      error string corresponding to the input error character.
 *
 *      local variables:
 *              int     i               error index counter
 *
 *      global variables:
 *              char    *errors[]       array of pointers to the
 *                                      error strings
 *              char    erb[]           Error string buffer
 *
 *      functions called:
 *              none
 *
 *      side effects:
 *              A pointer to the appropriate
 *              error code string is returned.
 */
char *
geterr(int c)
{
        int i;

        for (i=0; errors[i]!=NULL; i++) {
                if (c == errors[i][1]) {
                        return(errors[i]);
                }
        }
        sprintf(erb, "<e> %.*s", (int) (sizeof(erb)-5), ib);
        return(erb);
}



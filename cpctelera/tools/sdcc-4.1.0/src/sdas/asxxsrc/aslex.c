/* aslex.c */

/*
 *  Copyright (C) 1989-2010  Alan R. Baldwin
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
 */

/*
 * 28-Oct-97 JLH bug in getst(): sign extend on ~(SPACE|ILL)
 *           causes infinite loop
 */

/*
 * Extensions: P. Felber, M. Hope
 */

#include "dbuf_string.h"
#include "asxxxx.h"

/*)Module       aslex.c
 *
 *      The module aslex.c includes the general lexical
 *      analysis routines for the assembler.
 *
 *      aslex.c contains the following functions:
 *              int     comma()
 *              char    endline()
 *              int     get()
 *              int     getdstr()
 *              int     getdlm()
 *              VOID    getid()
 *              int     getmap()
 *              int     getnb()
 *              int     getlnm()
 *              VOID    getst()
 *              int     more()
 *              int     nxtline()
 *              VOID    unget()
 *
 *      aslex.c contains no local/static variables
 */

/*)Function     VOID    getid(id,c)
 *
 *              char *  id              a pointer to a string of
 *                                      maximum length NCPS-1
 *              int     c               mode flag
 *                                      >=0     this is first character to
 *                                              copy to the string buffer
 *                                      <0      skip white space, first
 *                                              character must be a LETTER
 *
 *      The function getid() scans the current assembler-source text line
 *      from the current position copying the next LETTER | DIGIT string
 *      into the external string buffer id[].  The string ends when a non
 *      LETTER or DIGIT character is found. The maximum number of characters
 *      copied is NCPS-1.  If the input string is larger than NCPS-1
 *      characters then the string is truncated.  The string is always
 *      NULL terminated.  If the mode argument (c) is >=0 then (c) is
 *      the first character copied to the string buffer, if (c) is <0
 *      then intervening white space (SPACES and TABS) are skipped and
 *      the first character found must be a LETTER else a 'q' error
 *      terminates the parse of this assembler-source text line.
 *
 *      local variables:
 *              char *  p               pointer to external string buffer
 *              int     c               current character value
 *
 *      global variables:
 *              char    ctype[]         a character array which defines the
 *                                      type of character being processed.
 *                                      This index is the character
 *                                      being processed.
 *
 *      called functions:
 *              int     get()           aslex.c
 *              int     getnb()         aslex.c
 *              VOID    unget()         aslex.c
 *              VOID    qerr()          assubr.c
 *
 *      side effects:
 *              Use of getnb(), get(), and unget() updates the
 *              global pointer ip, the position in the current
 *              assembler-source text line.
 */

VOID
getid(char *id, int c)
{
        char *p;

        if (c < 0) {
                c = getnb();
                if ((ctype[c] & LETTER) == 0)
                        qerr();
        }
        p = id;
        do {
                if (p < &id[NCPS-1])
                        *p++ = c;
        } while (ctype[c=get()] & (LETTER|DIGIT));
        unget(c);
        *p++ = 0;
}

/*)Function     VOID    getst(id,c)
 *
 *              char *  id              a pointer to a string of
 *                                      maximum length NCPS-1
 *              int     c               mode flag
 *                                      >=0     this is first character to
 *                                              copy to the string buffer
 *                                      <0      skip white space, first
 *                                              character must be a LETTER
 *
 *      The function getst() scans the current assembler-source text line
 *      from the current position copying the next character string into
 *      the external string buffer (id).  The string ends when a SPACE or
 *      ILL character is found. The maximum number of characters copied is
 *      NCPS-1.  If the input string is larger than NCPS-1 characters then
 *      the string is truncated.  The string is always NULL terminated.
 *      If the mode argument (c) is >=0 then (c) is the first character
 *      copied to the string buffer, if (c) is <0 then intervening white
 *      space (SPACES and TABS) are skipped and the first character found
 *      must be a LETTER else a 'q' error terminates the parse of this
 *      assembler-source text line.
 *
 *      local variables:
 *              char *  p               pointer to external string buffer
 *              int     c               current character value
 *
 *      global variables:
 *              char    ctype[]         a character array which defines the
 *                                      type of character being processed.
 *                                      This index is the character
 *                                      being processed.
 *
 *      called functions:
 *              int     get()           aslex.c
 *              int     getnb()         aslex.c
 *              VOID    unget()         aslex.c
 *              VOID    qerr()          assubr.c
 *
 *      side effects:
 *              use of getnb(), get(), and unget() updates the
 *              global pointer ip, the position in the current
 *              assembler-source text line.
 */

VOID
getst(char *id, int c)
{
        char *p;

        if (c < 0) {
                c = getnb();
                if ((ctype[c] & LETTER) == 0)
                        qerr();
        }
        p = id;
        do {
                if (p < &id[NCPS-1])
                        *p++ = c;
        } while (ctype[c=get()] & ~(SPACE|ILL) & 0xFF);
        unget(c);
        *p++ = 0;
}

/*)Function     int     getdstr(str, slen)
 *
 *              char *  str             character array to return string in
 *              int     slen            charater array length
 *
 *      The function getdstr() returns the character string
 *      within delimiters.  If no delimiting character
 *      is found a 'q' error is generated.
 *
 *      local variables:
 *              int     c               current character from
 *                                      assembler-source text line
 *              int     d               the delimiting character
 *
 *      global variables:
 *              none
 *
 *      called functions:
 *              int     get()           aslex.c
 *              int     getdlm()        aslex.c
 *              VOID    qerr()          assubr.c
 *
 *      side effects:
 *              Returns the character string delimited by the
 *              character returned from getdlm().  SPACEs and
 *              TABs before the delimited string are skipped.
 *              A 'q' error is generated if no delimited string
 *              is found or the input line terminates unexpectedly.
 */

VOID
getdstr(str, slen)
char * str;
int slen;
{
        char *p;
        int c, d;

        d = getdlm();

        p = str;
        while ((c = get()) != d) {
                if (c == '\0') {
                        qerr();
                }
                if (p < &str[slen-1]) {
                        *p++ = c;
                } else {
                        break;
                }
        }
        *p = 0;
}

/*)Function     int     getdlm()
 *
 *      The function getdlm() returns the delimiter character
 *      or if the end of the line is encountered a 'q' error
 *      is generated.
 *
 *      local variables:
 *              int     c               current character from
 *                                      assembler-source text line
 *
 *      global variables:
 *              none
 *
 *      called functions:
 *              int     get()           aslex.c
 *              int     getnb()         aslex.c
 *              int     more()          aslex.c
 *              VOID    qerr()          assubr.c
 *
 *      side effects:
 *              scans ip to the first non 'SPACE' or 'TAB' character
 *              and returns that character or the first character
 *              following a ^ character as the delimiting character.
 *              The end of the text line or the begining of a
 *              comment returns causes a 'q' error.
 */

int
getdlm()
{
        int c;

        if (more()) {
                if ((c = getnb()) == '^') {
                        c = get();
                }
        } else {
                c = '\0';
        }
        if (c == '\0') {
                qerr();
        }
        return (c);
}

/*)Function     int     getnb()
 *
 *      The function getnb() scans the current assembler-source
 *      text line returning the first character not a SPACE or TAB.
 *
 *      local variables:
 *              int     c               current character from
 *                                      assembler-source text line
 *
 *      global variables:
 *              none
 *
 *      called functions:
 *              int     get()           aslex.c
 *
 *      side effects:
 *              use of get() updates the global pointer ip, the position
 *              in the current assembler-source text line
 */

int
getnb(void)
{
        int c;

        while ((c=get()) == ' ' || c == '\t')
                ;
        return (c);
}

/*)Function     int     get()
 *
 *      The function get() returns the next character in the
 *      assembler-source text line, at the end of the line a
 *      NULL character is returned.
 *
 *      local variables:
 *              int     c               current character from
 *                                      assembler-source text line
 *
 *      global variables:
 *              char *  ip              pointer into the current
 *                                      assembler-source text line
 *
 *      called functions:
 *              none
 *
 *      side effects:
 *              updates ip to the next character position in the
 *              assembler-source text line.  If ip is at the end of the
 *              line, ip is not updated.
 */

int
get(void)
{
        int c;

        if ((c = *ip) != 0)
                ++ip;
        return (c & 0x00FF);
}

/*)Function     VOID    unget(c)
 *
 *              int     c               value of last character read from
 *                                      assembler-source text line
 *
 *      If (c) is not a NULL character then the global pointer ip
 *      is updated to point to the preceeding character in the
 *      assembler-source text line.
 *
 *      NOTE:   This function does not push the character (c)
 *              back into the assembler-source text line, only
 *              the pointer ip is changed.
 *
 *      local variables:
 *              int     c               last character read from
 *                                      assembler-source text line
 *
 *      global variables:
 *              char *  ip              position into the current
 *                                      assembler-source text line
 *
 *      called functions:
 *              none
 *
 *      side effects:
 *              ip decremented by 1 character position
 */

VOID
unget(int c)
{
        if (c)
                if (ip != ib)
                        --ip;
}

/*)Function     int     getmap(d)
 *
 *              int     d               value to compare with the
 *                                      assembler-source text line character
 *
 *      The function getmap() converts the 'C' style characters \b, \f,
 *      \n, \r, and \t to their equivalent ascii values and also
 *      converts 'C' style octal constants '\123' to their equivalent
 *      numeric values.  If the first character is equivalent to (d) then
 *      a (-1) is returned, if the end of the line is detected then
 *      a 'q' error terminates the parse for this line, or if the first
 *      character is not a \ then the character value is returned.
 *
 *      local variables:
 *              int     c               value of character from the
 *                                      assembler-source text line
 *              int     n               looping counter
 *              int     v               current value of numeric conversion
 *
 *      global variables:
 *              none
 *
 *      called functions:
 *              int     get()           aslex.c
 *              VOID    qerr()          assubr.c
 *
 *      side effects:
 *              use of get() updates the global pointer ip the position
 *              in the current assembler-source text line
 */

int
getmap(int d)
{
        int c, n, v;

        if ((c=get()) == '\0')
                qerr();
        if (c == d)
                return (-1);
        if (c == '\\') {
                c = get();
                switch (c) {

                case 'b':
                        c = '\b';
                        break;

                case 'f':
                        c = '\f';
                        break;

                case 'n':
                        c = '\n';
                        break;

                case 'r':
                        c = '\r';
                        break;

                case 't':
                        c = '\t';
                        break;

                case '0':
                case '1':
                case '2':
                case '3':
                case '4':
                case '5':
                case '6':
                case '7':
                        n = 0;
                        v = 0;
                        while (++n<=3 && c>='0' && c<='7') {
                                v = (v<<3) + c - '0';
                                c = get();
                        }
                        unget(c);
                        c = v;
                        break;

                default:
                        unget(c);
                        c = '\\';
                        break;
                }
        }
        return (c);
}

/*)Function     int     comma(flag)
 *
 *              int     flag            when flag is non zero a 'q' error is
 *                                      generated if a COMMA is not found.
 *
 *      The function comma() skips SPACEs and TABs and returns
 *      a '1' if the next character is a COMMA else a '0' is
 *      returned.  If a COMMA is not found and flag is non zero
 *      then a 'q' error is reported.
 *
 *      local variables:
 *              int     c               last character read from
 *                                      assembler-source text line
 *
 *      global variables:
 *              none
 *
 *      called functions:
 *              int     getnb()         aslex.c
 *              VOID    qerr()          assubr.c
 *              VOID    unget()         aslex.c
 *
 *      side effects:
 *              assembler-source text line pointer updated
 */

int
comma(int flag)
{
        int c;

        if ((c = getnb()) != ',') {
                if (flag) {
                        qerr();
                } else {
                        unget(c);
                }
                return(0);
        }
        return(1);
}

/*)Function     int     nxtline()
 *
 *      The function nxtline() reads a line of assembler-source text
 *      from an assembly source text file, include file, or macro.
 *      Lines of text are processed from assembler-source files until
 *      all files have been read.  If an include file is opened then
 *      lines of text are read from the include file (or nested
 *      include file) until the end of the include file is found.
 *      The input text line is transferred into the global string
 *      ib[] and converted to a NULL terminated string.  The string
 *      is then copied into the global string ic[] which is used
 *      for internal processing by the assembler.  The function
 *      nxtline() returns a (1) after succesfully reading
 *      a line, or a (0) if all files have been read.
 *
 *      local variables:
 *              int     len             string length
 *              struct asmf     *asmt   temporary pointer to the processing structure
 *
 *      global variables:
 *              char    afn[]           afile() constructed filespec
 *              int     afp             afile constructed path length
 *              asmf *  asmc            pointer to current assembler file structure
 *              asmf *  asmi            pointer to a queued include file structure
 *              asmf *  asmq            pointer to a queued macro structure
 *              char *  ib              string buffer containing
 *                                      assembler-source text line for processing
 *              char *  ic              string buffer containing
 *                                      assembler-source text line for listing
 *              int     asmline         source file line number
 *              int     incfil          current include file count
 *              int     incline         include file line number
 *              int     lnlist          LIST-NLIST state
 *              int     mcrline         macro line number
 *              int     srcline         current source line number
 *              int     uflag           -u, disable .list/.nlist processing
 *
 *      called functions:
 *              int     dbuf_init()
 *              int     dbuf_set_length()
 *              int     dbuf_getline()
 *              const char * dbuf_c_str()
 *              int     dbuf_append_str()
 *              int     fclose()        c-library
 *              char *  fgetm()         asmcro.c
 *              char *  strcpy()        c_library
 *
 *      side effects:
 *              include file will be closed at detection of end of file.
 *              the next sequential source file may be selected.
 *              The current file specification afn[] and the path
 *              length afp may be changed.
 *              The respective line counter will be updated.
 *
 * --------------------------------------------------------------
 *
 * How the assembler sequences the command line assembler
 * source files, include files, and macros is shown in a
 * simplified manner in the following.
 *
 *      main[asmain] sequences the command line files by creating
 *      a linked list of asmf structures, one for each file.
 *
 *      asmf structures:
 *                   -------------       -------------               -------------
 *                  | File 1      |     | File 2      |             | File N      |
 *       ------     |       ------|     |       ------|             |       ------|      
 *      | asmp | -->|      | next | --> |      | next | --> ... --> |      | NULL |
 *       ------      -------------       -------------               -------------
 *
 *      At the beginning of each assembler pass set asmc = asmp
 *      and process the files in sequence.
 *
 *      If the source file invokes the .include directive to process a
 *      file then a new asmf structure is prepended to the asmc structure
 *      currently being processed.  At the end of the include file the
 *      processing resumes at the point the asmc structure was interrupted.
 *      This is shown in the following:
 *
 *                   ------------- 
 *                  | Incl File 1 |
 *                  |       ------|
 *                  |      | next |
 *                   ------------- 
 *                             |
 *      asmf structures:       |
 *                             V
 *                   -------------       -------------               -------------
 *                  | File 1      |     | File 2      |             | File N      |
 *       ------     |       ------|     |       ------|             |       ------|      
 *      | asmp | -->|      | next | --> |      | next | --> ... --> |      | NULL |
 *       ------      -------------       -------------               -------------
 *
 *      At the .include point link the asmi structure to asmc
 *      and then set asmc = asmi (the include file asmf structure).
 *
 *      If a source file invokes a macro then a new asmf structure is
 *      prepended to the asmc structure currently being processed.  At the
 *      end of the macro the processing resumes at the point the asmc
 *      structure was interrupted.
 *      This is shown in the following:
 *
 *                   -------------       -------------
 *                  | Incl File 1 |     |    Macro    |
 *                  |       ------|     |       ------|
 *                  |      | next |     |      | next |
 *                   -------------       -------------
 *                             |                   |
 *      asmf structures:       |                   |
 *                             V                   V
 *                   -------------       -------------               -------------
 *                  | File 1      |     | File 2      |             | File N      |
 *       ------     |       ------|     |       ------|             |       ------|      
 *      | asmp | -->|      | next | --> |      | next | --> ... --> |      | NULL |
 *       ------      -------------       -------------               -------------
 *
 *      At the macro point link the asmq structure to asmc
 *      and then set asmc = asmq (the macro asmf structure).
 *
 *      Note that both include files and macros can be nested.
 *      Macros may be invoked within include files and include
 *      files can be invoked within macros.
 *
 *      Include files are opened, read, and closed on each pass
 *      of the assembler.
 *
 *      Macros are recreated during each pass of the assembler.
 */

int
nxtline(void)
{
        static struct dbuf_s dbuf_ib;
        static struct dbuf_s dbuf_ic;
        size_t len = 0;
        struct asmf *asmt;

        if (!dbuf_is_initialized (&dbuf_ib))
                dbuf_init (&dbuf_ib, 1024);
        if (!dbuf_is_initialized (&dbuf_ic))
                dbuf_init (&dbuf_ic, 1024);
        dbuf_set_length (&dbuf_ib, 0);
        dbuf_set_length (&dbuf_ic, 0);

loop:   if (asmc == NULL) return(0);

        /*
         * Insert Include File
         */
        if (asmi != NULL) {
                asmc = asmi;
                asmi = NULL;
                incline = 0;
        }
        /*
         * Insert Queued Macro
         */
        if (asmq != NULL) {
                asmc = asmq;
                asmq = NULL;
                mcrline = 0;
        }

        switch(asmc->objtyp) {
        case T_ASM:
                if ((len = dbuf_getline (&dbuf_ib, asmc->fp)) == 0) {
                        if ((asmc->flevel != flevel) || (asmc->tlevel != tlevel)) {
                                err('i');
                                fprintf(stderr, "?ASxxxx-Error-<i> at end of assembler file\n");
                                fprintf(stderr, "              %s\n", geterr('i'));
                        }
                        flevel = asmc->flevel;
                        tlevel = asmc->tlevel;
                        lnlist = asmc->lnlist;
                        asmc = asmc->next;
                        if (asmc != NULL) {
                                asmline = 0;
                        }
                        if ((lnlist & LIST_PAG) || (uflag == 1)) {
                                lop = NLPP;
                        }
                        goto loop;
                } else {
                        if (asmline++ == 0) {
                                strcpy(afn, asmc->afn);
                                afp = asmc->afp;
                        }
                        srcline = asmline;
                }
                break;

        case T_INCL:
                if ((len = dbuf_getline (&dbuf_ib, asmc->fp)) == 0) {
                        fclose(asmc->fp);
                        incfil -= 1;
                        if ((asmc->flevel != flevel) || (asmc->tlevel != tlevel)) {
                                err('i');
                                fprintf(stderr, "?ASxxxx-Error-<i> at end of include file\n");
                                fprintf(stderr, "              %s\n", geterr('i'));
                        }
                        srcline = asmc->line;
                        flevel = asmc->flevel;
                        tlevel = asmc->tlevel;
                        lnlist = asmc->lnlist;
                        asmc = asmc->next;
                        switch (asmc->objtyp) {
                        default:
                        case T_ASM:     asmline = srcline;      break;
                        case T_INCL:    incline = srcline;      break;
                        case T_MACRO:   mcrline = srcline;      break;
                        }
                        /*
                         * Scan for parent file
                         */
                        asmt = asmc;
                        while (asmt != NULL) {
                                if (asmt->objtyp != T_MACRO) {
                                        strcpy(afn, asmt->afn);
                                        afp = asmt->afp;
                                        break;
                                }
                                asmt = asmt->next;
                        }
                        if ((lnlist & LIST_PAG) || (uflag == 1)) {
                                lop = NLPP;
                        }
                        goto loop;
                } else {
                        if (incline++ == 0) {
                                strcpy(afn, asmc->afn);
                                afp = asmc->afp;
                        }
                        srcline = incline;
                }
                break;

        case T_MACRO:
                dbuf_append(&dbuf_ib, "\0", dbuf_ib.alloc - 1);
                ib = (char *)dbuf_c_str (&dbuf_ib);
                ib = fgetm(ib, dbuf_ib.alloc - 1, asmc->fp);
                if (ib == NULL) {
                        dbuf_set_length(&dbuf_ib, 0);
                        mcrfil -= 1;
                        srcline = asmc->line;
                        flevel = asmc->flevel;
                        tlevel = asmc->tlevel;
                        lnlist = asmc->lnlist;
                        asmc = asmc->next;
                        switch (asmc->objtyp) {
                        default:
                        case T_ASM:     asmline = srcline;      break;
                        case T_INCL:    incline = srcline;      break;
                        case T_MACRO:   mcrline = srcline;      break;
                        }
                        goto loop;
                } else {
                        len = strlen(ib);
                        dbuf_set_length(&dbuf_ib, len);
                        if (mcrline++ == 0) {
                                ;
                        }
                        srcline = mcrline;
                }
                break;

        default:
                fprintf(stderr, "?ASxxxx-Internal-nxtline(objtyp)-Error.\n\n");
                asexit(ER_FATAL);
                break;
        }
        ib = (char *)dbuf_c_str (&dbuf_ib);

        /* remove the trailing NL */
        if (len > 0 && '\n' == ib[len - 1])
          {
            --len;
            if (len > 0 && '\r' == ib[len - 1])
              --len;
            dbuf_set_length (&dbuf_ib, len);
            ib = (char *)dbuf_c_str (&dbuf_ib);
          }

        dbuf_append_str (&dbuf_ic, ib);
        ic = (char *)dbuf_c_str (&dbuf_ic);
        return(1);
}


/*)Function:    int     getlnm()
 *
 *      The function getlnm() returns the line number of the
 *      originating assembler or include file.
 *
 *      local variables:
 *              struct asmf     *asmt   temporary pointer to the processing structure
 *
 *      global variables:
 *              struct asmf     *asmc   pointer to the current input processing structure
 *              int             asmline line number in current assembler file
 *              int             line    line number
 *
 *      functions called:
 *              none
 *
 *      side effects:
 *              Sets line to the source file line number.
 */

int
getlnm()
{
        struct asmf *asmt;

        line = srcline;
        if (asmc->objtyp == T_MACRO) {
                asmt = asmc->next;
                while (asmt != NULL) {
                        switch (asmt->objtyp) {
                        case T_ASM:     return(line = asmline);
                        case T_INCL:    return(line = asmt->line);
                        default:        asmt = asmt->next;              break;
                        }
                }
        }
        return(line);
}

/*)Function     int     more()
 *
 *      The function more() scans the assembler-source text line
 *      skipping white space (SPACES and TABS) and returns a (0)
 *      if the end of the line or a comment delimeter (;) is found,
 *      or a (1) if their are additional characters in the line.
 *
 *      local variables:
 *              int     c               next character from the
 *                                      assembler-source text line
 *
 *      global variables:
 *              none
 *
 *      called functions:
 *              int     getnb()         aslex.c
 *              VOID    unget()         aslex.c
 *
 *      side effects:
 *              use of getnb() and unget() updates the global pointer ip
 *              the position in the current assembler-source text line
 */

int
more(void)
{
        int c;

        c = getnb();
        unget(c);
        return( (c == '\0' || c == ';') ? 0 : 1 );
}

/*)Function     char    endline()
 *
 *      The function endline() scans the assembler-source text line
 *      skipping white space (SPACES and TABS) and returns the next
 *      character or a (0) if the end of the line is found or a
 *      comment delimiter (;) is found.
 *
 *      local variables:
 *              int     c               next character from the
 *                                      assembler-source text line
 *
 *      global variables:
 *              none
 *
 *      called functions:
 *              int     getnb()         aslex.c
 *
 *      side effects:
 *              use of getnb() updates the global pointer ip the
 *              position in the current assembler-source text line
 */

char
endline(void)
{
        int c;

        c = getnb();
        return( (c == '\0' || c == ';') ? 0 : c );
}

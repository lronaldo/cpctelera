/* asmcro.c */

/*
 *  Copyright (C) 2010  Alan R. Baldwin
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

/*)Module       asmcro.c
 *
 *      The module asmcro.c includes the macro processor.
 *
 *      asmcro.c contains the following functions:
 *              char   *fgetm()         get a macro text line
 *              VOID    getdarg()       get a macro definition argument
 *              VOID    getxarg()       get a macro expansion argument
 *              VOID    getxstr()       get a macro expansion string
 *              int     macro()         run a macro
 *              VOID    macroscn()      scan macro text line for symbols
 *              int     macrosub()      substitute macro expansion arguments
 *              VOID    mcrinit()       initialize macro processing variables
 *              int     mcrprc()        macro processing control
 *              char   *mstring()       store a macro string
 *              char   *mstruct()       allocate macro space
 *              mcrdef *newdef()        initialize a macro definition
 *              mcrdef *nlookup()       lookup a macro
 */

/*)Function     int     mcrprc(code))
 *
 *              int     code            function code
 *
 *      The function of mcrprc() is to evaluate the
 *      following assembler directives:
 *
 *              .macro                  define a general macro
 *              .irp                    define an inline indefinite repeat macro by arguments
 *              .irpc                   define an inline indefinite repeat macro by characters
 *              .rept                   define an inline repeating macro
 *              .mexit                  exit to end of macro
 *              .endm                   end of macro
 *              .nchr                   assign number of characters in a string to a symbol
 *              .narg                   assign number of expansion arguments to a symbol
 *              .ntyp                   assign 0/1 if absolute/relocatable symbol
 *              .nval                   assign value of argument to an absolute symbol
 *              .mdelete                delete a macro definition
 *
 *      And to control the building and exiting of a macro
 *      by the use of the internal assembler directives:
 *
 *              O_BUILD                 building a macro processing
 *              O_EXIT                  exit a macro processing
 *
 *      local variables:
 *              int     d               character string delimiter
 *              expr    e1              expression structure
 *              char    id[]            id string
 *              mne    *mp              pointer to an assembler mnemonic structure
 *              macrofp *nfp            macro pseudo FILE Handle
 *              mcrdef *nq              pointer to a macro definition structure
 *              mcrdef *np              pointer to a macro definition structure
 *              int     rptcnt          repeat count evaluation
 *              sym    *sp              pointer to a symbol structure
 *              strlst *str             string list structure
 *
 *      global variables:
 *              asmf *  asmc            current asmf structure
 *              sym     dot             current program counter value structure
 *              a_uint  laddr           equate value
 *              int     lmode           assembler list mode
 *              mcrdef  mcrlst          pointer the macro definition linked list
 *              mcrdef  mcrp            pointer to macro definition being built
 *
 *      functions called:
 *              a_uint  absexpr()       asexpr.c
 *              VOID    clrexpr()       asexpr.c
 *              int     comma()         aslex.c
 *              VOID    getdarg()       asmcro.c
 *              int     getid()         aslex.c
 *              int     getdlm()        aslex.c
 *              int     getmap()        aslex.c
 *              int     getnb()         aslex.c
 *              VOID    getxarg()       asmcro.c
 *              sym *   lookup()        assym.c
 *              VOID    macro()         asmcro.c
 *              mne *   mlookup()       assym.c
 *              int     more()          aslex.c
 *              char *  mstring()       asmcro.c
 *              VOID *  mstruct()       asmcro.c
 *              mcrdef *newdef()        asmcro.c
 *              VOID    qerr()          assubr.c
 *              VOID    unget()         aslex.c
 *
 *      side effects:
 *              Macro directives processed and
 *              macro structures created.
 */
int
mcrprc(code)
int code;
{
        struct mcrdef *np,*nq;
        struct macrofp *nfp;
        struct strlst *str;
        struct mne *mp;
        struct sym *sp;
        struct expr e1;
        char id[NCPS];
        int d, rptcnt;

        switch(code) {
        case O_MACRO:
                /*
                 * Valid .macro definition forms:
                 *
                 *      .macro  mne(,)
                 *      .macro  mne(,)  arg1(,) arg2(,) ((,) arg3(,) ...(,) argn)
                 *
                 *      where 'mne' is a string beginning with a
                 *      LETTER followed by LETTERs or DIGITs.
                 *
                 *      If the 'arg' is immediately preceeded by a '?'
                 *      (ie ?arg) then the argument is a dummy argument.
                 *      Dummy arguments left blank become local symbols
                 *      when the macro is expanded.
                 */
                /*
                 * Get macro mnemonic
                 */
                if (more()) {
                        getid(id, getnb());
                } else {
                        qerr();
                }
                np = newdef(code, id);
                /*
                 * Get macro definition arguments
                 */
                while (more()) {
                        getdarg(np);
                }
                break;

        case O_IRP:
                /*
                 * Valid .irp definition forms:
                 *
                 *      .irp    sym(,)
                 *      .irp    sym(,)  arg1(,) arg2 ((,) arg3(,) ...(,) argn)
                 *
                 *      where 'sym' is a string beginning with
                 *      a LETTER followed by LETTERs or DIGITs.
                 *
                 *      The complete .irp definition is processed once
                 *      for each argument in the .irp definition.
                 */
                /*
                 * An inline definition
                 */
                np = newdef(code, NULL);
                /*
                 * Get macro definition argument
                 */
                getdarg(np);
                /*
                 * Get expansion arguments
                 */
                while (more()) {
                        getxarg(np);
                }
                np->rptcnt = np->xarg;
                break;

        case O_IRPC:
                /*
                 * Valid .irpc definition forms:
                 *
                 *      .irpc   sym(,)
                 *      .irpc   sym(,)  character_string
                 *
                 *      where 'sym' is a string beginning with
                 *      a LETTER followed by LETTERs or DIGITs.
                 *
                 *      The complete .irpc definition is processed once
                 *      for each character in the character_string.
                 *
                 *      If the character_string contains white space
                 *      or commas then the character string must be
                 *      delimited as ^/character_string/ where the
                 *      delimiting character ('/') must not be in
                 *      the string.
                 */
                /*
                 * An inline definition
                 */
                np = newdef(code, NULL);
                /*
                 * Get macro definition argument
                 */
                getdarg(np);
                /*
                 * Get expansion argument
                 */
                getxarg(np);
                np->rptcnt = (np->bgnxrg != NULL) ? strlen(np->bgnxrg->text) : 0;
                break;

        case O_REPT:
                /*
                 * Valid .rept definition forms:
                 *
                 *      .rept   sym
                 *
                 *      where 'sym' is a symbol or expression which
                 *      is evalauated to an absolute value.
                 *
                 *      The complete .rept definition is processed
                 *      value times.
                 */
                /*
                 * An inline definition
                 */
                np = newdef(code, NULL);
                /*
                 * Get repeat count
                 */
                if (more()) {
                        unget(getnb());
                        rptcnt = (int) absexpr();
                        if (rptcnt < 0) {
                                rptcnt = 0;
                        }
                } else {
                        err('o');
                        rptcnt = 0;
                }
                np->rptcnt = rptcnt;
                break;

        case O_ENDM:
                if (asmc->objtyp != T_MACRO) {
                        err('n');
                } else {
                        lmode = NLIST;
                }
                break;

        case O_MEXIT:
                if (asmc->objtyp == T_MACRO) {
                        nfp = (struct macrofp *) asmc->fp;
                        nfp->npexit = 1;
                        nfp->lstptr = nfp->np->endlst;
                } else {
                        err('n');
                }
                break;

        case O_NCHR:
                getid(id, -1);
                sp = lookup(id);
                if (sp == &dot) {
                        err('.');
                        break;
                }
                sp->s_flag |= S_ASG;
                sp->s_type = S_USER;
                comma(0);
                d = getdlm();
                rptcnt = 0;
                while (getmap(d) >= 0) {
                        rptcnt += 1;
                }
                lmode = ELIST;
                laddr = sp->s_addr = rptcnt;
                break;

        case O_NARG:
                nfp = (struct macrofp *) asmc->fp;
                np = nfp->np;
                if (asmc->objtyp != T_MACRO) {
                        err('n');
                        break;
                } else
                if (np->type != O_MACRO) {
                        err('n');
                        break;
                }
                getid(id, -1);
                sp = lookup(id);
                if (sp == &dot) {
                        err('.');
                        break;
                }
                sp->s_flag |= S_ASG;
                sp->s_type = S_USER;
                lmode = ELIST;
                laddr = sp->s_addr = np->xarg;
                break;

        case O_NTYP:
                getid(id, -1);
                sp = lookup(id);
                sp->s_flag |= S_ASG;
                sp->s_type = S_USER;
                sp->s_area = NULL;
                comma(0);
                clrexpr(&e1);
                expr(&e1, 0);
                lmode = ELIST;
                laddr = sp->s_addr = (e1.e_flag || e1.e_base.e_ap) ? 1 : 0;
                break;

        case O_NVAL:
                getid(id, -1);
                sp = lookup(id);
                if (sp == &dot) {
                        err('.');
                        break;
                }
                sp->s_flag |= S_ASG;
                sp->s_type = S_USER;
                sp->s_area = NULL;
                comma(0);
                clrexpr(&e1);
                expr(&e1, 0);
                lmode = ELIST;
                laddr = sp->s_addr = e1.e_addr;
                break;

        case O_MDEL:
                while (more()) {
                        getid(id, getnb());
                        np = nlookup(id);
                        if (np != NULL) {
                                if (np == mcrlst) {
                                        mcrlst = np->next;
                                } else {
                                        nq = mcrlst;
                                        while (nq != NULL) {
                                                if (np == nq->next) {
                                                        nq->next = np->next;
                                                        break;
                                                }
                                                nq = nq->next;
                                        }
                                }
                        }
                        if (more() && comma(0) && !more()) {
                                qerr();
                        }
                }
                break;

        case O_CHECK:
                if (mcrp == NULL) {
                        if (asmc->objtyp == T_MACRO) {
                                nfp = (struct macrofp *) asmc->fp;
                                if (nfp->npexit) {
                                        lmode = NLIST;
                                        return(1);
                                }
                        }
                        return(0);
                }
                while (more()) {
                        getid(id, getnb());
                        if (((mp = mlookup(id)) != NULL) &&
                             (mp->m_type == S_MACRO)) {
                                switch (mp->m_valu) {
                                case O_MACRO:
                                case O_IRP:
                                case O_IRPC:
                                case O_REPT:
                                        mcrp->nest += 1;
                                        break;
                                case O_ENDM:
                                        mcrp->nest -= 1;
                                        break;
                                default:
                                        break;
                                }
                                break;
                        }
                }
                /*
                 * Append to MACRO Definition
                 */
                str = (struct strlst *) mstruct (sizeof (struct strlst));
                str->next = NULL;
                str->text = mstring(ib);
                if (mcrp->bgnlst == NULL) {
                        mcrp->bgnlst = str;
                } else {
                        mcrp->endlst->next = str; 
                }
                mcrp->endlst = str;
                /*
                 * Check for .ENDM
                 */
                if (mcrp->nest == 0) {
                        switch (mcrp->type) {
                        case O_MACRO:
                                mcrp->next = mcrlst;
                                mcrlst = mcrp;
                                break;
                        case O_IRP:
                        case O_IRPC:
                        case O_REPT:
                                if (mcrp->rptcnt != 0) {
                                        macro(mcrp);
                                }
                                break;
                        default:
                                break;
                        }
                        mcrp = NULL;
                }
                if ((lnlist & LIST_MD) == 0) {
                        lmode = NLIST;
                }
                return(1);

        default:
                break;
        }
        return(0);
}

/*)Function     VOID    getdarg(np)
 *
 *              struct mcrdef *np       pointer to macro definition structure
 *
 *      The function getdarg() gets the next macro definition
 *      argument from the assembler input text.  The definiton
 *      may be any valid label or symbol (excluding temporary
 *      symbols).
 *
 *      local variables:
 *              strlst *str             text line structure
 *              char    id[]            text string
 *              int     c               character
 *
 *      global variables:
 *              char    ctype[]         charcter type array
 *              mcrdef  mcrp            link to macro definition being built
 *
 *      functions called:
 *              int     comma()         aslex.c
 *              int     getid()         aslex.c
 *              int     getnb()         aslex.c
 *              char *  mstring()       asmcro.c
 *              VOID *  mstruct()       asmcro.c
 *              VOID    qerr()          assubr.c
 *
 *      side effects:
 *              Macro definition argument is added to macro definition
 *              structure and the number of arguments is incremented.
 *              Failure to allocate space for the argument string will
 *              terminate the assembler.
 */

VOID
getdarg(np)
struct mcrdef * np;
{
        struct strlst *str;
        char id[NCPS];
        int c;

        /*
         * Skip leading ','
         */
        comma(0);
        if (more()) {
                if (((c=getnb()) == '?') && (ctype[c=get()] & LETTER)) {
                        unget(c);
                        c = '?';
                } else
                if (ctype[c] & LETTER) {
                        ;
                } else {
                        qerr();
                }
                getid(id, c);
        } else {
                qerr();
        }
        str = (struct strlst *) mstruct (sizeof (struct strlst));
        str->next = NULL;
        str->text = mstring(id);
        if (np->bgnarg == NULL) {
                np->bgnarg = str;
        } else {
                np->endarg->next = str; 
        }
        np->endarg = str;
        np->narg += 1;
}

/*)Function     VOID    getxarg(np)
 *
 *              struct mcrdef *np       pointer to macro definition structure
 *
 *      The function getxarg() gets the next macro expansion
 *      argument from the assembler input text.  The expansion
 *      may contain any ASCII character including the space and
 *      tab characters.  If the argument contains a comma then
 *      the argument string must be delimited using the form
 *
 *              ^/ ... / where the character '/' may be any
 *              printing character not in the delimited string.
 *
 *      If the undelimited string is of the form \arg then
 *      the argument is evaluated and represented by an
 *      unsigned integer in the current radix.
 *
 *      local variables:
 *              char    id[]            text string
 *              char *  frmt            format string pointer
 *              char *  sip             save ip pointer
 *              strlst *str             text line structure
 *
 *      global variables:
 *              char *  ip              source text line pointer
 *
 *      functions called:
 *              a_uint  absexpr()       asexpr.c
 *              VOID    getxstr()       asmcro.c
 *              char *  mstring()       asmcro.c
 *              VOID *  mstruct()       asmcro.c
 *              int     sprintf()       c_library
 *
 *      side effects:
 *              Macro expansion argument is added to macro definition
 *              structure and the number of arguments is incremented.
 *              Failure to allocate space for the argument string will
 *              terminate the assembler.
 */

VOID
getxarg(np)
struct mcrdef * np;
{
        struct strlst *str;
        char id[NCPS];
        char *frmt;
        char *sip;

        /*
         * Get the argument string
         */
        getxstr(id);

        str = (struct strlst *) mstruct (sizeof (struct strlst));
        str->next = NULL;
        if (*id == '\\') {
                sip = ip;
                ip = id + 1;
#ifdef  LONGINT
                switch (radix) {
                default:
                case 10:        frmt = "%lu";   break;
                case 8:         frmt = "%lo";   break;
                case 16:        frmt = "%lX";   break;
                }
#else
                switch (radix) {
                default:
                case 10:        frmt = "%u";    break;
                case 8:         frmt = "%o";    break;
                case 16:        frmt = "%X";    break;
                }
#endif
                sprintf(id, frmt, absexpr());
                ip = sip;
        }
        str->text = mstring(id);
        if (np->bgnxrg == NULL) {
                np->bgnxrg = str;
        } else {
                np->endxrg->next = str; 
        }
        np->endxrg = str;
        np->xarg += 1;
}

/*)Function     VOID    getxstr(id)
 *
 *              char * id               pointer to string
 *
 *      The function getxstr() processes the next macro expansion
 *      argument from the assembler input text.  The expansion
 *      may contain any ASCII character including the space and
 *      tab characters.  If the argument contains a comma then
 *      the argument string must be delimited using the form
 *
 *              ^/ ... / where the character '/' may be any
 *              printing character not in the delimited string.
 *
 *      local variables:
 *              int     c               character
 *              int     dc              delimiting character
 *              char *  p               character string pointer
 *
 *      global variables:
 *              char    ctype[]         charcter type array
 *
 *      functions called:
 *              int     comma()         aslex.c
 *              int     get()           aslex.c
 *              int     getnb()         aslex.c
 *              VOID    qerr()          assubr.c
 *              VOID    unget()         assym.c
 *
 *      side effects:
 *              Macro expansion argument is returned in id[].
 */

VOID
getxstr(id)
char *id;
{
        char *p;
        int c, dc;

        /*
         * The argument delimiters are SPACE, TAB, and ','.
         * If the argument contains a SPACE, TAB, or ',' then
         * the argument must be enclosed within a delimiter of
         * the form ^/ ... / where the character '/' may
         * be any character not in the delimited string.
         */
        p = id;
        /*
         * Skip leading ','
         */
        comma(0);
        switch (c=getnb()) {
        case '^':       dc = get();     break;
        default:        dc = ',';       break;
        }
        switch (c) {
        case '^':
                while ((c=get()) != '\0') {
                        if (c == dc) {
                                break;
                        }
                        *p++ = c;
                }
                if (ctype[c] & ILL) {
                        qerr();
                }
                break;
        default:
                unget(c);
                while ((c=get()) != '\0') {
                        if ((c == dc) ||
                            (c == ' ') ||
                            (c == '\t') ||
                            (c == ';')) {
                                unget(c);
                                break;
                        }
                        *p++ = c;
                }
                break;
        }
        *p = '\0';
}

/*)Function     VOID    macro(np)
 *
 *              mcrdef *np              macro definition structure
 *
 *      macro() prepares the macro described by
 *      np for insertion into the code stream.
 *
 *      local variables:
 *              macrofp *nfp            pseudo FILE Handle
 *              strlst *str             missing argument expansion string
 *              strlst *arg             macro definition argument string
 *              strlst *xrg             macro expansion  argument string
 *              char    xrgstr[]        dumby argument evaluation string
 *
 *      global variables:
 *              asmf    asmq            queued macro structure
 *              int     srcline         current assembler line number
 *              int     flevel          current IF-ELSE-ENDIF level
 *              int     tlevel          current IF-ELSE-ENDIF level index
 *              int     maxmcr          maximum macro nesting level encountered
 *              int     mcrfil          macro nesting counter
 *              int     lnlist          current LIST-NLIST flags
 *
 *      functions called:
 *              VOID    getxarg()       asmcro.c
 *              char *  mstring()       asmcro.c
 *              VOID *  mstruct()       asmcro.c
 *              char *  strcpy()        c_library
 *              int     sprintf()       c_library
 *              int     strlen()        c_library
 *              VOID    qerr()          assubr.c
 *
 *      side effects:
 *              Macro is inserted into assembler stream
 */

VOID
macro(np)
struct mcrdef * np;
{
        struct macrofp *nfp;
        struct strlst *str;
        struct strlst *arg;
        struct strlst *xrg;
        char xrgstr[NINPUT];

        if (++mcrfil > MAXMCR) {
                --mcrfil;
                err('m');
                return;
        }
        if (mcrfil > maxmcr) {
                maxmcr = mcrfil;
        }
        /*
         * Create an asmf structure for nxtline()
         */
        asmq = (struct asmf *) mstruct (sizeof (struct asmf));
        asmq->next = asmc;
        asmq->objtyp = T_MACRO;
        asmq->line = srcline;
        if (ftflevel != 0) {
                asmq->flevel = ftflevel - 1;
                ftflevel = 0;
        } else {
                asmq->flevel = flevel;
        }
        asmq->tlevel = tlevel;
        asmq->lnlist = lnlist;
        asmq->afp = 0;
        strcpy(asmq->afn,np->name);
        /*
         * Create a macrofp structure for fgetm()
         */
        nfp = (struct macrofp *) mstruct (sizeof (struct macrofp));
        nfp->np = np;
        nfp->lstptr = np->bgnlst;
        nfp->rptcnt = np->rptcnt;
        nfp->rptidx = 0;
        nfp->flevel = asmq->flevel;
        nfp->tlevel = asmq->tlevel;
        nfp->lnlist = asmq->lnlist;
        nfp->npexit = nfp->rptcnt ? 0 : 1;
        /*
         * Check if arguments are required
         */
        if (np->type == O_MACRO) {
                np->xarg = 0;
                np->bgnxrg = NULL;
                np->endxrg = NULL;
                while (more()) {
                        getxarg(np);
                }
                if (np->xarg > np->narg) {
                        qerr();
                }
                /*
                 * Fill in missing arguments and
                 * check for dummy arguments.
                 */
                arg = np->bgnarg;
                xrg = np->bgnxrg;
                while (arg != NULL) {
                        if (xrg == NULL) {
                                str = (struct strlst *) mstruct (sizeof (struct strlst));
                                str->next = NULL;
                                str->text = mstring("");
                                if (np->bgnxrg == NULL) {
                                        np->bgnxrg = str;
                                } else {
                                        np->endxrg->next = str; 
                                }
                                np->endxrg = str;
                                xrg = str;
                        }
                        if ((*arg->text == '?') && (strlen(xrg->text) == 0)) {
#ifdef  LONGINT
                                sprintf(xrgstr, "%lu$", mls.s_addr++);
#else
                                sprintf(xrgstr, "%u$", mls.s_addr++);
#endif
                                xrg->text = mstring(xrgstr);
                        }
                        arg = arg->next;
                        xrg = xrg->next;
                }
        }
        /*
         * Cast nfp as a FILE HANDLE
         */
        asmq->fp = (FILE *) nfp;

        return;
}

/*)Function     mcrdef *newdef(code, id)
 *
 *              int     code            macro type code
 *              char *  id              macro name string
 *
 *      The function mcrdef() creates a new macro
 *      definition structure and initializes it.
 *
 *      local variables:
 *              mne *   mp              pointer to a mnemonic structure
 *
 *      global variables:
 *              mcrdef *mcrp            pointer to the new macro definition structure
 *
 *      functions called:
 *              mne *   mlookup()       assym.c
 *              char *  mstring()       asmcro.c
 *              VOID *  mstruct()       asmcro.c
 *              mne *   nlookup()       asmcro.c
 *
 *      side effects:
 *              Macro definiton structure created
 *              and initialized.
 */

struct mcrdef *
newdef(code, id)
int code;
char *id;
{
        struct mne *mp;

        /*
         * New MACRO Definition
         */
        mcrp = (struct mcrdef *) mstruct (sizeof (struct mcrdef));
        mcrp->next = NULL;
        /*
         * Check for Assembler Directive Conflicts
         */
        if (id != NULL) {
                if (nlookup(id) != NULL) {
                        err('m');
                }
                mcrp->name = mstring(id);
                mp = mlookup(id);
                if (mp != NULL) {
                        if (mp->m_type < S_DIREOL) {
                                err('m');
                        }
                }
        } else {
                mcrp->name = mstring("");
        }
        mcrp->bgnlst = NULL;
        mcrp->endlst = NULL;
        mcrp->type = code;
        mcrp->rptcnt = 1;
        mcrp->nest = 1;
        mcrp->narg = 0;
        mcrp->bgnarg = NULL;
        mcrp->endarg = NULL;
        mcrp->xarg = 0;
        mcrp->bgnxrg = NULL;
        mcrp->endxrg = NULL;
        return (mcrp);
}

/*)Function     mcrdef *nlookup(id)
 *
 *              char *  id      macro name string
 *
 *      The function nlookup() searches the macro list
 *      for a match returning a pointer to the macro
 *      definition structure else it returns a NULL.
 *
 *      local variables:
 *              mcrdef *np      pointer to macro structure
 *
 *      global variables:
 *              none
 *
 *      functions called:
 *              symeq()         assym.c
 *
 *      side effects:
 *              none
 */

struct mcrdef *
nlookup(id)
char *id;
{
        struct mcrdef * np;

        np = mcrlst;
        while (np != NULL) {
                if (symeq(id, np->name, 1)) {
                        return (np);
                }
                np = np->next;
        }
        return (NULL);
}

/*)Function     char *fgetm(ptr, len, fp)
 *
 *              char *  ptr     pointer string address
 *              int     len     maximum number of characters to return
 *              FILE *  fp      pseudo FILE Handle
 *
 *      The function fgetm() reads characters from the pseudo
 *      stream fp into the string pointed to by ptr. The integer
 *      argument len indicates the maximum number of characters
 *      that the buffer ptr can store. Reading stops when an end
 *      of string or  len-1 characters were read. The string read
 *      is terminated with a 0.
 *
 *      Macro types O_MACRO, O_IRP, and O_IRPC will have
 *      the macro definition strings replaced by their
 *      respective macro expression strings.
 *
 *      When no more macro lines are available then
 *      the macro terminates by restoring the assembler
 *      conditional and listing state at the time the
 *      macro was invoked and a NULL is returned.
 *
 *      local variables:
 *              macrofp *nfp    pointer to the pseudo FILE Handle
 *              mcrdef *np      pointer to macro structure
 *
 *      global variables:
 *              char *  ib              string buffer containing
 *                                      assembler-source text line for processing
 *              char *  ip              pointer into the assembler-source
 *                                      text line in ib
 *              int     flevel          current IF-ELSE-ENDIF level
 *              int     tlevel          current IF-ELSE-ENDIF level index
 *              int     lnlist          current LIST-NLIST flags
 *              int     mcrline         current macro line number
 *
 *      functions called:
 *              asexit()                asmain.c
 *              fprintf()               c_library
 *              macroscn()              asmcro.c
 *              strncpy()               c_library
 *
 *      side effects:
 *              mcrline, the current macro line number
 *              is updated.
 */

char *
fgetm(ptr, len, fp)
char *ptr;
int len;
FILE *fp;
{
        struct macrofp *nfp;
        struct mcrdef *np;

        /*
         * macroscn() and macrosub()
         * require that ptr == ib !!!
         */
        if (ptr != ib) {
                fprintf(stderr, "?ASxxxx-Internal-fgetm(ptr)-Error.\n\n");
                asexit(ER_FATAL);
        }
        ip = ptr;

        nfp = (struct macrofp *) fp;
        np = nfp->np;

        if (nfp->lstptr == NULL) {
                if (nfp->npexit == 0) {
                        if ((flevel != nfp->flevel) ||
                            (tlevel != nfp->tlevel)) {
                                err('i');
                        }
                }
                /*
                 * Repeat macro until repeat count is zero or an
                 * .mexit has been processed then exit with NULL
                 */
                if ((--nfp->rptcnt <= 0) || (nfp->npexit != 0)) {
                        return(NULL);
                } else {
                        nfp->lstptr = np->bgnlst;
                        nfp->rptidx += 1;
                        mcrline = 0;
                }
                /*
                 * Reset IF-ELSE-ENDIF and LIST-NLIST levels
                 */
                flevel = nfp->flevel;
                tlevel = nfp->tlevel;
                lnlist = nfp->lnlist;
        }
        strncpy(ptr, nfp->lstptr->text, len);
        ptr[len-1] = '\0';
        nfp->lstptr = nfp->lstptr->next;
        /*
         * Macro String Processing
         */
        switch (np->type) {
        case O_MACRO:
        case O_IRP:
        case O_IRPC:
                macroscn(nfp);
                break;

        case O_REPT:
        default:
                break;
        }

        /*
         * Return Macro String
         */
        return(ptr);
}

/*)Function     VOID    macroscn(nfp)
 *
 *              struct  macrofp * nfp   a Macro 'FILE Handle'
 *
 *      The function mcroscn() scans the macro text line
 *      for a valid substitutable string.  The only valid targets
 *      for substitution strings are strings beginning with a
 *      LETTER and containing any combination of DIGITS and LETTERS.
 *      If a valid target is found then the function macrosub() is
 *      called to search the macro definition argument list.
 *
 *      local variables:
 *              int     c               temporary character value
 *              char    id[]            a string of maximum length NINPUT
 *
 *      global variables:
 *              char    ctype[]         a character array which defines the
 *                                      type of character being processed.
 *                                      The index is the character
 *                                      being processed.
 *
 *      called functions:
 *              int     endline()       aslex.c
 *              int     getid()         aslex.c
 *              int     macrosub()      asmcro.c
 *              int     unget()         aslex.c
 *
 *      side effects:
 *              The assembler-source text line may be updated
 *              and a substitution made for the string id[].
 */

VOID
macroscn(nfp)
struct  macrofp *nfp;
{
        int c;
        char id[NINPUT];

        while ((c = endline()) != 0) {
                if (ctype[c] & DIGIT) {
                        while (ctype[c] & (LETTER|DIGIT)) c = get();
                        unget(c);
                } else
                if (ctype[c] & LETTER) {
                        getid(id, c);
                        if (macrosub(id, nfp)) {
                                return;
                        }
                }
        }
        return;
}


/*)Function     int     macrosub(id, nfp)
 *
 *              char *  id              a pointer to the search string
 *                                      of maximum length NINPUT
 *              macrofp *nfp            a Macro 'FILE Handle'
 *
 *      The function macrosub() scans the current macro's argument
 *      definition list for a match to the string id[].  If a match
 *      is found then a substitution is made with the corresponding
 *      expansion argument.
 *
 *      local variables:
 *              int     arglen          definition argument string length
 *              int     indx            repeat argument index
 *              char *  p               pointer to definition argument string
 *              struct strlst *arg      pointer to macro definition arguments
 *              struct strlst *xrg      pointer to macro expansion arguments
 *              int     xrglen          length of xarg
 *              char    xrgstr[]        temporary argument string
 *
 *      global variables:
 *              char    ib[]            source text line
 *              char *  ip              pointer into the source text line
 *              int     zflag           case sensitivity flag
 *
 *      called functions:
 *              char *  strcat()        c_library
 *              char *  strcpy()        c_library
 *              int     strlen()        c_library
 *              int     symeq()         assym.c
 *
 *      side effects:
 *              The source text line may be updated with
 *              a substitution made for the string id[].
 *              If there is insufficient space to make
 *              the substitution then macrosub returns
 *              a 1, else 0.
 */

int
macrosub(id, nfp)
char *id;
struct  macrofp *nfp;
{
        char *p;
        char xrgstr[NINPUT*2];
        int indx;
        struct strlst *arg;
        int arglen;
        struct strlst *xrg;
        int xrglen;

        /*
         * Check for a macro substitution
         */
        arg = nfp->np->bgnarg;
        xrg = nfp->np->bgnxrg;
        while (arg != NULL) {
                *xrgstr = '\0';
                p = arg->text;
                if (nfp->np->type == O_MACRO) {
                        if (*p == '?') { ++p; }
                }
                if (symeq(id, p, zflag)) {
                        indx = nfp->rptidx;
                        /*
                         * Substitution string
                         */
                        switch (nfp->np->type) {
                        case O_IRP:
                                while ((--indx >= 0) && (xrg != NULL)) {
                                        xrg = xrg->next;
                                }
                                /* Continue into O_MACRO */
                        case O_MACRO:
                                if (xrg != NULL) {
                                        strcpy(xrgstr, xrg->text);
                                }
                                break;
                        case O_IRPC:
                                if (xrg != NULL) {
                                        xrgstr[0] = xrg->text[indx];
                                        xrgstr[1] = '\0';
                                }
                                break;
                        default:
                                break;
                        }
                        /*
                         * Lengths
                         */
                        arglen = strlen(id);
                        xrglen = strlen(xrgstr);
                        /*
                         * Verify string space is available
                         */
                        if ((strlen(ib) - arglen + xrglen) > (NINPUT*2 - 1)) {
                                return(1);
                        }
                        /*
                         * Beginning of Substitutable string
                         */
                        p  = ip - arglen;
                        /*
                         * Remove a leading '.
                         */
                        if (p != ib) {
                                p -= *(p - 1) == '\'' ? 1 : 0;
                        }
                        *p = 0;
                        /*
                         * Remove a trailing '.
                         */
                        ip += *ip == '\'' ? 1 : 0;
                        /*
                         * Append the tail of the original
                         * string to the new argument string
                         * and then replace the dummy argument
                         * and tail with this string.
                         */
                        strcat(xrgstr, ip);
                        strcat(ib, xrgstr);
                        /*
                         * Set pointer to first character
                         * after argument replacement.
                         */
                        ip = p + xrglen;
                        return(0);
                }
                arg = arg->next;
                xrg = xrg->next;
        }
        return(0);
}

/*)Function     VOID *  mhunk()
 *
 *      Allocate space.
 *      Return a pointer to the allocated space.
 *
 *      This function based on code by
 *              John L. Hartman
 *              jhartman at compuserve dot com
 *
 *      local variables:
 *              memlnk *lnk             memory link pointer
 *
 *      static variables:
 *              int     bytes           bytes remaining in buffer area
 *              char *  pnext           next location in buffer area
 *
 *      global variables:
 *              memlnk  mcrmem          pointer to 1K Byte block being allocated
 *              int     mcrblk          1K Byte block allocations
 *              memlnk  pmcrmem         pointer to first 1K Byte block allocated
 *
 *      functions called:
 *              VOID *  new()           assym.c
 *
 *      side effects:
 *              Space allocated for object.
 *              Out of Space terminates assembler.
 */
 
/*
 * To avoid wasting memory headers on small allocations
 * allocate a big chunk and parcel it out as required.
 *
 * Hunks are linked to allow reuse during each pass
 * of the assembler.
 */

#define MCR_SPC 1024

/*
 * MCR_MSK = 1  for a 2 byte boundary
 * MCR_MSK = 3  for a 4 byte boundary
 * MCR_MSK = 7  for a 8 byte boundary
 */

#define MCR_MSK 3

static  char *  pnext;
static  int     bytes;

VOID *
mhunk()
{
        struct memlnk *lnk;

        /*
         * 1st Call Initializes Linked Hunks
         */
        if (pmcrmem == NULL) {
                lnk = (struct memlnk *) new (sizeof(struct memlnk));
                lnk->ptr = (VOID *) new (MCR_SPC);
                lnk->next = NULL;
                pmcrmem = mcrmem = lnk;
                mcrblk = 1;
        } else
        /*
         * Start Reuse of Linked Hunks
         */
        if (mcrmem == NULL) {
                mcrmem = pmcrmem;
        } else
        /*
         * Allocate a New Hunk
         */
        if (mcrmem->next == NULL) {
                lnk = (struct memlnk *) new (sizeof(struct memlnk));
                lnk->ptr = (VOID *) new (MCR_SPC);
                lnk->next = NULL;
                mcrmem->next = lnk;
                mcrmem = lnk;
                mcrblk += 1;
        } else {
        /*
         * Reuse Next Hunk
         */
                mcrmem = mcrmem->next;
        }
        pnext = (char *) mcrmem->ptr;
        bytes = MCR_SPC;

        return(pnext);
}

/*)Function     char *  mstring(str)
 *
 *              char *  str             pointer to string to save
 *
 *      Allocate space for "str", copy str into new space.
 *      Return a pointer to the allocated string.
 *
 *      This function based on code by
 *              John L. Hartman
 *              jhartman at compuserve dot com
 *
 *      local variables:
 *              int     len             string length + 1
 *              int     bytes           bytes remaining in buffer area
 *
 *      static variables:
 *              char *  p               pointer to head of copied string
 *              char *  pnext           next location in buffer area
 *
 *      global variables:
 *              none
 *
 *      functions called:
 *              VOID *  mhunk()         asmcro.c
 *              char *  strcpy()        c_library
 *              int     strlen()        c_library
 *
 *      side effects:
 *              Space allocated for string, string copied
 *              to space.  Out of Space terminates assembler.
 */
 
char *
mstring(str)
char *str;
{
        int  len;
        char *p;
   
        /*
         * What we need, including a null.
         */
        len = strlen(str) + 1;

        if (len > bytes) {
                p = mhunk();
        } else {
                p = pnext;
        }
        pnext += len;
        bytes -= len;

        /*
         * Copy the name and terminating null.
         */
        strcpy(p, str);

        return(p);
}

/*)Function     char *  mstruct(n)
 *
 *              int     n               size required
 *
 *      Allocate n bytes of space.
 *      Return a pointer to the allocated space.
 *      Structure boundary is defined by MCR_MSK.
 *
 *      local variables:
 *              int     bofst           calculated boundary offset
 *              int     bytes           bytes remaining in buffer area
 *
 *      static variables:
 *              char *  p               pointer to head of copied string
 *              char *  pnext           next location in buffer area
 *
 *      global variables:
 *              none
 *
 *      functions called:
 *              VOID *  mhunk()         asmcro.c
 *
 *      side effects:
 *              Space allocated.
 *              Out of Space terminates assembler.
 */
 
char *
mstruct(n)
int n;
{
        int  bofst;
        char *p;
   
        /*
         * Memory Boundary Fixup
         */
        bofst = bytes & MCR_MSK;

        pnext += bofst;
        bytes -= bofst;

        if (n > bytes) {
                p = mhunk();
        } else {
                p = pnext;
        }
        pnext += n;
        bytes -= n;

        return(p);
}

/*)Function     VOID    mcrinit()
 *
 *      Initialize Macro Processor Variables
 *
 *      static variables:
 *              int     bytes           bytes remaining in buffer area
 *              char *  pnext           next location in buffer area
 *
 *      global variables:
 *              struct sym      mls     Macro local symbol
 *              int             mcrfil  macro nesting counter
 *              int             maxmcr  maximum macro nesting emcountered
 *              int             mcrline current macro line number
 *
 *              struct mcrdef  *mcrlst  link to list of defined macros
 *              struct mcrdef  *mcrp    macro being defined
 *              struct memlnk  *mcrmem  Macro Memory Allocation Structure
 *
 *      functions called:
 *              none
 *
 *      side effects:
 *              Prepares values for next assembler pass.
 */
 
VOID
mcrinit()
{
        mls.s_addr = 10000;

        mcrfil = 0;
        maxmcr = 0;
        mcrline = 0;

        mcrlst = NULL;
        mcrp = NULL;
        mcrmem = NULL;

        pnext = NULL;
        bytes = 0;
}

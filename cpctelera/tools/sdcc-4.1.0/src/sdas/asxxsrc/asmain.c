/* asmain.c */

/*
 *  Copyright (C) 1989-2012  Alan R. Baldwin
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
 *
 *   With enhancements from
 *
 *      John L. Hartman (JLH)
 *      jhartman at compuserve dot com
 *
 *      Boisy G. Pitre (BGP)
 *      boisy at boisypitre dot com
 *
 *      Mike McCarty
 *      mike dot mccarty at sbcglobal dot net
 */

#include <errno.h>
#include <math.h>
#include "sdas.h"
#include "dbuf_string.h"
#include "asxxxx.h"

/*)Module       asmain.c
 *
 *      The module asmain.c includes the command argument parser,
 *      the three pass sequencer, and the machine independent
 *      assembler parsing code.
 *
 *      asmain.c contains the following functions:
 *              int     main(argc, argv)
 *              VOID    asexit(n)
 *              VOID    asmbl()
 *              VOID    equate()
 *              FILE *  afile(fn, ft, wf)
 *              int     fndidx(str)
 *              int     intsiz()
 *              VOID    newdot(nap)
 *              VOID    phase(ap, a)
 *              VOID    usage()
 *
 *      asmain.c contains the array char *usetxt[] which
 *      references the usage text strings printed by usage().
 */

/* sdas specific */
static const char *search_path[100];
static int search_path_length;

/**
 * The search_path_append is used to append another directory to the end
 * of the include file search path.
 *
 * @param dir
 *              The directory to be added to the path.
 */
void
search_path_append(const char *dir)
{
        if (search_path_length < sizeof(search_path)/sizeof(char*))
        {
                search_path[search_path_length++] = dir;
        }
}

/**
 * The create_temp_path function is used to build a temporary file path
 * by concatenating dir and filename. If len >= 0 then only the left
 * substring of dir with length len is used to build the file path.
 *
 * @param dir
 *              The directory part of the path.
 * @param len
 *              < 0:  use the whole dir as the directory part of the path.
 *              >= 0: the length of dir to use as the directory part of the path.
 * @param filename
 *              The filename to be appended to the directory part of the path.
 * @returns
 *              The constructed path.
 */
static const char *
create_temp_path(const char * dir, int len, const char * filename)
{
        static struct dbuf_s dbuf;
        const char * path;

        if (!dbuf_is_initialized(&dbuf))
                dbuf_init (&dbuf, 1024);

        dbuf_set_length(&dbuf, 0);
        dbuf_append_str(&dbuf, dir);
        if (len >= 0)
                dbuf_set_length(&dbuf, len);
        path = dbuf_c_str(&dbuf);
        if ((path[strlen(path) - 1] != '/') &&
            (path[strlen(path) - 1] != DIR_SEPARATOR_CHAR)) {
                dbuf_append_char(&dbuf, DIR_SEPARATOR_CHAR);
        }
        dbuf_append_str(&dbuf, filename);
        path = dbuf_c_str(&dbuf);
        return path;
}

/**
 * The search_path_fopen function is used to open the named file.  If
 * the file isn't in the current directory, the search path is then used
 * to build a series of possible file names, and attempts to open them.
 * The first found is used.
 *
 * @param filename
 *              The name of the file to be opened.
 * @param mode
 *              The mode of the file to be opened.
 * @returns
 *              what the fopen function would return on success, or NULL if the
 *              file is not anywhere in the search path.
 */
static FILE *
search_path_fopen(const char *filename, const char *mode)
{
        FILE *fp;
        int j;

        fp = fopen(filename, mode);
        if (fp != NULL || filename[0] == '/' || filename[0] == '\\')
                return fp;

        /*
         * Try the path of the file opening the include file
         */
        fp = fopen(create_temp_path(afn, afp, filename), mode);
        if (fp != NULL)
                return fp;

        for (j = 0; j < search_path_length; ++j) {
                fp = fopen(create_temp_path(search_path[j], -1, filename), mode);
                if (fp != NULL)
                        return fp;
        }
        errno = ENOENT;
        return NULL;
}
/* end sdas specific */

/*)Function     int     main(argc, argv)
 *
 *              int     argc            argument count
 *              char *  argv            array of pointers to argument strings
 *
 *      The function main() is the entry point to the assembler.
 *      The purpose of main() is to (1) parse the command line
 *      arguments for options and source file specifications and
 *      (2) to process the source files through the 3 pass assembler.
 *      Before each assembler pass various variables are initialized
 *      and source files are rewound to their beginning.  During each
 *      assembler pass each assembler-source text line is processed.
 *      After each assembler pass the assembler information is flushed
 *      to any opened output files and the if-else-endif processing
 *      is checked for proper termination.
 *
 *      The function main() is also responsible for opening all
 *      output files (REL, LST, and SYM), sequencing the global (-g)
 *      and all-global (-a) variable definitions, and dumping the
 *      REL file header information.
 *
 *      local variables:
 *              char *  p               pointer to argument string
 *              int     c               character from argument string
 *              int     i               argument loop counter
 *              area *  ap              pointer to area structure
 *
 *      global variables:
 *              int     aflag           -a, make all symbols global flag
 *              char    afn[]           afile() constructed filespec
 *              int     afp             afile constructed path length
 *              area *  areap           pointer to an area structure
 *              asmf *  asmc            pointer to current assembler file structure
 *              int     asmline         assembler source file line number
 *              asmf *  asmp            pointer to first assembler file structure
 *              int     aserr           assembler error counter
 *              int     bflag           -b(b), listing mode flag
 *              int     cb[]            array of assembler output values
 *              int     cbt[]           array of assembler relocation types
 *                                      describing the data in cb[]
 *              int *   cp              pointer to assembler output array cb[]
 *              int *   cpt             pointer to assembler relocation type
 *                                      output array cbt[]
 *              char    eb[]            array of generated error codes
 *              char *  ep              pointer into error list array eb[]
 *              int     fflag           -f(f), relocations flagged flag
 *              int     flevel          IF-ELSE-ENDIF flag will be non
 *                                      zero for false conditional case
 *              a_uint  fuzz            tracks pass to pass changes in the
 *                                      address of symbols caused by
 *                                      variable length instruction formats
 *              int     gflag           -g, make undefined symbols global flag
 *              char  * ib              string buffer containing
 *                                      assembler-source text line for processing
 *              char  * ic              string buffer containing
 *                                      assembler-source text line for listing
 *              int     ifcnd[]         array of IF statement condition
 *                                      values (0 = FALSE) indexed by tlevel
 *              int     iflvl[]         array of IF-ELSE-ENDIF flevel
 *                                      values indexed by tlevel
 *              int     incfil          current include file count
 *              int     incline         include source file line number
 *              char *  ip              pointer into the assembler-source
 *                                      text line in ib[]
 *              jmp_buf jump_env        compiler dependent structure
 *                                      used by setjmp() and longjmp()
 *              int     lflag           -l, generate listing flag
 *              int     line            current assembler source
 *                                      line number
 *              int     lnlist          current LIST-NLIST state
 *              int     lop             current line number on page
 *              int     maxinc          maximum include file nesting counter
 *              int     oflag           -o, generate relocatable output flag
 *              int     jflag           -j, generate debug info flag
 *              int     page            current page number
 *              int     pflag           disable listing pagination
 *              int     pass            assembler pass number
 *              int     radix           current number conversion radix:
 *                                      2 (binary), 8 (octal), 10 (decimal),
 *                                      16 (hexadecimal)
 *              int     sflag           -s, generate symbol table flag
 *              int     srcline         current source line number
 *              char    stb[]           Subtitle string buffer
 *              sym *   symp            pointer to a symbol structure
 *              int     tlevel          current conditional level
 *              int     uflag           -u, disable .list/.nlist processing
 *              int     wflag           -w, enable wide listing format
 *              int     xflag           -x, listing radix flag
 *              int     zflag           -z, disable symbol case sensitivity
 *              FILE *  lfp             list output file handle
 *              FILE *  ofp             relocation output file handle
 *              FILE *  tfp             symbol table output file handle
 *
 *      called functions:
 *              FILE *  afile()         asmain.c
 *              VOID    allglob()       assym.c
 *              VOID    asexit()        asmain.c
 *              VOID    diag()          assubr.c
 *              VOID    err()           assubr.c
 *              VOID    exprmasks()     asexpr.c
 *              int     fprintf()       c_library
 *              int     int32siz()      asmain.c
 *              VOID    list()          aslist.c
 *              VOID    lstsym()        aslist.c
 *              VOID    mcrinit()       asmcro.c
 *              VOID    minit()         ___mch.c
 *              char *  new()           assym.c
 *              VOID    newdot()        asmain.c
 *              int     nxtline()       aslex.c
 *              VOID    outbuf()        asout.c
 *              VOID    outchk()        asout.c
 *              VOID    outgsd()        asout.c
 *              int     rewind()        c_library
 *              int     setjmp()        c_library
 *              char *  strcpy()        c_library
 *              VOID    symglob()       assym.c
 *              VOID    syminit()       assym.c
 *              VOID    usage()         asmain.c
 *
 *      side effects:
 *              Completion of main() completes the assembly process.
 *              REL, LST, and/or SYM files may be generated.
 */

/* sdas specific */
char relFile[FILSPC];
/* end sdas specific */

int
main(int argc, char *argv[])
{
        char *p = NULL;
        char *q;
        int c, i;
        struct area *ap;

        if (intsiz() < 4) {
                fprintf(stderr, "?ASxxxx-Error-Size of INT32 is not 32 bits or larger.\n\n");
                exit(ER_FATAL);
        }

        /* sdas specific */
        /* sdas initialization */
        sdas_init(argv[0]);
        /* end sdas specific */

        if (!is_sdas())
                fprintf(stdout, "\n");
        q = NULL;
        asmc = NULL;
        asmp = NULL;
        for (i=1; i<argc; ++i) {
                p = argv[i];
                if (*p == '-') {
                        if (asmc != NULL)
                                usage(ER_FATAL);
                        ++p;
                        while ((c = *p++) != 0) {
                                switch(c) {

                                case 'a':
                                case 'A':
                                        ++aflag;
                                        break;

                                case 'b':
                                case 'B':
                                        ++bflag;
                                        break;

                                case 'c':
                                case 'C':
                                        cflag = 1;      /* Cycle counts in listing */
                                        break;

                                case 'g':
                                case 'G':
                                        ++gflag;
                                        break;

                                case 'i':
                                case 'I':
                                        search_path_append(p);
                                        while (*p)
                                                ++p;
                                        break;

#if NOICE
                                case 'j':               /* NoICE Debug  JLH */
                                case 'J':
                                        ++jflag;
                                        ++oflag;        /* force object */
                                        break;
#endif

#if SDCDB
                                case 'y':               /* SDCC Debug */
                                case 'Y':
                                        ++yflag;
                                        break;
#endif

                                case 'l':
                                case 'L':
                                        ++lflag;
                                        break;

                                case 'o':
                                case 'O':
                                        ++oflag;
                                        break;

                                case 's':
                                case 'S':
                                        ++sflag;
                                        break;

                                case 'p':
                                case 'P':
                                        ++pflag;
                                        break;

                                case 'u':
                                case 'U':
                                        ++uflag;
                                        break;

                                case 'w':
                                case 'W':
                                        ++wflag;
                                        break;

                                case 'z':
                                case 'Z':
                                        ++zflag;
                                        break;

                                case 'x':
                                case 'X':
                                        xflag = 0;
                                        break;

                                case 'q':
                                case 'Q':
                                        xflag = 1;
                                        break;

                                case 'd':
                                case 'D':
                                        xflag = 2;
                                        break;

                                case 'f':
                                case 'F':
                                        ++fflag;
                                        break;

                                default:
                                        usage(ER_FATAL);
                                }
                        }
                } else {
                        if (asmc == NULL) {
                                q = p;
                                if (++i < argc) {
                                        p = argv[i];
                                        if (*p == '-')
                                                usage(ER_FATAL);
                                }
                                asmp = (struct asmf *)
                                                new (sizeof (struct asmf));
                                asmc = asmp;
                        } else {
                                asmc->next = (struct asmf *)
                                                new (sizeof (struct asmf));
                                asmc = asmc->next;
                        }
                        asmc->next = NULL;
                        asmc->objtyp = T_ASM;
                        asmc->line = 0;
                        asmc->flevel = 0;
                        asmc->tlevel = 0;
                        asmc->lnlist = LIST_NORM;
                        asmc->fp = afile(p, "", 0);
                        strcpy(asmc->afn,afn);
                        asmc->afp = afp;
                }
        }
        if (asmp == NULL)
                usage(ER_WARNING);
        if (lflag)
                lfp = afile(q, "lst", 1);
        /* sdas specific */
        if (oflag) {
                ofp = afile(q, (is_sdas() && p != q) ? "" : "rel", 1);
                // save the file name if we have to delete it on error
                strcpy(relFile,afn);
        }
        /* end sdas specific */
        if (sflag)
                tfp = afile(q, "sym", 1);
        exprmasks(2);
        syminit();
        for (pass=0; pass<3; ++pass) {
                aserr = 0;
                if (gflag && pass == 1)
                        symglob();
                if (aflag && pass == 1)
                        allglob();
                if (oflag && pass == 2)
                        outgsd();
                flevel = 0;
                tlevel = 0;
                lnlist = LIST_NORM;
                ifcnd[0] = 0;
                iflvl[0] = 0;
                radix = 10;
                page = 0;
                /* sdas specific */
                org_cnt = 0;
                /* end sdas specific */
                stb[0] = 0;
                lop  = NLPP;
                incfil = 0;
                maxinc = 0;
                srcline = 0;
                asmline = 0;
                incline = 0;
                asmc = asmp;
                while (asmc) {
                        if (asmc->fp)
                                rewind(asmc->fp);
                        asmc = asmc->next;
                }
                asmc = asmp;
                strcpy(afn, asmc->afn);
                afp = asmc->afp;
                ap = areap;
                while (ap) {
                        ap->a_fuzz = 0;
                        ap->a_size = 0;
                        ap = ap->a_ap;
                }
                fuzz = 0;
                dot.s_addr = 0;
                dot.s_area = &dca;
                outbuf("I");
                outchk(0,0);
                symp = &dot;
                mcrinit();
                minit();
                while (nxtline()) {
                        cp = cb;
                        cpt = cbt;
                        ep = eb;
                        ip = ib;

                        /* JLH: if line begins with ";!", then
                         * pass this comment on to the output file
                         */
                        if (oflag && (pass == 1) && (ip[0] == ';') && (ip[1] == '!')) {
                                fprintf(ofp, "%s\n", ip );
                        }

                        opcycles = OPCY_NONE;
                        if (setjmp(jump_env) == 0)
                                asmbl();
                        if (pass == 2) {
                                diag();
                                list();
                        }
                }
                newdot(dot.s_area);     /* Flush area info */
        }
        if (flevel || tlevel) {
                err('i');
                fprintf(stderr, "?ASxxxx-Error-<i> at end of assembly\n");
                fprintf(stderr, "              %s\n", geterr('i'));
        }
        if (oflag)
                outchk(ASXHUGE, ASXHUGE);       /* Flush */
        if (sflag) {
                lstsym(tfp);
        } else
        if (lflag) {
                lstsym(lfp);
        }
        asexit(aserr ? ER_ERROR : ER_NONE);
        return(0);
}

/*)Function     int     intsiz()
 *
 *      The function intsiz() returns the size of INT32
 *
 *      local variables:
 *              none
 *
 *      global variables:
 *              none
 *
 *      functions called:
 *              none
 *
 *      side effects:
 *              none
 */

int
intsiz(void)
{
        return(sizeof(INT32));
}

/*)Function     VOID    asexit(i)
 *
 *                      int     i       exit code
 *
 *      The function asexit() explicitly closes all open
 *      files and then terminates the program.
 *
 *      local variables:
 *              int     j               loop counter
 *
 *      global variables:
 *              asmf *  asmc            pointer to current assembler file structure
 *              asmf *  asmp            pointer to first assembler file structure
 *              FILE *  lfp             list output file handle
 *              FILE *  ofp             relocation output file handle
 *              FILE *  tfp             symbol table output file handle
 *
 *      functions called:
 *              int     fclose()        c_library
 *              VOID    exit()          c_library
 *
 *      side effects:
 *              All files closed. Program terminates.
 */

VOID
asexit(int i)
{
        if (lfp != NULL) fclose(lfp);
        if (ofp != NULL) fclose(ofp);
        if (tfp != NULL) fclose(tfp);

        while (asmc != NULL) {
                if ((asmc->objtyp == T_INCL) && (asmc->fp != NULL)) {
                        fclose(asmc->fp);
                }
                asmc = asmc->next;
        }

        asmc = asmp;
        while (asmc != NULL) {
                if ((asmc->objtyp == T_ASM) && (asmc->fp != NULL)) {
                        fclose(asmc->fp);
                }
                asmc = asmc->next;
        }
        /* sdas specific */
        if (i) {
                /* remove output file */
                printf ("removing %s\n", relFile);
                remove(relFile);
        }
        /* end sdas specific */
        exit(i);
}

/*)Function     VOID    asmbl()
 *
 *      The function asmbl() scans the assembler-source text for
 *      (1) labels, global labels, equates, global equates, and local
 *      symbols, (2) .if, .else, .endif, and .page directives,
 *      (3) machine independent assembler directives, (4) macros and
 *      macro definitions, and (5) machine dependent mnemonics.
 *
 *      local variables:
 *              mne *   mp              pointer to a mne structure
 *              mne *   xp              pointer to a mne structure
 *              mcrdef *np              pointer to a macro definition structure
 *              sym *   sp              pointer to a sym structure
 *              tsym *  tp              pointer to a tsym structure
 *              int     c               character from assembler-source
 *                                      text line
 *              area *  ap              pointer to an area structure
 *              expr    e1              expression structure
 *              char    id[]            id string
 *              char    opt[]           options string
 *              char    fn[]            filename string
 *              char *  p               pointer into a string
 *              int     d               temporary value
 *              int     uaf             user area options flag
 *              int     uf              area options
 *              a_uint  n               temporary value
 *              a_uint  v               temporary value
 *              int     flags           temporary flag
 *              FILE *  fp              include file handle
 *              int     m_type          mnemonic type
 *
 *      global variables:
 *              area *  areap           pointer to an area structure
 *              char    ctype[]         array of character types, one per
 *                                      ASCII character
 *              int     flevel          IF-ELSE-ENDIF flag will be non
 *                                      zero for false conditional case
 *              int     ftflevel;       IIFF-IIFT-IIFTF FLAG
 *              int     lnlist          current LIST-NLIST state
 *              a_uint  fuzz            tracks pass to pass changes in the
 *                                      address of symbols caused by
 *                                      variable length instruction formats
 *              int     ifcnd[]         array of IF statement condition
 *                                      values (0 = FALSE) indexed by tlevel
 *              int     iflvl[]         array of IF-ELSE-ENDIF flevel
 *                                      values indexed by tlevel
 *              int     incline         current include file line
 *              int     incfil          current include file count
 *              a_uint  laddr           address of current assembler line
 *                                      or value of .if argument
 *              int     lmode           listing mode
 *              int     lop             current line number on page
 *              char    module[]        module name string
 *              int     pass            assembler pass number
 *              int     radix           current number conversion radix:
 *                                      2 (binary), 8 (octal), 10 (decimal),
 *                                      16 (hexadecimal)
 *              char    stb[]           Subtitle string buffer
 *              sym *   symp            pointer to a symbol structure
 *              char    tb[]            Title string buffer
 *              int     tlevel          current conditional level
 *              int     jflag           -j, generate debug info flag
 *
 *      functions called:
 *              a_uint  absexpr()       asexpr.c
 *              area *  alookup()       assym.c
 *              VOID    clrexpr()       asexpr.c
 *              int     digit()         asexpr.c
 *              char    endline()       aslex.c
 *              VOID    equate()        asmain.c
 *              VOID    err()           assubr.c
 *              VOID    expr()          asexpr.c
 *              FILE *  fopen()         c_library
 *              int     get()           aslex.c
 *              VOID    getid()         aslex.c
 *              int     getmap()        aslex.c
 *              int     getnb()         aslex.c
 *              VOID    getst()         aslex.c
 *              VOID    getxstr()       asmcro.c
 *              sym *   lookup()        assym.c
 *              VOID    machine()       ___mch.c
 *              int     macro()         asmcro.c
 *              int     mcrprc()        asmcro.c
 *              mne *   mlookup()       assym.c
 *              int     more()          aslex.c
 *              mcrdef *nlookup()       asmcro.c
 *              char *  new()           assym.c
 *              VOID    newdot()        asmain.c
 *              VOID    outall()        asout.c
 *              VOID    outab()         asout.c
 *              VOID    outchk()        asout.c
 *              VOID    outrb()         asout.c
 *              VOID    outrw()         asout.c
 *              VOID    phase()         asmain.c
 *              VOID    qerr()          assubr.c
 *              char *  strcpy()        c_library
 *              char *  strncpy()       c_library
 *              char *  strsto()        assym.c
 *              VOID    unget()         aslex.c
 */

VOID
asmbl(void)
{
        struct mne *mp, *xp;
        struct mcrdef *np;
        struct sym *sp;
        struct tsym *tp;
        int c;
        struct area  *ap;
        struct expr e1;
        char id[NCPS];
        char equ[NCPS];
        char *equ_ip;
        int  equtype;
        char opt[NCPS];
        char fn[FILSPC+FILSPC];
        char *p;
        int d, uaf, uf;
        a_uint n, v;
        int flags;
        FILE * fp;
        int m_type;
        /* sdas specific */
        static struct area *abs_ap; /* pointer to current absolute area structure */
        /* end sdas specific */

        laddr = dot.s_addr;
        lmode = SLIST;

        /*
         * Check iiff-iift-iiftf processing
         */
        if (ftflevel != 0) {
                flevel = ftflevel - 1;
                ftflevel = 0;
        }

        /*
         * Check if Building a Macro
         * Check if Exiting  a Macro
         */
        if (mcrprc(O_CHECK) != 0) {
                return;
        }

loop:
        if ((c=endline()) == 0) { return; }

        /*
         * If the first character is a digit then assume
         * a reusable symbol is being specified.  The symbol
         * must end with $: to be valid.
         *      pass 0:
         *              Construct a tsym structure at the first
         *              occurance of the symbol.  Flag the symbol
         *              as multiply defined if not the first occurance.
         *      pass 1:
         *              Load area, address, and fuzz values
         *              into structure tsym.
         *      pass 2:
         *              Check for assembler phase error and
         *              multiply defined error.
         */
        if (ctype[c] & DIGIT) {
                if (flevel)
                        return;
                n = 0;
                while ((d = digit(c, 10)) >= 0) {
                        n = 10*n + d;
                        c = get();
                }
                if (c != '$' || get() != ':')
                        qerr();

                tp = symp->s_tsym;
                if (pass == 0) {
                        while (tp) {
                                if (n == tp->t_num) {
                                        tp->t_flg |= S_MDF;
                                        break;
                                }
                                tp = tp->t_lnk;
                        }
                        if (tp == NULL) {
                                tp=(struct tsym *) new (sizeof(struct tsym));
                                tp->t_lnk = symp->s_tsym;
                                tp->t_num = n;
                                tp->t_flg = 0;
                                tp->t_area = dot.s_area;
                                tp->t_addr = dot.s_addr;
                                symp->s_tsym = tp;
                        }
                } else {
                        while (tp) {
                                if (n == tp->t_num) {
                                        break;
                                }
                                tp = tp->t_lnk;
                        }
                        if (tp) {
                                if (pass == 1) {
                                        fuzz = tp->t_addr - dot.s_addr;
                                        tp->t_area = dot.s_area;
                                        tp->t_addr = dot.s_addr;
                                } else {
                                        phase(tp->t_area, tp->t_addr);
                                        if (tp->t_flg & S_MDF)
                                                err('m');
                                }
                        } else {
                                err('u');
                        }
                }
                lmode = ALIST;
                goto loop;
        }
        /*
         * If the first character is a letter then assume a label,
         * symbol, assembler directive, or assembler mnemonic is
         * being processed.
         */
        if ((ctype[c] & LETTER) == 0) {
                if (flevel) {
                        return;
                } else {
                        qerr();
                }
        }
        getid(id, c);
        c = getnb();
        /*
         * If the next character is a : then a label is being processed.
         * A double :: defines a global label.  If this is a new label
         * then create a symbol structure.
         *      pass 0:
         *              Flag multiply defined labels.
         *      pass 1:
         *              Load area, address, and fuzz values
         *              into structure symp.
         *      pass 2:
         *              Check for assembler phase error and
         *              multiply defined error.
         */
        if (c == ':') {
                if (flevel)
                        return;
                if ((c = get()) != ':') {
                        unget(c);
                        c = 0;
                }
                symp = lookup(id);
                if (symp == &dot)
                        qerr();
                if (pass == 0) {
                        if ((symp->s_type != S_NEW) && ((symp->s_flag & S_ASG) == 0))
                                symp->s_flag |= S_MDF;
                }
                if (symp->s_flag & S_MDF)
                        err('m');
                symp->s_type = S_USER;
                phase(symp->s_area, symp->s_addr);
                fuzz = symp->s_addr - dot.s_addr;
                symp->s_area = dot.s_area;
                symp->s_addr = dot.s_addr;
                if (c) {
                        symp->s_flag |= S_GBL;
                }
                lmode = ALIST;
                goto loop;
        }
        /*
         * If the next character is a = then an equate is being processed.
         *
         * Syntax:
         *    [labels] sym =  value   defines an equate.
         *    [labels] sym == value   defines a global equate.
         *    [labels] sym =: value   defines an internal machine equate.
         * If this is a new variable then create a symbol structure.
         */
        if (c == '=') {
                if (flevel)
                        return;
                switch (c = get()) {
                    case '=':   equtype = O_GBLEQU;                     break;
                    case ':':   equtype = O_LCLEQU;                     break;
                    default:    equtype = O_EQU;        unget(c);       break;
                }
                equate(id, &e1, equtype);
                goto loop;
        }
        unget(c);
        /*
         * Check for Equates if 'id' is not an Assembler Directive
         *
         * Syntax:
         *     [labels] sym .equ    value   defines an equate
         *     [labels] sym .glbequ value   defines a global equate
         *     [labels] sym .lclequ value   defines a local equate
         */
        if ((mlookup(id) == NULL) && (nlookup(id) == NULL)) {
                if (flevel)
                        return;
                /*
                 * Alternates for =, ==, and =:
                 */
                equ_ip = ip;     /* Save current char pointer for case equate not found. */
                getid(equ, -1);
                if ((mp = mlookup(equ)) == NULL || mp->m_type != S_EQU) {
                        ip = equ_ip;
                } else {
                        equate(id, &e1, mp->m_valu);
                        goto loop;
                }
        }
        /*
         * Completed scan for labels , equates, and symbols.
         */
        lmode = flevel ? SLIST : CLIST;
        /*
         * An assembler directive, mnemonic, or macro is
         * required to continue processing line.
         */
        mp = mlookup(id);
        np = nlookup(id);
        if ((mp == NULL) && (np == NULL)) {
                if (!flevel) {
                        err('o');
                }
                return;
        }
        /*
         * If we have gotten this far then we have found an
         * assembler directive an assembler mnemonic or
         * an assembler macro.
         *
         * Check for .if[], .iif[], .else, .endif,
         * .list, .nlist, and .page directives.
         */
        m_type = (mp != NULL) ? mp->m_type : ~0;

        switch (m_type) {

        case S_CONDITIONAL:
                /*
                 * BGP - .ifeq, .ifne, .ifgt, .iflt, .ifge, .ifle
                 */
                if (mp->m_valu < O_IFEND) {
                        if (mp->m_valu == O_IF) {
                                /*
                                 * Process conditionals of the form
                                 *
                                 *      .if     cnd(,)  arg1 (, arg2)
                                 *
                                 * where cnd is one of the following:
                                 *
                                 *      eq      ne
                                 *      gt      lt      ge      le
                                 *      def     ndef
                                 *      b       nb      idn     dif
                                 *      t       f       tf
                                 */
                                p = ip;
                                strcpy(id,".if");
                                getid(&id[3],getnb());
                                xp = mlookup(id);
                                if ((xp != NULL) &&
                                    (xp->m_type == S_CONDITIONAL) &&
                                    (xp->m_valu != O_IF)) {
                                        mp = xp;
                                        comma(0);
                                } else {
                                        ip = p;
                                }
                        }
                        if (flevel) {
                                n = 0;
                        } else {
                                switch (mp->m_valu) {
                                case O_IF:
                                case O_IFNE:    /* .if ne,.... */
                                case O_IFEQ:    /* .if eq,.... */
                                case O_IFGT:    /* .if gt,.... */
                                case O_IFLT:    /* .if lt,.... */
                                case O_IFGE:    /* .if ge,.... */
                                case O_IFLE:    /* .if le,.... */
                                        n = absexpr();
                                        switch (mp->m_valu) {
                                        default:
                                        case O_IF:
                                        case O_IFNE:    n = (((v_sint) n) != 0);        break;
                                        case O_IFEQ:    n = (((v_sint) n) == 0);        break;
                                        case O_IFGT:    n = (((v_sint) n) >  0);        break;
                                        case O_IFLT:    n = (((v_sint) n) <  0);        break;
                                        case O_IFGE:    n = (((v_sint) n) >= 0);        break;
                                        case O_IFLE:    n = (((v_sint) n) <= 0);        break;
                                        }
                                        break;

                                case O_IFF:     /* .if f */
                                case O_IFT:     /* .if t */
                                case O_IFTF:    /* .if tf */
                                        n = 0;
                                        break;

                                default:
                                        n = 0;
                                        qerr();
                                        break;
                                }
                        }
                        switch (mp->m_valu) {
                        default:
                                if (tlevel < MAXIF) {
                                        ++tlevel;
                                        ifcnd[tlevel] = (int) n;
                                        iflvl[tlevel] = flevel;
                                        if (!n) {
                                                ++flevel;
                                        }
                                } else {
                                        err('i');
                                }
                                if (!iflvl[tlevel]) {
                                        lmode = ELIST;
                                        laddr = n;
                                } else {
                                        lmode = SLIST;
                                }
                                break;

                        case O_IFF:     /* .if f */
                        case O_IFT:     /* .if t */
                        case O_IFTF:    /* .if tf */
                                if (tlevel == 0) {
                                        err('i');
                                        lmode = SLIST;
                                        break;
                                }
                                if (iflvl[tlevel] == 0) {
                                        if (ifcnd[tlevel]) {
                                                switch (mp->m_valu) {
                                                default:
                                                case O_IFF:     flevel = 1;     break;
                                                case O_IFT:     flevel = 0;     break;
                                                case O_IFTF:    flevel = 0;     break;
                                                }
                                        } else {
                                                switch (mp->m_valu) {
                                                default:
                                                case O_IFF:     flevel = 0;     break;
                                                case O_IFT:     flevel = 1;     break;
                                                case O_IFTF:    flevel = 0;     break;
                                                }
                                        }
                                        lmode = ELIST;
                                        laddr = flevel ? 0 : 1;
                                } else {
                                        lmode = SLIST;
                                }
                                break;
                        }
                        return;
                } else
                if (mp->m_valu < O_IIFEND) {
                        if (mp->m_valu == O_IIF) {
                                /*
                                 * Process conditionals of the form
                                 *
                                 *      .iif    cnd(,)  arg1 (, arg2)
                                 *
                                 * where cnd is one of the following:
                                 *
                                 *      eq      ne
                                 *      gt      lt      ge      le
                                 *      def     ndef
                                 *      b       nb      idn     dif
                                 *      t       f       tf
                                 */
                                p = ip;
                                strcpy(id,".iif");
                                getid(&id[4],getnb());
                                xp = mlookup(id);
                                if ((xp != NULL) &&
                                    (xp->m_type == S_CONDITIONAL) &&
                                    (xp->m_valu != O_IIF)) {
                                        mp = xp;
                                        comma(0);
                                } else {
                                        ip = p;
                                }
                        }
                        switch (mp->m_valu) {
                        case O_IIFF:    /* .iif f */
                        case O_IIFT:    /* .iif t */
                        case O_IIFTF:   /* .iif tf */
                                if (tlevel == 0) {
                                        err('i');
                                        lmode = SLIST;
                                        return;
                                }
                                if (iflvl[tlevel] == 0) {
                                        ftflevel = flevel + 1;
                                        if (ifcnd[tlevel] != 0) {
                                                switch (mp->m_valu) {
                                                default:
                                                case O_IIFF:    flevel = 1;     break;
                                                case O_IIFT:    flevel = 0;     break;
                                                case O_IIFTF:   flevel = 0;     break;
                                                }
                                        } else {
                                                switch (mp->m_valu) {
                                                default:
                                                case O_IIFF:    flevel = 0;     break;
                                                case O_IIFT:    flevel = 1;     break;
                                                case O_IIFTF:   flevel = 0;     break;
                                                }
                                        }
                                }
                                n = flevel ? 0 : 1;
                                /*
                                 * Skip trailing ','
                                 */
                                comma(0);
                                lmode = SLIST;
                                if (n) {
                                        goto loop;
                                }
                                return;

                        default:
                                if (flevel) {
                                        return;
                                }
                                break;
                        }
                        switch (mp->m_valu) {
                        case O_IIF:
                        case O_IIFNE:   /* .iif ne,.... */
                        case O_IIFEQ:   /* .iif eq,.... */
                        case O_IIFGT:   /* .iif gt,.... */
                        case O_IIFLT:   /* .iif lt,.... */
                        case O_IIFGE:   /* .iif ge,.... */
                        case O_IIFLE:   /* .iif le,.... */
                                n = absexpr();
                                switch (mp->m_valu) {
                                default:
                                case O_IIF:
                                case O_IIFNE:   n = (((v_sint) n) != 0);        break;
                                case O_IIFEQ:   n = (((v_sint) n) == 0);        break;
                                case O_IIFGT:   n = (((v_sint) n) >  0);        break;
                                case O_IIFLT:   n = (((v_sint) n) <  0);        break;
                                case O_IIFGE:   n = (((v_sint) n) >= 0);        break;
                                case O_IIFLE:   n = (((v_sint) n) <= 0);        break;
                                }
                                break;

                        default:
                                n = 0;
                                qerr();
                                break;
                        }
                        /*
                         * Skip trailing ','
                         */
                        comma(0);
                        lmode = SLIST;
                        if (n) {
                                goto loop;
                        }
                        return;
                }

                switch (mp->m_valu) {
                case O_ELSE:
                        if (tlevel != 0) {
                                if (ifcnd[tlevel]) {
                                        flevel = iflvl[tlevel] + 1;
                                        ifcnd[tlevel] = 0;
                                } else {
                                        flevel = iflvl[tlevel];
                                        ifcnd[tlevel] = 1;
                                }
                                if (!iflvl[tlevel]) {
                                        lmode = ELIST;
                                        laddr = ifcnd[tlevel];
                                        return;
                                }
                        } else {
                                err('i');
                        }
                        lmode = SLIST;
                        return;

                case O_ENDIF:
                        if (tlevel) {
                                flevel = iflvl[tlevel--];
                        } else {
                                err('i');
                        }
                        lmode = SLIST;
                        return;

                default:
                        break;
                }
                qerr();
                break;

        case S_LISTING:
                flags = 0;
                while ((c=endline()) != 0) {
                        if (c == ',') {
                                c = getnb();
                        }
                        if (c == '(') {
                                do {
                                        if ((c = getnb()) == '!') {
                                                flags |= LIST_NOT;
                                        } else {
                                                unget(c);
                                                getid(id, -1);
                                                if (symeq(id, "err", 1)) { flags |= LIST_ERR; } else
                                                if (symeq(id, "loc", 1)) { flags |= LIST_LOC; } else
                                                if (symeq(id, "bin", 1)) { flags |= LIST_BIN; } else
                                                if (symeq(id, "eqt", 1)) { flags |= LIST_EQT; } else
                                                if (symeq(id, "cyc", 1)) { flags |= LIST_CYC; } else
                                                if (symeq(id, "lin", 1)) { flags |= LIST_LIN; } else
                                                if (symeq(id, "src", 1)) { flags |= LIST_SRC; } else
                                                if (symeq(id, "pag", 1)) { flags |= LIST_PAG; } else
                                                if (symeq(id, "lst", 1)) { flags |= LIST_LST; } else
                                                if (symeq(id, "md" , 1)) { flags |= LIST_MD;  } else
                                                if (symeq(id, "me" , 1)) { flags |= LIST_ME;  } else
                                                if (symeq(id, "meb", 1)) { flags |= LIST_MEB; } else {
                                                        err('u');
                                                }
                                        }
                                        c = endline();
                                } while (c == ',') ;
                                if (c != ')') {
                                        qerr();
                                }
                        } else {
                                unget(c);
                                if (absexpr()) {
                                        flags |=  LIST_TORF;
                                } else {
                                        flags &= ~LIST_TORF;
                                }
                        }
                }
                if (!(flags & LIST_TORF) && flevel) {
                        return;
                }
                if (flags & ~LIST_TORF) {
                        if (flags & LIST_NOT) {
                                switch(mp->m_valu) {
                                case O_LIST:    lnlist = LIST_NONE;     break;
                                case O_NLIST:   lnlist = LIST_NORM;     break;
                                default:                                break;
                                }
                        }
                        if (flags & LIST_BITS) {
                                switch(mp->m_valu) {
                                case O_LIST:    lnlist |=  (flags & LIST_BITS); break;
                                case O_NLIST:   lnlist &= ~(flags & LIST_BITS); break;
                                default:                                        break;
                                }
                        }
                } else {
                        switch(mp->m_valu) {
                        case O_LIST:    lnlist = LIST_NORM;     break;
                        case O_NLIST:   lnlist = LIST_NONE;     break;
                        default:                                break;
                        }
                }
                lmode = (lnlist & LIST_LST) ? SLIST : NLIST;
                return;

        case S_PAGE:
                lmode = NLIST;
                if (more()) {
                        n = absexpr() ? 1 : 0;
                } else {
                        n = 0;
                }
                if (!n && flevel)
                        return;

                lop = NLPP;
                return;

        default:
                break;
        }
        if (flevel)
                return;
        /*
         * If we are not in a false state for .if/.else then
         * process the assembler directives here.
         */
        switch (m_type) {

        case S_HEADER:
                switch(mp->m_valu) {
                case O_TITLE:
                        p = tb;
                        if ((c = getnb()) != 0) {
                                do {
                                        if (p < &tb[NTITL-1])
                                                *p++ = c;
                                } while ((c = get()) != 0);
                        }
                        *p = 0;
                        unget(c);
                        lmode = SLIST;
                        break;

                case O_SBTTL:
                        p = stb;
                        if ((c = getnb()) != 0) {
                                do {
                                        if (p < &stb[NSBTL-1])
                                                *p++ = c;
                                } while ((c = get()) != 0);
                        }
                        *p = 0;
                        unget(c);
                        lmode = SLIST;
                        break;

                default:
                        break;
                }
                break;

        case S_MODUL:
                getst(id, getnb()); // a module can start with a digit
                if (pass == 0) {
                        if (module[0]) {
                                err('m');
                        } else {
                                strncpy(module, id, NCPS);
                        }
                }
                lmode = SLIST;
                break;

        case S_INCL:
                lmode = SLIST;
                if (incfil > maxinc) {
                        maxinc = incfil;
                }
                /*
                 * Copy the .include file specification
                 */
                getdstr(fn, FILSPC + FILSPC);
                /*
                 * Open File
                 */
                if ((fp = search_path_fopen(fn, "r")) == NULL) {
                        --incfil;
                        err('i');
                } else {
                        asmi = (struct asmf *) new (sizeof (struct asmf));
                        asmi->next = asmc;
                        asmi->objtyp = T_INCL;
                        asmi->line = srcline;
                        asmi->flevel = flevel;
                        asmi->tlevel = tlevel;
                        asmi->lnlist = lnlist;
                        asmi->fp = fp;
                        asmi->afp = afptmp;
                        strcpy(asmi->afn,afntmp);
                        if (lnlist & LIST_PAG) {
                                lop = NLPP;
                        }
                }
                break;

        /* sdas specific */
        case S_OPTSDCC:
                optsdcc = strsto(ip);
                lmode = SLIST;
                return; /* line consumed */
        /* end sdas specific */

        case S_AREA:
                getid(id, -1);
                uaf = 0;
                uf  = A_CON|A_REL;
                if ((c = getnb()) == '(') {
                        do {
                                getid(opt, -1);
                                mp = mlookup(opt);
                                if (mp && mp->m_type == S_ATYP) {
                                        ++uaf;
                                        v = mp->m_valu;
                                        uf |= (int) v;
                                } else {
                                        err('u');
                                }
                        } while ((c = getnb()) == ',');
                        if (c != ')')
                                qerr();
                } else {
                        unget(c);
                }
                if ((ap = alookup(id)) != NULL) {
                        if (uaf && uf != ap->a_flag)
                                err('m');
                } else {
                        ap = (struct area *) new (sizeof(struct area));
                        ap->a_ap = areap;
                        ap->a_id = strsto(id);
                        ap->a_ref = areap->a_ref + 1;
                        /* sdas specific */
                        ap->a_addr = 0;
                        /* end sdas specific */
                        ap->a_size = 0;
                        ap->a_fuzz = 0;
                        ap->a_flag = uaf ? uf : (A_CON|A_REL);
                        areap = ap;
                }
                newdot(ap);
                lmode = SLIST;
                if (dot.s_area->a_flag & A_ABS)
                        abs_ap = ap;
                break;

        case S_ORG:
                if (dot.s_area->a_flag & A_ABS) {
                        char buf[NCPS];

                        outall();
                        laddr = absexpr();
                        sprintf(buf, "%s%x", abs_ap->a_id, org_cnt++);
                        if ((ap = alookup(buf)) == NULL) {
                                ap = (struct area *) new (sizeof(struct area));
                                *ap = *areap;
                                ap->a_ap = areap;
                                ap->a_id = strsto(buf);
                                ap->a_ref = areap->a_ref + 1;
                                ap->a_size = 0;
                                ap->a_fuzz = 0;
                                areap = ap;
                        }
                        newdot(ap);
                        dot.s_addr = dot.s_org = laddr;
                } else {
                        err('o');
                }
                outall();
                lmode = ALIST;
                break;

        case S_RADIX:
                if (more()) {
                        switch (getnb()) {
                        case 'b':
                        case 'B':
                                radix = 2;
                                break;
                        case '@':
                        case 'o':
                        case 'O':
                        case 'q':
                        case 'Q':
                                radix = 8;
                                break;
                        case 'd':
                        case 'D':
                                radix = 10;
                                break;
                        case 'h':
                        case 'H':
                        case 'x':
                        case 'X':
                                radix = 16;
                                break;
                        default:
                                radix = 10;
                                qerr();
                                break;
                        }
                } else {
                        radix = 10;
                }
                lmode = SLIST;
                break;

        case S_GLOBL:
                do {
                        getid(id, -1);
                        sp = lookup(id);
                        sp->s_flag &= ~S_LCL;
                        sp->s_flag |=  S_GBL;
                } while (comma(0));
                lmode = SLIST;
                break;

        case S_LOCAL:
                do {
                        getid(id, -1);
                        sp = lookup(id);
                        sp->s_flag &= ~S_GBL;
                        sp->s_flag |=  S_LCL;
                } while (comma(0));
                lmode = SLIST;
                break;

        case S_EQU:
                /*
                 * Syntax:
                 *     [labels] .equ    sym, value   defines an equate
                 *     [labels] .glbequ sym, value   defines a global equate
                 *     [labels] .lclequ sym, value   defines a local equate
                 */
                getid(id, -1);
                comma(1);
                equate(id, &e1, mp->m_valu);
                break;

        case S_DATA:
                switch (mp->m_valu) {
                case O_1BYTE:
                case O_2BYTE:
                        do {
                                clrexpr(&e1);
                                expr(&e1, 0);
                                if (mp->m_valu == O_1BYTE) {
                                        outrb(&e1, R_NORM);
                                } else {
                                        outrw(&e1, R_NORM);
                                }
                        } while ((c = getnb()) == ',');
                        unget(c);
                        break;
                default:
                        break;
                }
                break;

        /* sdas z80 specific */
        case S_FLOAT:
                do {
                        double f1, f2;
                        unsigned int mantissa, exponent;
                        char readbuffer[80];

                        getid(readbuffer, ' '); /* Hack :) */
                        if ((c = getnb()) == '.') {
                                getid(&readbuffer[strlen(readbuffer)], '.');
                        }
                        else
                                unget(c);

                        f1 = strtod(readbuffer, (char **)NULL);
                        /* Convert f1 to a gb-lib type fp
                         * 24 bit mantissa followed by 7 bit exp and 1 bit sign
                         */

                        if (f1 != 0) {
                                f2 = floor(log(fabs(f1)) / log(2)) + 1;
                                mantissa = (unsigned int) ((0x1000000 * fabs(f1)) / exp(f2 * log(2)));
                                mantissa &= 0xffffff;
                                exponent = (unsigned int) (f2 + 0x40) ;
                                if (f1 < 0)
                                        exponent |=0x80;
                        }
                        else {
                                mantissa = 0;
                                exponent = 0;
                        }

                        outab(mantissa & 0xff);
                        outab((mantissa >> 8) & 0xff);
                        outab((mantissa >> 16) & 0xff);
                        outab(exponent & 0xff);

                } while ((c = getnb()) == ',');
                unget(c);
                break;
        /* end sdas z80 specific */

        /* sdas hc08 specific */
        case S_ULEB128:
        case S_SLEB128:
                do {
                        a_uint val = absexpr();
                        int bit = sizeof(val)*8 - 1;
                        int impliedBit;

                        if (mp->m_type == S_ULEB128) {
                                impliedBit = 0;
                        } else {
                                impliedBit = (val & (1 << bit)) ? 1 : 0;
                        }
                        while ((bit>0) && (((val & (1 << bit)) ? 1 : 0) == impliedBit)) {
                                bit--;
                        }
                        if (mp->m_type == S_SLEB128) {
                                bit++;
                        }
                        while (bit>=0) {
                                if (bit<7) {
                                        outab(val & 0x7f);
                                } else {
                                        outab(0x80 | (val & 0x7f));
                                }
                                bit -= 7;
                                val >>= 7;
                        }
                } while ((c = getnb()) == ',');
                unget(c);
                break;
        /* end sdas hc08 specific */

        case S_BLK:
                clrexpr(&e1);
                expr(&e1, 0);
                outchk(ASXHUGE,ASXHUGE);
                dot.s_addr += e1.e_addr*mp->m_valu;
                lmode = BLIST;
                break;

        case S_ASCIX:
                switch(mp->m_valu) {
                case O_ASCII:
                case O_ASCIZ:
                        if ((d = getnb()) == '\0')
                                qerr();
                        while ((c = getmap(d)) >= 0)
                                outab(c);
                        if (mp->m_valu == O_ASCIZ)
                                outab(0);
                        break;

                case O_ASCIS:
                        if ((d = getnb()) == '\0')
                                qerr();
                        c = getmap(d);
                        while (c >= 0) {
                                int n2;
                                if ((n2 = getmap(d)) >= 0) {
                                        outab(c);
                                } else {
                                        outab(c | 0x80);
                                }
                                n = n2;
                                c = n2;
                        }
                        break;
                default:
                        break;
                }
                break;

        case S_BOUNDARY:
                switch(mp->m_valu) {
                case O_EVEN:
                        outall();
                        laddr = dot.s_addr = (dot.s_addr + 1) & ~1;
                        lmode = ALIST;
                        break;

                case O_ODD:
                        outall();
                        laddr = dot.s_addr |= 1;
                        lmode = ALIST;
                        break;

                case O_BNDRY:
                        v = absexpr();
                        n = dot.s_addr % v;
                        if (n != 0) {
                                dot.s_addr += (v - n);
                        }
                        outall();
                        laddr = dot.s_addr;
                        lmode = ALIST;
                        break;

                default:
                        break;
                }
                break;

        case S_MACRO:
                lmode = SLIST;
                mcrprc((int) mp->m_valu);
                return;

        /*
         * If not an assembler directive then go to the
         * macro function or machine dependent function
         * which handles all the assembler mnemonics.
         *
         * MACRO Definitions take precedence
         * over machine specific mnemonics.
         */
        default:
                if (np != NULL) {
                        macro(np);
                } else {
                        machine(mp);
                }

                /*
                 * Include Files and Macros are not Debugged
                 */
                if (asmc->objtyp == T_ASM) {

#if NOICE
                        /*
                         * NoICE        JLH
                         * if -j, generate a line number symbol
                         */
                        if (jflag && (pass == 1)) {
                                DefineNoICE_Line();
                        }
#endif

#if SDCDB
                        /*
                         * SDCC Debug Information
                         * if cdb information then generate the line info
                         */
                        if (yflag && (pass == 1)) {
                                DefineSDCC_Line();
                        }
#endif
                }

                break;
        }

        if (is_sdas()) {
                if ((c = endline()) != 0) {
                        err('q');
                }
        }
        else {
                goto loop;
        }
}

/*)Function     VOID    equate(id,e1,equtype)
 *
 *              char *          id              ident to equate
 *              struct expr *   e1              value of equate
 *              a_uint          equtype         equate type (O_EQU,O_LCLEQU,O_GBLEQU)
 *
 *      The function equate() installs an equate of a
 *      given type.
 *
 *      equate has no return value
 *
 *      local variables:
 *              struct sym *    sp      symbol being equated
 *
 *      global variables:
 *              lmode                   set to ELIST
 *
 *      functions called:
 *              VOID    clrexpr()       asexpr.c
 *              VOID    expr()          asexpr.c
 *              VOID    err()           assubr.c
 *              sym  *  lookup()        assym.c
 *              VOID    outall()        asout.c
 *              VOID    rerr()          assubr.c
 *
 *      side effects:
 *              A new symbol may be created.
 *              Symbol parameters are updated.
 */

VOID
equate(char *id, struct expr *e1, a_uint equtype)
{
        struct sym *sp;

        clrexpr(e1);
        expr(e1, 0);

        sp = lookup(id);

        if (sp == &dot) {
                outall();
                if (e1->e_flag || e1->e_base.e_ap != dot.s_area)
                        err('.');
        } else {
                switch(equtype) {
                case O_EQU:
                default:
                        break;

                case O_GBLEQU:
                        sp->s_flag &= ~S_LCL;
                        sp->s_flag |=  S_GBL;
                        break;

                case O_LCLEQU:
                        sp->s_flag &= ~S_GBL;
                        sp->s_flag |=  S_LCL;
                        break;
                }

                if (e1->e_flag && (e1->e_base.e_sp->s_type == S_NEW)) {
                        rerr();
                } else {
                        sp->s_area = e1->e_base.e_ap;
                }
                sp->s_flag |= S_ASG;
                sp->s_type = S_USER;
        }

        sp->s_addr = laddr = e1->e_addr;
        lmode = ELIST;
}

/*)Function     FILE *  afile(fn, ft, wf)
 *
 *              char *  fn              file specification string
 *              char *  ft              file type string
 *              int     wf              read(0)/write(1) flag
 *
 *      The function afile() opens a file for reading or writing.
 *
 *      afile() returns a file handle for the opened file or aborts
 *      the assembler on an open error.
 *
 *      local variables:
 *              FILE *  fp              file handle for opened file
 *
 *      global variables:
 *              char    afn[]           afile() constructed filespec
 *              int     afp             afile() constructed path length
 *              char    afntmp[]        afilex() constructed filespec
 *              int     afptmp          afilex() constructed path length
 *
 *      functions called:
 *              VOID    asexit()        asmain.c
 *              VOID    afilex()        asmain.c
 *              FILE *  fopen()         c_library
 *              int     fprintf()       c_library
 *              char *  strcpy()        c_library
 *
 *      side effects:
 *              File is opened for read or write.
 */

FILE *
afile(char *fn, char *ft, int wf)
{
        FILE *fp;

        afilex(fn, ft);

        if ((fp = fopen(afntmp, wf?"w":"r")) == NULL) {
            fprintf(stderr, "?ASxxxx-Error-<cannot %s> : \"%s\"\n", wf?"create":"open", afntmp);
            asexit(ER_FATAL);
        }

        strcpy(afn, afntmp);
        afp = afptmp;

        return (fp);
}

/*)Function     VOID    afilex(fn, ft)
 *
 *              char *  fn              file specification string
 *              char *  ft              file type string
 *
 *      The function afilex() processes the file specification string:
 *              (1)     If the file type specification string ft
 *                      is not NULL then a file specification is
 *                      constructed with the file path\name in fn
 *                      and the extension in ft.
 *              (2)     If the file type specification string ft
 *                      is NULL then the file specification is
 *                      constructed from fn.  If fn does not have
 *                      a file type then the default source file
 *                      type dsft is appended to the file specification.
 *
 *      afilex() aborts the assembler on a file specification length error.
 *
 *      local variables:
 *              int     c               character value
 *              char *  p1              pointer into filespec string afntmp
 *              char *  p2              pointer to filetype string ft
 *
 *      global variables:
 *              char    afntmp[]        afilex() constructed filespec
 *              int     afptmp          afilex() constructed path length
 *              char    dsft[]          default assembler file type string
 *
 *      functions called:
 *              VOID    asexit()        asmain.c
 *              int     fndidx()        asmain.c
 *              int     fprintf()       c_library
 *              char *  strcpy()        c_library
 *              int     strlen()        c_library
 *
 *      side effects:
 *              File specification string may be modified.
 */

VOID
afilex(char *fn, char *ft)
{
        char *p1, *p2;
        int c;

        if (strlen(fn) > (FILSPC-7)) {
                fprintf(stderr, "?ASxxxx-Error-<filspc to long> : \"%s\"\n", fn);
                asexit(ER_FATAL);
        }

        /*
         * Save the File Name Index
         */
        strcpy(afntmp, fn);
        afptmp = fndidx(afntmp);

        /*
         * Skip to File Extension separator
         */
        p1 = strrchr(&afntmp[afptmp], FSEPX);

        /*
         * Copy File Extension
         */
        p2 = ft;

        // choose a file-extension
        if (*p2 == 0) {
                // no extension supplied
                if (p1 == NULL) {
                        // no extension in fn: use default extension
                        p2 = dsft;
                } else {
                        p2 = strrchr(&fn[afptmp], FSEPX) + 1;
                }
        }
        if (p1 == NULL) {
                p1 = &afntmp[strlen(afntmp)];
        }
        *p1++ = FSEPX;
        while ((c = *p2++) != 0) {
                if (p1 < &afntmp[FILSPC-1])
                        *p1++ = c;
        }
        *p1++ = 0;
}

/*)Function     int     fndidx(str)
 *
 *              char *  str             file specification string
 *
 *      The function fndidx() scans the file specification string
 *      to find the index to the file name.  If the file
 *      specification contains a 'path' then the index will
 *      be non zero.
 *
 *      fndidx() returns the index value.
 *
 *      local variables:
 *              char *  p1              temporary pointer
 *              char *  p2              temporary pointer
 *
 *      global variables:
 *              none
 *
 *      functions called:
 *              char *  strrchr()       c_library
 *
 *      side effects:
 *              none
 */

int
fndidx(char *str)
{
        char *p1, *p2;

        /*
         * Skip Path Delimiters
         */
        p1 = str;
        if ((p2 = strrchr(p1,  ':')) != NULL) { p1 = p2 + 1; }
        if ((p2 = strrchr(p1,  '/')) != NULL) { p1 = p2 + 1; }
        if ((p2 = strrchr(p1, '\\')) != NULL) { p1 = p2 + 1; }

        return((int) (p1 - str));
}

/*)Function     VOID    newdot(nap)
 *
 *              area *  nap             pointer to the new area structure
 *
 *      The function newdot():
 *              (1)     copies the current values of fuzz and the last
 *                      address into the current area referenced by dot
 *              (2)     loads dot with the pointer to the new area and
 *                      loads the fuzz and last address parameters
 *              (3)     outall() is called to flush any remaining
 *                      bufferred code from the old area to the output
 *
 *      local variables:
 *              area *  oap             pointer to old area
 *
 *      global variables:
 *              sym     dot             defined as sym[0]
 *              a_uint  fuzz            tracks pass to pass changes in the
 *                                      address of symbols caused by
 *                                      variable length instruction formats
 *
 *      functions called:
 *              none
 *
 *      side effects:
 *              Current area saved, new area loaded, buffers flushed.
 */

VOID
newdot(struct area *nap)
{
        struct area *oap;

        oap = dot.s_area;
        /* fprintf (stderr, "%s dot.s_area->a_size: %d dot.s_addr: %d\n",
                oap->a_id, dot.s_area->a_size, dot.s_addr); */
        oap->a_fuzz = fuzz;
        if (oap->a_flag & A_OVR) {
                // the size of an overlay is the biggest size encountered
                if (oap->a_size < dot.s_addr) {
                        oap->a_size = dot.s_addr;
                }
        }
        else if (oap->a_flag & A_ABS) {
                oap->a_addr = dot.s_org;
                oap->a_size += dot.s_addr - dot.s_org;
                dot.s_addr = dot.s_org = 0;
        }
        else {
                oap->a_addr = 0;
                oap->a_size = dot.s_addr;
        }
        if (nap->a_flag & A_OVR) {
                // a new overlay starts at 0, no fuzz
                dot.s_addr = 0;
                fuzz = 0;
        }
        else if (nap->a_flag & A_ABS) {
                // a new absolute starts at org, no fuzz
                dot.s_addr = dot.s_org;
                fuzz = 0;
        }
        else {
                dot.s_addr = nap->a_size;
                fuzz = nap->a_fuzz;
        }
        dot.s_area = nap;
        outall();
}

/*)Function     VOID    phase(ap, a)
 *
 *              area *  ap              pointer to area
 *              a_uint  a               address in area
 *
 *      Function phase() compares the area ap and address a
 *      with the current area dot.s_area and address dot.s_addr
 *      to determine if the position of the symbol has changed
 *      between assembler passes.
 *
 *      local variables:
 *              none
 *
 *      global varaibles:
 *              sym *   dot             defined as sym[0]
 *
 *      functions called:
 *              none
 *
 *      side effects:
 *              The p error is invoked if the area and/or address
 *              has changed.
 */

VOID
phase(struct area *ap, a_uint a)
{
        if (ap != dot.s_area || a != dot.s_addr)
                err('p');
}

char *usetxt[] = {
        "Usage: [-Options] file",
        "Usage: [-Options] outfile file1 [file2 file3 ...]",
        "  -d   Decimal listing",
        "  -q   Octal   listing",
        "  -x   Hex     listing (default)",
        "  -g   Undefined symbols made global",
        "  -a   All user symbols made global",
        "  -b   Display .define substitutions in listing",
        "  -bb  and display without .define substitutions",
        "  -c   Disable instruction cycle count in listing",
#if NOICE
        "  -j   Enable NoICE Debug Symbols",
#endif
#if SDCDB
        "  -y   Enable SDCC  Debug Symbols",
#endif
        "  -l   Create list   file/outfile[.lst]",
        "  -o   Create object file/outfile[.rel]",
        "  -s   Create symbol file/outfile[.sym]",
        "  -p   Disable automatic listing pagination",
        "  -u   Disable .list/.nlist processing",
        "  -w   Wide listing format for symbol table",
        "  -z   Disable case sensitivity for symbols",
        "  -f   Flag relocatable references by  `   in listing file",
        "  -ff  Flag relocatable references by mode in listing file",
        "  -I   Add the named directory to the include file",
        "       search path.  This option may be used more than once.",
        "       Directories are searched in the order given.",
        "",
        NULL
};

/*)Function     VOID    usage(n)
 *
 *              int     n               exit code
 *
 *      The function usage() outputs to the stderr device the
 *      assembler name and version and a list of valid assembler options.
 *
 *      local variables:
 *              char ** dp              pointer to an array of
 *                                      text string pointers.
 *
 *      global variables:
 *              char    cpu[]           assembler type string
 *              char *  usetxt[]        array of string pointers
 *
 *      functions called:
 *              VOID    asexit()        asmain.c
 *              int     fprintf()       c_library
 *
 *      side effects:
 *              program is terminated
 */

VOID
usage(int n)
{
        char   **dp;

        fprintf(stderr, "\n%s Assembler %s  (%s)\n\n", is_sdas() ? "sdas" : "ASxxxx", VERSION, cpu);
        fprintf(stderr, "\nCopyright (C) %s  Alan R. Baldwin", COPYRIGHT);
        fprintf(stderr, "\nThis program comes with ABSOLUTELY NO WARRANTY.\n\n");
        for (dp = usetxt; *dp; dp++)
                fprintf(stderr, "%s\n", *dp);
        asexit(n);
}

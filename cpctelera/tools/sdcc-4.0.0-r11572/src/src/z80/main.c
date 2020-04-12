/*-------------------------------------------------------------------------
  main.c - Z80 specific definitions.

  Michael Hope <michaelh@juju.net.nz> 2001

   This program is free software; you can redistribute it and/or modify it
   under the terms of the GNU General Public License as published by the
   Free Software Foundation; either version 2, or (at your option) any
   later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.

   In other words, you are welcome to use, share and improve this program.
   You are forbidden to forbid anyone else to use, share and improve
   what you give them.   Help stamp out software-hoarding!
-------------------------------------------------------------------------*/

#include <sys/stat.h>
#include "z80.h"
#include "SDCCsystem.h"
#include "SDCCutil.h"
#include "SDCCargs.h"
#include "dbuf_string.h"

#define OPTION_BO              "-bo"
#define OPTION_BA              "-ba"
#define OPTION_CODE_SEG        "--codeseg"
#define OPTION_CONST_SEG       "--constseg"
#define OPTION_DATA_SEG        "--dataseg"
#define OPTION_CALLEE_SAVES_BC "--callee-saves-bc"
#define OPTION_PORTMODE        "--portmode="
#define OPTION_ASM             "--asm="
#define OPTION_NO_STD_CRT0     "--no-std-crt0"
#define OPTION_RESERVE_IY      "--reserve-regs-iy"
#define OPTION_OLDRALLOC       "--oldralloc"
#define OPTION_FRAMEPOINTER    "--fno-omit-frame-pointer"
#define OPTION_EMIT_EXTERNS    "--emit-externs"

static char _z80_defaultRules[] = {
#include "peeph.rul"
#include "peeph-z80.rul"
};

static char _r2k_defaultRules[] = {
#include "peeph.rul"
#include "peeph-r2k.rul"
};

static char _tlcs90_defaultRules[] = {
#include "peeph.rul"
#include "peeph-tlcs90.rul"
};

static char _gbz80_defaultRules[] = {
#include "peeph.rul"
#include "peeph-gbz80.rul"
};



Z80_OPTS z80_opts;

static OPTION _z80_options[] = {
  {0, OPTION_CALLEE_SAVES_BC, &z80_opts.calleeSavesBC, "Force a called function to always save BC"},
  {0, OPTION_PORTMODE,        NULL, "Determine PORT I/O mode (z80/z180)"},
  {0, OPTION_ASM,             NULL, "Define assembler name (rgbds/asxxxx/isas/z80asm)"},
  {0, OPTION_CODE_SEG,        &options.code_seg, "<name> use this name for the code segment", CLAT_STRING},
  {0, OPTION_CONST_SEG,       &options.const_seg, "<name> use this name for the const segment", CLAT_STRING},
  {0, OPTION_DATA_SEG,        &options.data_seg, "<name> use this name for the data segment", CLAT_STRING},
  {0, OPTION_NO_STD_CRT0,     &options.no_std_crt0, "For the z80/gbz80 do not link default crt0.rel"},
  {0, OPTION_RESERVE_IY,      &z80_opts.reserveIY, "Do not use IY (incompatible with --fomit-frame-pointer)"},
  {0, OPTION_OLDRALLOC,       &options.oldralloc, "Use old register allocator"},
  {0, OPTION_FRAMEPOINTER,    &z80_opts.noOmitFramePtr, "Do not omit frame pointer"},
  {0, OPTION_EMIT_EXTERNS,    NULL, "Emit externs list in generated asm"},
  {0, NULL}
};

static OPTION _gbz80_options[] = {
  {0, OPTION_BO,              NULL, "<num> use code bank <num>"},
  {0, OPTION_BA,              NULL, "<num> use data bank <num>"},
  {0, OPTION_CALLEE_SAVES_BC, &z80_opts.calleeSavesBC, "Force a called function to always save BC"},
  {0, OPTION_CODE_SEG,        &options.code_seg, "<name> use this name for the code segment", CLAT_STRING},
  {0, OPTION_CONST_SEG,       &options.const_seg, "<name> use this name for the const segment", CLAT_STRING},
  {0, OPTION_DATA_SEG,        &options.data_seg, "<name> use this name for the data segment", CLAT_STRING},
  {0, OPTION_NO_STD_CRT0,     &options.no_std_crt0, "For the z80/gbz80 do not link default crt0.rel"},
  {0, NULL}
};

typedef enum
{
  /* Must be first */
  ASM_TYPE_ASXXXX,
  ASM_TYPE_RGBDS,
  ASM_TYPE_ISAS,
  ASM_TYPE_Z80ASM
}
ASM_TYPE;

static struct
{
  ASM_TYPE asmType;
  /* determine if we can register a parameter */
  int regParams;
  bool z88dk_fastcall;
}
_G;

static char *_keywords[] = {
  "sfr",
  "nonbanked",
  "banked",
  "at",
  "_naked",
  "critical",
  "interrupt",
  "z88dk_fastcall",
  "z88dk_callee",
  "smallc",
  "z88dk_shortcall",
  "z88dk_params_offset",
  NULL
};

static char *_keywordsgb[] = {
  "sfr",
  "nonbanked",
  "banked",
  "at",
  "_naked",
  "critical",
  "interrupt",
  "z88dk_callee",
  "smallc",
  NULL
};

static char *_keywordstlcs90[] = {
  "nonbanked",
  "banked",
  "at",
  "_naked",
  "critical",
  "interrupt",
  "z88dk_fastcall",
  "z88dk_callee",
  "smallc",
  NULL
};

extern PORT z80_port;
extern PORT r2k_port;
extern PORT gbz80_port;

#include "mappings.i"

static builtins _z80_builtins[] = {
  {"__builtin_memcpy", "vg*", 3, {"vg*", "Cvg*", "Ui"}},
  {"__builtin_strcpy", "cg*", 2, {"cg*", "Ccg*"}},
  {"__builtin_strncpy", "cg*", 3, {"cg*", "Ccg*", "Ui"}},
  {"__builtin_strchr", "cg*", 2, {"Ccg*", "i"}},
  {"__builtin_memset", "vg*", 3, {"vg*", "i", "Ui"}},
  {NULL, NULL, 0, {NULL}}
};

static void
_z80_init (void)
{
  z80_opts.sub = SUB_Z80;
  asm_addTree (&_asxxxx_z80);
}

static void
_z180_init (void)
{
  z80_opts.sub = SUB_Z180;
  asm_addTree (&_asxxxx_z80);
}

static void
_r2k_init (void)
{
  z80_opts.sub = SUB_R2K;
  asm_addTree (&_asxxxx_r2k);
}

static void
_r3ka_init (void)
{
  z80_opts.sub = SUB_R3KA;
  asm_addTree (&_asxxxx_r2k);
}

static void
_gbz80_init (void)
{
  z80_opts.sub = SUB_GBZ80;
}

static void
_tlcs90_init (void)
{
  z80_opts.sub = SUB_TLCS90;
  asm_addTree (&_asxxxx_z80);
}

static void
_ez80_z80_init (void)
{
  z80_opts.sub = SUB_EZ80_Z80;
  asm_addTree (&_asxxxx_z80);
}

static void
_reset_regparm (struct sym_link *funcType)
{
  _G.regParams = 0;
  _G.z88dk_fastcall = IFFUNC_ISZ88DK_FASTCALL (funcType);
  if (_G.z88dk_fastcall && IFFUNC_HASVARARGS (funcType))
    werror (E_Z88DK_FASTCALL_PARAMETERS);
}

static int
_reg_parm (sym_link *l, bool reentrant)
{
  if (_G.z88dk_fastcall)
    {
      if (_G.regParams)
        werror (E_Z88DK_FASTCALL_PARAMETERS);
      if (getSize (l) > 4)
        werror (E_Z88DK_FASTCALL_PARAMETER);
      _G.regParams++;
      return TRUE;
    }
 return FALSE;
}

enum
{
  P_BANK = 1,
  P_PORTMODE,
  P_CODESEG,
  P_CONSTSEG,
};

static int
do_pragma (int id, const char *name, const char *cp)
{
  struct pragma_token_s token;
  int err = 0;
  int processed = 1;

  init_pragma_token (&token);

  switch (id)
    {
    case P_BANK:
      {
        struct dbuf_s buffer;

        dbuf_init (&buffer, 128);

        cp = get_pragma_token (cp, &token);

        switch (token.type)
          {
          case TOKEN_EOL:
            err = 1;
            break;

          case TOKEN_INT:
            switch (_G.asmType)
              {
              case ASM_TYPE_ASXXXX:
                dbuf_printf (&buffer, "CODE_%d", token.val.int_val);
                break;

              case ASM_TYPE_RGBDS:
                dbuf_printf (&buffer, "CODE,BANK[%d]", token.val.int_val);
                break;

              case ASM_TYPE_ISAS:
                /* PENDING: what to use for ISAS? */
                dbuf_printf (&buffer, "CODE,BANK(%d)", token.val.int_val);
                break;

              default:
                wassert (0);
              }
            break;

          default:
            {
              const char *str = get_pragma_string (&token);

              dbuf_append_str (&buffer, (0 == strcmp ("BASE", str)) ? "HOME" : str);
            }
            break;
          }

        cp = get_pragma_token (cp, &token);
        if (TOKEN_EOL != token.type)
          {
            err = 1;
            break;
          }

        dbuf_c_str (&buffer);
        /* ugly, see comment in src/port.h (borutr) */
        gbz80_port.mem.code_name = dbuf_detach (&buffer);
        code->sname = gbz80_port.mem.code_name;
        options.code_seg = (char *) gbz80_port.mem.code_name;
      }
      break;

    case P_PORTMODE:
      {                         /*.p.t.20030716 - adding pragma to manipulate z80 i/o port addressing modes */
        const char *str;

        cp = get_pragma_token (cp, &token);

        if (TOKEN_EOL == token.type)
          {
            err = 1;
            break;
          }

        str = get_pragma_string (&token);

        cp = get_pragma_token (cp, &token);
        if (TOKEN_EOL != token.type)
          {
            err = 1;
            break;
          }

        if (!strcmp (str, "z80"))
          {
            z80_opts.port_mode = 80;
          }
        else if (!strcmp (str, "z180"))
          {
            z80_opts.port_mode = 180;
          }
        else if (!strcmp (str, "save"))
          {
            z80_opts.port_back = z80_opts.port_mode;
          }
        else if (!strcmp (str, "restore"))
          {
            z80_opts.port_mode = z80_opts.port_back;
          }
        else
          err = 1;
      }
      break;

    case P_CODESEG:
    case P_CONSTSEG:
      {
        char *segname;

        cp = get_pragma_token (cp, &token);
        if (token.type == TOKEN_EOL)
          {
            err = 1;
            break;
          }

        segname = Safe_strdup (get_pragma_string (&token));

        cp = get_pragma_token (cp, &token);
        if (token.type != TOKEN_EOL)
          {
            Safe_free (segname);
            err = 1;
            break;
          }

        if (id == P_CODESEG)
          {
            if (options.code_seg)
              Safe_free (options.code_seg);
            options.code_seg = segname;
          }
        else
          {
            if (options.const_seg)
              Safe_free (options.const_seg);
            options.const_seg = segname;
          }
      }
      break;

    default:
      processed = 0;
      break;
    }

  get_pragma_token (cp, &token);

  if (1 == err)
    werror (W_BAD_PRAGMA_ARGUMENTS, name);

  free_pragma_token (&token);
  return processed;
}

static struct pragma_s pragma_tbl[] = {
  {"bank", P_BANK, 0, do_pragma},
  {"portmode", P_PORTMODE, 0, do_pragma},
  {"codeseg", P_CODESEG, 0, do_pragma},
  {"constseg", P_CONSTSEG, 0, do_pragma},
  {NULL, 0, 0, NULL},
};

static int
_process_pragma (const char *s)
{
  return process_pragma_tbl (pragma_tbl, s);
}

static const char *_gbz80_rgbasmCmd[] = {
  "rgbasm", "-o$1.rel", "$1.asm", NULL
};

static const char *_gbz80_rgblinkCmd[] = {
  "xlink", "-tg", "-n$1.sym", "-m$1.map", "-zFF", "$1.lnk", NULL
};

static void
_gbz80_rgblink (void)
{
  FILE *lnkfile;
  struct dbuf_s lnkFileName;
  char *buffer;

  dbuf_init (&lnkFileName, PATH_MAX);

  /* first we need to create the <filename>.lnk file */
  dbuf_append_str (&lnkFileName, dstFileName);
  dbuf_append_str (&lnkFileName, ".lk");
  if (!(lnkfile = fopen (dbuf_c_str (&lnkFileName), "w")))
    {
      werror (E_FILE_OPEN_ERR, dbuf_c_str (&lnkFileName));
      dbuf_destroy (&lnkFileName);
      exit (1);
    }
  dbuf_destroy (&lnkFileName);

  fprintf (lnkfile, "[Objects]\n");

  fprintf (lnkfile, "%s.rel\n", dstFileName);

  fputStrSet (lnkfile, relFilesSet);

  fprintf (lnkfile, "\n[Libraries]\n");
  /* additional libraries if any */
  fputStrSet (lnkfile, libFilesSet);

  fprintf (lnkfile, "\n[Output]\n" "%s.gb", dstFileName);

  fclose (lnkfile);

  buffer = buildCmdLine (port->linker.cmd, dstFileName, NULL, NULL, NULL);
  /* call the linker */
  if (sdcc_system (buffer))
    {
      Safe_free (buffer);
      perror ("Cannot exec linker");
      exit (1);
    }
  Safe_free (buffer);
}

static bool
_parseOptions (int *pargc, char **argv, int *i)
{
  if (argv[*i][0] == '-')
    {
      if (IS_GB)
        {
          if (!strncmp (argv[*i], OPTION_BO, sizeof (OPTION_BO) - 1))
            {
              /* ROM bank */
              int bank = getIntArg (OPTION_BO, argv, i, *pargc);
              struct dbuf_s buffer;

              dbuf_init (&buffer, 16);
              dbuf_printf (&buffer, "CODE_%u", bank);
              dbuf_c_str (&buffer);
              /* ugly, see comment in src/port.h (borutr) */
              gbz80_port.mem.code_name = dbuf_detach (&buffer);
              options.code_seg = (char *) gbz80_port.mem.code_name;
              return TRUE;
            }
          else if (!strncmp (argv[*i], OPTION_BA, sizeof (OPTION_BA) - 1))
            {
              /* RAM bank */
              int bank = getIntArg (OPTION_BA, argv, i, *pargc);
              struct dbuf_s buffer;

              dbuf_init (&buffer, 16);
              dbuf_printf (&buffer, "DATA_%u", bank);
              dbuf_c_str (&buffer);
              /* ugly, see comment in src/port.h (borutr) */
              gbz80_port.mem.data_name = dbuf_detach (&buffer);
              return TRUE;
            }
        }

      if (!strncmp (argv[*i], OPTION_ASM, sizeof (OPTION_ASM) - 1))
        {
          char *asmblr = getStringArg (OPTION_ASM, argv, i, *pargc);

          if (!strcmp (asmblr, "rgbds"))
            {
              asm_addTree (&_rgbds_gb);
              gbz80_port.assembler.cmd = _gbz80_rgbasmCmd;
              gbz80_port.linker.cmd = _gbz80_rgblinkCmd;
              gbz80_port.linker.do_link = _gbz80_rgblink;
              _G.asmType = ASM_TYPE_RGBDS;
              return TRUE;
            }
          else if (!strcmp (asmblr, "asxxxx"))
            {
              _G.asmType = ASM_TYPE_ASXXXX;
              return TRUE;
            }
          else if (!strcmp (asmblr, "isas"))
            {
              asm_addTree (&_isas_gb);
              /* Munge the function prefix */
              gbz80_port.fun_prefix = "";
              _G.asmType = ASM_TYPE_ISAS;
              return TRUE;
            }
          else if (!strcmp (asmblr, "z80asm"))
            {
              port->assembler.externGlobal = TRUE;
              asm_addTree (&_z80asm_z80);
              _G.asmType = ASM_TYPE_ISAS;
              return TRUE;
            }
        }
      else if (!strncmp (argv[*i], OPTION_PORTMODE, sizeof (OPTION_PORTMODE) - 1))
        {
          char *portmode = getStringArg (OPTION_ASM, argv, i, *pargc);

          if (!strcmp (portmode, "z80"))
            {
              z80_opts.port_mode = 80;
              return TRUE;
            }
          else if (!strcmp (portmode, "z180"))
            {
              z80_opts.port_mode = 180;
              return TRUE;
            }
        }
      else if (!strncmp (argv[*i], OPTION_EMIT_EXTERNS, sizeof (OPTION_EMIT_EXTERNS) - 1))
        {
          port->assembler.externGlobal = 1;
          return TRUE;
        }
    }
  return FALSE;
}

static void
_setValues (void)
{
  const char *s;
  struct dbuf_s dbuf;

  if (options.nostdlib == FALSE)
    {
      const char *s;
      char *path;
      struct dbuf_s dbuf;

      dbuf_init (&dbuf, PATH_MAX);

      for (s = setFirstItem (libDirsSet); s != NULL; s = setNextItem (libDirsSet))
        {
          path = buildCmdLine2 ("-k\"%s" DIR_SEPARATOR_STRING "{port}\" ", s);
          dbuf_append_str (&dbuf, path);
          Safe_free (path);
        }
      path = buildCmdLine2 ("-l\"{port}.lib\"", s);
      dbuf_append_str (&dbuf, path);
      Safe_free (path);

      setMainValue ("z80libspec", dbuf_c_str (&dbuf));
      dbuf_destroy (&dbuf);

      for (s = setFirstItem (libDirsSet); s != NULL; s = setNextItem (libDirsSet))
        {
          struct stat stat_buf;

          path = buildCmdLine2 ("%s" DIR_SEPARATOR_STRING "{port}" DIR_SEPARATOR_STRING "crt0{objext}", s);
          if (stat (path, &stat_buf) == 0)
            {
              Safe_free (path);
              break;
            }
          else
            Safe_free (path);
        }

      if (s == NULL)
        setMainValue ("z80crt0", "\"crt0{objext}\"");
      else
        {
          struct dbuf_s dbuf;

          dbuf_init (&dbuf, 128);
          dbuf_printf (&dbuf, "\"%s\"", path);
          setMainValue ("z80crt0", dbuf_c_str (&dbuf));
          dbuf_destroy (&dbuf);
        }
    }
  else
    {
      setMainValue ("z80libspec", "");
      setMainValue ("z80crt0", "");
    }

  setMainValue ("z80extralibfiles", (s = joinStrSet (libFilesSet)));
  Safe_free ((void *) s);
  setMainValue ("z80extralibpaths", (s = joinStrSet (libPathsSet)));
  Safe_free ((void *) s);

  if (IS_GB)
    {
      setMainValue ("z80outputtypeflag", "-Z");
      setMainValue ("z80outext", ".gb");
    }
  else
    {
      setMainValue ("z80outputtypeflag", "-i");
      setMainValue ("z80outext", ".ihx");
    }

  setMainValue ("stdobjdstfilename", "{dstfilename}{objext}");
  setMainValue ("stdlinkdstfilename", "{dstfilename}{z80outext}");

  setMainValue ("z80extraobj", (s = joinStrSet (relFilesSet)));
  Safe_free ((void *) s);

  dbuf_init (&dbuf, 128);
  dbuf_printf (&dbuf, "-b_CODE=0x%04X -b_DATA=0x%04X", options.code_loc, options.data_loc);
  setMainValue ("z80bases", dbuf_c_str (&dbuf));
  dbuf_destroy (&dbuf);

  /* For the old register allocator (with the new one we decide to omit the frame pointer for each function individually) */
  if (!IS_GB && options.omitFramePtr)
    port->stack.call_overhead = 2;
}

static void
_finaliseOptions (void)
{
  port->mem.default_local_map = data;
  port->mem.default_globl_map = data;
  if (_G.asmType == ASM_TYPE_ASXXXX && IS_GB)
    asm_addTree (&_asxxxx_gb);

  if (IY_RESERVED)
    port->num_regs -= 2;

  _setValues ();
}

static void
_setDefaultOptions (void)
{
  options.nopeep = 0;
  options.stackAuto = 1;
  /* first the options part */
  options.intlong_rent = 1;
  options.float_rent = 1;
  options.noRegParams = 0;
  /* Default code and data locations. */
  options.code_loc = 0x200;

  options.data_loc = IS_GB ? 0xC000 : 0x8000;
  options.out_fmt = 'i';        /* Default output format is ihx */
}

#if 0
/* Mangling format:
    _fun_policy_params
    where:
      policy is the function policy
      params is the parameter format

   policy format:
    rsp
    where:
      r is 'r' for reentrant, 's' for static functions
      s is 'c' for callee saves, 'r' for caller saves
      f is 'f' for profiling on, 'x' for profiling off
    examples:
      rr - reentrant, caller saves
   params format:
    A combination of register short names and s to signify stack variables.
    examples:
      bds - first two args appear in BC and DE, the rest on the stack
      s - all arguments are on the stack.
*/
static const char *
_mangleSupportFunctionName (const char *original)
{
  struct dbuf_s dbuf;

  if (strstr (original, "longlong"))
    return (original);

  dbuf_init (&dbuf, 128);
  dbuf_printf (&dbuf, "%s_rr%s_%s", original, options.profile ? "f" : "x", options.noRegParams ? "s" : "bds"    /* MB: but the library only has hds variants ??? */
    );

  return dbuf_detach_c_str (&dbuf);
}
#endif

static const char *
_getRegName (const struct reg_info *reg)
{
  if (reg)
    {
      return reg->name;
    }
  /*  assert (0); */
  return "err";
}

static int
_getRegByName (const char *name)
{
  if (!strcmp (name, "a"))
    return 0;
  if (!strcmp (name, "c"))
    return 1;
  if (!strcmp (name, "b"))
    return 2;
  if (!strcmp (name, "e"))
    return 3;
  if (!strcmp (name, "d"))
    return 4;
  if (!strcmp (name, "l"))
    return 5;
  if (!strcmp (name, "h"))
    return 6;
  if (!strcmp (name, "iyl"))
    return 7;
  if (!strcmp (name, "iyh"))
    return 8;
  return -1;
}

static bool
_hasNativeMulFor (iCode *ic, sym_link *left, sym_link *right)
{
  sym_link *test = NULL;
  int result_size = IS_SYMOP (IC_RESULT(ic)) ? getSize (OP_SYM_TYPE (IC_RESULT(ic))) : 4;

  if (ic->op != '*')
    {
      return FALSE;
    }

  if (IS_LITERAL (left))
    test = left;
  else if (IS_LITERAL (right))
    test = right;
  /* 8x8 unsigned multiplication code is shorter than
     call overhead for the multiplication routine. */
  else if (IS_CHAR (right) && IS_UNSIGNED (right) && IS_CHAR (left) && IS_UNSIGNED (left) && !IS_GB)
    {
      return TRUE;
    }
  /* Same for any multiplication with 8 bit result. */
  else if (result_size == 1 && !IS_GB)
    {
      return TRUE;
    }
  else
    {
      return FALSE;
    }

  if (getSize (test) <= 2)
    {
      return TRUE;
    }

  return FALSE;
}

/* Indicate which extended bit operations this port supports */
static bool
hasExtBitOp (int op, int size)
{
  if (op == GETHBIT)
    return TRUE;
  else
    return FALSE;
}

/* Indicate the expense of an access to an output storage class */
static int
oclsExpense (struct memmap *oclass)
{
  if (IN_FARSPACE (oclass))
    return 1;

  return 0;
}


//#define LINKCMD "sdld{port} -nf {dstfilename}"
/*
#define LINKCMD \
    "sdld{port} -n -c -- {z80bases} -m -j" \
    " {z80libspec}" \
    " {z80extralibfiles} {z80extralibpaths}" \
    " {z80outputtypeflag} \"{linkdstfilename}\"" \
    " {z80crt0}" \
    " \"{dstfilename}{objext}\"" \
    " {z80extraobj}"
*/

static const char *_z80LinkCmd[] = {
  "sdldz80", "-nf", "$1", NULL
};

static const char *_gbLinkCmd[] = {
  "sdldgb", "-nf", "$1", NULL
};

/* $3 is replaced by assembler.debug_opts resp. port->assembler.plain_opts */
static const char *_z80AsmCmd[] = {
  "sdasz80", "$l", "$3", "$2", "$1.asm", NULL
};

static const char *_r2kAsmCmd[] = {
  "sdasrab", "$l", "$3", "$2", "$1.asm", NULL
};

static const char *_gbAsmCmd[] = {
  "sdasgb", "$l", "$3", "$2", "$1.asm", NULL
};

static const char *_tlcs90AsmCmd[] = {
  "sdastlcs90", "$l", "$3", "$2", "$1.asm", NULL
};

static const char *const _crt[] = { "crt0.rel", NULL, };
static const char *const _libs_z80[] = { "z80", NULL, };
static const char *const _libs_z180[] = { "z180", NULL, };
static const char *const _libs_r2k[] = { "r2k", NULL, };
static const char *const _libs_r3ka[] = { "r3ka", NULL, };
static const char *const _libs_tlcs90[] = { "tlcs90", NULL, };
static const char *const _libs_gb[] = { "gbz80", NULL, };
static const char *const _libs_ez80_z80[] = { "ez80_z80", NULL, };

/* Globals */
PORT z80_port =
{
  TARGET_ID_Z80,
  "z80",
  "Zilog Z80",                  /* Target name */
  NULL,                         /* Processor name */
  {
    glue,
    FALSE,
    NO_MODEL,
    NO_MODEL,
    NULL,                       /* model == target */
  },
  {                             /* Assembler */
    _z80AsmCmd,
    NULL,
    "-plosgffwy",               /* Options with debug */
    "-plosgffw",                /* Options without debug */
    0,
    ".asm"
  },
  {                             /* Linker */
    _z80LinkCmd,                //NULL,
    NULL,                       //LINKCMD,
    NULL,
    ".rel",
    1,                          /* needLinkerScript */
    _crt,                       /* crt */
    _libs_z80,                  /* libs */
  },
  {                             /* Peephole optimizer */
    _z80_defaultRules,
    z80instructionSize,
    0,
    0,
    0,
    z80notUsed,
    z80canAssign,
    z80notUsedFrom,
    z80symmParmStack,
  },
  /* Sizes: char, short, int, long, long long, near ptr, far ptr, gptr, func ptr, banked func ptr, bit, float */
  { 1, 2, 2, 4, 8, 2, 2, 2, 2, 2, 1, 4 },
  /* tags for generic pointers */
  { 0x00, 0x40, 0x60, 0x80 },   /* far, near, xstack, code */
  {
    "XSEG",
    "STACK",
    "CODE",
    "DATA",
    NULL,                       /* idata */
    NULL,                       /* pdata */
    NULL,                       /* xdata */
    NULL,                       /* bit */
    "RSEG (ABS)",
    "GSINIT",                   /* static initialization */
    NULL,                       /* overlay */
    "GSFINAL",
    "HOME",
    NULL,                       /* xidata */
    NULL,                       /* xinit */
    NULL,                       /* const_name */
    "CABS (ABS)",               /* cabs_name */
    "DABS (ABS)",               /* xabs_name */
    NULL,                       /* iabs_name */
    "INITIALIZED",              /* name of segment for initialized variables */
    "INITIALIZER",              /* name of segment for copies of initialized variables in code space */
    NULL,
    NULL,
    1,                          /* CODE  is read-only */
    1                           /* No fancy alignments supported. */
  },
  { NULL, NULL },
  { -1, 0, 0, 4, 0, 2, 0 },
  { -1, FALSE },
  { z80_emitDebuggerSymbol },
  {
    256,                        /* maxCount */
    3,                          /* sizeofElement */
    {6, 7, 8},                  /* sizeofMatchJump[] - Assumes operand allocated to registers */
    {6, 9, 15},                 /* sizeofRangeCompare[] - Assumes operand allocated to registers*/
    1,                          /* sizeofSubtract - Assumes use of a singel inc or dec */
    9,                          /* sizeofDispatch - Assumes operand allocated to register e or c*/
  },
  "_",
  _z80_init,
  _parseOptions,
  _z80_options,
  NULL,
  _finaliseOptions,
  _setDefaultOptions,
  z80_assignRegisters,
  _getRegName,
  _getRegByName,
  NULL,
  _keywords,
  0,                            /* no assembler preamble */
  NULL,                         /* no genAssemblerEnd */
  0,                            /* no local IVT generation code */
  0,                            /* no genXINIT code */
  NULL,                         /* genInitStartup */
  _reset_regparm,
  _reg_parm,
  _process_pragma,
  NULL,
  _hasNativeMulFor,
  hasExtBitOp,                  /* hasExtBitOp */
  oclsExpense,                  /* oclsExpense */
  TRUE,
  TRUE,                         /* little endian */
  0,                            /* leave lt */
  0,                            /* leave gt */
  1,                            /* transform <= to ! > */
  1,                            /* transform >= to ! < */
  1,                            /* transform != to !(a == b) */
  0,                            /* leave == */
  FALSE,                        /* Array initializer support. */
  0,                            /* no CSE cost estimation yet */
  _z80_builtins,                /* builtin functions */
  GPOINTER,                     /* treat unqualified pointers as "generic" pointers */
  1,                            /* reset labelKey to 1 */
  1,                            /* globals & local statics allowed */
  9,                            /* Number of registers handled in the tree-decomposition-based register allocator in SDCCralloc.hpp */
  PORT_MAGIC
};

PORT z180_port =
{
  TARGET_ID_Z180,
  "z180",
  "Zilog Z180",                  /* Target name */
  NULL,                         /* Processor name */
  {
    glue,
    FALSE,
    NO_MODEL,
    NO_MODEL,
    NULL,                       /* model == target */
  },
  {                             /* Assembler */
    _z80AsmCmd,
    NULL,
    "-plosgffwy",               /* Options with debug */
    "-plosgffw",                /* Options without debug */
    0,
    ".asm"
  },
  {                             /* Linker */
    _z80LinkCmd,                //NULL,
    NULL,                       //LINKCMD,
    NULL,
    ".rel",
    1,
    _crt,                       /* crt */
    _libs_z180,                 /* libs */
  },
  {                             /* Peephole optimizer */
    _z80_defaultRules,
    z80instructionSize,
    NULL,
    NULL,
    NULL,
    z80notUsed,
    z80canAssign,
    z80notUsedFrom,
    z80symmParmStack,
  },
  /* Sizes: char, short, int, long, long long, near ptr, far ptr, gptr, func ptr, banked func ptr, bit, float */
  { 1, 2, 2, 4, 8, 2, 2, 2, 2, 2, 1, 4 },
  /* tags for generic pointers */
  { 0x00, 0x40, 0x60, 0x80 },   /* far, near, xstack, code */
  {
    "XSEG",
    "STACK",
    "CODE",
    "DATA",
    NULL,                       /* idata */
    NULL,                       /* pdata */
    NULL,                       /* xdata */
    NULL,                       /* bit */
    "RSEG (ABS)",
    "GSINIT",
    NULL,                       /* overlay */
    "GSFINAL",
    "HOME",
    NULL,                       /* xidata */
    NULL,                       /* xinit */
    NULL,                       /* const_name */
    "CABS (ABS)",               /* cabs_name */
    "DABS (ABS)",               /* xabs_name */
    NULL,                       /* iabs_name */
    "INITIALIZED",              /* name of segment for initialized variables */
    "INITIALIZER",              /* name of segment for copies of initialized variables in code space */
    NULL,
    NULL,
    1,                          /* CODE  is read-only */
    1                           /* No fancy alignments supported. */
  },
  { NULL, NULL },
  { -1, 0, 0, 4, 0, 2, 0 },
  { -1, FALSE },
  { z80_emitDebuggerSymbol },
  {
    256,                        /* maxCount */
    3,                          /* sizeofElement */
    {6, 7, 8},                  /* sizeofMatchJump[] - Assumes operand allocated to registers */
    {6, 9, 15},                 /* sizeofRangeCompare[] - Assumes operand allocated to registers*/
    1,                          /* sizeofSubtract - Assumes use of a singel inc or dec */
    9,                          /* sizeofDispatch - Assumes operand allocated to register e or c*/
  },
  "_",
  _z180_init,
  _parseOptions,
  _z80_options,
  NULL,
  _finaliseOptions,
  _setDefaultOptions,
  z80_assignRegisters,
  _getRegName,
  _getRegByName,
  NULL,
  _keywords,
  0,                            /* no assembler preamble */
  NULL,                         /* no genAssemblerEnd */
  0,                            /* no local IVT generation code */
  0,                            /* no genXINIT code */
  NULL,                         /* genInitStartup */
  _reset_regparm,
  _reg_parm,
  _process_pragma,
  NULL,
  _hasNativeMulFor,
  hasExtBitOp,                  /* hasExtBitOp */
  oclsExpense,                  /* oclsExpense */
  TRUE,
  TRUE,                         /* little endian */
  0,                            /* leave lt */
  0,                            /* leave gt */
  1,                            /* transform <= to ! > */
  1,                            /* transform >= to ! < */
  1,                            /* transform != to !(a == b) */
  0,                            /* leave == */
  FALSE,                        /* Array initializer support. */
  0,                            /* no CSE cost estimation yet */
  _z80_builtins,                /* builtin functions */
  GPOINTER,                     /* treat unqualified pointers as "generic" pointers */
  1,                            /* reset labelKey to 1 */
  1,                            /* globals & local statics allowed */
  9,                            /* Number of registers handled in the tree-decomposition-based register allocator in SDCCralloc.hpp */
  PORT_MAGIC
};

PORT r2k_port =
{
  TARGET_ID_R2K,
  "r2k",
  "Rabbit 2000",                /* Target name */
  NULL,                         /* Processor name */
  {
    glue,
    FALSE,
    NO_MODEL,
    NO_MODEL,
    NULL,                       /* model == target */
  },
  {                             /* Assembler */
    _r2kAsmCmd,
    NULL,
    "-plosgffwy",               /* Options with debug */
    "-plosgffw",                /* Options without debug */
    0,
    ".asm"},
  {                             /* Linker */
    _z80LinkCmd,                //NULL,
    NULL,                       //LINKCMD,
    NULL,
    ".rel",
    1,
    _crt,                       /* crt */
    _libs_r2k,                  /* libs */
  },
  {                             /* Peephole optimizer */
    _r2k_defaultRules,
    z80instructionSize,
    NULL,
    NULL,
    NULL,
    z80notUsed,
    z80canAssign,
    z80notUsedFrom,
    z80symmParmStack,
  },
  /* Sizes: char, short, int, long, long long, near ptr, far ptr, gptr, func ptr, banked func ptr, bit, float */
  { 1, 2, 2, 4, 8, 2, 2, 2, 2, 2, 1, 4 },
  /* tags for generic pointers */
  { 0x00, 0x40, 0x60, 0x80 },   /* far, near, xstack, code */
  {
    "XSEG",
    "STACK",
    "CODE",
    "DATA",
    NULL,                       /* idata */
    NULL,                       /* pdata */
    NULL,                       /* xdata */
    NULL,                       /* bit */
    "RSEG (ABS)",
    "GSINIT",
    NULL,                       /* overlay */
    "GSFINAL",
    "HOME",
    NULL,                       /* xidata */
    NULL,                       /* xinit */
    NULL,                       /* const_name */
    "CABS (ABS)",               /* cabs_name */
    "DABS (ABS)",               /* xabs_name */
    NULL,                       /* iabs_name */
    "INITIALIZED",              /* name of segment for initialized variables */
    "INITIALIZER",              /* name of segment for copies of initialized variables in code space */
    NULL,
    NULL,
    1,                          /* CODE  is read-only */
    1                           /* No fancy alignments supported. */
  },
  { NULL, NULL },
  { -1, 0, 0, 4, 0, 2, 0 },
  { -1, FALSE },
  { z80_emitDebuggerSymbol },
  {
    256,                        /* maxCount */
    3,                          /* sizeofElement */
    {6, 7, 8},                  /* sizeofMatchJump[] - Assumes operand allocated to registers */
    {6, 9, 15},                 /* sizeofRangeCompare[] - Assumes operand allocated to registers*/
    1,                          /* sizeofSubtract - Assumes use of a singel inc or dec */
    9,                          /* sizeofDispatch - Assumes operand allocated to register e or c*/
  },
  "_",
  _r2k_init,
  _parseOptions,
  _z80_options,
  NULL,
  _finaliseOptions,
  _setDefaultOptions,
  z80_assignRegisters,
  _getRegName,
  _getRegByName,
  NULL,
  _keywords,
  0,                            /* no assembler preamble */
  NULL,                         /* no genAssemblerEnd */
  0,                            /* no local IVT generation code */
  0,                            /* no genXINIT code */
  NULL,                         /* genInitStartup */
  _reset_regparm,
  _reg_parm,
  _process_pragma,
  NULL,
  _hasNativeMulFor,
  hasExtBitOp,                  /* hasExtBitOp */
  oclsExpense,                  /* oclsExpense */
  TRUE,
  TRUE,                         /* little endian */
  0,                            /* leave lt */
  0,                            /* leave gt */
  1,                            /* transform <= to ! > */
  1,                            /* transform >= to ! < */
  1,                            /* transform != to !(a == b) */
  0,                            /* leave == */
  FALSE,                        /* Array initializer support. */
  0,                            /* no CSE cost estimation yet */
  _z80_builtins,                /* builtin functions */
  GPOINTER,                     /* treat unqualified pointers as "generic" pointers */
  1,                            /* reset labelKey to 1 */
  1,                            /* globals & local statics allowed */
  9,                            /* Number of registers handled in the tree-decomposition-based register allocator in SDCCralloc.hpp */
  PORT_MAGIC
};

PORT r3ka_port =
{
  TARGET_ID_R3KA,
  "r3ka",
  "Rabbit 3000A",               /* Target name */
  NULL,                         /* Processor name */
  {
    glue,
    FALSE,
    NO_MODEL,
    NO_MODEL,
    NULL,                       /* model == target */
  },
  {                             /* Assembler */
    _r2kAsmCmd,
    NULL,
    "-plosgffwy",               /* Options with debug */
    "-plosgffw",                /* Options without debug */
    0,
    ".asm"
  },
  {                             /* Linker */
    _z80LinkCmd,                //NULL,
    NULL,                       //LINKCMD,
    NULL,
    ".rel",
    1,
    _crt,                       /* crt */
    _libs_r3ka,                 /* libs */
  },
  {                             /* Peephole optimizer */
    _r2k_defaultRules,
    z80instructionSize,
    0,
    0,
    0,
    z80notUsed,
    z80canAssign,
    z80notUsedFrom,
    z80symmParmStack,
  },
  /* Sizes: char, short, int, long, long long, near ptr, far ptr, gptr, func ptr, banked func ptr, bit, float */
  { 1, 2, 2, 4, 8, 2, 2, 2, 2, 2, 1, 4 },
  /* tags for generic pointers */
  { 0x00, 0x40, 0x60, 0x80 },   /* far, near, xstack, code */
  {
    "XSEG",
    "STACK",
    "CODE",
    "DATA",
    NULL,                       /* idata */
    NULL,                       /* pdata */
    NULL,                       /* xdata */
    NULL,                       /* bit */
    "RSEG (ABS)",
    "GSINIT",
    NULL,                       /* overlay */
    "GSFINAL",
    "HOME",
    NULL,                       /* xidata */
    NULL,                       /* xinit */
    NULL,                       /* const_name */
    "CABS (ABS)",               /* cabs_name */
    "DABS (ABS)",               /* xabs_name */
    NULL,                       /* iabs_name */
    "INITIALIZED",              /* name of segment for initialized variables */
    "INITIALIZER",              /* name of segment for copies of initialized variables in code space */
    NULL,
    NULL,
    1,                          /* CODE  is read-only */
    1                           /* No fancy alignments supported. */
  },
  { NULL, NULL },
  { -1, 0, 0, 4, 0, 2, 0 },
  { -1, FALSE },
  { z80_emitDebuggerSymbol },
  {
    256,                        /* maxCount */
    3,                          /* sizeofElement */
    {6, 7, 8},                  /* sizeofMatchJump[] - Assumes operand allocated to registers */
    {6, 9, 15},                 /* sizeofRangeCompare[] - Assumes operand allocated to registers*/
    1,                          /* sizeofSubtract - Assumes use of a singel inc or dec */
    9,                          /* sizeofDispatch - Assumes operand allocated to register e or c*/
  },
  "_",
  _r3ka_init,
  _parseOptions,
  _z80_options,
  NULL,
  _finaliseOptions,
  _setDefaultOptions,
  z80_assignRegisters,
  _getRegName,
  _getRegByName,
  NULL,
  _keywords,
  0,                            /* no assembler preamble */
  NULL,                         /* no genAssemblerEnd */
  0,                            /* no local IVT generation code */
  0,                            /* no genXINIT code */
  NULL,                         /* genInitStartup */
  _reset_regparm,
  _reg_parm,
  _process_pragma,
  NULL,
  _hasNativeMulFor,
  hasExtBitOp,                  /* hasExtBitOp */
  oclsExpense,                  /* oclsExpense */
  TRUE,
  TRUE,                         /* little endian */
  0,                            /* leave lt */
  0,                            /* leave gt */
  1,                            /* transform <= to ! > */
  1,                            /* transform >= to ! < */
  1,                            /* transform != to !(a == b) */
  0,                            /* leave == */
  FALSE,                        /* Array initializer support. */
  0,                            /* no CSE cost estimation yet */
  _z80_builtins,                /* builtin functions */
  GPOINTER,                     /* treat unqualified pointers as "generic" pointers */
  1,                            /* reset labelKey to 1 */
  1,                            /* globals & local statics allowed */
  9,                            /* Number of registers handled in the tree-decomposition-based register allocator in SDCCralloc.hpp */
  PORT_MAGIC
};

/* Globals */
PORT gbz80_port =
{
  TARGET_ID_GBZ80,
  "gbz80",
  "Gameboy Z80-like",           /* Target name */
  NULL,
  {
    glue,
    FALSE,
    NO_MODEL,
    NO_MODEL,
    NULL,                       /* model == target */
  },
  {                             /* Assembler */
    _gbAsmCmd,
    NULL,
    "-plosgffwy",               /* Options with debug */
    "-plosgffw",                /* Options without debug */
    0,
    ".asm",
    NULL                        /* no do_assemble function */
  },
  {                             /* Linker */
    _gbLinkCmd,                 //NULL,
    NULL,                       //LINKCMD,
    NULL,
    ".rel",
    1,
    _crt,                       /* crt */
    _libs_gb,                   /* libs */
  },
  {                             /* Peephole optimizer */
    _gbz80_defaultRules,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    z80canAssign,
    NULL,
    z80symmParmStack,
  },
  /* Sizes: char, short, int, long, long long, near ptr, far ptr, gptr, func ptr, banked func ptr, bit, float */
  { 1, 2, 2, 4, 8, 2, 2, 2, 2, 2, 1, 4 },
  /* tags for generic pointers */
  { 0x00, 0x40, 0x60, 0x80 },   /* far, near, xstack, code */
  {
    "XSEG",
    "STACK",
    "CODE",
    "DATA",
    NULL,                       /* idata */
    NULL,                       /* pdata */
    NULL,                       /* xdata */
    NULL,                       /* bit */
    "RSEG",
    "GSINIT",
    NULL,                       /* overlay */
    "GSFINAL",
    "HOME",
    NULL,                       /* xidata */
    NULL,                       /* xinit */
    NULL,                       /* const_name */
    "CABS (ABS)",               /* cabs_name */
    "DABS (ABS)",               /* xabs_name */
    NULL,                       /* iabs_name */
    NULL,                       /* name of segment for initialized variables */
    NULL,                       /* name of segment for copies of initialized variables in code space */
    NULL,
    NULL,
    1,                          /* CODE is read-only */
    1                           /* No fancy alignments supported. */
  },
  { NULL, NULL },
  { -1, 0, 0, 2, 0, 4, 0 },
  { -1, FALSE },
  { z80_emitDebuggerSymbol },
  {
    256,                        /* maxCount */
    3,                          /* sizeofElement */
    {6, 7, 8},                  /* sizeofMatchJump[] - Assumes operand allocated to registers */
    {6, 9, 15},                 /* sizeofRangeCompare[] - Assumes operand allocated to registers*/
    1,                          /* sizeofSubtract - Assumes use of a singel inc or dec */
    9,                          /* sizeofDispatch - Assumes operand allocated to register e or c*/
  },
  "_",
  _gbz80_init,
  _parseOptions,
  _gbz80_options,
  NULL,
  _finaliseOptions,
  _setDefaultOptions,
  z80_assignRegisters,
  _getRegName,
  _getRegByName,
  NULL,
  _keywordsgb,
  0,                            /* no assembler preamble */
  NULL,                         /* no genAssemblerEnd */
  0,                            /* no local IVT generation code */
  0,                            /* no genXINIT code */
  NULL,                         /* genInitStartup */
  _reset_regparm,
  _reg_parm,
  _process_pragma,
  NULL,
  _hasNativeMulFor,
  hasExtBitOp,                  /* hasExtBitOp */
  oclsExpense,                  /* oclsExpense */
  TRUE,
  TRUE,                         /* little endian */
  0,                            /* leave lt */
  0,                            /* leave gt */
  1,                            /* transform <= to ! > */
  1,                            /* transform >= to ! < */
  1,                            /* transform != to !(a == b) */
  0,                            /* leave == */
  FALSE,                        /* Array initializer support. */
  0,                            /* no CSE cost estimation yet */
  NULL,                         /* no builtin functions */
  GPOINTER,                     /* treat unqualified pointers as "generic" pointers */
  1,                            /* reset labelKey to 1 */
  1,                            /* globals & local statics allowed */
  5,                            /* Number of registers handled in the tree-decomposition-based register allocator in SDCCralloc.hpp */
  PORT_MAGIC
};

PORT tlcs90_port =
{
  TARGET_ID_TLCS90,
  "tlcs90",
  "Toshiba TLCS-90",            /* Target name */
  NULL,                         /* Processor name */
  {
    glue,
    FALSE,
    NO_MODEL,
    NO_MODEL,
    NULL,                       /* model == target */
  },
  {                             /* Assembler */
    _tlcs90AsmCmd,
    NULL,
    "-plosgffwy",               /* Options with debug */
    "-plosgffw",                /* Options without debug */
    0,
    ".asm"
  },
  {                             /* Linker */
    _z80LinkCmd,                //NULL,
    NULL,                       //LINKCMD,
    NULL,
    ".rel",
    1,
    _crt,                       /* crt */
    _libs_tlcs90,               /* libs */
  },
  {                             /* Peephole optimizer */
    _tlcs90_defaultRules,
    z80instructionSize,
    NULL,
    NULL,
    NULL,
    z80notUsed,
    z80canAssign,
    z80notUsedFrom,
    z80symmParmStack,
  },
  /* Sizes: char, short, int, long, long long, near ptr, far ptr, gptr, func ptr, banked func ptr, bit, float */
  { 1, 2, 2, 4, 8, 2, 2, 2, 2, 2, 1, 4 },
  /* tags for generic pointers */
  { 0x00, 0x40, 0x60, 0x80 },   /* far, near, xstack, code */
  {
    "XSEG",
    "STACK",
    "CODE",
    "DATA",
    NULL,                       /* idata */
    NULL,                       /* pdata */
    NULL,                       /* xdata */
    NULL,                       /* bit */
    "RSEG (ABS)",
    "GSINIT",                   /* static initialization */
    NULL,                       /* overlay */
    "GSFINAL",
    "HOME",
    NULL,                       /* xidata */
    NULL,                       /* xinit */
    NULL,                       /* const_name */
    "CABS (ABS)",               /* cabs_name */
    "DABS (ABS)",               /* xabs_name */
    NULL,                       /* iabs_name */
    "INITIALIZED",              /* name of segment for initialized variables */
    "INITIALIZER",              /* name of segment for copies of initialized variables in code space */
    NULL,
    NULL,
    1,                          /* CODE  is read-only */
    1                           /* No fancy alignments supported. */
   },
  { NULL, NULL },
  { -1, 0, 0, 4, 0, 2, 0 },
  { -1, FALSE },
  { z80_emitDebuggerSymbol },
  {
    256,                        /* maxCount */
    3,                          /* sizeofElement */
    {6, 7, 8},                  /* sizeofMatchJump[] - Assumes operand allocated to registers */
    {6, 9, 15},                 /* sizeofRangeCompare[] - Assumes operand allocated to registers*/
    1,                          /* sizeofSubtract - Assumes use of a singel inc or dec */
    9,                          /* sizeofDispatch - Assumes operand allocated to register e or c*/
  },
  "_",
  _tlcs90_init,
  _parseOptions,
  _z80_options,
  NULL,
  _finaliseOptions,
  _setDefaultOptions,
  z80_assignRegisters,
  _getRegName,
  _getRegByName,
  NULL,
  _keywordstlcs90,
  0,                            /* no assembler preamble */
  NULL,                         /* no genAssemblerEnd */
  0,                            /* no local IVT generation code */
  0,                            /* no genXINIT code */
  NULL,                         /* genInitStartup */
  _reset_regparm,
  _reg_parm,
  _process_pragma,
  NULL,
  _hasNativeMulFor,
  hasExtBitOp,                  /* hasExtBitOp */
  oclsExpense,                  /* oclsExpense */
  TRUE,
  TRUE,                         /* little endian */
  0,                            /* leave lt */
  0,                            /* leave gt */
  1,                            /* transform <= to ! > */
  1,                            /* transform >= to ! < */
  1,                            /* transform != to !(a == b) */
  0,                            /* leave == */
  FALSE,                        /* Array initializer support. */
  0,                            /* no CSE cost estimation yet */
  _z80_builtins,                /* builtin functions */
  GPOINTER,                     /* treat unqualified pointers as "generic" pointers */
  1,                            /* reset labelKey to 1 */
  1,                            /* globals & local statics allowed */
  9,                            /* Number of registers handled in the tree-decomposition-based register allocator in SDCCralloc.hpp */
  PORT_MAGIC
};

PORT ez80_z80_port =
{
  TARGET_ID_EZ80_Z80,
  "ez80_z80",
  "eZ80-Z80",                  /* Target name */
  NULL,                         /* Processor name */
  {
    glue,
    FALSE,
    NO_MODEL,
    NO_MODEL,
    NULL,                       /* model == target */
  },
  {                             /* Assembler */
    _z80AsmCmd,
    NULL,
    "-plosgffwy",               /* Options with debug */
    "-plosgffw",                /* Options without debug */
    0,
    ".asm"
  },
  {                             /* Linker */
    _z80LinkCmd,                //NULL,
    NULL,                       //LINKCMD,
    NULL,
    ".rel",
    1,
    _crt,                       /* crt */
    _libs_ez80_z80,                 /* libs */
  },
  {                             /* Peephole optimizer */
    _z80_defaultRules,
    z80instructionSize,
    NULL,
    NULL,
    NULL,
    z80notUsed,
    z80canAssign,
    z80notUsedFrom,
    z80symmParmStack,
  },
  /* Sizes: char, short, int, long, long long, near ptr, far ptr, gptr, func ptr, banked func ptr, bit, float */
  { 1, 2, 2, 4, 8, 2, 2, 2, 2, 2, 1, 4 },
  /* tags for generic pointers */
  { 0x00, 0x40, 0x60, 0x80 },   /* far, near, xstack, code */
  {
    "XSEG",
    "STACK",
    "CODE",
    "DATA",
    NULL,                       /* idata */
    NULL,                       /* pdata */
    NULL,                       /* xdata */
    NULL,                       /* bit */
    "RSEG (ABS)",
    "GSINIT",
    NULL,                       /* overlay */
    "GSFINAL",
    "HOME",
    NULL,                       /* xidata */
    NULL,                       /* xinit */
    NULL,                       /* const_name */
    "CABS (ABS)",               /* cabs_name */
    "DABS (ABS)",               /* xabs_name */
    NULL,                       /* iabs_name */
    "INITIALIZED",              /* name of segment for initialized variables */
    "INITIALIZER",              /* name of segment for copies of initialized variables in code space */
    NULL,
    NULL,
    1,                          /* CODE  is read-only */
    1                           /* No fancy alignments supported. */
  },
  { NULL, NULL },
  { -1, 0, 0, 4, 0, 2, 0 },
  { -1, FALSE },
  { z80_emitDebuggerSymbol },
  {
    256,                        /* maxCount */
    3,                          /* sizeofElement */
    {6, 7, 8},                  /* sizeofMatchJump[] - Assumes operand allocated to registers */
    {6, 9, 15},                 /* sizeofRangeCompare[] - Assumes operand allocated to registers*/
    1,                          /* sizeofSubtract - Assumes use of a singel inc or dec */
    9,                          /* sizeofDispatch - Assumes operand allocated to register e or c*/
  },
  "_",
  _ez80_z80_init,
  _parseOptions,
  _z80_options,
  NULL,
  _finaliseOptions,
  _setDefaultOptions,
  z80_assignRegisters,
  _getRegName,
  _getRegByName,
  NULL,
  _keywords,
  0,                            /* no assembler preamble */
  NULL,                         /* no genAssemblerEnd */
  0,                            /* no local IVT generation code */
  0,                            /* no genXINIT code */
  NULL,                         /* genInitStartup */
  _reset_regparm,
  _reg_parm,
  _process_pragma,
  NULL,
  _hasNativeMulFor,
  hasExtBitOp,                  /* hasExtBitOp */
  oclsExpense,                  /* oclsExpense */
  TRUE,
  TRUE,                         /* little endian */
  0,                            /* leave lt */
  0,                            /* leave gt */
  1,                            /* transform <= to ! > */
  1,                            /* transform >= to ! < */
  1,                            /* transform != to !(a == b) */
  0,                            /* leave == */
  FALSE,                        /* Array initializer support. */
  0,                            /* no CSE cost estimation yet */
  _z80_builtins,                /* builtin functions */
  GPOINTER,                     /* treat unqualified pointers as "generic" pointers */
  1,                            /* reset labelKey to 1 */
  1,                            /* globals & local statics allowed */
  9,                            /* Number of registers handled in the tree-decomposition-based register allocator in SDCCralloc.hpp */
  PORT_MAGIC
};


#include "common.h"
#include "SDCCicode.h"
#include "SDCCglobl.h"
#include "SDCCgen.h"

#include "peep.h"
#include "gen.h"

#define NOTUSEDERROR() do {werror(E_INTERNAL_ERROR, __FILE__, __LINE__, "error in notUsed()");} while(0)

// #define D(_s) { printf _s; fflush(stdout); }
#define D(_s)

#define ISINST(l, i) (!STRNCASECMP((l), (i), sizeof(i) - 1) && (!(l)[sizeof(i) - 1] || isspace((unsigned char)((l)[sizeof(i) - 1]))))
#define STARTSINST(l, i) (!STRNCASECMP((l), (i), sizeof(i) - 1))

typedef enum
{
  S4O_CONDJMP,
  S4O_WR_OP,
  S4O_RD_OP,
  S4O_TERM,
  S4O_VISITED,
  S4O_ABORT,
  S4O_CONTINUE
} S4O_RET;

static struct
{
  lineNode *head;
} _G;

//extern bool stm8_regs_used_as_parms_in_calls_from_current_function[YH_IDX + 1];
//extern bool stm8_regs_used_as_parms_in_pcalls_from_current_function[YH_IDX + 1];

/*-----------------------------------------------------------------*/
/* incLabelJmpToCount - increment counter "jmpToCount" in entry    */
/* of the list labelHash                                           */
/*-----------------------------------------------------------------*/
static bool
incLabelJmpToCount (const char *label)
{
  labelHashEntry *entry;

  entry = getLabelRef (label, _G.head);
  if (!entry)
    return FALSE;
  entry->jmpToCount++;
  return TRUE;
}

/*-----------------------------------------------------------------*/
/* findLabel -                                                     */
/* 1. extracts label in the opcode pl                              */
/* 2. increment "label jump-to count" in labelHash                 */
/* 3. search lineNode with label definition and return it          */
/*-----------------------------------------------------------------*/
static lineNode *
findLabel (const lineNode *pl)
{
  char *p;
  lineNode *cpl;

  /* 1. extract label in opcode */

  /* In each jump the label is at the end */
  p = strlen (pl->line) - 1 + pl->line;

  /* Skip trailing whitespace */
  while(isspace(*p))
    p--;

  /* scan backward until space or ',' */
  for (; p > pl->line; p--)
    if (isspace(*p) || *p == ',' || *p == '#')
      break;

  /* sanity check */
  if (p == pl->line)
    {
      NOTUSEDERROR();
      return NULL;
    }

  /* skip ',' resp. '\t' */
  ++p;

  /* 2. increment "label jump-to count" */
  if (!incLabelJmpToCount (p))
    return NULL;

  /* 3. search lineNode with label definition and return it */
  for (cpl = _G.head; cpl; cpl = cpl->next)
    if (cpl->isLabel &&
      strncmp (p, cpl->line, strlen(p)) == 0 &&
      cpl->line[strlen(p)] == ':')
        return cpl;

  return NULL;
}

static const char *leftArg (const char *arg)
{
  while (isspace (*arg))
    arg++;
  while (*arg && !isspace (*arg))
    arg++;
  while (isspace (*arg))
    arg++;
  return (arg);
}

static const char *rightArg (const char *arg)
{
  arg = leftArg (arg);
  unsigned int parens = 0;
  for(; *arg; arg++)
    {
      if (*arg == '(')
        parens++;
      if (*arg == ')')
        parens--;
      if (isspace (*arg) && !parens)
        break;
    }
  while (isspace (*arg))
    arg++;
  return (arg);
}

static bool isReg8(const char *arg)
{
  return (!strncmp (arg, "xl", 2) || !strncmp (arg, "xh", 2) ||
    !strncmp (arg, "yl", 2) || !strncmp (arg, "yh", 2) ||
    !strncmp (arg, "zl", 2) || !strncmp (arg, "zh", 2));
}

static bool isReg16(const char *arg)
{
  return ((arg[0] == 'x' || arg[0] == 'y' || arg[0] == 'z') && !isalnum(arg[1]));
}

static bool isSprel(const char *arg)
{
  if (*arg != '(')
    return false;
  arg++;
  for(;; arg++)
    {
      if (!*arg)
        return false;
      if (*arg == ',')
        break;
    }
  return (!strncmp (arg, ", sp)", 5));
}

static bool isYrel(const char *arg)
{
  if (*arg != '(')
    return false;
  arg++;
  for(;; arg++)
    {
      if (!*arg)
        return false;
      if (*arg == ',')
        break;
    }
  return (!strncmp (arg, ", y)", 4));
}

static bool
isInt(const char *str)
{
  int ret;
  while(str[0] == '#' || str[0] == '(' || str[0] == '[' || isspace ((unsigned char)str[0]))
    str++;
  if(sscanf(str, "0x%x", &ret))
    return(ret);
  if(!sscanf(str, "%d", &ret))
    return(false);
  return(true);
}

static int
readint(const char *str)
{
  int ret;
  while(str[0] == '#' || str[0] == '(' || str[0] == '[' || isspace ((unsigned char)str[0]))
    str++;
  if(sscanf(str, "0x%x", &ret))
    return(ret);
  if(!sscanf(str, "%d", &ret))
    {
      wassertl (0, "readint() got non-integer argument:");
      fprintf (stderr, "%s\n", str);
      ret = -1;
    }
  return(ret);
}

int
f8instructionSize (lineNode *pl)
{
  const char *larg = leftArg (pl->line);
  const char *rarg = rightArg (pl->line);

  if (ISINST (pl->line, "adc") || ISINST (pl->line, "add") || ISINST (pl->line, "and") ||  ISINST (pl->line, "cp") ||
    ISINST (pl->line, "or") || ISINST (pl->line, "sbc") || ISINST (pl->line, "sub") || ISINST (pl->line, "xor"))
    {
      if (!strncmp (larg, "xl", 2) && isReg8 (rarg))
        return 1;
      else if (isReg8 (larg) && !strncmp (rarg, "xl", 2))
        return 2;
      else if (!strncmp (larg, "xl", 2) && isSprel (rarg))
        return 2;
      else if (isReg8 (larg) && isSprel (rarg))
        return 3;
      else if (!strncmp (larg, "xl", 2) && rarg[0] == '#')
        return 2;
      else if (isReg8 (larg) && rarg[0] == '#')
        return 3;
      else if (!strncmp (larg, "xl", 2) )
        return 3;
      else
        return 4;
    }

  if (ISINST (pl->line, "bool") || ISINST (pl->line, "clr") || ISINST (pl->line, "daa") || ISINST (pl->line, "dec") ||
    ISINST (pl->line, "inc") ||  ISINST (pl->line, "pop") ||  ISINST (pl->line, "push") ||  ISINST (pl->line, "rlc") ||
    ISINST (pl->line, "rrc") || ISINST (pl->line, "sll") || ISINST (pl->line, "sra") || ISINST (pl->line, "srl") ||
    ISINST (pl->line, "thrd") || ISINST (pl->line, "tst"))
    {
      if (!strncmp (larg, "xl", 2))
        return 1;
      else if (isReg8 (larg))
        return 2;
      else if (larg[0] == '#' || isSprel (larg))
        return 2;
      else
        return 3;
    }

  if ((ISINST (pl->line, "adcw") || ISINST (pl->line, "sbcw") && !rarg[0]) ||
    ISINST (pl->line, "boolw") || ISINST (pl->line, "clrw") || ISINST (pl->line, "decw") || ISINST (pl->line, "incw") ||
    ISINST (pl->line, "incnw") || ISINST (pl->line, "mul") || ISINST (pl->line, "negw") || ISINST (pl->line, "popw") ||
    ISINST (pl->line, "pushw") || ISINST (pl->line, "sllw") || ISINST (pl->line, "sraw") || ISINST (pl->line, "srlw") ||
    ISINST (pl->line, "rlcw") || ISINST (pl->line, "rrcw") || ISINST (pl->line, "tstw"))
    {
      if (larg[0] == 'y')
        return 1;
      else if (isReg16 (larg))
        return 2;
      else if (isSprel (larg))
        return 2;
      else
        return 3;
    }

  if (ISINST (pl->line, "xch") && larg[0] == 'f' && isSprel (rarg))
    return 1;
  else if (ISINST (pl->line, "xch"))
    return 1 + (larg[0] != 'y');
  
  if (ISINST (pl->line, "addw") && !strncmp (larg, "sp", 2) && rarg[0] == '#')
    return 2;

  if (ISINST (pl->line, "addw") && rarg[0] == '#' && isInt (rarg) && readint (rarg) <= 0xff)
    {
      if (larg[0] == 'y')
        return 2;
      else
        return 3;
    }

  if (ISINST (pl->line, "addw") || ISINST (pl->line, "cpw") || ISINST (pl->line, "orw") || ISINST (pl->line, "sbcw") ||
    ISINST (pl->line, "subw") || ISINST (pl->line, "xorw"))
    {
      if (larg[0] == 'y' && rarg[0] == 'x')
        return 1;
      else if (isReg16 (larg) && isReg16 (rarg))
        return 2;
      else if (larg[0] == 'y' && isSprel (rarg))
        return 2;
      else if (isReg16 (larg) && isSprel (rarg))
        return 3;
      else if (larg[0] == 'y')
        return 3;
      else
        return 4;
    }

  if (ISINST (pl->line, "ld"))
    {
      if (!strncmp (larg, "xl", 2) && isReg8 (rarg))
        return 1;
      else if (isReg8 (larg) && !strncmp (rarg, "xl", 2))
        return 2;
      else if (!strncmp (larg, "xl", 2) && (isSprel (rarg) || isYrel (rarg)) ||
        (isSprel (larg) || isYrel (larg)) && !strncmp (rarg, "xl", 2))
        return 2;
      else if (isReg8 (larg) && (isSprel (rarg) || isYrel (rarg)) ||
        (isSprel (larg) || isYrel (larg)) && isReg8 (rarg))
        return 3;
      else if (!strncmp (larg, "xl", 2) && rarg[0] == '#')
        return 2;
      else if (!strncmp (larg, "xl", 2) && !strncmp (rarg, "(y)", 3))
        return 1;
      else if (isReg8 (larg) && !strncmp (rarg, "(y)", 3))
        return 2;
      else if (!strncmp (larg, "(y)", 3) && !strncmp (rarg, "xl", 2))
        return 1;
      else if (!strncmp (larg, "(y)", 3) && isReg8 (rarg))
        return 2;
      else if (isReg8 (larg) && isReg8 (rarg))
        return 2;
      else if (!strncmp (larg, "xl", 2) || !strncmp (rarg, "xl", 2))
        return 3;
      else
        return 4;
    }

  if ((ISINST (pl->line, "ldi") || ISINST (pl->line, "ldwi")) && isYrel (larg))
    return 2;

  if (ISINST (pl->line, "ldw"))
    {
      if (larg[0] == 'y' && rarg[0] == 'x')
        return 1;
      else if (isReg16 (larg) && rarg[0] == 'y')
        return 1;
      else if (larg[0] == 'y' && rarg[0] == 'z')
        return 2;
      else if (larg[0] == 'y' && (isSprel (rarg) || isYrel (rarg)) ||
        isSprel (larg) && rarg[0] == 'y')
        return 2;
      else if (isReg16 (larg) && isSprel (rarg) ||
        isSprel (larg) && isReg16 (rarg))
        return 3;
      else if (larg[0] == 'y' && !strncmp (rarg, "(y)", 3))
        return 1;
      else if (!strncmp (larg, "(y)", 3) && rarg[0] == 'x')
        return 1;
      else if (isYrel (larg) && rarg[0] == 'x')
        return 2;
      else if (larg[0] == 'y' && rarg[0] == '#' && isInt (rarg) && readint (rarg) <= 0xff)
        return 2;
      else if (isReg16 (larg) && rarg[0] == '#' && isInt (rarg) && readint (rarg) <= 0xff)
        return 3;
      else if (larg[0] == 'y' || rarg[0] == 'y')
        return 3;
      else
        return 4;
    }

  if (ISINST (pl->line, "xchb"))
    {
      if (!strncmp (larg, "xl", 2))
        return 3;
      else
        return 4;
    }

  if (ISINST (pl->line, "rot"))
    {
      if (!strncmp (larg, "xl", 2))
        return 2;
      else
        return 3;
    }

  if (ISINST (pl->line, "zex") || ISINST (pl->line, "sex"))
    {
      if (!strncmp (larg, "y", 1) && !strncmp (rarg, "xl", 2))
        return 1;
      else
        return 2;
    }

  if (ISINST (pl->line, "mad") || ISINST (pl->line, "nop"))
    return 1;

  if (ISINST (pl->line, "call") || ISINST (pl->line, "jp"))
    {
      if (larg[0] == 'y')
        return 1;
      else if (isReg16 (larg))
        return 2;
      else
        return 3;
    }
  else if (STARTSINST (pl->line, "jr"))
    {
      if (ISINST (pl->line, "jro") || ISINST (pl->line, "jrsgt") || ISINST (pl->line, "jrgt"))
        return 3;
      else
        return 2;
    }
  else if (ISINST (pl->line, "dnjnz"))
    {
      if (!strncmp (larg, "yh", 2))
        return 2;
      else
        return 3;
    }
  else if (ISINST (pl->line, "ret") || ISINST (pl->line, "reti"))
    return 1;

  // If the instruction is unrecognized, we shouldn't try to optimize.
  // For all we know it might be some .ds or similar possibly long line
  // Return a large value to discourage optimization.
  if (pl->ic)
    werrorfl(pl->ic->filename, pl->ic->lineno, W_UNRECOGNIZED_ASM, __func__, 999, pl->line);
  else
    werrorfl("unknown", 0, W_UNRECOGNIZED_ASM, __func__, 999, pl->line);
  return(999);
}

static bool
f8MightReadFlag (const lineNode *pl, const char *what)
{
  if (ISINST (pl->line, "adc") || ISINST (pl->line, "sbc"))
    return !strcmp (what, "cf");
  if (ISINST (pl->line, "adcw") || ISINST (pl->line, "sbcw"))
    return !strcmp (what, "cf");
  if (ISINST (pl->line, "add") || ISINST (pl->line, "and") || ISINST (pl->line, "cp") || ISINST (pl->line, "or") || ISINST (pl->line, "sub") || ISINST (pl->line, "xor"))
    return false;
  if (ISINST (pl->line, "sll") || ISINST (pl->line, "srl") || ISINST (pl->line, "inc") || ISINST (pl->line, "dec") || ISINST (pl->line, "clr") || ISINST (pl->line, "push") || ISINST (pl->line, "tst"))
    return false;
  if (ISINST (pl->line, "rlc") || ISINST (pl->line, "rrc"))
    return !strcmp (what, "cf");
  if (ISINST (pl->line, "addw") || ISINST (pl->line, "orw") || ISINST (pl->line, "subw"))
    return false;
  if (ISINST (pl->line, "adcw") || ISINST (pl->line, "rlcw") || ISINST (pl->line, "rrcw") || ISINST (pl->line, "sbcw"))
    return !strcmp (what, "cf");
  if (ISINST (pl->line, "clrw") || ISINST (pl->line, "incw") || ISINST (pl->line, "pushw") || ISINST (pl->line, "sraw") || ISINST (pl->line, "srlw") || ISINST (pl->line, "sllw") || ISINST (pl->line, "tstw"))
    return false;
  if (ISINST (pl->line, "ld") || ISINST (pl->line, "ldw") || ISINST (pl->line, "ldi") || ISINST (pl->line, "ldwi"))
    return false;
  if (ISINST (pl->line, "bool") || ISINST (pl->line, "cax") || ISINST (pl->line, "mad") || ISINST (pl->line, "msk") || ISINST (pl->line, "pop") || ISINST (pl->line, "rot") || ISINST (pl->line, "sra") || ISINST (pl->line, "thrd"))
    return false;
  if (ISINST (pl->line, "daa"))
    return (!strcmp (what, "cf") || !strcmp (what, "hf"));
  if (ISINST (pl->line, "xch"))
    return (leftArg (pl->line)[0] == 'f');
  if (ISINST (pl->line, "boolw") || ISINST (pl->line, "caxw") || ISINST (pl->line, "cpw") || ISINST (pl->line, "decw") || ISINST (pl->line, "incnw") || ISINST (pl->line, "mul") || ISINST (pl->line, "negw") || ISINST (pl->line, "popw") || ISINST (pl->line, "sex") || ISINST (pl->line, "xchw") || ISINST (pl->line, "zex"))
    return false;
  if (ISINST (pl->line, "xchb"))
    return false;

  if (ISINST (pl->line, "call") || ISINST (pl->line, "dnjnz") || ISINST (pl->line, "jr") || ISINST (pl->line, "jp"))
    return false;
  if (ISINST (pl->line, "jrc") || ISINST (pl->line, "jrnc"))
    return !strcmp (what, "cf");
  if (ISINST (pl->line, "jrn") || ISINST (pl->line, "jrnn"))
    return !strcmp (what, "nf");
  if (ISINST (pl->line, "jrz") || ISINST (pl->line, "jrnz"))
    return !strcmp (what, "zf");
  if (ISINST (pl->line, "jrno") || ISINST (pl->line, "jro"))
    return !strcmp (what, "of");
  if (ISINST (pl->line, "jrsle") || ISINST (pl->line, "jrsgt"))
    return !strcmp (what, "zf") || !strcmp (what, "nf") || !strcmp (what, "of");
  if (ISINST (pl->line, "jrle") || ISINST (pl->line, "jrgt"))
    return !strcmp (what, "cf") || !strcmp (what, "zf");

  if (ISINST (pl->line, "ret"))
    return false;
  if (ISINST (pl->line, "reti"))
    return true;
  if (ISINST (pl->line, "nop"))
    return false;

  if (pl->ic)
    werrorfl(pl->ic->filename, pl->ic->lineno, W_UNRECOGNIZED_ASM, __func__, 999, pl->line);
  else
    werrorfl("unknown", 0, W_UNRECOGNIZED_ASM, __func__, 999, pl->line);

  // Fail-safe fallback.
  return true;
}

/* Check if reading arg implies reading what. */
static bool argCont(const char *arg, char what)
{
  if (!arg || strlen (arg) == 0 || !(what == 'x' || what == 'y' || what == 'z'))
    return false;

  while (isblank ((unsigned char)(arg[0])))
    arg++;

  if (arg[0] == ',')
    arg++;

  while (isblank ((unsigned char)(arg[0])))
    arg++;

  if (arg[0] == '#')
    return false;

  if (arg[0] == '(') 
    arg++;
  if (arg[0] == '0' && (tolower(arg[1])) == 'x') 
    arg += 2; // Skip hex prefix to avoid false x positive.

  if (strlen(arg) == 0)
    return false;

  return (strchr(arg, what));
}

static bool
f8MightRead (const lineNode *pl, const char *what)
{
  char extra = 0;

  if (!strcmp (what, "xl") || !strcmp (what, "xh"))
    extra = 'x';
  else if (!strcmp (what, "yl") || !strcmp (what, "yh"))
    extra = 'y';
  else if (!strcmp (what, "zl") || !strcmp (what, "zh"))
    extra = 'z';
  else
    return f8MightReadFlag(pl, what);

  if (ISINST (pl->line, "jp") || ISINST (pl->line, "jr"))
    return false;

  // 8-bit 2-op inst, and some others.
  if (ISINST (pl->line, "adc") || ISINST (pl->line, "add") || ISINST (pl->line, "and") || ISINST (pl->line, "cp") || ISINST (pl->line, "or") || ISINST (pl->line, "sbc") || ISINST (pl->line, "sub") || ISINST (pl->line, "xch") || ISINST (pl->line, "xor"))
    {
      const char *larg = leftArg (pl->line);
      const char *rarg = rightArg (pl->line);

      if (larg[0] == what[0] && larg[1] == what[1] || argCont (larg + 1, extra))
        return true;
      if (rarg && (rarg[0] == what[0] && rarg[1] == what[1] || argCont (rarg + 1, extra)))
        return true;
      return false;
    }
  // 8-bit 1-op inst, and some others
  if (ISINST (pl->line, "pop"))
    return false;
  if (ISINST (pl->line, "clr"))
    {
      const char *larg = leftArg (pl->line);
      return (larg[0] == '(' && argCont (larg, extra));
    }
  if (ISINST (pl->line, "bool") || ISINST (pl->line, "dec") || ISINST (pl->line, "inc") || ISINST (pl->line, "push") || ISINST (pl->line, "rlc") || ISINST (pl->line, "rot") || ISINST (pl->line, "rrc") || ISINST (pl->line, "sll") || ISINST (pl->line, "sra") || ISINST (pl->line, "srl") || ISINST (pl->line, "tst") || ISINST (pl->line, "xchb"))
    {
      const char *larg = leftArg (pl->line);

      return (larg[0] == what[0] && larg[1] == what[1] || argCont (larg + 1, extra));
    }
  // 16-bit 2/1-op inst, and some others.
  if (ISINST (pl->line, "clrw") || ISINST (pl->line, "popw"))
    return false;
  if (ISINST (pl->line, "adcw") || ISINST (pl->line, "addw") || ISINST (pl->line, "boolw") || ISINST (pl->line, "cpw") || ISINST (pl->line, "decw") || ISINST (pl->line, "incw") || ISINST (pl->line, "mul") || ISINST (pl->line, "negw") || ISINST (pl->line, "orw") || ISINST (pl->line, "pushw") || ISINST (pl->line, "rlcw") || ISINST (pl->line, "rrcw") || ISINST (pl->line, "sllw") || ISINST (pl->line, "sraw") || ISINST (pl->line, "srlw") || ISINST (pl->line, "subw") || ISINST (pl->line, "sbcw") || ISINST (pl->line, "tstw") || ISINST (pl->line, "incnw") || ISINST (pl->line, "xorw"))
    {
      const char *larg = leftArg (pl->line);
      const char *rarg = rightArg (pl->line);

      if (argCont (larg, extra))
        return true;
      if (rarg && argCont (rarg, extra))
        return true;
      return false;
    }
  // ld
  if (ISINST (pl->line, "ld"))
    {
      const char *larg = leftArg (pl->line);
      const char *rarg = rightArg (pl->line);

      if (rarg && (rarg[0] == what[0] && rarg[1] == what[1] || argCont (rarg + 1, extra)))
        return true;
      if (larg[0] == what[0] && larg[1] == what[1])
        return false;
      if (argCont (larg + 1, extra))
        return true;
      return false;
    }
  // ldw
  if (ISINST (pl->line, "ldi") || ISINST (pl->line, "ldwi"))
    return (extra == 'y' || extra == 'z');
  if (ISINST (pl->line, "ldw"))
    {
      const char *larg = leftArg (pl->line);
      const char *rarg = rightArg (pl->line);

      if (rarg && (rarg[0] == what[0] && rarg[1] == what[1] || argCont (rarg + 1, extra)))
        return true;
      if (larg[0] == what[0] && larg[1] == what[1])
        return false;
      if (argCont (larg + 1, extra))
        return true;
      return false;
    }
  if (ISINST (pl->line, "sex") || ISINST (pl->line, "zex"))
    {
      const char *rarg = rightArg (pl->line);
      if (rarg && (rarg[0] == what[0] && rarg[1] == what[1]))
        return true;
      return false;
    }
  if (ISINST (pl->line, "call"))
    {
      const char *larg = leftArg (pl->line);
      if (*larg == '#')
        larg++;
      if (*larg == '_')
        larg++;
      const symbol *f = findSym (SymbolTab, 0, larg);
      if (*larg == extra)
        return true;
      if (f && IS_FUNC (f->type))
        return f8IsParmInCall (f->type, what);
      else // Fallback needed for calls through function pointers and for calls to literal addresses.
        return true; // todo: improve accuracy here.
    }
  if (STARTSINST (pl->line, "jr"))
    return false;
  if (ISINST (pl->line, "ret"))
    return f8IsReturned(what);
  if (ISINST (pl->line, "reti"))
    return true;

  if (pl->ic)
    werrorfl(pl->ic->filename, pl->ic->lineno, W_UNRECOGNIZED_ASM, __func__, 999, pl->line);
  else
    werrorfl("unknown", 0, W_UNRECOGNIZED_ASM, __func__, 999, pl->line);

  // Fail-safe fallback.
  return true;
}

static bool
f8UncondJump (const lineNode *pl)
{
  return (ISINST (pl->line, "jp") || ISINST (pl->line, "jr"));
}

static bool
f8CondJump (const lineNode *pl)
{
  return (!f8UncondJump (pl) && STARTSINST (pl->line, "jr") ||
    ISINST (pl->line, "dnjz"));
}

static bool
f8SurelyWritesFlag (const lineNode *pl, const char *what)
{
  // 8-bit 2-op inst.
  if (ISINST (pl->line, "adc") || ISINST (pl->line, "add") || ISINST (pl->line, "cp") || ISINST (pl->line, "sbc") || ISINST (pl->line, "sub"))
    return true;
  if (ISINST (pl->line, "or") || ISINST (pl->line, "and") || ISINST (pl->line, "xor"))
    return (!strcmp (what, "zf") || !strcmp (what, "nf"));
  // 8-bit 1-op inst.
  if (ISINST (pl->line, "dec") || ISINST (pl->line, "inc"))
    return true;
  if (ISINST (pl->line, "clr") || ISINST (pl->line, "push"))
    return false;
  if (ISINST (pl->line, "srl") || ISINST (pl->line, "sll") || ISINST (pl->line, "rrc") || ISINST (pl->line, "rlc"))
    return (!strcmp (what, "zf") || !strcmp (what, "cf"));
  if (ISINST (pl->line, "tst"))
    return strcmp (what, "hf");
  // 16-bit 1/2-op inst.
  if (ISINST (pl->line, "adcw") || ISINST (pl->line, "cpw") || ISINST (pl->line, "decw") || ISINST (pl->line, "incw") || ISINST (pl->line, "negw") || ISINST (pl->line, "sbcw") || ISINST (pl->line, "subw"))
    return strcmp (what, "hf");
  if (ISINST (pl->line, "addw"))
    {
      const char *arg = leftArg (pl->line);
      if (!strncmp (arg, "sp", 2))
        return false;
      else
        return strcmp (what, "hf");
    }
  if (ISINST (pl->line, "clrw") || ISINST (pl->line, "pushw"))
    return false;
  if (ISINST (pl->line, "orw"))
    return (!strcmp (what, "of") || !strcmp (what, "zf") || !strcmp (what, "nf"));
  if (ISINST (pl->line, "tstw"))
    return strcmp (what, "hf");
  // ld / ldw
  if (ISINST (pl->line, "ld") || ISINST (pl->line, "ldw"))
    {
      const char *rarg = rightArg (pl->line);
      const char *rest = strstr (rarg, ", ");
      if (!rest || strchr (rarg, '#') && strchr (rarg, '#') < rest || strchr (rarg, ';') && strchr (rarg, ';') < rest)
        return false; // todo: ld xl, mm / ldw y, mm
      if (!strncmp (rest, ", y)", 2) || !strncmp (rest, ", z)", 2) || !strncmp (rest, ", sp)", 3))
        return (!strcmp (what, "zf") || !strcmp (what, "nf"));
      return false;
    }
  if (ISINST (pl->line, "ldi") || ISINST (pl->line, "ldwi"))
    return (!strcmp (what, "zf") || !strcmp (what, "nf"));
  // 8-bit 0-op inst.
  if (ISINST (pl->line, "bool") || ISINST (pl->line, "cax"))
    return !strcmp (what, "zf");
  if (ISINST (pl->line, "daa") || ISINST (pl->line, "sra"))
    return (!strcmp (what, "zf") || !strcmp (what, "cf"));
  if (ISINST (pl->line, "mad"))
    return (!strcmp (what, "zf") || !strcmp (what, "nf"));
  if (ISINST (pl->line, "msk") || ISINST (pl->line, "pop"))
    return false;
  if (ISINST (pl->line, "rot"))
    return false;
  if (ISINST (pl->line, "xch"))
    return leftArg (pl->line)[0] == 'f';
  // 16-bit 0-op inst.
  if (ISINST (pl->line, "boolw") || ISINST (pl->line, "zex"))
    return !strcmp (what, "zf");
  if (ISINST (pl->line, "incnw") || ISINST (pl->line, "popw") || ISINST (pl->line, "xchw"))
    return false;
  if (ISINST (pl->line, "mul") || ISINST (pl->line, "sraw") || ISINST (pl->line, "srlw") || ISINST (pl->line, "rlcw") || ISINST (pl->line, "rrcw"))
    return (!strcmp (what, "zf") || !strcmp (what, "nf") || !strcmp (what, "cf"));
  if (ISINST (pl->line, "sex"))
    return (!strcmp (what, "zf") || !strcmp (what, "nf"));
  if (ISINST (pl->line, "sllw"))
    {
      if (!strcmp (what, "zf") || !strcmp (what, "nf"))
        return true;
      if (!rightArg (pl->line) && !strcmp (what, "cf"))
        return true;
      return false;
    }
  if (ISINST (pl->line, "xchb"))
    return !strcmp (what, "zf");
  // jumps
  if (STARTSINST (pl->line, "jr"))
    return false;
  if (ISINST (pl->line, "jp")) // todo: improve accuracy by checking for function call vs. local jump.
    return false;
  if (ISINST (pl->line, "call"))
    return true;
  if (ISINST (pl->line, "ret"))
    return true;
  if (ISINST (pl->line, "reti"))
    return false;
  if (ISINST (pl->line, "nop"))
    return false;

  if (pl->ic)
    werrorfl(pl->ic->filename, pl->ic->lineno, W_UNRECOGNIZED_ASM, __func__, 999, pl->line);
  else
    werrorfl("unknown", 0, W_UNRECOGNIZED_ASM, __func__, 999, pl->line);

  // Fail-safe fallback.
  return false;
}

static bool
f8SurelyWrites (const lineNode *pl, const char *what)
{
  char extra = 0;

  if (!strcmp (what, "xl") || !strcmp (what, "xh"))
    extra = 'x';
  else if (!strcmp (what, "yl") || !strcmp (what, "yh"))
    extra = 'y';
  else if (!strcmp (what, "zl") || !strcmp (what, "zh"))
    extra = 'z';
  else
    return (f8SurelyWritesFlag (pl, what));

  // 8-bit 1/2-op inst, and some others.
  if (ISINST (pl->line, "push") || ISINST (pl->line, "tst"))
     return false;
  if (ISINST (pl->line, "adc") || ISINST (pl->line, "add") || ISINST (pl->line, "and") || ISINST (pl->line, "bool") || ISINST (pl->line, "clr") || ISINST (pl->line, "dec") || ISINST (pl->line, "cp") || ISINST (pl->line, "inc") || ISINST (pl->line, "or") || ISINST (pl->line, "pop") || ISINST (pl->line, "rlc") || ISINST (pl->line, "rot") || ISINST (pl->line, "rrc") || ISINST (pl->line, "sbc") || ISINST (pl->line, "sub") || ISINST (pl->line, "sll") || ISINST (pl->line, "sra") || ISINST (pl->line, "srl") || ISINST (pl->line, "xor"))
    {
      const char *larg = leftArg (pl->line);
      return (larg[0] == what[0] && larg[1] == what[1]);
    }
  // 16-bit 2/1-op inst, and some others.
  if (ISINST (pl->line, "pushw") || ISINST (pl->line, "tstw"))
    return false;
  if (ISINST (pl->line, "adcw") || ISINST (pl->line, "addw") || ISINST (pl->line, "boolw") || ISINST (pl->line, "clrw") || ISINST (pl->line, "cpw") || ISINST (pl->line, "decw") || ISINST (pl->line, "incw") || ISINST (pl->line, "mul") || ISINST (pl->line, "negw") || ISINST (pl->line, "orw") || ISINST (pl->line, "popw") || ISINST (pl->line, "rlcw") || ISINST (pl->line, "rrcw") || ISINST (pl->line, "sex") || ISINST (pl->line, "sllw") || ISINST (pl->line, "sraw") || ISINST (pl->line, "srlw") || ISINST (pl->line, "subw") || ISINST (pl->line, "sbcw") || ISINST (pl->line, "incnw") || ISINST (pl->line, "xorw") || ISINST (pl->line, "zex"))
    {
      const char *larg = leftArg (pl->line);
      return (larg[0] == extra);
    }
  if (ISINST (pl->line, "ld"))
    return (pl->line[3] == what[0] && pl->line[4] == what[1]);
  if (ISINST (pl->line, "ldw"))
    return (pl->line[4] == extra);
  if (ISINST (pl->line, "ldi") || ISINST (pl->line, "ldwi"))
    return false;
  if (ISINST (pl->line, "xch"))
    {
      const char *larg = leftArg (pl->line);
      const char *rarg = rightArg (pl->line);
      return (larg[0] == what[0] && larg[1] == what[1] || rarg[0] == what[0] && rarg[1] == what[1]);
    }
  if (STARTSINST (pl->line, "jr"))
    return false;
  if (ISINST (pl->line, "jp") || ISINST (pl->line, "call"))
    return false; // Todo: Improve accuracy?
  if (ISINST (pl->line, "ret") || ISINST (pl->line, "reti"))
    return true;

  if (pl->ic)
    werrorfl(pl->ic->filename, pl->ic->lineno, W_UNRECOGNIZED_ASM, __func__, 999, pl->line);
  else
    werrorfl("unknown", 0, W_UNRECOGNIZED_ASM, __func__, 999, pl->line);

  // Fail-safe fallback.
  return false;
}

static bool
f8SurelyReturns (const lineNode *pl)
{
  return (false);
}

/*-----------------------------------------------------------------*/
/* scan4op - "executes" and examines the assembler opcodes,        */
/* follows conditional and un-conditional jumps.                   */
/* Moreover it registers all passed labels.                        */
/*                                                                 */
/* Parameter:                                                      */
/*    lineNode **pl                                                */
/*       scanning starts from pl;                                  */
/*       pl also returns the last scanned line                     */
/*    const char *pReg                                             */
/*       points to a register (e.g. "ar0"). scan4op() tests for    */
/*       read or write operations with this register               */
/*    const char *untilOp                                          */
/*       points to NULL or a opcode (e.g. "push").                 */
/*       scan4op() returns if it hits this opcode.                 */
/*    lineNode **plCond                                            */
/*       If a conditional branch is met plCond points to the       */
/*       lineNode of the conditional branch                        */
/*                                                                 */
/* Returns:                                                        */
/*    S4O_ABORT                                                    */
/*       on error                                                  */
/*    S4O_VISITED                                                  */
/*       hit lineNode with "visited" flag set: scan4op() already   */
/*       scanned this opcode.                                      */
/*    S4O_FOUNDOPCODE                                              */
/*       found opcode and operand, to which untilOp and pReg are   */
/*       pointing to.                                              */
/*    S4O_RD_OP, S4O_WR_OP                                         */
/*       hit an opcode reading or writing from pReg                */
/*    S4O_CONDJMP                                                  */
/*       hit a conditional jump opcode. pl and plCond return the   */
/*       two possible branches.                                    */
/*    S4O_TERM                                                     */
/*       acall, lcall, ret and reti "terminate" a scan.            */
/*-----------------------------------------------------------------*/
static S4O_RET
scan4op (lineNode **pl, const char *what, const char *untilOp,
         lineNode **plCond)
{
  for (; *pl; *pl = (*pl)->next)
    {
      if (!(*pl)->line || (*pl)->isDebug || (*pl)->isComment || (*pl)->isLabel)
        continue;
      D(("Scanning %s for %s\n", (*pl)->line, what));
      /* don't optimize across inline assembler,
         e.g. isLabel doesn't work there */
      if ((*pl)->isInline)
        {
          D(("S4O_ABORT at inline asm\n"));
          return S4O_ABORT;
        }

      if ((*pl)->visited)
        {
          D(("S4O_VISITED\n"));
          return S4O_VISITED;
        }

      (*pl)->visited = TRUE;

      if(f8MightRead(*pl, what))
        {
          D(("S4O_RD_OP\n"));
          return S4O_RD_OP;
        }

      // Check writes before conditional jumps, some jumps (btjf, btjt) write 'c'
      if(f8SurelyWrites(*pl, what))
        {
          D(("S4O_WR_OP\n"));
          return S4O_WR_OP;
        }

      if(f8UncondJump(*pl))
        {
          *pl = findLabel (*pl);
            if (!*pl)
              {
                D(("S4O_ABORT at unconditional jump\n"));
                return S4O_ABORT;
              }
        }
      if(f8CondJump(*pl))
        {
          *plCond = findLabel (*pl);
          if (!*plCond)
            {
              D(("S4O_ABORT at conditional jump\n"));
              return S4O_ABORT;
            }
          D(("S4O_CONDJMP\n"));
          return S4O_CONDJMP;
        }

      if(f8SurelyReturns(*pl))
        {
          D(("S4O_TERM\n"));
          return S4O_TERM;
        }
    }
  D(("S4O_ABORT\n"));
  return S4O_ABORT;
}

/*-----------------------------------------------------------------*/
/* doTermScan - scan through area 2. This small wrapper handles:   */
/* - action required on different return values                    */
/* - recursion in case of conditional branches                     */
/*-----------------------------------------------------------------*/
static bool
doTermScan (lineNode **pl, const char *what)
{
  lineNode *plConditional;
  for (;; *pl = (*pl)->next)
    {
      switch (scan4op (pl, what, NULL, &plConditional))
        {
          case S4O_TERM:
          case S4O_VISITED:
          case S4O_WR_OP:
            /* all these are terminating conditions */
            return true;
          case S4O_CONDJMP:
            /* two possible destinations: recurse */
              {
                lineNode *pl2 = plConditional;
                D(("CONDJMP trying other branch first\n"));
                if (!doTermScan (&pl2, what))
                  return false;
                D(("Other branch OK.\n"));
              }
            continue;
          case S4O_RD_OP:
          default:
            /* no go */
            return false;
        }
    }
}

/*-----------------------------------------------------------------*/
/* univisitLines - clear "visited" flag in all lines               */
/*-----------------------------------------------------------------*/
static void
unvisitLines (lineNode *pl)
{
  for (; pl; pl = pl->next)
    pl->visited = false;
}

bool
f8notUsed (const char *what, lineNode *endPl, lineNode *head)
{
  lineNode *pl;
  if(!strcmp(what, "x"))
    return(f8notUsed("xl", endPl, head) && f8notUsed("xh", endPl, head));
  else if(!strcmp(what, "y"))
    return(f8notUsed("yl", endPl, head) && f8notUsed("yh", endPl, head));
  else if(!strcmp(what, "z"))
    return(f8notUsed("zl", endPl, head) && f8notUsed("zh", endPl, head));

  // Only handle general-purpose registers and documented flags.
  if (!strcmp(what, "xl") || !strcmp(what, "xh") || !strcmp(what, "yl") || !strcmp(what, "yh") || !strcmp(what, "zl") || !strcmp(what, "zh") ||
    !strcmp(what, "of") || !strcmp(what, "zf") || !strcmp(what, "nf") || !strcmp(what, "cf") || !strcmp(what, "hf"))
    ;
  else
    return false;

  _G.head = head;

  unvisitLines (_G.head);

  pl = endPl->next;
  return (doTermScan (&pl, what));
}

bool
f8notUsedFrom (const char *what, const char *label, lineNode *head)
{
  lineNode *cpl;

  for (cpl = head; cpl; cpl = cpl->next)
    if (cpl->isLabel && !strncmp (label, cpl->line, strlen(label)))
      return (f8notUsed (what, cpl, head));

  return false;
}

/* can be directly assigned with ld */
bool
f8canAssign (const char *op1, const char *op2, const char *exotic)
{
  return false;
}


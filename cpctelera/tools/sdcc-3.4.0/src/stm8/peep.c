#include "common.h"
#include "SDCCicode.h"
#include "SDCCglobl.h"
#include "SDCCgen.h"

#define NOTUSEDERROR() do {werror(E_INTERNAL_ERROR, __FILE__, __LINE__, "error in notUsed()");} while(0)

//#define D(_s) { printf _s; fflush(stdout); }
#define D(_s)

#define EQUALS(l, i) (!strcmp((l), (i)))
#define ISINST(l, i) (!strncmp((l), (i), sizeof(i) - 1))

#define ISINST(l, i) (!strncmp((l), (i), sizeof(i) - 1))

// This function should behave just like C99 isblank(). Remove it, once MSVC supports isblank().
static int _isblank(int c)
{
  return (c == ' ' || c == '\t');
}

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

static int
readint(const char *str)
{
  int ret;
  while(str[0] == '#' || str[0] == '(')
    str++;
  if(sscanf(str, "0x%x", &ret))
    return(ret);
  if(!sscanf(str, "%d", &ret))
    {
      wassertl (0, "readint() got non-integer argument.");
      ret = -1;
    }
  return(ret);
}

static int
isReg(const char *what)
{
  if(what[0] == '(')
    what++;
  if(what[0] == 'a' || what[0] == 'x' || what[0] == 'y')
    return(TRUE);
  if(!strcmp(what, "sp"))
    return(TRUE);
  return(FALSE);
}

static char *
nextToken(char *p)
{
  /* strtok replacement */
  static char *str, *ret, *end;
  // Use an internal buffer to prevent *p from being modified
  static char buf[128];
  if(p) {
    strncpy(buf, p, sizeof(buf));
    buf[sizeof(buf)-1] = '\0';
    str = buf;
    end = buf + strlen(p);
  }
  if(str >= end)
    return(NULL);
  ret = str;
  // Strip separators
  while(*str == ',' || isspace(*str))
    str++;
  if(*str == '(')
  {
    // Take an expression in brackets
    while(*str && *str != ')')
      str++;
    str++;
  } else {
    // Take until EOL or separator
    while(*str && *str != ',' && !isspace(*str))
      str++;
  }
  *str = '\0';
  str++;
  return(ret);
}

static int
isRelativeAddr(const char *what, const char *mode)
{
  char buf[4];
  strcpy(buf, mode);
  strcat(buf, ")");
  return(what[0] == '(' && strstr(what, buf));
}

static int
isLabel(const char *what)
{
  const char *end;	
  end = strchr(what, '+');
  if(!end)
    end = what + strlen(what);
  if(what[0] == '#')
    return (what[1] == '_' || what[1] == '<' || what[1] == '>');
  return(what[0] == '_' || *(end-1) == '$');
}

static int
isImmediate(const char *what)
{
  return(what[0] == '#');
}

static int
isShortoff(const char *what, const char *mode)
{
  return(isRelativeAddr(what, mode) && readint(what) <= 0xFF);
}

static int
isLongoff(const char *what, const char *mode)
{
  return(isRelativeAddr(what, mode) && readint(what) > 0xFF);
}

static int
isSpIndexed(const char *what)
{
  return isRelativeAddr(what, "sp");
}


int
stm8instructionSize(const lineNode *pl)
{
  char *operand;
  char *op1start;
  char *op2start;
  int i = 0;
  operand = nextToken(pl->line);
  op1start = nextToken(NULL);
  op2start = nextToken(NULL);

  while(op2start && isspace(*op2start)) op2start++;
  //printf("line=%s operand=%s op1start=%s op2start=%s\n", pl->line, operand, op1start, op2start);

  /* arity=1 */
  if(EQUALS(operand, "clr")
                || EQUALS(operand, "dec")
                || EQUALS(operand, "inc")
                || EQUALS(operand, "push")
                || EQUALS(operand, "swap")
                || EQUALS(operand, "jp")
                || EQUALS(operand, "cpl")
                || EQUALS(operand, "tnz"))
  {
    if(!op1start)
      return(3);
    if(!strcmp(op1start, "a") || !strcmp(op1start, "(x)"))
      return(1);
    if(!strcmp(op1start, "(y)"))
      return(2);
    if(op1start[0] == '(')
      op1start++;
    if(strstr(op1start, ",y)"))
      i++; // costs extra byte for operating with y
    if(isLabel(op1start))
      return(3);
    if(readint(op1start) <= 0xFF)
      return(2+i);
    /* op1 > 0xFF */
    if(EQUALS(operand, "jp") && !strchr(op1start, 'y'))
      return(3);
    return(4);
  }
  if(EQUALS(operand, "exg"))
  {
    if(isReg(op2start))
      return(1);
    else
      return(3);
  }
  if(EQUALS(operand, "addw") || EQUALS(operand, "subw"))
  {
    if(!strcmp(op1start, "sp"))
      return(2);
    if(isImmediate(op2start) && op1start[0] == 'y')
      return(4);
    if(isImmediate(op2start))
      return(3);
    if(isSpIndexed(op2start))
      return(3);
    if(isLongoff(op2start, "x"))
      return(4);
  }
  if(EQUALS(operand, "cplw"))
  {
    if(op1start[0] == 'y')
      return(2);
    else
      return(1);
  }
  if(EQUALS(operand, "ldf"))
  {
    if(isRelativeAddr(op1start, "y") || isRelativeAddr(op2start, "y"))
      return(5);
    else
      return(4);
  }
  /* Operations that costs 2 or 3 bytes for immediate */
  if(ISINST(operand, "ld")
                || EQUALS(operand, "cp")
                || EQUALS(operand, "cpw")
                || ISINST(operand, "adc")
                || EQUALS(operand, "add")
                || ISINST(operand, "and")
                || ISINST(operand, "bcp")
                || ISINST(operand, "call")
                || ISINST(operand, "callr")
                || ISINST(operand, "jr")
                || ISINST(operand, "or")
                || ISINST(operand, "sbc")
                || EQUALS(operand, "sub")
                || ISINST(operand, "xor"))
  {
    char suffix;

    if(!op1start || !op2start)
      return(4);
    suffix = operand[strlen(operand)-1];
    if(suffix == 'w' && isImmediate(op2start))
      i++; // costs extra byte
    if(isSpIndexed(op1start) || isSpIndexed(op2start))
      return(2);
    if(!strcmp(op1start, "(x)") || !strcmp(op2start, "(x)"))
      return(1);
    if(!strcmp(op1start, "(y)") || !strcmp(op2start, "(y)"))
      return(2);
    if(isLabel(op1start) || isLabel(op2start))
      return(3+i);
    if(isShortoff(op1start, "x") || isShortoff(op2start, "x"))
      return(2);
    if(isShortoff(op1start, "y") || isShortoff(op2start, "y"))
      return(3);
    if(isLongoff(op1start, "x") || isLongoff(op2start, "x"))
      return(3);
    if(isRelativeAddr(op1start, "y"))
      return(4);
    if(strchr(op1start, 'y') || (strchr(op2start, 'y') && !EQUALS(operand, "ldw")))
      i++; // costs extra byte for operating with y
    if(isReg(op1start) && isReg(op2start))
      return(1+i);
    if(op2start[0] == '#')
      return(2+i); // ld reg, #immd
    if(readint(op2start) <= 0xFF)
      return(2+i);
    return(3+i);
  }

  /* mov costs 3, 4 or 5 bytes depending on it's addressing mode */
  if(EQUALS(operand, "mov")) {
    if(op2start && op2start[0] == '#')
      return(4);
    if(op2start && isLabel(op2start))
      return(5);
    if(op2start && readint(op2start) <= 0xFF)
      return(3);
    if(op2start && readint(op2start) > 0xFF)
      return(5);
  }

  /* Operations that always costs 1 byte */
  if(EQUALS(operand, "ccf")
                || EQUALS(operand, "divw")
                || EQUALS(operand, "exgw")
                || EQUALS(operand, "iret")
                || EQUALS(operand, "nop")
                || EQUALS(operand, "rcf")
                || EQUALS(operand, "ret")
                || EQUALS(operand, "retf")
                || EQUALS(operand, "rvf")
                || EQUALS(operand, "scf"))
  {
    return(1);
  }
  /* Operations that costs 2 or 1 bytes depending on 
     is the Y or X register used */
  if(EQUALS(operand, "clrw")
                || EQUALS(operand, "decw")
                || EQUALS(operand, "div")
                || EQUALS(operand, "incw")
                || EQUALS(operand, "mul")
                || EQUALS(operand, "negw")
                || EQUALS(operand, "popw")
                || EQUALS(operand, "pushw")
                || EQUALS(operand, "rlcw")
                || EQUALS(operand, "rlwa")
                || EQUALS(operand, "rrcw")
                || EQUALS(operand, "rrwa")
                || EQUALS(operand, "sllw")
                || EQUALS(operand, "slaw")
                || EQUALS(operand, "sraw")
                || EQUALS(operand, "srlw")
                || EQUALS(operand, "swapw")
                || EQUALS(operand, "tnzw"))
  {
    if((op1start && !strcmp(op1start, "y")) || (op2start && !strcmp(op2start, "y")))
      return(2);
    else
      return(1);
  }
  return(5); // Maximum instruction size, e.g. btjt.
}

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

  /* In each mcs51 jumping opcode the label is at the end of the opcode */
  p = strlen (pl->line) - 1 + pl->line;

  /* scan backward until ',' or '\t' */
  for (; p > pl->line; p--)
    if (*p == ',' || *p == '\t')
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
    {
      if (   cpl->isLabel
          && strncmp (p, cpl->line, strlen(p)) == 0)
        {
          return cpl;
        }
    }
  return NULL;
}

/* Check if reading arg implies reading what. */
static bool argCont(const char *arg, const char *what)
{
  while (_isblank ((unsigned char)(arg[0])))
    arg++;

  if (arg[0] == '#')
    return FALSE;

  if (arg[0] == '(' && arg[1] == '0' && arg[2] == 'x')
    arg += 3; // Skip hex prefix to avoid false x positive.

  return (strstr(arg, what) != NULL);
}

/* Check if writing arg implies reading what. */
static bool argCont2(const char *arg, const char *what)
{
  while (_isblank ((unsigned char)(arg[0])))
    arg++;

  if (arg[0] != '(')
    return FALSE;
  if (arg[1] == '0' && arg[2] == 'x')
    arg += 2; // Skip hex prefix to avoid false x positive.
  return (strstr (arg + 1, what) && strstr (arg + 1, what) < strchr (arg + 1, ')'));
}

static bool
isReturned(const char *what)
{
  symbol *sym;
  sym_link *sym_lnk;
  int size;
  lineNode *l;

  l = _G.head;
  do
  {
    l = l->next;
  } while(l->isComment || l->ic == NULL || l->ic->op != FUNCTION);

  sym = OP_SYMBOL(IC_LEFT(l->ic));

  if(sym && IS_DECL(sym->type))
    {
      // Find size of return value.
      specifier *spec;
      if(sym->type->select.d.dcl_type != FUNCTION)
        NOTUSEDERROR();
      spec = &(sym->etype->select.s);
      if(spec->noun == V_VOID)
        size = 0;
      else if(spec->noun == V_CHAR || spec->noun == V_BOOL)
        size = 1;
      else if(spec->noun == V_INT && !(spec->b_long))
        size = 2;
      else
        size = 4;

      // Check for returned pointer.
      sym_lnk = sym->type;
      while (sym_lnk && !IS_PTR (sym_lnk))
        sym_lnk = sym_lnk->next;
      if(IS_PTR(sym_lnk))
        size = 2;
    }
  else
    {
      NOTUSEDERROR();
      return TRUE;
    }

  switch(*what)
    {
    case 'a':
      return(size == 1);
    case 'x':
      return(size > 1);
    case 'y':
      return(size > 2);
    default:
      return FALSE;
    }
}

static bool
z80MightRead(const lineNode *pl, const char *what)
{
  const char *extra = 0;
  if (!strcmp (what, "xl") || !strcmp (what, "xh"))
    extra = "x";
  if (!strcmp (what, "yl") || !strcmp (what, "yh"))
    extra = "y";

  if (ISINST (pl->line, "addw\t"))
    return (extra && extra[0] == pl->line[5]);

  if (ISINST (pl->line, "ld\t"))
    {
      if (argCont (strchr (pl->line, ',') + 2, what))
        return TRUE;
      if (extra && argCont (strchr (pl->line, ',') + 1, extra))
        return TRUE;
      if (extra && argCont2 (pl->line + 3, extra))
        return TRUE;
      return FALSE;
    }

  if (ISINST (pl->line, "ldw\t"))
    {
      if (extra && argCont (strchr (pl->line, ',') + 1, extra))
        return TRUE;
      if (extra && argCont2 (pl->line + 4, extra))
        return TRUE;
      return FALSE;
    }

  if(ISINST(pl->line, "ret"))
    return(isReturned(what));

  return TRUE;
}

static bool
z80UncondJump(const lineNode *pl)
{
  return (ISINST(pl->line, "jp\t") || ISINST(pl->line, "jra\t") || ISINST(pl->line, "jpf\t"));
}

static bool
z80CondJump(const lineNode *pl)
{
  return (!z80UncondJump(pl) && ISINST(pl->line, "jr") ||
    ISINST(pl->line, "btjt\t") || ISINST(pl->line, "btjf\t"));
}

static bool
z80SurelyWrites(const lineNode *pl, const char *what)
{
  char *extra = 0;
  if (!strcmp (what, "xl") || !strcmp (what, "xh"))
    extra = "x";
  if (!strcmp (what, "yl") || !strcmp (what, "yh"))
    extra = "y";

  if (ISINST (pl->line, "ld\t"))
    return (strncmp (pl->line + 3, what, strlen (what)) == 0);

  if (ISINST (pl->line, "ldw\t"))
    {
      return (extra && strncmp (pl->line + 4, extra, strlen (extra)) == 0);
    }

  if(ISINST(pl->line, "ret"))
    return TRUE;

  return FALSE;
}

static bool
z80SurelyReturns(const lineNode *pl)
{
  return(ISINST(pl->line, "ret"));
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
          D(("S4O_RD_OP: Inline asm\n"));
          return S4O_ABORT;
        }

      if ((*pl)->visited)
        {
          D(("S4O_VISITED\n"));
          return S4O_VISITED;
        }

      (*pl)->visited = TRUE;

      if(z80MightRead(*pl, what))
        {
          D(("S4O_RD_OP\n"));
          return S4O_RD_OP;
        }

      if(z80UncondJump(*pl))
        {
          *pl = findLabel (*pl);
            if (!*pl)
              {
                D(("S4O_ABORT\n"));
                return S4O_ABORT;
              }
        }
      if(z80CondJump(*pl))
        {
          *plCond = findLabel (*pl);
          if (!*plCond)
            {
              D(("S4O_ABORT\n"));
              return S4O_ABORT;
            }
          D(("S4O_CONDJMP\n"));
          return S4O_CONDJMP;
        }

      if(z80SurelyWrites(*pl, what))
        {
          D(("S4O_WR_OP\n"));
          return S4O_WR_OP;
        }

      /* Don't need to check for de, hl since z80MightRead() does that */
      if(z80SurelyReturns(*pl))
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
            return TRUE;
          case S4O_CONDJMP:
            /* two possible destinations: recurse */
              {
                lineNode *pl2 = plConditional;
                D(("CONDJMP trying other branch first\n"));
                if (!doTermScan (&pl2, what))
                  return FALSE;
                D(("Other branch OK.\n"));
              }
            continue;
          case S4O_RD_OP:
          default:
            /* no go */
            return FALSE;
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
    pl->visited = FALSE;
}

bool
stm8notUsed (const char *what, lineNode *endPl, lineNode *head)
{
  lineNode *pl;
  if(strcmp(what, "x") == 0)
    return(stm8notUsed("xl", endPl, head) && stm8notUsed("xh", endPl, head));
  else if(strcmp(what, "y") == 0)
    return(stm8notUsed("yl", endPl, head) && stm8notUsed("yh", endPl, head));

  if(strcmp(what, "a") && strcmp(what, "xl") && strcmp(what, "xh") && strcmp(what, "yl") && strcmp(what, "yh"))
    return FALSE;

  _G.head = head;

  unvisitLines (_G.head);

  pl = endPl->next;
  return (doTermScan (&pl, what));
}

bool
stm8notUsedFrom (const char *what, const char *label, lineNode *head)
{
  lineNode *cpl;

  for (cpl = _G.head; cpl; cpl = cpl->next)
    if (cpl->isLabel && !strncmp (label, cpl->line, strlen(label)))
      return (stm8notUsed (what, cpl, head));

  return FALSE;
}

/* can be directly assigned with ld */
bool
stm8canAssign (const char *op1, const char *op2, const char *exotic)
{
  //fprintf(stderr, "op1=%s op2=%s exotic=%s\n", op1, op2, exotic);
  const char *reg, *payload;
  reg = op1[0] == 'a' ? op1 : op2;
  payload = reg == op1 ? op2 : op1;
  if(isRelativeAddr(payload, "x")
                || isRelativeAddr(payload, "y")
                || isRelativeAddr(payload, "sp")
                || !strcmp(payload, "(x)")
                || !strcmp(payload, "(y)")
                || !strcmp(payload, "xl")
                || !strcmp(payload, "xh"))
    return(reg[0] == 'a');
  return(FALSE);
}

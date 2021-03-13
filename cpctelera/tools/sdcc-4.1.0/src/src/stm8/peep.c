#include "common.h"
#include "SDCCicode.h"
#include "SDCCglobl.h"
#include "SDCCgen.h"

#include "peep.h"

#define NOTUSEDERROR() do {werror(E_INTERNAL_ERROR, __FILE__, __LINE__, "error in notUsed()");} while(0)

// #define D(_s) { printf _s; fflush(stdout); }
#define D(_s)

#define EQUALS(l, i) (!STRCASECMP((l), (i)))
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

/*----------------------------------------------------------------------------*/
/* strNextCharBlock - Returns the next block of chars (after spaces, comma)   */
/* Leading spaces and Current block are skipped and search stops at next block*/
/* Valid block separators are: ' ' and ','                                    */
/* If no block is found (EOS or ';'), returns NULL                            */
/*----------------------------------------------------------------------------*/
static char *
strNextCharBlock(const char *str)
{
  if (!str || !str[0])
    return 0;

  while (isblank ((unsigned char)(str[0])))
    str++; // skip leading blanks

  while (str[0] && !isblank ((unsigned char)(str[0])) && str[0] != ';')
    {
      if (str[0] == ',')
        {
          str++; // current block is finished with ','
          break;
        }
      str++; // next char of current block
    }

  while (isblank ((unsigned char)(str[0])))
    str++; // skip trailing blanks 
    
  if (str[0] && str[0] != ';')
    return (char *)str;
  return 0;
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

static int
isReg(const char *what)
{
  if(what[0] == '(')
    what++;
  if(what[0] == 'a' || what[0] == 'x' || what[0] == 'y')
    return(true);
  if(!strcmp(what, "sp"))
    return(true);
  return(false);
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

static bool
isRelativeAddr(const char *what, const char *mode)
{
  char buf[4];
  strcpy(buf, mode);
  strcat(buf, ")");
  return(what[0] == '(' && strstr(what, buf));
}

static bool
isLabel(const char *what)
{
  const char *end;

  end = strchr(what, '+');
  if(!end)
    end = what + strlen(what);
  if(what[0] == '(' && !strchr(what, ','))
    what++;
  if(what[0] == '#')
    return (what[1] == '_' || what[1] == '<' || what[1] == '>');
  return(what[0] == '_' || *(end-1) == '$');
}

static bool
isImmediate(const char *what)
{
  return(what[0] == '#');
}

static bool
isShortoff(const char *what, const char *mode)
{
  return(isRelativeAddr(what, mode) && isInt(what) && readint(what) <= 0xff);
}

static bool
isLongoff(const char *what, const char *mode)
{
  return(isRelativeAddr(what, mode) && (!isInt(what) || readint(what) > 0xff));
}

static bool
isPtr(const char *what)
{
  return(what[0] == '[' || what[0] == '(' && (what[1] == '[' || what[1] == '('));
}

static bool
isSpIndexed(const char *what)
{
  return isRelativeAddr(what, "sp");
}

/*-----------------------------------------------------------------*/
/* stm8InstIsRegToReg - Checks if 'line' is a reg to reg move      */
/* isword == FALSE : Look for registers a, xl, xh, yl & yh         */
/* isword == TRUE  : Look for registers x, y & sp                  */
/*-----------------------------------------------------------------*/
static bool
stm8InstIsRegToReg(const char *line, bool isword)
{
  int regNumber = 0;

  if ((line = strNextCharBlock(line)))
    {
      while(line[0])
        {
          bool regFound = false;
          char chrLow = tolower ((unsigned char)line[0]);

          // Check for register names
          if (isword)
            {
              if (chrLow == 'x' || chrLow == 'y')
                regFound = true;

              if (chrLow == 's')
                {
                  line++;
                  if (line[0] == 'p')
                    regFound = true;
                  else
                    return false;
                }
            }
          else
            {
              if (chrLow == 'a')
                regFound = true;

              if (chrLow == 'x' || chrLow == 'y')
                {
                  line++;
                  chrLow = tolower ((unsigned char)line[0]);
                  if (chrLow == 'h' || chrLow == 'l')
                    regFound = true;
                  else
                    return false;
                }
            }

          // If register, process next character
          if (regFound)
            line++;

          // Continue only if valid separator or end
          if (!line[0] || line[0] == ',' || isblank (line[0]))
            {
              if (regFound)
                regNumber++;
            }
          else
            {
              return false;
            }

          // Next char if not eos
          if(line[0])
            line++;
        }
    }
  return (regNumber == 2);
}

int
stm8instructionSize(lineNode *pl)
{ // this function is quite rough, it makes all indirect addressing cases to the longest.
  char *operand;
  char *op1start;
  char *op2start;
  
  operand = nextToken(pl->line);
  op1start = nextToken(NULL);
  op2start = nextToken(NULL);

  while(op1start && isspace((unsigned char)op1start[0])) op1start++;
  while(op2start && isspace((unsigned char)op2start[0])) op2start++;
  //printf("line=%s operand=%s op1start=%s op2start=%s\n", pl->line, operand, op1start, op2start);

  /* Operations that always costs 1 byte */
  if (ISINST(operand, "ccf")
    || ISINST(operand, "divw")
    || ISINST(operand, "exgw")
    || ISINST(operand, "iret")
    || ISINST(operand, "nop")
    || ISINST(operand, "rcf")
    || ISINST(operand, "ret")
    || ISINST(operand, "retf")
    || ISINST(operand, "rvf")
    || ISINST(operand, "break")
    || ISINST(operand, "halt")
    || ISINST(operand, "rim")
    || ISINST(operand, "trap")
    || ISINST(operand, "wfi")
    || ISINST(operand, "sim")
    || ISINST(operand, "scf"))
      return 1;

  /* Operations that always costs 3 byte */
  if(ISINST(operand, "jrh")
    || ISINST(operand, "jrnh")
    || ISINST(operand, "jril")
    || ISINST(operand, "jrih")
    || ISINST(operand, "jrm")
    || ISINST(operand, "jrnm"))
      return 3;

  /* Operations that always costs 2 byte */
  if(STARTSINST(operand, "jr")
    || ISINST(operand, "callr")
    || ISINST(operand, "wfe"))
      return 2;

  /* Operations that always costs 4 byte */
  if(ISINST(operand, "bccm")
    || ISINST(operand, "bcpl")
    || ISINST(operand, "bres")
    || ISINST(operand, "bset")
    || ISINST(operand, "callf")
    || ISINST(operand, "int")
    || ISINST(operand, "jpf"))
      return 4;

  /* Operations that always costs 5 byte */
  if(ISINST(operand, "btjf")
    || ISINST(operand, "btjt"))
      return 5;

  if (EQUALS(operand, "push")
    || EQUALS(operand, "pop"))
  {
    wassert (op1start);
    if (!strcmp(op1start, "a"))
      return 1;
    if (!strcmp(op1start, "cc"))
      return 1;
    if (isImmediate(op1start)) // immediate
      return 2;
    else // longmem
      return 3;
  }

  /* arity=1 */
  if(EQUALS(operand, "clr")
                || EQUALS(operand, "dec")
                || EQUALS(operand, "inc")
                || EQUALS(operand, "swap")
                || EQUALS(operand, "jp")
                || EQUALS(operand, "call")
                || EQUALS(operand, "cpl")
                || EQUALS(operand, "neg")
                || EQUALS(operand, "sll")
                || EQUALS(operand, "sla")
                || EQUALS(operand, "srl")
                || EQUALS(operand, "sra")
                || EQUALS(operand, "rlc")
                || EQUALS(operand, "rrc")
                || EQUALS(operand, "tnz"))
  {
    int i = 0;

    wassert (op1start);
    if(!strcmp(op1start, "a") || !strcmp(op1start, "(x)"))
      return(1);
    if(!strcmp(op1start, "(y)"))
      return(2);
    if(op1start[0] == '('|| op1start[0] == '[')
      op1start++;
    if(strstr(op1start, ",y)"))
      i++; // costs extra byte for operating with y
    if ((ISINST(operand, "jp") || ISINST(operand, "call")) && *op1start != '(' && *op1start != '[') // jp and call are 3 bytes for direct long addressing mode.
      return(3);
    if(isLabel(op1start))
      return(4);
    if(readint(op1start) <= 0xFF)
      return(2+i);
    /* op1 > 0xFF */
    if((ISINST(operand, "jp") || ISINST(operand, "call")) && !strchr(op1start, 'y'))
      return(3);
    return(4);
  }

  if(EQUALS(operand, "exg"))
  {
    assert (!strcmp(op1start, "a") && op2start != NULL);
    if(isReg(op2start))
      return(1);
    else
      return(3);
  }

  if(EQUALS(operand, "addw") || EQUALS(operand, "subw"))
  {
    assert (op1start != NULL);
    if(!strcmp(op1start, "sp"))
      return(2);
    if(isImmediate(op2start) && op1start[0] == 'y')
      return(4);
    if(isImmediate(op2start) && op1start[0] == 'x')
      return(3);
    if(isSpIndexed(op2start))
      return(3);
    return(4);
  }

  if(ISINST(operand, "cplw"))
  {
    assert (op1start != NULL);
    if(op1start[0] == 'y')
      return(2);
    else
      return(1);
  }

  if(ISINST(operand, "ldf"))
  {
    assert (op1start != NULL);
    if(isRelativeAddr(op1start, "y") || isRelativeAddr(op2start, "y"))
      return(5);
    else
      return(4);
  }

  /* Operations that costs 2 or 3 bytes for immediate */
  if(STARTSINST(operand, "ld")
                || !strncmp(operand, "cp", 2)
                || EQUALS(operand, "adc")
                || EQUALS(operand, "add")
                || EQUALS(operand, "and")
                || EQUALS(operand, "bcp")
                || EQUALS(operand, "or")
                || EQUALS(operand, "sbc")
                || EQUALS(operand, "sub")
                || EQUALS(operand, "xor"))
  {
    int i = 0;
    char suffix;
    wassert (op1start && op2start);
    suffix = operand[strlen(operand)-1];
    if(suffix == 'w' && isImmediate(op2start))
      {
        i++; // costs extra byte
        if(!strcmp(op1start, "y"))
          i++;
      }
    if(isImmediate(op2start))
      return(2+i); // ld reg, #immd
    if(isSpIndexed(op1start) || isSpIndexed(op2start))
      return(2);
    if(!strcmp(op1start, "(x)") || !strcmp(op2start, "(x)"))
      return(1);
    if(!strcmp(op1start, "(y)") || !strcmp(op2start, "(y)"))
      return(2);
    if(isShortoff(op1start, "x") || isShortoff(op2start, "x"))
      return(2);
    if(isShortoff(op1start, "y") || isShortoff(op2start, "y"))
      return(3);
    if(isLongoff(op1start, "x") || isLongoff(op2start, "x"))
      return(3);
    if(isLongoff(op1start, "y") || isLongoff(op2start, "y"))
      return(4);
    if(isPtr(op1start) || isPtr(op2start))
      return(4);
    if(strchr(op1start, 'y') || strchr(op2start, 'y'))
      i++; // costs extra byte for operating with y
    if(isLabel(op1start) || isLabel(op2start))
      return(3+i);
    if(isReg(op1start) && isReg(op2start))
      {
        if (!strncmp(op1start, "x", 1) && (!strncmp(op2start, "y", 1) || !strncmp(op2start, "sp", 2))
          || !strncmp(op1start, "sp", 2) && !strncmp(op2start, "x", 1))
          return(1);
        return(1+i);
      }
    if(!strcmp(op2start, "a"))
      return(3);
    if(readint(op2start) <= 0xFF)
      return(2+i);
    else
      return(3+i);
    return 4;
  }

  /* mov costs 3, 4 or 5 bytes depending on its addressing mode */
  if(ISINST(operand, "mov")) {
    assert (op1start != NULL && op2start != NULL);
    if(isImmediate(op2start))
      return(4);
    if(isLabel(op2start))
      return(5);
    if(readint(op2start) <= 0xFF)
      return(3);
    if(readint(op2start) > 0xFF)
      return(5);
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
    assert (op1start != NULL);
    if((op1start && !strcmp(op1start, "y")) || (op2start && !strcmp(op2start, "y")))
      return(2);
    else
      return(1);
  }

  if(ISINST(pl->line, ".db") || ISINST(pl->line, ".byte"))
    {
      int i, j;
      for(i = 1, j = 0; pl->line[j]; i += pl->line[j] == ',', j++);
      return(i);
    }

  if(ISINST(pl->line, ".dw") || ISINST(pl->line, ".word"))
    {
      int i, j;
      for(i = 1, j = 0; pl->line[j]; i += pl->line[j] == ',', j++);
      return(i * 2);
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

  /* In each jump the label is at the end */
  p = strlen (pl->line) - 1 + pl->line;

  /* Skip trailing whitespace */
  while(isspace(*p))
    p--;

  /* scan backward until space or ',' */
  for (; p > pl->line; p--)
    if (isspace(*p) || *p == ',')
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
static bool argCont(const char *arg, char what)
{
  if (arg == NULL || strlen (arg) == 0 || !(what == 'a' || what == 'x' || what == 'y'))
    return FALSE;

  while (isblank ((unsigned char)(arg[0])))
    arg++;

  if (arg[0] == ',')
    arg++;

  while (isblank ((unsigned char)(arg[0])))
    arg++;

  if (arg[0] == '#')
    return FALSE;

  if (arg[0] == '(' && arg[1] == '0' && (tolower(arg[2])) == 'x') 
    arg += 3; // Skip hex prefix to avoid false x positive.

  if (strlen(arg) == 0)
    return FALSE;

  if (arg[0] == '_' && what == 'a') // The STM8 has no a-relative addressing modes.
    return FALSE;

  return (strchr(arg, what) != NULL);
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
      else if(spec->noun == V_INT && spec->b_long || spec->noun == V_FLOAT)
        size = 4;
      else // long long is not returned in registers.
        size = 0;

      // Check for returned pointer.
      sym_lnk = sym->type;
      while (sym_lnk && !IS_PTR (sym_lnk))
        sym_lnk = sym_lnk->next;
      if(IS_PTR(sym_lnk))
        size = IS_FUNCPTR(sym_lnk) ? FUNCPTRSIZE : GPTRSIZE;
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
stm8MightReadFlag(const lineNode *pl, const char *what)
{
  if (strcmp (what, "c") && strcmp (what, "n") && strcmp (what, "z"))
    return true;

  if (ISINST (pl->line, "push"))
     return (pl->line[5] == 'c');

  if (!strcmp (what, "n"))
    return (ISINST (pl->line, "jrmi") || ISINST (pl->line, "jrpl") || ISINST (pl->line, "jrsge") || ISINST (pl->line, "jrsgte") || ISINST (pl->line, "jrsle") || ISINST (pl->line, "jrslt"));

  if (!strcmp (what, "z"))
    return (ISINST (pl->line, "jreq") || ISINST (pl->line, "jrne") || ISINST (pl->line, "jrsgte") || ISINST (pl->line, "jrsle"));

  if (!strcmp (what, "c"))
    return (ISINST (pl->line, "jrc") || ISINST (pl->line, "jrnc") || ISINST (pl->line, "jruge") || ISINST (pl->line, "jrugt") || ISINST (pl->line, "jrule") || ISINST (pl->line, "jrult") ||
      ISINST (pl->line, "adc") || ISINST (pl->line, "sbc") ||
      ISINST (pl->line, "ccf") || ISINST (pl->line, "rlc") || ISINST (pl->line, "rlcw") || ISINST (pl->line, "rrc") || ISINST (pl->line, "rrcw"));

  return true;
}

static bool
stm8MightRead(const lineNode *pl, const char *what)
{
  char extra = 0;

  if (!strcmp (what, "xl") || !strcmp (what, "xh"))
    extra = 'x';
  else if (!strcmp (what, "yl") || !strcmp (what, "yh"))
    extra = 'y';
  else if (strcmp (what, "a") != 0)
    return stm8MightReadFlag(pl, what);

  if (!extra)
    {
      if (ISINST (pl->line, "adc")
        || ISINST (pl->line, "and")
        || ISINST (pl->line, "bcp")
        || ISINST (pl->line, "cp")
        || ISINST (pl->line, "div")
        || ISINST (pl->line, "mul")
        || ISINST (pl->line, "or")
        || ISINST (pl->line, "rlwa")
        || ISINST (pl->line, "rrwa")
        || ISINST (pl->line, "sbc")
        || ISINST (pl->line, "trap")
        || ISINST (pl->line, "xor"))
          return TRUE;

      if ((ISINST (pl->line, "add")
        || ISINST (pl->line, "cpl")
        || ISINST (pl->line, "dec")
        || ISINST (pl->line, "exg")
        || ISINST (pl->line, "inc")
        || ISINST (pl->line, "neg")
        || ISINST (pl->line, "rlc")
        || ISINST (pl->line, "rrc")
        || ISINST (pl->line, "sll")
        || ISINST (pl->line, "sla")
        || ISINST (pl->line, "sra")
        || ISINST (pl->line, "srl")
        || ISINST (pl->line, "sub")
        || ISINST (pl->line, "tnz")) &&
        pl->line[4] == 'a')
          return TRUE;

      if ((ISINST (pl->line, "push")
        || ISINST (pl->line, "swap")) &&
        pl->line[5] == 'a')
          return TRUE;

      if ((ISINST (pl->line, "ld") || ISINST (pl->line, "ldf")) && argCont (strchr (pl->line, ','), 'a'))
          return TRUE;
    }
  else
    {
      if (ISINST (pl->line, "divw") || ISINST (pl->line, "exgw") || ISINST (pl->line, "trap"))
        return TRUE;
 
      if (ISINST (pl->line, "exg") && strstr (strchr(pl->line, ','), what))
        return true;

      if ((ISINST (pl->line, "div") || ISINST (pl->line, "mul")) && pl->line[4] == extra)
        return true;

      if ((ISINST (pl->line, "addw")
        || ISINST (pl->line, "cplw")
        || ISINST (pl->line, "decw")
        || ISINST (pl->line, "incw")
        || ISINST (pl->line, "negw")
        || ISINST (pl->line, "rlcw")
        || ISINST (pl->line, "rlwa")
        || ISINST (pl->line, "rrcw")
        || ISINST (pl->line, "rrwa")
        || ISINST (pl->line, "sllw")
        || ISINST (pl->line, "slaw")
        || ISINST (pl->line, "sraw")
        || ISINST (pl->line, "srlw")
        || ISINST (pl->line, "subw")
        || ISINST (pl->line, "tnzw")) &&
        pl->line[5] == extra)
          return TRUE;

      if ((ISINST (pl->line, "pushw")
        || ISINST (pl->line, "swapw")) && pl->line[6] == extra)
          return TRUE;

      if (ISINST (pl->line, "cpw") && pl->line[4] == extra)
        return TRUE;

      if ((strchr (pl->line, ',') ? argCont (strchr (pl->line, ','), extra) : argCont (strchr (pl->line, '('), extra)) &&
        (ISINST (pl->line, "adc")
        || ISINST (pl->line, "add")
        || ISINST (pl->line, "and")
        || ISINST (pl->line, "bcp")
        || ISINST (pl->line, "call")
        || ISINST (pl->line, "clr")
        || ISINST (pl->line, "cp")
        || ISINST (pl->line, "cpl")
        || ISINST (pl->line, "dec")
        || ISINST (pl->line, "inc")
        || ISINST (pl->line, "jp")
        || ISINST (pl->line, "neg")
        || ISINST (pl->line, "or")
        || ISINST (pl->line, "rlc")
        || ISINST (pl->line, "rrc")
        || ISINST (pl->line, "sbc")
        || ISINST (pl->line, "sll")
        || ISINST (pl->line, "sla")
        || ISINST (pl->line, "sra")
        || ISINST (pl->line, "srl")
        || ISINST (pl->line, "sub")
        || ISINST (pl->line, "swap")
        || ISINST (pl->line, "tnz")
        || ISINST (pl->line, "cpw")
        || ISINST (pl->line, "ldf")
        || ISINST (pl->line, "ldw")
        || ISINST (pl->line, "ld")
        || ISINST (pl->line, "xor")))
          return TRUE;

      if (ISINST (pl->line, "ld") || ISINST (pl->line, "ldw"))
        {
          char buf[64], *p;
          strcpy (buf, pl->line);
          if (!!(p = strstr (buf, "0x")) || !!(p = strstr (buf, "0X")))
            p[0] = p[1] = ' ';
          if (!!(p = strchr (buf, '(')) && !!strchr (p, extra))
            return TRUE;
        }
    }

  if(ISINST(pl->line, "ret") || ISINST(pl->line, "retf"))
    return(isReturned(what));

  return FALSE;
}

static bool
stm8UncondJump(const lineNode *pl)
{
  return (ISINST(pl->line, "jp") || ISINST(pl->line, "jra") || ISINST(pl->line, "jrt") || ISINST(pl->line, "jpf"));
}

static bool
stm8CondJump(const lineNode *pl)
{
  return (!stm8UncondJump(pl) && STARTSINST(pl->line, "jr") ||
    ISINST(pl->line, "btjt") || ISINST(pl->line, "btjf"));
}

static bool
stm8SurelyWritesFlag(const lineNode *pl, const char *what)
{
  if (!strcmp (what, "n") || !strcmp (what, "z"))
    {
      if (ISINST (pl->line, "addw") && !strcmp (pl->line + 5, "sp"))
        return false;
      if (ISINST (pl->line, "sub") && !strcmp (pl->line + 4, "sp"))
        return false;
      if (ISINST (pl->line, "ld"))
        return !stm8InstIsRegToReg(pl->line, false);
      if (ISINST (pl->line, "ldw"))
        return !stm8InstIsRegToReg(pl->line, true);
      if (ISINST (pl->line, "pop"))
        return (pl->line[5] == 'c');
      if (ISINST (pl->line, "bccm") || ISINST (pl->line, "bcpl") ||
        ISINST (pl->line, "break") ||
        ISINST (pl->line, "bres") || ISINST (pl->line, "bset") ||
        ISINST (pl->line, "btjf") || ISINST (pl->line, "btjt") ||
        ISINST (pl->line, "call") || ISINST (pl->line, "callf") || ISINST (pl->line, "callr") ||
        ISINST (pl->line, "ccf") ||
        ISINST (pl->line, "exg") || ISINST (pl->line, "exgw") ||
        ISINST (pl->line, "halt") || ISINST (pl->line, "int") ||
        STARTSINST (pl->line, "jp") ||
        STARTSINST (pl->line, "jr") ||
        ISINST (pl->line, "mov") || ISINST (pl->line, "mul") ||
        ISINST (pl->line, "nop") ||
        ISINST (pl->line, "popw") || ISINST (pl->line, "push") || ISINST (pl->line, "pushw") ||
        ISINST (pl->line, "rcf") ||
        ISINST (pl->line, "ret") || ISINST (pl->line, "retf") ||
        ISINST (pl->line, "rvf") || ISINST (pl->line, "scf") ||
        ISINST (pl->line, "sim") || ISINST (pl->line, "trap") || ISINST (pl->line, "wfe") || ISINST (pl->line, "wfi"))
        return false;
      return true;
    }
  else if (!strcmp (what, "c"))
    {        
      if (ISINST (pl->line, "addw") && !strcmp (pl->line + 5, "sp"))
        return false;
      if (ISINST (pl->line, "sub") && !strcmp (pl->line + 4, "sp"))
        return false;
        
      if (ISINST (pl->line, "adc") ||
        STARTSINST (pl->line, "add") || // add, addw
        STARTSINST (pl->line, "btj") || // btjt, btjf
        ISINST (pl->line, "ccf") ||
        STARTSINST (pl->line, "cp") || // cp, cpw, cpl, cplw
        STARTSINST (pl->line, "div") || // div, divw
        STARTSINST (pl->line, "neg") || // neg, negw
        ISINST (pl->line, "rcf") ||
        STARTSINST (pl->line, "rlc") || // rlc, rlcw
        STARTSINST (pl->line, "rrc") || // rrc, rrcw
        ISINST (pl->line, "sbc") ||
        ISINST (pl->line, "scf") ||
        STARTSINST (pl->line, "sl") || // sll, sla, sllw, slaw
        STARTSINST (pl->line, "sr") || // sra, sraw, srl, srlw
        STARTSINST (pl->line, "sub")) // sub, subw
        return true;
    }

  return false;
}

static bool
stm8SurelyWrites(const lineNode *pl, const char *what)
{
  char extra = 0;
  if (!strcmp (what, "xl") || !strcmp (what, "xh"))
    extra = 'x';
  else if (!strcmp (what, "yl") || !strcmp (what, "yh"))
    extra = 'y';
  else if (strcmp (what, "a"))
    return (stm8SurelyWritesFlag (pl, what));

  if (!extra)
    {
      if (ISINST (pl->line, "adc")
        || ISINST (pl->line, "and")
        || ISINST (pl->line, "div")
        || ISINST (pl->line, "iret")
        || ISINST (pl->line, "or")
        || ISINST (pl->line, "rlwa")
        || ISINST (pl->line, "rrwa")
        || ISINST (pl->line, "sbc")
        || ISINST (pl->line, "xor"))
          return TRUE;

      if ((ISINST (pl->line, "add")
        || ISINST (pl->line, "clr")
        || ISINST (pl->line, "cpl")
        || ISINST (pl->line, "dec")
        || ISINST (pl->line, "exg")
        || ISINST (pl->line, "inc")
        || ISINST (pl->line, "neg")
        || ISINST (pl->line, "pop")
        || ISINST (pl->line, "rlc")
        || ISINST (pl->line, "rrc")
        || ISINST (pl->line, "sll")
        || ISINST (pl->line, "sla")
        || ISINST (pl->line, "sra")
        || ISINST (pl->line, "srl")
        || ISINST (pl->line, "ldf")
        || ISINST (pl->line, "sub")) &&
        pl->line[4] == 'a')
          return TRUE;

      if (ISINST (pl->line, "swap") && pl->line[5] == 'a')
        return TRUE;

      if (ISINST (pl->line, "ld") && pl->line[3] == 'a')
        return TRUE;
    }
  else
    {
      if (ISINST (pl->line, "divw")
        || ISINST (pl->line, "exgw")
        || ISINST (pl->line, "iret"))
          return TRUE;

      if ((ISINST (pl->line, "div")
        || ISINST (pl->line, "ldw")
        || ISINST (pl->line, "mul"))
        && pl->line[4] == extra)
          return TRUE;

      if ((ISINST (pl->line, "addw")
        || ISINST (pl->line, "clrw")
        || ISINST (pl->line, "cplw")
        || ISINST (pl->line, "decw")
        || ISINST (pl->line, "incw")
        || ISINST (pl->line, "negw")
        || ISINST (pl->line, "popw")
        || ISINST (pl->line, "rlcw")
        || ISINST (pl->line, "rlwa")
        || ISINST (pl->line, "rrcw")
        || ISINST (pl->line, "rrwa")
        || ISINST (pl->line, "sllw")
        || ISINST (pl->line, "slaw")
        || ISINST (pl->line, "sraw")
        || ISINST (pl->line, "srlw")
        || ISINST (pl->line, "subw")) &&
        pl->line[5] == extra)
          return TRUE;

      if (ISINST (pl->line, "swapw") && pl->line[6] == extra)
        return TRUE;

      if (ISINST (pl->line, "ld")
        && strncmp (pl->line + 3, what, strlen (what)) == 0)
        return TRUE;

      if (ISINST (pl->line, "exg") && strstr (strstr (pl->line, ","), what))
        return true;
    }

  return false;
}

static bool
stm8SurelyReturns(const lineNode *pl)
{
  return(ISINST(pl->line, "ret") || ISINST(pl->line, "retf"));
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

      if(stm8MightRead(*pl, what))
        {
          D(("S4O_RD_OP\n"));
          return S4O_RD_OP;
        }

      // Check writes before conditional jumps, some jumps (btjf, btjt) write 'c'
      if(stm8SurelyWrites(*pl, what))
        {
          D(("S4O_WR_OP\n"));
          return S4O_WR_OP;
        }

      if(stm8UncondJump(*pl))
        {
          *pl = findLabel (*pl);
            if (!*pl)
              {
                D(("S4O_ABORT at unconditional jump\n"));
                return S4O_ABORT;
              }
        }
      if(stm8CondJump(*pl))
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

      /* Don't need to check for de, hl since stm8MightRead() does that */
      if(stm8SurelyReturns(*pl))
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
stm8notUsed (const char *what, lineNode *endPl, lineNode *head)
{
  lineNode *pl;
  if(strcmp(what, "x") == 0)
    return(stm8notUsed("xl", endPl, head) && stm8notUsed("xh", endPl, head));
  else if(strcmp(what, "y") == 0)
    return(stm8notUsed("yl", endPl, head) && stm8notUsed("yh", endPl, head));

  _G.head = head;

  unvisitLines (_G.head);

  pl = endPl->next;
  return (doTermScan (&pl, what));
}

bool
stm8notUsedFrom (const char *what, const char *label, lineNode *head)
{
  lineNode *cpl;

  for (cpl = head; cpl; cpl = cpl->next)
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

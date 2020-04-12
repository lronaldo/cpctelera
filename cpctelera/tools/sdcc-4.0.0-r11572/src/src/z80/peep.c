/*-------------------------------------------------------------------------
  peep.c - source file for peephole optimizer helper functions

  Written By - Philipp Klaus Krause

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

#include "common.h"
#include "SDCCicode.h"
#include "z80.h"
#include "SDCCglobl.h"
#include "SDCCpeeph.h"
#include "gen.h"

#define NOTUSEDERROR() do {werror(E_INTERNAL_ERROR, __FILE__, __LINE__, "error in notUsed()");} while(0)

#if 0
#define D(_s) { printf _s; fflush(stdout); }
#else
#define D(_s)
#endif

#define ISINST(l, i) (!STRNCASECMP((l), (i), sizeof(i) - 1) && (!(l)[sizeof(i) - 1] || isspace((unsigned char)((l)[sizeof(i) - 1]))))

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

extern bool z80_regs_used_as_parms_in_calls_from_current_function[IYH_IDX + 1];
extern bool z80_symmParm_in_calls_from_current_function;
extern bool z80_regs_preserved_in_calls_from_current_function[IYH_IDX + 1];

/*-----------------------------------------------------------------*/
/* univisitLines - clear "visited" flag in all lines               */
/*-----------------------------------------------------------------*/
static void
unvisitLines (lineNode *pl)
{
  for (; pl; pl = pl->next)
    pl->visited = FALSE;
}

#define AOP(op) op->aop
#define AOP_SIZE(op) AOP(op)->size

static bool
isReturned(const char *what)
{
  symbol *sym;
  sym_link *sym_lnk;
  int size;
  lineNode *l;

  if(strncmp(what, "iy", 2) == 0)
    return FALSE;
  if(strlen(what) != 1)
    return TRUE;

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
      if(spec->noun == V_VOID || spec->noun == V_INT && spec->b_longlong) // long long is not returned via registers for the Z80-related ports
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
      size = 4;
    }

  switch(*what)
    {
    case 'd':
      return(size >= 4);
    case 'e':
      return(size >= 3);
    case 'h':
      return(size >= 2);
    case 'l':
      return(size >= 1);
    default:
      return FALSE;
    }
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

  /* In each z80 jumping opcode the label is at the end of the opcode */
  p = strlen (pl->line) - 1 + pl->line;

  /* scan backward until ',' or '\t' */
  for (; p > pl->line; p--)
    if (*p == ',' || isspace(*p))
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
      if (cpl->isLabel
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
  wassert (arg);

  while(isspace (*arg) || *arg == ',')
    arg++;

  if (arg[0] == '#')
    return false;

  if(arg[0] == '(' && arg[1] && arg[2] && (arg[2] != ')' && arg[3] != ')'))
    return FALSE;

  if(*arg == '(')
    arg++;

  // Get suitable end to avoid reading into further arguments.
  const char *end = strchr(arg, ',');
  if (!end)
    end = arg + strlen(arg);

  const char *found = StrStr(arg, what);

  return(found && found < end);
}

static bool
z80MightBeParmInCallFromCurrentFunction(const char *what)
{
  if (strchr(what, 'l') && z80_regs_used_as_parms_in_calls_from_current_function[L_IDX])
    return TRUE;
  if (strchr(what, 'h') && z80_regs_used_as_parms_in_calls_from_current_function[H_IDX])
    return TRUE;
  if (strchr(what, 'e') && z80_regs_used_as_parms_in_calls_from_current_function[E_IDX])
    return TRUE;
  if (strchr(what, 'd') && z80_regs_used_as_parms_in_calls_from_current_function[D_IDX])
    return TRUE;
  if (strchr(what, 'c') && z80_regs_used_as_parms_in_calls_from_current_function[C_IDX])
    return TRUE;
  if (strchr(what, 'b') && z80_regs_used_as_parms_in_calls_from_current_function[B_IDX])
    return TRUE;
  if (strstr(what, "iy") && (z80_regs_used_as_parms_in_calls_from_current_function[IYL_IDX] || z80_regs_used_as_parms_in_calls_from_current_function[IYH_IDX]))
    return TRUE;

  return FALSE;
}

static bool
z80MightRead(const lineNode *pl, const char *what)
{
  if(strcmp(what, "iyl") == 0 || strcmp(what, "iyh") == 0)
    what = "iy";
  if(strcmp(what, "ixl") == 0 || strcmp(what, "ixh") == 0)
    what = "ix";

  if(strcmp(pl->line, "call\t__initrleblock") == 0)
    return TRUE;

  if(strcmp(pl->line, "call\t___sdcc_call_hl") == 0 && (strchr(what, 'h') != 0 || strchr(what, 'l') != 0))
    return TRUE;

  if(strcmp(pl->line, "call\t___sdcc_call_iy") == 0 && strstr(what, "iy") != 0)
    return TRUE;

  if(strncmp(pl->line, "call\t", 5) == 0 && strchr(pl->line, ',') == 0)
    {
      const symbol *f = findSym (SymbolTab, 0, pl->line + 6);
      if (f)
      {
        const value *args = FUNC_ARGS (f->type);

        if (IFFUNC_ISZ88DK_FASTCALL (f->type) && args) // Has one register argument of size up to 32 bit.
          {
            const unsigned int size = getSize (args->type);
            wassert (!args->next); // Only one argment allowed in __z88dk_fastcall functions.
            if (strchr(what, 'l') && size >= 1)
              return TRUE;
            if (strchr(what, 'h') && size >= 2)
              return TRUE;
            if (strchr(what, 'e') && size >= 3)
              return TRUE;
            if (strchr(what, 'd') && size >= 4)
              return TRUE;
          }
        return FALSE;
      }
      else // Fallback needed for calls through function pointers and for calls to literal addresses.
        return z80MightBeParmInCallFromCurrentFunction(what);
    }

  if(ISINST(pl->line, "reti") || ISINST(pl->line, "retn"))
    return(false);

  if(ISINST(pl->line, "ret")) // --reserve-regs-iy uses ret in code gen for calls through function pointers
    return(IY_RESERVED ? isReturned(what) || z80MightBeParmInCallFromCurrentFunction(what) : isReturned(what));

  if(!strcmp(pl->line, "ex\t(sp), hl") || !strcmp(pl->line, "ex\t(sp),hl"))
    return(!strcmp(what, "h") || !strcmp(what, "l"));
  if(!strcmp(pl->line, "ex\t(sp), ix") || !strcmp(pl->line, "ex\t(sp),ix"))
    return(!!strstr(what, "ix"));
  if(!strcmp(pl->line, "ex\t(sp), iy") || !strcmp(pl->line, "ex\t(sp),iy"))
    return(!!strstr(what, "iy"));
  if(!strcmp(pl->line, "ex\tde, hl") || !strcmp(pl->line, "ex\tde,hl"))
    return(!strcmp(what, "h") || !strcmp(what, "l") || !strcmp(what, "d") || !strcmp(what, "e"));
  if(ISINST(pl->line, "ld"))
    {
      if(argCont(strchr(pl->line, ','), what))
        return(true);
      if(*(strchr(pl->line, ',') - 1) == ')' && strstr(pl->line + 3, what) && (strchr(pl->line, '#') == 0 || strchr(pl->line, '#') > strchr(pl->line, ',')))
        return(true);
      return(false);
    }

  if(!strcmp(pl->line, "xor\ta, a") || !strcmp(pl->line, "xor\ta,a"))
    return(false);

  if(ISINST(pl->line, "adc") ||
     ISINST(pl->line, "add") ||
     ISINST(pl->line, "and") ||
     ISINST(pl->line, "sbc") ||
     ISINST(pl->line, "sub") ||
     ISINST(pl->line, "xor"))
    {
      const char *arg = pl->line + 4;
      while(isspace(*arg))
        arg++;
      if(arg[0] == 'a' && arg[1] == ',')
        {
          if(!strcmp(what, "a"))
            return(true);
          arg += 2;
        }
      else if(!strncmp(arg, "hl", 2) && arg[2] == ',') // add hl, rr
        {
          if(!strcmp(what, "h") || !strcmp(what, "l"))
            return(true);
          arg += 3;
        }
      else if(arg[0] == 'i') // add ix/y, rr
        {
          if(!strncmp(arg, what, 2))
            return(true);
          arg += 3;
        }
      return(argCont(arg, what));
    }

  if(ISINST(pl->line, "or") || ISINST(pl->line, "cp") )
    {
      const char *arg = pl->line + 3;
      while(isspace(*arg))
        arg++;
      if(*arg == 'a' && *(arg + 1) == ',')
        {
          if(!strcmp(what, "a"))
            return(true);
          arg += 2;
        }
      else if(!strncmp(arg, "hl", 2) && *(arg + 2) == ',')
        {
          if(!strcmp(what, "h") || !strcmp(what, "l"))
            return(true);
          arg += 3;
        }
      return(argCont(arg, what));
    }

  if(ISINST(pl->line, "neg"))
    return(strcmp(what, "a") == 0);

  if(ISINST(pl->line, "pop"))
    return(false);

  if(ISINST(pl->line, "push"))
    return(strstr(pl->line + 5, what) != 0);

  if(ISINST(pl->line, "dec") ||
     ISINST(pl->line, "inc"))
    {
      return(argCont(pl->line + 4, what));
    }

  if(ISINST(pl->line, "cpl"))
    return(!strcmp(what, "a"));

  if(ISINST(pl->line, "di") || ISINST(pl->line, "ei"))
    return(false);

  // Rotate and shift group
  if(ISINST(pl->line, "rlca") ||
     ISINST(pl->line, "rla")  ||
     ISINST(pl->line, "rrca") ||
     ISINST(pl->line, "rra"))
    {
      return(strcmp(what, "a") == 0);
    }
  if(ISINST(pl->line, "rl") ||
     ISINST(pl->line, "rr"))
    {
      return(argCont(pl->line + 3, what));
    }
  if(ISINST(pl->line, "rlc") ||
     ISINST(pl->line, "sla") ||
     ISINST(pl->line, "sra") ||
     ISINST(pl->line, "srl"))
    {
      return(argCont(pl->line + 4, what));
    }
  if(!IS_GB && !IS_RAB &&
    (ISINST(pl->line, "rld") ||
     ISINST(pl->line, "rrd")))
    return(!!strstr("ahl", what));

  // Bit set, reset and test group
  if(ISINST(pl->line, "bit") ||
     ISINST(pl->line, "set") ||
     ISINST(pl->line, "res"))
    {
      return(argCont(strchr(pl->line + 4, ','), what));
    }

 if(ISINST(pl->line, "ccf") ||
    ISINST(pl->line, "nop"))
    return(false);

  if(ISINST(pl->line, "jp") || ISINST(pl->line, "jr"))
    return(false);

  if(ISINST(pl->line, "djnz"))
    return(strchr(what, 'b') != 0);

  if(!IS_GB && (ISINST(pl->line, "ldd") || ISINST(pl->line, "lddr") || ISINST(pl->line, "ldi") || ISINST(pl->line, "ldir")))
    return(strchr("bcdehl", *what));

  if(!IS_GB && !IS_RAB && (ISINST(pl->line, "cpd") || ISINST(pl->line, "cpdr") || ISINST(pl->line, "cpi") || ISINST(pl->line, "cpir")))
    return(strchr("abchl", *what));

  if(!IS_GB && !IS_RAB && ISINST(pl->line, "out"))
    return(strstr(strchr(pl->line + 4, ','), what) != 0 || strstr(pl->line + 4, "(c)") && (!strcmp(what, "b") || !strcmp(what, "c")));
  if(!IS_GB && !IS_RAB && ISINST(pl->line, "in"))
    return(!strstr(strchr(pl->line + 4, ','), "(c)") && !strcmp(what, "a") || strstr(strchr(pl->line + 4, ','), "(c)") && (!strcmp(what, "b") || !strcmp(what, "c")));

  if(!IS_GB && !IS_RAB &&
    (ISINST(pl->line, "ini") || ISINST(pl->line, "ind") || ISINST(pl->line, "inir") || ISINST(pl->line, "indr") ||
    ISINST(pl->line, "outi") || ISINST(pl->line, "outd") || ISINST(pl->line, "otir") || ISINST(pl->line, "otdr")))
    return(strchr("bchl", *what));

  if((IS_Z180 || IS_EZ80_Z80) && ISINST(pl->line, "in0"))
    return(false);

  if((IS_Z180 || IS_EZ80_Z80) && ISINST(pl->line, "mlt"))
    return(argCont(pl->line + 4, what));

  if((IS_Z180 || IS_EZ80_Z80) &&
    (ISINST(pl->line, "otim") || ISINST(pl->line, "otimr") || ISINST(pl->line, "otir") || ISINST(pl->line, "otirx")))
    return(strchr("bchl", *what));

  if((IS_Z180 || IS_EZ80_Z80) && ISINST(pl->line, "slp"))
    return(false);

  if((IS_Z180 || IS_EZ80_Z80) && ISINST(pl->line, "tst"))
    return(argCont(pl->line + 4, what));

  if((IS_Z180 || IS_EZ80_Z80) && ISINST(pl->line, "tstio"))
    return(!strcmp(what, "c"));

  if(IS_RAB && ISINST(pl->line, "mul"))
    return(!strcmp(what, "b") || !strcmp(what, "c") || !strcmp(what, "d") || !strcmp(what, "e"));

  if(IS_RAB && ISINST(pl->line, "bool"))
    return(argCont(pl->line + 5, what));

  /* TODO: Can we know anything about rst? */
  if(ISINST(pl->line, "rst"))
    return(true);

  return(true);
}

static bool
z80UncondJump(const lineNode *pl)
{
  if((ISINST(pl->line, "jp") || ISINST(pl->line, "jr")) &&
     strchr(pl->line, ',') == 0)
    return TRUE;
  return FALSE;
}

static bool
z80CondJump(const lineNode *pl)
{
  if(((ISINST(pl->line, "jp") || ISINST(pl->line, "jr")) &&
      strchr(pl->line, ',') != 0) ||
     ISINST(pl->line, "djnz"))
    return TRUE;
  return FALSE;
}

static bool
z80SurelyWrites(const lineNode *pl, const char *what)
{
  if(strcmp(what, "iyl") == 0 || strcmp(what, "iyh") == 0)
    what = "iy";
  if(strcmp(what, "ixl") == 0 || strcmp(what, "ixh") == 0)
    what = "ix";

  if(ISINST(pl->line, "xor") && strcmp(what, "a") == 0)
    return(true);
  if(ISINST(pl->line, "ld") && strncmp(pl->line + 3, "hl", 2) == 0 && (what[0] == 'h' || what[0] == 'l'))
    return(true);
  if(ISINST(pl->line, "ld") && strncmp(pl->line + 3, "de", 2) == 0 && (what[0] == 'd' || what[0] == 'e'))
    return(true);
  if(ISINST(pl->line, "ld") && strncmp(pl->line + 3, "bc", 2) == 0 && (what[0] == 'b' || what[0] == 'c'))
    return(true);
  if((ISINST(pl->line, "ld") || ISINST(pl->line, "in"))
    && strncmp(pl->line + 3, what, strlen(what)) == 0 && pl->line[3 + strlen(what)] == ',')
    return(true);
  if(ISINST(pl->line, "pop") && strstr(pl->line + 4, what))
    return(true);
  if(ISINST(pl->line, "call") && strchr(pl->line, ',') == 0)
    {
      const symbol *f = findSym (SymbolTab, 0, pl->line + 6);
      const bool *preserved_regs;

      if(!strcmp(what, "ix"))
        return(false);

      if(f)
          preserved_regs = f->type->funcAttrs.preserved_regs;
      else // Err on the safe side.
        preserved_regs = z80_regs_preserved_in_calls_from_current_function;

      if(!strcmp(what, "c"))
        return !preserved_regs[C_IDX];
      if(!strcmp(what, "b"))
        return !preserved_regs[B_IDX];
      if(!strcmp(what, "e"))
        return !preserved_regs[E_IDX];
      if(!strcmp(what, "d"))
        return !preserved_regs[D_IDX];
      if(!strcmp(what, "l"))
        return !preserved_regs[L_IDX];
      if(!strcmp(what, "h"))
        return !preserved_regs[H_IDX];
      if(!strcmp(what, "iy"))
        return !preserved_regs[IYL_IDX] && !preserved_regs[IYH_IDX];
    }
  if(strcmp(pl->line, "ret") == 0)
    return true;
  if(strcmp(pl->line, "ld\tiy")  == 0 && strncmp(what, "iy", 2) == 0)
    return true;

  if (IS_Z180 || IS_EZ80_Z80)
    {
      if (ISINST(pl->line, "mlt"))
        return(strchr(pl->line + 4, *what) != 0);

      if (ISINST(pl->line, "otim") ||
        ISINST(pl->line, "otimr") ||
        ISINST(pl->line, "otdm") ||
        ISINST(pl->line, "otdmr"))
        return(strchr("bchl", *what) != NULL);

      if (ISINST(pl->line, "in0"))
        return(!strncmp(pl->line + 4, what, strlen(what)));
    }

  return(false);
}

static bool
z80SurelyReturns(const lineNode *pl)
{
  if(strcmp(pl->line, "\tret") == 0)
    return TRUE;
  return FALSE;
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

/* Regular 8 bit reg */
static bool
isReg(const char *what)
{
  if(strlen(what) != 1)
    return FALSE;
  switch(*what)
    {
    case 'a':
    case 'b':
    case 'c':
    case 'd':
    case 'e':
    case 'h':
    case 'l':
      return TRUE;
    }
  return FALSE;
}

/* 8-Bit reg only accessible by 16-bit and undocumented instructions */
static bool
isUReg(const char *what)
{
  if(strcmp(what, "iyl") == 0 || strcmp(what, "iyh") == 0)
    return TRUE;
  if(strcmp(what, "ixl") == 0 || strcmp(what, "ixh") == 0)
    return TRUE;
  return FALSE;
}

static bool
isRegPair(const char *what)
{
  if(strlen(what) != 2)
    return FALSE;
  if(strcmp(what, "bc") == 0)
    return TRUE;
  if(strcmp(what, "de") == 0)
    return TRUE;
  if(strcmp(what, "hl") == 0)
    return TRUE;
  if(strcmp(what, "sp") == 0)
    return TRUE;
  if(strcmp(what, "ix") == 0)
    return TRUE;
  if(strcmp(what, "iy") == 0)
    return TRUE;
  return FALSE;
}

/* Check that what is never read after endPl. */

bool
z80notUsed (const char *what, lineNode *endPl, lineNode *head)
{
  lineNode *pl;
  D(("Checking for %s\n", what));
  if(isRegPair(what))
    {
      char low[2], high[2];
      low[0] = what[1];
      high[0] = what[0];
      low[1] = 0;
      high[1] = 0;
      if(strcmp(what, "iy") == 0)
        {
          if(IY_RESERVED)
            return FALSE;
          return(z80notUsed("iyl", endPl, head) && z80notUsed("iyh", endPl, head));
        }
      if(strcmp(what, "ix") == 0)
        {
          if(IY_RESERVED)
            return FALSE;
          return(z80notUsed("ixl", endPl, head) && z80notUsed("ixh", endPl, head));
        }
      return(z80notUsed(low, endPl, head) && z80notUsed(high, endPl, head));
    }

  if(!isReg(what) && !isUReg(what))
    return FALSE;

  _G.head = head;

  unvisitLines (_G.head);

  pl = endPl->next;
  if (!doTermScan (&pl, what))
    return FALSE;

  return TRUE;
}

bool
z80notUsedFrom (const char *what, const char *label, lineNode *head)
{
  lineNode *cpl;

  for (cpl = head; cpl; cpl = cpl->next)
    {
      if (cpl->isLabel && !strncmp (label, cpl->line, strlen(label)))
        {
          return z80notUsed (what, cpl, head);
        }
    }

  return false;
}

bool
z80canAssign (const char *op1, const char *op2, const char *exotic)
{
  const char *dst, *src;

  // Indexed accesses: One is the indexed one, the other one needs to be a reg or immediate.
  if(exotic)
  {
    if(!strcmp(exotic, "ix") || !strcmp(exotic, "iy"))
    {
      if(isReg(op1))
        return TRUE;
    }
    else if(!strcmp(op2, "ix") || !strcmp(op2, "iy"))
    {
      if(isReg(exotic) || exotic[0] == '#')
        return TRUE;
    }

    return FALSE;
  }

  // Everything else.
  dst = op1;
  src = op2;

  // 8-bit regs can be assigned to each other directly.
  if(isReg(dst) && isReg(src))
    return TRUE;

  // Same if at most one of them is (hl).
  if(isReg(dst) && !strcmp(src, "(hl)"))
    return TRUE;
  if(!strcmp(dst, "(hl)") && isReg(src))
    return TRUE;

  // Can assign between a and (bc), (de)
  if(!strcmp(dst, "a") && (!strcmp(src, "(bc)") || ! strcmp(src, "(de)")))
    return TRUE;
  if((!strcmp(dst, "(bc)") || ! strcmp(dst, "(de)")) && !strcmp(src, "a"))
    return TRUE;

  // Can load immediate values directly into registers and register pairs.
  if((isReg(dst) || isRegPair(dst)) && src[0] == '#')
    return TRUE;

  if((!strcmp(dst, "a") || isRegPair(dst)) && !strncmp(src, "(#", 2))
    return TRUE;
  if(!strncmp(dst, "(#", 2) && (!strcmp(src, "a") || isRegPair(src)))
    return TRUE;

  // Can load immediate values directly into (hl).
  if(!strcmp(dst, "(hl)") && src[0] == '#')
    return TRUE;

  return FALSE;
}

int z80instructionSize(lineNode *pl)
{
  const char *op1start, *op2start;

  /* move to the first operand:
   * leading spaces are already removed, skip the mnenonic */
  for (op1start = pl->line; *op1start && !isspace (*op1start); ++op1start);

  /* skip the spaces between mnemonic and the operand */
  while (isspace (*op1start))
    ++op1start;
  if (!(*op1start))
    op1start = NULL;

  if (op1start)
    {
      /* move to the second operand:
       * find the comma and skip the following spaces */
      op2start = strchr(op1start, ',');
      if (op2start)
        {
          do
            ++op2start;
          while (isspace (*op2start));

          if ('\0' == *op2start)
            op2start = NULL;
        }
    }
  else
    op2start = NULL;

  if(TARGET_IS_TLCS90) // Todo: More accurate estimate.
    return(6);

  /* All ld instructions */
  if(ISINST(pl->line, "ld"))
    {
      /* These 4 are the only cases of 4 byte long ld instructions. */
      if(!STRNCASECMP(op1start, "ix", 2) || !STRNCASECMP(op1start, "iy", 2))
        return(4);
      if((argCont(op1start, "(ix)") || argCont(op1start, "(iy)")) && op2start[0] == '#')
        return(4);

      if(op1start[0] == '('               && STRNCASECMP(op1start, "(bc)", 4) &&
         STRNCASECMP(op1start, "(de)", 4) && STRNCASECMP(op1start, "(hl)", 4) &&
         STRNCASECMP(op2start, "hl", 2)   && STRNCASECMP(op2start, "a", 1) ||
         op2start[0] == '('               && STRNCASECMP(op2start, "(bc)", 4) &&
         STRNCASECMP(op1start, "(de)", 4) && STRNCASECMP(op2start, "(hl)", 4) &&
         STRNCASECMP(op1start, "hl", 2)   && STRNCASECMP(op1start, "a", 1))
        return(4);

      /* Rabbit 16-bit pointer load */
      if(IS_RAB && !STRNCASECMP(op1start, "hl", 2) && (argCont(op2start, "(hl)") || argCont(op2start, "(iy)")))
        return(4);
      if(IS_RAB && !STRNCASECMP(op1start, "hl", 2) && (argCont(op2start, "(sp)") || argCont(op2start, "(ix)")))
        return(3);

      if(IS_EZ80_Z80 && /* eZ80 16-bit pointer load */
        (!STRNCASECMP(op1start, "bc", 2) || !STRNCASECMP(op1start, "de", 2) || !STRNCASECMP(op1start, "hl", 2) || !STRNCASECMP(op1start, "ix", 2) || !STRNCASECMP(op1start, "iy", 2)))
        {
          if (!STRNCASECMP(op2start, "(hl)", 4))
            return(2);
          if (argCont(op2start, "(ix)") || argCont(op2start, "(iy)"))
            return(3);
        }

      /* These 4 are the only remaining cases of 3 byte long ld instructions. */
      if(argCont(op2start, "(ix)") || argCont(op2start, "(iy)"))
        return(3);
      if(argCont(op1start, "(ix)") || argCont(op1start, "(iy)"))
        return(3);
      if((op1start[0] == '(' && STRNCASECMP(op1start, "(bc)", 4) && STRNCASECMP(op1start, "(de)", 4) && STRNCASECMP(op1start, "(hl)", 4)) ||
         (op2start[0] == '(' && STRNCASECMP(op2start, "(bc)", 4) && STRNCASECMP(op2start, "(de)", 4) && STRNCASECMP(op2start, "(hl)", 4)))
        return(3);
      if(op2start[0] == '#' &&
         (!STRNCASECMP(op1start, "bc", 2) || !STRNCASECMP(op1start, "de", 2) || !STRNCASECMP(op1start, "hl", 2) || !STRNCASECMP(op1start, "sp", 2)))
        return(3);

      /* These 3 are the only remaining cases of 2 byte long ld instructions. */
      if(op2start[0] == '#')
        return(2);
      if(!STRNCASECMP(op1start, "i", 1) || !STRNCASECMP(op1start, "r", 1) ||
         !STRNCASECMP(op2start, "i", 1) || !STRNCASECMP(op2start, "r", 1))
        return(2);
      if(!STRNCASECMP(op2start, "ix", 2) || !STRNCASECMP(op2start, "iy", 2))
        return(2);

      /* All other ld instructions */
      return(1);
    }

  /* Exchange */
  if(ISINST(pl->line, "exx"))
    return(1);
  if(ISINST(pl->line, "ex"))
    {
      if(!op2start)
        {
          werrorfl(pl->ic->filename, pl->ic->lineno, W_UNRECOGNIZED_ASM, __FUNCTION__, 4, pl->line);
          return(4);
        }
      if(argCont(op1start, "(sp)") && (IS_RAB || !STRNCASECMP(op2start, "ix", 2) || !STRNCASECMP(op2start, "iy", 2)))
        return(2);
      return(1);
    }

  /* Push / pop */
  if(ISINST(pl->line, "push") || ISINST(pl->line, "pop"))
    {
      if(!STRNCASECMP(op1start, "ix", 2) || !STRNCASECMP(op1start, "iy", 2))
        return(2);
      return(1);
    }

  /* 16 bit add / subtract / and */
  if((ISINST(pl->line, "add") || ISINST(pl->line, "adc") || ISINST(pl->line, "sbc") || IS_RAB && ISINST(pl->line, "and")) &&
     !STRNCASECMP(op1start, "hl", 2))
    {
      if(ISINST(pl->line, "add") || ISINST(pl->line, "and"))
        return(1);
      return(2);
    }
  if(ISINST(pl->line, "add") && (!STRNCASECMP(op1start, "ix", 2) || !STRNCASECMP(op1start, "iy", 2)))
    return(2);

  /* signed 8 bit adjustment to stack pointer */
  if((IS_RAB || IS_GB) && ISINST(pl->line, "add") && !STRNCASECMP(op1start, "sp", 2))
    return(2);

  /* 16 bit adjustment to stack pointer */
  if(IS_TLCS90 && ISINST(pl->line, "add") && !STRNCASECMP(op1start, "sp", 2))
    return(3);
  
  /* 8 bit arithmetic, two operands */
  if(op2start &&  op1start[0] == 'a' &&
     (ISINST(pl->line, "add") || ISINST(pl->line, "adc") || ISINST(pl->line, "sub") || ISINST(pl->line, "sbc") ||
      ISINST(pl->line, "cp")  || ISINST(pl->line, "and") || ISINST(pl->line, "or")  || ISINST(pl->line, "xor")))
    {
      if(argCont(op2start, "(ix)") || argCont(op2start, "(iy)"))
        return(3);
      if(op2start[0] == '#')
        return(2);
      return(1);
    }

  if(ISINST(pl->line, "rlca") || ISINST(pl->line, "rla") || ISINST(pl->line, "rrca") || ISINST(pl->line, "rra"))
    return(1);

  /* Increment / decrement */
  if(ISINST(pl->line, "inc") || ISINST(pl->line, "dec"))
    {
      if(!STRNCASECMP(op1start, "ix", 2) || !STRNCASECMP(op1start, "iy", 2))
        return(2);
      if(argCont(op1start, "(ix)") || argCont(op1start, "(iy)"))
        return(3);
      return(1);
    }

  if(ISINST(pl->line, "rlc") || ISINST(pl->line, "rl")  || ISINST(pl->line, "rrc") || ISINST(pl->line, "rr") ||
     ISINST(pl->line, "sla") || ISINST(pl->line, "sra") || ISINST(pl->line, "srl"))
    {
      if(argCont(op1start, "(ix)") || argCont(op1start, "(iy)"))
        return(4);
      return(2);
    }

  if(ISINST(pl->line, "rld") || ISINST(pl->line, "rrd"))
    return(2);

  /* Bit */
  if(ISINST(pl->line, "bit") || ISINST(pl->line, "set") || ISINST(pl->line, "res"))
    {
      if(argCont(op2start, "(ix)") || argCont(op2start, "(iy)"))
        return(4);
      return(2);
    }

  if(ISINST(pl->line, "jr") || ISINST(pl->line, "djnz"))
    return(2);

  if(ISINST(pl->line, "jp"))
    {
      if(!STRNCASECMP(op1start, "(hl)", 4))
        return(1);
      if(!STRNCASECMP(op1start, "(ix)", 4) || !STRNCASECMP(op1start, "(iy)", 4))
        return(2);
      return(3);
    }

  if (IS_RAB &&
      (ISINST(pl->line, "ipset3") || ISINST(pl->line, "ipset2") ||
       ISINST(pl->line, "ipset1") || ISINST(pl->line, "ipset0") ||
       ISINST(pl->line, "ipres")))
    return(2);
  
  if(ISINST(pl->line, "reti") || ISINST(pl->line, "retn"))
    return(2);

  if(ISINST(pl->line, "ret") || ISINST(pl->line, "rst"))
    return(1);

  if(ISINST(pl->line, "call"))
    return(3);

  if(ISINST(pl->line, "ldi") || ISINST(pl->line, "ldd") || ISINST(pl->line, "cpi") || ISINST(pl->line, "cpd"))
    return(2);

  if(ISINST(pl->line, "neg"))
    return(2);

  if(ISINST(pl->line, "daa") || ISINST(pl->line, "cpl")  || ISINST(pl->line, "ccf") || ISINST(pl->line, "scf") ||
     ISINST(pl->line, "nop") || ISINST(pl->line, "halt") || ISINST(pl->line,  "ei") || ISINST(pl->line, "di"))
    return(1);

  if(ISINST(pl->line, "im"))
    return(2);

  if(ISINST(pl->line, "in") || ISINST(pl->line, "out") || ISINST(pl->line, "ot"))
    return(2);

  if((IS_Z180 || IS_EZ80_Z80) && (ISINST(pl->line, "in0") || ISINST(pl->line, "out0")))
    return(3);

  if(ISINST(pl->line, "di") || ISINST(pl->line, "ei"))
    return(1);

  if((IS_Z180 || IS_EZ80_Z80) && ISINST(pl->line, "mlt"))
    return(2);

  if((IS_Z180 || IS_EZ80_Z80) && ISINST(pl->line, "tst"))
    return((op1start[0] == '#' || op2start && op1start[0] == '#') ? 3 : 2);
  
  if(IS_RAB && ISINST(pl->line, "mul"))
    return(1);

  if(ISINST(pl->line, "lddr") || ISINST(pl->line, "ldir"))
    return(2);

  if(IS_R3KA &&
    (ISINST(pl->line, "lddsr") || ISINST(pl->line, "ldisr") ||
     ISINST(pl->line, "lsdr")  || ISINST(pl->line, "lsir")  ||
     ISINST(pl->line, "lsddr") || ISINST(pl->line, "lsidr")))
    return(2);
  
  if(IS_R3KA && (ISINST(pl->line, "uma") || ISINST(pl->line, "ums")))
    return(2);

  if(IS_RAB && ISINST(pl->line, "bool"))
    return(!STRNCASECMP(op1start, "hl", 2) ? 1 : 2);

  if(IS_EZ80_Z80 && (ISINST(pl->line, "lea") || ISINST(pl->line, "pea")))
    return(3);

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

  /* If the instruction is unrecognized, we shouldn't try to optimize.  */
  /* For all we know it might be some .ds or similar possibly long line */
  /* Return a large value to discourage optimization.                   */
  if (pl->ic)
    werrorfl(pl->ic->filename, pl->ic->lineno, W_UNRECOGNIZED_ASM, __func__, 999, pl->line);
  else
    werrorfl("unknown", 0, W_UNRECOGNIZED_ASM, __func__, 999, pl->line);
  return(999);
}

bool z80symmParmStack (void)
{
  return z80_symmParm_in_calls_from_current_function;
}


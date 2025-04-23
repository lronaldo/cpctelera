#include "common.h"
#include "SDCCgen.h"

#include "peep.h"

#define NOTUSEDERROR() do {werror(E_INTERNAL_ERROR, __FILE__, __LINE__, "error in notUsed()");} while(0)

// #define D(_s) { printf _s; fflush(stdout); }
#define D(_s)

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
    if (cpl->isLabel &&
      strncmp (p, cpl->line, strlen(p)) == 0 &&
      cpl->line[strlen(p)] == ':')
        return cpl;

  return NULL;
}

static bool
mos6502MightReadFlag(const lineNode *pl, const char *what)
{
  if (ISINST (pl->line, "adc") ||
    ISINST (pl->line, "rol") ||
    ISINST (pl->line, "ror") ||
    ISINST (pl->line, "sbc"))
    return (!strcmp(what, "c"));

  if (ISINST (pl->line, "bcc") || ISINST (pl->line, "bcs"))
    return (!strcmp(what, "c"));

  if (ISINST (pl->line, "beq") || ISINST (pl->line, "bne"))
    return (!strcmp(what, "z"));

  if (ISINST (pl->line, "bmi") || ISINST (pl->line, "bpl"))
    return (!strcmp(what, "n"));

  if (ISINST (pl->line, "bvc") || ISINST (pl->line, "bvs"))
    return (!strcmp(what, "v"));

  return false;
}

static bool
mos6502MightRead(const lineNode *pl, const char *what)
{
  if (!strcmp (what, "n") || !strcmp (what, "z") || !strcmp (what, "c") || !strcmp (what, "v"))
    return (mos6502MightReadFlag (pl, what));

  return true;
}

static bool
mos6502SurelyWritesFlag(const lineNode *pl, const char *what)
{
  if (ISINST (pl->line, "adc") ||
    ISINST (pl->line, "sbc"))
    return (!strcmp(what, "n") || !strcmp(what, "z") || !strcmp(what, "c") || !strcmp(what, "v"));

  if (ISINST (pl->line, "asl") ||
    ISINST (pl->line, "cmp") ||
    ISINST (pl->line, "cpx") ||
    ISINST (pl->line, "cpy") ||
    ISINST (pl->line, "lsr") ||
    ISINST (pl->line, "rol") ||
    ISINST (pl->line, "ror"))
    return (!strcmp(what, "n") || !strcmp(what, "z") || !strcmp(what, "c"));

  if (ISINST (pl->line, "and") ||
    ISINST (pl->line, "dec") ||
    ISINST (pl->line, "dex") ||
    ISINST (pl->line, "dey") ||
    ISINST (pl->line, "eor") ||
    ISINST (pl->line, "inc") ||
    ISINST (pl->line, "inx") ||
    ISINST (pl->line, "iny") ||
    ISINST (pl->line, "lda") ||
    ISINST (pl->line, "ldx") ||
    ISINST (pl->line, "ldy") ||
    ISINST (pl->line, "ora") ||
    ISINST (pl->line, "pla") ||
    ISINST (pl->line, "tax") ||
    ISINST (pl->line, "tay") ||
    ISINST (pl->line, "tsx") ||
    ISINST (pl->line, "tsa") ||
    ISINST (pl->line, "tya"))
    return (!strcmp(what, "n") || !strcmp(what, "z"));

  if (ISINST (pl->line, "bit"))
    return (!strcmp(what, "n") || !strcmp(what, "z") || !strcmp(what, "v"));

  if (ISINST (pl->line, "clc") ||
    ISINST (pl->line, "sec"))
    return (!strcmp(what, "c"));

  if (ISINST (pl->line, "clv"))
    return (!strcmp(what, "v"));

  if (ISINST (pl->line, "plp") ||
    ISINST (pl->line, "rti"))
    return true;
    
  return false;
}

static bool
mos6502SurelyWrites(const lineNode *pl, const char *what)
{
  if (!strcmp (what, "n") || !strcmp (what, "z") || !strcmp (what, "c") || !strcmp (what, "v"))
    return (mos6502SurelyWritesFlag(pl, what));

  return false;
}


static bool
mos6502UncondJump (const lineNode *pl)
{
  return (ISINST (pl->line, "jmp"));
}

static bool
mos6502CondJump (const lineNode *pl)
{
  return (ISINST (pl->line, "bpl") || ISINST (pl->line, "bmi") ||
    ISINST (pl->line, "bvc") || ISINST (pl->line, "bvs") ||
    ISINST (pl->line, "bcc") || ISINST (pl->line, "bcs") ||
    ISINST (pl->line, "bne") || ISINST (pl->line, "beq"));
}

static bool
mos6502SurelyReturns (const lineNode *pl)
{
  return (ISINST (pl->line, "rts") || ISINST (pl->line, "rti") );
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

      if (mos6502MightRead (*pl, what))
        {
          D(("S4O_RD_OP\n"));
          return S4O_RD_OP;
        }

      // Check writes before conditional jumps, some jumps (btjf, btjt) write 'c'
      if (mos6502SurelyWrites (*pl, what))
        {
          D(("S4O_WR_OP\n"));
          return S4O_WR_OP;
        }

      if (mos6502UncondJump (*pl))
        {
          *pl = findLabel (*pl);
            if (!*pl)
              {
                D(("S4O_ABORT at unconditional jump\n"));
                return S4O_ABORT;
              }
        }
      if (mos6502CondJump (*pl))
        {
          *plCond = (*pl)->next->next;
          if (!*plCond)
            {
              D(("S4O_ABORT at conditional jump\n"));
              return S4O_ABORT;
            }
          D(("S4O_CONDJMP\n"));
          return S4O_CONDJMP;
        }

      /* Don't need to check for de, hl since pdkMightRead() does that */
      if (mos6502SurelyReturns (*pl))
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

bool mos6502notUsed (const char *what, lineNode *endPl, lineNode *head)
{
  lineNode *pl;

  _G.head = head;

  unvisitLines (_G.head);

  // Todo: Implement WDC 65C02 support, remove this check!
  if (TARGET_IS_MOS65C02)
    return (false);

  pl = endPl->next;
  return (doTermScan (&pl, what));
}

bool mos6502notUsedFrom (const char *what, const char *label, lineNode *head)
{
  lineNode *cpl;

  for (cpl = head; cpl; cpl = cpl->next)
    if (cpl->isLabel && !strncmp (label, cpl->line, strlen(label)))
      return (mos6502notUsed (what, cpl, head));

  return false;
}


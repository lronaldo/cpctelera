/*-------------------------------------------------------------------------
  peep.c - source file for peephole optimizer helper functions

  Written By -  Bernhard Held

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

#include <ctype.h>
#include "common.h"
#include "ralloc.h"
#include "gen.h"

#define D(x) x
#define DEADMOVEERROR() do {werror(E_INTERNAL_ERROR, __FILE__, __LINE__, "error in deadmove");} while(0)

typedef enum
{
  S4O_FOUNDOPCODE,
  S4O_PUSHPOP,
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
/* univisitLines - clear "visited" flag in all lines               */
/*-----------------------------------------------------------------*/
static void
unvisitLines (lineNode *pl)
{
  for (; pl; pl = pl->next)
    pl->visited = FALSE;
}

/*-----------------------------------------------------------------*/
/* cleanLabelRef - clear label jump-counter and pass-flag          */
/*-----------------------------------------------------------------*/
static void
cleanLabelRef (void)
{
  int key;
  labelHashEntry *entry;

  if (!labelHash)
    return;
  for (entry = (labelHashEntry *) hTabFirstItem (labelHash, &key);
       entry;
       entry = (labelHashEntry *) hTabNextItem (labelHash, &key))
    {
      entry->passedLabel = FALSE;
      entry->jmpToCount = 0;
    }
}

/*-----------------------------------------------------------------*/
/* checkLabelRef - check all entries in labelHash                  */
/* The path from 'pop' to 'push' must be the only possible path.   */
/* There must not be any paths in or out of this path.             */
/* This is checked by counting the label references.               */
/*-----------------------------------------------------------------*/
static bool
checkLabelRef (void)
{
  int key;
  labelHashEntry *entry;

  if (!labelHash)
    {
      /* no labels at all: no problems ;-) */
      return TRUE;
    }

  for (entry = (labelHashEntry *) hTabFirstItem (labelHash, &key);
       entry;
       entry = (labelHashEntry *) hTabNextItem (labelHash, &key))
    {

      /* In our path we passed a label,
         but we didn't meet all references (jumps) to this label.
         This means that the code jumps from outside into this path. */
      if (entry->passedLabel &&
          entry->jmpToCount != entry->refCount)
        {
          return FALSE;
        }

      /* In our path we jumped to (referenced) a label,
         but we we didn't pass it.
         This means that there's a code path into our path. */
      if (!entry->passedLabel &&
          entry->jmpToCount != 0)
        {
          return FALSE;
        }
    }
  return TRUE;
}

/*-----------------------------------------------------------------*/
/* setLabelRefPassedLabel - set flag "passedLabel" in entry        */
/* of the list labelHash                                           */
/*-----------------------------------------------------------------*/
static bool
setLabelRefPassedLabel (const char *label)
{
  labelHashEntry *entry;

  entry = getLabelRef (label, _G.head);
  if (!entry)
    return FALSE;
  entry->passedLabel = TRUE;
  return TRUE;
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
      DEADMOVEERROR();
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
          && strcmp (p, cpl->line) == 0)
        {
          return cpl;
        }
    }
  return NULL;
}

/*-----------------------------------------------------------------*/
/* isFunc - returns TRUE if it's a CALL or PCALL (not _gptrget())  */
/*-----------------------------------------------------------------*/
static bool
isFunc (const lineNode *pl)
{
  if (pl && pl->ic)
    {
      if (   pl->ic->op == CALL
          || pl->ic->op == PCALL)
        return TRUE;
    }
  return FALSE;
}

/*-----------------------------------------------------------------*/
/* termScanAtFunc - returns S4O_TERM if it's a 'normal' function   */
/* call and it's a 'caller save'. returns S4O_CONTINUE if it's     */
/* 'callee save' or 'naked'. returns S4O_ABORT if it's 'banked'    */
/* and uses the register for the destination.                      */
/*-----------------------------------------------------------------*/
static S4O_RET
termScanAtFunc (const lineNode *pl, int rIdx)
{
  sym_link *ftype;
  bool banked_reg = (rIdx == R0_IDX) || (rIdx == R1_IDX) || (rIdx == R2_IDX);

  if (!isFunc (pl))
    return S4O_CONTINUE;
  // let's assume calls to literally given locations use the default
  // most notably :  (*(void (*)()) 0) ();  see bug 1749275
  if (IS_VALOP (IC_LEFT (pl->ic)))
    return (options.model == MODEL_HUGE) && banked_reg ? S4O_ABORT : options.all_callee_saves ? S4O_CONTINUE : S4O_TERM;
  ftype = OP_SYM_TYPE(IC_LEFT(pl->ic));
  if (IS_FUNCPTR (ftype))
    ftype = ftype->next;
  if (IFFUNC_ISBANKEDCALL(ftype) && banked_reg)
    return S4O_ABORT;
  if (FUNC_ARGS (ftype) && getSize (FUNC_ARGS (ftype)->type) > 4)
    return S4O_ABORT;
  if (FUNC_CALLEESAVES(ftype))
    return S4O_CONTINUE;
  if (FUNC_ISNAKED(ftype))
    return S4O_CONTINUE;
  return S4O_TERM;
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
/*    S4O_PUSHPOP                                                  */
/*       hit a "push" or "pop" opcode                              */
/*    S4O_CONDJMP                                                  */
/*       hit a conditional jump opcode. pl and plCond return the   */
/*       two possible branches.                                    */
/*    S4O_TERM                                                     */
/*       acall, lcall, ret and reti "terminate" a scan.            */
/*-----------------------------------------------------------------*/
static S4O_RET
scan4op (lineNode **pl, const char *pReg, const char *untilOp,
         lineNode **plCond)
{
  char *p;
  int len;
  bool isConditionalJump;
  int rIdx;
  S4O_RET ret;
  bool findPushPop;

  findPushPop = untilOp && (strcmp (untilOp, "push") == 0 || strcmp (untilOp, "pop") == 0);

  /* pReg points to e.g. "ar0"..."ar7" */
  len = strlen (pReg);

  /* get index into pReg table */
  for (rIdx = 0; rIdx < mcs51_nRegs; ++rIdx)
    if (strcmp (regs8051[rIdx].name, pReg + 1) == 0)
      break;

  /* sanity check */
  if (rIdx >= mcs51_nRegs)
    {
      DEADMOVEERROR();
      return S4O_ABORT;
    }

  for (; *pl; *pl = (*pl)->next)
    {
      if (!(*pl)->line || (*pl)->isDebug || (*pl)->isComment)
        continue;

      /* don't optimize across inline assembler,
         e.g. isLabel doesn't work there */
      if ((*pl)->isInline)
        return S4O_ABORT;

      if ((*pl)->visited)
        return S4O_VISITED;
      (*pl)->visited = TRUE;

      /* found untilOp? */
      if (untilOp && strncmp ((*pl)->line, untilOp, strlen (untilOp)) == 0)
        {
          p = (*pl)->line + strlen (untilOp);
          if (*p == '\t' && strncmp (p + 1, pReg, len) == 0)
            return S4O_FOUNDOPCODE;
          else
            {
              /* found untilOp but without our pReg */
              return S4O_ABORT;
            }
        }

      /* found pReg? */
      p = strchr ((*pl)->line, '\t');
      if (p)
        {
          /* skip '\t' */
          p++;

          /* when looking for push or pop and we find a direct access of sp: abort */
          if (findPushPop && strstr (p, "sp"))
            return S4O_ABORT;

          /* course search */
          if (strstr (p, pReg + 1))
            {
              /* ok, let's have a closer look */

              /* does opcode read from pReg? */
              if (bitVectBitValue (port->peep.getRegsRead ((*pl)), rIdx))
                return S4O_RD_OP;
              /* does opcode write to pReg? */
              if (bitVectBitValue (port->peep.getRegsWritten ((*pl)), rIdx))
                return S4O_WR_OP;

              /* we can get here, if the register name is
                 part of a variable name: ignore it */
            }
        }

      /* found label? */
      if ((*pl)->isLabel)
        {
          const char *start;
          char label[SDCC_NAME_MAX + 1];
          int len;

          if (!isLabelDefinition ((*pl)->line, &start, &len, FALSE))
            return S4O_ABORT;
          memcpy (label, start, len);
          label[len] = '\0';
          /* register passing this label */
          if (!setLabelRefPassedLabel (label))
            {
              DEADMOVEERROR();
              return S4O_ABORT;
            }
          continue;
        }

      /* branch or terminate? */
      isConditionalJump = FALSE;
      switch ((*pl)->line[0])
        {
          case 'a':
            if (strncmp ("acall", (*pl)->line, 5) == 0)
              {
                /* for comments see 'lcall' */
                ret = termScanAtFunc (*pl, rIdx);
                if (ret != S4O_CONTINUE)
                  return ret;
                break;
              }
            if (strncmp ("ajmp", (*pl)->line, 4) == 0)
              {
                *pl = findLabel (*pl);
                if (!*pl)
                  return S4O_ABORT;
              }
            break;
          case 'c':
            if (strncmp ("cjne", (*pl)->line, 4) == 0)
              {
                isConditionalJump = TRUE;
                break;
              }
            break;
          case 'd':
            if (strncmp ("djnz", (*pl)->line, 4) == 0)
              {
                isConditionalJump = TRUE;
                break;
              }
            break;
          case 'j':
            if (strncmp ("jmp", (*pl)->line, 3) == 0)
              /* "jmp @a+dptr": no chance to trace execution */
              return S4O_ABORT;
            if (strncmp ("jc",  (*pl)->line, 2) == 0 ||
                strncmp ("jnc", (*pl)->line, 3) == 0 ||
                strncmp ("jz",  (*pl)->line, 2) == 0 ||
                strncmp ("jnz", (*pl)->line, 3) == 0)
              {
                isConditionalJump = TRUE;
                break;
              }
            if (strncmp ("jbc", (*pl)->line, 3) == 0 ||
                strncmp ("jb",  (*pl)->line, 2) == 0 ||
                strncmp ("jnb", (*pl)->line, 3) == 0)
              {
                isConditionalJump = TRUE;
                break;
              }
            break;
          case 'l':
            if (strncmp ("lcall", (*pl)->line, 5) == 0)
              {
                const char *p = (*pl)->line+5;
                while (*p == ' ' || *p == '\t')
                  p++;
                while (isdigit (*p))
                  p++;
                if (isdigit(p[-1]) && *p == '$') /* at least one digit */
                  {
                    /* this is a temp label for a pcall */
                    *pl = findLabel (*pl);
                    if (!*pl)
                      return S4O_ABORT;
                    break;
                  }

                ret = termScanAtFunc (*pl, rIdx);
                /* If it's a 'normal' 'caller save' function call, all
                   registers have been saved until the 'lcall'. The
                   'life range' of all registers end at the lcall,
                   and we can terminate our search.
                 * If the function is 'banked', the registers r0, r1 and r2
                   are used to tell the trampoline the destination. After
                   that their 'life range' ends just like the other registers.
                 * If it's a 'callee save' function call, registers are saved
                   by the callee. We've got no information, if the register
                   might live beyond the lcall. Therefore we've to continue
                   the search.
                */
                if (ret != S4O_CONTINUE)
                  return ret;
                break;
              }
            if (strncmp ("ljmp", (*pl)->line, 4) == 0)
              {
                *pl = findLabel (*pl);
                if (!*pl)
                  return S4O_ABORT;
              }
            break;
          case 'p':
            if (strncmp ("pop", (*pl)->line, 3) == 0 ||
                strncmp ("push", (*pl)->line, 4) == 0)
              return S4O_PUSHPOP;
            break;
          case 'r':
            if (strncmp ("reti", (*pl)->line, 4) == 0)
              return S4O_TERM;

            if (strncmp ("ret", (*pl)->line, 3) == 0)
              {
                /* pcall uses 'ret' */
                if (isFunc (*pl))
                  {
                    /* for comments see 'lcall' */
                    ret = termScanAtFunc (*pl, rIdx);
                    if (ret != S4O_CONTINUE)
                      return ret;
                    break;
                  }

                /* it's a normal function return */
                if (!((*pl)->ic))
                  return S4O_ABORT; /* but no ic? */
                if (!currFunc->type)
                  return S4O_ABORT;  /* not a function? */
                if (FUNC_CALLEESAVES (currFunc->type))
                  return S4O_ABORT; /* returning from callee saves function */
                if (getSize(currFunc->etype) > 4)
                  {
                    for (unsigned i = 0; i < getSize(currFunc->etype); i++)
                      if (strstr (pReg, fReturn8051[i]))
                        return S4O_ABORT; /* return value is partially in r4-r7 */
                  }
                return S4O_TERM;
              }
            break;
          case 's':
            if (strncmp ("sjmp", (*pl)->line, 4) == 0)
              {
                *pl = findLabel (*pl);
                if (!*pl)
                  return S4O_ABORT;
              }
            break;
          default:
            break;
        } /* switch ((*pl)->line[0]) */

      if (isConditionalJump)
        {
          *plCond = findLabel (*pl);
          if (!*plCond)
            return S4O_ABORT;
          return S4O_CONDJMP;
        }
    } /* for (; *pl; *pl = (*pl)->next) */
  return S4O_ABORT;
}

/*-----------------------------------------------------------------*/
/* doPushScan - scan through area 1. This small wrapper handles:   */
/* - action required on different return values                    */
/* - recursion in case of conditional branches                     */
/*-----------------------------------------------------------------*/
static bool
doPushScan (lineNode **pl, const char *pReg)
{
  lineNode *plConditional, *pushPl = NULL;

  for (;; *pl = (*pl)->next)
    {
      switch (scan4op (pl, pReg, "push", &plConditional))
        {
          case S4O_FOUNDOPCODE:
            /* this is what we're looking for */
            return TRUE;
          case S4O_VISITED:
            if (!pushPl)
              {
                DEADMOVEERROR();
                return FALSE;
              }
            *pl = pushPl;
            /* already checked */
            return TRUE;
          case S4O_CONDJMP:
            /* two possible destinations: recurse */
              {
                lineNode *pushPl2 = plConditional;

                if (!doPushScan (&pushPl2, pReg))
                  return FALSE;
                pushPl = pushPl2;
              }
            continue;
          default:
            return FALSE;
        }
    }
}

/*-----------------------------------------------------------------*/
/* doTermScan - scan through area 2. This small wrapper handles:   */
/* - action required on different return values                    */
/* - recursion in case of conditional branches                     */
/*-----------------------------------------------------------------*/
static bool
doTermScan (lineNode **pl, const char *pReg)
{
  lineNode *plConditional;

  for (;; *pl = (*pl)->next)
    {
      switch (scan4op (pl, pReg, NULL, &plConditional))
        {
          case S4O_TERM:
          case S4O_VISITED:
          case S4O_WR_OP:
            /* all these are terminating conditions */
            return TRUE;
          case S4O_PUSHPOP:
            /* don't care, go on */
            continue;
          case S4O_CONDJMP:
            /* two possible destinations: recurse */
              {
                lineNode *pl2 = plConditional;

                if (!doTermScan (&pl2, pReg))
                  return FALSE;
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
/* removeDeadPopPush - remove pop/push pair if possible            */
/*-----------------------------------------------------------------*/
static bool
removeDeadPopPush (const char *pReg, lineNode *currPl, lineNode *head)
{
  lineNode *pushPl, *pl;

  /* A pop/push pair can be removed, if these criteria are met
     (ar0 is just an example here, ar0...ar7 are possible):

     pop ar0

      ; area 1

      ; There must not be in area 1:
      ;    - read or write access of ar0
      ;    - "acall", "lcall", "pop", "ret", "reti" or "jmp @a+dptr" opcodes
      ;    - "push" opcode, which doesn't push ar0 
      ;    - inline assembly
      ;    - a jump in or out of area 1 (see checkLabelRef())

      ; area 1 must be terminated by a:
     push ar0

      ; area 2

      ; There must not be:
      ;    - read access of ar0
      ;    - "jmp @a+dptr" opcode
      ;    - inline assembly
      ;    - a jump in or out of area 2 (see checkLabelRef())

      ; An "acall", "lcall" (not callee save), "ret" (not PCALL with
      ; callee save), "reti" or write access of r0 terminate
      ; the search, and the "pop/push ar0" can safely be removed.
  */

  /* area 1 */
  pushPl = currPl->next;
  if (!doPushScan (&pushPl, pReg))
    return FALSE;

  if (!checkLabelRef())
    return FALSE;

  /* area 2 */
  pl = pushPl->next;
  if (!doTermScan (&pl, pReg))
    return FALSE;
  if (!checkLabelRef())
    return FALSE;

  /* Success! */
  if (options.noPeepComments)
    {
      /* remove pushPl from list */
      pushPl->prev->next = pushPl->next;
      pushPl->next->prev = pushPl->prev;
    }
  else
    {
      /* replace 'push ar0' by comment */
      #define STR ";\tPeephole\tpush %s removed"
      int size = sizeof(STR) + 2;

      pushPl->line = Safe_alloc (size);
      SNPRINTF (pushPl->line, size, STR, pReg);
      pushPl->isComment = TRUE;
    }

  /* 'pop ar0' will be removed by peephole framework after returning TRUE */
  return TRUE;
}

/*-----------------------------------------------------------------*/
/* removeDeadMove - remove superflous 'mov r%1,%2'                 */
/*-----------------------------------------------------------------*/
static bool
removeDeadMove (const char *pReg, lineNode *currPl)
{
  lineNode *pl;

  /* "mov r0,a" can be removed, if these criteria are met
     (r0 is just an example here, r0...r7 are possible):

      ; There must not be:
      ;    - read access of r0
      ;    - "jmp @a+dptr" opcode
      ;    - inline assembly
      ;    - a jump in or out of this area (see checkLabelRef())

      ; An "acall", "lcall" (not callee save), "ret" (not PCALL with
      ; callee save), "reti" or write access of r0 terminate
      ; the search, and the "mov r0,a" can safely be removed.
  */
  pl = currPl->next;
  if (!doTermScan (&pl, pReg))
    return FALSE;

  if (!checkLabelRef())
    return FALSE;

  return TRUE;
}

/*-----------------------------------------------------------------*/
/* mcs51DeadMove - dispatch condition deadmove between             */
/* - remove pop/push                                               */
/* - remove mov r%1,%2                                             */
/*-----------------------------------------------------------------*/
bool
mcs51DeadMove (const char *reg, lineNode *currPl, lineNode *head)
{
  char pReg[5] = "ar";

  _G.head = head;
  strcat (pReg, reg);

  unvisitLines (_G.head);
  cleanLabelRef();

  if (strncmp (currPl->line, "pop", 3) == 0)
    return removeDeadPopPush (pReg, currPl, head);
  else if (   strncmp (currPl->line, "mov", 3) == 0
           && (currPl->line[3] == ' ' || currPl->line[3] == '\t'))
    return removeDeadMove (pReg, currPl);
  else
    {
      fprintf (stderr, "Error: "
                       "peephole rule with condition deadMove "
                       "used with unknown opocde:\n"
                       "\t%s\n", currPl->line);
      return FALSE;
    }
}

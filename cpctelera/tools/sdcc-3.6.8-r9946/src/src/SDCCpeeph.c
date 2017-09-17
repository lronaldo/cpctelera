/*-------------------------------------------------------------------------
  SDCCpeeph.c - The peep hole optimizer: for interpreting the
                peep hole rules

  Copyright (C) 1999, Sandeep Dutta . sandeep.dutta@usa.net

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
-------------------------------------------------------------------------*/

#include "common.h"
#include "dbuf_string.h"

#define ISCHARDIGIT(c) isdigit((unsigned char)c)
#define ISCHARSPACE(c) isspace((unsigned char)c)
#define ISCHARALNUM(c) isalnum((unsigned char)c)

static peepRule *rootRules = NULL;
static peepRule *currRule = NULL;

#define HTAB_SIZE 53

hTab *labelHash = NULL;

static struct
{
  allocTrace values;
  allocTrace labels;
} _G;

static int hashSymbolName (const char *name);
static void buildLabelRefCountHash (lineNode * head);
static void bindVar (int key, char **s, hTab ** vtab);

static bool matchLine (char *, const char *, hTab **);

#define FBYNAME(x) static int x (hTab *vars, lineNode *currPl, lineNode *endPl, \
        lineNode *head, char *cmdLine)

#if !OPT_DISABLE_PIC14
void peepRules2pCode(peepRule *);
#endif

#if !OPT_DISABLE_PIC16
void pic16_peepRules2pCode(peepRule *);
#endif

/*-----------------------------------------------------------------*/
/* getPatternVar - finds a pattern variable                        */
/*-----------------------------------------------------------------*/

static char*
getPatternVar (hTab *vars, char **cmdLine)
{
  int varNumber;
  char *digitend;

  if (!cmdLine || !*cmdLine || !**cmdLine)
    return NULL; /* no parameters given */

  while (**cmdLine && ISCHARSPACE(**cmdLine))
    (*cmdLine)++; /* skip whitespace */

  if (**cmdLine != '%')
    goto error;
  (*cmdLine)++;
  if (!ISCHARDIGIT (**cmdLine))
    goto error;
  varNumber = strtol (*cmdLine, &digitend, 10);
  *cmdLine = digitend;
  return hTabItemWithKey (vars, varNumber);

error:
  fprintf (stderr,
           "*** internal error: peephole restriction malformed: %s\n", *cmdLine);
  return NULL;
}

/*-----------------------------------------------------------------*/
/* pcDistance - finds a label backward or forward                  */
/*-----------------------------------------------------------------*/

static int
pcDistance (lineNode *cpos, char *lbl, bool back)
{
  lineNode *pl = cpos;
  char buff[MAX_PATTERN_LEN];
  int dist = 0;

  SNPRINTF (buff, sizeof(buff) - 1, "%s:", lbl);
  while (pl)
    {
      if (pl->line &&
          !pl->isComment &&
          !pl->isLabel &&
          !pl->isDebug)
        {
          if (port->peep.getSize)
            {
              dist += port->peep.getSize(pl);
#if 0
              fprintf(stderr, "Line: %s, dist: %i, total: %i\n", pl->line, port->peep.getSize(pl), dist);
#endif
            }
          else
            {
              dist += 4;    // maximum instruction size
            }
        }

      if (strncmp (pl->line, buff, strlen (buff)) == 0)
        return dist;

      if (back)
        pl = pl->prev;
      else
        pl = pl->next;

    }
  return 0;
}

/*-----------------------------------------------------------------*/
/* portIsDS390 - return true if port is DS390                      */
/*-----------------------------------------------------------------*/
FBYNAME (portIsDS390)
{
    return ((strcmp(port->target,"ds390") == 0) ||
            (strcmp(port->target,"ds400") == 0));
}

/*-----------------------------------------------------------------*/
/* flat24bitMode - will check to see if we are in flat24 mode      */
/*-----------------------------------------------------------------*/
FBYNAME (flat24bitMode)
{
  return (options.model == MODEL_FLAT24);
}

/*-----------------------------------------------------------------*/
/* xramMovcOption - check if using movc to read xram               */
/*-----------------------------------------------------------------*/
FBYNAME (xramMovcOption)
{
  return (options.xram_movc && (strcmp(port->target,"mcs51") == 0));
}

/*-----------------------------------------------------------------*/
/* useAcallAjmp - Enable replacement of lcall/ljmp with acall/ajmp */
/*-----------------------------------------------------------------*/
FBYNAME (useAcallAjmp)
{
  return (options.acall_ajmp && (strcmp(port->target,"mcs51") == 0));
}

/*-----------------------------------------------------------------*/
/* labelInRange - will check to see if label is within range       */
/*-----------------------------------------------------------------*/
FBYNAME (labelInRange)
{
  int dist = 0;
  char *lbl = getPatternVar (vars, &cmdLine);

  if (!lbl)
    {
      fprintf (stderr,
             "*** internal error: labelInRange peephole restriction"
             " malformed: %s\n", cmdLine);

      /* If no parameters given, assume that %5 pattern variable
         has the label name for backward compatibility */
      lbl = hTabItemWithKey (vars, 5);
    }

  if (!lbl)
    return FALSE;

  do
    {
      /* Don't optimize jumps in a jump table; a more generic test */
      if (currPl->ic && currPl->ic->op == JUMPTABLE)
        return FALSE;

      /* if the previous two instructions are "ljmp"s then don't
         do it since it can be part of a jump table */
      if (currPl->prev && currPl->prev->prev &&
          strstr (currPl->prev->line, "ljmp") &&
          strstr (currPl->prev->prev->line, "ljmp"))
        return FALSE;

      /* Calculate the label distance. For mcs51 the jump can be
         -127 to + 127 bytes, for Z80 -126 to +129 bytes.*/
      dist = (pcDistance (currPl, lbl, TRUE) +
              pcDistance (currPl, lbl, FALSE));

      /* Use 125 for now. Could be made more exact using port and
         exact jump location instead of currPl. */
      if (!dist || dist > 127)
        return FALSE;

      lbl = getPatternVar (vars, &cmdLine);
    }
  while (lbl);

  return TRUE;
}

/*-----------------------------------------------------------------*/
/* labelJTInRange - will check to see if label %5 and up are       */
/* within range.                                                   */
/* Specifically meant to optimize long (3-byte) jumps to short     */
/* (2-byte) jumps in jumptables                                    */
/*-----------------------------------------------------------------*/
FBYNAME (labelJTInRange)
{
  char *lbl;
  int dist, count, i;

  /* Only optimize within a jump table */
  if (currPl->ic && currPl->ic->op != JUMPTABLE)
    return FALSE;

  count = elementsInSet( IC_JTLABELS (currPl->ic) );

  /* check all labels (this is needed if the case statements are unsorted) */
  for (i=0; i<count; i++)
    {
      /* assumes that the %5 pattern variable has the first ljmp label */
      lbl = hTabItemWithKey (vars, 5+i);
      if (!lbl)
        return FALSE;

      dist = (pcDistance (currPl, lbl, TRUE) +
              pcDistance (currPl, lbl, FALSE));

      /* three terms used to calculate allowable distance */
      /* Could be made more exact and port-specific. */
      if (!dist ||
          dist > 127+           /* range of sjmp */
                 (3+3*i)+       /* offset between this jump and currPl,
                                   should use pcDistance instead? */
                 (count-i-1)    /* if peephole applies distance is shortened */
         )
        return FALSE;
    }
  return TRUE;
}

/*-----------------------------------------------------------------*/
/* optimizeReturn - is it allowed to optimize RET instructions     */
/*-----------------------------------------------------------------*/
FBYNAME (optimizeReturn)
{
  return (options.peepReturn >= 0);
}

/*-----------------------------------------------------------------*/
/* labelIsReturnOnly - Check if label is followed by ret           */
/*-----------------------------------------------------------------*/
FBYNAME (labelIsReturnOnly)
{
  /* assumes that %5 pattern variable has the label name */
  const char *label, *p;
  const lineNode *pl;
  int len;
  char * retInst;

  /* Don't optimize jumps in a jump table; a more generic test */
  if (currPl->ic && currPl->ic->op == JUMPTABLE)
    return FALSE;

  if (!(label = getPatternVar (vars, &cmdLine)))
    {
      fprintf (stderr,
             "*** internal error: labelIsReturnOnly peephole restriction"
             " malformed: %s\n", cmdLine);

      /* If no parameters given, assume that %5 pattern variable
         has the label name for backward compatibility */
      label = hTabItemWithKey (vars, 5);
    }

  if (!label)
    return FALSE;
  len = strlen(label);

  for(pl = currPl; pl; pl = pl->next)
    {
      if (pl->line && !pl->isDebug && !pl->isComment && pl->isLabel)
        {
          if (strncmp(pl->line, label, len) == 0)
            break; /* Found Label */
          if (strlen(pl->line) != 7       || !ISCHARDIGIT(*(pl->line))   ||
              !ISCHARDIGIT(*(pl->line+1)) || !ISCHARDIGIT(*(pl->line+2)) ||
              !ISCHARDIGIT(*(pl->line+3)) || !ISCHARDIGIT(*(pl->line+4)) ||
              *(pl->line+5) != '$')
            {
              return FALSE; /* non-local label encountered */
            }
        }
    }
  if (!pl)
    return FALSE; /* did not find the label */
  pl = pl->next;
  while (pl && (pl->isDebug || pl->isComment || pl->isLabel))
    pl = pl->next;
  if (!pl || !pl->line || pl->isDebug)
    return FALSE; /* next line not valid */
  p = pl->line;
  for (p = pl->line; *p && ISCHARSPACE(*p); p++)
    ;

  retInst = "ret";
  if (TARGET_HC08_LIKE)
    retInst = "rts";
  if (strcmp(p, retInst) == 0)
    return TRUE;
  return FALSE;
}

/*-----------------------------------------------------------------*/
/* labelIsUncondJump - Check if label %5 is followed by an         */
/* unconditional jump and put the destination of that jump in %6   */
/*-----------------------------------------------------------------*/
FBYNAME (labelIsUncondJump)
{
  /* assumes that %5 pattern variable has the label name */
  const char *label;
  char *p, *q;
  const lineNode *pl;
  bool found = FALSE;
  int len;
  char * jpInst = NULL;
  char * jpInst2 = NULL;

  label = hTabItemWithKey (vars, 5);
  if (!label)
    return FALSE;
  len = strlen(label);

  for (pl = currPl; pl; pl = pl->prev)
    {
      if (pl->line && !pl->isDebug && !pl->isComment && pl->isLabel)
        {
          if (strncmp(pl->line, label, len) == 0)
            {
              found = TRUE;
              break; /* Found Label */
            }
          if (strlen(pl->line) != 7       || !ISCHARDIGIT(*(pl->line))   ||
              !ISCHARDIGIT(*(pl->line+1)) || !ISCHARDIGIT(*(pl->line+2)) ||
              !ISCHARDIGIT(*(pl->line+3)) || !ISCHARDIGIT(*(pl->line+4)) ||
              *(pl->line+5) != '$')
            {
              break; /* non-local label encountered */
            }
        }
    }

  if (!found)
    {
      for (pl = currPl; pl; pl = pl->next)
        {
          if (pl->line && !pl->isDebug && !pl->isComment && pl->isLabel)
            {
              if (strncmp(pl->line, label, len) == 0)
                {
                  found = TRUE;
                  break; /* Found Label */
                }
              if (strlen(pl->line) != 7       || !ISCHARDIGIT(*(pl->line))   ||
                  !ISCHARDIGIT(*(pl->line+1)) || !ISCHARDIGIT(*(pl->line+2)) ||
                  !ISCHARDIGIT(*(pl->line+3)) || !ISCHARDIGIT(*(pl->line+4)) ||
                  *(pl->line+5) != '$')
                {
                  return FALSE; /* non-local label encountered */
                }
            }
        }
    }

  if (!pl || !found)
    return FALSE; /* did not find the label */
  pl = pl->next;
  while (pl && (pl->isDebug || pl->isComment))
    pl = pl->next;
  if (!pl || !pl->line)
    return FALSE; /* next line not valid */
  p = pl->line;
  while (*p && ISCHARSPACE(*p))
    p++;

  if (TARGET_MCS51_LIKE)
    {
      jpInst = "ljmp";
      jpInst2 = "sjmp";
    }
  else if (TARGET_HC08_LIKE)
    {
      jpInst = "jmp";
      jpInst2 = "bra";
    }
  else if (TARGET_Z80_LIKE)
    {
      jpInst = "jp";
      jpInst2 = "jr";
    }
  else if (TARGET_IS_STM8)
    {
      jpInst = "jp";
      jpInst2 = "jra";
    }
  len = strlen(jpInst);
  if (strncmp(p, jpInst, len))
    {
      len = jpInst2 ? strlen(jpInst2) : 0;
      if(!jpInst2 || strncmp(p, jpInst2, len))
        return FALSE; /* next line is no jump */
    }

  p += len;
  while (*p && ISCHARSPACE(*p))
    p++;

  q = p;
  while (*q && *q!=';')
    q++;
  while (q>p && ISCHARSPACE(*q))
    q--;
  len = q-p;
  if (len == 0)
    return FALSE; /* no destination? */

  if (TARGET_Z80_LIKE)
    {
      while (q>p && *q!=',')
        q--;
      if (*q==',')
        return FALSE; /* conditional jump */
    }

  /* now put the destination in %6 */
  bindVar (6, &p, &vars);

  return TRUE;
}

/*-----------------------------------------------------------------*/
/* okToRemoveSLOC - Check if label %1 is a SLOC and not other      */
/* usage of it in the code depends on a value from this section    */
/*-----------------------------------------------------------------*/
FBYNAME (okToRemoveSLOC)
{
  const lineNode *pl;
  const char *sloc, *p;
  int dummy1, dummy2, dummy3;

  /* assumes that %1 as the SLOC name */
  sloc = hTabItemWithKey (vars, 1);
  if (sloc == NULL) return FALSE;
  p = strstr(sloc, "sloc");
  if (p == NULL) return FALSE;
  p += 4;
  if (sscanf(p, "%d_%d_%d", &dummy1, &dummy2, &dummy3) != 3) return FALSE;
  /*TODO: ultra-paranoid: get funtion name from "head" and check that */
  /* the sloc name begins with that.  Probably not really necessary */

  /* Look for any occurance of this SLOC before the peephole match */
  for (pl = currPl->prev; pl; pl = pl->prev) {
        if (pl->line && !pl->isDebug && !pl->isComment
          && *pl->line != ';' && strstr(pl->line, sloc))
                return FALSE;
  }
  /* Look for any occurance of this SLOC after the peephole match */
  for (pl = endPl->next; pl; pl = pl->next) {
        if (pl->line && !pl->isDebug && !pl->isComment
          && *pl->line != ';' && strstr(pl->line, sloc))
                return FALSE;
  }
  return TRUE; /* safe for a peephole to remove it :) */
}

/*-----------------------------------------------------------------*/
/* deadMove - Check, if a pop/push pair can be removed             */
/*-----------------------------------------------------------------*/
FBYNAME (deadMove)
{
  const char *reg = hTabItemWithKey (vars, 1);

  if (port->peep.deadMove)
    return port->peep.deadMove (reg, currPl, head);

  fprintf (stderr, "Function deadMove not initialized in port structure\n");
  return FALSE;
}

/*-----------------------------------------------------------------*/
/* labelHashEntry- searches for a label in the list labelHash      */
/* Builds labelHash, if it does not yet exist.                     */
/* Returns the labelHashEntry or NULL                              */
/*-----------------------------------------------------------------*/
labelHashEntry *
getLabelRef (const char *label, lineNode *head)
{
  labelHashEntry *entry;

  /* If we don't have the label hash table yet, build it. */
  if (!labelHash)
    {
      buildLabelRefCountHash (head);
    }

  entry = hTabFirstItemWK (labelHash, hashSymbolName (label));

  while (entry)
    {
      if (!strcmp (label, entry->name))
        {
          break;
        }
      entry = hTabNextItemWK (labelHash);
    }
  return entry;
}

/* labelRefCount:

 * takes two parameters: a variable (bound to a label name)
 * and an expected reference count.
 *
 * Returns TRUE if that label is defined and referenced exactly
 * the given number of times.
 */
FBYNAME (labelRefCount)
{
  int varNumber, expectedRefCount;
  bool rc = FALSE;

  if (sscanf (cmdLine, "%*[ \t%]%d %d", &varNumber, &expectedRefCount) == 2)
    {
      char *label = hTabItemWithKey (vars, varNumber);

      if (label)
        {
          labelHashEntry *entry = getLabelRef (label, head);

          if (entry)
            {
#if 0
              /* debug spew. */
              fprintf (stderr, "labelRefCount: %s has refCount %d, want %d\n",
                       label, entry->refCount, expectedRefCount);
#endif

              rc = (expectedRefCount == entry->refCount);
            }
          else
            {
              // Not a local label. We do not know how often it might be referenced.
              rc = FALSE;
            }
        }
      else
        {
          fprintf (stderr, "*** internal error: var %d not bound"
                   " in peephole labelRefCount rule.\n",
                   varNumber);
        }

    }
  else
    {
      fprintf (stderr,
               "*** internal error: labelRefCount peephole restriction"
               " malformed: %s\n", cmdLine);
    }
  return rc;
}

/* labelRefCountChange:
 * takes two parameters: a variable (bound to a label name)
 * and a signed int for changing the reference count.
 *
 * Please note, this function is not a conditional. It unconditionally
 * changes the label. It should be passed as the 'last' function
 * so it only is applied if all other conditions have been met.
 *
 * should always return TRUE
 */
FBYNAME (labelRefCountChange)
{
  int varNumber, RefCountDelta;
  bool rc = FALSE;

  /* If we don't have the label hash table yet, build it. */
  if (!labelHash)
    {
      buildLabelRefCountHash (head);
    }

  if (sscanf (cmdLine, "%*[ \t%]%d %i", &varNumber, &RefCountDelta) == 2)
    {
      char *label = hTabItemWithKey (vars, varNumber);

      if (label)
        {
          labelHashEntry *entry;

          entry = hTabFirstItemWK (labelHash, hashSymbolName (label));

          while (entry)
            {
              if (!strcmp (label, entry->name))
                {
                  break;
                }
              entry = hTabNextItemWK (labelHash);
            }
          if (entry)
            {
              if (0 <= entry->refCount + RefCountDelta)
                {
                  entry->refCount += RefCountDelta;
                  rc = TRUE;
                }
              else
                {
                  fprintf (stderr, "*** internal error: label %s may not get"
                          " negative refCount in %s peephole.\n",
                           label, __FUNCTION__);
                }
            }
            else
            {
              // Not a local label. We do not know how often it might be referenced.
              return TRUE;
            }
        }
      else
        {
          fprintf (stderr, "*** internal error: var %d not bound"
                   " in peephole %s rule.\n",
                   varNumber, __FUNCTION__);
        }
    }
  else
    {
      fprintf (stderr,
               "*** internal error: labelRefCountChange peephole restriction"
               " malformed: %s\n", cmdLine);
    }
  return rc;
}

/* Within the context of the lines currPl through endPl, determine
** if the variable var contains a symbol that is volatile. Returns
** TRUE only if it is certain that this was not volatile (the symbol
** was found and not volatile, or var was a constant or CPU register).
** Returns FALSE if the symbol was found and volatile, the symbol was
** not found, or var was a indirect/pointer addressing mode.
*/
static bool
notVolatileVariable(char *var, lineNode *currPl, lineNode *endPl)
{
  char symname[SDCC_NAME_MAX + 1];
  char *p = symname;
  char *vp = var;
  lineNode *cl;
  operand *op;
  iCode *last_ic;

  /* Can't tell if indirect accesses are volatile or not, so
  ** assume they are, just to be safe.
  */
  if (TARGET_IS_MCS51 || TARGET_IS_DS390 || TARGET_IS_DS400)
    {
      if (*var=='@')
        return FALSE;
    }
  if (TARGET_Z80_LIKE)
    {
      if (strstr (var, "(bc)"))
        return FALSE;
      if (strstr (var, "(de)"))
        return FALSE;
      if (strstr (var, "(hl)"))
        return FALSE;
      if (strstr (var, "(ix"))
        return FALSE;
      if (strstr (var, "(iy"))
        return FALSE;
    }

  if (TARGET_IS_STM8)
    {
      if (strstr (var, "(x)"))
        return FALSE;
      if (strstr (var, "(y)"))
        return FALSE;
      if (strstr (var, ", x)"))
        return FALSE;
      if (strstr (var, ", y)"))
        return FALSE;
      if (strstr (var, ", sp)"))
        return FALSE;
      if (strchr (var, '[') && strchr (var, ']'))
        return FALSE;
    }

  /* Extract a symbol name from the variable */
  while (*vp && (*vp!='_'))
    vp++;
  while (*vp && (ISCHARALNUM(*vp) || *vp=='_'))
    *p++ = *vp++;
  *p='\0';

  if (!symname[0])
    {
      /* Nothing resembling a symbol name was found, so it can't
         be volatile
      */
      return TRUE;
    }

  last_ic = NULL;
  for (cl = currPl; cl!=endPl->next; cl = cl->next)
  {
    if (cl->ic && (cl->ic!=last_ic))
      {
        last_ic = cl->ic;
        switch (cl->ic->op)
          {
          case IFX:
            op = IC_COND (cl->ic);
            if (IS_SYMOP (op) &&
                ( !strcmp(OP_SYMBOL (op)->rname, symname) ||
                  (OP_SYMBOL (op)->isspilt &&
                   SPIL_LOC (op) &&
                   !strcmp(SPIL_LOC (op)->rname, symname)) ))
              {
                return !op->isvolatile;
              }
          case JUMPTABLE:
            op = IC_JTCOND (cl->ic);
            if (IS_SYMOP (op) &&
                ( !strcmp(OP_SYMBOL (op)->rname, symname) ||
                  (OP_SYMBOL (op)->isspilt &&
                   SPIL_LOC (op) &&
                   !strcmp(SPIL_LOC (op)->rname, symname)) ))
              {
                return !op->isvolatile;
              }
          default:
            op = IC_LEFT (cl->ic);
            if (IS_SYMOP (op) &&
                ( !strcmp(OP_SYMBOL (op)->rname, symname) ||
                  (OP_SYMBOL (op)->isspilt &&
                   SPIL_LOC (op) &&
                   !strcmp(SPIL_LOC (op)->rname, symname)) ))
              {
                return !op->isvolatile;
              }
            op = IC_RIGHT (cl->ic);
            if (IS_SYMOP (op) &&
                ( !strcmp(OP_SYMBOL (op)->rname, symname) ||
                  (OP_SYMBOL (op)->isspilt &&
                   SPIL_LOC (op) &&
                   !strcmp(SPIL_LOC (op)->rname, symname)) ))
              {
                return !op->isvolatile;
              }
            op = IC_RESULT (cl->ic);
            if (IS_SYMOP (op) &&
                ( !strcmp(OP_SYMBOL (op)->rname, symname) ||
                  (OP_SYMBOL (op)->isspilt &&
                   SPIL_LOC (op) &&
                   !strcmp(SPIL_LOC (op)->rname, symname)) ))
              {
                return !op->isvolatile;
              }
          }
      }
  }

  /* Couldn't find the symbol for some reason. Assume volatile. */
  return FALSE;
}

/*  notVolatile:
 *
 *  This rule restriction has two different behaviours depending on
 *  the number of parameters given.
 *
 *    if notVolatile                 (no parameters given)
 *       The rule is applied only if none of the iCodes originating
 *       the matched pattern reference a volatile operand.
 *
 *    if notVolatile %1 ...          (one or more parameters given)
 *       The rule is applied if the parameters are not expressions
 *       containing volatile symbols and are not pointer accesses.
 *
 */
FBYNAME (notVolatile)
{
  int varNumber;
  char *var;
  bool notvol;
  char *digitend;
  lineNode *cl;
  operand *op;

  if (!cmdLine)
    {
      /* If no parameters given, just scan the iCodes for volatile operands */
      for (cl = currPl; cl!=endPl->next; cl = cl->next)
      {
        if (cl->ic)
          {
            switch (cl->ic->op)
              {
              case IFX:
                op = IC_COND (cl->ic);
                if (IS_SYMOP (op) && op->isvolatile)
                  return FALSE;
              case JUMPTABLE:
                op = IC_JTCOND (cl->ic);
                if (IS_SYMOP (op) && op->isvolatile)
                  return FALSE;
              default:
                op = IC_LEFT (cl->ic);
                if (IS_SYMOP (op) && op->isvolatile)
                  return FALSE;
                op = IC_RIGHT (cl->ic);
                if (IS_SYMOP (op) && op->isvolatile)
                  return FALSE;
                op = IC_RESULT (cl->ic);
                if (IS_SYMOP (op) && op->isvolatile)
                  return FALSE;
              }
          }
      }
      return TRUE;
    }

  /* There were parameters; check the volatility of each */
  while (*cmdLine && ISCHARSPACE(*cmdLine))
    cmdLine++;
  while (*cmdLine)
    {
      if (*cmdLine!='%')
        goto error;
      cmdLine++;
      if (!ISCHARDIGIT(*cmdLine))
        goto error;
      varNumber = strtol(cmdLine, &digitend, 10);
      cmdLine = digitend;
      while (*cmdLine && ISCHARSPACE(*cmdLine))
        cmdLine++;

      var = hTabItemWithKey (vars, varNumber);

      if (var)
        {
          notvol = notVolatileVariable (var, currPl, endPl);
          if (!notvol)
            return FALSE;
        }
      else
        {
          fprintf (stderr, "*** internal error: var %d not bound"
                   " in peephole notVolatile rule.\n",
                   varNumber);
          return FALSE;
        }
    }

  return TRUE;

error:
  fprintf (stderr,
           "*** internal error: notVolatile peephole restriction"
           " malformed: %s\n", cmdLine);
  return FALSE;
}

/*------------------------------------------------------------------*/
/* setFromConditionArgs - parse a peephole condition's arguments    */
/* to produce a set of strings, one per argument. Variables %x will */
/* be replaced with their values. String literals (in single quotes)*/
/* are accepted and return in unquoted form.                        */
/*------------------------------------------------------------------*/
static set *
setFromConditionArgs (char *cmdLine, hTab * vars)
{
  int varNumber;
  char *var;
  char *digitend;
  set *operands = NULL;

  if (!cmdLine)
    return NULL;

  while (*cmdLine && ISCHARSPACE(*cmdLine))
    cmdLine++;

  while (*cmdLine)
    {
      if (*cmdLine == '%')
        {
          cmdLine++;
          if (!ISCHARDIGIT(*cmdLine))
            goto error;
          varNumber = strtol(cmdLine, &digitend, 10);
          cmdLine = digitend;

          var = hTabItemWithKey (vars, varNumber);

          if (var)
            {
              addSetHead (&operands, var);
            }
          else
            goto error;
        }
      else if (*cmdLine == '\'' )
        {
          char quote = *cmdLine;

          var = ++cmdLine;
          while (*cmdLine && *cmdLine != quote)
            cmdLine++;
          if (*cmdLine == quote)
            *cmdLine++ = '\0';
          else
            goto error;
          addSetHead (&operands, var);
        }
      else
        goto error;

      while (*cmdLine && ISCHARSPACE(*cmdLine))
        cmdLine++;
    }

  return operands;

error:
  deleteSet (&operands);
  return NULL;
}

static const char *
operandBaseName (const char *op)
{
  if (TARGET_IS_MCS51 || TARGET_IS_DS390 || TARGET_IS_DS400)
    {
      if (!strcmp (op, "acc") || !strncmp (op, "acc.", 4))
        return "a";
      if (!strncmp (op, "ar", 2) && ISCHARDIGIT(*(op+2)) && !*(op+3))
        return op+1;
      // bug 1739475, temp fix
      if (op[0] == '@')
        return operandBaseName(op+1);
    }
  if (TARGET_Z80_LIKE)
    {
      if (!strcmp (op, "d") || !strcmp (op, "e") || !strcmp (op, "(de)"))
        return "de";
      if (!strcmp (op, "b") || !strcmp (op, "c") || !strcmp (op, "(bc)"))
        return "bc";
      if (!strcmp (op, "h") || !strcmp (op, "l") || !strcmp (op, "(hl)") || !strcmp (op, "(hl+)")  || !strcmp (op, "(hl-)"))
        return "hl";
      if (!strcmp (op, "iyh") || !strcmp (op, "iyl") || strstr (op, "iy"))
        return "iy";
      if (!strcmp (op, "ixh") || !strcmp (op, "ixl") || strstr (op, "ix"))
        return "ix";
      if (!strcmp (op, "a"))
        return "af";
    }

  return op;
}

/*-----------------------------------------------------------------*/
/* notUsed - Check, if value in register is not read again         */
/*-----------------------------------------------------------------*/
FBYNAME (notUsed)
{
  const char *what;
  bool ret;

  set *operands = setFromConditionArgs (cmdLine, vars);

  if (!operands || elementsInSet(operands) != 1)
  {
    fprintf (stderr,
             "*** internal error: notUsed peephole restriction"
             " malformed: %s\n", cmdLine);
    return FALSE;
  }

  what = setFirstItem (operands);

  if (!port->peep.notUsed)
    {
      fprintf (stderr, "Function notUsed not initialized in port structure\n");
      return FALSE;
    }

  ret = port->peep.notUsed (what, endPl, head);

  deleteSet(&operands);

  return (ret);
}

/*-----------------------------------------------------------------*/
/* notUsed - Check, if value in register is not read again         */
/*           starting from label                                   */
/*-----------------------------------------------------------------*/
FBYNAME (notUsedFrom)
{
  const char *what, *label;
  set *operands = setFromConditionArgs (cmdLine, vars);

  if (!operands || elementsInSet(operands) != 2)
  {
    fprintf (stderr,
             "*** internal error: notUsedFrom peephole restriction"
             " malformed: %s\n", cmdLine);
    return FALSE;
  }

  what = setFirstItem (operands);
  label = setNextItem (operands); 

  if (port->peep.notUsedFrom)
    return port->peep.notUsedFrom (what, label, head);

  fprintf (stderr, "Function notUsed not initialized in port structure\n");
  return FALSE;
}

/*-----------------------------------------------------------------*/
/* canAssign - Check, if we can do ld dst, src.                    */
/*-----------------------------------------------------------------*/
FBYNAME (canAssign)
{
  set *operands;
  const char *dst, *src, *exotic;

  operands = setFromConditionArgs (cmdLine, vars);

  if (!operands || elementsInSet(operands) < 2 || elementsInSet(operands) > 3)
    {
      fprintf (stderr,
               "*** internal error: canAssign peephole restriction"
               " malformed: %s\n", cmdLine);
      return FALSE;
    }

  if(elementsInSet(operands) == 3)
    {
      exotic = setFirstItem (operands);
      src = setNextItem (operands);
      dst = setNextItem (operands);
    }
  else
    {
      exotic = 0;
      src = setFirstItem (operands);
      dst = setNextItem (operands);
    }

  if (port->peep.canAssign)
    {  
      bool ret = port->peep.canAssign (dst, src, exotic);
      deleteSet (&operands);
      return (ret);
    }

  deleteSet (&operands);

  fprintf (stderr, "Function canAssign not initialized in port structure\n");
  return FALSE;
}

/*-----------------------------------------------------------------*/
/* operandsNotRelated - returns true if the condition's operands   */
/* are not related (taking into account register name aliases).    */
/* N-way comparison performed between all operands.                */
/*-----------------------------------------------------------------*/
FBYNAME (operandsNotRelated)
{
  set *operands;
  const char *op1, *op2;

  operands = setFromConditionArgs (cmdLine, vars);

  if (!operands)
    {
      fprintf (stderr,
               "*** internal error: operandsNotRelated peephole restriction"
               " malformed: %s\n", cmdLine);
      return FALSE;
    }

  while ((op1 = setFirstItem (operands)))
    {
      deleteSetItem (&operands, (void*)op1);
      op1 = operandBaseName (op1);

      for (op2 = setFirstItem (operands); op2; op2 = setNextItem (operands))
        {
          op2 = operandBaseName (op2);
          if (strcmp (op1, op2) == 0)
            {
              deleteSet (&operands);
              return FALSE;
            }
        }
    }

  deleteSet (&operands);
  return TRUE;
}

/*-----------------------------------------------------------------*/
/* notSimilar - Check, if one is another's substring               */
/*-----------------------------------------------------------------*/
FBYNAME (notSimilar)
{
  set *operands;
  const char *op1, *op2;

  operands = setFromConditionArgs (cmdLine, vars);

  if (!operands)
    {
      fprintf (stderr,
               "*** internal error: notSame peephole restriction"
               " malformed: %s\n", cmdLine);
      return FALSE;
    }

  while ((op1 = setFirstItem (operands)))
    {
      deleteSetItem (&operands, (void*)op1);

      for (op2 = setFirstItem (operands); op2; op2 = setNextItem (operands))
        {
          if ((strstr (op1, op2) || strstr (op2, op1)) && strcmp (op1, op2) == 0)
            {
              deleteSet (&operands);
              return FALSE;
            }
        }
    }

  deleteSet (&operands);
  return TRUE;
}

/*-----------------------------------------------------------------*/
/* symmParmStack - Caller readjusts stack by the number of bytes
   that were pushed in all calls to this function                  */
/*-----------------------------------------------------------------*/
FBYNAME (symmParmStack)
{
  if (port->peep.symmParmStack)
    return port->peep.symmParmStack();
  return FALSE;
}

/*-----------------------------------------------------------------*/
/* notSame - Check, that arguments are pairwise not the same       */
/*-----------------------------------------------------------------*/
FBYNAME (notSame)
{
  set *operands;
  const char *op1, *op2;

  operands = setFromConditionArgs (cmdLine, vars);

  if (!operands)
    {
      fprintf (stderr,
               "*** internal error: notSame peephole restriction"
               " malformed: %s\n", cmdLine);
      return FALSE;
    }

  while ((op1 = setFirstItem (operands)))
    {
      deleteSetItem (&operands, (void*)op1);

      for (op2 = setFirstItem (operands); op2; op2 = setNextItem (operands))
        {
          if (strcmp (op1, op2) == 0)
            {
              deleteSet (&operands);
              return FALSE;
            }
        }
    }

  deleteSet (&operands);
  return TRUE;
}

/*-----------------------------------------------------------------*/
/* operandsLiteral - returns true if the condition's operands are  */
/* literals.                                                       */
/*-----------------------------------------------------------------*/
FBYNAME (operandsLiteral)
{
  set *operands;
  const char *op;

  operands = setFromConditionArgs (cmdLine, vars);

  if (!operands)
    {
      fprintf (stderr,
               "*** internal error: operandsLiteral peephole restriction"
               " malformed: %s\n", cmdLine);
      return FALSE;
    }

  for (op = setFirstItem (operands); op; op = setNextItem (operands))
    {
      if (!isdigit( (unsigned char)(*op) ))
        {
          deleteSet (&operands);
          return FALSE;
        }
    }

  deleteSet (&operands);
  return TRUE;
}

static long *
immdGet (const char *pc, long *pl)
{
  long s = 1;

  if (!pc || !pl)
    return NULL;

  // omit space
  for (; ISCHARSPACE (*pc); pc++);
  // parse sign
  for (; !ISCHARDIGIT (*pc); pc++)
    if (*pc == '-')
      s *= -1;
  else if (*pc == '+')
      s *= +1;
  else
    return NULL;

  if (pc[0] == '0' && (pc[1] == 'x' || pc[1] == 'X'))
    {
      if (sscanf (pc + 2, "%lx", pl) != 1)
        return NULL;
    }
  else
    {
      if (sscanf (pc, "%ld", pl) != 1)
        return NULL;
    }

  *pl *= s;
  return pl;
}

static bool
immdError (const char *info, const char *param, const char *cmd)
{
  fprintf (stderr, "*** internal error: immdInRange gets "
           "%s: \"%s\" in \"%s\"\n", info, param, cmd);
  return FALSE;
}

/*-----------------------------------------------------------------*/
/* immdInRange - returns true if the sum or difference of two      */
/* immediates is in a give range.                                  */
/*-----------------------------------------------------------------*/
FBYNAME (immdInRange)
{
  char r[64], operator[8];
  const char *op;
  long i, j, k, h, low, high, left_l, right_l, order;
  const char *padd[] = {"+", "'+'", "\"+\"", "add", "'add'", "\"add\""};
  const char *psub[] = {"-", "'-'", "\"-\"", "sub", "'sub'", "\"sub\""};

  for (i = order = 0; order < 6;)
    {
      // pick up one parameter in the temp buffer r[64]
      for (; ISCHARSPACE (cmdLine[i]) && cmdLine[i]; i++);
      for (j = i; !ISCHARSPACE (cmdLine[j]) && cmdLine[j]; j++);
      if (!cmdLine[i]) // unexpected end
        return immdError ("no enough input", "", cmdLine);
      else
        {
          for (k = i; k < j; k++)
            r[k - i] = cmdLine[k];
          r[j - i] = 0;
        }
      // parse the string by order
      switch (order)
        {
          case 0: // lower bound
            if (!immdGet (r, &low))
              return immdError ("bad lower bound", r, cmdLine);
            break;
          case 1: // upper bound
            if (!immdGet (r, &high))
              return immdError ("bad upper bound", r, cmdLine);
            break;
          case 2: // operator
            if (sscanf (r, "%s", operator) != 1)
              return immdError ("bad operator", r, cmdLine);
            break;
          case 3: // left operand
            if (immdGet (r, &left_l)) // the left operand is given directly
              {
              }
            else if (r[0] == '%') // the left operand is passed via pattern match
              {
                if (!immdGet (r + 1, &k) || !(op = hTabItemWithKey (vars, (int) k)))
                  return immdError ("bad left operand", r, cmdLine);
                else if (!immdGet (op, &left_l))
                  return immdError ("bad left operand", op, r);
              }
            else
              return immdError ("bad left operand", r, cmdLine);
            break;
          case 4: // right operand
            if (immdGet (r, &right_l)) // the right operand is given directly
              {
              }
            else if (r[0] == '%') // the right operand is passed via pattern match
              {
                if (!immdGet (r + 1, &k) || !(op = hTabItemWithKey (vars, (int) k)))
                  return immdError ("bad right operand", r, cmdLine);
                else if (!immdGet (op, &right_l))
                  return immdError ("bad right operand", op, r);
              }
            else
              return immdError ("bad right operand", r, cmdLine);
            break;
          case 5: // result
            if (r[0] != '%' || !immdGet (r + 1, &h))
              return immdError ("bad result container", r, cmdLine);
            break;
          default: // should not reach
            return immdError ("unexpected input", "", cmdLine);
            break;
        }
      order++;
      i = j;
    }

  // calculate
  for (j = k = 0; k < sizeof (padd) / sizeof (padd[0]); k++) // add
    if (strcmp (operator, padd[k]) == 0)
      {
        i = left_l + right_l;
        j = 1;
        break;
      }
  if (!j)
    for (k = 0; k < sizeof (psub) / sizeof (psub[0]); k++) // sub
      if (strcmp (operator, psub[k]) == 0)
        {
          i = left_l - right_l;
          j = 1;
          break;
        }
  if (!j)
    return immdError ("bad operator", operator, cmdLine);

  // bind the result
  if ((low <= i && i <= high) || (high <= i && i <= low))
    {
      char *p[] = {r, NULL};
      sprintf (r, "%ld", i);
      bindVar ((int) h, p, &vars);
      return TRUE;
    }
  else
    {
      return FALSE;
    }
}

static const struct ftab
{
  char *fname;
  int (*func) (hTab *, lineNode *, lineNode *, lineNode *, char *);
}
ftab[] =                                            // sorted on the number of times used
{                                                   // in the peephole rules on 2010-06-12
  {
    "labelRefCount", labelRefCount                  // 161
  },
  {
    "notVolatile", notVolatile                      // 118
  },
  {
    "notUsed", notUsed                              // 96
  },
  {
    "labelRefCountChange", labelRefCountChange      // 86
  },
  {
    "labelInRange", labelInRange                    // 51
  },
  {
    "notSame", notSame                              // 28
  },
  {
    "operandsNotRelated", operandsNotRelated        // 28
  },
  {
    "labelJTInRange", labelJTInRange                // 13
  },
  {
    "24bitMode", flat24bitMode                      // 9
  },
  {
    "canAssign", canAssign                          // 8
  },
  {
    "optimizeReturn", optimizeReturn                // ? just a guess
  },
  {
    "notUsedFrom", notUsedFrom                      // ? just a guess
  },
  {
    "labelIsReturnOnly", labelIsReturnOnly          // 6
  },
  {
    "operandsLiteral", operandsLiteral              // 6
  },
  {
    "portIsDS390", portIsDS390                      // 5
  },
  {
    "labelIsUncondJump", labelIsUncondJump          // 4
  },
  {
    "deadMove", deadMove                            // 2
  },
  {
    "useAcallAjmp", useAcallAjmp                    // 2
  },
  {
    "xramMovcOption", xramMovcOption                // 2
  },
  {
    "okToRemoveSLOC", okToRemoveSLOC                // 0
  },
  {
    "immdInRange", immdInRange
  },
  {
    "notSimilar", notSimilar
  },
  {
    "symmParmStack", symmParmStack
  },
};

/*-----------------------------------------------------------------*/
/* callFuncByName - calls a function as defined in the table       */
/*-----------------------------------------------------------------*/
static int
callFuncByName (char *fname,
                hTab *vars,
                lineNode *currPl, /* first source line matched */
                lineNode *endPl,  /* last source line matched */
                lineNode *head)
{
  int   i;
  char  *cmdCopy, *funcName, *funcArgs, *cmdTerm;
  char  c;
  int   rc;

  /* Isolate the function name part (we are passed the full condition
   * string including arguments)
   */
  cmdTerm = cmdCopy = Safe_strdup(fname);

  do
    {
      funcArgs = funcName = cmdTerm;
      while ((c = *funcArgs) && c != ' ' && c != '\t' && c != '(')
        funcArgs++;
      *funcArgs = '\0';  /* terminate the function name */
      if (c)
        funcArgs++;

      /* Find the start of the arguments */
      if (c == ' ' || c == '\t')
        while ((c = *funcArgs) && (c == ' ' || c == '\t'))
          funcArgs++;

      /* If the arguments started with an opening parenthesis,  */
      /* use the closing parenthesis for the end of the         */
      /* arguments and look for the start of another condition  */
      /* that can optionally follow. If there was no opening    */
      /* parethesis, then everything that follows are arguments */
      /* and there can be no additional conditions.             */
      if (c == '(')
        {

          int num_parenthesis = 0;
          cmdTerm = funcArgs;          

          while ((c = *cmdTerm) && (c != ')' || num_parenthesis))
            {
              if (c == '(')
                num_parenthesis++;
              else if (c == ')')
                num_parenthesis--;
              cmdTerm++;
            }
          *cmdTerm = '\0';  /* terminate the arguments */
          if (c == ')')
            {
              cmdTerm++;
              while ((c = *cmdTerm) && (c == ' ' || c == '\t' || c == ','))
                cmdTerm++;
              if (!*cmdTerm)
                cmdTerm = NULL;
            }
          else
            cmdTerm = NULL; /* closing parenthesis missing */
        }
      else
        cmdTerm = NULL;

      if (!*funcArgs)
        funcArgs = NULL;

      rc = -1;
      for (i = 0; i < ((sizeof (ftab)) / (sizeof (struct ftab))); i++)
        {
          if (strcmp (ftab[i].fname, funcName) == 0)
            {
              rc = (*ftab[i].func) (vars, currPl, endPl, head, funcArgs);
              break;
            }
        }

      if (rc == -1)
        {
          fprintf (stderr,
                   "could not find named function \"%s\" in "
                   "peephole function table\n",
                   funcName);
          // If the function couldn't be found, let's assume it's
          // a bad rule and refuse it.
          rc = FALSE;
          break;
        }
    }
  while (rc && cmdTerm);

  Safe_free(cmdCopy);

  return rc;
}

/*-----------------------------------------------------------------*/
/* newPeepRule - creates a new peeprule and attach it to the root  */
/*-----------------------------------------------------------------*/
static peepRule *
newPeepRule (lineNode * match,
             lineNode * replace,
             char *cond,
             int restart,
             int barrier)
{
  peepRule *pr;

  pr = Safe_alloc ( sizeof (peepRule));
  pr->match = match;
  pr->replace = replace;
  pr->restart = restart;
  pr->barrier = barrier;

  if (cond && *cond)
    {
      pr->cond = Safe_strdup (cond);
    }
  else
    pr->cond = NULL;

  pr->vars = newHashTable (100);

  /* if root is empty */
  if (!rootRules)
    rootRules = currRule = pr;
  else
    currRule = currRule->next = pr;

  return pr;
}

#define SKIP_SPACE(x,y) { while (*x && (ISCHARSPACE(*x) || *x == '\n')) x++; \
                         if (!*x) { fprintf(stderr,y); return ; } }

#define EXPECT_STR(x,y,z) { while (*x && strncmp(x,y,strlen(y))) x++ ;   \
                           if (!*x) { fprintf(stderr,z); return ; } }
#define EXPECT_CHR(x,y,z) { while (*x && *x != y) x++ ;   \
                           if (!*x) { fprintf(stderr,z); return ; } }

/*-----------------------------------------------------------------*/
/* getPeepLine - parses the peep lines                             */
/*-----------------------------------------------------------------*/
static void
getPeepLine (lineNode ** head, char **bpp)
{
  char lines[MAX_PATTERN_LEN];
  char *lp;
  int isComment;

  lineNode *currL = NULL;
  char *bp = *bpp;
  while (1)
    {

      if (!*bp)
        {
          fprintf (stderr, "unexpected end of match pattern\n");
          return;
        }

      if (*bp == '\n')
        {
          bp++;
          while (ISCHARSPACE (*bp) ||
                 *bp == '\n')
            bp++;
        }

      if (*bp == '}')
        {
          bp++;
          break;
        }

      /* read till end of line */
      lp = lines;
      while ((*bp != '\n' && *bp != '}') && *bp)
        *lp++ = *bp++;
      *lp = '\0';

      lp = lines;
      while (*lp && ISCHARSPACE(*lp))
        lp++;
      isComment = (*lp == ';');

      if (!isComment || (isComment && !options.noPeepComments))
        {
          const char *dummy1;
          int dummy2;

          if (!currL)
            *head = currL = newLineNode (lines);
          else
            currL = connectLine (currL, newLineNode (lines));
          currL->isComment = isComment;
          currL->isLabel = isLabelDefinition (currL->line, &dummy1, &dummy2,
                                              TRUE);
        }

    }

  *bpp = bp;
}

/*-----------------------------------------------------------------*/
/* readRules - reads the rules from a string buffer                */
/*-----------------------------------------------------------------*/
static void
readRules (char *bp)
{
  char restart = 0, barrier = 0;
  char lines[MAX_PATTERN_LEN];
  size_t safetycounter;
  char *lp, *rp;
  lineNode *match;
  lineNode *replace;
  lineNode *currL = NULL;

  if (!bp)
    return;
top:
  restart = 0;
  barrier = 0;

  /* look for the token "replace" that is the
     start of a rule */
  while (*bp && strncmp (bp, "replace", 7))
    {
      if (!strncmp (bp, "barrier", 7))
        barrier = 1;
      bp++;
    }

  /* if not found */
  if (!*bp)
    return;

  /* then look for either "restart" or '{' */
  while (strncmp (bp, "restart", 7) &&
         *bp != '{' && bp)
    bp++;

  /* not found */
  if (!*bp)
    {
      fprintf (stderr, "expected 'restart' or '{'\n");
      return;
    }

  /* if brace */
  if (*bp == '{')
    bp++;
  else
    {                           /* must be restart */
      restart++;
      bp += strlen ("restart");
      /* look for '{' */
      EXPECT_CHR (bp, '{', "expected '{'\n");
      bp++;
    }

  /* skip thru all the blank space */
  SKIP_SPACE (bp, "unexpected end of rule\n");

  match = replace = currL = NULL;
  /* we are the start of a rule */
  getPeepLine (&match, &bp);

  /* now look for by */
  EXPECT_STR (bp, "by", "expected 'by'\n");

  /* then look for a '{' */
  EXPECT_CHR (bp, '{', "expected '{'\n");
  bp++;

  /* save char position (needed for generating error msg) */
  rp = bp;

  SKIP_SPACE (bp, "unexpected end of rule\n");
  getPeepLine (&replace, &bp);

  /* look for a 'if' */
  while ((ISCHARSPACE (*bp) || *bp == '\n') && *bp)
    bp++;

  if (strncmp (bp, "if", 2) == 0)
    {
      bp += 2;
      while ((ISCHARSPACE (*bp) || *bp == '\n' || (*bp == '/' && *(bp+1) == '/')) && *bp)
      {
        bp++;
        if (*bp == '/')
          while (*bp && *bp != '\n')
            bp++;
      }
      if (!*bp)
        {
          fprintf (stderr, "expected condition name\n");
          return;
        }

      /* look for the condition */
      lp = lines;
      for (safetycounter = 0; *bp && (*bp != '\n'); safetycounter++)
        {
          wassertl(safetycounter < MAX_PATTERN_LEN, "Peephole line too long.\n");
          *lp++ = *bp++;
        }
      *lp = '\0';

      newPeepRule (match, replace, lines, restart, barrier);
    }
  else
    {
      if (*bp && strncmp (bp, "replace", 7) && strncmp (bp, "barrier", 7))
        {
          /* not the start of a new peeprule, so "if" should be here */

          char strbuff[1000];
          char *cp;

          /* go to the start of the line following "{" of the "by" token */
          while (*rp && (*rp == '\n'))
            rp++;

          /* copy text of rule starting with line after "by {" */
          cp = strbuff;
          while (*rp && (rp < bp) && ((cp - strbuff) < sizeof(strbuff)))
              *cp++ = *rp++;

          /* and now the rest of the line */
          while (*rp && (*rp != '\n') && ((cp - strbuff) < sizeof(strbuff)))
            *cp++ = *rp++;

          *cp = '\0';
          fprintf (stderr, "%s\nexpected '} if ...'\n", strbuff);
          return;
        }
      newPeepRule (match, replace, NULL, restart, barrier);
    }
  goto top;

}

/*-----------------------------------------------------------------*/
/* keyForVar - returns the numeric key for a var                   */
/*-----------------------------------------------------------------*/
static int
keyForVar (const char *d)
{
  int i = 0;

  while (ISCHARDIGIT (*d))
    {
      i *= 10;
      i += (*d++ - '0');
    }

  return i;
}

/*-----------------------------------------------------------------*/
/* bindVar - binds a value to a variable in the given hashtable    */
/*-----------------------------------------------------------------*/
static void
bindVar (int key, char **s, hTab ** vtab)
{
  char vval[MAX_PATTERN_LEN];
  char *vvx;
  char *vv = vval;

  /* first get the value of the variable */
  vvx = *s;
  /* the value is ended by a ',' or space or newline or null or ) */
  while (*vvx &&
         *vvx != ',' &&
         !ISCHARSPACE (*vvx) &&
         *vvx != '\n' &&
         *vvx != ':' &&
         *vvx != ')')
    {
      char ubb = 0;
      /* if we find a '(' then we need to balance it */
      if (*vvx == '(')
        {
          ubb++;
          while (ubb)
            {
              *vv++ = *vvx++;
              if (*vvx == '(')
                ubb++;
              if (*vvx == ')')
                ubb--;
            }
          // include the trailing ')'
          *vv++ = *vvx++;
        }
      else
        *vv++ = *vvx++;
    }
  *s = vvx;
  *vv = '\0';
  /* got value */
  vvx = traceAlloc (&_G.values, Safe_strdup(vval));

  hTabAddItem (vtab, key, vvx);
}

/*-----------------------------------------------------------------*/
/* matchLine - matches one line                                    */
/*-----------------------------------------------------------------*/
static bool
matchLine (char *s, const char *d, hTab ** vars)
{
  if (!s || !(*s))
    return FALSE;

  /* skip leading white spaces */
  while (ISCHARSPACE (*s))
    s++;
  while (ISCHARSPACE (*d))
    d++;

  while (*s && *d)
    {
      /* if the destination is a var */
      if (*d == '%' && ISCHARDIGIT (*(d + 1)) && vars)
        {
          const char *v = hTabItemWithKey (*vars, keyForVar (d + 1));
          /* if the variable is already bound
             then it MUST match with dest */
          if (v)
            {
              while (*v)
                if (*v++ != *s++)
                  return FALSE;
            }
          else
            /* variable not bound we need to bind it */
            bindVar (keyForVar (d + 1), &s, vars);

          /* in either case go past the variable */
          d++;
          while (ISCHARDIGIT (*d))
            d++;
        }
      else if (ISCHARSPACE (*s) && ISCHARSPACE (*d)) /* whitespace sequences match any whitespace sequences */
        {
          while (ISCHARSPACE (*s))
            s++;
          while (ISCHARSPACE (*d))
            d++;
        }
      else if (*s == ',' && *d == ',') /* Allow comman to match comma followed by whitespace */
        {
          s++, d++;
          while (ISCHARSPACE (*s))
            s++;
          while (ISCHARSPACE (*d))
            d++;
        }
      else if (*s && *d) /* they should be an exact match otherwise */
        {
          if (*s++ != *d++)
            return FALSE;
        }
    }

  /* skip trailing whitespaces */
  if (*s)
    while (ISCHARSPACE (*s))
      s++;

  if (*d)
    while (ISCHARSPACE (*d))
      d++;

  /* after all this if only one of them
     has something left over then no match */
  if (*s || *d)
    return FALSE;

  return TRUE;
}

/*-----------------------------------------------------------------*/
/* matchRule - matches a all the rule lines                        */
/*-----------------------------------------------------------------*/
static bool
matchRule (lineNode * pl,
           lineNode ** mtail,
           peepRule * pr,
           lineNode * head)
{
  lineNode *spl;                /* source pl */
  lineNode *rpl;                /* rule peep line */

/*     setToNull((void *) &pr->vars);    */
/*     pr->vars = newHashTable(100); */

  /* for all the lines defined in the rule */
  rpl = pr->match;
  spl = pl;
  while (spl && rpl)
    {

      /* if the source line starts with a ';' then
         comment line don't process or the source line
         contains == . debugger information skip it */
      if (spl->line &&
          (*spl->line == ';' || spl->isDebug))
        {
          spl = spl->next;
          continue;
        }

      if (!matchLine (spl->line, rpl->line, &pr->vars))
        return FALSE;

      rpl = rpl->next;
      if (rpl)
        spl = spl->next;
    }

  /* if rules ended */
  if (!rpl)
    {
      /* if this rule has additional conditions */
      if (pr->cond)
        {
          if (callFuncByName (pr->cond, pr->vars, pl, spl, head))
            {
              *mtail = spl;
              return TRUE;
            }
          else
            return FALSE;
        }
      else
        {
          *mtail = spl;
          return TRUE;
        }
    }
  else
    return FALSE;
}

static void
reassociate_ic_down (lineNode *shead, lineNode *stail,
                     lineNode *rhead, lineNode *rtail)
{
  lineNode *csl;        /* current source line */
  lineNode *crl;        /* current replacement line */

  csl = shead;
  crl = rhead;
  while (1)
    {
      /* skip over any comments */
      while (csl!=stail->next && csl->isComment)
        csl = csl->next;
      while (crl!=rtail->next && crl->isComment)
        crl = crl->next;

      /* quit if we reach the end */
      if ((csl==stail->next) || (crl==rtail->next) || crl->ic)
        break;

      if (matchLine(csl->line,crl->line,NULL))
        {
          crl->ic = csl->ic;
          csl = csl->next;
          crl = crl->next;
        }
      else
        break;
    }
}

static void
reassociate_ic_up (lineNode *shead, lineNode *stail,
                   lineNode *rhead, lineNode *rtail)
{
  lineNode *csl;        /* current source line */
  lineNode *crl;        /* current replacement line */

  csl = stail;
  crl = rtail;
  while (1)
    {
      /* skip over any comments */
      while (csl!=shead->prev && csl->isComment)
        csl = csl->prev;
      while (crl!=rhead->prev && crl->isComment)
        crl = crl->prev;

      /* quit if we reach the end */
      if ((csl==shead->prev) || (crl==rhead->prev) || crl->ic)
        break;

      if (matchLine(csl->line,crl->line,NULL))
        {
          crl->ic = csl->ic;
          csl = csl->prev;
          crl = crl->prev;
        }
      else
        break;
    }
}

/*------------------------------------------------------------------*/
/* reassociate_ic - reassociate replacement lines with origin iCode */
/*------------------------------------------------------------------*/
static void
reassociate_ic (lineNode *shead, lineNode *stail,
                lineNode *rhead, lineNode *rtail)
{
  lineNode *csl;        /* current source line */
  lineNode *crl;        /* current replacement line */
  bool single_iCode;
  iCode *ic;

  /* Check to see if all the source lines (excluding comments) came
  ** for the same iCode
  */
  ic = NULL;
  for (csl=shead;csl!=stail->next;csl=csl->next)
    if (csl->ic && !csl->isComment)
      {
        ic = csl->ic;
        break;
      }
  single_iCode = (ic!=NULL);
  for (csl=shead;csl!=stail->next;csl=csl->next)
    if ((csl->ic != ic) && !csl->isComment)
      {
        /* More than one iCode was found. However, if it's just the
        ** last line with the different iCode and it was not changed
        ** in the replacement, everything else must be the first iCode.
        */
        if ((csl==stail) && matchLine (stail->line, rtail->line, NULL))
          {
            rtail->ic = stail->ic;
            for (crl=rhead;crl!=rtail;crl=crl->next)
              crl->ic = ic;
            return;
          }

        single_iCode = FALSE;
        break;
      }

  /* If all of the source lines came from the same iCode, then so have
  ** all of the replacement lines too.
  */
  if (single_iCode)
    {
      for (crl=rhead;crl!=rtail->next;crl=crl->next)
        crl->ic = ic;
      return;
    }

  /* The source lines span iCodes, so we may end up with replacement
  ** lines that we don't know which iCode(s) to associate with. Do the
  ** best we can by using the following strategies:
  **    1) Start at the top and scan down. As long as the source line
  **       matches the replacement line, they have the same iCode.
  **    2) Start at the bottom and scan up. As long as the source line
  **       matches the replacement line, they have the same iCode.
  **    3) For any label in the source, look for a matching label in
  **       the replacment. If found, they have the same iCode. From
  **       these matching labels, scan down for additional matching
  **       lines; if found, they also have the same iCode.
  */

  /* Strategy #1: Start at the top and scan down for matches
  */
  reassociate_ic_down(shead,stail,rhead,rtail);

  /* Strategy #2: Start at the bottom and scan up for matches
  */
  reassociate_ic_up(shead,stail,rhead,rtail);

  /* Strategy #3: Try to match labels
  */
  csl = shead;
  while (1)
    {
      /* skip over any comments */
      while (csl!=stail->next && csl->isComment)
        csl = csl->next;
      if (csl==stail->next)
        break;

      if (csl->isLabel)
        {
          /* found a source line label; look for it in the replacment lines */
          crl = rhead;
          while (1)
            {
              while (crl!=rtail->next && crl->isComment)
                crl = crl->next;
              if (crl==rtail->next)
                break;
              if (matchLine(csl->line, crl->line, NULL))
                {
                  reassociate_ic_down(csl,stail,crl,rtail);
                  break;
                }
              else
                crl = crl->next;
            }
        }
      csl = csl->next;
    }

  /* Try to assign a meaningful iCode to any comment that is missing
     one. Since they are comments, it's ok to make mistakes; we are just
     trying to improve continuity to simplify other tests.
  */
  ic = NULL;
  for (crl=rtail;crl!=rhead->prev;crl=crl->prev)
    {
      if (!crl->ic && ic && crl->isComment)
        crl->ic = ic;
      ic = crl->ic;
    }
}


/*-----------------------------------------------------------------*/
/* replaceRule - does replacement of a matching pattern            */
/*-----------------------------------------------------------------*/
static void
replaceRule (lineNode ** shead, lineNode * stail, peepRule * pr)
{
  lineNode *cl = NULL;
  lineNode *pl = NULL, *lhead = NULL;
  /* a long function name and long variable name can evaluate to
     4x max pattern length e.g. "mov dptr,((fie_var>>8)<<8)+fie_var" */
  char lb[MAX_PATTERN_LEN*4];
  char *lbp;
  lineNode *comment = NULL;

  /* collect all the comment lines in the source */
  for (cl = *shead; cl != stail; cl = cl->next)
    {
      if (cl->line && (*cl->line == ';' || cl->isDebug))
        {
          pl = (pl ? connectLine (pl, newLineNode (cl->line)) :
                (comment = newLineNode (cl->line)));
          pl->isDebug = cl->isDebug;
          pl->isComment = cl->isComment || (*cl->line == ';');
        }
    }
  cl = NULL;

  /* for all the lines in the replacement pattern do */
  for (pl = pr->replace; pl; pl = pl->next)
    {
      char *v;
      char *l;
      lbp = lb;

      l = pl->line;

      while (*l)
        {
          /* if the line contains a variable */
          if (*l == '%' && ISCHARDIGIT (*(l + 1)))
            {
              v = hTabItemWithKey (pr->vars, keyForVar (l + 1));
              if (!v)
                {
                  fprintf (stderr, "used unbound variable in replacement\n");
                  l++;
                  continue;
                }
              while (*v) {
                *lbp++ = *v++;
              }
              l++;
              while (ISCHARDIGIT (*l)) {
                l++;
              }
              continue;
            }
          *lbp++ = *l++;
        }

      *lbp = '\0';
      if (cl)
        cl = connectLine (cl, newLineNode (lb));
      else
        lhead = cl = newLineNode (lb);
      cl->isComment = pl->isComment;
      cl->isLabel   = pl->isLabel;
    }

  /* add the comments if any to the head of list */
  if (comment)
    {
      lineNode *lc = comment;
      while (lc->next)
        lc = lc->next;
      lc->next = lhead;
      if (lhead)
        lhead->prev = lc;
      lhead = comment;
    }

  if (lhead && cl)
    {
      /* determine which iCodes the replacment lines relate to */
      reassociate_ic(*shead,stail,lhead,cl);

      /* now we need to connect / replace the original chain */
      /* if there is a prev then change it */
      if ((*shead)->prev)
        {
          (*shead)->prev->next = lhead;
          lhead->prev = (*shead)->prev;
        }
      *shead = lhead;
      /* now for the tail */
      if (stail && stail->next)
        {
          stail->next->prev = cl;
          if (cl)
            cl->next = stail->next;
        }
    }
  else
    {
      /* the replacement is empty - delete the source lines */
      if ((*shead)->prev)
        (*shead)->prev->next = stail->next;
      if (stail->next)
        stail->next->prev = (*shead)->prev;
      *shead = stail->next;
    }
}

/* Returns TRUE if this line is a label definition.

 * If so, start will point to the start of the label name,
 * and len will be it's length.
 */
bool
isLabelDefinition (const char *line, const char **start, int *len, bool isPeepRule)
{
  const char *cp = line;

  /* This line is a label if it consists of:
   * [optional whitespace] followed by identifier chars
   * (alnum | $ | _ ) followed by a colon.
   */

  while (*cp && ISCHARSPACE (*cp))
    {
      cp++;
    }

  if (!*cp)
    {
      return FALSE;
    }

  *start = cp;

  while (ISCHARALNUM (*cp) || (*cp == '$') || (*cp == '_') ||
         (isPeepRule && (*cp == '%')))
    {
      cp++;
    }

  if ((cp == *start) || (*cp != ':'))
    {
      return FALSE;
    }

  *len = (cp - (*start));
  return TRUE;
}

/* Not perfect, will not find all references yet. 
   Will however find references in call on Z80, which is sufficient to fix #2970351. */
bool
isLabelReference (const char *line, const char **start, int *len)
{
  const char *s, *e;
  if (!TARGET_Z80_LIKE && !TARGET_IS_STM8)
    return FALSE;

  s = line;
  while (ISCHARSPACE (*s))
    ++s;
  	
  if(strncmp(s, "call", 4))
    return FALSE;
  s += 4;

  while (ISCHARSPACE (*s))
    ++s;
  
  /* Skip condition in conditional call */
  if (strchr(s, ',')) 
    s = strchr(s, ',') + 1;
    
  e = s, *len = 0;
  while(*e && !ISCHARSPACE (*e) && *e != ';')
    ++e, ++(*len);
    
  *start = s;

  return TRUE;
}

/* Quick & dirty string hash function. */
static int
hashSymbolName (const char *name)
{
  int hash = 0;

  while (*name)
    {
      hash = (hash << 6) ^ *name;
      name++;
    }

  if (hash < 0)
    {
      hash = -hash;
    }

  return hash % HTAB_SIZE;
}

/* Build a hash of all labels in the passed set of lines
 * and how many times they are referenced.
 */
static void
buildLabelRefCountHash (lineNode * head)
{
  lineNode *line;
  const char *label;
  int labelLen;
  int i;

  assert (labelHash == NULL);
  labelHash = newHashTable (HTAB_SIZE);

  /* First pass: locate all the labels. */
  for (line = head; line; line = line->next)
    {
      bool ref = FALSE;
      /* run isLabelDefinition to:
         - look for labels in inline assembler
         - calculate labelLen
      */ 
      if ((line->isLabel || line->isInline) && isLabelDefinition (line->line, &label, &labelLen, FALSE) ||
        (ref = TRUE) && isLabelReference (line->line, &label, &labelLen))
        {
          labelHashEntry *entry, *e;

          assert (labelLen <= SDCC_NAME_MAX);
              
          entry = traceAlloc (&_G.labels, Safe_alloc(sizeof (labelHashEntry)));

          memcpy (entry->name, label, labelLen);
          entry->name[labelLen] = 0;
          entry->refCount = -1;
     
          for (e = hTabFirstItemWK (labelHash, hashSymbolName (entry->name)); e; e = hTabNextItemWK (labelHash))
            if (!strcmp (entry->name, e->name))
              goto c;

          /* Assume function entry points are referenced somewhere,   */
          /* even if we can't find a reference (might be from outside */
          /* the function) */
          if (line->ic && (line->ic->op == FUNCTION) || ref)
            entry->refCount++;

          hTabAddItem (&labelHash, hashSymbolName (entry->name), entry);
        }
      c:;
    }


  /* Second pass: for each line, note all the referenced labels. */
  /* This is ugly, O(N^2) stuff. Optimizations welcome... */
  for (line = head; line; line = line->next)
    {
      if (line->isComment)
        continue;
        
      for (i = 0; i < HTAB_SIZE; i++)
        {
          labelHashEntry *thisEntry;

          thisEntry = hTabFirstItemWK (labelHash, i);

          while (thisEntry)
            {
              const char *s;
              if ((s = strstr (line->line, thisEntry->name)) && !ISCHARALNUM (*(s + strlen (thisEntry->name))) && (s == line->line || !ISCHARALNUM (*(s - 1))))
                thisEntry->refCount++;

              thisEntry = hTabNextItemWK (labelHash);
            }
        }
    }

#if 0
  /* Spew the contents of the table. Debugging fun only. */
  for (i = 0; i < HTAB_SIZE; i++)
    {
      labelHashEntry *thisEntry;

      thisEntry = hTabFirstItemWK (labelHash, i);

      while (thisEntry)
        {
          fprintf (stderr, "label: %s ref %d\n",
                   thisEntry->name, thisEntry->refCount);
          thisEntry = hTabNextItemWK (labelHash);
        }
    }
#endif
}

/* How does this work?
   peepHole
    For each rule,
     For each line,
      Try to match
      If it matches,
       replace and restart.

    matchRule
     matchLine

  Where is stuff allocated?

*/

/*-----------------------------------------------------------------*/
/* peepHole - matches & substitutes rules                          */
/*-----------------------------------------------------------------*/
void
peepHole (lineNode ** pls)
{
  lineNode *spl;
  peepRule *pr;
  lineNode *mtail = NULL;
  bool restart, replaced;

#if !OPT_DISABLE_PIC14 || !OPT_DISABLE_PIC16
  /* The PIC port uses a different peep hole optimizer based on "pCode" */
  if (TARGET_PIC_LIKE)
    return;
#endif

  assert(labelHash == NULL);

  do
    {
      restart = FALSE;

      /* for all rules */
      for (pr = rootRules; pr; pr = pr->next)
        {
          if (restart && pr->barrier)
            break;

          for (spl = *pls; spl; spl = replaced ? spl : spl->next)
            {
              replaced = FALSE;

              /* if inline assembler then no peep hole */
              if (spl->isInline)
                continue;

              /* don't waste time starting a match on debug symbol
              ** or comment */
              if (spl->isDebug || spl->isComment || *(spl->line)==';')
                continue;

              mtail = NULL;

              /* Tidy up any data stored in the hTab */

              /* if it matches */
              if (matchRule (spl, &mtail, pr, *pls))
                {
                  /* restart at the replaced line */
                  replaced = TRUE;

                  /* then replace */
                  if (spl == *pls)
                    {
                      replaceRule (pls, mtail, pr);
                      spl = *pls;
                    }
                  else
                    replaceRule (&spl, mtail, pr);

                  /* if restart rule type then
                     start at the top again */
                  if (pr->restart)
                    {
                      restart = TRUE;
                    }
                }

              if (pr->vars)
                {
                  hTabDeleteAll (pr->vars);
                  Safe_free (pr->vars);
                  pr->vars = NULL;
                }

              freeTrace (&_G.values);
            }
        }
    } while (restart == TRUE);

  if (labelHash)
    {
      hTabDeleteAll (labelHash);
      freeTrace (&_G.labels);
    }
  labelHash = NULL;
}


/*-----------------------------------------------------------------*/
/* readFileIntoBuffer - reads a file into a string buffer          */
/*-----------------------------------------------------------------*/
static char *
readFileIntoBuffer (char *fname)
{
  FILE *f;
  char *rs = NULL;
  int nch = 0;
  int ch;
  char lb[MAX_PATTERN_LEN];

  if (!(f = fopen (fname, "r")))
    {
      fprintf (stderr, "cannot open peep rule file\n");
      return NULL;
    }

  while ((ch = fgetc (f)) != EOF)
    {
      lb[nch++] = ch;

      /* if we maxed out our local buffer */
      if (nch >= (MAX_PATTERN_LEN - 2))
        {
          lb[nch] = '\0';
          /* copy it into allocated buffer */
          if (rs)
            {
              rs = Safe_realloc (rs, strlen (rs) + strlen (lb) + 1);
              strncatz (rs, lb,  strlen (rs) + strlen (lb) + 1);
            }
          else
            {
              rs = Safe_strdup (lb);
            }
          nch = 0;
        }
    }
  fclose (f);

  /* if some characters left over */
  if (nch)
    {
      lb[nch] = '\0';
      /* copy it into allocated buffer */
      if (rs)
        {
          rs = Safe_realloc (rs, strlen (rs) + strlen (lb) + 1);
          strncatz (rs, lb, strlen (rs) + strlen (lb) + 1);
        }
      else
        {
          rs = Safe_strdup (lb);
        }
    }
  return rs;
}

/*-----------------------------------------------------------------*/
/* initPeepHole - initialises the peep hole optimizer stuff        */
/*-----------------------------------------------------------------*/
void
initPeepHole (void)
{
  char *s;

  /* read in the default rules */
  if (!options.nopeep)
    {
      readRules (port->peep.default_rules);
    }

  /* if we have any additional file read it too */
  if (options.peep_file)
    {
      readRules (s = readFileIntoBuffer (options.peep_file));
      setToNull ((void *) &s);
      /* override nopeep setting, default rules have not been read */
      options.nopeep = 0;
    }

#if !OPT_DISABLE_PIC14
  /* Convert the peep rules into pcode.
     NOTE: this is only support in the PIC port (at the moment)
  */
  if (TARGET_IS_PIC14)
    peepRules2pCode (rootRules);
#endif

#if !OPT_DISABLE_PIC16
  /* Convert the peep rules into pcode.
     NOTE: this is only support in the PIC port (at the moment)
       and the PIC16 port (VR 030601)
  */
  if (TARGET_IS_PIC16)
    pic16_peepRules2pCode (rootRules);

#endif
}

/*-----------------------------------------------------------------*/
/* StrStr - case-insensitive strstr implementation                 */
/*-----------------------------------------------------------------*/
const char * StrStr (const char * str1, const char * str2)
{
	const char * cp = str1;
	const char * s1;
	const char * s2;

	if ( !*str2 )
	    return str1;

	while (*cp)
	{
		s1 = cp;
		s2 = str2;

		while ( *s1 && *s2 && !(tolower(*s1)-tolower(*s2)) )
			s1++, s2++;

		if (!*s2)
			return( cp );

		cp++;
	}

	return (NULL) ;
}

/*-------------------------------------------------------------------------
  peep.c - source file for peephole optimizer helper functions

  Written By - Philipp Klaus Krause
  Copyright (C) 2020, Sebastian 'basxto' Riedel <sdcc@basxto.de>

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
    if (cpl->isLabel &&
      strncmp (p, cpl->line, strlen(p)) == 0 &&
      cpl->line[strlen(p)] == ':')
        return cpl;

  return NULL;
}

/* Check if reading arg implies reading what. */
static bool argCont(const char *arg, const char *what)
{
  wassert (arg);

  while(isspace (*arg) || *arg == ',')
    arg++;

  if (arg[0] == '#' || arg[0] == '_')
    return false;

  if(arg[0] == '(' && arg[1] && arg[2] && (arg[2] != ')' && arg[3] != ')')
     && !(IS_SM83 && (arg[3] == '-' || arg[3] == '+') && arg[4] == ')'))
    return false;

  if(*arg == '(')
    arg++;

  if (arg[0] == '#' || arg[0] == '_')
    return false;
    
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
  if (strchr(what, 'a') && z80_regs_used_as_parms_in_calls_from_current_function[A_IDX])
    return true;
  if (strstr(what, "iy") && (z80_regs_used_as_parms_in_calls_from_current_function[IYL_IDX] || z80_regs_used_as_parms_in_calls_from_current_function[IYH_IDX]))
    return true;

  return false;
}

/* Check if the flag implies reading what. */
static bool
z80MightReadFlagCondition(const char *cond, const char *what)
{
  while(isspace (*cond))
    cond++;

  if(!STRNCASECMP(cond, "po", 2) || !STRNCASECMP(cond, "pe", 2))
    return !strcmp(what, "pf");
  if(tolower(cond[0]) == 'm' || tolower(cond[0]) == 'p')
    return !strcmp(what, "sf");

  // skip inverted conditions
  if(tolower(cond[0]) == 'n')
    cond++;

  if(tolower(cond[0]) == 'c')
    return !strcmp(what, "cf");
  if(tolower(cond[0]) == 'z')
    return !strcmp(what, "zf");
  return true;
}

static bool
z80MightReadFlag(const lineNode *pl, const char *what)
{
  if(ISINST(pl->line, "ld") ||
     ISINST(pl->line, "or") ||
     ISINST(pl->line, "cp") ||
     ISINST(pl->line, "di") ||
     ISINST(pl->line, "ei") ||
     ISINST(pl->line, "im") ||
     ISINST(pl->line, "in"))
    return false;
  if(ISINST(pl->line, "nop") ||
     ISINST(pl->line, "add") ||
     ISINST(pl->line, "sub") ||
     ISINST(pl->line, "and") ||
     ISINST(pl->line, "xor") ||
     ISINST(pl->line, "dec") ||
     ISINST(pl->line, "inc") ||
     ISINST(pl->line, "cpl") ||
     ISINST(pl->line, "bit") ||
     ISINST(pl->line, "res") ||
     ISINST(pl->line, "set") ||
     ISINST(pl->line, "pop") ||
     ISINST(pl->line, "rlc") ||
     ISINST(pl->line, "rrc") ||
     ISINST(pl->line, "sla") ||
     ISINST(pl->line, "sra") ||
     ISINST(pl->line, "srl") ||
     ISINST(pl->line, "scf") ||
     ISINST(pl->line, "cpd") ||
     ISINST(pl->line, "cpi") ||
     ISINST(pl->line, "ind") ||
     ISINST(pl->line, "ini") ||
     ISINST(pl->line, "ldd") ||
     ISINST(pl->line, "ldi") ||
     ISINST(pl->line, "neg") ||
     ISINST(pl->line, "rld") ||
     ISINST(pl->line, "rrd") ||
     ISINST(pl->line, "mlt") ||
     ISINST(pl->line, "out"))
    return false;
  if(ISINST(pl->line, "halt") ||
     ISINST(pl->line, "rlca") ||
     ISINST(pl->line, "rrca") ||
     ISINST(pl->line, "cpdr") ||
     ISINST(pl->line, "cpir") ||
     ISINST(pl->line, "indr") ||
     ISINST(pl->line, "inir") ||
     ISINST(pl->line, "lddr") ||
     ISINST(pl->line, "ldir") ||
     ISINST(pl->line, "outd") ||
     ISINST(pl->line, "outi") ||
     ISINST(pl->line, "djnz"))
    return false;

  if(IS_RAB &&
    ISINST(pl->line, "bool"))
    return false;

  if(IS_SM83 &&
    (ISINST(pl->line, "stop") ||
    ISINST(pl->line, "ldh")))
    return false;

  if((IS_SM83 || IS_Z80N) &&
      ISINST(pl->line, "swap"))
    return false;

  if(IS_R800 &&
     (ISINST(pl->line, "multu") ||
     ISINST(pl->line, "multuw")))
    return false;

  if(ISINST(pl->line, "rl") ||
     ISINST(pl->line, "rr") ||
     ISINST(pl->line, "rla") ||
     ISINST(pl->line, "rra") ||
     ISINST(pl->line, "sbc") ||
     ISINST(pl->line, "adc") ||
     ISINST(pl->line, "ccf"))
    return (!strcmp(what, "cf"));

  if(ISINST(pl->line, "daa"))
    return (!strcmp(what, "cf") || !strcmp(what, "nf") ||
            !strcmp(what, "hf"));

  if(ISINST(pl->line, "push"))
    return (argCont(pl->line + 4, "af"));

  if(ISINST(pl->line, "ex"))
    return (argCont(pl->line + 2, "af"));

  // catch c, nc, z, nz, po, pe, p and m
  if(ISINST(pl->line, "jp") ||
     ISINST(pl->line, "jr"))
    return (strchr(pl->line, ',') && z80MightReadFlagCondition(pl->line + 2, what));

  // flags don't matter according to calling convention
  if(ISINST(pl->line, "reti") ||
     ISINST(pl->line, "retn"))
    return false;

  if(ISINST(pl->line, "call"))
    return (strchr(pl->line, ',') && z80MightReadFlagCondition(pl->line + 4, what));

  if(ISINST(pl->line, "ret"))
    return (pl->line[3] == '\t' && z80MightReadFlagCondition(pl->line + 3, what));

  // we don't know anything about this
  if(ISINST(pl->line, "rst"))
    return true;

  if (IS_SM83 && ISINST(pl->line, "ldhl"))
    return false;
    
  if(IS_RAB && (ISINST(pl->line, "ioi") || ISINST(pl->line, "ioe") || ISINST(pl->line, "ipres")))
    return(false);

  if(IS_Z80N &&
    (ISINST(pl->line, "bsla") ||
    ISINST(pl->line, "bsra") ||
    ISINST(pl->line, "bsrl") ||
    ISINST(pl->line, "bsrf") ||
    ISINST(pl->line, "brlc")))
    return(false);

  if((IS_Z180 || IS_EZ80_Z80 || IS_Z80N) &&
    ISINST(pl->line, "tst"))
    return(false);
 
  return true; // Fail-safe: we have no idea what happens at this line, so assume it might read anything.
}

static bool
z80MightRead(const lineNode *pl, const char *what)
{
  if(strcmp(what, "iyl") == 0 || strcmp(what, "iyh") == 0)
    what = "iy";
  if(strcmp(what, "ixl") == 0 || strcmp(what, "ixh") == 0)
    what = "ix";

  if(ISINST(pl->line, "call") && strcmp(what, "sp") == 0)
    return TRUE;

  if(strcmp(pl->line, "call\t__initrleblock") == 0 && (strchr(what, 'd') != 0 || strchr(what, 'e') != 0))
    return TRUE;

  if(strcmp(pl->line, "call\t___sdcc_call_hl") == 0 && (strchr(what, 'h') != 0 || strchr(what, 'l') != 0))
    return TRUE;

  if(strcmp(pl->line, "call\t___sdcc_call_iy") == 0 && strstr(what, "iy") != 0)
    return TRUE;

  if(strncmp(pl->line, "call\t___sdcc_bcall_", 19) == 0)
    if (strchr (what, pl->line[19]) != 0 || strchr (what, pl->line[20]) != 0 || strchr (what, pl->line[21]) != 0)
      return TRUE;

  if(ISINST(pl->line, "call") && strchr(pl->line, ',') == 0)
    {
      const symbol *f = findSym (SymbolTab, 0, pl->line + 6);
      if (f && IS_FUNC (f->type))
        return z80IsParmInCall(f->type, what);
      else // Fallback needed for calls through function pointers and for calls to literal addresses.
        return z80MightBeParmInCallFromCurrentFunction(what);
    }

  if(ISINST(pl->line, "reti") || ISINST(pl->line, "retn"))
    return(strcmp(what, "sp") == 0);

  if(ISINST(pl->line, "ret")) // --reserve-regs-iy and the sm83 port use ret in code gen for calls through function pointers
    return((IY_RESERVED || IS_SM83) ? z80IsReturned(what) || z80MightBeParmInCallFromCurrentFunction(what) : z80IsReturned(what)) || strcmp(what, "sp") == 0;

  if(!strcmp(pl->line, "ex\t(sp), hl") || !strcmp(pl->line, "ex\t(sp),hl"))
    return(!strcmp(what, "h") || !strcmp(what, "l") || strcmp(what, "sp") == 0);
  if(!strcmp(pl->line, "ex\t(sp), ix") || !strcmp(pl->line, "ex\t(sp),ix"))
    return(!!strstr(what, "ix") || strcmp(what, "sp") == 0);
  if(!strcmp(pl->line, "ex\t(sp), iy") || !strcmp(pl->line, "ex\t(sp),iy"))
    return(!!strstr(what, "iy") || strcmp(what, "sp") == 0);
  if(!strcmp(pl->line, "ex\tde, hl") || !strcmp(pl->line, "ex\tde,hl"))
    return(!strcmp(what, "h") || !strcmp(what, "l") || !strcmp(what, "d") || !strcmp(what, "e"));
  if(ISINST(pl->line, "ld"))
    {
      if(argCont(strchr(pl->line, ','), what))
        return(true);
      if(*(strchr(pl->line, ',') - 1) == ')' && strstr(pl->line + 3, what) &&
        (strchr(pl->line, '#') == 0 || strchr(pl->line, '#') > strchr(pl->line, ',')) &&
        (strchr(pl->line, '_') == 0 || strchr(pl->line, '_') > strchr(pl->line, ',')))
        return(true);
      if (!strcmp(what, "sp") && strchr(pl->line, '(')) // Assume any indirect memory access to be a stack access. This avoids optimizing out stackframe setups for local variables (bug #3173).
        return(true);
      return(false);
    }

  //ld a, #0x00
  if((ISINST(pl->line, "xor") || ISINST(pl->line, "sub")) &&
     (!strcmp(pl->line+4, "a, a") || !strcmp(pl->line+4, "a,a") || (!strchr(pl->line, ',') && !strcmp(pl->line+4, "a"))))
    return(false);

  //ld a, #0x00
  if(!strcmp(pl->line, "and\ta, #0x00") || !strcmp(pl->line, "and\ta,#0x00") || !strcmp(pl->line, "and\t#0x00"))
    return(false);

  //ld a, #0xff
  if(!strcmp(pl->line, "or\ta, #0xff") || !strcmp(pl->line, "or\ta,#0xff") || !strcmp(pl->line, "or\t#0xff"))
    return(false);

  if(ISINST(pl->line, "adc") ||
     ISINST(pl->line, "add") ||
     ISINST(pl->line, "and") ||
     ISINST(pl->line, "sbc") ||
     ISINST(pl->line, "sub") ||
     ISINST(pl->line, "xor") ||
     IS_R800 && ISINST(pl->line, "multu") ||
     IS_R800 && ISINST(pl->line, "multuw"))
    {
      const char *arg;
      if (ISINST(pl->line, "multu"))
        arg = pl->line + 5;
      else if (ISINST(pl->line, "multuw"))
        arg = pl->line + 6;
      else
        arg = pl->line + 4;
      while(isspace(*arg))
        arg++;
      if(arg[0] == 'a' && arg[1] == ',')
        {
          if(!strcmp(what, "a"))
            return(true);
          arg += 2;
        }
      else if(IS_Z80N && !strncmp(arg, "bc", 2) && *(arg + 2) == ',')
        {
          if(!strcmp(what, "b") || !strcmp(what, "c"))
            return(true);
          arg += 3;
        }
      else if(IS_Z80N && !strncmp(arg, "de", 2) && *(arg + 2) == ',')
        {
          if(!strcmp(what, "d") || !strcmp(what, "e"))
            return(true);
          arg += 3;
        }
      else if(!strncmp(arg, "hl", 2) && arg[2] == ',') // add hl, rr
        {
          if(!strcmp(what, "h") || !strcmp(what, "l"))
            return(true);
          arg += 3;
        }
      else if(!strncmp(arg, "sp", 2) && arg[2] == ',') // add sp, rr
        {
          if(!strcmp(what, "sp"))
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
      else if(!strncmp(arg, "iy", 2) && *(arg + 2) == ',')
        {
          if(!strcmp(what, "iy"))
            return(true);
          arg += 3;
        }
      return(argCont(arg, what));
    }

  if(ISINST(pl->line, "neg"))
    return(strcmp(what, "a") == 0);

  if(ISINST(pl->line, "pop"))
    return(strcmp(what, "sp") == 0);

  if(ISINST(pl->line, "push"))
    return(strstr(pl->line + 5, what) != 0 || strcmp(what, "sp") == 0);

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
     ISINST(pl->line, "rra")  ||
     ISINST(pl->line, "daa"))
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
     ISINST(pl->line, "rrc") ||
     ISINST(pl->line, "sra") ||
     ISINST(pl->line, "srl"))
    {
      return(argCont(pl->line + 4, what));
    }
  if((IS_SM83 || IS_Z80N) && ISINST(pl->line, "swap"))
    {
      return(argCont(pl->line + 5, what));
    }
  if(!IS_SM83 && !IS_RAB &&
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
    ISINST(pl->line, "scf")  ||
    ISINST(pl->line, "nop")  ||
    ISINST(pl->line, "halt") ||
    (IS_SM83 && ISINST(pl->line, "stop")))
    return(false);

  if(ISINST(pl->line, "jp") || ISINST(pl->line, "jr"))
    return(false);

  if(ISINST(pl->line, "djnz"))
    return(strchr(what, 'b') != 0);

  if(!IS_SM83 && (ISINST(pl->line, "ldd") || ISINST(pl->line, "lddr") || ISINST(pl->line, "ldi") || ISINST(pl->line, "ldir")))
    return(strchr("bcdehl", *what));
  if(IS_SM83 && (ISINST(pl->line, "ldd") || ISINST(pl->line, "ldi")))
    return(strchr("hl", *what) || strstr(strchr(pl->line + 4, ','), what) != 0);

  if(!IS_SM83 && !IS_RAB && (ISINST(pl->line, "cpd") || ISINST(pl->line, "cpdr") || ISINST(pl->line, "cpi") || ISINST(pl->line, "cpir")))
    return(strchr("abchl", *what));

  if(!IS_SM83 && !IS_RAB && ISINST(pl->line, "out"))
    return(strstr(strchr(pl->line + 4, ','), what) != 0 || strstr(pl->line + 4, "(c)") && (!strcmp(what, "b") || !strcmp(what, "c")));
  if(!IS_SM83 && !IS_RAB && ISINST(pl->line, "in"))
    return(!strstr(strchr(pl->line + 4, ','), "(c)") && !strcmp(what, "a") || strstr(strchr(pl->line + 4, ','), "(c)") && (!strcmp(what, "b") || !strcmp(what, "c")));

  if(IS_SM83 && (ISINST(pl->line, "out") || ISINST(pl->line, "ldh") || ISINST(pl->line, "in")))
    return(strstr(strchr(pl->line + 3, ','), what) != 0 || (!strcmp(what, "c") && strstr(pl->line + 3, "(c)")));

  if(!IS_SM83 && !IS_RAB &&
    (ISINST(pl->line, "ini") || ISINST(pl->line, "ind") || ISINST(pl->line, "inir") || ISINST(pl->line, "indr") ||
    ISINST(pl->line, "outi") || ISINST(pl->line, "outd") || ISINST(pl->line, "otir") || ISINST(pl->line, "otdr")))
    return(strchr("bchl", *what));

  if((IS_Z180 || IS_EZ80_Z80) && ISINST(pl->line, "in0"))
    return(false);

  if((IS_Z180 || IS_EZ80_Z80 || IS_Z80N) && ISINST(pl->line, "mlt"))
    return(argCont(pl->line + 4, what));

  if((IS_Z180 || IS_EZ80_Z80) &&
    (ISINST(pl->line, "otim") || ISINST(pl->line, "otimr") || ISINST(pl->line, "otir") || ISINST(pl->line, "otirx")))
    return(strchr("bchl", *what));

  if((IS_Z180 || IS_EZ80_Z80) && ISINST(pl->line, "slp"))
    return(false);

  if((IS_Z180 || IS_EZ80_Z80 || IS_Z80N) && ISINST(pl->line, "tst"))
    return(argCont(pl->line + 4, what));

  if((IS_Z180 || IS_EZ80_Z80) && ISINST(pl->line, "tstio"))
    return(!strcmp(what, "c"));

  if(IS_RAB && (ISINST(pl->line, "ioi") || ISINST(pl->line, "ioe")))
    return(false);
    
  if(IS_RAB && ISINST(pl->line, "mul"))
    return(!strcmp(what, "b") || !strcmp(what, "c") || !strcmp(what, "d") || !strcmp(what, "e"));

  if(IS_RAB && ISINST(pl->line, "bool"))
    return(argCont(pl->line + 5, what));
    
  if(IS_R3KA && ISINST(pl->line, "lsdr") || ISINST(pl->line, "lidr") || ISINST(pl->line, "lsddr") || ISINST(pl->line, "lsidr"))
    return(strchr("bcdehl", *what));

  if(IS_EZ80_Z80 && ISINST(pl->line, "lea"))
    return(argCont(strchr(pl->line + 4, ','), what));

  if(IS_EZ80_Z80 && ISINST(pl->line, "pea"))
    return(argCont(pl->line + 4, what) || !strcmp(what, "sp"));

  if (IS_SM83 && (ISINST(pl->line, "lda") || ISINST(pl->line, "ldhl")))
    return(!strcmp(what, "sp"));

  if(IS_Z80N &&
    (ISINST(pl->line, "bsla") ||
    ISINST(pl->line, "bsra") ||
    ISINST(pl->line, "bsrl") ||
    ISINST(pl->line, "bsrf") ||
    ISINST(pl->line, "brlc")))
    return(strchr("bde", *what));

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

// TODO: z80 flags only partly implemented
static bool
z80SurelyWritesFlag(const lineNode *pl, const char *what)
{
  /* LD instruction is never change flags except LD A,I and LD A,R.
    But it is most popular instruction so place it first */
  if(ISINST(pl->line, "ld"))
    {
      if(IS_SM83 || IS_RAB || !!strcmp(what, "pf") ||
          !argCont(pl->line+3, "a"))
        return false;
      const char *p = strchr(pl->line+4, ',');
      if (p == NULL)
        return false; /* unknown instruction */
      ++p;
      return argCont(p, "i") || argCont(p, "r");
    }

  if(ISINST(pl->line, "rlca") ||
     ISINST(pl->line, "rrca") ||
     ISINST(pl->line, "rra")  ||
     ISINST(pl->line, "rla"))
    return (IS_SM83 || !!strcmp(what, "zf") && !!strcmp(what, "sf") && !!strcmp(what, "pf"));

  if(ISINST(pl->line, "adc") ||
     ISINST(pl->line, "and") ||
     ISINST(pl->line, "sbc") ||
     ISINST(pl->line, "sub") ||
     ISINST(pl->line, "xor") ||
     ISINST(pl->line, "and") ||
     ISINST(pl->line, "rlc") ||
     ISINST(pl->line, "rrc") ||
     ISINST(pl->line, "sla") ||
     ISINST(pl->line, "sra") ||
     ISINST(pl->line, "srl") ||
     ISINST(pl->line, "neg"))
    return true;

  if(ISINST(pl->line, "or") ||
     ISINST(pl->line, "cp") ||
     ISINST(pl->line, "rl") ||
     ISINST(pl->line, "rr"))
    return true;

  if(ISINST(pl->line, "bit") ||
     ISINST(pl->line, "cpd") ||
     ISINST(pl->line, "cpi") ||
     ISINST(pl->line, "ind") ||
     ISINST(pl->line, "ini") ||
     ISINST(pl->line, "rrd"))
    return (!!strcmp(what, "cf"));

  if(ISINST(pl->line, "cpdr") ||
     ISINST(pl->line, "cpir") ||
     ISINST(pl->line, "indr") ||
     ISINST(pl->line, "inir") ||
     ISINST(pl->line, "otdr") ||
     ISINST(pl->line, "otir") ||
     ISINST(pl->line, "outd") ||
     ISINST(pl->line, "outi"))
    return (!!strcmp(what, "cf"));

  if(ISINST(pl->line, "daa"))
    return (!!strcmp(what, "nf"));

  if(ISINST(pl->line, "ccf") ||
    ISINST(pl->line, "scf"))
    return (!strcmp(what, "hf") || !strcmp(what, "nf") || !strcmp(what, "cf"));

  if(ISINST(pl->line, "cpl"))
    return (!strcmp(what, "hf") || !strcmp(what, "nf"));

  if(ISINST(pl->line, "inc") || ISINST(pl->line, "dec"))
    {
      // 8-bit inc affects all flags other than c.
      if (strlen(pl->line + 4) == 1 || // 8-bit register
        !strcmp(pl->line + 4, "(hl)") ||
        !strcmp(pl->line + 6, "(ix)") ||
        !strcmp(pl->line + 6, "(iy)"))
        return (!!strcmp(what, "cf"));
      return false; // 16-bit inc does not affect flags.
    }

  if(ISINST(pl->line, "add"))
    return (argCont(pl->line + 4, "a") ||
           (!!strcmp(what, "zf") && !!strcmp(what, "sf") && !!strcmp(what, "pf")));

  if(ISINST(pl->line, "ldd") ||
    ISINST(pl->line, "lddr") ||
    ISINST(pl->line, "ldi") ||
    ISINST(pl->line, "ldir"))
    return (!strcmp(what, "hf") || !strcmp(what, "pf") || !strcmp(what, "nf"));

  // pop af writes
  if(ISINST(pl->line, "pop"))
    return (argCont(pl->line + 4, "af"));

  // according to calling convention caller has to save flags
  if(ISINST(pl->line, "ret") ||
     ISINST(pl->line, "call"))
    return true;

  if(ISINST(pl->line, "rld") ||
    ISINST(pl->line, "rrd"))
    return (!strcmp(what, "hf") || !strcmp(what, "pf") || !strcmp(what, "nf"));

  if(ISINST(pl->line, "djnz") ||
    ISINST(pl->line, "ex") ||
    ISINST(pl->line, "nop") ||
    ISINST(pl->line, "out") ||
    ISINST(pl->line, "push") ||
    ISINST(pl->line, "res") ||
    ISINST(pl->line, "set"))
    return false;

  if(IS_SM83 &&
     (ISINST(pl->line, "swap") ||
      ISINST(pl->line, "ldhl") ||
      ISINST(pl->line, "lda")))
    return true;

  if(IS_Z80N &&
    ISINST(pl->line, "swap"))
    return false;

  /* handle IN0 r,(n) and IN r,(c) instructions */
  if(ISINST(pl->line, "in0") || (!strncmp(pl->line, "in\t", 3) &&
     (!strcmp(pl->line+5, "(c)") || !strcmp(pl->line+5, "(bc)"))))
    return (!!strcmp(what, "cf"));

  if(IS_RAB &&
    ISINST(pl->line, "bool"))
    return true;

  if(IS_RAB &&
    ISINST(pl->line, "ipres"))
    return false;

  if(ISINST(pl->line, "mlt"))
    return (!IS_Z80N);

  if((IS_Z180 || IS_EZ80_Z80 || IS_Z80N) &&
    ISINST(pl->line, "tst"))
    return true;

  return false; // Fail-safe: we have no idea what happens at this line, so assume it writes nothing.
}

static bool
callSurelyWrites (const lineNode *pl, const char *what)
{
  const symbol *f = 0;
  if (ISINST(pl->line, "call") && !strchr(pl->line, ','))
    f = findSym (SymbolTab, 0, pl->line + 6);
  else if ((ISINST(pl->line, "jp") || ISINST(pl->line, "jr")) && !strchr(pl->line, ','))
    f = findSym (SymbolTab, 0, pl->line + 4);

  const bool *preserved_regs;

  if (f && (strlen(what) == 2 && what[1] == 'f')) // Flags are never preserved across function calls.
    return(true);

  if(!strcmp(what, "ix"))
    return(false);

  if(f)
    preserved_regs = f->type->funcAttrs.preserved_regs;
  else if (ISINST(pl->line, "call"))
    preserved_regs = z80_regs_preserved_in_calls_from_current_function;
  else // Err on the safe side for jp and jr - might not be a function call, might e.g. be a jump table.
    return (false);

  if (!strcmp (what, "a"))
    return !preserved_regs[A_IDX];
  if (!strcmp (what, "c"))
    return !preserved_regs[C_IDX];
  if (!strcmp (what, "b"))
    return !preserved_regs[B_IDX];
  if (!strcmp (what, "e"))
    return !preserved_regs[E_IDX];
  if (!strcmp (what, "d"))
    return !preserved_regs[D_IDX];
  if (!strcmp (what, "l"))
    return !preserved_regs[L_IDX];
  if (!strcmp (what, "h"))
    return !preserved_regs[H_IDX];
  if (!strcmp (what, "iyl"))
    return !preserved_regs[IYL_IDX];
  if (!strcmp (what, "iyh"))
    return !preserved_regs[IYH_IDX];
  if (!strcmp (what, "iy"))
    return !preserved_regs[IYL_IDX] && !preserved_regs[IYH_IDX];

  return (false);
}

static bool
z80SurelyWrites (const lineNode *pl, const char *what)
{
  if(strcmp(what, "iyl") == 0 || strcmp(what, "iyh") == 0)
    what = "iy";
  if(strcmp(what, "ixl") == 0 || strcmp(what, "ixh") == 0)
    what = "ix";

  //ld a, #0x00
  if((ISINST(pl->line, "xor") || ISINST(pl->line, "sub")) && !strcmp(what, "a") &&
     (!strcmp(pl->line+4, "a, a") || !strcmp(pl->line+4, "a,a") || (!strchr(pl->line, ',') && !strcmp(pl->line+4, "a"))))
    return(true);

  //ld a, #0x00
  if(!strcmp(what, "a") && (!strcmp(pl->line, "and\ta, #0x00") || !strcmp(pl->line, "and\ta,#0x00") || !strcmp(pl->line, "and\t#0x00")))
    return(true);

  //ld a, #0xff
  if(!strcmp(what, "a") && (!strcmp(pl->line, "or\ta, #0xff") || !strcmp(pl->line, "or\ta,#0xff") || !strcmp(pl->line, "or\t#0xff")))
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

  if(IS_SM83 && (ISINST(pl->line, "ldd") || ISINST(pl->line, "ldi") || ISINST(pl->line, "ldh")))
    return(strncmp(pl->line + 4, what, strlen(what)) == 0);

  if(ISINST(pl->line, "pop") && strstr(pl->line + 4, what))
    return(true);
  if (ISINST(pl->line, "call") && strchr(pl->line, ',') == 0)
    return (callSurelyWrites (pl, what));

  if(strcmp(pl->line, "ret") == 0)
    return true;

  if (IS_Z180 || IS_EZ80_Z80 || IS_Z80N)
    if (ISINST(pl->line, "mlt"))
      return(strchr(pl->line + 4, *what) != 0);

  if (IS_R800)
    if (ISINST(pl->line, "multu"))
      return(strchr("hl", *what) != NULL);

  if (IS_R800)
    if (ISINST(pl->line, "multuw"))
      return(strchr("dehl", *what) != NULL);

  if (IS_Z180 || IS_EZ80_Z80)
    {
      if (ISINST(pl->line, "otim") ||
        ISINST(pl->line, "otimr") ||
        ISINST(pl->line, "otdm") ||
        ISINST(pl->line, "otdmr"))
        return(strchr("bchl", *what) != NULL);

      if (ISINST(pl->line, "in0"))
        return(!strncmp(pl->line + 4, what, strlen(what)));
    }

  if (IS_EZ80_Z80 && ISINST(pl->line, "lea"))
    return (strstr(pl->line + 4, what));

  if (IS_SM83 && ISINST(pl->line, "lda") && strncmp(pl->line + 4, "hl", 2) == 0 && (what[0] == 'h' || what[0] == 'l'))
    return(true);

  if (IS_SM83 && ISINST(pl->line, "ldhl") && (what[0] == 'h' || what[0] == 'l'))
    return(true);

  return(false);
}

static bool
z80SurelyReturns(const lineNode *pl)
{
  if(strcmp(pl->line, "ret") == 0)
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
  bool isFlag = (strlen(what) == 2 && what[1] == 'f');
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

      if(isFlag)
        {
          if(z80MightReadFlag(*pl, what))
            {
              D(("S4O_RD_OP (flag)\n"));
              return S4O_RD_OP;
            }
        }
      else
        {
          if (z80MightRead (*pl, what))
            {
              D (("S4O_RD_OP\n"));
              return S4O_RD_OP;
            }
        }

      if (z80UncondJump (*pl))
        {
          lineNode *tlbl = findLabel (*pl);
          if (!tlbl) // jp/jr could be a tail call.
            {
              const symbol *f = findSym (SymbolTab, 0, (*pl)->line + 4);
              if (f && z80IsParmInCall(f->type, what))
                {
                  D (("S4O_RD_OP\n"));
                  return S4O_RD_OP;
                }
              else if(callSurelyWrites (*pl, what))
                {
                  D (("S4O_WR_OP\n"));
                  return S4O_WR_OP;
                }
            }
          *pl = tlbl;
          if (!*pl)
            {
              D (("S4O_ABORT\n"));
              return S4O_ABORT;
            }
        }
      if (z80CondJump(*pl))
        {
          *plCond = findLabel (*pl);
          if (!*plCond)
            {
              D (("S4O_ABORT\n"));
              return S4O_ABORT;
            }
          D (("S4O_CONDJMP\n"));
          return S4O_CONDJMP;
        }

      if (isFlag)
        {
          if (z80SurelyWritesFlag (*pl, what))
            {
              D (("S4O_WR_OP (flag)\n"));
              return S4O_WR_OP;
            }
        }
      else
        {
        if(z80SurelyWrites(*pl, what))
          {
            D(("S4O_WR_OP\n"));
            return S4O_WR_OP;
          }
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

  if(strcmp(what, "af") == 0)
    {
      if(!z80notUsed("a", endPl, head))
        return FALSE;
      what++;
    }

  if(strcmp(what, "f") == 0)
    return z80notUsed("zf", endPl, head) && z80notUsed("cf", endPl, head) &&
           z80notUsed("sf", endPl, head) && z80notUsed("pf", endPl, head) &&
           z80notUsed("nf", endPl, head) && z80notUsed("hf", endPl, head);

  if(strcmp(what, "iy") == 0)
    {
      if(IY_RESERVED)
        return FALSE;
      return(z80notUsed("iyl", endPl, head) && z80notUsed("iyh", endPl, head));
    }

  if(strcmp(what, "ix") == 0)
    return(z80notUsed("ixl", endPl, head) && z80notUsed("ixh", endPl, head));

  if(isRegPair(what))
    {
      char low[2], high[2];
      low[0] = what[1];
      high[0] = what[0];
      low[1] = 0;
      high[1] = 0;
      return(z80notUsed(low, endPl, head) && z80notUsed(high, endPl, head));
    }

  // P/V and L/V (rabbits) are the same flag
  if(!strcmp(what, "vf") || !strcmp(what, "lf"))
    what = "pf";

  // SM83 does not use what it does not have
  // but this allows to write rules for all Z80ies
  if(IS_SM83 && (!strcmp(what, "sf") || !strcmp(what, "pf")))
    {
      D(("Flag %s does not exist\n", what));
      return true;
    }

  // enable sp and flags
  if(!isReg(what) && !isUReg(what) &&
     strcmp(what, "sp") && strcmp(what+1, "f"))
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
      if(isReg(op1) || (IS_EZ80_Z80 && isRegPair(op1)))
        return TRUE;
    }
    else if(!strcmp(op2, "ix") || !strcmp(op2, "iy"))
    {
      if(isReg(exotic) || exotic[0] == '#' || (IS_EZ80_Z80 && isRegPair(exotic)))
        return TRUE;
    }

    return FALSE;
  }

  // Everything else.
  dst = op1;
  src = op2;

  // 8-bit regs can be assigned to each other directly.
  if(isReg(dst) && isReg(src))
    return true;
  if(HAS_IYL_INST) // With some restrictions also for iyl, iyh, ixl, ixh.
    {
      if (isUReg(dst) && isReg(src) && src[0] <= 'e')
        return true;
      if (isUReg(src) && isReg(dst) && dst[0] <= 'e')
        return true;    
      if (isUReg(dst) && isUReg(src) && src[1] == dst[1])
        return true;
    }

  // Immediates van be loaded into 8-bit registers.
  if((isReg(dst) || HAS_IYL_INST && isUReg(dst)) && src[0] == '#')
    return true;

  // Same if at most one of them is (hl).
  if((isReg(dst) || (IS_EZ80_Z80 && isRegPair(dst))) && !strcmp(src, "(hl)"))
    return TRUE;
  if(!strcmp(dst, "(hl)") && (isReg(src) || (IS_EZ80_Z80 && isRegPair(src))))
    return TRUE;

  // Can assign between a and (bc), (de), (hl+), (hl-)
  if(!strcmp(dst, "a") &&
     (!strcmp(src, "(bc)") || !strcmp(src, "(de)") || !strcmp(src, "(hl+)") || !strcmp(src, "(hl-)")))
    return TRUE;
  if((!strcmp(dst, "(bc)") || !strcmp(dst, "(de)") || !strcmp(src, "(hl+)") || !strcmp(src, "(hl-)"))
     && !strcmp(src, "a"))
    return TRUE;

  // Can load immediate values directly into registers and register pairs.
  if((isReg(dst) || isRegPair(dst) || !strcmp(src, "sp")) && src[0] == '#')
    return TRUE;

  if((!strcmp(dst, "a") || (!IS_SM83 && (isRegPair(dst) || !strcmp(src, "sp")))) && !strncmp(src, "(#", 2))
    return TRUE;
  if(!strncmp(dst, "(#", 2) && (!strcmp(src, "a") || (!IS_SM83 && isRegPair(src)) || !strcmp(src, "sp")))
    return TRUE;

  // Can load immediate values directly into (hl).
  if(!strcmp(dst, "(hl)") && src[0] == '#')
    return true;

  // Can load between hl / ix / iy and sp.
  if(!strcmp(dst, "sp") && (!strcmp(src, "hl") || !strcmp(src, "ix") || !strcmp(src, "iy")) ||
    (!strcmp(dst, "hl") || !strcmp(dst, "ix") || !strcmp(dst, "iy")) && !strcmp(src, "sp"))
    return true;

  // Rabbit can load between iy and hl.
  if (IS_RAB &&
    (!strcmp(dst, "hl") && (!strcmp(src, "ix") || !strcmp(src, "iy")) ||
    (!strcmp(dst, "ix") || !strcmp(dst, "iy")) && !strcmp(src, "hl")))
    return true;

  return false;
}

static const char *
registerBaseName (const char *op)
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
  return op;
}

// canJoinRegs(reg_hi reg_lo [dst]) returns TRUE,
bool z80canJoinRegs (const char **regs, char dst[20])
{
  //check for only 2 source registers
  if (!regs[0] || !regs[1] || regs[2])
    return FALSE;
  size_t l1 = strlen (regs[0]);
  size_t l2 = strlen (regs[1]);
  if (l1 + l2 >= 20)
    return FALSE;
  if (l1 == 0 || l2 == 0)
    {
      if (l1 == 0 && l2 == 0)
        return FALSE;
      strcpy (dst, registerBaseName (regs[l1 ? 0 : 1]));
    }
  else
    {
      memcpy (&dst[0], regs[0], l1);
      memcpy (&dst[l1], regs[1], l2 + 1); //copy including \0
    }
  if (!strcmp (dst, "ixhixl") || !strcmp (dst, "iyhiyl"))
    {
      if (IS_SM83)
        return FALSE;
      dst[2] = '\0';
    }
  return isRegPair (dst);
}

bool z80canSplitReg (const char *reg, char dst[][16], int nDst)
{
  int i;
  if (nDst < 0 || nDst > 2)
    return FALSE;
  if (!strcmp (reg, "bc") || !strcmp (reg, "de") || !strcmp (reg, "hl"))
    {
      for (i = 0; i < nDst; ++i)
        {
          dst[i][0] = reg[i];
          dst[i][1] = '\0';
        }
    }
  else if (HAS_IYL_INST && (!strcmp (reg, "ix") || !strcmp (reg, "iy")))
    {
      for (i = 0; i < nDst; ++i)
        {
          dst[i][0] = reg[0];
          dst[i][1] = reg[1];
          dst[i][2] = "hl"[i];
          dst[i][3] = '\0';
        }
    }
  else
    return FALSE;

  return TRUE;
}

int z80instructionSize(lineNode *pl)
{
  const char *op1start, *op2start;

  /* move to the first operand:
   * leading spaces are already removed, skip the mnemonic */
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
         STRNCASECMP(op1start, "(de)", 4) && STRNCASECMP(op1start, "(hl" , 3) &&
         STRNCASECMP(op2start, "hl", 2)   && STRNCASECMP(op2start, "a", 1)   &&
         (!IS_SM83 || STRNCASECMP(op2start, "sp", 2)) ||
         op2start[0] == '('               && STRNCASECMP(op2start, "(bc)", 4) &&
         STRNCASECMP(op1start, "(de)", 4) && STRNCASECMP(op2start, "(hl" , 3) &&
         STRNCASECMP(op1start, "hl", 2)   && STRNCASECMP(op1start, "a", 1))
        return(4);

      /* Rabbit 16-bit pointer load */
      if(IS_RAB && !STRNCASECMP(op1start, "hl", 2) && (argCont(op2start, "(hl)") || argCont(op2start, "(iy)")))
        return(4);
      if(IS_RAB && !STRNCASECMP(op1start, "hl", 2) && (argCont(op2start, "(sp)") || argCont(op2start, "(ix)")))
        return(3);

      /* eZ80 16-bit pointer load */
      if(IS_EZ80_Z80 &&
        (!STRNCASECMP(op1start, "bc", 2) || !STRNCASECMP(op1start, "de", 2) || !STRNCASECMP(op1start, "hl", 2) || !STRNCASECMP(op1start, "ix", 2) || !STRNCASECMP(op1start, "iy", 2)))
        {
          if (!STRNCASECMP(op2start, "(hl)", 4))
            return(2);
          if (argCont(op2start, "(ix)") || argCont(op2start, "(iy)"))
            return(3);
        }
      if(IS_EZ80_Z80 &&
        (!STRNCASECMP(op2start, "bc", 2) || !STRNCASECMP(op2start, "de", 2) || !STRNCASECMP(op2start, "hl", 2) || !STRNCASECMP(op2start, "ix", 2) || !STRNCASECMP(op2start, "iy", 2)))
        {
          if (!STRNCASECMP(op1start, "(hl)", 4))
            return(2);
          if (argCont(op1start, "(ix)") || argCont(op1start, "(iy)"))
            return(3);
        }

      /* These 4 are the only remaining cases of 3 byte long ld instructions. */
      if(argCont(op2start, "(ix)") || argCont(op2start, "(iy)"))
        return(3);
      if(argCont(op1start, "(ix)") || argCont(op1start, "(iy)"))
        return(3);
      if((op1start[0] == '(' && STRNCASECMP(op1start, "(bc)", 4) && STRNCASECMP(op1start, "(de)", 4) && STRNCASECMP(op1start, "(hl", 3)) ||
         (op2start[0] == '(' && STRNCASECMP(op2start, "(bc)", 4) && STRNCASECMP(op2start, "(de)", 4) && STRNCASECMP(op2start, "(hl", 3)))
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

  // load from sp with offset
  if(IS_SM83 && (ISINST(pl->line, "lda") || ISINST(pl->line, "ldhl")))
    {
      return(2);
    }
  // load from/to 0xffXX addresses
  if(IS_SM83 && (ISINST(pl->line, "ldh") || ISINST(pl->line, "in") || ISINST(pl->line, "out")))
    {
      if(!STRNCASECMP(pl->line, "(c)", 3))
        return(1);
      return(2);
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
  if(ISINST(pl->line, "push")  && IS_Z80N && op1start[0] == '#')
    return(4);
  if(ISINST(pl->line, "push") || ISINST(pl->line, "pop"))
    {
      if(!STRNCASECMP(op1start, "ix", 2) || !STRNCASECMP(op1start, "iy", 2))
        return(2);
      return(1);
    }

  /* 16 bit add / subtract / and / or */
  if(IS_Z80N && ISINST(pl->line, "add") && (!STRNCASECMP(op1start, "bc", 2) || !STRNCASECMP(op1start, "de", 2) || !STRNCASECMP(op1start, "hl", 2)))
    return(4);
  if((ISINST(pl->line, "add") || ISINST(pl->line, "adc") || ISINST(pl->line, "sbc") || IS_RAB && (ISINST(pl->line, "and") || ISINST(pl->line, "or"))) &&
     !STRNCASECMP(op1start, "hl", 2))
    {
      if(ISINST(pl->line, "add") || ISINST(pl->line, "and") || ISINST(pl->line, "or"))
        return(1);
      return(2);
    }
  if((ISINST(pl->line, "add") || IS_RAB && (ISINST(pl->line, "and") || ISINST(pl->line, "or")))&& (!STRNCASECMP(op1start, "ix", 2) || !STRNCASECMP(op1start, "iy", 2)))
    return(2);

  /* signed 8 bit adjustment to stack pointer */
  if((IS_RAB || IS_SM83) && ISINST(pl->line, "add") && !STRNCASECMP(op1start, "sp", 2))
    return(2);

  /* 16 bit adjustment to stack pointer */
  if(IS_TLCS90 && ISINST(pl->line, "add") && !STRNCASECMP(op1start, "sp", 2))
    return(3);

  /* 8 bit arithmetic, two operands */
  if(op2start && op1start[0] == 'a' &&
     (ISINST(pl->line, "add") || ISINST(pl->line, "adc") || ISINST(pl->line, "sub") || ISINST(pl->line, "sbc") ||
      ISINST(pl->line, "cp")  || ISINST(pl->line, "and") || ISINST(pl->line, "or")  || ISINST(pl->line, "xor")))
    {
      if(argCont(op2start, "(ix)") || argCont(op2start, "(iy)"))
        return(3);
      if(op2start[0] == '#')
        return(2);
      return(1);
    }
  /* 8 bit arithmetic, shorthand for a */
  if(!op2start &&
     (ISINST(pl->line, "add") || ISINST(pl->line, "adc") || ISINST(pl->line, "sub") || ISINST(pl->line, "sbc") ||
      ISINST(pl->line, "cp")  || ISINST(pl->line, "and") || ISINST(pl->line, "or")  || ISINST(pl->line, "xor")))
    {
      if(argCont(op1start, "(ix)") || argCont(op1start, "(iy)"))
        return(3);
      if(op1start[0] == '#')
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

  if(!IS_SM83 && (ISINST(pl->line, "reti") || ISINST(pl->line, "retn")))
    return(2);

  if(ISINST(pl->line, "ret") || ISINST(pl->line, "reti") || ISINST(pl->line, "rst"))
    return(1);

  if(ISINST(pl->line, "call"))
    return(3);

  /* Alias for ld a, (hl+)/(hld) etc */
  if(IS_SM83 && (ISINST(pl->line, "ldi") || ISINST(pl->line, "ldd")))
    return (1);

  if(ISINST(pl->line, "ldi") || ISINST(pl->line, "ldd") || ISINST(pl->line, "cpi") || ISINST(pl->line, "cpd"))
    return(2);

  if(ISINST(pl->line, "neg"))
    return(2);

  if(ISINST(pl->line, "daa") || ISINST(pl->line, "cpl")  || ISINST(pl->line, "ccf") || ISINST(pl->line, "scf") ||
     ISINST(pl->line, "nop") || ISINST(pl->line, "halt") || ISINST(pl->line,  "ei") || ISINST(pl->line, "di"))
    return(1);
  
  /* We always emit 2B stop due to hardware bugs*/
  if (IS_SM83 && ISINST(pl->line, "stop"))
    return(2);

  if(ISINST(pl->line, "im"))
    return(2);

  if(ISINST(pl->line, "in") || ISINST(pl->line, "out") || ISINST(pl->line, "ot") ||
     ISINST(pl->line, "ini") || ISINST(pl->line, "inir") || ISINST(pl->line, "ind") ||
     ISINST(pl->line, "indr") || ISINST(pl->line, "outi") || ISINST(pl->line, "otir") ||
     ISINST(pl->line, "outd") || ISINST(pl->line, "otdr"))
    {
      return(2);
    }

  if((IS_Z180 || IS_EZ80_Z80) && (ISINST(pl->line, "in0") || ISINST(pl->line, "out0")))
    return(3);

  if((IS_Z180 || IS_EZ80_Z80 || IS_Z80N) && ISINST(pl->line, "mlt"))
    return(2);

  if (IS_R800 && (ISINST(pl->line, "multu") || ISINST(pl->line, "multuw")))
    return(2);

  if((IS_Z180 || IS_EZ80_Z80 || IS_Z80N) && ISINST(pl->line, "tst"))
    return((op1start[0] == '#' || op2start && op1start[0] == '#') ? 3 : 2);

  if(IS_RAB && (ISINST(pl->line, "ioi") || ISINST(pl->line, "ioe")))
    return(1);
    
  if(IS_RAB && ISINST(pl->line, "mul"))
    return(1);

  if(ISINST(pl->line, "lddr") || ISINST(pl->line, "ldir") || ISINST(pl->line, "cpir") || ISINST(pl->line, "cpdr"))
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

  if((IS_SM83 || IS_Z80N) && ISINST(pl->line, "swap"))
    return(2);

  if(IS_Z80N && (ISINST(pl->line, "swapnib") || ISINST(pl->line, "mirror") || ISINST(pl->line, "mul") ||
     ISINST(pl->line, "outinb") || ISINST(pl->line, "ldix") || ISINST(pl->line, "ldirx") ||
     ISINST(pl->line, "lddx") || ISINST(pl->line, "lddrx") || ISINST(pl->line, "ldpirx")))
    return(2);

  if(IS_Z80N &&
    (ISINST(pl->line, "bsla") ||
    ISINST(pl->line, "bsra") ||
    ISINST(pl->line, "bsrl") ||
    ISINST(pl->line, "bsrf") ||
    ISINST(pl->line, "brlc")))
    return(2);

  if(IS_Z80N && ISINST(pl->line, "nextreg"))
    return(op2start && !STRNCASECMP(op2start, "a", 1) ? 3 : 4);

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
  
  if(IS_SM83 && ISINST(pl->line, ".tile"))
    {
      const char *value;
      int i;
      // skip directive
      for (value = pl->line; *value && !isspace (*value); ++value);
      // skip space
      for (; *value && isspace (*value); ++value);
      // just part of the syntax
      if(*value == '^')
        ++value;
      // delimiter can be freely chosen
      char delimiter = *(value++);
      for (i = 0; *value && *value != delimiter; ++value, ++i);
      // has to end with delimiter
      // 8 characters per 2B tile
      if (*value == delimiter && i%8 == 0)
        return i/4;
      
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

bool z80symmParmStack (const char *name)
{
  if (!strcmp (name, "___sdcc_enter_ix"))
   return false;
  return z80_symmParm_in_calls_from_current_function;
}


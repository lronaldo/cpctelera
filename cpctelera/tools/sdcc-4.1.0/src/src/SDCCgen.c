/*-------------------------------------------------------------------------
  SDCCgen.c - source files for target code generation common functions

  Copyright (C) 2012, Borut Razem

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

/* Use the D macro for basic (unobtrusive) debugging messages */
#define D(x) do if (options.verboseAsm) {x;} while(0)

genLine_t genLine;

/*-----------------------------------------------------------------*/
/* newLineNode - creates a new peep line                           */
/*-----------------------------------------------------------------*/
lineNode *
newLineNode (const char *line)
{
  lineNode *pl;

  pl = Safe_alloc (sizeof (lineNode));
  pl->line = Safe_strdup (line);
  pl->ic = NULL;
  return pl;
}

/*-----------------------------------------------------------------*/
/* connectLine - connects two lines                                */
/*-----------------------------------------------------------------*/
lineNode *
connectLine (lineNode * pl1, lineNode * pl2)
{
  if (!pl1 || !pl2)
    {
      fprintf (stderr, "trying to connect null line\n");
      return NULL;
    }

  pl2->prev = pl1;
  pl1->next = pl2;

  return pl2;
}

void
destroy_line_list (void)
{
  lineNode *pl;

  pl = genLine.lineCurr;

  while (pl)
    {
      lineNode *p;

      if (pl->line)
        Safe_free (pl->line);

      if (pl->aln)
        Safe_free (pl->aln);

      p = pl;
      pl = pl->prev;
      Safe_free (p);
    }
  genLine.lineHead = genLine.lineCurr = NULL;
}

/*-----------------------------------------------------------------*/
/* emit_raw - emit raw unformatted line                            */
/*-----------------------------------------------------------------*/
static void
add_line_node (const char *line)
{
  lineNode *pl;

  pl = Safe_alloc (sizeof (lineNode));

  memcpy (pl, (lineElem_t *) & genLine.lineElement, sizeof (lineElem_t));

  pl->line = Safe_strdup (line);

  if (genLine.lineCurr)
    {
      pl->next = NULL;
      genLine.lineCurr->next = pl;
      pl->prev = genLine.lineCurr;
      genLine.lineCurr = pl;
    }
  else
    {
      pl->prev = pl->next = NULL;
      genLine.lineCurr = genLine.lineHead = pl;
    }
}

void
emit_raw (const char *line)
{
  const char *p = line;

  while (isspace ((unsigned char) *p))
    p++;

  if (*p)
    {
      if (!port->rtrackUpdate || !port->rtrackUpdate(line))
        {
          genLine.lineElement.isComment = (*p == ';');
          add_line_node (line);
        }
    }
}

/*-----------------------------------------------------------------*/
/* format_opcode - format the opcode and arguments for emitting    */
/*-----------------------------------------------------------------*/
const char *
format_opcode (const char *inst, const char *fmt, va_list ap)
{
  struct dbuf_s dbuf;

  dbuf_init (&dbuf, INITIAL_INLINEASM);

  if (inst && *inst)
    {
      dbuf_append_str (&dbuf, inst);

      if (fmt && *fmt)
        {
          dbuf_append_char (&dbuf, '\t');
          dbuf_tvprintf (&dbuf, fmt, ap);
        }
    }
  else
    {
      if (fmt && *fmt)
        {
          dbuf_tvprintf (&dbuf, fmt, ap);
        }
    }

  return dbuf_detach_c_str (&dbuf);
}

void
va_emitcode (const char *inst, const char *fmt, va_list ap)
{
  const char *line = format_opcode (inst, fmt, ap);
  emit_raw (line);
  dbuf_free (line);
}

/*-----------------------------------------------------------------*/
/* emitcode - writes the code into a file : for now it is simple   */
/*-----------------------------------------------------------------*/
void
emitcode (const char *inst, const char *fmt, ...)
{
  va_list ap;

  va_start (ap, fmt);
  va_emitcode (inst, fmt, ap);
  va_end (ap);
}

void
emitLabel (const symbol *tlbl)
{
  if (!tlbl)
    return;
  emitcode ("", "!tlabeldef", labelKey2num (tlbl->key));
  genLine.lineCurr->isLabel = 1;
}

/*-----------------------------------------------------------------*/
/* genInline - write the inline code out                           */
/*-----------------------------------------------------------------*/
void
genInline (iCode * ic)
{
  char *buf, *bp, *begin;
  bool inComment = FALSE;
  bool inLiteral = FALSE;
  bool inLiteralString = FALSE;

  D (emitcode (";", "genInline"));

  genLine.lineElement.isInline += (!options.asmpeep);

  buf = bp = begin = Safe_strdup (IC_INLINE (ic));

  /* Emit each line as a code */
  while (*bp)
    {
      switch (*bp)
        {
        case '\'':
          inLiteral = !inLiteral;
          ++bp;
          break;

        case '"':
          inLiteralString = !inLiteralString;
          ++bp;
          break;

        case ';':
          if (!inLiteral && !inLiteralString)
            {
              inComment = TRUE;
            }
          ++bp;
          break;

        case ':':
          /* Add \n for labels, not dirs such as c:\mydir */
          if (!inComment && !inLiteral && !inLiteralString && (isspace ((unsigned char) bp[1])))
            {
              ++bp;
              *bp = '\0';
              ++bp;
              emitcode (begin, NULL);
              begin = bp;
            }
          else
            {
              ++bp;
            }
          break;

        case '\x87':
        case '\n':
          inLiteral = FALSE;
          inLiteralString = FALSE;
          inComment = FALSE;
          *bp++ = '\0';

          /* Don't emit leading whitespaces */
          while (isspace (*begin))
            ++begin;

          if (*begin)
            emitcode (begin, NULL);

          begin = bp;
          break;

        default:
          ++bp;
          break;
        }
    }
  if (begin != bp)
    {
      /* Don't emit leading whitespaces */
      while (isspace (*begin))
        ++begin;

      if (*begin)
        emitcode (begin, NULL);
    }

  Safe_free (buf);

  /* consumed; we can free it here */
  dbuf_free (IC_INLINE (ic));

  genLine.lineElement.isInline -= (!options.asmpeep);
}

/*-----------------------------------------------------------------*/
/* printLine - prints a line chain into a given file               */
/*-----------------------------------------------------------------*/
void
printLine (lineNode * head, struct dbuf_s *oBuf)
{
  iCode *last_ic = NULL;
  bool debug_iCode_tracking = (getenv ("DEBUG_ICODE_TRACKING") != NULL);

  while (head)
    {
      if (head->ic != last_ic)
        {
          last_ic = head->ic;
          if (debug_iCode_tracking)
            {
              if (head->ic)
                dbuf_printf (oBuf, "; block = %d, seq = %d\n", head->ic->block, head->ic->seq);
              else
                dbuf_append_str (oBuf, "; iCode lost\n");
            }
        }

      /* don't indent comments & labels */
      if (head->line && (head->isComment || head->isLabel))
        {
          dbuf_printf (oBuf, "%s\n", head->line);
        }
      else
        {
          if (head->isInline && *head->line == '#')
            {
              /* comment out preprocessor directives in inline asm */
              dbuf_append_char (oBuf, ';');
            }
          dbuf_printf (oBuf, "\t%s\n", head->line);
        }
      head = head->next;
    }
}

/*-----------------------------------------------------------------*/
/* ifxForOp - returns the icode containing the ifx for operand     */
/*-----------------------------------------------------------------*/
iCode *
ifxForOp (const operand *op, const iCode *ic)
{
  iCode *ifxIc;

  /* if true symbol then needs to be assigned */
  if (!IS_TRUE_SYMOP (op))
    {
      /* if this has register type condition and
         while skipping ipop's (see bug 1509084),
         the next instruction is ifx with the same operand
         and live to of the operand is upto the ifx only then */
      for (ifxIc = ic->next; ifxIc && ifxIc->op == IPOP; ifxIc = ifxIc->next)
        ;

      if (ifxIc && ifxIc->op == IFX &&
        IC_COND (ifxIc)->key == op->key &&
        OP_SYMBOL_CONST (op)->liveFrom >= ic->seq &&
        OP_SYMBOL_CONST (op)->liveTo <= ifxIc->seq)
        return ifxIc;
    }

  return NULL;
}

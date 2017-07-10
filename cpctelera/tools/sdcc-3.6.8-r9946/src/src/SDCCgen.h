/*-------------------------------------------------------------------------
  SDCCgen.h - header file for target code generation common functions

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

#ifndef SDCCGEN_H
#define SDCCGEN_H 1

#include <stdarg.h>

#include <SDCCicode.h>

#define initGenLineElement()  memset (&genLine.lineElement, 0, sizeof (lineElem_t))
#define labelKey2num(key)     ((key) + 100)

/* can be inherited by each port */
typedef struct asmLineNodeBase
{
  int size;
  bitVect *regsRead;
  bitVect *regsWritten;
}
asmLineNodeBase;

typedef struct lineElem_s
{
  char *line;
  iCode *ic;
  unsigned int isInline:1;
  unsigned int isComment:1;
  unsigned int isDebug:1;
  unsigned int isLabel:1;
  unsigned int visited:1;
  asmLineNodeBase *aln;
}
lineElem_t;

typedef struct lineNode_s
{
#ifdef UNNAMED_STRUCT_TAG
  struct lineElem_s;
#else
  /* exactly the same members as of struct lineElem_s */
  char *line;
  iCode *ic;
  unsigned int isInline:1;
  unsigned int isComment:1;
  unsigned int isDebug:1;
  unsigned int isLabel:1;
  unsigned int visited:1;
  asmLineNodeBase *aln;
#endif
  struct lineNode_s *prev;
  struct lineNode_s *next;
}
lineNode;

typedef struct genLine_s
{
  /* double linked list of lines */
  lineNode *lineHead;
  lineNode *lineCurr;

  /* global line */
  lineElem_t lineElement;
} genLine_t;

extern genLine_t genLine;

#ifdef __cplusplus
extern "C" {
#endif

lineNode *newLineNode (const char *line);
lineNode *connectLine (lineNode * pl1, lineNode * pl2);
void destroy_line_list (void);
const char *format_opcode (const char *inst, const char *fmt, va_list ap);
void emit_raw (const char *line);
void va_emitcode (const char *inst, const char *fmt, va_list ap);
void emitcode (const char *inst, const char *fmt, ...);
void emitLabel (symbol * tlbl);
void genInline (iCode * ic);
void printLine (lineNode *, struct dbuf_s *);
iCode *ifxForOp (operand *op, const iCode *ic);

#ifdef __cplusplus
}
#endif

#endif

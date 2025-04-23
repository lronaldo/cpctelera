/*
enum-1.c from the execute part of the gcc torture tests.
*/

#include <testfwk.h>

typedef enum
{
  END = -1,
  EMPTY = (1 << 8 ) ,
  BACKREF,
  BEGLINE,
  ENDLINE,
  BEGWORD,
  ENDWORD,
  LIMWORD,
  NOTLIMWORD,
  QMARK,
  STAR,
  PLUS,
  REPMN,
  CAT,
  OR,
  ORTOP,
  LPAREN,
  RPAREN,
  CSET
} token;

static token tok;

static int
atom ()
{
  if ((tok >= 0 && tok < (1 << 8 ) ) || tok >= CSET || tok == BACKREF
      || tok == BEGLINE || tok == ENDLINE || tok == BEGWORD
      || tok == ENDWORD || tok == LIMWORD || tok == NOTLIMWORD)
    return 1;
  else
    return 0;
}

void
testTortureExecute (void)
{
  tok = 0;
  if (atom () != 1)
    ASSERT (0);
  return;
}

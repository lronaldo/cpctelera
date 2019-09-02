/*
   pr66226.c from the execute part of the gcc torture tests.
 */

#include <testfwk.h>

struct assembly_operand
{
  int type, value, symtype, symflags, marker;
};

struct assembly_operand to_input, from_input;
#if 0 // Enabe when SDCC can pass struct
void
assemblez_1 (int internal_number, struct assembly_operand o1)
{
  if (o1.type != from_input.type)
    __builtin_abort ();
}

void
t0 (struct assembly_operand to, struct assembly_operand from)
{
  if (to.value == 0)
    assemblez_1 (32, from);
  else
    ASSERT (0);
}
#endif
void
testTortureExecute (void)
{
  to_input.value = 0;
  to_input.type = 1;
  to_input.symtype = 2;
  to_input.symflags = 3;
  to_input.marker = 4;

  from_input.value = 5;
  from_input.type = 6;
  from_input.symtype = 7;
  from_input.symflags = 8;
  from_input.marker = 9;
#if 0
  t0 (to_input, from_input);
#endif
  return;
}

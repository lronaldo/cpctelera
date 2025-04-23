/*
   20111208-1.c from the execute part of the gcc torture suite.
 */

#include <testfwk.h>

#ifdef __SDCC
#pragma std_c99
#endif

#include <stdint.h>
#include <string.h>

#if !(defined (__GNUC__) && __GNUC__ < 5)
/* PR tree-optimization/51315 */
/* Reported by Jurij Smakov <jurij@wooyd.org> */

int a;

static void
do_something (int item)
{
  a = item;
}

int
pack_unpack (char *s, char *p)
{
  char *send, *pend;
  char type;
  int integer_size;

  send = s + strlen (s);
  pend = p + strlen (p);

  while (p < pend)
    {
      type = *p++;

      switch (type)
 {
 case 's':
   integer_size = 2;
   goto unpack_integer;

 case 'l':
   integer_size = 4;
   goto unpack_integer;

 unpack_integer:
   switch (integer_size)
     {
     case 2:
       {
  union
  {
    int16_t i;
    char a[sizeof (int16_t)];
  }
  v;
  memcpy (v.a, s, sizeof (int16_t));
  s += sizeof (int16_t);
  do_something (v.i);
       }
       break;

     case 4:
       {
  union
  {
    int32_t i;
    char a[sizeof (int32_t)];
  }
  v;
  memcpy (v.a, s, sizeof (int32_t));
  s += sizeof (int32_t);
  do_something (v.i);
       }
       break;
     }
   break;
 }
    }
  return (int) *s;
}
#endif

void
testTortureExecute (void)
{
#if !(defined (__GNUC__) && __GNUC__ < 5)
  int n = pack_unpack ("\200\001\377\376\035\300", "sl");
  if (n != 0)
    ASSERT (0);
  return;
#endif
}


/*
   20020619-1.c from the execute part of the gcc torture suite.
 */

#include <testfwk.h>

#ifdef __SDCC
#pragma std_c99
#endif

#include <stdint.h>
#include <string.h>

typedef int32_t int32;

static int32 ref(void)
{
  union {
    char c[5];
    int32 i;
  } u;

  memset (&u, 0, sizeof(u));
  u.c[0] = 1;
  u.c[1] = 2;
  u.c[2] = 3;
  u.c[3] = 4;

  return u.i;
}

void
testTortureExecute (void)
{
  int32 b = ref();
  if (b != 0x01020304
      && b != 0x04030201)
    ASSERT (0);
  return;
}

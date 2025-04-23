/*
    bug1981238.c
*/
#include <testfwk.h>

#pragma disable_warning 147

#if defined(__SUNPRO_C) || defined(__GNUC__) || defined(__clang__)
#pragma pack(1)
#endif

__code struct {
  char x:1;
  char  :0;
  char d:2;
  char b:5;
} pad = {1, 2, 1};

__code struct {
  int p:1;
  int q:7;
  char :0;
  int s:1;
  int t:7;
} noPad = {1, 20, 1, 127};

__code struct {
  char  :0;
  char b;
} initialNoPad[] = {{2}, {4}};

void
testBitfield (void)
{
#if (defined (__SDCC) || defined(__SUNPRO_C) || defined(__GNUC__) || defined(__clang__)) && !defined(__POWERPC__)
  ASSERT (sizeof (pad) == 2);
  ASSERT (sizeof (noPad) == 2);
  ASSERT (sizeof (initialNoPad[0]) == 1);

  ASSERT (pad.x == -1 || pad.x == 1);
  ASSERT (pad.d == -2 || pad.d == 2);
  ASSERT (pad.b == 1);
  ASSERT (noPad.p == -1 || noPad.p == 1);
  ASSERT (noPad.q == 20);
  ASSERT (noPad.s == -1 || noPad.s == 1);
  ASSERT (noPad.t == -1 || noPad.t == 127);

  ASSERT (initialNoPad[0].b == 2);
  ASSERT (initialNoPad[1].b == 4);
#endif
}

